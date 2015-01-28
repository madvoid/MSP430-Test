// helper.c
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
//	helper.h
//
// Description:
// 	Functions used by main.
//
// Notes:
//
// Todo:
//	Make universal
//****************************************************************************************************



// Includes ------------------------------------------------------------------------------------------
#include "helper.h"



// Functions -----------------------------------------------------------------------------------------
void FloatToPrint(float floatValue, int32_t splitValue[2]){
	int32_t i32IntegerPart;
	int32_t i32FractionPart;

	i32IntegerPart = (int32_t) floatValue;
	i32FractionPart = (int32_t) (floatValue * 1000.0f);
	i32FractionPart = i32FractionPart - (1000 * i32IntegerPart);
	if(i32FractionPart < 0){
		i32FractionPart *= -1;
	}

	splitValue[0] = i32IntegerPart;
	splitValue[1] = i32FractionPart;
}

void i2cSetReset(void){
	// TODO: Fix so delay isn't needed
	__delay_cycles(200);	// 200 us wait for whatever transfer to finish
	UCB0CTLW0 |= UCSWRST;
}
