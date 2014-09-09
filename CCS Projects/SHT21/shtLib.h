// shtLib.h
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


#ifndef SHTLIB_H_
#define SHTLIB_H_


// Includes ------------------------------------------------------------------------------------------
#include <stdint.h>


// Defines -------------------------------------------------------------------------------------------
#define SHT21_I2C_ADDRESS  0x40
#define SHT21_TEMP_NOBLOCK 0xF3
#define SHT21_HUM_NOBLOCK  0xF5


// Variables -----------------------------------------------------------------------------------------
volatile unsigned char g_shtRxArr[2];
volatile uint8_t g_shtRxCount;
float g_temp;
float g_hum;


// Functions -----------------------------------------------------------------------------------------
extern void SHT21Init(void);
extern void SHT21ReadTemperature(void);
extern void SHT21ReadHumidity(void);


#endif /* SHTLIB_H_ */
