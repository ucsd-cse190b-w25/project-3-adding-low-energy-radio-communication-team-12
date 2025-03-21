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
//#include "ble_commands.h"
#include "ble.h"

#include <stdlib.h>
#include <timer.h>
#include <lsm6dsl.h>
#include <i2c.h>
#include <leds.h>

#define LSM6DSL_ADDR 0x6A // I2C address of the LSM6DSL accelerometer
#define WAKE_UP_SRC 0x1B  // Wake_up_src
#define WAKE_UP_SRC_Msk 0x08 // mask for reading relevant bit
#define write 0
#define read 1

int dataAvailable = 0;

// ? Global Variables
// volatile uint16_t ct = 0;
// volatile uint8_t bol = 0;
// volatile uint8_t act = 0;
// volatile uint16_t i50 = 0;

volatile int16_t pX = 0;
volatile int16_t pY = 0;
volatile int16_t pZ = 0;

// uint16_t *count = &ct;
volatile uint16_t boolLost = 0;
volatile uint16_t boolAct = 0;
// BRUTE FORCE HOWEVER COULD COME BACKTO FIX
volatile uint16_t incTenSeconds = -1;

int16_t *prevX = &pX;
int16_t *prevY = &pY;
int16_t *prevZ = &pZ;

uint8_t cyclesSaved = 0;
volatile uint8_t timeLost = 0;
// ?


SPI_HandleTypeDef hspi3;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void setupGPIOInterrupt();
static void MX_SPI3_Init(void);

int _write(int file, char *ptr, int len) {
    int i = 0;
    for (i = 0; i < len; i++) {
        ITM_SendChar(*ptr++);
    }
    return len;
}

// void EXTI15_10_IRQHandler(void) {
  
  // if (__HAL_GPIO_EXTI_GET_IT(LSM6DSL_Pin) != RESET) {
      
  //   __HAL_GPIO_EXTI_CLEAR_IT(LSM6DSL_Pin);

  //   // Read the STATUS register to clear the interrupt flag inside the LSM6DSL
  //   uint8_t status;
  //   i2c_transaction(LSM6DSL_ADDR, 1, &status, 1);  // Read from STATUS_REG

  //   boolLost = 0;
  //   incTenSeconds = 0;
  //   timeLost = 0;
  //   timer_reset(TIM2);
  //   leds_set((uint8_t)2);
  //   setDiscoverability(0);
    // disconnectBLE();
    // standbyBle();
  // }
// }


// ? Interupt Handler
void TIM2_IRQHandler() {
  //increment i50
  ((incTenSeconds)++);

  if (incTenSeconds == 6) {

    boolLost = 1;
  }

  boolAct = 1;
  
  // manually reset update bit so interrupts can happen again
  TIM2->SR &= ~TIM_SR_UIF;
}
// ?

// void LPTIM1_IRQHandler(void) {
//   // Check if the Compare Match interrupt flag is set
//   if (LPTIM1->ISR & LPTIM_ISR_CMPM) {
//     // Clear the Compare Match flag by writing to the Interrupt Clear Register
//     LPTIM1->ICR = LPTIM_ICR_CMPMCF;

//     // Increment our counter
//     incTenSeconds++;

//     if (incTenSeconds == 2) {
//       boolLost = 1;
//     }

//     boolAct = 1;
//   }
// }


// ? Motion function
int motion() {

	uint16_t threshold = 3000;

	int16_t x,y,z;

	lsm6dsl_read_xyz(&x, &y, &z);

	// not lost, therefore reset count
	z -= 16550;
	
	if (abs(x - *prevX) * abs(x - *prevX) + abs(y - *prevY) * abs(y - *prevY) + abs(z - *prevZ) * abs(z - *prevZ) > threshold * threshold) {

		*prevX = x;
		*prevY = y;
		*prevZ = z;
		return 1;
	}


	*prevX = x;
	*prevY = y;
	*prevZ = z;
	return 0;
}
// ?

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
  MX_SPI3_Init();

  //RESET BLE MODULE
  HAL_GPIO_WritePin(BLE_RESET_GPIO_Port,BLE_RESET_Pin,GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(BLE_RESET_GPIO_Port,BLE_RESET_Pin,GPIO_PIN_SET);

  ble_init();
  standbyBle();
  HAL_Delay(10);

  uint8_t nonDiscoverable = 0;


  // ? OldMain initalization

  leds_init();
	// timer_init(TIM2);
  timer_init(TIM2);
	i2c_init();
	lsm6dsl_init();	
  // setupMotionInterrupt();
  timer_reset(TIM2);
	//timer_set_ms(TIM2, 10000);

  // ? Initalize Count
	boolLost = 0;
  timeLost = 0;

	uint8_t b1, b2, bits;
  // ?

  // ! REMOVE LEDS WHEN DONE
  while (1)
  {

    if (motion() == 1) {
      boolLost = 0;
			incTenSeconds = 0;
      timeLost = 0;
      timer_reset(TIM2);
			// leds_set((uint8_t)0);
      setDiscoverability(0);
      disconnectBLE();
      standbyBle();
    }

    // want to connect, set discoverability to 1
    // want to disconnect, set discoverability to 0 and call disconnectble
    
    if(!nonDiscoverable && HAL_GPIO_ReadPin(BLE_INT_GPIO_Port,BLE_INT_Pin)){
	    catchBLE();
	  }

    
  
    // // Read the register to clear the interrupt flag inside the LSM6DSL
    // uint8_t data[1];

    // uint8_t reg = WAKE_UP_SRC;
    // i2c_transaction(LSM6DSL_ADDR, write, &reg, 1);
    // i2c_transaction(LSM6DSL_ADDR, read, data, 1);


    // if (data[0] & 0x0F) {

    //   boolLost = 0;
    //   incTenSeconds = 0;
    //   timeLost = 0;
    //   timer_reset(TIM2);
    //   leds_set((uint8_t)2);
    //   // setDiscoverability(0);
    //   // disconnectBLE();
    // }
  

    if(boolLost == 1){
    	if(boolAct == 1){
        setDiscoverability(1);
        // leds_set((uint8_t)3);
        // Send a string to the NORDIC UART service, remember to not include the newline
        // unsigned char test_str[] = "Team 12 ";
        unsigned char test_str[18] = {0};
        snprintf(test_str, sizeof(test_str), "T12 missing %ds", timeLost);
        updateCharValue(NORDIC_UART_SERVICE_HANDLE, READ_CHAR_HANDLE, 0, sizeof(test_str)-1, test_str);
        timeLost += 10;
        boolAct = 0;
      }
    }

    // Enter Stop Mode, then wait for interrupt
    HAL_SuspendTick();
    __HAL_RCC_GPIOE_CLK_DISABLE();
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_GPIOD_CLK_DISABLE();
    __HAL_RCC_GPIOC_CLK_DISABLE();
    RCC->APB1ENR1 &= ~RCC_APB1ENR1_I2C2EN;  
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
    HAL_ResumeTick();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    i2c_init();
    // Wait for interrupt, only uncomment if low power is needed
    // __WFI();
  }
}

/**
  * @brief System Clock Configuration
  * @attention This changes the System clock frequency, make sure you reflect that change in your timer
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  // This lines changes system clock frequency
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 7;
  hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIO_LED1_GPIO_Port, GPIO_LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BLE_CS_GPIO_Port, BLE_CS_Pin, GPIO_PIN_SET);


  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BLE_RESET_GPIO_Port, BLE_RESET_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : BLE_INT_Pin */
  GPIO_InitStruct.Pin = BLE_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BLE_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO_LED1_Pin BLE_RESET_Pin */
  GPIO_InitStruct.Pin = GPIO_LED1_Pin|BLE_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BLE_CS_Pin */
  GPIO_InitStruct.Pin = BLE_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(BLE_CS_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

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

// void setupGPIOInterrupt() {
//   GPIO_InitTypeDef GPIO_InitStruct = {0};

//   // Configure GPIO as input with external interrupt (falling edge)
//   GPIO_InitStruct.Pin = LSM6DSL_Pin;
//   // Original
//   // GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
//   // Newly Suggested
//   GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
//   // Original
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   // Newly Suggested
//   // GPIO_InitStruct.Pull = GPIO_PULLUP;

//   HAL_GPIO_Init(LSM6DSL_Port, &GPIO_InitStruct);

//   // Enable NVIC interrupt
//   HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
//   HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  
// }