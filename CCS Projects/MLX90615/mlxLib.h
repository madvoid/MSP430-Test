// mlxLib.h
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//	Inspired by Adafruit MLX90614 Library
//
// Requirements:
//	mlxLib.c
//
// Description:
// 	Interface with MLX90615
//
// Notes:
//
// Todo:
//	Make more durable, timeouts, testing, etc.
//****************************************************************************************************

#ifndef MLXLIB_H_
#define MLXLIB_H_

// Includes ------------------------------------------------------------------------------------------
#include <stdint.h>



// Defines -------------------------------------------------------------------------------------------
#define MLX90615_I2C_ADDRESS 0x5B
// RAM - Section 8.4.5 of data sheet explains why 0x20 is added to these addresses
// Reserved value addresses not included
#define MX90615_RAWIR 0x025
#define MLX90615_TA 0x26
#define MLX90615_TOBJ 0x27
// EEPROM - Section 8.4.5 of data sheet explains why 0x10 is added to these addresses
// Fatory calibration values and ID number addresses not included
#define MLX90615_TMIN_SA 0x10
#define MLX90615_PWMTRANGE 0x11
#define MLX90615_CONFIG 0x12
#define MLX90615_EMISS 0x13
// Enter sleep command
#define MLX90615_ENTER_SLEEP 0xC6	// 0b1100,0110



// Global --------------------------------------------------------------------------------------------
uint8_t g_mlxValBytes[3];				// Recieved value byte storage
float g_objectTemp;						// Temperature of object in field of view
float g_ambientTemp;					// Ambient temperature



// Functions -----------------------------------------------------------------------------------------
extern void MLX90615GetObjTemp(void);
extern void MLX90615GetAmbTemp(void);
extern void MLX90615Sleep(void);
extern void MLX90615Wake(void);

#endif /* MLXLIB_H_ */
