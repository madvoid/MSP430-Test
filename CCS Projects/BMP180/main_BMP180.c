// main_BMP180_Cals.c
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//	Borrows from Texas Instruments' bmp180 library.
//
// Requirements:
//
//
// Description:
// 	Interface with BMP180 to get Calibration values
//
// Notes:
//
// Todo:
//	Make more durable, timeouts, testing, etc.
//****************************************************************************************************



// Includes ------------------------------------------------------------------------------------------
#include <stdint.h>
#include <msp430.h>
#include "main.h"
#include "bmpLib.h"



// Defines -------------------------------------------------------------------------------------------



// Global --------------------------------------------------------------------------------------------



// Main ----------------------------------------------------------------------------------------------
int main(void) {
	  WDTCTL = WDTPW | WDTHOLD;       // Stop WDT
	  g_sensorCode = 0x00;

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

	  // Create structure instance
	  tBMP180Cals BmpCals;

	  // Get calibration values
	  BMP180GetCalVals(&BmpCals);

	  // Get temperature values
	  BMP180GetTemp(&BmpCals);

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
    	if(g_sensorCode == SCODE_BMP180_CALS){
    		g_bmpCalBytes[2*g_bmpCalCount+g_bmpByteCount] = UCB0RXBUF;	// Read rxbuffer
    	}else if(g_sensorCode == SCODE_BMP180_VALS){
    		g_bmpValBytes[g_bmpByteCount] = UCB0RXBUF;
    	}
    	g_bmpByteCount++;						// Increment byte count
    	if(g_bmpByteCount == g_bmpByteCountEnd){
    		__bic_SR_register_on_exit(LPM0_bits); 	// Exit LPM0
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
	__bic_SR_register_on_exit(LPM3_bits); 	// Exit LPM0
}
