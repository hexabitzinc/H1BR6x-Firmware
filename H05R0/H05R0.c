/*
    BitzOS (BOS) V0.1.0 - Copyright (C) 2017 Hexabitz
    All rights reserved

    File Name     : H05R0.c
    Description   : Source code for module H05R0.
										SPI-based uSD driver with Fatfs.  
		
		Required MCU resources : 
		
			>> USARTs 1,2,3,4,5 for module ports.
			>> DMA1 Ch5, DMA1 Ch6, DMA2 Ch2 for port-to-memory messaging.
			>> DMA1 Ch1, DMA1 Ch3, DMA2 Ch3 for port-to-port streaming.
			>> SPI1 for uSD
			>> PB0 for uSD detect
			
*/
	
/* Includes ------------------------------------------------------------------*/
#include "BOS.h"



/* Define UART variables */
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;


/* Private variables ---------------------------------------------------------*/
log_t logs[MAX_LOGS];
logVar_t logVars[MAX_LOG_VARS];
FATFS SDFatFs;  /* File system object for SD card logical drive */
char SDPath[4]; /* SD card logical drive path */
FIL MyFile;     /* File object */
uint32_t byteswritten, bytesread;                     /* File write/read counts */
uint8_t buffer[100];
char logHeaderText1[] = "Datalog created by BOS V%d.%d.%d on %s\n\r";
char logHeaderText2[] = "Log type: Rate @ %.2f Hz\n\n\r";
char logHeaderText3[] = "Log type: Event @ %d events max\n\n\r";

TaskHandle_t LogTaskHandle = NULL;

/* Private function prototypes -----------------------------------------------*/	
void LogTask(void * argument);


/* Create CLI commands --------------------------------------------------------*/




/* -----------------------------------------------------------------------
	|												 Private Functions	 														|
   ----------------------------------------------------------------------- 
*/

/* --- H05R0 module initialization. 
*/
void Module_Init(void)
{
	SD_CardInfo CardInfo;
	
	/* Array ports */
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_USART4_UART_Init();
  MX_USART5_UART_Init();
	
	/* uSD - GPIO and SPI */
	BSP_SD_Init();
	//SD_IO_Init();
	
	/* Get the uSD size and info */
	BSP_SD_GetCardInfo(&CardInfo);
	
	/* Create the logging task */
	xTaskCreate(LogTask, (const char *) "LogTask", 2500, NULL, osPriorityNormal, &LogTaskHandle);
	
  
}
/*-----------------------------------------------------------*/

/* --- H05R0 message processing task. 
*/
Module_Status Module_MessagingTask(uint16_t code, uint8_t port, uint8_t src, uint8_t dst)
{
	Module_Status result = H05R0_OK;
	
	switch (code)
	{

		default:
			result = H05R0_ERR_UnknownMessage;
			break;
	}			

	return result;	
}

/*-----------------------------------------------------------*/

/* --- Get the port for a given UART. 
*/
uint8_t GetPort(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART4)
			return P1;
	else if (huart->Instance == USART2)
			return P2;
	else if (huart->Instance == USART3)
			return P3;
	else if (huart->Instance == USART1)
			return P4;
	else if (huart->Instance == USART5)
			return P5;
	
	return 0;
}

/*-----------------------------------------------------------*/

/* --- Logging task. 
*/
void LogTask(void * argument)
{
  /* Link the micro SD disk I/O driver */
  if(FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
  {
    /* Mount the default drive */
    if(f_mount(&SDFatFs, SDPath, 1) != FR_OK)
    {
      /* FatFs Initialization Error */
      //Error_Handler();
    }
    else
    {
			
			/* Infinite loop */
			for(;;)
			{
				
			}			
		
		}
	}
	
	/* Unmount the drive */
	f_mount(0, SDPath, 0); 

	
}


/* -----------------------------------------------------------------------
	|																APIs	 																 	|
   ----------------------------------------------------------------------- 
*/

/* --- Create a new data log. 
				logName: Log file name. Max 10 char. If log already exists, a new column will be added.
				type: RATE or EVENT
				lengthrate: data rate in Hz (max 1000 Hz) or number of events captured.
				columnFormat: FMT_SPACE, FMT_TAB, FMT_COMMA
				indexColumn: FMT_SAMPLE, FMT_TIME
				indexColumnLabel: Index Column label text. Max 30 char.
*/
Module_Status CreateLog(const char* logName, uint8_t type, float lengthrate, columnFormat_t columnFormat, indexColumn_t indexColumn,\
	const char* indexColumnLabel)
{
	FRESULT res; 
	uint8_t i=0;
	
	/* Check if log already exists */
	for( i=0 ; i<MAX_LOGS ; i++)
  {
		if(!strcmp(logs[i].name, logName))
		{
			return H05R0_ERR_LogNameExists;
		}
  }
	
	/* Check parameters are correct */
	if ( (type != RATE && type != EVENT)	||
			 (columnFormat != FMT_SPACE && columnFormat != FMT_SPACE && columnFormat != FMT_COMMA)	||
			 (indexColumn != FMT_NONE && indexColumn != FMT_SAMPLE && indexColumn != FMT_TIME)	||
			 (lengthrate > 1000) )
		return H05R0_ERR_WrongParams;				
					
	/* Name does not exist. Fill first empty location */
	for( i=0 ; i<MAX_LOGS ; i++)
  {
		if(logs[i].name == 0)
		{
			/* Check if file exists on disk */
			res = f_open(&MyFile, logName, FA_CREATE_NEW);
			if(res == FR_EXIST)
				return H05R0_ERR_LogNameExists;
			else if (res != FR_OK)
				return H05R0_ERR_SD;	
			
			/* Log created successfuly */
			logs[i].name = logName;
			logs[i].type = type;
			logs[i].length = lengthrate;
			logs[i].columnFormat = columnFormat;
			logs[i].indexColumn = indexColumn;
			logs[i].indexColumnLabel = indexColumnLabel;
			
			/* Write log header */
			sprintf( ( char * ) buffer, logHeaderText1, _firmMajor, _firmMinor, _firmPatch, modulePNstring[4]);
			res = f_write(&MyFile, logHeaderText1, sizeof(logHeaderText1), (void *)&byteswritten);
			if(type == RATE) {
				sprintf( ( char * ) buffer, logHeaderText2, lengthrate);
				res = f_write(&MyFile, logHeaderText2, sizeof(logHeaderText2), (void *)&byteswritten);				
			} else if (type == EVENT) {
				sprintf( ( char * ) buffer, logHeaderText3, lengthrate);
				res = f_write(&MyFile, logHeaderText3, sizeof(logHeaderText3), (void *)&byteswritten);	
			}
			
			return H05R0_OK;
		}
  }	

	return H05R0_ERR_MaxLogs;	
}

/* --- Save data from a port to an existing data log. 
				logName: Log file name.
				source: data source. Port (P1-Px).
				columnLabel: Column label text. Max 30 char.
*/
Module_Status LogPort(const char* logName, uint8_t source, const char* ColumnLabel)
{
	uint8_t i = 0, j = 0;

	/* Search for this log to make sure it exists */
	for( j=0 ; j<MAX_LOGS ; j++)
	{
		if (!strcmp(logs[j].name, logName))
		{
			/* Make sure there's enough space for this log variable */
			for( i=0 ; i<MAX_LOG_VARS ; i++)
			{
				if(logVars[i].port == 0 && logVars[i].memory == 0)
				{
					logVars[i].port = source;
					logVars[i].logIndex = j;
					logVars[i].varLabel = ColumnLabel;
					
					return H05R0_OK;
				}
			}
			return H05R0_ERR_MaxLogVars;
		}
	}		

	return H05R0_ERR_LogDoesNotExist;	
}

/* --- Save data from a port button to an existing data log. 
				logName: Log file name.
				source: data source. Button (B1-Bx).
				columnLabel: Column label text. Max 30 char.
*/
Module_Status LogButton(const char* logName, uint8_t source, const char* ColumnLabel)
{
	uint8_t i = 0, j = 0;

	/* Search for this log to make sure it exists */
	for( j=0 ; j<MAX_LOGS ; j++)
	{
		if (!strcmp(logs[j].name, logName))
		{
			/* Make sure there's enough space for this log variable */
			for( i=0 ; i<MAX_LOG_VARS ; i++)
			{
				if(logVars[i].port == 0 && logVars[i].memory == 0)
				{
					logVars[i].port = source<<4;			// Distinguish buttons from ports
					logVars[i].logIndex = j;
					logVars[i].varLabel = ColumnLabel;
					
					return H05R0_OK;
				}
			}
			return H05R0_ERR_MaxLogVars;
		}
	}		

	return H05R0_ERR_LogDoesNotExist;	
}

/* --- Save data from a memory location to an existing data log. 
				logName: Log file name.
				source: memory location.
				columnLabel: Column label text. Max 30 char.
*/
Module_Status LogMemory(const char* logName, uint32_t memory, const char* ColumnLabel)
{
	uint8_t i = 0, j = 0;

	/* Search for this log to make sure it exists */
	for( j=0 ; j<MAX_LOGS ; j++)
	{
		if (!strcmp(logs[j].name, logName))
		{
			/* Make sure there's enough space for this log variable */
			for( i=0 ; i<MAX_LOG_VARS ; i++)
			{
				if(logVars[i].port == 0 && logVars[i].memory == 0)
				{
					logVars[i].memory = memory;
					logVars[i].logIndex = j;
					logVars[i].varLabel = ColumnLabel;
					
					return H05R0_OK;
				}
			}
			return H05R0_ERR_MaxLogVars;
		}
	}		

	return H05R0_ERR_LogDoesNotExist;		
}

/* --- Start an existing data log. 
				logName: Log file name.
*/
Module_Status StartLog(const char* logName)
{
	Module_Status result = H05R0_OK;


	return result;	
}

/* --- Stop a running data log. 
				logName: Log file name.
*/
Module_Status StopLog(const char* logName)
{
	Module_Status result = H05R0_OK;


	return result;	
}

/* --- Pause a running data log. 
				logName: Log file name.
*/
Module_Status PauseLog(const char* logName)
{
	Module_Status result = H05R0_OK;


	return result;	
} 

/* --- Resume a paused data log. 
				logName: Log file name.
*/
Module_Status ResumeLog(const char* logName)
{
	Module_Status result = H05R0_OK;


	return result;	
} 

/* --- Start an existing data log. 
				logName: Log file name.
				options: DELETE_ALL, KEEP_ON_DISK
*/
Module_Status DeleteLog(const char* logName, options_t options)
{
	Module_Status result = H05R0_OK;


	return result;	
}

/*-----------------------------------------------------------*/

/* -----------------------------------------------------------------------
	|															Commands																 	|
   ----------------------------------------------------------------------- 
*/



/*-----------------------------------------------------------*/


/************************ (C) COPYRIGHT HEXABITZ *****END OF FILE****/
