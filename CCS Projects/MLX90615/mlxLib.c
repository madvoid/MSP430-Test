// mlxLib.c
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//	Inspired by Adafruit MLX90614 Library
//
// Requirements:
//	mlxLib.h
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
#include "mlxLib.h"



// Functions -----------------------------------------------------------------------------------------
void MLX90615GetObjTemp(void){
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
}

void MLX90615GetAmbTemp(void){
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
	uint16_t tempVals = 0;
	tempVals = ( ((uint16_t) g_mlxValBytes[1]) << 8 ) | ( (uint16_t) g_mlxValBytes[0] );
	g_ambientTemp = ((float) tempVals) * 0.02 - 273.15;
}


void MLX90615Sleep(void){
	// Configure USCI_B0 for I2 mode - Sending
	UCB0CTLW0 |= UCSWRST;		// Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK; // I2C mode, master, sync, sending, SMCLK
	UCB0BRW = 0xA;				// Baudrate = SMCLK / 10;
	UCB0CTLW1 |= UCASTP_2;		// Auto stop
	UCB0TBCNT = 0x02;			// Auto stop after 2 bytes
	UCB0I2CSA = MLX90615_I2C_ADDRESS;	// I2C Address
	UCB0CTL1 &= ~UCSWRST;		// Clear reset
	UCB0IE &= ~UCRXIE;			// Ensure TX/RX interrupt not set
	UCB0IE &= ~UCTXIE;

	UCB0CTL1 |= UCTXSTT;			// Send start
	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
	UCB0TXBUF = MLX90615_ENTER_SLEEP;		// Send sleep command
	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
	UCB0TXBUF = 0x6D;				// Send PEC, see Fig. 11 in 8.4.8.1 of data sheet
	while(UCB0CTLW0 & UCTXSTP);		// Wait for stop
}


void MLX90615Wake(void){
	// Switch to GPIO
	P1DIR |= (BIT6 | BIT7);			// Switch to output
	P1OUT |= (BIT6 | BIT7);			// Ensure both high
	P1SEL1 &= ~(BIT6 | BIT7);		// Switch P1.6, P1.7 to GPIO pin

	// Drive SCL low
	P1OUT &= ~(BIT7);				// P1.7 (SCL) to low

	// Delay for 39ms to wake from sleep
	__delay_cycles(39000);

	// SCL high and switch back to I2C mode
	P1OUT |= BIT7;
	P1SEL1 |= BIT6 | BIT7;                    // I2C pins

	// Delay for > 0.3 s for valid values to appear. Comment out, change if application allows
	__delay_cycles(305000);

	// For some reason, using timers and low power modes prevents the uC from sleeping later on
	// For now delay_cycles() will be used, timers will be added if it can be figured out
	// TODO: Convert delay_cycles() to timer values
	// NOTE: May need to change timers in other libraries
}
