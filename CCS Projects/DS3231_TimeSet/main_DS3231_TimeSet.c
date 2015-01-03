// main_DS3231_TimeSet.c
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
// 	Set time on DS3231
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
uint8_t g_setTimeArr[] = {00,57,02,7,3,01,15};	//{seconds, minutes, hours, day, date, month, year}
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
	}
	while(UCB0CTLW0 & UCTXSTP);	// Wait for stop

	__delay_cycles(500000);

	__no_operation();

	while(1);	// Stop here if time is being set

}
