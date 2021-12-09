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

//#include "systick.h"
#include "terminal.h"

//#include <stdio.h>

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DAC_HandleTypeDef hdac;

TIM_HandleTypeDef htim9;

/* USER CODE BEGIN PV */

/* main constants */
static const uint32_t FREQ_STEP_MAX = 1024;
static const uint32_t FREQ_STEP_MIN = 1;

static const uint32_t TICK_STEP_MAX = 64;
static const uint32_t TICK_STEP_MIN = 0;

static const uint32_t UINT32_DIGITS = 10;
static const uint32_t STR2INT_BASE10 = 10;

//#define __NORMALIZE_AMPLITUDE

/*
 * base_freq = (Ftim_clk) / (tim_period * DAC_MAX )
 * TIM_period = (Ftim_clk) / (Fwanted * DAC_MAX)
 */
static const float base_freq = (80000000.f) / (260.417f * 4096.f);

/* main variables */
static uint32_t fine_step = 1;
static uint32_t tick_step = 0;
static uint32_t main_step = 1;

/* control state machine */
static uint8_t main_step_mode = FREQ_STEP_1;

static float curr_freq = base_freq;

/* general i/o buffer */
static char str[TERM_1L_SIZE] = {' '};

/* system generator and terminal pointers */
static trigen *sysgen = NULL;
static term_win *systerm = NULL;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM9_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* misc functions */

static uint32_t clamp_ui32(uint32_t val, uint32_t min, uint32_t max)
{
	const uint32_t ret = (val < min) ? min : val;
	return (ret > max) ? max : ret;
}

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

static char *uint32_to_str(uint32_t val, char *buf)
{
	/* transform each uint32 digit into char */
	for(uint32_t i = UINT32_DIGITS; i > 0; i--, val /= STR2INT_BASE10)
		buf[i - 1] = "0123456789"[val % STR2INT_BASE10];

	return buf;
}

static char *uint32_to_zstr(uint32_t val, char *buf)
{
	/* transform a given value */
	buf = uint32_to_str(val, buf);

	/* find the msd / skip leading zeroes */
	return __uint32_skip_zeroes(buf);
}

/* print */
static void print_info(void)
{
	/* dislay info */
	term_cls(systerm);

	/* generator parameters */
	term_putsxy(systerm, "Mul:", 0, 0);
	term_putsxy(systerm, "Div:", 0, 1);
	term_putsxy(systerm, uint32_to_zstr(fine_step, str), 4, 0);
	term_putsxy(systerm, uint32_to_zstr(tick_step + 1, str), 4, 1);

	/* generator output */
	term_putsxy(systerm, "Freq:", 9, 0);
	term_putsxy(systerm, uint32_to_zstr(curr_freq, str), 9, 1);

	term_putchxy(systerm, main_step_mode, 15, 0);
}

/* mode state machine */
static enum STEP_CONTROL mode_change(void)
{
	switch(main_step_mode)
	{
	case FREQ_STEP_1:
		main_step = 10;
		main_step_mode = FREQ_STEP_10;
		break;
	case FREQ_STEP_10:
		main_step = 100;
		main_step_mode = FREQ_STEP_100;
		break;
	case FREQ_STEP_100:
		main_step = 2;
		main_step_mode = FREQ_STEP_SH;
		break;

	case FREQ_STEP_SH:
		main_step = 1;
		main_step_mode = FREQ_STEP_1;
		break;

	default:
		main_step_mode = FREQ_STEP_1;
		break;
	};

	return main_step_mode;
}

/* main generator timer callback */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	if (&htim9 == htim && NULL != sysgen)
	{
		//#ifdef __NORMALIZE_AMPLITUDE
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, gen_get_norm_amp(sysgen));
		//#else
		//HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, sysgen->count);
		//#endif
		gen_tick(sysgen);
	}
}

/* control key interrupt callbacks */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	HAL_TIM_Base_Stop_IT(&htim9);

	switch(GPIO_Pin)
	{
	/* divider ticks */
	case GPIO_PIN_8:
		if (tick_step > TICK_STEP_MIN) 	{ tick_step -= 1; }
		break;
	case GPIO_PIN_6:
		if (tick_step < TICK_STEP_MAX)	{ tick_step += 1; }
		break;

		/* main counter ticks */
	case GPIO_PIN_9:
		/* dec (or shift right) frequency */
		if (fine_step > main_step) {
			if (main_step_mode == FREQ_STEP_SH)
				fine_step = fine_step >> 1;
			else
				fine_step -= main_step;
		}
		break;

	case GPIO_PIN_11:
		/* inc (or shift left) frequency */
		if (fine_step < FREQ_STEP_MAX) {
			if (main_step_mode == FREQ_STEP_SH)
				fine_step = fine_step << 1;
			else
				fine_step += main_step;
		}
		break;

	case GPIO_PIN_15:
		/* change freq regulation mode*/
		mode_change();
		break;

	default:
		break;
	};

	/* reconfigure main generator */

	fine_step = clamp_ui32(fine_step, FREQ_STEP_MIN, FREQ_STEP_MAX);
	tick_step = clamp_ui32(tick_step, TICK_STEP_MIN, TICK_STEP_MAX);

	gen_set_step(sysgen, fine_step);
	gen_set_tick(sysgen, tick_step);

	/* calculate approximate current frequency */
	curr_freq = (float)(base_freq * (float)(fine_step)) / (float)(tick_step + 1);

	print_info();

	HAL_TIM_Base_Start_IT(&htim9);
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
	MX_TIM9_Init();
	/* USER CODE BEGIN 2 */

	/* initialize main generator */
	gen_init(sysgen, GEN_12BIT, start_step, start_tick);

	/* initialize terminal -> initialize lcd screen */
	term_init(systerm);
	HAL_Delay(10);

	print_info();

	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
	HAL_TIM_Base_Start_IT(&htim9);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		term_draw(systerm);
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
	RCC_OscInitStruct.PLL.PLLM = 8;
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
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

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
 * @brief TIM9 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM9_Init(void)
{

	/* USER CODE BEGIN TIM9_Init 0 */

	/* USER CODE END TIM9_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};

	/* USER CODE BEGIN TIM9_Init 1 */

	/* USER CODE END TIM9_Init 1 */
	htim9.Instance = TIM9;
	htim9.Init.Prescaler = 0;
	htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim9.Init.Period = 260;
	htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim9.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim9) != HAL_OK)
	{
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim9, &sClockSourceConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM9_Init 2 */

	/* USER CODE END TIM9_Init 2 */

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
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pin : FINE_OR_FAST_Pin */
	GPIO_InitStruct.Pin = FINE_OR_FAST_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(FINE_OR_FAST_GPIO_Port, &GPIO_InitStruct);

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
