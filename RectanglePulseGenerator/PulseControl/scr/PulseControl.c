#include "PulseControl.h"

void (*Period_Start_Action) (void) = NULL;
void (*DutyCycle_End_Action) (void) = NULL;

uint32_t Period_PreparedValue;
uint32_t DutyCycle_PreparedValue;

void PulseControl_Init(
		void (*Period_Start_Handler) (void),
		void (*DutyCycle_End_Handler) (void)
		)
{
	Period_Start_Action = Period_Start_Handler;
	DutyCycle_End_Action = DutyCycle_End_Handler;

	/**********************************************/
	//
	// Настройка таймера, регулирующего период.
	//
	/**********************************************/

	Timer_Period_RCC_ON;  // Включение тактирования таймера

	// Период (сек) = 1 / (F_APB1 / PSC / ARR)

	Timer_Period->ARR = 100000;        // Число, до которого досчитает таймер
	Timer_Period->PSC = 84 - 1;          // Делитель частоты таймера (1)

	Timer_Period->CR1 |= TIM_CR1_URS;   // Событие прерывания таймера - переполнение
	Timer_Period->DIER |= TIM_DIER_UIE; // Разрешение прерывания таймера

	NVIC_EnableIRQ(Timer_Period_IRQ);  // Регистрация прерывания в контроллере NVIC


	/**********************************************/
	//
	// Настройка таймера, регулирующего скважность.
	//
	/**********************************************/

	Timer_DutyCycle_RCC_ON;  // Включение тактирования таймера

	Timer_DutyCycle->CR1 &= ~(TIM_CR1_DIR);  // Установка счета вверх

	Timer_DutyCycle->CR1 |= TIM_CR1_OPM;   // Включение режима генерации импульса

	// Период (сек) = 1 / (F_APB1 / PSC / ARR)

	Timer_DutyCycle->ARR = 100;        // Число, до которого досчитает таймер
	Timer_DutyCycle->PSC = 84 - 1;          // Делитель частоты таймера (1)

	Timer_DutyCycle->CR1 |= TIM_CR1_URS;   // Событие прерывания таймера - переполнение
	Timer_DutyCycle->DIER |= TIM_DIER_UIE; // Разрешение прерывания таймера

	NVIC_EnableIRQ(Timer_DutyCycle_IRQ);  // Регистрация прерывания в контроллере NVIC
}

void PulseControl_Generation_Start(uint32_t Period_InitValue, uint32_t DutyCycle_InitValue)
{
	if (Period_Start_Action != NULL)
	{
		Period_Start_Action();
	}

    Period_PreparedValue = Period_InitValue;
    DutyCycle_PreparedValue = DutyCycle_InitValue;

    Timer_Period->ARR = Period_PreparedValue;
    Timer_DutyCycle->ARR = DutyCycle_PreparedValue;

	// Запуск счета таймеров
	Timer_Period->CR1 |= TIM_CR1_CEN;
	Timer_DutyCycle->CR1 |= TIM_CR1_CEN;
}

void PulseControl_Generation_Stop(void)
{
	// Остановка счета таймеров
	Timer_Period->CR1 &= ~(TIM_CR1_CEN);
	Timer_DutyCycle->CR1 &= ~(TIM_CR1_CEN);
}

void PulseControl_SetPeriod_us(uint32_t PeriodValue)
{
	if (PeriodValue >= 1000000)
	{
		return;
	}

	if (PeriodValue == 0)
	{
		Timer_Period->CR1 &= ~(TIM_CR1_CEN);
		Timer_Period->SR &= ~(TIM_SR_UIF);  // Сброс флаг прерывания
		return;
	}

	Period_PreparedValue = PeriodValue;

	if ((Timer_Period->CR1 & TIM_CR1_CEN) == 0)
	{
		Timer_Period->CR1 |= TIM_CR1_CEN;
	}
}

void PulseControl_SetDutyCycle_us(uint32_t DutyCycleValue)
{
	if (DutyCycleValue >= 1000)
	{
		return;
	}

	if (DutyCycleValue == 0)
	{
		Timer_DutyCycle->CR1 &= ~(TIM_CR1_CEN);
		Timer_DutyCycle->SR &= ~(TIM_SR_UIF);  // Сброс флаг прерывания
		return;
	}

	DutyCycle_PreparedValue = DutyCycleValue;

	if ((Timer_DutyCycle->CR1 & TIM_CR1_CEN) == 0)
	{
		Timer_DutyCycle->CR1 |= TIM_CR1_CEN;
	}
}

// Прерывание таймера - период
void TIM2_IRQHandler(void)
{
	Timer_Period->SR &= ~(TIM_SR_UIF);  // Сброс флаг прерывания

	if (Period_Start_Action != NULL)
	{
		Period_Start_Action();
	}

	Timer_Period->ARR = Period_PreparedValue;
	Timer_DutyCycle->CR1 |= TIM_CR1_CEN;  // Запуск таймера
}

// Прерывание таймера - скважность
void TIM5_IRQHandler(void)
{
	Timer_DutyCycle->SR &= ~(TIM_SR_UIF);  // Сброс флаг прерывания

	if (DutyCycle_End_Action != NULL)
	{
		DutyCycle_End_Action();
	}

	Timer_DutyCycle->ARR = DutyCycle_PreparedValue;
}
