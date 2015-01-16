// dsLib.h
//
//****************************************************************************************************
// Author:
// 	Nipun Gunawardena
//
// Credits:
//	Inspired by https://github.com/mizraith/RTClib/blob/master/RTC_DS3231.cpp
//
// Requirements:
//	dsLib.c
//
// Description:
// 	Interface with DS3231 to get time
//
// Notes:
//
// Todo:
//	Make more durable, timeouts, testing, etc.
//****************************************************************************************************

#ifndef DSLIB_H_
#define DSLIB_H_



// Includes ------------------------------------------------------------------------------------------
#include <stdint.h>
#include <msp430.h>



// Defines -------------------------------------------------------------------------------------------
#define DS3231_I2C_ADDRESS 0x68
#define DS3231_REG_SECONDS    0x00
#define DS3231_REG_MINUTES    0x01
#define DS3231_REG_HOURS      0x02
#define DS3231_REG_DAYOFWEEK  0x03
#define DS3231_REG_DAYOFMONTH 0x04
#define DS3231_REG_MONTH      0x05
#define DS3231_REG_YEAR       0x06
#define DS3231_REG_A1SECONDS  0x07
#define DS3231_REG_A1MINUTES  0x08
#define DS3231_REG_A1HOURS    0x09
#define DS3231_REG_A1DAYDATE  0x0A
#define DS3231_REG_A2MINUTES  0x0B
#define DS3231_REG_A2HOURS    0x0C
#define DS3231_REG_A2DAYDATE  0x0D
#define DS3231_REG_CONTROL    0x0E
#define DS3231_REG_STATUS_CTL 0x0F
#define DS3231_REG_AGING      0x10
#define DS3231_REG_TEMP_MSB   0x11
#define DS3231_REG_TEMP_LSB   0x12
#define DS3231_TIME_LENGTH	  0x07



// Global --------------------------------------------------------------------------------------------
uint8_t g_getTimeArr[7];	//{seconds, minutes, hours, day, date, month, year}




// Functions -----------------------------------------------------------------------------------------
extern uint8_t bcd2bin (uint8_t val);
extern uint8_t bin2bcd (uint8_t val);
extern void DS3231GetCurrentTime(void);



#endif /* DSLIB_H_ */
