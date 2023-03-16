#pragma once

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

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
void HD44780_WriteString(uint8_t X, uint8_t Y, char* String);
void HD44780_WriteNumber(uint8_t X, uint8_t Y, double Number, const char* Format);
void HD44780_ClearRegion(uint8_t X, uint8_t Y, double Length);
void HD44780_ClearAll(void);
