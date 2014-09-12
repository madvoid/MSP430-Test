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

	// Set sensor code
	g_sensorCode = SCODE_BMP180_CALS;

	// Reset counts
	g_bmpByteCount = 0;
	g_bmpCalCount = 0;
	g_bmpByteCountEnd = 2;		// Each send will be responded with 2 bytes

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;					  // Auto stop
	UCB0TBCNT = g_bmpByteCountEnd;			  // Auto stop count
	UCB0I2CSA = BMP180_I2C_ADDRESS;           // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE |= UCRXIE;						  // Set rx interrupt
	UCB0IE &= ~UCTXIE;						  // Clear tx interrupt

	while(g_bmpCalCount < 11){
		UCB0CTLW0 |= UCTXSTT;				// Send start
		while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt flag
		UCB0TXBUF = g_bmpCalRegs[g_bmpCalCount];// Send data byte
		while(!(UCB0IFG & UCTXIFG0));		// Wait for tx interrupt flag
		UCB0CTLW0 &= ~UCTR;					// Change to receive
		UCB0CTLW0 |= UCTXSTT;				// Send restart
		while(UCB0CTLW0 & UCTXSTT);			// Wait for restart
		__bis_SR_register(LPM0_bits);		// Enter low power mode and wait for bytes
		g_bmpCalCount++;					// Increment calibration count
		g_bmpByteCount = 0;					// Reset byte count
		UCB0CTLW0 |= UCTR;					// tx mode
	}

	calInst->ac1 = (int16_t) ( (g_bmpCalBytes[0] << 8) | g_bmpCalBytes[1] );
	calInst->ac2 = (int16_t) ( (g_bmpCalBytes[2] << 8) | g_bmpCalBytes[3] );
	calInst->ac3 = (int16_t) ( (g_bmpCalBytes[4] << 8) | g_bmpCalBytes[5] );
	calInst->ac4 = (uint16_t)( (g_bmpCalBytes[6] << 8) | g_bmpCalBytes[7] );
	calInst->ac5 = (uint16_t)( (g_bmpCalBytes[8] << 8) | g_bmpCalBytes[9] );
	calInst->ac6 = (uint16_t)( (g_bmpCalBytes[10] << 8) | g_bmpCalBytes[11] );
	calInst->b1 =  (int16_t) ( (g_bmpCalBytes[12] << 8) | g_bmpCalBytes[13] );
	calInst->b2 =  (int16_t) ( (g_bmpCalBytes[14] << 8) | g_bmpCalBytes[15] );
	calInst->mb =  (int16_t) ( (g_bmpCalBytes[16] << 8) | g_bmpCalBytes[17] );
	calInst->mc =  (int16_t) ( (g_bmpCalBytes[18] << 8) | g_bmpCalBytes[19] );
	calInst->md =  (int16_t) ( (g_bmpCalBytes[20] << 8) | g_bmpCalBytes[21] );
}


void BMP180GetRawTemp(void){

	// Set sensor code
	g_sensorCode = SCODE_BMP180_VALS;

	// Reset counts
	g_bmpByteCount = 0;
	g_bmpByteCountEnd = 0x02;		// Each send will be responded with 2 bytes

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;					  // Auto stop
	UCB0TBCNT = g_bmpByteCountEnd;			  // Auto stop count
	UCB0I2CSA = BMP180_I2C_ADDRESS;           // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE |= UCRXIE;						  // Set rx interrupt
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
	__bis_SR_register(LPM0_bits);		// Enter low power mode and wait for bytes
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

	// Set sensor code
	g_sensorCode = SCODE_BMP180_VALS;

	// Reset counts
	g_bmpByteCount = 0;
	g_bmpByteCountEnd = 0x03;		// Each send will be responded with 2 bytes

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;					  // Auto stop
	UCB0TBCNT = g_bmpByteCountEnd;			  // Auto stop count
	UCB0I2CSA = BMP180_I2C_ADDRESS;           // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE |= UCRXIE;						  // Set rx interrupt
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
		TB0CCR0 = 175;					// (175 ticks) * (1 second / 32768 ticks) = 5.3 ms > 4.5 ms required
		break;
	case 1:
		TB0CCR0 = 250;					// (250 ticks) * (1 second / 32768 ticks) = 7.6 ms > 7.5 ms required
		break;
	case 2:
		TB0CCR0 = 450;					// (450 ticks) * (1 second / 32768 ticks) = 13.7 ms > 13.5 ms required
		break;
	case 3:
		TB0CCR0 = 850;					// (850 ticks) * (1 second / 32768 ticks) = 25.9 ms > 25.5 ms required
		break;
	default:
		TB0CCR0 = 850;					// Safe default value
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
	__bis_SR_register(LPM0_bits);		// Enter low power mode and wait for bytes
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
