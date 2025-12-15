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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern SPI_HandleTypeDef hspi3;

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

  // Initialize SPI
  MX_SPI3_Init();

  // LD2 blink 3 times - show task started
  for(int i = 0; i < 3; i++) {
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      osDelay(200);
  }
  osDelay(500);

  // CORRECT SHIFT REGISTER INITIALIZATION:
  // MR# = PA7 = HIGH (release reset)
  // OE# = PC7 = LOW (enable outputs)
  HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_SET);     // PA7 HIGH
  HAL_GPIO_WritePin(Enable_GPIO_Port, Enable_Pin, GPIO_PIN_RESET); // PC7 LOW
  osDelay(100);

  // === TEST 1: ALL LEDs ON ===
  // Clear all first
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t clearData[3] = {0x00, 0x00, 0x00};
  HAL_SPI_Transmit(&hspi3, clearData, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
  osDelay(1000);

  // Turn ALL LEDs ON
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t allOnData[3] = {0xFF, 0xFF, 0xFF};
  HAL_SPI_Transmit(&hspi3, allOnData, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);

  // LD2 ON solid for 3 seconds while ALL LEDs should be ON
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  osDelay(3000);
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  // === TEST 2: TEST EACH LED SEPARATELY ===

  // Test Car RED (bit 0)
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t test1[3] = {0x00, 0x00, 0x01}; // BIT_TL1_RED
  HAL_SPI_Transmit(&hspi3, test1, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
  // LD2: 1 blink
  for(int i = 0; i < 2; i++) {
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      osDelay(300);
  }
  osDelay(2000);

  // Test Car ORANGE (bit 1)
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t test2[3] = {0x00, 0x00, 0x02}; // BIT_TL1_ORANGE
  HAL_SPI_Transmit(&hspi3, test2, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
  // LD2: 2 blinks
  for(int i = 0; i < 4; i++) {
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      osDelay(200);
  }
  osDelay(2000);

  // Test Car GREEN (bit 2)
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t test3[3] = {0x00, 0x00, 0x04}; // BIT_TL1_GREEN
  HAL_SPI_Transmit(&hspi3, test3, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
  // LD2: 3 blinks
  for(int i = 0; i < 6; i++) {
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      osDelay(150);
  }
  osDelay(2000);

  // Test Pedestrian RED (bit 3)
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t test4[3] = {0x00, 0x00, 0x08}; // BIT_PL1_RED
  HAL_SPI_Transmit(&hspi3, test4, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
  // LD2: 4 blinks
  for(int i = 0; i < 8; i++) {
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      osDelay(150);
  }
  osDelay(2000);

  // Test Pedestrian GREEN (bit 4)
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t test5[3] = {0x00, 0x00, 0x10}; // BIT_PL1_GREEN
  HAL_SPI_Transmit(&hspi3, test5, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
  // LD2: 5 blinks
  for(int i = 0; i < 10; i++) {
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      osDelay(120);
  }
  osDelay(2000);

  // Test Pedestrian BLUE (bit 5)
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t test6[3] = {0x00, 0x00, 0x20}; // BIT_PL1_BLUE
  HAL_SPI_Transmit(&hspi3, test6, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
  // LD2: 6 blinks
  for(int i = 0; i < 12; i++) {
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      osDelay(100);
  }
  osDelay(2000);

  // === TEST 3: INITIAL STATE (Pedestrian RED + Car GREEN) ===
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t initialState[3] = {0x00, 0x00, 0x08 | 0x04}; // Bits 3 + 2
  HAL_SPI_Transmit(&hspi3, initialState, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
  // LD2: Solid ON for 3 seconds
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  osDelay(3000);
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  // === TEST 4: WALKING STATE (Pedestrian GREEN + Car RED) ===
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t walkingState[3] = {0x00, 0x00, 0x10 | 0x01}; // Bits 4 + 0
  HAL_SPI_Transmit(&hspi3, walkingState, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
  // LD2: 3 fast blinks
  for(int i = 0; i < 6; i++) {
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      osDelay(150);
  }
  osDelay(2000);

  // === TEST 5: WARNING STATE (Car ORANGE only) ===
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t warningState[3] = {0x00, 0x00, 0x02}; // Bit 1
  HAL_SPI_Transmit(&hspi3, warningState, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
  // LD2: 2 medium blinks
  for(int i = 0; i < 4; i++) {
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      osDelay(250);
  }
  osDelay(2000);

  // === FINAL: ALL LEDs BLINK 3 TIMES ===
  for(int blink = 0; blink < 3; blink++) {
      // All ON
      HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
      uint8_t blinkOn[3] = {0xFF, 0xFF, 0xFF};
      HAL_SPI_Transmit(&hspi3, blinkOn, 3, 100);
      HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
      osDelay(500);

      // All OFF
      HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
      uint8_t blinkOff[3] = {0x00, 0x00, 0x00};
      HAL_SPI_Transmit(&hspi3, blinkOff, 3, 100);
      HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
      osDelay(500);
  }

  // === SET READY STATE ===
  // Initial state for pedestrian control
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
  uint8_t readyState[3] = {0x00, 0x00, 0x08 | 0x04}; // Ped RED + Car GREEN
  HAL_SPI_Transmit(&hspi3, readyState, 3, 100);
  HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);

  // LD2 slow blink forever = ready
  while(1) {
      HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      osDelay(1000);
  }

  /* USER CODE END StartTask02 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

