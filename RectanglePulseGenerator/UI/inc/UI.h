#pragma once

#include "stm32f4xx_hal.h"
#include <math.h>

#include "LCD_HD44780.h" // Необходимо для использования типа DisplayedNumber


#define UI_NumberOfFields 18


typedef enum
{
	OK,
	ValueOverflow
} UpdateUI_Status;

struct FieldValue
{
	uint8_t X;
	uint8_t Y;

	DisplayedNumber Number;
	double MaxValue;

	double ValueMultiplier;

	double Additive;

	char* Format;
};

struct MultiDigitNumber
{
	uint8_t X;
	uint8_t Y;

	struct FieldValue* AllFields[7];

	uint8_t AmountOfDigits;

	double Value;
	double MaxValue;
};



void UI_Init(double Vref,
		void (*Function_WriteString) (uint8_t X, uint8_t Y, char* String),
		void (*Function_WriteNumber) (uint8_t X, uint8_t Y,
				DisplayedNumber* Number,
				const char* Format),
		void (*Function_SetCursor) (uint8_t X, uint8_t Y));

UpdateUI_Status UI_DisplayUpdate(uint32_t Value);
void UI_ChangeField(uint8_t FieldNumber);
struct FieldValue* UI_GetField(uint8_t FieldNumber);
