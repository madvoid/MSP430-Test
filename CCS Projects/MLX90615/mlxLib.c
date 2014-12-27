// mlxLib.c
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//	Inspired by Adafruit MLX90614 Library
//
// Requirements:
//	mlxLib.h
//
// Description:
// 	Interface with MLX90615
//
// Notes:
//
// Todo:
//	Make more durable, timeouts, testing, etc.
//****************************************************************************************************



// Includes ------------------------------------------------------------------------------------------
#include <stdint.h>
#include <msp430.h>
#include "mlxLib.h"



// Functions -----------------------------------------------------------------------------------------
extern void MLX90615GetObjTemp(void){
	// Configure USCI_B0 for I2 mode - Sending
	UCB0CTLW0 |= UCSWRST;		// Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK; // I2C mode, master, sync, sending, SMCLK
	UCB0BRW = 0xA;				// Baudrate = SMCLK / 10;
	UCB0CTLW1 |= UCASTP_2;		// Auto stop
	UCB0TBCNT = 0x03;			// Auto stop after 3 bytes
	UCB0I2CSA = MLX90615_I2C_ADDRESS;	// I2C Address
	UCB0CTL1 &= ~UCSWRST;		// Clear reset
	UCB0IE &= ~UCRXIE;			// Ensure Interrupts off
	UCB0IE &= ~UCTXIE;

	// Send object temperature read command
	UCB0CTL1 |= UCTXSTT;			// Send start
	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
	UCB0TXBUF = MLX90615_TOBJ;		// Send object temperature command
	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
	UCB0CTLW0 &= ~UCTR;				// Change to receive
	UCB0CTLW0 |= UCTXSTT;			// Send restart
	while(UCB0CTLW0 & UCTXSTT);		// Wait for restart

	// Receive Bytes
	while(!(UCB0IFG & UCRXIFG0));	// Wait for RX interrupt flag
	g_mlxValBytes[0] = UCB0RXBUF;	// 0th byte
	while(!(UCB0IFG & UCRXIFG0));	// Wait for RX interrupt flag
	g_mlxValBytes[1] = UCB0RXBUF;	// 1st byte
	while(!(UCB0IFG & UCRXIFG0));	// Wait for RX interrupt flag
	g_mlxValBytes[2] = UCB0RXBUF;	// 2nd byte
	while(UCB0CTLW0 & UCTXSTP);		// Wait for stop

	// Calculate temperature
	g_objectTemp = 0.0;
	uint16_t tempVals = ( ((uint16_t) g_mlxValBytes[1]) << 8 ) | ( (uint16_t) g_mlxValBytes[0] );
	g_objectTemp = ((float) tempVals) * 0.02 - 273.15;
}

extern void MLX90615GetAmbTemp(void){
	// Configure USCI_B0 for I2 mode - Sending
	UCB0CTLW0 |= UCSWRST;		// Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK; // I2C mode, master, sync, sending, SMCLK
	UCB0BRW = 0xA;				// Baudrate = SMCLK / 10;
	UCB0CTLW1 |= UCASTP_2;		// Auto stop
	UCB0TBCNT = 0x03;			// Auto stop after 3 bytes
	UCB0I2CSA = MLX90615_I2C_ADDRESS;	// I2C Address
	UCB0CTL1 &= ~UCSWRST;		// Clear reset
	UCB0IE &= ~UCRXIE;			// Ensure TX/RX interrupt not set
	UCB0IE &= ~UCTXIE;

	// Send ambient temperature read command
	UCB0CTL1 |= UCTXSTT;			// Send start
	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
	UCB0TXBUF = MLX90615_TA;		// Send object temperature command
	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
	UCB0CTLW0 &= ~UCTR;				// Change to receive
	UCB0CTLW0 |= UCTXSTT;			// Send restart
	while(UCB0CTLW0 & UCTXSTT);		// Wait for restart

	// Receive Bytes
	while(!(UCB0IFG & UCRXIFG0));	// Wait for RX interrupt flag
	g_mlxValBytes[0] = UCB0RXBUF;	// 0th byte
	while(!(UCB0IFG & UCRXIFG0));	// Wait for RX interrupt flag
	g_mlxValBytes[1] = UCB0RXBUF;	// 1st byte
	while(!(UCB0IFG & UCRXIFG0));	// Wait for RX interrupt flag
	g_mlxValBytes[2] = UCB0RXBUF;	// 2nd byte
	while(UCB0CTLW0 & UCTXSTP);		// Wait for stop

	g_ambientTemp = 0.0;
	uint16_t tempVals = 0;
	tempVals = ( ((uint16_t) g_mlxValBytes[1]) << 8 ) | ( (uint16_t) g_mlxValBytes[0] );
	g_ambientTemp = ((float) tempVals) * 0.02 - 273.15;
}


extern void MLX90615Sleep(void){
	// Configure USCI_B0 for I2 mode - Sending
	UCB0CTLW0 |= UCSWRST;		// Software reset enabled
	UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC | UCTR | UCSSEL__SMCLK; // I2C mode, master, sync, sending, SMCLK
	UCB0BRW = 0xA;				// Baudrate = SMCLK / 10;
	UCB0CTLW1 |= UCASTP_2;		// Auto stop
	UCB0TBCNT = 0x02;			// Auto stop after 3 bytes
	UCB0I2CSA = MLX90615_I2C_ADDRESS;	// I2C Address
	UCB0CTL1 &= ~UCSWRST;		// Clear reset
	UCB0IE &= ~UCRXIE;			// Ensure TX/RX interrupt not set
	UCB0IE &= ~UCTXIE;

	UCB0CTL1 |= UCTXSTT;			// Send start
	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
	UCB0TXBUF = MLX90615_ENTER_SLEEP;		// Send sleep command
	while(!(UCB0IFG & UCTXIFG0));	// Wait for TX interrupt flag
	UCB0TXBUF = 0x6D;				// Send PEC, see Fig. 11 in 8.4.8.1 of data sheet
	while(UCB0CTLW0 & UCTXSTP);		// Wait for stop
}
