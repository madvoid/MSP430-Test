// template.c
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//	http://hci.rwth-aachen.de/msp430	
//
// Requirements:
// 	Requires msp430-gcc and MSP430 Launchpad (MSPEXP430G2)
//
// Description:
//	Blank template for msp430 project
//
// Notes:
//	
//
//****************************************************************************************************




// Includes ------------------------------------------------------------------------------------------
#include <msp430g2553.h>




// Defines -------------------------------------------------------------------------------------------
#define LED_RED BIT0 			// Red LED on LaunchPad
#define LED_GREEN BIT6			// Green LED on LaunchPad
#define LED_OUT P1OUT
#define LED_DIR P1DIR




// Functions -----------------------------------------------------------------------------------------
void delay(unsigned long count){
	volatile long i;        // Declare i as volatile int
	for(i=0;i<count;i++);  // Repeat x times
}




// Main ----------------------------------------------------------------------------------------------
int main(void){
	// Turn off watchdog timer
	WDTCTL = WDTPW|WDTHOLD;

	// Set LED output as 0
	LED_OUT  = 0x00;

	// Set pin as I/O.
	P1SEL  = 0x00;

	// Set direction
	LED_DIR = LED_RED | LED_GREEN;

	LED_OUT = LED_RED;

	// Loop
	while (1){
		//  Toggle the LED ouput pins
		delay(16000);
		LED_OUT ^= LED_RED | LED_GREEN;
	}	
}
