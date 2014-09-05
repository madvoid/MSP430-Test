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
	temp = 0.0f;
	hum = 0.0f;
	g_shtRxCount = 0;
}



void SHT21ReadTemperature(void){
	// Set correct SHT21 command
	g_shtTxCommand = SHT21_TEMP_NOBLOCK;

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;                    // Automatic stop generated after UCB0TBCNT is reached
	UCB0TBCNT = 0x01;                       	// number of bytes to be sent
	UCB0I2CSA = SHT21_I2C_ADDRESS;            // Slave address
	UCB0CTL1 &= ~UCSWRST;						// Clear reset
	UCB0IE |= UCTXIE0 | UCRXIE0;

	// Start transfer
	UCB0CTL1 |= UCTXSTT;

	// Wait until stop is transmitted
	while (UCB0CTLW0 & UCTXSTP);            // Ensure stop condition got sent

	// Setup timer for 85ms measurement delay
	TB0CCTL0 = CCIE;                          // TBCCR0 interrupt enabled
	TB0CCR0 = 2800;							// (2800 ticks) * (1 second / 32768 ticks) = 85.4 ms > 85 ms required
	TB0CTL = TBSSEL__ACLK | MC__UP;           // ACLK, up mode

	__bis_SR_register(LPM3_bits);       // Enter LPM3 w/ interrupt

	// Reset receive count
	g_shtRxCount = 0;

	// Configure USCI_B0 for I2C mode - Receive
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0CTLW0 &= ~UCTR;						// Clear write
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 8
	UCB0CTLW1 |= UCASTP_2;                    // Automatic stop generated after UCB0TBCNT is reached
	UCB0TBCNT = 0x02;                       	// number of bytes to be received
	UCB0I2CSA = SHT21_I2C_ADDRESS;            // Slave address
	UCB0CTL1 &= ~UCSWRST;						// Clear reset
	UCB0IE |= UCTXIE0 | UCRXIE0;

	// Start transfer
	UCB0CTL1 |= UCTXSTT;

	// Delay during transfer
	while (UCB0CTLW0 & UCTXSTT);            // Ensure start condition got sent
	__bis_SR_register(LPM0_bits);       	// Enter LPM0 w/ interrupts

	uint16_t tempRaw = ((uint16_t)g_shtRxArr[0] << 8) | (uint16_t)(g_shtRxArr[1]);
	temp = (float)(tempRaw & 0xFFFC);
	temp = -46.85f + 175.72f * (temp/65536.0f);
}



void SHT21ReadHumidity(void){
	// Set correct SHT21 command
	g_shtTxCommand = SHT21_HUM_NOBLOCK;

	// Configure USCI_B0 for I2C mode - Sending
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 4
	UCB0CTLW1 |= UCASTP_2;                    // Automatic stop generated after UCB0TBCNT is reached
	UCB0TBCNT = 0x01;                         // number of bytes to be sent
	UCB0I2CSA = SHT21_I2C_ADDRESS;            // Slave address
	UCB0CTL1 &= ~UCSWRST;					  // Clear reset
	UCB0IE |= UCTXIE0 | UCRXIE0;

	// Start transfer
	UCB0CTL1 |= UCTXSTT;

	// Wait until stop is transmitted
	while (UCB0CTLW0 & UCTXSTP);            // Ensure stop condition got sent

	// Setup timer for 85ms
	TB0CCTL0 = CCIE;                          // TBCCR0 interrupt enabled
	TB0CCR0 = 960;							// (960 ticks) * (1 second / 32768 ticks) = 29.3 ms > 29 ms required
	TB0CTL = TBSSEL__ACLK | MC__UP;           // ACLK, up mode

	__bis_SR_register(LPM3_bits);       // Enter LPM3 w/ interrupt

	// Reset receive count
	g_shtRxCount = 0;

	// Configure USCI_B0 for I2C mode - Receive
	UCB0CTLW0 |= UCSWRST;                     // Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCSSEL__SMCLK;   // I2C mode, Master mode, sync, Sending, SMCLK
	UCB0CTLW0 &= ~UCTR;						// Clear write
	UCB0BRW = 0x0004;                         // baudrate = SMCLK / 8
	UCB0CTLW1 |= UCASTP_2;                    // Automatic stop generated after UCB0TBCNT is reached
	UCB0TBCNT = 0x02;                       	// number of bytes to be received
	UCB0I2CSA = SHT21_I2C_ADDRESS;            // Slave address
	UCB0CTL1 &= ~UCSWRST;						// Clear reset
	UCB0IE |= UCTXIE0 | UCRXIE0;

	// Start transfer
	UCB0CTL1 |= UCTXSTT;

	// Delay during transfer - How to use LPM for that?
	while (UCB0CTLW0 & UCTXSTT);            // Ensure start condition got sent
	__bis_SR_register(LPM0_bits);       // Enter LPM0 w/ interrupts
	//  __delay_cycles(10000);

	uint16_t humRaw = ((uint16_t)g_shtRxArr[0] << 8) | (uint16_t)(g_shtRxArr[1]);
	hum = (float)(humRaw & 0xFFFC);
	hum = -6.0f + 125.0f * (hum/65536.0f);
}
