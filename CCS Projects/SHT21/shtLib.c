// shtLib.c
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//	Borrows from Texas Instruments' sht21 library.
//
// Requirements:
//	shtLib.c
//
// Description:
// 	Library to interface with Sensirion SHT21 to measure humidity without driverlib
//
// Notes:
//
// Todo:
//	Make more durable, timeouts, testing, etc.
//****************************************************************************************************


// Includes ------------------------------------------------------------------------------------------
#include <stdint.h>
#include <msp430.h>
#include "shtLib.h"



// Functions -----------------------------------------------------------------------------------------
void SHT21Init(void){
	g_temp = 0.0f;
	g_hum = 0.0f;
	g_shtRxCount = 0;
}



void SHT21ReadTemperature(void){

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;                    // Automatic stop generated after UCB0TBCNT is reached
	UCB0TBCNT = 0x02;                         // number of bytes to be sent
	UCB0I2CSA = SHT21_I2C_ADDRESS;            // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE &= ~UCTXIE;						  // Ensure tx interrupt off
	UCB0IE |= UCRXIE0;						  // Ensure rx interrupt on

	// Send temperature read command
	UCB0CTL1 |= UCTXSTT;					// Send start
	while(!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt flag
	UCB0TXBUF = SHT21_TEMP_NOBLOCK;			// Send data byte
	while(!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt flag
	UCB0CTLW0 |= UCTXSTP;					// Send stop
	UCB0CTLW0 &= ~UCTR;						// Change to receive
	while (UCB0CTLW0 & UCTXSTP);            // Ensure stop condition got sent

	// Setup timer for 85ms measurement delay
	TB0CCTL0 = CCIE;                          // TBCCR0 interrupt enabled
	TB0CCR0 = 2800;							// (2800 ticks) * (1 second / 32768 ticks) = 85.4 ms > 85 ms required
	TB0CTL = TBSSEL__ACLK | MC__UP;           // ACLK, up mode

	__bis_SR_register(LPM3_bits);       // Enter LPM3 w/ interrupt

	// Reset receive count
	g_shtRxCount = 0;

	// Start transfer
	UCB0CTL1 |= UCTXSTT;

	// Delay during transfer
	while (UCB0CTLW0 & UCTXSTT);            // Ensure start condition got sent
	__bis_SR_register(LPM0_bits);       	// Enter LPM0 w/ interrupts

	uint16_t tempRaw = ((uint16_t)g_shtRxArr[0] << 8) | (uint16_t)(g_shtRxArr[1]);
	g_temp = (float)(tempRaw & 0xFFFC);
	g_temp = -46.85f + 175.72f * (g_temp/65536.0f);
}



void SHT21ReadHumidity(void){

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;                    // Automatic stop generated after UCB0TBCNT is reached
	UCB0TBCNT = 0x02;                         // number of bytes to be sent
	UCB0I2CSA = SHT21_I2C_ADDRESS;            // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE &= ~UCTXIE;						  // Ensure tx interrupt off
	UCB0IE |= UCRXIE0;						  // Ensure rx interrupt on

	// Send humidity read command
	UCB0CTL1 |= UCTXSTT;					// Send start
	while(!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt flag
	UCB0TXBUF = SHT21_HUM_NOBLOCK;			// Send data byte
	while(!(UCB0IFG & UCTXIFG0));			// Wait for tx interrupt flag
	UCB0CTLW0 |= UCTXSTP;					// Send stop
	UCB0CTLW0 &= ~UCTR;						// Change to receive
	while (UCB0CTLW0 & UCTXSTP);            // Ensure stop condition got sent

	// Setup timer for 29ms
	TB0CCTL0 = CCIE;                          // TBCCR0 interrupt enabled
	TB0CCR0 = 960;							// (960 ticks) * (1 second / 32768 ticks) = 29.3 ms > 29 ms required
	TB0CTL = TBSSEL__ACLK | MC__UP;           // ACLK, up mode

	__bis_SR_register(LPM3_bits);       // Enter LPM3 w/ interrupt

	// Reset receive count
	g_shtRxCount = 0;

	// Start transfer
	UCB0CTL1 |= UCTXSTT;

	// Delay during transfer
	while (UCB0CTLW0 & UCTXSTT);            // Ensure start condition got sent
	__bis_SR_register(LPM0_bits);       // Enter LPM0 w/ interrupts

	uint16_t humRaw = ((uint16_t)g_shtRxArr[0] << 8) | (uint16_t)(g_shtRxArr[1]);
	g_hum = (float)(humRaw & 0xFFFC);
	g_hum = -6.0f + 125.0f * (g_hum/65536.0f);
}
