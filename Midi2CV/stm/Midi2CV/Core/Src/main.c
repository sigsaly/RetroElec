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
#include <stdio.h>
#include <stdarg.h>
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

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define U_BUF_SIZE (64)
#define MIDI_BUF_SIZE (64)

uint8_t rx_push_pos = 0;
uint8_t rx_pop_pos = 0;
uint8_t rx_data;
uint8_t rx_buff[U_BUF_SIZE];

uint8_t tx_push_pos = 0;
uint8_t tx_pop_pos = 0;
uint8_t tx_data;
uint8_t tx_buff[U_BUF_SIZE];

uint8_t rmidi_push_pos = 0;
uint8_t rmidi_pop_pos = 0;
uint8_t rmidi_data;
uint8_t rmidi_buff[MIDI_BUF_SIZE];

uint8_t tmidi_push_pos = 0;
uint8_t tmidi_pop_pos = 0;
uint8_t tmidi_data;
uint8_t tmidi_buff[MIDI_BUF_SIZE];


void u_putchar(uint8_t tx_data)
{
	//if(huart1.gState == HAL_UART_STATE_READY)
	//{
	//	HAL_UART_Transmit_IT(&huart1, &tx_data, 1);
	//}
	//else
	{
		tx_buff[tx_push_pos] = tx_data;
		if(tx_push_pos >= U_BUF_SIZE - 1)
			tx_push_pos = 0;
		else
			tx_push_pos++;
	}
}

int __io_putchar(int ch)
{
	u_putchar((uint8_t)ch);
	return ch;
}

void u_printf(const char* format, ...)
{
	va_list arg;
	va_start(arg, format);
	vprintf(format, arg);
	va_end(arg);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1){
		rx_buff [rx_push_pos] = rx_data;
		if(rx_push_pos >= U_BUF_SIZE - 1)
			rx_push_pos = 0;
		else
			rx_push_pos++;
		HAL_UART_Receive_IT(&huart1, &rx_data, 1);

	}
	else if(huart->Instance == USART2){
		rmidi_buff [rmidi_push_pos] = rmidi_data;
		if(rmidi_push_pos >= MIDI_BUF_SIZE - 1)
			rmidi_push_pos = 0;
		else
			rmidi_push_pos++;
		HAL_UART_Receive_IT(&huart2, &rmidi_data, 1);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
}

void uart_proc(void)
{
	static uint8_t midi_data[3];
	static int midi_index = 0;
	while(rmidi_push_pos != rmidi_pop_pos)
	{
		uint8_t ch = rmidi_buff[rmidi_pop_pos];
		if(rmidi_pop_pos >= MIDI_BUF_SIZE - 1)
			rmidi_pop_pos = 0;
		else
			rmidi_pop_pos++;

		if(ch & 0x80){
			midi_index = 0;
			midi_data[midi_index] = ch;
			midi_index++;
			//u_printf("status: %02x\n", ch);
		}else{
			if(midi_index <= 2){
				midi_data[midi_index] = ch;
				midi_index++;
				if(midi_index == 3){
					u_printf("midi: %02x %02x %02x\n", midi_data[0], midi_data[1], midi_data[2]);
					midi_index = 0;
				}
			}else{
				u_printf("Error: midi_index: %d\n", midi_index);
			}
			//u_printf("data: %02x\n", ch);
		}

	}

	if(tmidi_push_pos != tmidi_pop_pos)
	{
		if(huart2.gState == HAL_UART_STATE_READY)
		{
			uint8_t ch = tmidi_buff[tmidi_pop_pos];
			if(tmidi_pop_pos >= MIDI_BUF_SIZE - 1)
				tmidi_pop_pos = 0;
			else
				tmidi_pop_pos++;
			HAL_UART_Transmit_IT(&huart2, (uint8_t *)&ch, 1);
		}
	}

	/* echo received chars */
	while(rx_push_pos != rx_pop_pos)
	{
		uint8_t ch = rx_buff[rx_pop_pos];
		if(rx_pop_pos >= U_BUF_SIZE - 1)
			rx_pop_pos = 0;
		else
			rx_pop_pos++;
		u_putchar(ch);
	}

	if(tx_push_pos != tx_pop_pos)
	{
		if(huart1.gState == HAL_UART_STATE_READY)
		{
			uint8_t ch = tx_buff[tx_pop_pos];
			if(tx_pop_pos >= U_BUF_SIZE - 1)
				tx_pop_pos = 0;
			else
				tx_pop_pos++;
			HAL_UART_Transmit_IT(&huart1, (uint8_t *)&ch, 1);
		}
	}
}

void send_midi(uint8_t data)
{
	if(huart2.gState == HAL_UART_STATE_READY)
	{
		HAL_UART_Transmit_IT(&huart2, &data, 1);
	}else{
		tmidi_buff[tmidi_push_pos] = data;
		if(tmidi_push_pos >= MIDI_BUF_SIZE - 1)
			tmidi_push_pos = 0;
		else
			tmidi_push_pos++;
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();


  u_printf("\nSTM32 starts...\r\n");
  HAL_UART_Receive_IT(&huart1, &rx_data, 1);
  HAL_UART_Receive_IT(&huart2, &rmidi_data, 1);

  /* Infinite loop */
  while (1)
  {
	  uart_proc();
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 32800; //31250;
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
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

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
