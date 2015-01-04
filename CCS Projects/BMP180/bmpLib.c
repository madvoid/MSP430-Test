// bmpLib.c
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
// 	Library to interface with BMP180
//
// Notes:
//
// Todo:
//	Make more durable, timeouts, testing, etc.
//****************************************************************************************************



// Includes ------------------------------------------------------------------------------------------
#include <msp430.h>
#include <stdint.h>
#include "main.h"
#include "bmpLib.h"




// Functions -----------------------------------------------------------------------------------------
void BMP180GetCalVals(tBMP180Cals *calInst){

	// Initialize variables
	static const uint8_t bmpCalRegs[11] = {BMP180_REG_CAL_AC1, BMP180_REG_CAL_AC2, BMP180_REG_CAL_AC3, BMP180_REG_CAL_AC4, BMP180_REG_CAL_AC5, BMP180_REG_CAL_AC6, BMP180_REG_CAL_B1, BMP180_REG_CAL_B2, BMP180_REG_CAL_MB, BMP180_REG_CAL_MC, BMP180_REG_CAL_MD};
	uint8_t bmpCalBytes[22];		// Received byte storage
	uint8_t bmpCalCount;		// Calibration values recieved

	// Reset counts
	bmpCalCount = 0;
	g_bmpByteCountEnd = 2;		// Each send will be responded with 2 bytes

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;					  // Auto stop
	UCB0TBCNT = g_bmpByteCountEnd;			  // Auto stop count
	UCB0I2CSA = BMP180_I2C_ADDRESS;           // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE &= ~UCRXIE;						  // Set rx interrupt
	UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

	while(bmpCalCount < 11){
		UCB0CTLW0 |= UCTXSTT;				// Send start
		while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt flag
		UCB0TXBUF = bmpCalRegs[bmpCalCount];// Send data byte
		while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt flag
		UCB0CTLW0 &= ~UCTR;					// Change to receive
		UCB0CTLW0 |= UCTXSTT;				// Send restart
		while(UCB0CTLW0 & UCTXSTT);			// Wait for restart

		while(!(UCB0IFG & UCRXIFG0));		// Wait for RX interrupt flag
		bmpCalBytes[2*bmpCalCount] = UCB0RXBUF;	// Read rxbuffer
		while(!(UCB0IFG & UCRXIFG0));		// Wait for RX interrupt flag
		bmpCalBytes[2*bmpCalCount+1] = UCB0RXBUF;	// Read rxbuffer
		while(UCB0CTLW0 & UCTXSTP);		// Wait for stop

		bmpCalCount++;					// Increment calibration count
		UCB0CTLW0 |= UCTR;					// tx mode
	}

	calInst->ac1 = (int16_t) ( (bmpCalBytes[0] << 8) | bmpCalBytes[1] );
	calInst->ac2 = (int16_t) ( (bmpCalBytes[2] << 8) | bmpCalBytes[3] );
	calInst->ac3 = (int16_t) ( (bmpCalBytes[4] << 8) | bmpCalBytes[5] );
	calInst->ac4 = (uint16_t)( (bmpCalBytes[6] << 8) | bmpCalBytes[7] );
	calInst->ac5 = (uint16_t)( (bmpCalBytes[8] << 8) | bmpCalBytes[9] );
	calInst->ac6 = (uint16_t)( (bmpCalBytes[10] << 8) | bmpCalBytes[11] );
	calInst->b1 =  (int16_t) ( (bmpCalBytes[12] << 8) | bmpCalBytes[13] );
	calInst->b2 =  (int16_t) ( (bmpCalBytes[14] << 8) | bmpCalBytes[15] );
	calInst->mb =  (int16_t) ( (bmpCalBytes[16] << 8) | bmpCalBytes[17] );
	calInst->mc =  (int16_t) ( (bmpCalBytes[18] << 8) | bmpCalBytes[19] );
	calInst->md =  (int16_t) ( (bmpCalBytes[20] << 8) | bmpCalBytes[21] );
}


void BMP180GetRawTemp(void){

	// Reset counts
	g_bmpByteCountEnd = 0x02;		// Each send will be responded with 2 bytes

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;					  // Auto stop
	UCB0TBCNT = g_bmpByteCountEnd;			  // Auto stop count
	UCB0I2CSA = BMP180_I2C_ADDRESS;           // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE &= ~UCRXIE;						  // Set rx interrupt
	UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

	// Start transmission
	UCB0CTLW0 |= UCTXSTT;				// Send start
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt flag
	UCB0TXBUF = BMP180_REG_CONTROL;     // Send control register address
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt flag
	UCB0TXBUF = BMP180_READ_TEMP;		// Send temperature measurement start command
	while(UCB0CTLW0 & UCTXSTP);			// Ensure stop condition got sent

	// Setup timer for 4.5ms measurement delay
	TB0CCTL0 = CCIE;                        // TBCCR0 interrupt enabled
	TB0CCR0 = 175;							// (175 ticks) * (1 second / 32768 ticks) = 5.3 ms > 4.5 ms required
	TB0CTL = TBSSEL__ACLK | MC__UP;         // ACLK, up mode

	__bis_SR_register(LPM3_bits);           // Enter LPM3 w/ interrupt

	// Start transmission
	UCB0CTLW0 |= UCTXSTT;				// Send start
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt flag
	UCB0TXBUF = BMP180_REG_TEMPDATA;    // Send data byte
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt flag
	UCB0CTLW0 &= ~UCTR;					// Change to receive
	UCB0CTLW0 |= UCTXSTT;				// Send restart
	while(UCB0CTLW0 & UCTXSTT);			// Wait for restart
	while(!(UCB0IFG & UCRXIFG));		// Wait for receive
	g_bmpValBytes[0] = UCB0RXBUF;		// Receive first byte
	while(!(UCB0IFG & UCRXIFG));		// Wait for receive
	g_bmpValBytes[1] = UCB0RXBUF;		// Receive second byte
	while(UCB0CTLW0 & UCTXSTP);			// Wait for stop
}


void BMP180GetTemp(tBMP180Cals *calInst){
	BMP180GetRawTemp();

	// Calculate UT
	int32_t UT = (int32_t)( (g_bmpValBytes[0] << 8) + g_bmpValBytes[1] );

	// Calculate X1
	int32_t X1 = (UT - (int32_t)calInst->ac6) * ((int32_t)calInst->ac5) / 32768;		// TODO: Optimize by bitshift?

	// Calculate X2
	int32_t X2 = ((int32_t)calInst->mc * 2048) / (X1 + (int32_t)calInst->md);

	// Calculate B5
	int32_t B5 = X1 + X2;

	g_bmpTemp = ( ((float)B5 + 8.0f)/16.0f )/10.0f;		// Divide by 10 because result is normally in 0.1C - See datasheet
}


void BMP180GetRawPressure(uint8_t oss){

	// Reset counts
	g_bmpByteCountEnd = 0x03;		// Each send will be responded with 2 bytes

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;					  // Auto stop
	UCB0TBCNT = g_bmpByteCountEnd;			  // Auto stop count
	UCB0I2CSA = BMP180_I2C_ADDRESS;           // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE &= ~UCRXIE;						  // Set rx interrupt
	UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

	// Start transmission
	UCB0CTLW0 |= UCTXSTT;				// Send start
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt flag
	UCB0TXBUF = BMP180_REG_CONTROL;     // Send control register address
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt flag
	UCB0TXBUF = (BMP180_READ_PRES_BASE + (oss < 6));	// Send pressure measurement start command
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt flag
	UCB0CTLW0 |= UCTXSTP;				// Send stop
	while(UCB0CTLW0 & UCTXSTP);			// Ensure stop condition got sent

	// Setup timer based on oversampling setting
	switch(oss){
	case 0:
		TB0CCR0 = 375;					// (375 ticks) * (1 second / 32768 ticks) = 11.4 ms > 4.5 ms required
		break;
	case 1:
		TB0CCR0 = 450;					// (450 ticks) * (1 second / 32768 ticks) = 13.7 ms > 7.5 ms required
		break;
	case 2:
		TB0CCR0 = 650;					// (650 ticks) * (1 second / 32768 ticks) = 19.8 ms > 13.5 ms required
		break;
	case 3:
		TB0CCR0 = 1050;					// (1050 ticks) * (1 second / 32768 ticks) = 32.0 ms > 25.5 ms required
		break;
	default:
		TB0CCR0 = 1050;					// Safe default value
		break;
	}

	// Start timer
	TB0CCTL0 = CCIE;                   // TBCCR0 interrupt enabled
	TB0CTL = TBSSEL__ACLK | MC__UP;    // ACLK, up mode

	__bis_SR_register(LPM3_bits);      // Enter LPM3 w/ interrupt

	// Start transmission
	UCB0CTLW0 |= UCTXSTT;				// Send start
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt flag
	UCB0TXBUF = BMP180_REG_PRESSUREDATA;// Send data byte
	while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt flag
	UCB0CTLW0 &= ~UCTR;					// Change to receive
	UCB0CTLW0 |= UCTXSTT;				// Send restart
	while(UCB0CTLW0 & UCTXSTT);			// Wait for restart
	while(!(UCB0IFG & UCRXIFG));		// Wait for receive
	g_bmpValBytes[0] = UCB0RXBUF;		// Receive first byte
	while(!(UCB0IFG & UCRXIFG));		// Wait for receive
	g_bmpValBytes[1] = UCB0RXBUF;		// Receive second byte
	while(!(UCB0IFG & UCRXIFG));		// Wait for receive
	g_bmpValBytes[2] = UCB0RXBUF;		// Receive third byte
	while(UCB0CTLW0 & UCTXSTP);			// Wait for stop
}


void BMP180GetPressure(uint8_t oss, tBMP180Cals *calInst){
	// Get raw temp
	BMP180GetTemp(calInst);

	// Calculate UT
	int32_t UT = (int32_t)( (g_bmpValBytes[0] << 8) + g_bmpValBytes[1] );

	// Get raw pressure
	BMP180GetRawPressure(oss);

	// Calculate UP
	int32_t UP = ( ((int32_t)g_bmpValBytes[0] << 16) + ((int32_t)g_bmpValBytes[1] << 8) + (int32_t)g_bmpValBytes[2]) >> (8 - oss);

	// Calculate X1
	int32_t X1 = (UT - (int32_t)calInst->ac6) * ((int32_t)calInst->ac5) / 32768;

	// Calculate X2
	int32_t X2 = ((int32_t)calInst->mc * 2048) / (X1 + (int32_t)calInst->md);

	// Calculate B5
	int32_t B5 = X1 + X2;

	// Calculate B6
	int32_t B6 = B5 - 4000;

	// Recalculate X1
	X1 = ((int32_t)calInst->b2 * ((B6*B6) / 4096)) / 2048;

	// Recalculate X2
	X2 = (int32_t)calInst->ac2 * B6 / 2048;

	// Calculate X3
	int32_t X3 = X1 + X2;

	// Calculate B3
	int32_t B3 = ( ( ((int32_t)calInst->ac1*4 + X3) << oss) + 2 ) / 4;

	// Recalculate X1 (2nd time)
	X1 = (int32_t)calInst->ac3 * B6 / 8192;

	// Recalculate X2 (2nd time)
	X2 = ((int32_t)calInst->b1 * ((B6*B6) / 4096)) / 65536;

	// Recalculate X3 (1st time)
	X3 = ((X1 + X2) + 2) / 4;

	// Calculate B4
	uint32_t B4 = (uint32_t)calInst->ac4 * (uint32_t)(X3 + 32768) / 32768;

	// Calculate B7
	uint32_t B7 = ((uint32_t)UP - B3) * (50000 >> oss);

	// Calculate p
	int32_t p;
	if(B7 < 0x80000000){
		p = (B7 * 2) / B4;
	} else{
		p = (B7 / B4) * 2;
	}

	// Recalculate X1 (3rd time)
	X1 = (p / 256) * (p / 256);
	X1 = (X1 * 3038) / 65536;

	// Recalculate X2 (3rd time)
	X2 = (-7537 * p) / 65536;

	// Recalculate P
	p = p + (X1 + X2 + 3791) / 16;

	g_pressure = p;
}
