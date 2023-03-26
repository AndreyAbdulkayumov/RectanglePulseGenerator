#include "IncrementalEncoder.h"

// S1 - PE11
// S2 - PE9

uint32_t Value = 0;

uint32_t Current_CNT = 0;


void IncrementalEncoder_Init(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;    // Тактирование GPIOE
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;     // Тактирование таймера №1

	// Включаем альтернативные функции Энкодера у порта E
	GPIOE->MODER |=  GPIO_MODER_MODER9_1   // CH1
			      | GPIO_MODER_MODER11_1;  // CH2

	GPIOE->MODER &= ~(GPIO_MODER_MODER9_0     // CH1
					| GPIO_MODER_MODER11_0);  // CH2

	// Установка высокой скорости контактов порта Е
	GPIOE->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR9_1     // CH1
			          | GPIO_OSPEEDER_OSPEEDR11_1);  // CH2

	GPIOE->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR9_0      // CH1
			        | GPIO_OSPEEDER_OSPEEDR11_0;    // CH2

	// Включаем альтернативные функции Энкодера у порта E
	GPIOE->AFR[1] |= (1 << 4) | (1 << 12);  // Включение AF1 у CH1, CH2

	TIM1->CR1 &= ~(TIM_CR1_CEN);   // Отключение таймера №1
	TIM1->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC2E);   // Отключение модулей захвата на 1 и 2 каналах

	TIM1->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P);   // Установка захвата по фронту импульса

	// Конфигурация канала 1 на ввод.
	// Захват сигнала после фильтра 1 канала
	TIM1->CCMR1 |= TIM_CCMR1_CC1S_0;
	TIM1->CCMR1 &= ~(TIM_CCMR1_CC1S_1);

	// Конфигурация канала 2 на ввод.
	// Захват сигнала после фильтра 2 канала
	TIM1->CCMR1 |= TIM_CCMR1_CC2S_0;
	TIM1->CCMR1 &= ~(TIM_CCMR1_CC2S_1);

	// Включение режима энкодера №3.
	TIM1->SMCR |= TIM_SMCR_SMS_0;
	TIM1->SMCR |= TIM_SMCR_SMS_1;
	TIM1->SMCR &= ~TIM_SMCR_SMS_2;

	// Выбраем вход 2 канала после фильтра для синхронизации счетчика
	TIM1->SMCR &= ~TIM_SMCR_TS_0;
	TIM1->SMCR |= TIM_SMCR_TS_1;
	TIM1->SMCR |= TIM_SMCR_TS_2;

	// Включение цифрового фильтра на 1 канале с выбранным уровнем фильтрации
	TIM1->CCMR1 |= TIM_CCMR1_IC1F_0;
	TIM1->CCMR1 &= ~TIM_CCMR1_IC1F_1;
	TIM1->CCMR1 |= TIM_CCMR1_IC1F_2;
	TIM1->CCMR1 &= ~TIM_CCMR1_IC1F_3;

	// Включение цифрового фильтра на 2 канале с выбранным уровнем фильтрации
	TIM1->CCMR1 |= TIM_CCMR1_IC2F_0;
	TIM1->CCMR1 &= ~TIM_CCMR1_IC2F_1;
	TIM1->CCMR1 |= TIM_CCMR1_IC2F_2;
	TIM1->CCMR1 &= ~TIM_CCMR1_IC2F_3;

	TIM1->CCER |= TIM_CCER_CC1E;  // Включение модуля захвата 1
	TIM1->CCER |= TIM_CCER_CC2E;  // Включение модуля захвата 2
	TIM1->CR1 |= TIM_CR1_CEN;     // Включение таймера 1
}

uint32_t IncrementalEncoder_GetValue(uint32_t MaxValue)
{
	if (Value == 0 && (TIM1->CR1 & TIM_CR1_DIR) != 0)
	{
		TIM1->CNT = 0;
	}

	else
	{
		Value = TIM1->CNT * 0.25;
	}

	if (Value >= MaxValue)
	{
		Value = 0;
		TIM1->CNT = 0;
	}

	return Value;
}

void IncrementalEncoder_GetValue_FromCallback(
		void (*EventHandler) (uint32_t),
		uint32_t MaxValue)
{
	if (Current_CNT == TIM1->CNT)
	{
		return;
	}

	if (Value == 0 && (TIM1->CR1 & TIM_CR1_DIR) != 0)
	{
		TIM1->CNT = 0;
	}

	else
	{
		Value = TIM1->CNT * 0.25;
	}

	if (Value >= MaxValue)
	{
		Value = 0;
		TIM1->CNT = 0;
	}

	EventHandler(Value);

	Current_CNT = TIM1->CNT;
}

void IncrementalEncoder_SetInitialValue(uint32_t InitialValue)
{
	Value = InitialValue;
	TIM1->CNT = Value * 4;
}
