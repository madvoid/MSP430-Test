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

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;
	UCB0TBCNT = 0x02;
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
		while(UCB0CTLW0 & UCTXSTT);			// Wait for start
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
