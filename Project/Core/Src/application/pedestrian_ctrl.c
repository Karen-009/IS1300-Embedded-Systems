/*
Task 1: Pedestrian control
Included functions:
	int pdMS_to:TICKS();
 */
#include <string.h>
#include <stdbool.h>
#include "../Inc/application/pedestrian_ctrl.h"
#include "main.h" //HAL definitions
#include "cmsis_os2.h" //osDelay (CMSIS-RTOS v2)
#include "../Inc/config.h"
#include "../Inc/spi.h"
#include "usart.h"
#include "../Inc/application/traffic_manager.h"

CrossingState_t crossing1 = {0}; //State for pedestrian crossing
CrossingState_t crossing2 = {0}; //State for pedestrian crossing 2

static uint8_t cycleCounter = 0;

CrossingState_t* GetCrossingState(PedestrianCrossing_t crossing) {
    switch(crossing) {
        case PED_CROSSING_1: return &crossing1;
        case PED_CROSSING_2: return &crossing2;
        default: return NULL;
    }
}

void TogglePedestrianIndicator(PedestrianCrossing_t crossing) {
    static bool indicatorState1 = false;
    static bool indicatorState2 = false;
    static uint32_t lastToggle1 = 0;
    static uint32_t lastToggle2 = 0;

    uint32_t currentTime = osKernelGetTickCount();

    switch(crossing) {
        case PED_CROSSING_1:
            if ((currentTime - lastToggle1) >= config.toggleFreq) {
                indicatorState1 = !indicatorState1;
                if(indicatorState1) {
                    shiftRegData[0] |= BIT_PL_BLUE;
                } else {
                    shiftRegData[0] &= ~BIT_PL_BLUE;
                }
                UpdateShiftRegisters();
                lastToggle1 = currentTime;
            }
            break;

        case PED_CROSSING_2:
            if ((currentTime - lastToggle2) >= config.toggleFreq) {
                indicatorState2 = !indicatorState2;
                if(indicatorState2) {
                    shiftRegData[1] |= BIT_PL_BLUE;
                } else {
                    shiftRegData[1] &= ~BIT_PL_BLUE;
                }
                UpdateShiftRegisters();
                lastToggle2 = currentTime;
            }
            break;
    }
}
bool IsDelayPassed(uint32_t startTime, uint32_t delayMs) {
    uint32_t currentTime = osKernelGetTickCount();  // CMSIS function
    return ((currentTime - startTime) >= delayMs);  // Direct ms comparison
}

void SetCrossingCarLights(PedestrianCrossing_t crossing, LightState_t state) {
	switch(crossing) {
		case PED_CROSSING_1:
			SetCarLaneLight(1, state);
			SetCarLaneLight(4, state);
			break;

		case PED_CROSSING_2:
			SetCarLaneLight(2, state);
			SetCarLaneLight(3, state);
			break;
	}
}

void ProcessCrossingState(PedestrianCrossing_t crossing, CrossingState_t *cross) {
	switch (cross->state) {
		case STATE_INIT:
			SetSinglePedestrianLight(crossing, LIGHT_RED);
			cross->state = STATE_WAIT_BUTTON;
			cross->buttonPressed = false;
			cross->isActive = false;
			break;

		case STATE_WAIT_BUTTON:
			if(IsButtonPressed(crossing)) {
				cross->state = STATE_BLINKING_PED;
				cross->blinkStartTime = osKernelGetTickCount();
				cross->lastBlinkTime = osKernelGetTickCount();
				cross->buttonPressed = true;
				cross->isActive = true;

                // Set pedestrian request flag
                if (crossing == PED_CROSSING_1) {
                    osEventFlagsSet(pedEventFlags, PED_EVENT_REQUEST_1);
                } else {
                    osEventFlagsSet(pedEventFlags, PED_EVENT_REQUEST_2);
                }

				SetSinglePedestrianLight(crossing, LIGHT_RED);
				TogglePedestrianIndicator(crossing);
			}
			break;

		case STATE_BLINKING_PED:
    // Continue blinking while waiting
	    if (IsDelayPassed(cross->lastBlinkTime, config.toggleFreq)) {
	        TogglePedestrianIndicator(crossing);
	        cross->lastBlinkTime = osKernelGetTickCount();
	    }
	
	    if (IsDelayPassed(cross->blinkStartTime, config.pedestrianDelay)) {
	        if (!Task3_CanPedestrianGoGreen(crossing)) {
	            if (crossing == PED_CROSSING_1) {
	                osEventFlagsSet(pedEventFlags, PED_EVENT_TIMEOUT_1);
	            } else {
	                osEventFlagsSet(pedEventFlags, PED_EVENT_TIMEOUT_2);
	            }
	            break;
	        }

			if (AreCrossingCarLight(crossing, LIGHT_RED)) {
	            if (pedCrossingSemaphore != NULL &&
	                osSemaphoreAcquire(pedCrossingSemaphore, 0) == osOK) {
	                cross->state = STATE_PED_GREEN_CAR_RED;
	                cross->walkingStartTime = osKernelGetTickCount();
	                SetSinglePedestrianLight(crossing, LIGHT_GREEN);
	                
	                // Clear the request flag
	                if (crossing == PED_CROSSING_1) {
	                    osEventFlagsClear(pedEventFlags, PED_EVENT_REQUEST_1);
	                    osEventFlagsClear(pedEventFlags, PED_EVENT_TIMEOUT_1);
	                } else {
	                    osEventFlagsClear(pedEventFlags, PED_EVENT_REQUEST_2);
	                    osEventFlagsClear(pedEventFlags, PED_EVENT_TIMEOUT_2);
	                }
	            }
	            break; 
	        }
	        // Normal case: Car lights are GREEN or ORANGE, need transition
	        // Try to acquire pedestrian crossing semaphore
	        if (pedCrossingSemaphore != NULL &&
	            osSemaphoreAcquire(pedCrossingSemaphore, 0) == osOK) {
	            // Got semaphore - signal car control to change lights
	            cross->state = STATE_WAIT_CAR_TRANSITION;  // New intermediate state
	            cross->transitionStartTime = osKernelGetTickCount();
	            SetSinglePedestrianLight(crossing, LIGHT_OFF);
	            
	            // Signal car control to transition (GREEN → ORANGE → RED)
	            if (crossing == PED_CROSSING_1) {
	                // Crossing 1 affects horizontal car lanes (2 & 4)
	                osEventFlagsSet(dirHorizontalEvents, DIR_EVENT_REQUEST_STOP);
	            } else {
	                // Crossing 2 affects vertical car lanes (1 & 3)  
	                osEventFlagsSet(dirVerticalEvents, DIR_EVENT_REQUEST_STOP);
	            }
	            
	            // Clear flags
	            if (crossing == PED_CROSSING_1) {
	                osEventFlagsClear(pedEventFlags, PED_EVENT_REQUEST_1);
	                osEventFlagsClear(pedEventFlags, PED_EVENT_TIMEOUT_1);
	            } else {
	                osEventFlagsClear(pedEventFlags, PED_EVENT_REQUEST_2);
	                osEventFlagsClear(pedEventFlags, PED_EVENT_TIMEOUT_2);
	            }
	        }
	    }
	    break;

		case STATE_CAR_ORANGE_TO_RED: //Check if car lane now is red
			if(IsDelayPassed(cross->orangeStartTime, config.orangeDelay)) {
				cross->state = STATE_CAR_RED;
				SetCrossingCarLights(crossing, LIGHT_RED);
			}
			break;

		case STATE_CAR_RED:
			if(AreCrossingCarLight(crossing, LIGHT_RED)) {
				cross->state = STATE_PED_GREEN_CAR_RED;
				cross->walkingStartTime = osKernelGetTickCount();
				SetSinglePedestrianLight(crossing, LIGHT_GREEN);
			}
			break;

		case STATE_PED_GREEN_CAR_RED:
			if(IsDelayPassed(cross->walkingStartTime, config.walkingDelay)) {
				cross->state = STATE_CAR_ORANGE_2;
				cross->orangeStartTime = osKernelGetTickCount();
				SetSinglePedestrianLight(crossing, LIGHT_RED);
				SetCrossingCarLights(crossing, LIGHT_ORANGE);
			}
			break;

		case STATE_CAR_ORANGE_2:
		    if(IsDelayPassed(cross->orangeStartTime, config.orangeDelay)) {
		        SetCrossingCarLights(crossing, LIGHT_GREEN);
		        cross->state = STATE_INIT;
		        cross->isActive = false;
		        cycleCounter++;

		        /* R3.3: Release the pedestrian crossing semaphore */
		        if (pedCrossingSemaphore != NULL) {
		            osSemaphoreRelease(pedCrossingSemaphore);
		        }
		    }
		    break;
		
		case STATE_WAIT_CAR_ORANGE:
		    // Wait for car lights to become red
		    if (AreCrossingCarLight(crossing, LIGHT_RED)) {
		        cross->state = STATE_PED_GREEN_CAR_RED;
		        cross->walkingStartTime = osKernelGetTickCount();
		        SetSinglePedestrianLight(crossing, LIGHT_GREEN);
		    }
		    else if (IsDelayPassed(cross->waitStartTime, config.orangeDelay + 1000)) {
		        cross->state = STATE_PED_GREEN_CAR_RED;
		        cross->walkingStartTime = osKernelGetTickCount();
		        SetSinglePedestrianLight(crossing, LIGHT_GREEN);
		    }
		    break;
		
		default:
			cross->state = STATE_INIT;
			break;
	}
}

void PedestrianCtrl_Init(void) {
	//Initialize shift register data
	memset(&crossing1, 0, sizeof(CrossingState_t));
	memset(&crossing2, 0, sizeof(CrossingState_t));

	crossing1.state = STATE_INIT;
	crossing2.state = STATE_INIT;

	cycleCounter = 0;

	memset(shiftRegData, 0, sizeof(shiftRegData));
	    // 1. Release reset (MR# = HIGH = PC9)
	    HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_SET);

	    // 2. Enable outputs (OE# = LOW = PC7)
	    HAL_GPIO_WritePin(Enable_GPIO_Port, Enable_Pin, GPIO_PIN_RESET);

	    // 3. Clear all outputs
	    UpdateShiftRegisters();
	    osDelay(10);
	    // 4. Set initial lights
	    SetSinglePedestrianLight(PED_CROSSING_1, LIGHT_RED);
	    SetSinglePedestrianLight(PED_CROSSING_2, LIGHT_RED);
	    SetCarLaneLight(1, LIGHT_RED);//Vertival
	    SetCarLaneLight(2, LIGHT_GREEN);//Horizontal
	    SetCarLaneLight(3, LIGHT_GREEN); //Horizontal
	    SetCarLaneLight(4, LIGHT_RED);//Vertical

	    shiftRegData[0] &= ~BIT_PL_BLUE;
	    shiftRegData[1] &= ~BIT_PL_BLUE;

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
    case STATE_CAR_ORANGE_TO_RED:   return "CAR_ORANGE_TO_RED";
    case STATE_CAR_RED: 	return "CAR_RED";
	case STATE_PED_GREEN_CAR_RED:	return "PED_GREEN_CAR_RED";
	case STATE_CAR_ORANGE_2:	return "CAR_ORANGE_2";
	default:	return"UNKOWN";
	}
}

uint32_t PedestrianCtrl_GetCycleCount(void) {
	return cycleCounter;
}
/* Add after other includes in pedestrian_ctrl.c */

void pedestrianCtrlTask(void *argument) {


    //Task timing variables
    uint32_t xLastWakeTime = osKernelGetTickCount();
    const uint32_t xTaskPeriod = 10; //10ms per task period

        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

    	ProcessCrossingState(PED_CROSSING_1, &crossing1);
    	ProcessCrossingState(PED_CROSSING_2, &crossing2);

    	xLastWakeTime += xTaskPeriod;
    	osDelayUntil(xLastWakeTime);

}
