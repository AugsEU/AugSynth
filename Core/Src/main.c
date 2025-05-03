/**
	******************************************************************************
	* @file           : main.c
	* @brief          : Main program body
	******************************************************************************
	* @attention
	*
	* Copyright (c) 2025 STMicroelectronics.
	* All rights reserved.
	*
	* This software is licensed under terms that can be found in the LICENSE file
	* in the root directory of this software component.
	* If no LICENSE file comes with this software, it is provided AS-IS.
	*
	******************************************************************************
	*/

/* ---------------------------------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "UnityBuild.h"

#define UNUSED_VAR(x) (void)x

/* ----------------------------------------------------------------------------*/
/* Globals --------------------------------------------------------------------*/
I2S_HandleTypeDef hi2s1;
DMA_HandleTypeDef hdma_spi1_tx;
UART_HandleTypeDef huart2;

uint16_t gAudioBuffer[AUDIO_BUFF_LEN];
uint8_t gRxBuff[RX_BUFF_LEN];
uint8_t gMidiBuff[RX_BUFF_LEN];
uint8_t gMidiBuffIdx = 0;

/* ----------------------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2S1_Init(void);
static void MX_USART2_UART_Init(void);


/* ----------------------------------------------------------------------------*/
/* Private user code ---------------------------------------------------------*/

//when DMA output to dac is halfway done, we will reload the first 1000 values
//while the DMA output continues with the 2nd half of the buffer
//the BUFF_LEN_DIV4=500 but we are doing left and right samples (*2)= 1000 samples.
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef* hi2s)
{
	UNUSED_VAR(hi2s);

	FillSoundBuffer(gAudioBuffer, AUDIO_BUFF_LEN_DIV4);
}



//The DMA output has reached the end of the buffer.  The 1st half is done loading and the DMA
//is going to start sending the 1st half of elements/values to the DAC Chip again.  While it is
//doing that we want to fill in the 2nd half again starting at 1000 elements past &audiobuff
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef* hi2s)
{
	UNUSED_VAR(hi2s);
	
	FillSoundBuffer(gAudioBuffer + AUDIO_BUFF_LEN_DIV2, AUDIO_BUFF_LEN_DIV4);
}


//callback when a value is received at the midi port
// (the HAL_NVIC_SetPriority in HAL_UART_MspInit must be set to 0!)
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
	UNUSED_VAR(huart);

	ProcessMidiMessage(gRxBuff);

	HAL_UART_Receive_IT(&huart2, gRxBuff, RX_BUFF_LEN);
}



/**
	* @brief  The application entry point.
	* @retval int
	*/
int main(void)
{
	/* MCU Configuration--------------------------------------------------------*/
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_I2S1_Init();
	MX_USART2_UART_Init();

	SynthInit();

	HAL_I2S_Init(&hi2s1);
	HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t*) &gAudioBuffer[0], AUDIO_BUFF_LEN);

	/* Infinite loop */
	while (1)
	{
		HAL_UART_Receive_IT(&huart2, gRxBuff, RX_BUFF_LEN);
	}
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
	RCC_OscInitStruct.PLL.PLLM = 12;
	RCC_OscInitStruct.PLL.PLLN = 96;
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

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
	{
		Error_Handler();
	}
}



/**
	* @brief I2S1 Initialization Function
	* @param None
	* @retval None
	*/
static void MX_I2S1_Init(void)
{
	hi2s1.Instance = SPI1;
	hi2s1.Init.Mode = I2S_MODE_MASTER_TX;
	hi2s1.Init.Standard = I2S_STANDARD_PHILIPS;
	hi2s1.Init.DataFormat = I2S_DATAFORMAT_16B;
	hi2s1.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
	hi2s1.Init.AudioFreq = I2S_AUDIOFREQ_32K;
	hi2s1.Init.CPOL = I2S_CPOL_LOW;
	hi2s1.Init.ClockSource = I2S_CLOCK_PLL;
	hi2s1.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;

	if (HAL_I2S_Init(&hi2s1) != HAL_OK)
	{
		Error_Handler();
	}
}



/**
	* @brief USART2 Initialization Function
	* @param None
	* @retval None
	*/
static void MX_USART2_UART_Init(void)
{
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;

	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
}



/**
	* Enable DMA controller clock
	*/
static void MX_DMA_Init(void)
{
	/* DMA controller clock enable */
	__HAL_RCC_DMA2_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA2_Stream2_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 13, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
}



/**
	* @brief GPIO Initialization Function
	* @param None
	* @retval None
	*/
static void MX_GPIO_Init(void)
{
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
}



/**
	* @brief  This function is executed in case of error occurrence.
	* @retval None
	*/
void Error_Handler(void)
{
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
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
