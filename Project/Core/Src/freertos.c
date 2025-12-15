/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "ssd1306_tests.h"


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern SPI_HandleTypeDef hspi3;
extern I2C_HandleTypeDef hi2c1;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for pedestrianCtrl */
osThreadId_t pedestrianCtrlHandle;
const osThreadAttr_t pedestrianCtrl_attributes = {
  .name = "pedestrianCtrl",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of pedestrianCtrl */
  pedestrianCtrlHandle = osThreadNew(StartTask02, NULL, &pedestrianCtrl_attributes);



  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the pedestrianCtrl thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */

  // 1. LD2 shows task started
  for(int i = 0; i < 3; i++) {
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

      //maybe display here?

      osDelay(200);
  }

  // 2. Initialize shift registers
  HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_SET);     // Release reset
  HAL_GPIO_WritePin(Enable_GPIO_Port, Enable_Pin, GPIO_PIN_RESET); // Enable outputs
  ssd1306_Init();							//OLED init
  osDelay(100);

  // 3. TEST ONLY PEDESTRIAN LEDs

  // Test A: Pedestrian RED only (bit 3 = 0x08)
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t pedRed[3] = {0x00, 0x00, 0x08}; // BIT_PL1_RED
  HAL_SPI_Transmit(&hspi3, pedRed, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);

  // LD2: 1 blink = testing pedestrian RED
  for(int i = 0; i < 2; i++) {
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      osDelay(300);
  }
  osDelay(3000); // Wait 3 seconds - check pedestrian RED LED

  // Test B: Pedestrian GREEN only (bit 4 = 0x10)
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t pedGreen[3] = {0x00, 0x00, 0x10}; // BIT_PL1_GREEN
  HAL_SPI_Transmit(&hspi3, pedGreen, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);

  // LD2: 2 blinks = testing pedestrian GREEN
  for(int i = 0; i < 4; i++) {
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      osDelay(200);
  }
  osDelay(3000); // Wait 3 seconds - check pedestrian GREEN LED

  // Test C: Pedestrian BLUE only (bit 5 = 0x20)
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t pedBlue[3] = {0x00, 0x00, 0x20}; // BIT_PL1_BLUE
  HAL_SPI_Transmit(&hspi3, pedBlue, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);

  // LD2: 3 blinks = testing pedestrian BLUE
  for(int i = 0; i < 6; i++) {
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      osDelay(150);
  }
  osDelay(3000); // Wait 3 seconds - check pedestrian BLUE LED

  // Test D: ALL pedestrian LEDs (bits 3,4,5 = 0x38)
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t allPed[3] = {0x00, 0x00, 0x38}; // All pedestrian LEDs
  HAL_SPI_Transmit(&hspi3, allPed, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);

  // LD2: Solid ON = all pedestrian LEDs should be ON
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  osDelay(5000); // Wait 5 seconds

  // 4. FINAL: Clear all
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t clear[3] = {0x00, 0x00, 0x00};
  HAL_SPI_Transmit(&hspi3, clear, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);

  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  while(1) {
      osDelay(1000);
      ssd1306_TestAll();
  }

  /* USER CODE END StartTask02 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

