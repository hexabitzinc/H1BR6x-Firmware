/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
	
/*
		MODIFIED by Hexabitz for BitzOS (BOS) V0.1.1 - Copyright (C) 2017 Hexabitz
    All rights reserved
*/

/* Includes ------------------------------------------------------------------*/
#include "BOS.h"


/* Private variables ---------------------------------------------------------*/
#if _module == 2
float myfloat = -234.15;
uint8_t myuint8 = 200;
int8_t myint8 = -50;
uint16_t myuint16 = 3400;
int16_t myint16 = -5999;
uint32_t myuint32 = 133234;
int32_t myint32 = -500912;
volatile bool bool1 = true;
volatile float float2 = -12.5;
#endif

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);


/* Main functions ------------------------------------------------------------*/

int main(void)
{


  /* MCU Configuration----------------------------------------------------------*/

  /* Reset all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all user peripherals */

	/* Initialize BitzOS */
	BOS_Init();

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  while (1)
  {


  }


}

/*-----------------------------------------------------------*/

/* FrontEndTask function */
void FrontEndTask(void * argument)
{
#if _module == 1
	volatile uint32_t mynum = 0x12ABCDEF;
	volatile float myfloat;
	const float constf = 6543.21;
	Delay_ms(500);
	WriteRemote(2, (uint32_t) &mynum, 0x08016000, FMT_UINT32, 100);
#endif	
	
#if _module == 2	
	AddBOSvar(FMT_BOOL, (uint32_t) &bool1);
	AddBOSvar(FMT_FLOAT, (uint32_t) &float2);
#endif	
	
  /* Infinite loop */
  for(;;)
  {
	#if _module == 1
	
//		mybool = *(bool *)ReadRemoteVar(2, 1, &format1, 100);
//		Delay_ms(10);
//		myfloat = *(float *)ReadRemoteVar(2, 2, &format2, 100);

//		WriteRemote(2, (uint32_t) &mybool, 1, FMT_BOOL, 0);
//		Delay_ms(10);
//		WriteRemote(2, (uint32_t) &constf, 2, FMT_FLOAT, 100);

		//WriteRemote(2, (uint32_t) &mynum, 0x08016000, FMT_UINT32, 100);
		Delay_ms(10);
		WriteRemote(2, (uint32_t) &constf, 0x20000034, FMT_FLOAT, 0);
		
		//myremotevar += 1;
//	myremotevar = ReadRemoteMemory(2, 0x20000028, FMT_UINT32, 100);
//	Delay_ms(10);
//	myremotevar = ReadRemoteMemory(2, 0x2000002c, FMT_INT32, 100);
//	Delay_ms(10);
//	myremotevar = ReadRemoteMemory(2, 0x20000022, FMT_UINT16, 100);
//	Delay_ms(10);
//	myremotevar = ReadRemoteMemory(2, 0x20000024, FMT_INT16, 100);
//	Delay_ms(10);
//	myremotevar = ReadRemoteMemory(2, 0x20000020, FMT_UINT8, 100);
//	Delay_ms(10);
//	myremotevar = ReadRemoteMemory(2, 0x20000021, FMT_INT8, 100);
//	Delay_ms(10);
//	myremotevar = *(float *)ReadRemoteMemory(2, 0x2000001c, FMT_FLOAT, 1000);
//	Delay_ms(10);
		
	Delay_ms(1000);
	
	#endif
	
	#if _module == 2	
	
		++float2;
		
		if (bool1)
			bool1 = false;
		else
			bool1 = true;
		
		Delay_ms(10);
		
	#endif
	}
}


/*-----------------------------------------------------------*/

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
	

	__SYSCFG_CLK_ENABLE();

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
	
}

/*-----------------------------------------------------------*/

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
