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
#include "mlxLib.h"



// Defines -------------------------------------------------------------------------------------------




// Global --------------------------------------------------------------------------------------------




// Main ----------------------------------------------------------------------------------------------
int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
	// Configure GPIO
	P1OUT &= ~BIT0;                           // Clear P1.0 output latch
	P1DIR |= BIT0;                            // For LED
	P1SEL1 |= BIT6 | BIT7;                    // I2C pins
	PJSEL0 |= BIT4 | BIT5;					// Set J.4 & J.5 to accept crystal input for ACLK

	  // Prepare P1.1 for switch
	  P1OUT = BIT1;                      // Pull-up resistor on P1.1
	  P1REN = BIT1;                      // Select pull-up mode for P1.1

	  // Red LED
	  P4DIR = 0xFF;							  	// P4 output
	  P4OUT |= BIT6;							// P4.6 LED on

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

	  // Wait for button press
	  while(P1IN & BIT1);
	  P4OUT &= ~ BIT6;

	// Get object and ambient temperature
	MLX90615GetObjTemp();
	MLX90615Sleep();
	P1OUT |= BIT0;
	__delay_cycles(5000000);
	P1OUT &= ~BIT0;
	MLX90615Wake();
//	__delay_cycles(500000);
	MLX90615GetAmbTemp();

	P1OUT |= BIT0;

	__no_operation();

	while(1);		// Loop forever
}



// Interrupts ----------------------------------------------------------------------------------------

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
