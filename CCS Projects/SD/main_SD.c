#include <msp430.h>
#include "FatFS/ff.h"
#include "FatFS/diskio.h"


FATFS sdVolume;		// FatFs work area needed for each volume
FIL logfile;		// File object needed for each open file
uint16_t fp;		// Used for sizeof

uint8_t status = 17;


int main(void)
{
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
	  __bis_SR_register(GIE);	  WDTCTL = WDTPW | WDTHOLD;       // Stop WDT

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



	// Mount the SD Card
	switch( f_mount(&sdVolume, "", 0) ){
		case FR_OK:
			status = 41;
			break;
		case FR_INVALID_DRIVE:
			status = 1;
			break;
		case FR_DISK_ERR:
			status = 2;
			break;
		case FR_NOT_READY:
			status = 3;
			break;
		case FR_NO_FILESYSTEM:
			status = 4;
			break;
		default:
			status = 5;
			break;
	}

	status = status + 1;

	  __no_operation();
}
