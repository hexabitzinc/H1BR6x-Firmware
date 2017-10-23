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
		MODIFIED by Hexabitz for BitzOS (BOS) V0.1.3 - Copyright (C) 2017 Hexabitz
    All rights reserved
*/

/* Includes ------------------------------------------------------------------*/
#include "BOS.h"


/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/



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
	CreateLog("log1", RATE, 10, FMT_SPACE, FMT_SAMPLE, "Time");
	LogVar("log1", MEMORY_DATA_UINT32, 	0x08000100, "100");
	LogVar("log1", MEMORY_DATA_UINT32, 	0x08000104, "104");
	LogVar("log1", MEMORY_DATA_INT32, 	0x08000108, "108");
	LogVar("log1", MEMORY_DATA_INT32, 	0x0800010C, "10C");
	LogVar("log1", MEMORY_DATA_UINT16, 	0x08000110, "110");
	LogVar("log1", MEMORY_DATA_UINT16, 	0x08000112, "112");
	
	CreateLog("log2_5", EVENT, 10, FMT_COMMA, FMT_SAMPLE, "Sample");
	LogVar("log2_5", MEMORY_DATA_INT16, 	0x20000014, "RAM014");
	LogVar("log2_5", MEMORY_DATA_INT16, 	0x200003D0, "RAM3D0");
	LogVar("log2_5", MEMORY_DATA_UINT8, 	0x200003F4, "RAM3F4");
	LogVar("log2_5", MEMORY_DATA_UINT8, 	0x200003F8, "RAM3F8");
	LogVar("log2_5", MEMORY_DATA_INT8,   0x2000011A, "RAM11A");
	LogVar("log2_5", MEMORY_DATA_INT8, 	0x2000011B, "RAM11B");
	
	CreateLog("log_N1_2", EVENT, 10, FMT_TAB, FMT_SAMPLE, "Sample");
	LogVar("log_N1_2", MEMORY_DATA_UINT32, 	0x0800011C, "11C");
	LogVar("log_N1_2", MEMORY_DATA_UINT32, 	0x08000120, "120");
	LogVar("log_N1_2", MEMORY_DATA_INT32, 	0x08000124, "124");
	LogVar("log_N1_2", MEMORY_DATA_INT32, 	0x08000128, "128");
	LogVar("log_N1_2", MEMORY_DATA_UINT16, 	0x0800012A, "12A");
	LogVar("log_N1_2", MEMORY_DATA_UINT16, 	0x0800012C, "12C");
	
	StartLog("log2_5"); StartLog("log_N1_2"); StartLog("log1");
	
	Delay_s(5);
	
	StopLog("log1"); StopLog("log2_5"); StopLog("log_N1_2");
	
  /* Infinite loop */
  for(;;)
  {

	}
}

/*-----------------------------------------------------------*/

/************************ (C) COPYRIGHT HEXABITZ *****END OF FILE****/
