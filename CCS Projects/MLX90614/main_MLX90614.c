// main_MLX90614.c
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
// 	Interface with MLX90614 Library
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
#define MLX90614_I2C_ADDRESS 0x5A
// RAM
#define MLX90614_RAWIR1 0x04
#define MLX90614_RAWIR2 0x05
#define MLX90614_TA 0x06
#define MLX90614_TOBJ1 0x07
#define MLX90614_TOBJ2 0x08
// EEPROM - Section 8.4.5 of data sheet explains why 0x20 is added to these addresses
#define MLX90614_TOMAX 0x20
#define MLX90614_TOMIN 0x21
#define MLX90614_PWMCTRL 0x22
#define MLX90614_TARANGE 0x23
#define MLX90614_EMISS 0x24
#define MLX90614_CONFIG 0x25
#define MLX90614_ADDR 0x0E
#define MLX90614_ID1 0x3C
#define MLX90614_ID2 0x3D
#define MLX90614_ID3 0x3E
#define MLX90614_ID4 0x3F



// Global --------------------------------------------------------------------------------------------
volatile uint8_t g_mlxValBytes[3];		// Recieved value byte storage
volatile uint8_t g_mlxRxCount;			// Recieved value count
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
	UCB0I2CSA = MLX90614_I2C_ADDRESS;	// I2C Address
	UCB0CTL1 &= ~UCSWRST;		// Clear reset
	UCB0IE |= UCRXIE;			// Set rx interrupt
	UCB0IE &= ~UCTXIE;

	// Reset receive count
	g_mlxRxCount = 0;

	// Send object temperature read command
	UCB0CTL1 |= UCTXSTT;			// Send start
	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
	UCB0TXBUF = MLX90614_TOBJ1;		// Send object temperature command
	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
	UCB0CTLW0 &= ~UCTR;				// Change to receive
	UCB0CTLW0 |= UCTXSTT;			// Send restart
	while(UCB0CTLW0 & UCTXSTT);		// Wait for restart
	__bis_SR_register(LPM0_bits);	// Enter low power mode and wait for bytes

	g_objectTemp = 0.0;
	uint16_t tempVals = ( ((uint16_t) g_mlxValBytes[1]) << 8 ) | ( (uint16_t) g_mlxValBytes[0] );
	g_objectTemp = ((float) tempVals) * 0.02 - 273.15;

//	UCB0CTLW0 |= UCTR;				// Change to send
//	g_mlxRxCount = 0;				// Reset receive count
//
//	// Send ambient temperature read command
//	UCB0CTL1 |= UCTXSTT;			// Send start
//	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
//	UCB0TXBUF = MLX90614_TA;		// Send object temperature command
//	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
//	UCB0CTLW0 &= ~UCTR;				// Change to receive
//	UCB0CTLW0 |= UCTXSTT;			// Send restart
//	while(UCB0CTLW0 & UCTXSTT);		// Wait for restart
//	__bis_SR_register(LPM0_bits);	// Enter low power mode and wait for bytes
//
//	g_ambientTemp = 0.0;
//	tempVals = 0;
//	tempVals = ( ((uint16_t) g_mlxValBytes[1]) << 8 ) | ( (uint16_t) g_mlxValBytes[0] );
//	g_ambientTemp = ((float) tempVals) * 0.02 - 273.15;

	__no_operation();
}




// Interrupts ----------------------------------------------------------------------------------------

// EUSCI_B I2C Interrupt
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCI_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG)){
    case USCI_NONE:          break;         // Vector 0: No interrupts
    case USCI_I2C_UCALIFG:   break;         // Vector 2: ALIFG
    case USCI_I2C_UCNACKIFG: break;         // Vector 4: NACKIFG
    case USCI_I2C_UCSTTIFG:  break;         // Vector 6: STTIFG
    case USCI_I2C_UCSTPIFG:  break;         // Vector 8: STPIFG
    case USCI_I2C_UCRXIFG3:  break;         // Vector 10: RXIFG3
    case USCI_I2C_UCTXIFG3:  break;         // Vector 12: TXIFG3
    case USCI_I2C_UCRXIFG2:  break;         // Vector 14: RXIFG2
    case USCI_I2C_UCTXIFG2:  break;         // Vector 16: TXIFG2
    case USCI_I2C_UCRXIFG1:  break;         // Vector 18: RXIFG1
    case USCI_I2C_UCTXIFG1:  break;         // Vector 20: TXIFG1
    case USCI_I2C_UCRXIFG0:  		        // Vector 22: RXIFG0
    	g_mlxValBytes[g_mlxRxCount] = UCB0RXBUF;
    	g_mlxRxCount++;
    	if(g_mlxRxCount == 3){
    		__bic_SR_register_on_exit(LPM0_bits);
    	}
    	break;
    case USCI_I2C_UCTXIFG0:                 // Vector 24: TXIFG0
    	break;
    default: break;
  }
}


// Timer B1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer0_B0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_B0_VECTOR))) Timer0_B0_ISR (void)
#else
#error Compiler not supported!
#endif
{
	TB0CTL &= ~MC__STOP;
	__bic_SR_register_on_exit(LPM3_bits); 	// Exit LPM3
}
