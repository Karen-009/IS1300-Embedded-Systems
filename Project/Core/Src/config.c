#include "../Inc/config.h"
#include "main.h"
#include "../Inc/application/pedestrian_ctrl.h"
#include "main.h" //HAL definitions
#include "cmsis_os2.h" //osDelay (CMSIS-RTOS v2)
#include "../Inc/spi.h"
#include "usart.h"
#include <stdint.h>
#include <stdbool.h>

	uint8_t shiftRegData[3] = {0, 0, 0}; //All 3 shift registers

	//Initial state of car lanes for Task 1
	LightState_t carLightState[4] = {
			LIGHT_GREEN, LIGHT_GREEN, LIGHT_GREEN, LIGHT_GREEN
	};

	uint32_t lastPressTime1 = 0;
	uint32_t lastPressTime2 = 0;
	const uint32_t debounceDelay = 50;

	//Values all the Delays & toggle
	TrafficConfig_t config = {
	    .greenDelay = DEFAULT_TOGGLE_FREQ,
	    .redDelayMax = DEFAULT_REDDELAYMAX,
	    .toggleFreq = DEFAULT_TOGGLE_FREQ,
	    .pedestrianDelay = DEFAULT_PEDESTRIAN_DELAY,
	    .orangeDelay = DEFAULT_ORANGE_DELAY,
	    .walkingDelay = DEFAULT_WALKING_DELAY
	};

	void UpdateShiftRegisters(void) {

	    HAL_StatusTypeDef status;
	    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);  // STCP = LOW
	    uint8_t spiData[3] = {shiftRegData[2], shiftRegData[1], shiftRegData[0]};
	    status = HAL_SPI_Transmit(&hspi3, spiData, 3, HAL_MAX_DELAY);

	    if(status != HAL_OK) {
	        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // SPI error indicator
	    }

	    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);  // STCP = HIGH
	    osDelay(1); // Small delay to ensure shift register latches
	}

	//Function to set the car lane LEDs
	void SetCarLaneLight(int lane, LightState_t state) {
		    carLightState[lane-1] = state;

			switch(lane) {
			case LANE_1: //Shift register 1
				shiftRegData[0] &= ~(BIT_TL_RED | BIT_TL_ORANGE | BIT_TL_GREEN);
				switch(state) {
					case LIGHT_RED:		shiftRegData[0] |= BIT_TL_RED;		break;
					case LIGHT_ORANGE:	shiftRegData[0] |= BIT_TL_ORANGE;	break;
					case LIGHT_GREEN:	shiftRegData[0] |= BIT_TL_GREEN;	break;
					default:	break;
				}
				break;

			case LANE_2: //Shift register 2
				shiftRegData[1] &= ~(BIT_TL_RED | BIT_TL_ORANGE | BIT_TL_GREEN);
				switch(state) {
					case LIGHT_RED:		shiftRegData[1] |= BIT_TL_RED;		break;
					case LIGHT_ORANGE:	shiftRegData[1] |= BIT_TL_ORANGE;	break;
					case LIGHT_GREEN:	shiftRegData[1] |= BIT_TL_GREEN;	break;
					default:	break;
				}
				carLightState[lane-1] = state;

				break;
			case LANE_3: //Shift register 3
				shiftRegData[2] &= ~(BIT_TL3_RED | BIT_TL3_ORANGE | BIT_TL3_GREEN);
				switch(state) {
					case LIGHT_RED:		shiftRegData[2] |= BIT_TL3_RED;		break;
					case LIGHT_ORANGE:	shiftRegData[2] |= BIT_TL3_ORANGE;	break;
					case LIGHT_GREEN:	shiftRegData[2] |= BIT_TL3_GREEN;	break;
					default:	break;
				}
				carLightState[2] = state;
				break;

			case LANE_4: //Shift register 3
				shiftRegData[2] &= ~(BIT_TL_RED | BIT_TL_ORANGE | BIT_TL_GREEN);
				switch(state) {
					case LIGHT_RED:		shiftRegData[2] |= BIT_TL_RED;		break;
					case LIGHT_ORANGE:	shiftRegData[2] |= BIT_TL_ORANGE;	break;
					case LIGHT_GREEN:	shiftRegData[2] |= BIT_TL_GREEN;	break;
					default:	break;
				}
				carLightState[3] = state;
				break;
			}
			UpdateShiftRegisters();
		}

	//Function to control the two pedestrian lights
	void SetSinglePedestrianLight(PedestrianCrossing_t crossing, LightState_t state) {
		    if (shiftRegMutex != NULL) {
		        osMutexAcquire(shiftRegMutex, osWaitForever);
		    }

		    switch(crossing) {
		    case PED_CROSSING_1:
		        shiftRegData[0] &= ~(BIT_PL_RED | BIT_PL_GREEN | BIT_PL_BLUE);
				switch(state) {
					case LIGHT_RED:		shiftRegData[0] |= BIT_PL_RED;		break;
					case LIGHT_GREEN:	shiftRegData[0] |= BIT_PL_GREEN;	break;
					case LIGHT_BLUE:	shiftRegData[0] |= BIT_PL_BLUE;		break;
					case LIGHT_OFF:											break;
					default:	break;
				}
				break;
			case PED_CROSSING_2:
		        shiftRegData[1] &= ~(BIT_PL_RED | BIT_PL_GREEN | BIT_PL_BLUE);
		        switch(state) {
					case LIGHT_RED:		shiftRegData[1] |= BIT_PL_RED;		break;
					case LIGHT_GREEN:	shiftRegData[1] |= BIT_PL_GREEN;	break;
					case LIGHT_BLUE:	shiftRegData[1] |= BIT_PL_BLUE;		break;
					case LIGHT_OFF:											break;
					default:	break;
		        }
		        break;
		    }

		    UpdateShiftRegisters();

		    if (shiftRegMutex != NULL) {
		        osMutexRelease(shiftRegMutex);
		    }
		}
	//Function to set all the car lane LEDs to the same state, task 1
	void SetAllCarLights(LightState_t state) {
		SetCarLaneLight(1, state);
		SetCarLaneLight(2, state);
		SetCarLaneLight(3, state);
		SetCarLaneLight(4, state);
	}

	bool IsCarPresent(TrafficLane_t lane) {
	    switch(lane) {
	        case LANE_1:
	            return (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_SET);
	        case LANE_2:
	            return (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_SET);
	        case LANE_3:
	            return (HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin) == GPIO_PIN_SET);
	        case LANE_4:
	            return (HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == GPIO_PIN_SET);
	        default:
	            return false;
	    }
	}

	bool IsButtonPressed(PedestrianCrossing_t crossing) {
	    uint32_t currentTime = osKernelGetTickCount();

	    switch(crossing) {
	    	case PED_CROSSING_1:
	    		if((HAL_GPIO_ReadPin(PA15_GPIO_Port, PA15_Pin) == GPIO_PIN_RESET) && ((currentTime - lastPressTime1) > debounceDelay)) {
	    			lastPressTime1 = currentTime;
	    			return true;
	    		}
	    		break;
	    	case PED_CROSSING_2:
	    		if((HAL_GPIO_ReadPin(PB7_GPIO_Port, PB7_Pin) == GPIO_PIN_RESET) && ((currentTime - lastPressTime2) > debounceDelay)) {
	    			lastPressTime2 = currentTime;
	    			return true;
	    		}
	    		break;
	        default:
	            return false;
	    }
	    return false;
	}

	// Get current light state for a lane
	LightState_t GetCarLaneState(TrafficLane_t lane) {
	    if (lane >= 1 && lane <= LANE_4) {
	        return carLightState[lane - 1]; // Using the shared array
	    }
	    return LIGHT_OFF;
	}

	// Check physical car presence at sensors
	bool AreCarsPresent(TrafficDirection_t direction) {
	    switch(direction) {
	        case DIR_VERTICAL:
	            return IsCarPresent(LANE_1) || IsCarPresent(LANE_3);
	        case DIR_HORIZONTAL:
	            return IsCarPresent(LANE_2) || IsCarPresent(LANE_4);
	        default:
	            return false;
	    }
	}

	// Check if car lights for a crossing direction are in specific state
	bool AreCrossingCarLights(TrafficDirection_t crossingDir, LightState_t state) {
	    switch(crossingDir) {
	        case DIR_VERTICAL:
	            return (GetCarLaneState(LANE_1) == state && GetCarLaneState(LANE_3) == state);  // Fixed: LANE_3, not LANE_2
	        case DIR_HORIZONTAL:
	            return (GetCarLaneState(LANE_2) == state && GetCarLaneState(LANE_4) == state);
	        default:
	            return false;
	    }
	}

	//Check the state of the car lanes to see if it is safe for pedestrian to cross
	bool CanPedestrianCross(PedestrianCrossing_t crossing) {
		switch(crossing) {
			case PED_CROSSING_1:
	            return AreCrossingCarLights(DIR_VERTICAL, LIGHT_RED);

			case PED_CROSSING_2:
	            return AreCrossingCarLights(DIR_HORIZONTAL, LIGHT_RED);
			default:
				return false;
		}
	}

	// Check if car lanes for a crossing are in a specific state
	bool AreCrossingCarLight(PedestrianCrossing_t crossing, LightState_t state) {
	    switch(crossing) {
	        case PED_CROSSING_1:
	            return AreCrossingCarLights(DIR_VERTICAL, state);
	        case PED_CROSSING_2:
	            return AreCrossingCarLights(DIR_HORIZONTAL, state);
	        default:
	            return false;
	    }
	}

	osEventFlagsId_t pedEventFlags;
	osEventFlagsId_t dirVerticalEvents = NULL;
	osEventFlagsId_t dirHorizontalEvents = NULL;

	uint32_t currentRedWaitTime = 0;
	uint32_t greenStartTime = 0;

