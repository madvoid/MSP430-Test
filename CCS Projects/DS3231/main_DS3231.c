// main_DS3231.c
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//	Inspired by https://github.com/mizraith/RTClib/blob/master/RTC_DS3231.cpp
//
// Requirements:
//
//
// Description:
// 	Interface with DS3231 to get time
//
// Notes:
//
// Todo:
//	Make more durable, timeouts, testing, etc.
//****************************************************************************************************



// Includes ------------------------------------------------------------------------------------------
#include <stdint.h>
#include <msp430.h>
#include "dsLib.h"
#include "helper.h"




// Variables -----------------------------------------------------------------------------------------
int count = 0;




// Main ----------------------------------------------------------------------------------------------
int main(void) {
		WDTCTL = WDTPW | WDTHOLD;       // Stop WDT

		// Configure GPIO
		  P1OUT = BIT1 | BIT3;                      // Pull-up resistor on P1.1, 1.3
		  P1REN = BIT1 | BIT3;                      // Select pull-up mode for P1.1, 1.3
		  P1DIR = 0xFF ^ (BIT1 | BIT3);             // Set all but P1.1, 1.3 to output direction
		  P1IES = BIT3;                             // P1.3 Hi/Lo edge
		  P1IFG = 0;                                // Clear all P1 interrupt flags
		  P1IE = BIT3;                              // P1.3 interrupt enabled
		  P1OUT |= BIT0;							// LED On
		P1SEL1 |= BIT6 | BIT7;                    // I2C pins
		PJSEL0 |= BIT4 | BIT5;					  // Set J.4 & J.5 to accept crystal input for ACLK

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

//		i2cSetReset();		// Disable i2c module (for testing)

		DS3231GetCurrentTime();
		__no_operation();
		DS3231SetAlarm1Plus10Sec();
		DS3231TurnAlarm1On();


		while(1){
			__bis_SR_register(LPM4_bits | GIE);     // Enter LPM4 w/interrupt
			P1OUT ^= BIT0;							// LED Toggle
			DS3231GetCurrentTime();
			DS3231SetAlarm1Plus10Sec();
			count++;
			if(count == 5){
				break;
			}
		}
		DS3231TurnAlarm1Off();

		__no_operation();
		while(1);
}



// Port 1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT1_VECTOR))) Port_1 (void)
#else
#error Compiler not supported!
#endif
{
  P1IFG &= ~(BIT3);                           // Clear P1.1 IFG
  __bic_SR_register_on_exit(LPM4_bits);     // Exit LPM4
}
