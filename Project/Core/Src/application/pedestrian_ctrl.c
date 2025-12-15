/*
Task 1: Pedestrian control
Included functions:
	int pdMS_to:TICKS();
 */

#include "../Inc/application/pedestrian_ctrl.h"
#include "main.h" //HAL definitions
#include "cmsis_os2.h" //osDelay (CMSIS-RTOS v2)
#include <string.h>
#include "../Inc/config.h"
#include <stdbool.h>
#include "../Inc/spi.h"
#include "usart.h"

//Private variables (static for encapsulation)

static PedestrianState_t currentState = STATE_INIT;
static TrafficConfig_t config = {
		.toggleFreq = DEFAULT_TOGGLE_FREQ,
		.pedestrianDelay = DEFAULT_PEDESTRIAN_DELAY,
		.walkingDelay = DEFAULT_WALKING_DELAY,
		.orangeDelay = DEFAULT_ORANGE_DELAY
};

static uint32_t xBlinkStartTime = 0;
static uint32_t xOrangeStartTime = 0;
static uint32_t xWalkingStartTime = 0;
static uint32_t cycleCounter = 0;

/*Private helper functions
 * Sets pedestrian LEDs to specified states: RED, GREEN, OFF (for blinking)
 */

/*static void ShiftOutByte(uint8_t data) {
    for(int8_t i = 7; i >= 0; i--) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, (data & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
        HAL_Delay(1); // small delay
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
    }
}*/

static void UpdateShiftRegisters(void) {
	    HAL_StatusTypeDef status;

	    // 1. Latch LOW before shifting
	    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);  // STCP = LOW

	    // 2. Send data via SPI - ALL 3 BYTES in ONE transmission
	    uint8_t spiData[3] = {shiftRegData[0], shiftRegData[1], shiftRegData[2]};

	    // Try with HAL_SPI_Transmit for all 3 bytes at once
	    status = HAL_SPI_Transmit(&hspi3, spiData, 3, HAL_MAX_DELAY);

	    if(status != HAL_OK) {
	        // SPI error - blink LD2 to indicate error
	        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	        return;
	    }

	    // 3. Latch HIGH to update outputs
	    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);  // STCP = HIGH

	    // Small delay to ensure shift register latches
	    osDelay(1);
    /*HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);

    // Send data
    HAL_SPI_Transmit(&hspi3, &shiftRegData[2], 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi3, &shiftRegData[1], 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi3, &shiftRegData[0], 1, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

    // Small delay to ensure shift register latches the data
    HAL_Delay(1);  // Or use a smaller delay with HAL_Delay_us() if available
    */
}

static void SetPedestrianLight(LightState_t state) {
	shiftRegData[0]&= ~(BIT_PL1_RED | BIT_PL1_GREEN | BIT_PL1_BLUE);

	switch(state) {
	case LIGHT_RED:
		shiftRegData[0] |= BIT_PL1_RED;
		break;

	case LIGHT_GREEN:
		shiftRegData[0] |= BIT_PL1_GREEN;
		break;
	case LIGHT_OFF:
		break;

	case LIGHT_BLUE:
		shiftRegData[0] |= BIT_PL1_BLUE;
		break;

	default:
		break;
	}
	UpdateShiftRegisters();
}

/*Sets car traffic light vis SPI shift register
 * States: RED, ORANGE, GREEN
 */

static void SetCarLight(LightState_t state) {
	shiftRegData[0]&= ~(BIT_TL1_RED | BIT_TL1_ORANGE | BIT_TL1_GREEN);
	//Calculate shift register pattern
	//Assuming: Bit 0-2: vertical, Bit: 3-5: horizontal
	switch (state) {
	case LIGHT_RED:
		shiftRegData[0] |= BIT_TL1_RED;
		break;
	case LIGHT_ORANGE:
		shiftRegData[0] |= BIT_TL1_ORANGE;
		break;
	case LIGHT_GREEN:
		shiftRegData[0] |= BIT_TL1_GREEN;
		break;
	default:
		shiftRegData[0] |= BIT_PL1_RED;
		break;
	}
	UpdateShiftRegisters();
}

/*
 * Toggles the blinking indicator LED
 */
static void TogglePedestrianIndicator(void) {
	static bool indicatorState = false;
	indicatorState = !indicatorState;

	if(indicatorState) {
		SetPedestrianLight(LIGHT_BLUE);
	} else {
		SetPedestrianLight(LIGHT_OFF);
	}
}

// Only check upper pedestrian button (PA15/SW5)
static bool IsButtonPressedDebounced(void) {
    static uint32_t lastPressTime = 0;
    const uint32_t debounceDelay = 50;

    uint32_t currentTime = osKernelGetTickCount();

    // Only check PA15 (upper crossing)
    if ((HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) == GPIO_PIN_RESET) &&
        ((currentTime - lastPressTime) > debounceDelay)) {
        lastPressTime = currentTime;
        return true;
    }
    return false;
}
/*
 * Check if a delay period has passed
 * startTime: WHen the delay started, in ticks
 * delayMS: Delay duration in ms
 * returns true if delay has passed
 */

static bool IsDelayPassed(uint32_t startTime, uint32_t delayMs) {
    uint32_t currentTime = osKernelGetTickCount();  // CMSIS function
    return ((currentTime - startTime) >= delayMs);  // Direct ms comparison
}

/*
 * Public API functions
 */

void PedestrianCtrl_Init(void) {
	//Initialize state
	currentState = STATE_INIT;
	cycleCounter = 0;

	//Initialize shift register data
	memset(shiftRegData, 0, sizeof(shiftRegData));

	    // Initialize shift registers with YOUR pins:
	    // 1. Release reset (MR# = HIGH = PC9)
	    HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_SET);

	    // 2. Enable outputs (OE# = LOW = PC7)
	    HAL_GPIO_WritePin(Enable_GPIO_Port, Enable_Pin, GPIO_PIN_RESET);

	    // 3. Clear all outputs
	    UpdateShiftRegisters();
	    osDelay(10);

	    // 4. Set initial lights
	    SetPedestrianLight(LIGHT_RED);
	    SetCarLight(LIGHT_GREEN);

	    // Initialize timers
	    xBlinkStartTime = 0;
	    xOrangeStartTime = 0;
	    xWalkingStartTime = 0;

}

void PedestrianCtrl_SetConfig(const TrafficConfig_t *newConfig) {
	if (newConfig != NULL) {
		if(newConfig->toggleFreq >= 10 &&
			newConfig->pedestrianDelay >= 1000 &&
			newConfig->walkingDelay >= 3000 &&
			newConfig->orangeDelay >= 1000){
			memcpy(&config, newConfig, sizeof(TrafficConfig_t));
		}
	}
}

const char* PedestrianCtrl_GetStateString(PedestrianState_t state) {
	switch (state) {
	case STATE_INIT:	return "INIT";
	case STATE_WAIT_BUTTON:	return "WAIT_BUTTON";
	case STATE_BLINKING_PED:	return "BLINKING_PED";
	case STATE_CAR_ORANGE:	return "CAR_ORANGE";
	case STATE_PED_GREEN_CAR_RED:	return "PED_GREN_CAR_RED";
	case STATE_CAR_ORANGE_2:	return "CAR_ORANGE_2";
	default:	return"UNKOWN";
	}
}

uint32_t PedestrianCtrl_GetCycleCount(void) {
	return cycleCounter;
}
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
/* Add after other includes in pedestrian_ctrl.c */

void pedestrianCtrlTask(void *argument) {

	//Enables shift register output, OE
    PedestrianCtrl_Init(); //Initialize task
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);

    //Task timing variables
    uint32_t xLastWakeTime = osKernelGetTickCount();
    const uint32_t xTaskPeriod = 10; //10ms per task period
    uint32_t xLastBlinkTime = osKernelGetTickCount();

    //Main task loop
    for(;;) {
        switch (currentState) {
            case STATE_INIT:
                SetPedestrianLight(LIGHT_RED);
                SetCarLight(LIGHT_GREEN);
                currentState = STATE_WAIT_BUTTON;
                break;

            case STATE_WAIT_BUTTON:
                if (IsButtonPressedDebounced()) {
                    currentState = STATE_BLINKING_PED;
                    xBlinkStartTime = osKernelGetTickCount();
                    xLastBlinkTime = osKernelGetTickCount();
                }
                break;

            case STATE_BLINKING_PED:
                if(IsDelayPassed(xLastBlinkTime, config.toggleFreq)) {
                    TogglePedestrianIndicator();
                    xLastBlinkTime = osKernelGetTickCount();
                }
                if(IsDelayPassed(xBlinkStartTime, config.pedestrianDelay)) {
                    currentState = STATE_CAR_ORANGE;
                    SetPedestrianLight(LIGHT_OFF);
                    xOrangeStartTime = osKernelGetTickCount();
                }
                break;

            case STATE_CAR_ORANGE:
                SetCarLight(LIGHT_ORANGE);
                if(IsDelayPassed(xOrangeStartTime, config.orangeDelay)) {
                    currentState = STATE_PED_GREEN_CAR_RED;
                    xWalkingStartTime = osKernelGetTickCount();
                }
                break;

            case STATE_PED_GREEN_CAR_RED:
                SetPedestrianLight(LIGHT_GREEN);
                SetCarLight(LIGHT_RED);
                if(IsDelayPassed(xWalkingStartTime, config.walkingDelay)) {
                    currentState = STATE_CAR_ORANGE_2;
                    xOrangeStartTime = osKernelGetTickCount();
                }
                break;

            case STATE_CAR_ORANGE_2:
                SetCarLight(LIGHT_ORANGE);
                if(IsDelayPassed(xOrangeStartTime, config.orangeDelay)) {
                    currentState = STATE_INIT;
                    cycleCounter++;
                }
                break;

            default:
                currentState = STATE_INIT;
                break;
        }

        // Use xLastWakeTime with osDelayUntil:
        xLastWakeTime += xTaskPeriod;
        osDelayUntil(xLastWakeTime);
    }
}

