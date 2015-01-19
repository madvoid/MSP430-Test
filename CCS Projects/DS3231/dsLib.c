// dsLib.c
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//	Inspired by https://github.com/mizraith/RTClib/blob/master/RTC_DS3231.cpp
//
// Requirements:
//	dsLib.h
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
#include "dsLib.h"



// Functions -----------------------------------------------------------------------------------------
uint8_t bcd2bin (uint8_t val){
	return val - 6 * (val >> 4);
}

uint8_t bin2bcd (uint8_t val){
	return val + 6 * (val / 10);
}

void DS3231GetCurrentTime(void){
    int i;	// For loop counter
    static const uint8_t timeMask[] = {0x7F,0x7F,0x3F,0x7,0x3F,0x1F,0xFF}; //{seconds, minutes, hours, day, date, month, year}

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;					  // Auto stop
	UCB0TBCNT = DS3231_TIME_LENGTH;			  // Auto stop count
	UCB0I2CSA = DS3231_I2C_ADDRESS;           // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE &= ~UCRXIE;						  // Clear rx interrupt
	UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

	UCB0CTLW0 |= UCTXSTT;			// Send start
	while(!(UCB0IFG & UCTXIFG0));	// Wait for tx interrupt
	UCB0TXBUF = DS3231_REG_SECONDS;	// Send register address
	while(!(UCB0IFG & UCTXIFG0));	// Wait for send
	UCB0CTLW0 &= ~UCTR;				// Change to receive
	UCB0CTLW0 |= UCTXSTT;			// Send restart
	while(UCB0CTLW0 & UCTXSTT);		// Wait for start
	for(i = 0; i < DS3231_TIME_LENGTH; i++){
		while(!(UCB0IFG & UCRXIFG0));		// Wait for RX interrupt flag
		g_getTimeArr[i] = UCB0RXBUF;		// Read data
	}
	while(UCB0CTLW0 & UCTXSTP);	// Wait for stop

//	__no_operation();

	for(i = 0; i < DS3231_TIME_LENGTH; i++){
		g_getTimeArr[i] = bcd2bin(g_getTimeArr[i] & timeMask[i]);
	}
}


void DS3231SetAlarm1Plus10Sec(void){

	// Get current time's seconds, add 10 seconds
	uint8_t seconds = g_getTimeArr[0];
	if(seconds < 50){
		seconds = seconds + 10;
	}else{
		seconds = seconds - 50;
	}
	seconds = bin2bcd(seconds & 0x7F);	// Convert to BCD format and mask to register
	int i;	// For loop counter

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;					  // Auto stop
	UCB0TBCNT = 5;			  				  // Auto stop count - 4 alarm registers + address reg
	UCB0I2CSA = DS3231_I2C_ADDRESS;           // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE &= ~UCRXIE;						  // Clear rx interrupt
	UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

	UCB0CTLW0 |= UCTXSTT;				// Send start
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
	UCB0TXBUF = DS3231_REG_A1SECONDS;	// Send register address
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
	UCB0TXBUF = seconds;				// Send seconds
	for(i = 0; i < 3; i++){
		while(!(UCB0IFG & UCTXIFG0));	// Wait for tx interrupt
		UCB0TXBUF = 0x80;				// A1MX Masks to allow alarm to go on when seconds match
	}
	while(UCB0CTLW0 & UCTXSTP);	// Wait for stop

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;					  // Auto stop
	UCB0TBCNT = 5;			  				  // Auto stop count - 4 alarm registers + address reg
	UCB0I2CSA = DS3231_I2C_ADDRESS;           // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE &= ~UCRXIE;						  // Clear rx interrupt
	UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

	// Clear interrupt bit
	UCB0CTLW0 |= UCTXSTT;				// Send start
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
	UCB0TXBUF = DS3231_REG_STATUS_CTL;	// Send register address
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
	UCB0TXBUF = 0x00;					// 0b00000000 - No OSF, No 32KHz, No BSY, A2F, A1F
	UCB0CTLW0 |= UCTXSTP;				// Send stop
	while(UCB0CTLW0 & UCTXSTP);			// Wait for stop
}


void DS3231TurnAlarm1On(void){
	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;					  // Auto stop
	UCB0TBCNT = 2;			  				  // Address + 1 reg
	UCB0I2CSA = DS3231_I2C_ADDRESS;           // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE &= ~UCRXIE;						  // Clear rx interrupt
	UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

	// Clear interrupt bit
	UCB0CTLW0 |= UCTXSTT;				// Send start
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
	UCB0TXBUF = DS3231_REG_CONTROL;		// Send register address
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
	UCB0TXBUF = 0x1D;					// 0b00011101 - Osc on, SQWV Off, No convert temp, RS2,1 Whatever, INTCN 1, A2IE off, A1IE on
	while(UCB0CTLW0 & UCTXSTP);			// Wait for stop
}


void DS3231TurnAlarm1Off(void){
	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;					  // Auto stop
	UCB0TBCNT = 2;			  				  // Address + 1 reg
	UCB0I2CSA = DS3231_I2C_ADDRESS;           // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE &= ~UCRXIE;						  // Clear rx interrupt
	UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

	// Clear interrupt bit
	UCB0CTLW0 |= UCTXSTT;				// Send start
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
	UCB0TXBUF = DS3231_REG_CONTROL;		// Send register address
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
	UCB0TXBUF = 0x1C;					// 0b00011101 - Osc on, SQWV Off, No convert temp, RS2,1 Whatever, INTCN 1, A2IE off, A1IE on
	while(UCB0CTLW0 & UCTXSTP);			// Wait for stop
}
