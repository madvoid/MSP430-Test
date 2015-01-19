// helper.h
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//
//
// Requirements:
//	LEMS main.c
//	helper.c
//
// Description:
// 	Functions used by main.
//
// Notes:
//
// Todo:
//	Make universal
//****************************************************************************************************

#ifndef HELPER_H_
#define HELPER_H_



// Includes ------------------------------------------------------------------------------------------
#include <stdint.h>
#include <msp430.h>



// Functions -----------------------------------------------------------------------------------------
extern void FloatToPrint(float floatValue, int32_t splitValue[2]);
extern void i2cSetReset(void);



#endif /* HELPER_H_ */
