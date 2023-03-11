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

int PulseTime_ms = 0;


/*****************************************/
//
// Private functions
//
/*****************************************/

//Функция записи команды в дисплей без проверки флага занятости
void SendCommand_Without_BF(int data)
{
	uint8_t Data_High = data & 0xf0;
	
	//Записываем старшую тетраду
	I2C_Buffer[0] = Data_High | Backlight | E_0  | RW_0 | RS_0;
	I2C_Buffer[1] = Data_High | Backlight | E_1  | RW_0 | RS_0;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 2, HAL_MAX_DELAY);

	HAL_Delay(PulseTime_ms);

	I2C_Buffer[0] = Data_High | Backlight | E_0  | RW_0 | RS_0;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 1, HAL_MAX_DELAY);

	HAL_Delay(PulseTime_ms);
}

// Функция записи команды в дисплей
void SendCommand(int data)
{
	//Для 4-х битного режима
	int Data_High = data & 0xf0;       // Старшая тетрада data
	int Data_Low = (data & 0x0f) << 4;    // Младшая тетрада data
		
	HAL_Delay(PulseTime_ms);  // Вместо проверки флага занятости

	// Записываем старшую тетраду
	I2C_Buffer[0] = Data_High | Backlight | E_0  | RW_0 | RS_0;
	I2C_Buffer[1] = Data_High | Backlight | E_1  | RW_0 | RS_0;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 2, HAL_MAX_DELAY);

	HAL_Delay(PulseTime_ms);

	I2C_Buffer[0] = Data_High | Backlight | E_0  | RW_0 | RS_0;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 1, HAL_MAX_DELAY);

	HAL_Delay(PulseTime_ms);

	// Записываем младшую тетраду
	I2C_Buffer[0] = Data_Low | Backlight | E_0  | RW_0 | RS_0;
	I2C_Buffer[1] = Data_Low | Backlight | E_1  | RW_0 | RS_0;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 2, HAL_MAX_DELAY);

	HAL_Delay(PulseTime_ms);

	I2C_Buffer[0] = Data_Low | Backlight | E_0  | RW_0 | RS_0;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 1, HAL_MAX_DELAY);

	HAL_Delay(PulseTime_ms);
}


// Функция записи команды в дисплей
void SendByte(int byte)
{
	// Для 4-х битного режима
	int Byte_High = byte & 0xf0;          // Старшая тетрада data
	int Byte_Low = (byte & 0x0f) << 4;    // Младшая тетрада data

	HAL_Delay(10);    // Вместо проверки флага занятости
	
	// Записываем старшую тетраду
	I2C_Buffer[0] = Byte_High | Backlight | E_0  | RW_0 | RS_1;
	I2C_Buffer[1] = Byte_High | Backlight | E_1  | RW_0 | RS_1;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 2, HAL_MAX_DELAY);

	HAL_Delay(PulseTime_ms);

	I2C_Buffer[0] = Byte_High | Backlight | E_0  | RW_0 | RS_1;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 1, HAL_MAX_DELAY);

	HAL_Delay(PulseTime_ms);

	// Записываем младшую тетраду
	I2C_Buffer[0] = Byte_Low | Backlight | E_0  | RW_0 | RS_1;
	I2C_Buffer[1] = Byte_Low | Backlight | E_1  | RW_0 | RS_1;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 2, HAL_MAX_DELAY);

	HAL_Delay(PulseTime_ms);

	I2C_Buffer[0] = Byte_Low | Backlight | E_0  | RW_0 | RS_1;
	HAL_I2C_Master_Transmit(I2C_Module, I2C_Address, I2C_Buffer, 1, HAL_MAX_DELAY);

	HAL_Delay(PulseTime_ms);
}


/*****************************************/
//
// Public functions
//
/*****************************************/

// Инициализация дисплея по I2C
void HD44780_Init_I2C(I2C_HandleTypeDef* Module, uint8_t Address,
		DisplayedLines Lines, CursorType Cursor)
{
	I2C_Module = Module;
	I2C_Address = Address << 1; // Установка адреса дисплея

	// Первоначальные команды для контроллера дисплея
	HAL_Delay(500);
	SendCommand(0x30);
	HAL_Delay(10);
	SendCommand(0x30);
	HAL_Delay(10);
	SendCommand(0x30);
	HAL_Delay(1);
	SendCommand_Without_BF(32);

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

	SendCommand(0x01);     // Очистка дисплея
}

// Установки курсора в заданное положение
void HD44780_SetCursor(uint8_t x, uint8_t y)
{
	if (y > 0 && y <= 2)
	{
		int CursorPosition = 0x80 + 0x40 * (y - 1) + (x - 1);

		SendCommand(CursorPosition);
	}
}

// Вывод строки
void HD44780_WriteString(char* String)
{
	while(*String != 0)
	{
		SendByte(*String++);
	}
}

// Вывод числа на дисплей
void HD44780_WriteNumber(int Number)
{
	int ed, dec, sot;
	
	// Вывод числа num на дисплей
	// Разбитие числа на разряды
	ed = Number % 10 + 48;
	dec = ((Number % 100) / 10) + 48;
	sot = ((Number % 1000) / 100) + 48;

	// Вывод числа
	if (Number < 10)
	{
		SendByte(CharCode_Empty);
		SendByte(CharCode_Empty);

		if (Number < 0)
		{
			SendByte(CharCode_Minus);
		}

		else
		{
			SendByte(CharCode_Empty);
		}

		SendByte(ed);   // Вывод единиц
	}

	else if (Number >= 10 && Number < 100)
	{
		SendByte(CharCode_Empty);

		if (Number < 0)
		{
			SendByte(CharCode_Minus);
		}

		else
		{
			SendByte(CharCode_Empty);
		}

		SendByte(dec);  // Вывод десятков
		SendByte(ed);   // Вывод единиц
	}

	else if (Number >= 100 && Number < 1000)
	{
		if (Number < 0)
		{
			SendByte(CharCode_Minus);
		}

		else
		{
			SendByte(CharCode_Empty);
		}

		SendByte(sot);  // Вывод сотен
		SendByte(dec);  // Вывод десятков
		SendByte(ed);   // Вывод единиц
	}

	else
	{
		HD44780_WriteString("Very much");
	}
}

// Очистка экрана дисплея
void HD44780_Clear()
{
	SendCommand(0x01); // Команда очистки дисплея
}
