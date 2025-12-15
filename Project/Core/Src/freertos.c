/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "ssd1306_tests.h"


/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* USER CODE BEGIN PD */
extern SPI_HandleTypeDef hspi3;
extern I2C_HandleTypeDef hi2c1;
/* USER CODE END PD */

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

/* Function prototypes */
void StartDefaultTask(void *argument);
void StartTask02(void *argument);

/* FreeRTOS init */
void MX_FREERTOS_Init(void)
{
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
  pedestrianCtrlHandle = osThreadNew(StartTask02, NULL, &pedestrianCtrl_attributes);
}

/* Default task */
void StartDefaultTask(void *argument)
{
  for(;;)
  {
    osDelay(1); // Idle task
  }
}

/* Pedestrian + OLED task */
void StartTask02(void *argument)
{
    /* 1. Blink LD2 to indicate task start */
    for(int i = 0; i < 3; i++) {
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        osDelay(200);
    }

    /* 2. Initialize shift registers */
    HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(Enable_GPIO_Port, Enable_Pin, GPIO_PIN_RESET);

    /* 3. OLED animation init */
    ssd1306_AnimInit();
    osDelay(100);

    /* 4. Test pedestrian LEDs */
    HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
    uint8_t pedRed[3] = {0x00, 0x00, 0x08};
    HAL_SPI_Transmit(&hspi3, pedRed, 3, 100);
    HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
    osDelay(3000);

    HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
    uint8_t pedGreen[3] = {0x00, 0x00, 0x10};
    HAL_SPI_Transmit(&hspi3, pedGreen, 3, 100);
    HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
    osDelay(3000);

    HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
    uint8_t pedBlue[3] = {0x00, 0x00, 0x20};
    HAL_SPI_Transmit(&hspi3, pedBlue, 3, 100);
    HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
    osDelay(3000);

    HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
    uint8_t allPed[3] = {0x00, 0x00, 0x38};
    HAL_SPI_Transmit(&hspi3, allPed, 3, 100);
    HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);
    osDelay(5000);

    // Clear all LEDs
    HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_RESET);
    uint8_t clear[3] = {0x00, 0x00, 0x00};
    HAL_SPI_Transmit(&hspi3, clear, 3, 100);
    HAL_GPIO_WritePin(STPC_GPIO_Port, STPC_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

    /* 5. MAIN LOOP: only increments frame array & calls AniFrame */
    int columnFrames[NUM_COLUMNS] = {0}; // Start fully visible

    for(;;)
    {
        // Increment frames (0→8, clamp at 8)
        for(int i = 0; i < NUM_COLUMNS; i++)
        {
            if(columnFrames[i] < 8)
                columnFrames[i]++;
        }

        ssd1306_AniFrame(columnFrames); // Handle all animation & bottom static part
        osDelay(40);                     // ~25 FPS
    }
}
