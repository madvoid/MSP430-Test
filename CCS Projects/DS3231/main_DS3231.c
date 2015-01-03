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



// Main ----------------------------------------------------------------------------------------------
int main(void) {
		WDTCTL = WDTPW | WDTHOLD;       // Stop WDT

		// Configure GPIO
		P1OUT &= ~BIT0;                           // Clear P1.0 output latch
		P1DIR |= BIT0;                            // For LED
		P1SEL1 |= BIT6 | BIT7;                    // I2C pins
		P1DIR &= ~BIT1;							  // Set P1.1 as input
		P1OUT |= BIT1;                      	  // Pull-up resistor on P1.1 for switch
		P1REN |= BIT1;                      	  // Select pull-up mode for P1.1 for switch
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

		DS3231GetCurrentTime();		//{seconds, minutes, hours, day, date, month, year}

		__no_operation();
}


