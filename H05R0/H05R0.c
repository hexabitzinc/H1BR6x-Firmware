/*
    BitzOS (BOS) V0.1.2 - Copyright (C) 2017 Hexabitz
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
char buffer[20], lineBuffer[100];
char logHeaderText1[] = "Datalog created by BOS V%d.%d.%d on %s\n";
char logHeaderText2[] = "Log type: Rate @ %.2f Hz\n\n";
char logHeaderText3[] = "Log type: Events\n\n";
uint16_t openLog = 0xFFFF, activeLogs;
TaskHandle_t LogTaskHandle = NULL;
uint8_t temp_uint8 = 0; 
float *ptemp_float[MAX_LOG_VARS]; 

/* Private function prototypes -----------------------------------------------*/	
void LogTask(void * argument);
uint8_t CheckLogVarEvent(uint16_t varIndex);
Module_Status OpenThisLog(uint16_t logindex);


/* Create CLI commands --------------------------------------------------------*/

portBASE_TYPE addLogCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE deleteLogCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE logVarCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE startCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE stopCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE pauseCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );
portBASE_TYPE resumeCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString );

/* CLI command structure : addlog */
static const CLI_Command_Definition_t addLogCommandDefinition =
{
	( const int8_t * ) "addlog", /* The command string to type. */
	( const int8_t * ) "addlog:\r\n Add a new log file. Specifiy log name (1st par.); type (2nd par.): 'rate' or 'event'; \
rate (3rd par.): logging rate in Hz (max 1kHz), delimiter format (4th par.): 'space', 'tab' or 'comma'; index column format \
(5th par.): 'none', 'sample' or 'time'; and index column label text (6th par.)\r\n\r\n",
	addLogCommand, /* The function to run. */
	6 /* Six parameters are expected. */
};
/*-----------------------------------------------------------*/
/* CLI command structure : logvar */
static const CLI_Command_Definition_t logVarCommandDefinition =
{
	( const int8_t * ) "logvar", /* The command string to type. */
	( const int8_t * ) "logvar:\r\n Add a new log variable to an existing log (1st par.). Specify variable type (2nd and 3rd par.): \
'port digital', 'port data', 'port button', 'memory uint8', 'memory int8', 'memory uint16', 'memory int16', 'memory uint32', \
'memory int32', 'memory float' ; source (4th par.): ports 'p1'..'px', buttons 'b1'..'bx' or memory location (Flash or RAM); \
and column label text (5th par.)\r\n\r\n",
	logVarCommand, /* The function to run. */
	5 /* Five parameters are expected. */
};
/*-----------------------------------------------------------*/
/* CLI command structure : deletelog */
static const CLI_Command_Definition_t deletelogCommandDefinition =
{
	( const int8_t * ) "deletelog", /* The command string to type. */
	( const int8_t * ) "deletelog:\r\n Delete a log file. Specifiy log name (1st par.) and delete options (2nd par.): 'all' or \
'keepdisk' to keep log on the uSD card\r\n\r\n",
	deleteLogCommand, /* The function to run. */
	2 /* Two parameters are expected. */
};
/*-----------------------------------------------------------*/
/* CLI command structure : start */
static const CLI_Command_Definition_t startCommandDefinition =
{
	( const int8_t * ) "start", /* The command string to type. */
	( const int8_t * ) "start:\r\n Start the log with log name (1st par.)\r\n\r\n",
	startCommand, /* The function to run. */
	1 /* One parameter is expected. */
};
/*-----------------------------------------------------------*/
/* CLI command structure : stop */
static const CLI_Command_Definition_t stopCommandDefinition =
{
	( const int8_t * ) "stop", /* The command string to type. */
	( const int8_t * ) "stop:\r\n Stop the log with log name (1st par.)\r\n\r\n",
	stopCommand, /* The function to run. */
	1 /* One parameter is expected. */
};
/*-----------------------------------------------------------*/
/* CLI command structure : pause */
static const CLI_Command_Definition_t pauseCommandDefinition =
{
	( const int8_t * ) "pause", /* The command string to type. */
	( const int8_t * ) "pause:\r\n Pause the log with log name (1st par.)\r\n\r\n",
	pauseCommand, /* The function to run. */
	1 /* One parameter is expected. */
};
/*-----------------------------------------------------------*/
/* CLI command structure : resume */
static const CLI_Command_Definition_t resumeCommandDefinition =
{
	( const int8_t * ) "resume", /* The command string to type. */
	( const int8_t * ) "resume:\r\n Resume the log with log name (1st par.)\r\n\r\n",
	resumeCommand, /* The function to run. */
	1 /* One parameter is expected. */
};
/*-----------------------------------------------------------*/

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
	
	/* This module needs more time to process buttons */
	needToDelayButtonStateReset = true;
		
	/* uSD - GPIO and SPI */
	if (BSP_SD_Init() == MSD_ERROR)
	{
		/* No SD card. Insert SD card and reboot */
		while(1) { IND_ON(); Delay_ms_no_rtos(500); IND_OFF(); Delay_ms_no_rtos(500); };		
	}	
	
	/* Get the uSD size and info */
	BSP_SD_GetCardInfo(&CardInfo);
	
	/* Link the micro SD disk I/O driver */
  if(FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
  {
    /* Mount the default drive */
    if(f_mount(&SDFatFs, SDPath, 1) != FR_OK)
    {
			/* Unmount the drive */
			f_mount(0, SDPath, 0); 
      /* SD card malfunction. Re-insert the card and reboot */
			while(1) { IND_ON(); Delay_ms_no_rtos(500); IND_OFF(); Delay_ms_no_rtos(500); };	
    }
    else
    {		
			/* Create the logging task */
			xTaskCreate(LogTask, (const char *) "LogTask", (2*configMINIMAL_STACK_SIZE), NULL, osPriorityNormal, &LogTaskHandle);			
		}
	}
	
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
	uint8_t i, j, ii, eventResult; 
	static uint8_t newline;
	
	/* Infinite loop */
	for(;;)
	{
		/* Check all active logs */
		for( j=0 ; j<MAX_LOGS ; j++)
		{	
			if ( (activeLogs >> j) & 0x01 )
			{			
				/* Open this log file if it's closed (and close open one) */
				OpenThisLog(j);					
				
				/* Check all registered variables */
				for( i=0 ; i<MAX_LOG_VARS ; i++)
				{
					if (logVars[i].type)
					{
						eventResult = CheckLogVarEvent(i);
						
						/* Check for rate or event */
						if ( (logs[j].type == RATE && (HAL_GetTick()-logs[j].t0) >= (configTICK_RATE_HZ/logs[j].rate)) || (logs[j].type == EVENT && eventResult) )
						{			
							/* Execute this section only once per cycle */
							if (newline)
							{	
								newline = 0;										// Event index written once per line
								
								++(logs[j].sampleCount);				// Advance one sample
								
								/* Write new line */
								f_write(&MyFile, "\n\r", 2, (void *)&byteswritten);	
								
								/* Write index */
								if (logs[j].indexColumnFormat == FMT_TIME)
								{
									;
								}
								else if (logs[j].indexColumnFormat == FMT_SAMPLE)
								{
									sprintf( ( char * ) buffer, "%d", logs[j].sampleCount);	strcat(lineBuffer, buffer);
									memset(buffer, 0, sizeof(buffer));
								}					
							}
						
							/* Write delimiter - Add all delimiters before the variable if the log type is EVENT to clarify variable column 
									(only one variable is logged per event line) */	
							for( ii=0 ; ii<=i ; ii++)
							{	
								if (logs[j].delimiterFormat == FMT_SPACE)
									strcat(lineBuffer, " ");
								else if (logs[j].delimiterFormat == FMT_TAB)
									strcat(lineBuffer, "\t");
								else if (logs[j].delimiterFormat == FMT_COMMA)
									strcat(lineBuffer, ",");	
									
								if (logs[j].type != EVENT || logs[j].sampleCount == 1)	break;			// Do not print extra delimiters on 1st sample 
							}
							
							/* Write variable value */
							switch (logVars[i].type)
							{
								case PORT_DIGITAL:
									//sprintf( ( char * ) buffer, "%d", HAL_GPIO_ReadPin());
									//f_write(&MyFile, buffer, 1, (void *)&byteswritten);	
									break;
								
								case PORT_BUTTON:
									switch (button[logVars[i].source].state)
									{
										case OFF:	strcat(lineBuffer, "OFF"); break; 
										case ON:	strcat(lineBuffer, "ON"); break; 
										case OPEN:	strcat(lineBuffer, "OPEN"); break; 
										case CLOSED:	strcat(lineBuffer, "CLOSED"); break; 
										case CLICKED:	strcat(lineBuffer, "CLICKED"); break; 
										case DBL_CLICKED:	strcat(lineBuffer, "DBL_CLICKED"); break; 
										case PRESSED:	strcat(lineBuffer, "PRESSED"); break; 
										case RELEASED:	strcat(lineBuffer, "RELEASED"); break; 
										case PRESSED_FOR_X1_SEC: sprintf( ( char * ) buffer, "PRESSED_FOR_%d_SEC", button[logVars[i].source].pressedX1Sec); strcat(lineBuffer, buffer); break;
										case PRESSED_FOR_X2_SEC: sprintf( ( char * ) buffer, "PRESSED_FOR_%d_SEC", button[logVars[i].source].pressedX2Sec); strcat(lineBuffer, buffer); break;
										case PRESSED_FOR_X3_SEC: sprintf( ( char * ) buffer, "PRESSED_FOR_%d_SEC", button[logVars[i].source].pressedX3Sec); strcat(lineBuffer, buffer); break;
										case RELEASED_FOR_Y1_SEC:	sprintf( ( char * ) buffer, "RELEASED_FOR_%d_SEC", button[logVars[i].source].releasedY1Sec); strcat(lineBuffer, buffer); break;
										case RELEASED_FOR_Y2_SEC:	sprintf( ( char * ) buffer, "RELEASED_FOR_%d_SEC", button[logVars[i].source].releasedY2Sec); strcat(lineBuffer, buffer); break;
										case RELEASED_FOR_Y3_SEC:	sprintf( ( char * ) buffer, "RELEASED_FOR_%d_SEC", button[logVars[i].source].releasedY3Sec); strcat(lineBuffer, buffer); break;
										case NONE: if (logs[j].type == RATE) strcat(lineBuffer, "NORMAL"); break;								
										default: break;
									}		
									/* Reset button state */
									if (button[logVars[i].source].state != NONE)	delayButtonStateReset = false;
									break;
								
								case PORT_DATA:
									
									break;
								
								case MEMORY_DATA_UINT8: sprintf( ( char * ) buffer, "%u", *(__IO uint8_t *)logVars[i].source); strcat(lineBuffer, buffer); break;

								case MEMORY_DATA_INT8: sprintf( ( char * ) buffer, "%i", *(__IO uint8_t *)logVars[i].source); strcat(lineBuffer, buffer); break;

								case MEMORY_DATA_UINT16: sprintf( ( char * ) buffer, "%u", *(__IO uint16_t *)logVars[i].source); strcat(lineBuffer, buffer); break;

								case MEMORY_DATA_INT16: sprintf( ( char * ) buffer, "%i", *(__IO uint16_t *)logVars[i].source); strcat(lineBuffer, buffer); break;

								case MEMORY_DATA_UINT32: sprintf( ( char * ) buffer, "%u", *(__IO uint32_t *)logVars[i].source); strcat(lineBuffer, buffer); break;

								case MEMORY_DATA_INT32: sprintf( ( char * ) buffer, "%i", *(__IO uint32_t *)logVars[i].source); strcat(lineBuffer, buffer); break;

								case MEMORY_DATA_FLOAT: sprintf( ( char * ) buffer, "%f", *(__IO float *)logVars[i].source); strcat(lineBuffer, buffer); break;
								
								default:			
									break;
							}							
							
							/* Write the line buffer */
							f_write(&MyFile, lineBuffer, strlen((const char *)lineBuffer), (void *)&byteswritten);
							/* Clear the buffers */
							memset(buffer, 0, sizeof(buffer)); memset(lineBuffer, 0, byteswritten);					
						}			
						/* Advance samples counter even if event has not occured */
						else if ((HAL_GetTick()-logs[j].t0) >= (configTICK_RATE_HZ/logs[j].rate) && logs[j].type == EVENT && !eventResult && newline) 
						{
							newline = 0;										// Execute this only once per line
							++(logs[j].sampleCount);				// Advance one sample
						}			
					}					
				}	
				/* Start a new line entry */
				newline = 1;			
				/* Reset the rate timer */	
				if ( (HAL_GetTick()-logs[j].t0) >= (configTICK_RATE_HZ/logs[j].rate) ) 
					logs[j].t0 = HAL_GetTick();
				break;
				
			}	
			else	
				break;				
		}	
		
		taskYIELD();
	}								

}

/*-----------------------------------------------------------*/

/* --- Check if a logged variable event has occured. 
				varIndex: Log variable array index.
*/
uint8_t CheckLogVarEvent(uint16_t varIndex)
{
	switch (logVars[varIndex].type)
	{
		case PORT_DIGITAL:
			break;
		
		case PORT_BUTTON:
			if (button[logVars[varIndex].source].state != temp_uint8 && button[logVars[varIndex].source].state != 0) {
				temp_uint8 = button[logVars[varIndex].source].state;
				return 1;
			} else if (button[logVars[varIndex].source].state != temp_uint8 && button[logVars[varIndex].source].state == 0) {
				temp_uint8 = button[logVars[varIndex].source].state;
				return 0;
			}
			break;
		
		case PORT_DATA:			
			break;
		
		case MEMORY_DATA_UINT8:
			if (*(__IO uint8_t *)logVars[varIndex].source != (uint8_t)*ptemp_float[varIndex]) {
				*ptemp_float[varIndex] = *(__IO uint8_t *)logVars[varIndex].source;
				return 1;
			}
			break;
			
		case MEMORY_DATA_INT8:
			if (*(__IO uint8_t *)logVars[varIndex].source != (int8_t)*ptemp_float[varIndex]) {
				*ptemp_float[varIndex] = *(__IO uint8_t *)logVars[varIndex].source;
				return 1;
			}
			break;
			
		case MEMORY_DATA_UINT16:
			if (*(__IO uint16_t *)logVars[varIndex].source != (uint16_t)*ptemp_float[varIndex]) {
				*ptemp_float[varIndex] = *(__IO uint16_t *)logVars[varIndex].source;
				return 1;
			}
			break;
			
		case MEMORY_DATA_INT16:
			if (*(__IO uint16_t *)logVars[varIndex].source != (int16_t)*ptemp_float[varIndex]) {
				*ptemp_float[varIndex] = *(__IO uint16_t *)logVars[varIndex].source;
				return 1;
			}
			break;
			
		case MEMORY_DATA_UINT32:
			if (*(__IO uint32_t *)logVars[varIndex].source != (uint32_t)*ptemp_float[varIndex]) {
				*ptemp_float[varIndex] = *(__IO uint32_t *)logVars[varIndex].source;
				return 1;
			}
			break;
			
		case MEMORY_DATA_INT32:
			if (*(__IO uint32_t *)logVars[varIndex].source != (int32_t)*ptemp_float[varIndex]) {
				*ptemp_float[varIndex] = *(__IO uint32_t *)logVars[varIndex].source;
				return 1;
			}
			break;
			
		case MEMORY_DATA_FLOAT:
			if (*(__IO float *)logVars[varIndex].source != *ptemp_float[varIndex]) {
				*ptemp_float[varIndex] = *(__IO float *)logVars[varIndex].source;
				return 1;
			}
			break;
					
		default:			
			break;
	}
	
	return 0;	
} 

/*-----------------------------------------------------------*/

/* --- Open this log file if it's closed (and close open one). 
				logindex: Log array index.
*/
Module_Status OpenThisLog(uint16_t logindex)
{
	FRESULT res; char name[15] = {0}; 	
		
	if ( openLog != logindex )
	{					
		/* Close currently open log and store pointer lcoation */
		logs[openLog].filePtr = MyFile.fptr;
		openLog = 0xFFFF; f_close(&MyFile);
		/* Append log name with extension */
		strcpy((char *)name, logs[logindex].name); strncat((char *)name, ".TXT", 4);
		/* Open this log */			
		res = f_open(&MyFile, name, FA_OPEN_EXISTING | FA_WRITE | FA_READ);
		if (res != FR_OK)	return H05R0_ERROR;	
		openLog = logindex;
		MyFile.fptr = logs[logindex].filePtr;			// Load last pointer value
	}	
		
	return H05R0_OK;
} 


/* -----------------------------------------------------------------------
	|																APIs	 																 	|
   ----------------------------------------------------------------------- 
*/

/* --- Create a new data log. 
				logName: Log file name. Max 10 char. 
				type: RATE or EVENT
				rate: data rate in Hz (max 1000 Hz).
				delimiterFormat: FMT_SPACE, FMT_TAB, FMT_COMMA
				indexColumn: FMT_SAMPLE, FMT_TIME
				indexColumnLabel: Index Column label text. Max 30 char.
*/
Module_Status CreateLog(const char* logName, logType_t type, float rate, delimiterFormat_t delimiterFormat, indexColumnFormat_t indexColumnFormat,\
	const char* indexColumnLabel)
{
	FRESULT res; char name[15] = {0};
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
			 (delimiterFormat != FMT_SPACE && delimiterFormat != FMT_TAB && delimiterFormat != FMT_COMMA)	||
			 (indexColumnFormat != FMT_NONE && indexColumnFormat != FMT_SAMPLE && indexColumnFormat != FMT_TIME)	||
			 (rate > 1000) )
		return H05R0_ERR_WrongParams;				
					
	/* Name does not exist. Fill first empty location */
	for( i=0 ; i<MAX_LOGS ; i++)
  {
		if(logs[i].name == 0)
		{
			/* Append log name with extension */
			strcpy((char *)name, logName); strncat((char *)name, ".TXT", 4);
			/* Check if file exists on disk */
			res = f_open(&MyFile, name, FA_CREATE_NEW | FA_WRITE | FA_READ);
			if(res == FR_EXIST)
				return H05R0_ERR_LogNameExists;
			else if (res != FR_OK)
				return H05R0_ERR_SD;	
			
			/* Log created successfuly */
			openLog = i;
			logs[i].name = logName;
			logs[i].type = type;
			logs[i].rate = rate;
			logs[i].delimiterFormat = delimiterFormat;
			logs[i].indexColumnFormat = indexColumnFormat;
			logs[i].indexColumnLabel = indexColumnLabel;
			
			/* Write log header */
			strncpy(name, modulePNstring[4], 5); name[5] = 0;				// Copy only module PN
			sprintf( ( char * ) buffer, logHeaderText1, _firmMajor, _firmMinor, _firmPatch, name);
			res = f_write(&MyFile, buffer, strlen(logHeaderText1), (void *)&byteswritten);
			memset(buffer, 0, byteswritten);
			if(type == RATE) {
				sprintf( ( char * ) buffer, logHeaderText2, rate);
				res = f_write(&MyFile, buffer, strlen(logHeaderText2), (void *)&byteswritten);				
			} else if (type == EVENT) {
				res = f_write(&MyFile, logHeaderText3, strlen(logHeaderText3), (void *)&byteswritten);	
			}
			memset(buffer, 0, byteswritten);
			
			/* Write index label */
			res = f_write(&MyFile, indexColumnLabel, strlen(indexColumnLabel), (void *)&byteswritten);
							
			return H05R0_OK;
		}
  }	

	return H05R0_ERR_MaxLogs;	
}

/*-----------------------------------------------------------*/

/* --- Save data from a source to an existing data log. 
				logName: Log file name.
				type: PORT_DIGITAL, PORT_DATA, PORT_BUTTON, MEMORY_DATA.
				source: data source. Ports (P1-Px), buttons (B1-Bx) or memory location.
				columnLabel: Column label text. Max 30 char.
*/
Module_Status LogVar(const char* logName, logVarType_t type, uint32_t source, const char* ColumnLabel)
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
				if(logVars[i].type == 0)
				{		
					logVars[i].type = type;
					logVars[i].source = source;
					logVars[i].logIndex = j;
					logVars[i].varLabel = ColumnLabel;
					
					/* Alocate memory for temporary old value variables */
					if( logs[j].type == EVENT && logVars[i].type == MEMORY_DATA_UINT8)
						ptemp_float[i] = (float*)malloc(sizeof(uint8_t));
					else if (logs[j].type == EVENT && logVars[i].type == MEMORY_DATA_INT8)
						ptemp_float[i] = (float*)malloc(sizeof(int8_t));
					else if (logs[j].type == EVENT && logVars[i].type == MEMORY_DATA_UINT16)
						ptemp_float[i] = (float*)malloc(sizeof(uint16_t));
					else if (logs[j].type == EVENT && logVars[i].type == MEMORY_DATA_INT16)
						ptemp_float[i] = (float*)malloc(sizeof(int16_t));
					else if (logs[j].type == EVENT && logVars[i].type == MEMORY_DATA_UINT32)
						ptemp_float[i] = (float*)malloc(sizeof(uint32_t));
					else if (logs[j].type == EVENT && logVars[i].type == MEMORY_DATA_INT32)
						ptemp_float[i] = (float*)malloc(sizeof(int32_t));
					else if (logs[j].type == EVENT && logVars[i].type == MEMORY_DATA_FLOAT)
						ptemp_float[i] = (float*)malloc(sizeof(float));
					/* Check memory was alocated */
					if (logs[j].type == EVENT && (logVars[i].type == MEMORY_DATA_UINT8 || logVars[i].type == MEMORY_DATA_INT8 || \
						logVars[i].type == MEMORY_DATA_UINT16 || logVars[i].type == MEMORY_DATA_INT16 ||logVars[i].type == MEMORY_DATA_UINT32 || \
						logVars[i].type == MEMORY_DATA_INT32 || logVars[i].type == MEMORY_DATA_FLOAT) && ptemp_float[i] == NULL)
						return H05R0_ERR_MemoryFull;
					
					/* Write delimiter */
					OpenThisLog(j);
					if (logs[j].delimiterFormat == FMT_SPACE)
						f_write(&MyFile, " ", 1, (void *)&byteswritten);
					else if (logs[j].delimiterFormat == FMT_TAB)
						f_write(&MyFile, "\t", 1, (void *)&byteswritten);
					else if (logs[j].delimiterFormat == FMT_COMMA)
						f_write(&MyFile, ",", 1, (void *)&byteswritten);
					/* Write variable label */
					f_write(&MyFile, ColumnLabel, strlen(ColumnLabel), (void *)&byteswritten);
					
					return H05R0_OK;
				}
			}
			return H05R0_ERR_MaxLogVars;
		}
	}		

	return H05R0_ERR_LogDoesNotExist;	
}

/*-----------------------------------------------------------*/

/* --- Start an existing data log. 
				logName: Log file name.
*/
Module_Status StartLog(const char* logName)
{
	uint8_t j = 0;

	/* Search for this log to make sure it exists */
	for( j=0 ; j<MAX_LOGS ; j++)
	{
		if (!strcmp(logs[j].name, logName))
		{
			activeLogs |= (0x01 << j);
			logs[j].sampleCount = 0;
			logs[j].t0 = HAL_GetTick();
			OpenThisLog(j);
			/* Write new line */
			f_write(&MyFile, "\n\r", 2, (void *)&byteswritten);
			return H05R0_OK;
		}		
	}

	return H05R0_ERR_LogDoesNotExist;	
}

/*-----------------------------------------------------------*/

/* --- Stop a running data log. 
				logName: Log file name.
*/
Module_Status StopLog(const char* logName)
{
	uint8_t j = 0;

	/* Search for this log to make sure it exists */
	for( j=0 ; j<MAX_LOGS ; j++)
	{
		if (!strcmp(logs[j].name, logName))
		{
			if ( (activeLogs >> j) & 0x01 )
			{
				activeLogs &= ~(0x01 << j);
				logs[j].sampleCount = 0;
				logs[j].t0 = 0;
				/* Close log file */
				logs[j].filePtr = MyFile.fptr;
				openLog = 0xFFFF; f_close(&MyFile);
				return H05R0_OK;
			}
			else
				return H05R0_ERR_LogIsNotActive;
		}		
	}
	
	return H05R0_ERR_LogDoesNotExist;	
}

/*-----------------------------------------------------------*/

/* --- Pause a running data log. 
				logName: Log file name.
*/
Module_Status PauseLog(const char* logName)
{
	uint8_t j = 0;

	/* Search for this log to make sure it exists */
	for( j=0 ; j<MAX_LOGS ; j++)
	{
		if (!strcmp(logs[j].name, logName))
		{
			if ( (activeLogs >> j) & 0x01 )
			{
				activeLogs &= ~(0x01 << j);
				return H05R0_OK;
			}
			else
				return H05R0_ERR_LogIsNotActive;
		}		
	}

	return H05R0_ERR_LogDoesNotExist;	
} 

/*-----------------------------------------------------------*/

/* --- Resume a paused data log. 
				logName: Log file name.
*/
Module_Status ResumeLog(const char* logName)
{
	uint8_t j = 0;

	/* Search for this log to make sure it exists */
	for( j=0 ; j<MAX_LOGS ; j++)
	{
		if (!strcmp(logs[j].name, logName))
		{
			activeLogs |= (0x01 << j);
			return H05R0_OK;
		}		
	}

	return H05R0_ERR_LogDoesNotExist;	
} 

/*-----------------------------------------------------------*/

/* --- Delete an existing data log. 
				logName: Log file name.
				options: DELETE_ALL, KEEP_ON_DISK
*/
Module_Status DeleteLog(const char* logName, options_t options)
{
	Module_Status result = H05R0_OK;

	
	// Free ptemp vars
	
	return result;	
}

/*-----------------------------------------------------------*/

/* -----------------------------------------------------------------------
	|															Commands																 	|
   ----------------------------------------------------------------------- 
*/

portBASE_TYPE addLogCommand( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t *pcCommandString )
{
	Module_Status result = H05R0_OK;
	
	int8_t *pcParameterString1, *pcParameterString2, *pcParameterString3, *pcParameterString4, *pcParameterString5, *pcParameterString6; 
	portBASE_TYPE xParameterStringLength1 = 0, xParameterStringLength2 = 0, xParameterStringLength3 = 0; 
	portBASE_TYPE xParameterStringLength4 = 0, xParameterStringLength5 = 0, xParameterStringLength6 = 0;
	logType_t type; delimiterFormat_t dformat; indexColumnFormat_t iformat; float rate;
	static const int8_t *pcOKMessage = ( int8_t * ) "Log created successfully\r\n";
	static const int8_t *pcWrongValue = ( int8_t * ) "Log creation failed. Wrong parameters\r\n";
	static const int8_t *pcLogExists = ( int8_t * ) "Log creation failed. Log name already exists\r\n";
	static const int8_t *pcSDerror = ( int8_t * ) "Log creation failed. SD card error\r\n";
	static const int8_t *pcMaxLogs = ( int8_t * ) "Log creation failed. Maximum number of logs reached\r\n";
	
	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );
	
	/* Obtain the 1st parameter string: log name */
	pcParameterString1 = ( int8_t * ) FreeRTOS_CLIGetParameter (pcCommandString, 1, &xParameterStringLength1);
	/* Obtain the 2nd parameter string: log type */
	pcParameterString2 = ( int8_t * ) FreeRTOS_CLIGetParameter (pcCommandString, 2, &xParameterStringLength2);
	/* Obtain the 3rd parameter string: log rate */
	pcParameterString3 = ( int8_t * ) FreeRTOS_CLIGetParameter (pcCommandString, 3, &xParameterStringLength3);
	/* Obtain the 4th parameter string: delimiter format */
	pcParameterString4 = ( int8_t * ) FreeRTOS_CLIGetParameter (pcCommandString, 4, &xParameterStringLength4);
	/* Obtain the 5th parameter string: index format */
	pcParameterString5 = ( int8_t * ) FreeRTOS_CLIGetParameter (pcCommandString, 5, &xParameterStringLength5);
	/* Obtain the 6th parameter string: index label */
	pcParameterString6 = ( int8_t * ) FreeRTOS_CLIGetParameter (pcCommandString, 6, &xParameterStringLength6);
	
	/* log name */
	pcParameterString1[xParameterStringLength1] = 0;		// Get rid of the remaining parameters
	
	/* type */
	if (!strncmp((const char *)pcParameterString2, "rate", xParameterStringLength2))
		type = RATE;
	else if (!strncmp((const char *)pcParameterString2, "event", xParameterStringLength2))
		type = EVENT;		
	else
		result = H05R0_ERR_WrongParams;
	
	/* rate */
	rate = atof( ( const char * ) pcParameterString3 );
	if (rate < 0.0f || rate > 1000.0f)
		result = H05R0_ERR_WrongParams;
	
	/* delimiter format */
	if (!strncmp((const char *)pcParameterString4, "space", xParameterStringLength4))
		dformat = FMT_SPACE;
	else if (!strncmp((const char *)pcParameterString4, "tab", xParameterStringLength4))
		dformat = FMT_TAB;		
	else if (!strncmp((const char *)pcParameterString4, "comma", xParameterStringLength4))
		dformat = FMT_COMMA;		
	else
		result = H05R0_ERR_WrongParams;
	
	/* index format */
	if (!strncmp((const char *)pcParameterString5, "sample", xParameterStringLength5))
		iformat = FMT_SAMPLE;
	else if (!strncmp((const char *)pcParameterString5, "time", xParameterStringLength5))
		iformat = FMT_TIME;		
	else if (!strncmp((const char *)pcParameterString5, "none", xParameterStringLength5))
		iformat = FMT_NONE;		
	else
		result = H05R0_ERR_WrongParams;

	/* index name */
	pcParameterString6[xParameterStringLength6] = 0;		// Get rid of the remaining parameters

	
	/* Create the log */
	if (result == H05R0_OK) {
		result = CreateLog((const char *)pcParameterString1, type, rate, dformat, iformat, (const char *)pcParameterString6);	
	}
	
	/* Respond to the command */
	if (result == H05R0_OK) {
		strcpy( ( char * ) pcWriteBuffer, ( char * ) pcOKMessage);
	} else if (result == H05R0_ERR_WrongParams) {
		strcpy( ( char * ) pcWriteBuffer, ( char * ) pcWrongValue);
	} else if (result ==  H05R0_ERR_LogNameExists) {
		strcpy( ( char * ) pcWriteBuffer, ( char * ) pcLogExists);
	} else if (result ==  H05R0_ERR_SD) {
		strcpy( ( char * ) pcWriteBuffer, ( char * ) pcSDerror);
	} else if (result ==  H05R0_ERR_MaxLogs) {
		strcpy( ( char * ) pcWriteBuffer, ( char * ) pcMaxLogs);
	}
	
	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/


/************************ (C) COPYRIGHT HEXABITZ *****END OF FILE****/
