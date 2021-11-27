/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#include "systick.h"
#include "terminal.h"

#include <stdio.h>
#include <string.h>

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DAC_HandleTypeDef hdac;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//static uint32_t tim2_ticks = 0;

static const uint32_t FREQ_STEP_MAX = 4096;
static const uint32_t FREQ_STEP_MIN = 1;

static const uint32_t TICK_STEP_MAX = 32;
static const uint32_t TICK_STEP_MIN = 0;

uint32_t fine_step = 1;
uint32_t tick_step = 0;

const uint32_t base_freq = 12;
uint32_t curr_freq = base_freq;

char str[TERM_1L_SIZE] = {' '};

static trigen *sysgen = NULL;
static term_win *systerm = NULL;

/* misc functions */
uint32_t clamp_ui32(uint32_t val, uint32_t min, uint32_t max)
{
	const uint32_t ret = (val < min) ? min : val;
	return (ret > max) ? max : ret;
}

static const uint32_t UINT32_DIGITS = 10;
static const uint32_t STR2INT_BASE10 = 10;

static char *__uint32_skip_zeroes(char *buf)
{
	char *msd = &buf[UINT32_DIGITS - 1];

	/* find the msd / skip leading zeroes */
	for (uint32_t i = 0; i < UINT32_DIGITS; i++)
		if (buf[i] != '0') {
			msd = &buf[i];
			break;
		}

	return msd;
}

char *uint32_to_str(uint32_t val, char *buf)
{
	/* transform each uint32 digit into char */
	for(uint32_t i = UINT32_DIGITS; i > 0; i--, val /= STR2INT_BASE10)
		buf[i - 1] = "0123456789"[val % STR2INT_BASE10];

	return buf;
}

char *uint32_to_zstr(uint32_t val, char *buf)
{
	/* transform a given value */
	buf = uint32_to_str(val, buf);

	/* find the msd / skip leading zeroes */
	return __uint32_skip_zeroes(buf);
}

/*void tim2_delay(uint32_t us)
{
	uint32_t delay = tim2_ticks + us;
	while(tim2_ticks < delay);
}*/

/* main generator timer callback */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	if (&htim2 == htim && NULL != sysgen)
	{
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, sysgen->count);
		gen_tick(sysgen);
	}
}

/* control key interrupt callbacks */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	HAL_TIM_Base_Stop_IT(&htim2);

	switch(GPIO_Pin)
	{
	case GPIO_PIN_8:
		if (tick_step > TICK_STEP_MIN)	{ tick_step -= 1; }
		break;

	case GPIO_PIN_6:
		if (tick_step < TICK_STEP_MAX)	{ tick_step += 1; }
		break;

	case GPIO_PIN_9:
		if (fine_step > FREQ_STEP_MIN)	{ fine_step /= 2; }
		break;

	case GPIO_PIN_11:
		if (fine_step < FREQ_STEP_MAX)	{ fine_step *= 2; }
		break;

	default:
		break;
	};

	/* reconfigure main generator */
	fine_step = clamp_ui32(fine_step, FREQ_STEP_MIN, FREQ_STEP_MAX);
	tick_step = clamp_ui32(tick_step, TICK_STEP_MIN, TICK_STEP_MAX);

	gen_set_step(sysgen, fine_step);
	gen_set_tick(sysgen, tick_step);

	term_cls(systerm);
	term_putslx(systerm, "Mul:", 0, 0);
	term_putslx(systerm, "Div:", 1, 0);
	term_putslx(systerm, uint32_to_zstr(fine_step, str), 0, 4);
	term_putslx(systerm, uint32_to_zstr(tick_step + 1, str), 1, 4);

	curr_freq = (base_freq * fine_step) / (tick_step + 1);
	term_putslx(systerm, "Freq:", 0, 8);
	term_putslx(systerm, uint32_to_zstr(curr_freq, str), 1, 8);

	HAL_TIM_Base_Start_IT(&htim2);
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* USER CODE BEGIN 1 */
	trigen __gen_main;
	sysgen = &__gen_main;

	term_win __term_main;
	systerm = &__term_main;

	uint32_t start_step = 1;
	uint32_t start_tick = 0;

	fine_step = start_step;
	tick_step = start_tick;

	//if (GEN_EARG == gen_init(sysgen, GEN_12BIT, start_step, start_tick))
	//	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);

	gen_init(sysgen, GEN_12BIT, start_step, start_tick);
	gen_set_step(sysgen, start_step);
	gen_set_tick(sysgen, start_tick);

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
	MX_DAC_Init();
	MX_TIM2_Init();
	/* USER CODE BEGIN 2 */

	term_init(systerm);
	sys_delay(10);

	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
	HAL_TIM_Base_Start_IT(&htim2);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		//HAL_TIM_Base_Stop_IT(&htim2);

		//term_putsl(systerm, uint32_to_str(fine_step, str), 0);
		//term_putsl(systerm, uint32_to_str(tick_step, str), 1);
		term_draw(systerm);

		//HAL_TIM_Base_Start_IT(&htim2);

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
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 160;
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
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void)
{

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 1;
	htim2.Init.CounterMode = TIM_COUNTERMODE_DOWN;
	htim2.Init.Period = 800;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
	{
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_ENABLE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */

	/* USER CODE END TIM2_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE, LCDRS_PIN_Pin|LCDRW_PIN_Pin|LCDEN_PIN_Pin|LCDD4_PIN_Pin
			|LCDD5_PIN_Pin|LCDD6_PIN_Pin|LCDD7_PIN_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : LCDRS_PIN_Pin LCDRW_PIN_Pin LCDEN_PIN_Pin LCDD4_PIN_Pin
                           LCDD5_PIN_Pin LCDD6_PIN_Pin LCDD7_PIN_Pin */
	GPIO_InitStruct.Pin = LCDRS_PIN_Pin|LCDRW_PIN_Pin|LCDEN_PIN_Pin|LCDD4_PIN_Pin
			|LCDD5_PIN_Pin|LCDD6_PIN_Pin|LCDD7_PIN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : LED_BLUE_Pin */
	GPIO_InitStruct.Pin = LED_BLUE_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_BLUE_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : TICK_DEC_Pin TICK_INC_Pin STEP_DEC_Pin STEP_INC_Pin */
	GPIO_InitStruct.Pin = TICK_DEC_Pin|TICK_INC_Pin|STEP_DEC_Pin|STEP_INC_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
