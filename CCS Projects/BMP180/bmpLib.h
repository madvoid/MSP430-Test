// bmpLib.h
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//	Borrows from Texas Instruments' bmp180 library.
//
// Requirements:
//	bmpLib.c
//
// Description:
// 	Library to interface with BMP180
//
// Notes:
//
// Todo:
//	Make more durable, timeouts, testing, etc.
//****************************************************************************************************

#ifndef BMPLIB_H_
#define BMPLIB_H_



// Includes ------------------------------------------------------------------------------------------
#include <stdint.h>



// Defines -------------------------------------------------------------------------------------------
#define BMP180_I2C_ADDRESS 0x77
#define BMP180_READ_TEMP 0x2E
#define BMP180_READ_PRES_BASE 0x34
#define BMP180_REG_CAL_AC1 0xAA
#define BMP180_REG_CAL_AC2 0xAC
#define BMP180_REG_CAL_AC3 0xAE
#define BMP180_REG_CAL_AC4 0xB0
#define BMP180_REG_CAL_AC5 0xB2
#define BMP180_REG_CAL_AC6 0xB4
#define BMP180_REG_CAL_B1  0xB6
#define BMP180_REG_CAL_B2  0xB8
#define BMP180_REG_CAL_MB  0xBA
#define BMP180_REG_CAL_MC  0xBC
#define BMP180_REG_CAL_MD  0xBE
#define BMP180_REG_CHIPID  0xD0
#define BMP180_REG_VERSION 0xD1
#define BMP180_REG_SOFTRESET 0xE0
#define BMP180_REG_CONTROL 0xF4
#define BMP180_REG_TEMPDATA 0xF6
#define BMP180_REG_PRESSUREDATA 0xF6



// Variables -----------------------------------------------------------------------------------------
volatile uint8_t g_bmpByteCount;	// Bytes received in interrupt vector
volatile uint8_t g_bmpByteCountEnd;	// When to stop receiving
volatile uint8_t g_bmpValBytes[3];	// Bytes received for temperature or pressure
float g_bmpTemp;
float g_pressure;
typedef struct{
	int16_t  ac1;
	int16_t  ac2;
	int16_t  ac3;
	uint16_t ac4;
	uint16_t ac5;
	uint16_t ac6;
	int16_t  b1;
	int16_t  b2;
	int16_t  mb;
	int16_t  mc;
	int16_t  md;
} tBMP180Cals;



// Functions -----------------------------------------------------------------------------------------
extern void BMP180GetCalVals(tBMP180Cals *calInst);
extern void BMP180GetRawTemp(void);
extern void BMP180GetTemp(tBMP180Cals *calInst);
extern void BMP180GetRawPressure(uint8_t oss);
extern void BMP180GetPressure(uint8_t oss, tBMP180Cals *calInst);


#endif /* BMPLIB_H_ */
