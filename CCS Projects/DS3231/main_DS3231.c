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



// Defines -------------------------------------------------------------------------------------------
#define SETTIME 0	// Used to select whether to set the time or not
#define DS3231_I2C_ADDRESS 0x68
#define DS3231_REG_SECONDS    0x00
#define DS3231_REG_MINUTES    0x01
#define DS3231_REG_HOURS      0x02
#define DS3231_REG_DAYOFWEEK  0x03
#define DS3231_REG_DAYOFMONTH 0x04
#define DS3231_REG_MONTH      0x05
#define DS3231_REG_YEAR       0x06
#define DS3231_REG_A1SECONDS  0x07
#define DS3231_REG_A1MINUTES  0x08
#define DS3231_REG_A1HOURS    0x09
#define DS3231_REG_A1DAYDATE  0x0A
#define DS3231_REG_A2MINUTES  0x0B
#define DS3231_REG_A2HOURS    0x0C
#define DS3231_REG_A2DAYDATE  0x0D
#define DS3231_REG_CONTROL    0x0E
#define DS3231_REG_STATUS_CTL 0x0F
#define DS3231_REG_AGING      0x10
#define DS3231_REG_TEMP_MSB   0x11
#define DS3231_REG_TEMP_LSB   0x12



// Global --------------------------------------------------------------------------------------------
uint8_t g_setTimeArr[] = {00,43,03,4,31,12,14};	//{seconds, minutes, hours, day, date, month, year}
uint8_t g_getTimeArr[7];
uint8_t g_rtcTimeLength = 7;


// Functions -----------------------------------------------------------------------------------------
uint8_t bcd2bin (uint8_t val){
	return val - 6 * (val >> 4);
}

uint8_t bin2bcd (uint8_t val)
{
	return val + 6 * (val / 10);
}



// Main ----------------------------------------------------------------------------------------------
int main(void) {
		WDTCTL = WDTPW | WDTHOLD;       // Stop WDT

		// Configure GPIO
		P1OUT &= ~BIT0;                           // Clear P1.0 output latch
		P1DIR |= BIT0;                            // For LED
		P1SEL1 |= BIT6 | BIT7;                    // I2C pins
		P1DIR &= ~BIT1;						// Set P1.1 as input
		P1OUT |= BIT1;                      // Pull-up resistor on P1.1 for switch
		P1REN |= BIT1;                      // Select pull-up mode for P1.1 for switch
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

		int i;	// For loop counter
		#if SETTIME
			for(i = 0; i < g_rtcTimeLength; i++){
				g_setTimeArr[i] = bin2bcd(g_setTimeArr[i]);
			}

			while(P1IN & BIT1);		// Wait for switch

			// Configure USCI_B0 for I2C mode - Sending
			UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
			UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
			UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
			UCB0CTLW1 |= UCASTP_2;					  // Auto stop
			UCB0TBCNT = g_rtcTimeLength+1;			  // Auto stop count
			UCB0I2CSA = DS3231_I2C_ADDRESS;           // Slave address
			UCB0CTL1 &= ~UCSWRST;					  // Clear reset
			UCB0IE &= ~UCRXIE;						  // Clear rx interrupt
			UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

			UCB0CTLW0 |= UCTXSTT;			// Send start
			while(!(UCB0IFG & UCTXIFG0));	// Wait for tx interrupt
			UCB0TXBUF = DS3231_REG_SECONDS;	// Send register address

			for(i = 0; i < g_rtcTimeLength; i++){
				while(!(UCB0IFG & UCTXIFG0));	// Wait for tx interrupt
				UCB0TXBUF = g_setTimeArr[i];
//				if(i < g_rtcTimeLength - 1){
//					while(!(UCB0IFG & UCTXIFG0));
//				}
			}
			while(UCB0CTLW0 & UCTXSTP);	// Wait for stop

			__delay_cycles(500000);

			while(1);	// Stop here if time is being set

		#endif


			// Configure USCI_B0 for I2C mode - Sending
			UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
			UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
			UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
			UCB0CTLW1 |= UCASTP_2;					  // Auto stop
			UCB0TBCNT = g_rtcTimeLength;			  // Auto stop count
			UCB0I2CSA = DS3231_I2C_ADDRESS;           // Slave address
			UCB0CTL1 &= ~UCSWRST;					  // Clear reset
			UCB0IE &= ~UCRXIE;						  // Clear rx interrupt
			UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

			UCB0CTLW0 |= UCTXSTT;			// Send start
			while(!(UCB0IFG & UCTXIFG0));	// Wait for tx interrupt
			UCB0TXBUF = DS3231_REG_SECONDS;	// Send register address
			while(!(UCB0IFG & UCTXIFG0));
			UCB0CTLW0 |= UCTXSTP;					// Send stop
			UCB0CTLW0 &= ~UCTR;					// Change to receive
			while(UCB0CTLW0 & UCTXSTP);			// Ensure stop condition got sent

			// Configure USCI_B0 for I2C mode - Receiving
//			UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
//			UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Receiving, SMCLK
//			UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
//			UCB0CTLW1 |= UCASTP_2;					  // Auto stop
//			UCB0TBCNT = g_rtcTimeLength;			  // Auto stop count
//			UCB0I2CSA = DS3231_I2C_ADDRESS;           // Slave address
//			UCB0CTL1 &= ~UCSWRST;					  // Clear reset
//			UCB0IE &= ~UCRXIE;						  // Clear rx interrupt
//			UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

			UCB0CTLW0 |= UCTXSTT;			// Send start
			while(UCB0CTLW0 & UCTXSTT);	// Wait for tx interrupt
			for(i = 0; i < g_rtcTimeLength; i++){
				while(!(UCB0IFG & UCRXIFG0));		// Wait for RX interrupt flag
				g_getTimeArr[i] = UCB0RXBUF;		// Read data
			}
			while(UCB0CTLW0 & UCTXSTP);	// Wait for stop

			__no_operation();

			for(i = 0; i < g_rtcTimeLength; i++){
				g_getTimeArr[i] = bcd2bin(g_getTimeArr[i]);
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
    	break;
    case USCI_I2C_UCTXIFG0:                 // Vector 24: TXIFG0
    	break;
    default: break;
  }
}
