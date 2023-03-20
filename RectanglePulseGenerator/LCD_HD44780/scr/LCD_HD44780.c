#include "LCD_HD44780.h"

#define CharCode_Empty            0x20
#define CharCode_Minus            0x2D

#define CharCode_Cursor_Disable   0x0C
#define CharCode_Cursor_Rectangle 0x0D
#define CharCode_Cursor_Line      0x0E

#define CharCode_NumberOfLine_1   0x20
#define CharCode_NumberOfLine_2   0x28

#define On 1
#define Off 0
#define Backlight_on 8
#define Backlight_off 0
#define RS_0 0
#define RS_1 1
#define RW_0 0
#define RW_1 2
#define E_0 0
#define E_1 4

I2C_HandleTypeDef* I2C_Module;

uint8_t Backlight = Backlight_on;

uint8_t I2C_Address;

uint8_t I2C_Buffer[2] = { 0, 0 };

/*****************************************/
//
// При использовании I2C автоматически соблюдаются
// минимальные временные показатели для импульсов управления.
//
// Проверка флага занятости дисплея не проводится.
// Вместо нее поставлены задержки перед записью команды или байта.
//
// Для управления дисплеем используется 4-х битный режим.
//
/*****************************************/


/*****************************************/
//
// Private functions
//
/*****************************************/

// Функция записи команды в дисплей без проверки флага занятости
void SendCommand_Without_BF(int data)
{
	uint8_t Data_High = data & 0xf0;
	
	//Записываем старшую тетраду
	I2C_Buffer[0] = Data_High | Backlight | E_0  | RW_0 | RS_0;
	I2C_Buffer[1] = Data_High | Backlight | E_1  | RW_0 | RS_0;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 2, HAL_MAX_DELAY);

	I2C_Buffer[0] = Data_High | Backlight | E_0  | RW_0 | RS_0;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 1, HAL_MAX_DELAY);
}


void SendCommand(int data)
{
	int Data_High = data & 0xf0;       // Старшая тетрада data
	int Data_Low = (data & 0x0f) << 4;    // Младшая тетрада data
		
	HAL_Delay(1);  // Вместо проверки флага занятости

	// Записываем старшую тетраду
	I2C_Buffer[0] = Data_High | Backlight | E_0  | RW_0 | RS_0;
	I2C_Buffer[1] = Data_High | Backlight | E_1  | RW_0 | RS_0;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 2, HAL_MAX_DELAY);

	I2C_Buffer[0] = Data_High | Backlight | E_0  | RW_0 | RS_0;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 1, HAL_MAX_DELAY);

	// Записываем младшую тетраду
	I2C_Buffer[0] = Data_Low | Backlight | E_0  | RW_0 | RS_0;
	I2C_Buffer[1] = Data_Low | Backlight | E_1  | RW_0 | RS_0;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 2, HAL_MAX_DELAY);

	I2C_Buffer[0] = Data_Low | Backlight | E_0  | RW_0 | RS_0;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 1, HAL_MAX_DELAY);
}


void SendByte(int byte)
{
	int Byte_High = byte & 0xf0;          // Старшая тетрада data
	int Byte_Low = (byte & 0x0f) << 4;    // Младшая тетрада data

	HAL_Delay(5);    // Вместо проверки флага занятости
	
	// Записываем старшую тетраду
	I2C_Buffer[0] = Byte_High | Backlight | E_0  | RW_0 | RS_1;
	I2C_Buffer[1] = Byte_High | Backlight | E_1  | RW_0 | RS_1;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 2, HAL_MAX_DELAY);

	I2C_Buffer[0] = Byte_High | Backlight | E_0  | RW_0 | RS_1;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 1, HAL_MAX_DELAY);

	// Записываем младшую тетраду
	I2C_Buffer[0] = Byte_Low | Backlight | E_0  | RW_0 | RS_1;
	I2C_Buffer[1] = Byte_Low | Backlight | E_1  | RW_0 | RS_1;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 2, HAL_MAX_DELAY);

	I2C_Buffer[0] = Byte_Low | Backlight | E_0  | RW_0 | RS_1;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 1, HAL_MAX_DELAY);
}


/*****************************************/
//
// Public functions
//
/*****************************************/


void HD44780_Init_I2C(I2C_HandleTypeDef* Module, uint8_t Address,
		DisplayedLines Lines, CursorType Cursor)
{
	I2C_Module = Module;
	I2C_Address = Address << 1; // Установка адреса дисплея

	// Первоначальные команды для контроллера дисплея
	HAL_Delay(500);
	SendCommand(0x38);
	HAL_Delay(10);
	SendCommand(0x38);
	HAL_Delay(10);
	SendCommand(0x38);
	HAL_Delay(1);
	SendCommand_Without_BF(0x38);

	SendCommand(0x02);

	// Первоначальная настройка

	// Установка колличества строк
	switch (Lines)
	{
	case OneLine:
		SendCommand(CharCode_NumberOfLine_1);
		break;

	case TwoLines:
		SendCommand(CharCode_NumberOfLine_2);
		break;

	default:
		SendCommand(CharCode_NumberOfLine_1);
		break;
	}

	HAL_Delay(1);

	// Настройка курсора
	switch (Cursor)
	{
	case Disable:
		SendCommand(CharCode_Cursor_Disable);
		break;

	case Rectangle:
		SendCommand(CharCode_Cursor_Rectangle);
		break;

	case Line:
		SendCommand(CharCode_Cursor_Line);
		break;

	default:
		SendCommand(CharCode_Cursor_Disable);
		break;
	}

	HD44780_ClearAll();
}


void HD44780_SetCursor(uint8_t x, uint8_t y)
{
	int CursorPosition = 0;

	if (y == 1 || y == 2)
	{
		CursorPosition = 0x80 + 0x40 * (y - 1) + (x - 1);
	}

	else if (y == 3 || y == 4)
	{
		CursorPosition = 0x94 + 0x40 * (y - 3) + (x - 1);
	}

	SendCommand(CursorPosition);
}


void HD44780_WriteString(uint8_t X, uint8_t Y, char* String)
{
	HD44780_SetCursor(X, Y);

	while(*String != 0)
	{
		SendByte(*String++);
	}
}


void HD44780_WriteNumber(uint8_t X, uint8_t Y,
		DisplayedNumber* Number,
		const char* Format)
{
	if (Number->Value == Number->DisplayedValue)
	{
		return;
	}

	char str[10];

	sprintf(str, Format, Number->Value);

	HD44780_ClearRegion(X, Y, Number->DisplayedValueLength);

	HD44780_WriteString(X, Y, str);

	Number->DisplayedValueLength = strlen(str);
	Number->DisplayedValue = Number->Value;
}


void HD44780_ClearRegion(uint8_t X, uint8_t Y, double Length)
{
	HD44780_SetCursor(X, Y);

	for (uint8_t i = 0; i < Length; i++)
	{
		SendByte(CharCode_Empty);
	}
}


void HD44780_ClearAll()
{
	SendCommand(0x01); // Команда очистки дисплея
}
