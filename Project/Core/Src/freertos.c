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
#include "../Inc/config.h"
#include "../Inc/application/pedestrian_ctrl.h"
#include "../Inc/application/car_ctrl.h"
#include "../Inc/application/traffic_manager.h"



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
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for carCtrl */
osThreadId_t carCtrlHandle;
const osThreadAttr_t carCtrl_attributes = {
  .name = "carCtrl",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for sensorTask */
osThreadId_t sensorTaskHandle;
const osThreadAttr_t sensorTask_attributes = {
  .name = "sensorTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for shiftRegMutex */
osMutexId_t shiftRegMutexHandle;
const osMutexAttr_t shiftRegMutex_attributes = {
  .name = "shiftRegMutex"
};
/* Definitions for pedCrossingSemaphore */
osSemaphoreId_t pedCrossingSemaphoreHandle;
const osSemaphoreAttr_t pedCrossingSemaphore_attributes = {
  .name = "pedCrossingSemaphore"
};
/* Definitions for dirVerticalEvents */
osEventFlagsId_t dirVerticalEventsHandle;
const osEventFlagsAttr_t dirVerticalEvents_attributes = {
  .name = "dirVerticalEvents"
};
/* Definitions for dirHorizontalEvents */
osEventFlagsId_t dirHorizontalEventsHandle;
const osEventFlagsAttr_t dirHorizontalEvents_attributes = {
  .name = "dirHorizontalEvents"
};
/* Definitions for pedEventFlags */
osEventFlagsId_t pedEventFlagsHandle;
const osEventFlagsAttr_t pedEventFlags_attributes = {
  .name = "pedEventFlags"
};
/* Definitions for carSensorEvent */
osEventFlagsId_t carSensorEventHandle;
const osEventFlagsAttr_t carSensorEvent_attributes = {
  .name = "carSensorEvent"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);
void StartTask04(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of shiftRegMutex */
  shiftRegMutexHandle = osMutexNew(&shiftRegMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  shiftRegMutex = shiftRegMutexHandle;
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of pedCrossingSemaphore */
  pedCrossingSemaphoreHandle = osSemaphoreNew(1, 1, &pedCrossingSemaphore_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  pedCrossingSemaphore = pedCrossingSemaphoreHandle;
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

  /* creation of carCtrl */
  carCtrlHandle = osThreadNew(StartTask03, NULL, &carCtrl_attributes);

  /* creation of sensorTask */
  sensorTaskHandle = osThreadNew(StartTask04, NULL, &sensorTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* creation of dirVerticalEvents */
  dirVerticalEventsHandle = osEventFlagsNew(&dirVerticalEvents_attributes);

  /* creation of dirHorizontalEvents */
  dirHorizontalEventsHandle = osEventFlagsNew(&dirHorizontalEvents_attributes);

  /* creation of pedEventFlags */
  pedEventFlagsHandle = osEventFlagsNew(&pedEventFlags_attributes);

  /* creation of carSensorEvent */
  carSensorEventHandle = osEventFlagsNew(&carSensorEvent_attributes);


  /* USER CODE BEGIN RTOS_EVENTS */
  dirVerticalEvents = dirVerticalEventsHandle;
  dirHorizontalEvents = dirHorizontalEventsHandle;
  pedEventFlags = pedEventFlagsHandle;  // THIS IS WHAT'S MISSING!
  carSensorEvents = carSensorEventHandle;
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
	  for(;;)
	  {
		pedestrianCtrlTask(argument);
	  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the carCtrlTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
  for(;;) {
	carCtrlTask(argument);
  }
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
* @brief Function implementing the sensorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask04 */
void StartTask04(void *argument)
{
  /* USER CODE BEGIN StartTask04 */
  /* Infinite loop */
  for(;;)
  {
	  Task3_Coordinator();
  }
  /* USER CODE END StartTask04 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

