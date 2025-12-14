/*
Task 1: Pedestrian control
Included functions:
	int pdMS_to:TICKS();
 */

#include "application/pedestrian_ctrl.h"
#include "main.h" //HAL definitions
#include "cmsis_os2.h" //osDelay (CMSIS-RTOS v2)
#include <string.h>
#include "config.h"
#include <stdbool.h>

//Private variables (static for encapsulation)

static PedestrianState_t currentState = STATE_INIT;
static TrafficConfig_t config = {
		.toggleFreq = DEFAULT_TOGGLE_FREQ,
		.pedestrianDelay = DEFAULT_PEDESTRIAN_DELAY,
		.walkingDelay = DEFAULT_WALKING_DELAY,
		.orangeDelay = DEFAULT_ORANGE_DELAY
};

static TickType_t xBlinkStartTime = 0;
static TickType_t xOrangeStartTime = 0;
static TickType_t xWalkingStartTime = 0;
static uint32_t cycleCounter = 0;

/*Private helper functions
 * Sets pedestrian LEDs to specified states: RED, GREEN, OFF (for blinking)
 */

static void ShiftOutByte(uint8_t data) {
    for(int8_t i = 7; i >= 0; i--) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, (data & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
        HAL_Delay(1); // small delay
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
    }
}

static void UpdateShiftRegisters(void) {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); //Set latch to low (hold outputs while shifting)
	//Because data shifts though the chain
	ShiftOutByte(shiftRegData[2]); //U3
	ShiftOutByte(shiftRegData[1]); //U2
	ShiftOutByte(shiftRegData[0]); //U1

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); //Set latch to high
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
	uint8_t spiData = 0;
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

/*
 * Check if pedestrian button is pressed
 */
static bool IsButtonPressed(void) {
	return (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET);
}
/*
 * Check if a delay period has passed
 * startTime: WHen the delay started, in ticks
 * delayMS: Delay duration in ms
 * returns true if delay has passed
 */

static bool IsDelayPassed(TickType_t startTime, uint32_t delayMs) {
    TickType_t currentTime = xTaskGetTickCount();
    TickType_t delayTicks = pdMS_TO_TICKS(delayMs);
    return ((currentTime - startTime) >= delayTicks);
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

	//Set initial lights(RS 1.1)
	SetPedestrianLight(LIGHT_RED);
	SetCarLight(LIGHT_GREEN);

	//Initialize timers
	xBlinkStartTime = 0;
	xOrangeStartTime = 0;
	xWalkingStartTime = 0;

	//Enables shift register output, OE
	HAL_GPIO_WritePin(Enable_GPIO_Port, Enable_Pin, GPIO_PIN_RESET);
	//Release shift register reset, MR
	HAL_GPIO_WritePin(Reset_GPIO_Port, RESET_Pin, GPIO_PIN_SET);

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

void pedestrianCtrlTask(void *arguemnt) {
	PedestrianCtrl_Init(); //Initialize task

	//Task timing variables
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xTaskPeriod = pdMS_TO_TICKS(10); //10ms per task period

	TickType_t xLastBlinkTime = xTaskGetTickCount(); //Blinking timing variables

//Main task loop
	for(;;) {
		switch (currentState) {
		case STATE_INIT: //Initialize State, RS1.1
			SetPedestrianLight(LIGHT_RED);
			SetCarLight(LIGHT_GREEN);
			currentState = STATE_WAIT_BUTTON;
			break;

		case STATE_WAIT_BUTTON: //Button Pressed?
			if (IsButtonPressed()) {
				currentState = STATE_BLINKING_PED;
				xBlinkStartTime = xTaskGetTickCount();
				xLastBlinkTime = xTaskGetTickCount();
			}
			break;

		case STATE_BLINKING_PED:
			if(IsDelayPassed(xLastBlinkTime, config.toggleFreq)) { //RS 1.2: Toggle Pedestrian LED if Button Pressed = yes
				TogglePedestrianIndicator();
				xLastBlinkTime = xTaskGetTickCount();
			}
			if(IsDelayPassed(xBlinkStartTime, config.pedestrianDelay)) {
				currentState = STATE_CAR_ORANGE;
				SetPedestrianLight(LIGHT_OFF); //Stop blinking
				xOrangeStartTime = xTaskGetTickCount();
			}
			break;

		case STATE_CAR_ORANGE: //RS 1.6
			SetCarLight(LIGHT_ORANGE);

			if(IsDelayPassed(xOrangeStartTime, config.orangeDelay)) {
				currentState = STATE_PED_GREEN_CAR_RED;
				xWalkingStartTime = xTaskGetTickCount();
			}
			break;

		case STATE_PED_GREEN_CAR_RED: //RS 1.4 and RS 1.5
			SetPedestrianLight(LIGHT_GREEN);
			SetCarLight(LIGHT_RED);

			if(IsDelayPassed(xWalkingStartTime, config.walkingDelay)) {
				currentState = STATE_CAR_ORANGE_2;
				xOrangeStartTime = xTaskGetTickCount();
			}
			break;

		case STATE_CAR_ORANGE_2: //RS 1.6
			SetCarLight(LIGHT_ORANGE);
			if(IsDelayPassed(xOrangeStartTime, config.orangeDelay)) {
				currentState = STATE_INIT;
				cycleCounter++; //Counts the completed cycles
			}
			break;

		default: //Should ideally never happen
			currentState = STATE_INIT;
			break;
		}
		vTaskDelayUntil(&xLastWakeTime, xTaskPeriod); //Maintain fixed task period of 10ms
	}
}
