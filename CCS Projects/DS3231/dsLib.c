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


uint8_t roundUpToTen(uint8_t val){
	// This function will round seconds up the nearest number evenly divisible by 10, except
	// in the case of 09, 19, 29, ... seconds in which it will add 11 to it. For example:
	// roundUpToTen(42) = 50
	// roundUpToTen(29) = 40

	if(val == 59){
		return 10;
	}

	if(val >= 49){
		return 0;
	}

	uint8_t rem = val % 10;

	if(rem == 9){
		val += 1;
		rem = 0;
	}

	return val + (10 - rem);
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


void DS3231ClearAlarm1Bits(void){
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
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
	UCB0CTLW0 |= UCTXSTP;				// Send stop
	while(UCB0CTLW0 & UCTXSTP);			// Wait for stop
}


void DS3231SetAlarm1Plus10Sec(void){

	// Get current time's seconds, add 10 seconds
	uint8_t seconds = g_getTimeArr[0];
	if(seconds < 50){
		seconds = seconds + 10;
	}else{
		seconds = seconds - 50;
	}
	seconds = bin2bcd(seconds);	// Convert to BCD format

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;					  // Auto stop
	UCB0TBCNT = 0x6;			  				  // Auto stop count - 6 greater than necessary, will manual stop
	UCB0I2CSA = DS3231_I2C_ADDRESS;           // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE &= ~UCRXIE;						  // Clear rx interrupt
	UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

	UCB0CTLW0 |= UCTXSTT;				// Send start
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
	UCB0TXBUF = DS3231_REG_A1SECONDS;	// Send register address
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt
	UCB0TXBUF = seconds & 0x7F;				// Mask and send seconds
	while(!(UCB0IFG & UCTXIFG0));	// Wait for tx interrupt
	UCB0TXBUF = 0x80;				// Send A1M2 Mask
	while(!(UCB0IFG & UCTXIFG0));	// Wait for tx interrupt
	UCB0TXBUF = 0x80;				// Send A1M3 Mask
	while(!(UCB0IFG & UCTXIFG0));	// Wait for tx interrupt
	UCB0TXBUF = 0x80;				// Send A1M4 Mask
	while(!(UCB0IFG & UCTXIFG0));	// Wait for tx interrupt
	UCB0CTLW0 |= UCTXSTP;					// Send stop
	while(UCB0CTLW0 & UCTXSTP);	// Wait for stop

	DS3231ClearAlarm1Bits();

}


void DS3231SetAlarm1Round10Sec(void){
	// This function is very similar to DS3231SetAlarm1Plus10Sec().
	// If RAM or memory becomes a problem, they will be merged to reduce resource consumption

	// Get current time's seconds, round to semi-nearest 10 seconds
	uint8_t seconds = g_getTimeArr[0];
	seconds = roundUpToTen(seconds);
	seconds = bin2bcd(seconds);	// Convert to BCD format

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;					  // Auto stop
	UCB0TBCNT = 0x6;			  			  // Auto stop count - 6 greater than necessary, will manual stop
	UCB0I2CSA = DS3231_I2C_ADDRESS;           // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE &= ~UCRXIE;						  // Clear rx interrupt
	UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

	UCB0CTLW0 |= UCTXSTT;					// Send start
	while(!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt
	UCB0TXBUF = DS3231_REG_A1SECONDS;		// Send register address
	while(!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt
	UCB0TXBUF = seconds & 0x7F;				// Mask and send seconds
	while(!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt
	UCB0TXBUF = 0x80;						// Send A1M2 Mask
	while(!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt
	UCB0TXBUF = 0x80;						// Send A1M3 Mask
	while(!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt
	UCB0TXBUF = 0x80;						// Send A1M4 Mask
	while(!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt
	UCB0CTLW0 |= UCTXSTP;					// Send stop
	while(UCB0CTLW0 & UCTXSTP);				// Wait for stop

	DS3231ClearAlarm1Bits();

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

	// Set interrupt bit
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
	UCB0TXBUF = 0x1C;					// 0b00011101 - Osc on, SQWV Off, No convert temp, RS2,1 Whatever, INTCN 1, A2IE off, A1IE off
	while(UCB0CTLW0 & UCTXSTP);			// Wait for stop
}
