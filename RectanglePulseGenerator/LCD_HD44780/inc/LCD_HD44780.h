#pragma once

#include "stm32f4xx_hal.h"

typedef enum
{
	Disable,
	Rectangle,
	Line

} CursorType;

typedef enum
{
	OneLine,
	TwoLines
} DisplayedLines;

void HD44780_Init_I2C(I2C_HandleTypeDef* Module, uint8_t Address,
		DisplayedLines Lines, CursorType Cursor);
void HD44780_SetCursor(uint8_t x, uint8_t y);
void HD44780_WriteString(char* String);
void HD44780_WriteNumber(int Number);
void HD44780_Clear(void);
