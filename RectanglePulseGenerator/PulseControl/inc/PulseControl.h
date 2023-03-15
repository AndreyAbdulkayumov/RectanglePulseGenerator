#pragma once

#include "stm32f4xx.h"

#define Timer_Period    	     TIM2
#define Timer_Period_IRQ         TIM2_IRQn
#define Timer_Period_RCC_ON	     RCC->APB1ENR |= RCC_APB1ENR_TIM2EN

#define Timer_DutyCycle 	     TIM5
#define Timer_DutyCycle_IRQ   	 TIM5_IRQn
#define Timer_DutyCycle_RCC_ON   RCC->APB1ENR |= RCC_APB1ENR_TIM5EN

void PulseControl_Init(
		void (*Period_Start_Handler) (void),
		void (*DutyCycle_End_Handler) (void)
		);
void PulseControl_Generation_Start(void);
void PulseControl_Generation_Stop(void);
void PulseControl_SetPeriod_us(uint32_t PeriodValue);
void PulseControl_SetDutyCycle_us(uint32_t DutyCycleValue);
