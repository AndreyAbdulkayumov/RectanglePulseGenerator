/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LCD_HD44780.h"
#include "IncrementalEncoder.h"
#include "PulseControl.h"
#include "UI.h"

#include <Math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
	Field,
	Value
} EncoderChangeMode;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

const double Vref = 2.91;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

EncoderChangeMode CurrentChangeMode;

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DAC_HandleTypeDef hdac;

I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

extern struct MultiDigitNumber Period;
extern struct MultiDigitNumber DutyCycle;
extern struct MultiDigitNumber Amplitude;

extern struct MultiDigitNumber* SelectedVariable;
extern struct FieldValue* SelectedField;


uint8_t FieldNumber = 1;

uint32_t DAC_Code = 0;

double CachedValue = 0;
double CachedFieldValue = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_DAC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// 4095 - максимальное значение для 12 - разрядного ЦАП
uint32_t DAC_GetCodeFrom_Volt(double Value)
{
	if (Value >= Vref)
	{
		return 4095;
	}

	return Value * 4095 / Vref;
}


void Period_Start_Handler(void)
{
	HAL_DAC_SetValue(
			&hdac,
			DAC_CHANNEL_1,
			DAC_ALIGN_12B_R,
			DAC_Code);
}


void DutyCycle_End_Handler(void)
{
	HAL_DAC_SetValue(
			&hdac,
			DAC_CHANNEL_1,
			DAC_ALIGN_12B_R,
			0);
}


void ChangeField(void)
{
	UI_ChangeField(FieldNumber);

	IncrementalEncoder_SetInitialValue(
			SelectedField->Number.Value / SelectedField->Additive);
}


void DisplayUpdate(uint32_t Value)
{
	if (UI_DisplayUpdate(Value) == ValueOverflow)
	{
		IncrementalEncoder_SetInitialValue(
				SelectedField->Number.Value / SelectedField->Additive);
	}

	if (SelectedVariable == &Period)
	{
		PulseControl_SetPeriod_us((uint32_t)Period.Value);
	}

	else if (SelectedVariable == &DutyCycle)
	{
		PulseControl_SetDutyCycle_us((uint32_t)DutyCycle.Value);
	}

	else if (SelectedVariable == &Amplitude)
	{
		DAC_Code = DAC_GetCodeFrom_Volt(Amplitude.Value / 1000);
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_DAC_Init();
  /* USER CODE BEGIN 2 */

  uint8_t ButtonPressed = 0;
  uint8_t SelectedFieldNumber = FieldNumber;

  /*********************************************/
  //
  //	Инициализация дисплея и UI
  //
  /*********************************************/

  // I2C Address: 0x3F or 0x27
  HD44780_Init_I2C(&hi2c1, 0x3F, TwoLines, Line);

  UI_Init(Vref,
		  HD44780_WriteString,
		  HD44780_WriteNumber,
		  HD44780_SetCursor);

  /*********************************************/
  //
  //	Инициализация энкодера
  //
  /*********************************************/

  CurrentChangeMode = Field;

  IncrementalEncoder_Init();
  IncrementalEncoder_SetInitialValue(FieldNumber / 1);

  /*********************************************/
  //
  //	Инициализация ЦАП
  //
  /*********************************************/

  DAC_Code = DAC_GetCodeFrom_Volt(Amplitude.Value / 1000);
  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

  /*********************************************/
  //
  //	Инициализация генератора импульсов
  //
  /*********************************************/

  PulseControl_Init(Period_Start_Handler, DutyCycle_End_Handler);

  PulseControl_Generation_Start(
		  (uint32_t)Period.Value,
		  (uint32_t)DutyCycle.Value);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  if ((GPIOE->IDR & (1 << 13)) == 0 && ButtonPressed == 0)
	  {
		  ButtonPressed = 1;

		  if (CurrentChangeMode == Field)
		  {
			  CurrentChangeMode = Value;

			  ChangeField();
		  }

		  else
		  {
			  CurrentChangeMode = Field;

			  IncrementalEncoder_SetInitialValue(FieldNumber);
		  }
	  }

	  else if (ButtonPressed != 0)
	  {
		  ButtonPressed = 0;
	  }

	  // Изменение поля

	  if (CurrentChangeMode == Field)
	  {
		  SelectedFieldNumber = IncrementalEncoder_GetValue(
				  UI_NumberOfFields + 1);

		  if (SelectedFieldNumber == 0)
		  {
			  SelectedFieldNumber = 1;
			  IncrementalEncoder_SetInitialValue(SelectedFieldNumber);
		  }

		  if (SelectedFieldNumber != FieldNumber)
		  {
			  if (SelectedFieldNumber > UI_NumberOfFields)
			  {
				  SelectedFieldNumber = 1;
			  }

			  FieldNumber = SelectedFieldNumber;

			  HD44780_SetCursor(
					  UI_GetField(FieldNumber)->X,
					  UI_GetField(FieldNumber)->Y);
		  }
	  }

	  // Изменение значения выбранного поля

	  else
	  {
		  CachedValue = SelectedVariable->Value;
		  CachedFieldValue = SelectedField->Number.Value;

		  IncrementalEncoder_GetValue_FromCallback(DisplayUpdate,
				  ((SelectedField->MaxValue + SelectedField->Additive) /
				  SelectedField->Additive));

		  if (SelectedVariable == &Period ||
			  SelectedVariable == &DutyCycle)
		  {
			  if (Period.Value < DutyCycle.Value)
			  {
				  SelectedVariable->Value = CachedValue;
				  SelectedField->Number.Value = CachedFieldValue;

				  DisplayUpdate((uint32_t)SelectedField->Number.Value);

				  IncrementalEncoder_SetInitialValue(
						  SelectedField->Number.Value / SelectedField->Additive);
			  }
		  }
	  }

	  HAL_Delay(100);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */

  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : Encoder_Button_Pin */
  GPIO_InitStruct.Pin = Encoder_Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Encoder_Button_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
