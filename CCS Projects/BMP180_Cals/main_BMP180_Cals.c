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




// Defines -------------------------------------------------------------------------------------------
// BMP180 Registers & Commands
#define BMP180_I2C_ADDRESS 0x77
#define BMP180_READ_TEMP 0x2E
#define BMP180_READ_PRES_BASE 0x34
#define BMP180_REG_CAL_AC1 0xAA
#define BMP180_REG_CAL_AC2 0xAC
#define BMP180_REG_CAL_AC3 0xAE
#define BMP180_REG_CAL_AC4 0xB0
#define BMP180_REG_CAL_AC5 0xB2
#define BMP180_REG_CAL_AC6 0xB4
#define BMP180_REG_CAL_B1  0xB6
#define BMP180_REG_CAL_B2  0xB8
#define BMP180_REG_CAL_MB  0xBA
#define BMP180_REG_CAL_MC  0xBC
#define BMP180_REG_CAL_MD  0xBE
#define BMP180_REG_CHIPID  0xD0
#define BMP180_REG_VERSION 0xD1
#define BMP180_REG_SOFTRESET 0xE0
#define BMP180_REG_CONTROL 0xF4
#define BMP180_REG_TEMPDATA 0xF6
#define BMP180_REG_PRESSUREDATA 0xF6




// Global --------------------------------------------------------------------------------------------
static const uint8_t g_calRegs[11] = {BMP180_REG_CAL_AC1, BMP180_REG_CAL_AC2, BMP180_REG_CAL_AC3, BMP180_REG_CAL_AC4, BMP180_REG_CAL_AC5, BMP180_REG_CAL_AC6, BMP180_REG_CAL_B1, BMP180_REG_CAL_B2, BMP180_REG_CAL_MB, BMP180_REG_CAL_MC, BMP180_REG_CAL_MD};
uint8_t g_calBytes[22];		// Received byte storage
uint8_t g_calCount = 0;		// Calibration values recieved
uint8_t g_byteCount = 0;	// Bytes received in interrupt vector



// Main ----------------------------------------------------------------------------------------------
int main(void) {
	  WDTCTL = WDTPW | WDTHOLD;       // Stop WDT

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

	  // Configure USCI_B0 for I2C mode - Sending
	  UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	  UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	  UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	  UCB0I2CSA = BMP180_I2C_ADDRESS;            // Slave address
	  UCB0CTL1 &= ~UCSWRST;						 // Clear reset
	  UCB0IE |= UCTXIE0 | UCRXIE0;

	  while(g_calCount < 11){
		  UCB0CTL1 |= UCTXSTT;				// Start transfer
		  while (UCB0CTLW0 & UCTXSTT);  	// Ensure start condition got sent
		  __bis_SR_register(LPM0_bits);     // Enter LPM0 w/ interrupts
		  UCB0CTL1 |= UCTXSTP;				// Send stop
		  UCB0CTL1 |= UCTR;					// Change to write
		  g_calCount++;						// Increment calibration count
	  }

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
  switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
  {
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
    	__no_operation();
    	g_calBytes[2*g_calCount+g_byteCount] = UCB0RXBUF;	// Read rxbuffer
    	g_byteCount++;						// Increment byte count
    	if(g_byteCount == 2){
    		g_byteCount = 0;				// Reset byte count
//    		UCB0CTL1 |= UCTXSTP;			// Send stop
    		__bic_SR_register_on_exit(LPM0_bits); 	// Exit LPM0
    	}
    	break;
    case USCI_I2C_UCTXIFG0:                 // Vector 24: TXIFG0
    	UCB0TXBUF = g_calRegs[g_calCount];
    	UCB0CTLW0 &= ~UCTR;					// Change to read
    	UCB0CTL1 |= UCTXSTT;				// Send restart
    	break;
    default: break;
  }
}

