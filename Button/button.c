// button.c
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//	http://hci.rwth-aachen.de/msp430	
//
// Requirements:
// 	Requires msp430-gcc
//
// Description:
//	Light blinks when button is pressed - uses interrupts
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
	WDTCTL = WDTPW|WDTHOLD;		// Turn off watchdog timer
	P1DIR = LED_GREEN;		// Set LEDs as outputs
	P1OUT = 0x08;			// P1.3 configured to pullup
	P1REN |= 0x08;			// P1.3 enable pullup
	P1IE |= 0x08;			// P1.3 interrupt enabled
	P1IES |= 0x08;			// P1.3 Hi/lo edge
	P1IFG &= ~0x08;			// Clear any existing interrupt

	__bis_SR_register(LPM4_bits + GIE);
}



// Port 1 ISR ----------------------------------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_INC__)
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT1_VECTOR))) Port_1(void)
#else
#error Compiler not supported!
#endif
{
	P1OUT ^= LED_GREEN;		// Toggle LED	
	delay(5000);
	P1IFG &= ~0x08;			// P1.3 IFG cleared
}
