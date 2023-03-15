#pragma once

#include "stm32f4xx_hal.h"

void IncrementalEncoder_Init(void);

uint32_t IncrementalEncoder_GetValue(uint32_t MaxValue);
void IncrementalEncoder_SetInitialValue(uint32_t InitialValue);
