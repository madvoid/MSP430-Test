// main_MLX90615.c
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//	Inspired by Adafruit MLX90614 Library
//
// Requirements:
//
//
// Description:
// 	Interface with MLX90615
//
// Notes:
//
// Todo:
//	Make more durable, timeouts, testing, etc.
//****************************************************************************************************



// Includes ------------------------------------------------------------------------------------------
#include <stdint.h>
#include <msp430.h>



// Defines -------------------------------------------------------------------------------------------
#define MLX90615_I2C_ADDRESS 0x5B
// RAM - Section 8.4.5 of data sheet explains why 0x20 is added to these addresses
// Reserved value addresses not included
#define MX90615_RAWIR 0x025
#define MLX90615_TA 0x26
#define MLX90615_TOBJ 0x27
// EEPROM - Section 8.4.5 of data sheet explains why 0x10 is added to these addresses
// Fatory calibration values and ID number addresses not included
#define MLX90615_TMIN_SA 0x10
#define MLX90615_PWMTRANGE 0x11
#define MLX90615_CONFIG 0x12
#define MLX90615_EMISS 0x13
// Enter sleep command
#define MLX90625_ENTER_SLEEP 0xC6	// 0b1100,0110





// Global --------------------------------------------------------------------------------------------
uint8_t g_mlxValBytes[3];		// Recieved value byte storage
float g_objectTemp;						// Temperature of object in field of view
float g_ambientTemp;					// Ambient temperature



// Main ----------------------------------------------------------------------------------------------
int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
	// Configure GPIO
	P1OUT &= ~BIT0;                           // Clear P1.0 output latch
	P1DIR |= BIT0;                            // For LED
	P1SEL1 |= BIT6 | BIT7;                    // I2C pins
	PJSEL0 |= BIT4 | BIT5;					// Set J.4 & J.5 to accept crystal input for ACLK

	// Disable the GPIO power-on default high-impedance mode to activate
	// previously configured port settings
	PM5CTL0 &= ~LOCKLPM5;

	// Clock setup
	CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
	CSCTL1 = DCOFSEL_0;                       // Set DCO to 1MHz
	CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK; // Set ACLK = LFXTCLK; MCLK = DCO
	CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers to 1
	CSCTL4 &= ~LFXTOFF;						// Turn on LFXT

	// Lock CS registers - Why isn't PUC created?
	CSCTL0_H = 0;

	// Enable interrupts
	__bis_SR_register(GIE);



	// Configure USCI_B0 for I2 mode - Sending
	UCB0CTLW0 |= UCSWRST;		// Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK; // I2C mode, master, sync, sending, SMCLK
	UCB0BRW = 0xA;				// Baudrate = SMCLK / 10;
	UCB0CTLW1 |= UCASTP_2;		// Auto stop
	UCB0TBCNT = 0x03;			// Auto stop after 3 bytes
	UCB0I2CSA = MLX90615_I2C_ADDRESS;	// I2C Address
	UCB0CTL1 &= ~UCSWRST;		// Clear reset
	UCB0IE &= ~UCRXIE;			// Ensure Interrupts off
	UCB0IE &= ~UCTXIE;

	// Send object temperature read command
	UCB0CTL1 |= UCTXSTT;			// Send start
	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
	UCB0TXBUF = MLX90615_TOBJ;		// Send object temperature command
	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
	UCB0CTLW0 &= ~UCTR;				// Change to receive
	UCB0CTLW0 |= UCTXSTT;			// Send restart
	while(UCB0CTLW0 & UCTXSTT);		// Wait for restart

	// Receive Bytes
	while(!(UCB0IFG & UCRXIFG0));	// Wait for RX interrupt flag
	g_mlxValBytes[0] = UCB0RXBUF;	// 0th byte
	while(!(UCB0IFG & UCRXIFG0));	// Wait for RX interrupt flag
	g_mlxValBytes[1] = UCB0RXBUF;	// 1st byte
	while(!(UCB0IFG & UCRXIFG0));	// Wait for RX interrupt flag
	g_mlxValBytes[2] = UCB0RXBUF;	// 2nd byte
	while(UCB0CTLW0 & UCTXSTP);		// Wait for stop

	// Calculate temperature
	g_objectTemp = 0.0;
	uint16_t tempVals = ( ((uint16_t) g_mlxValBytes[1]) << 8 ) | ( (uint16_t) g_mlxValBytes[0] );
	g_objectTemp = ((float) tempVals) * 0.02 - 273.15;



	// Configure USCI_B0 for I2 mode - Sending
	UCB0CTLW0 |= UCSWRST;		// Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK; // I2C mode, master, sync, sending, SMCLK
	UCB0BRW = 0xA;				// Baudrate = SMCLK / 10;
	UCB0CTLW1 |= UCASTP_2;		// Auto stop
	UCB0TBCNT = 0x03;			// Auto stop after 3 bytes
	UCB0I2CSA = MLX90615_I2C_ADDRESS;	// I2C Address
	UCB0CTL1 &= ~UCSWRST;		// Clear reset
	UCB0IE &= ~UCRXIE;			// Ensure TX/RX interrupt not set
	UCB0IE &= ~UCTXIE;

	// Send ambient temperature read command
	UCB0CTL1 |= UCTXSTT;			// Send start
	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
	UCB0TXBUF = MLX90615_TA;		// Send object temperature command
	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
	UCB0CTLW0 &= ~UCTR;				// Change to receive
	UCB0CTLW0 |= UCTXSTT;			// Send restart
	while(UCB0CTLW0 & UCTXSTT);		// Wait for restart

	// Receive Bytes
	while(!(UCB0IFG & UCRXIFG0));	// Wait for RX interrupt flag
	g_mlxValBytes[0] = UCB0RXBUF;	// 0th byte
	while(!(UCB0IFG & UCRXIFG0));	// Wait for RX interrupt flag
	g_mlxValBytes[1] = UCB0RXBUF;	// 1st byte
	while(!(UCB0IFG & UCRXIFG0));	// Wait for RX interrupt flag
	g_mlxValBytes[2] = UCB0RXBUF;	// 2nd byte
	while(UCB0CTLW0 & UCTXSTP);		// Wait for stop

	g_ambientTemp = 0.0;
	tempVals = 0;
	tempVals = ( ((uint16_t) g_mlxValBytes[1]) << 8 ) | ( (uint16_t) g_mlxValBytes[0] );
	g_ambientTemp = ((float) tempVals) * 0.02 - 273.15;

	__no_operation();
}

