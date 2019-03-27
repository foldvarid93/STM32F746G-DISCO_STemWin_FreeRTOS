/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * This notice applies to any and all portions of this file
 * that are not between comment pairs USER CODE BEGIN and
 * USER CODE END. Other portions of this file, whether
 * inserted by the user or by software development tools
 * are owned by their respective copyright owners.
 *
 * Copyright (c) 2019 STMicroelectronics International N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted, provided that the following conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other
 *    contributors to this software may be used to endorse or promote products
 *    derived from this software without specific written permission.
 * 4. This software, including modifications and/or derivative works of this
 *    software, must execute solely and exclusively on microcontroller or
 *    microprocessor devices manufactured by or for STMicroelectronics.
 * 5. Redistribution and use of this software other than as permitted under
 *    this license is void and will automatically terminate your rights under
 *    this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "crc.h"
#include "gpio.h"
#include "adc.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_sdram.h"
#include "stm32746g_discovery_ts.h"
#include "stm32746g_discovery_audio.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
osTimerId TsTimer;
extern ADC_HandleTypeDef hadc3;
extern SAI_HandleTypeDef haudio_in_sai;
int AdcValue;
double Factor;
float factor;
uint16_t dmaBuffer[470];
#define DMA_BUFFER_LENGTH 470
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
extern void GRAPHICS_HW_Init(void);
extern void GRAPHICS_Init(void);
extern void MainTask(void);
extern void NewData(void);
extern void SAIData(void);
extern void ADCData(void);
extern void GRAPHICS_IncTick(void);
extern void TouchUpdate(void);
static void TsTimerCallback(void const *n);//static void TimerCallback(TimerHandle_t xTimer);
//fft tasks
static void GUI_Task ( void const * argument);
static void Signal_Task ( void const * argument);
static void FFT_Task ( void const * argument);
static void ADC_Task(void const * argument);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void TsTimerCallback(void const *n){
	TouchUpdate();
}
void vApplicationTickHook(void) {
	HAL_IncTick();
}
void GUI_Task ( void const * argument){ /* Gui background Task */
	GRAPHICS_Init();
	MainTask();
	osTimerDef(TS_Timer, TsTimerCallback);
	TsTimer = osTimerCreate(osTimer(TS_Timer), osTimerPeriodic, (void *) 0);//touchscreen read in timer 100ms periodic
	osTimerStart(TsTimer, 100);/* Start the TS Timer */
	while (1) {
		NewData();
		//SAIData();
		//ADCData();
		GUI_Exec();
		vTaskDelay(10);
	}
}
void Signal_Task ( void const * argument){/*Collect sample*/
	BSP_AUDIO_IN_InitEx(INPUT_DEVICE_INPUT_LINE_1, DEFAULT_AUDIO_IN_FREQ,85, DEFAULT_AUDIO_IN_CHANNEL_NBR);
	HAL_SAI_Receive_DMA(&haudio_in_sai, (uint8_t*)dmaBuffer,DMA_BUFFER_LENGTH );
	while(1){
		vTaskDelay(100);
	}
}
void FFT_Task ( void const * argument){/*computing fft*/
	while(1){
		vTaskDelay(100);
	}
}
static void ADC_Task(void const * argument) {/*read in analog input(s)*/
	HAL_ADC_Start(&hadc3);
	while (1) {
		if(HAL_ADC_PollForConversion(&hadc3,0)==HAL_OK){
			AdcValue=HAL_ADC_GetValue(&hadc3);
		}
		HAL_ADC_Start(&hadc3);
		vTaskDelay(50);
	}
}
//
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
	/* USER CODE END 1 */

	/* Enable I-Cache---------------------------------------------------------*/
	//SCB_EnableICache();

	/* Enable D-Cache---------------------------------------------------------*/
	//SCB_EnableDCache();

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
	MX_CRC_Init();
	MX_ADC3_Init();
	/* USER CODE BEGIN 2 */
	xTaskCreate ((TaskFunction_t) GUI_Task, "GUI_Task", 1024, NULL, 1, NULL);
	xTaskCreate ((TaskFunction_t) Signal_Task, "Signal_Task", 1024, NULL, 1, NULL);
	xTaskCreate ((TaskFunction_t) FFT_Task, "FFT_Task", 1024, NULL, 1, NULL);
	xTaskCreate ((TaskFunction_t) ADC_Task, "ADC_Task", 1024, NULL, 1, NULL);
	vTaskStartScheduler ();
	/* USER CODE END 2 */

	/* Infinite loop */
}
/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
	/**Configure the main internal regulator output voltage*/
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/**Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 12;
	RCC_OscInitStruct.PLL.PLLN = 192;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 8;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/**Activate the Over-Drive mode*/
	if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
		Error_Handler();
	}
	/**Initializes the CPU, AHB and APB busses clocks*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC| RCC_PERIPHCLK_USART6 | RCC_PERIPHCLK_I2C3;
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
	PeriphClkInitStruct.PLLSAI.PLLSAIR = 5;
	PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
	PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV4;
	PeriphClkInitStruct.PLLSAIDivQ = 1;
	PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_4;
	PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_PCLK2;
	PeriphClkInitStruct.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
		Error_Handler();
	}
}
/* USER CODE BEGIN 4 */
/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM6 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM6) {
		//HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}
void BSP_AUDIO_IN_HalfTransfer_CallBack(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	//xTaskNotifyFromISR(appGlobals.fftTaskId, TASK_EVENT_DMA_HALF_DONE, eSetBits,&xHigherPriorityTaskWoken);
	//xTaskNotifyFromISR(appGlobals.signalTaskId, TASK_EVENT_DMA_HALF_DONE, eSetBits,&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void BSP_AUDIO_IN_TransferComplete_CallBack(void) {
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	//xTaskNotifyFromISR(appGlobals.fftTaskId, TASK_EVENT_DMA_DONE, eSetBits,&xHigherPriorityTaskWoken);
	//xTaskNotifyFromISR(appGlobals.signalTaskId, TASK_EVENT_DMA_DONE, eSetBits,&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
/* USER CODE END 4 */
/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

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
	 tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
