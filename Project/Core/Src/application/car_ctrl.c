/*
 * Karen implemented the whole file
 */

#include "../Inc/application/car_ctrl.h"
#include "../Inc/main.h" //HAL definitions
#include "cmsis_os2.h" //osDelay (CMSIS-RTOS v2)
#include <string.h>
#include <stdint.h>
#include "../Inc/config.h"
#include <stdbool.h>
#include "../Inc/spi.h"
#include "usart.h"

LightState_t carLightStates[4];

	void SetDirectionLights(TrafficDirection_t direction, LightState_t state) {
		switch(direction) {
			case DIR_VERTICAL:
				SetCarLaneLight(LANE_1, state);
				SetCarLaneLight(LANE_4, state);
				break;
			case DIR_HORIZONTAL:
				SetCarLaneLight(LANE_2, state);
				SetCarLaneLight(LANE_3, state);
				break;
		}
	}


	void RequestDirectionChange(TrafficDirection_t fromDir, TrafficDirection_t toDir) {
	    if (fromDir == DIR_VERTICAL) {
	        osEventFlagsSet(dirVerticalEvents, DIR_EVENT_REQUEST_STOP);
	    } else {
	        osEventFlagsSet(dirHorizontalEvents, DIR_EVENT_REQUEST_STOP);
	    }

	    // Reset the green timer for the new direction
	    greenStartTime = osKernelGetTickCount();
	}

	void ProcessMainFSM(void) {
	    // Check pedestrian requests from Task 1
	    uint32_t pedEvents = osEventFlagsGet(pedEventFlags);
	    bool pedRequest1 = (pedEvents & PED_EVENT_REQUEST_1) != 0;
	    bool pedRequest2 = (pedEvents & PED_EVENT_REQUEST_2) != 0;

	    // Check pedestrian timeout flags
	    bool pedTimeout1 = (pedEvents & PED_EVENT_TIMEOUT_1) != 0;
	    bool pedTimeout2 = (pedEvents & PED_EVENT_TIMEOUT_2) != 0;


	    switch (currentCarState) {
	        case STATE_ACTIVE_V:
	        	if (pedRequest2) { // Crossing 2 wants to cross vertical street
	        	        RequestDirectionChange(DIR_VERTICAL, DIR_HORIZONTAL);
	        	        currentCarState = STATE_TRANSITION_H;
	        	        osEventFlagsClear(pedEventFlags, PED_EVENT_REQUEST_2);
	        	        break;
	        	   }

	        	if (pedTimeout2) {
	        	    RequestDirectionChange(DIR_VERTICAL, DIR_HORIZONTAL);
	        	    currentCarState = STATE_TRANSITION_H;
	        	    osEventFlagsClear(pedEventFlags, PED_EVENT_TIMEOUT_2);

	        	    // Also clear any pending request
	        	    osEventFlagsClear(pedEventFlags, PED_EVENT_REQUEST_2);
	        	    break;
	        	}

	            // R2.4: Auto transition after greenDelay if no cars in vertical direction
	            if (!AreCarsPresent(DIR_VERTICAL) &&
	                (osKernelGetTickCount() - greenStartTime >= config.greenDelay)) {
	                RequestDirectionChange(DIR_VERTICAL, DIR_HORIZONTAL);
	                currentCarState = STATE_TRANSITION_H;
	                currentRedWaitTime = 0; // Reset red wait timer
	                break;
	            }

	            // R2.6: If cars present in both directions, transition after redDelayMax
	            if (AreCarsPresent(DIR_VERTICAL) && AreCarsPresent(DIR_HORIZONTAL)) {
	                if (currentRedWaitTime == 0) {
	                    currentRedWaitTime = osKernelGetTickCount(); // Start timing
	                }
	                if (osKernelGetTickCount() - currentRedWaitTime >= config.redDelayMax) {
	                    RequestDirectionChange(DIR_VERTICAL, DIR_HORIZONTAL);
	                    currentCarState = STATE_TRANSITION_H;
	                    currentRedWaitTime = 0; // Reset for next cycle
	                }
	            } else {
	                currentRedWaitTime = 0; // Reset if condition no longer met
	            }

	            // R2.7: Immediate transition if opposite has cars and ours is free
	            if (!AreCarsPresent(DIR_VERTICAL) && AreCarsPresent(DIR_HORIZONTAL)) {
	                RequestDirectionChange(DIR_VERTICAL, DIR_HORIZONTAL);
	                currentCarState = STATE_TRANSITION_H;
	                currentRedWaitTime = 0; // Reset red wait timer
	                break;
	            }

	            // Check if pedestrian just pressed button (not timeout yet)
	            if (pedRequest2) {
	                // Acknowledge the request (but don't transition yet)
	                osEventFlagsClear(pedEventFlags, PED_EVENT_REQUEST_2);
	                // The timeout will be handled by pedestrian task
	            }

	            break;

	        case STATE_TRANSITION_H:
	            // Waiting for vertical direction to complete stop transition
	            if (osEventFlagsGet(dirVerticalEvents) & DIR_EVENT_IS_STOP) {
	                // Now start horizontal direction
	                osEventFlagsSet(dirHorizontalEvents, DIR_EVENT_REQUEST_GO);
	                currentCarState = STATE_ACTIVE_H;
	                greenStartTime = osKernelGetTickCount();
	                currentRedWaitTime = 0; // Reset for horizontal flow
	            }
	            break;

	        case STATE_ACTIVE_H:
	            // Horizontal direction has green, vertical has red
	            // R1.3: Pedestrian button pressed more than pedestrianDelay ago
	        	if (pedRequest1) { // Crossing 1 wants to cross horizontal street
	        	        RequestDirectionChange(DIR_HORIZONTAL, DIR_VERTICAL);
	        	        currentCarState = STATE_TRANSITION_V;
	        	        osEventFlagsClear(pedEventFlags, PED_EVENT_REQUEST_1);
	        	        break;
	        	    }

	        	    // R1.3: Pedestrian button pressed more than pedestrianDelay ago
	        	if (pedTimeout1) {
	        	    RequestDirectionChange(DIR_HORIZONTAL, DIR_VERTICAL);
	        	    currentCarState = STATE_TRANSITION_V;
	        	    osEventFlagsClear(pedEventFlags, PED_EVENT_TIMEOUT_1);

	        	    osEventFlagsClear(pedEventFlags, PED_EVENT_REQUEST_1);
	        	    break;
	        	}

	            // R2.4: Auto transition after greenDelay if no cars in horizontal direction
	            if (!AreCarsPresent(DIR_HORIZONTAL) &&
	                (osKernelGetTickCount() - greenStartTime >= config.greenDelay)) {
	                RequestDirectionChange(DIR_HORIZONTAL, DIR_VERTICAL);
	                currentCarState = STATE_TRANSITION_V;
	                currentRedWaitTime = 0;
	                break;
	            }

	            // R2.6: If cars present in both directions, transition after redDelayMax
	            if (AreCarsPresent(DIR_VERTICAL) && AreCarsPresent(DIR_HORIZONTAL)) {
	                if (currentRedWaitTime == 0) {
	                    currentRedWaitTime = osKernelGetTickCount();
	                }
	                if (osKernelGetTickCount() - currentRedWaitTime >= config.redDelayMax) {
	                    RequestDirectionChange(DIR_HORIZONTAL, DIR_VERTICAL);
	                    currentCarState = STATE_TRANSITION_V;
	                    currentRedWaitTime = 0;
	                }
	            } else {
	                currentRedWaitTime = 0;
	            }

	            // R2.7: Immediate transition if opposite has cars and ours is free
	            if (!AreCarsPresent(DIR_HORIZONTAL) && AreCarsPresent(DIR_VERTICAL)) {
	                RequestDirectionChange(DIR_HORIZONTAL, DIR_VERTICAL);
	                currentCarState = STATE_TRANSITION_V;
	                currentRedWaitTime = 0;
	                break;
	            }

	            if (pedRequest1) {
	                osEventFlagsClear(pedEventFlags, PED_EVENT_REQUEST_1);
	            }

	            break;

	        case STATE_TRANSITION_V:
	            // Waiting for horizontal direction to complete stop transition
	            if (osEventFlagsGet(dirHorizontalEvents) & DIR_EVENT_IS_STOP) {
	                osEventFlagsSet(dirVerticalEvents, DIR_EVENT_REQUEST_GO); // Now start vertical direction
	                currentCarState = STATE_ACTIVE_V;
	                greenStartTime = osKernelGetTickCount();
	                currentRedWaitTime = 0; // Reset for vertical flow
	            }
	            break;

	        default:
	        	currentCarState = STATE_ACTIVE_V;
	            SetDirectionLights(DIR_VERTICAL, LIGHT_GREEN);
	            SetDirectionLights(DIR_HORIZONTAL, LIGHT_RED);
	            greenStartTime = osKernelGetTickCount();
	            break;
	    }
	}


	void ProcessVerticalDirection(void) {
	    uint32_t events = osEventFlagsGet(dirVerticalEvents);

	    switch(verticalLightState) {
	        case LIGHT_GREEN:
	            if (events & DIR_EVENT_REQUEST_STOP) {
	                verticalLightState = LIGHT_ORANGE;
	                SetDirectionLights(DIR_VERTICAL, LIGHT_ORANGE);
	                osDelay(config.orangeDelay);
	                verticalLightState = LIGHT_RED;
	                SetDirectionLights(DIR_VERTICAL, LIGHT_RED);
	                osEventFlagsSet(dirVerticalEvents, DIR_EVENT_IS_STOP);
	                osEventFlagsClear(dirVerticalEvents, DIR_EVENT_REQUEST_STOP);
	            }
	            break;

	        case LIGHT_RED:
				if (events & DIR_EVENT_REQUEST_STOP) {
			        osEventFlagsSet(dirVerticalEvents, DIR_EVENT_IS_STOP);
			        osEventFlagsClear(dirVerticalEvents, DIR_EVENT_REQUEST_STOP);
			    }
	            if (events & DIR_EVENT_REQUEST_GO) {
	                verticalLightState = LIGHT_ORANGE;
	                SetDirectionLights(DIR_VERTICAL, LIGHT_ORANGE);
	                osDelay(config.orangeDelay);
	                verticalLightState = LIGHT_GREEN;
	                SetDirectionLights(DIR_VERTICAL, LIGHT_GREEN);
	                osEventFlagsClear(dirVerticalEvents, DIR_EVENT_IS_STOP);
	                osEventFlagsClear(dirVerticalEvents, DIR_EVENT_REQUEST_GO);
	            }
	            break;

	        case LIGHT_ORANGE:
	            break;
	        default:
	        	break;
	    }
	}

	void ProcessHorizontalDirection(void) {
	    uint32_t events = osEventFlagsGet(dirHorizontalEvents);

	    switch(horizontalLightState) {
	        case LIGHT_GREEN:
	            if (events & DIR_EVENT_REQUEST_STOP) {
	                horizontalLightState = LIGHT_ORANGE;
	                SetDirectionLights(DIR_HORIZONTAL, LIGHT_ORANGE);
	                osDelay(config.orangeDelay);
	                horizontalLightState = LIGHT_RED;
	                SetDirectionLights(DIR_HORIZONTAL, LIGHT_RED);
	                osEventFlagsSet(dirHorizontalEvents, DIR_EVENT_IS_STOP);
	                osEventFlagsClear(dirHorizontalEvents, DIR_EVENT_REQUEST_STOP);
	            }
	            break;
	        case LIGHT_ORANGE:
	            break;
	        case LIGHT_RED:
				if (events & DIR_EVENT_REQUEST_STOP) {
			        // Lights are already red - immediately signal IS_STOP
			        osEventFlagsSet(dirVerticalEvents, DIR_EVENT_IS_STOP);
			        osEventFlagsClear(dirVerticalEvents, DIR_EVENT_REQUEST_STOP);
			    }
	            if (events & DIR_EVENT_REQUEST_GO) {
	            	horizontalLightState = LIGHT_ORANGE;
	            	SetDirectionLights(DIR_HORIZONTAL, LIGHT_ORANGE);
	                osDelay(config.orangeDelay);
	                horizontalLightState = LIGHT_GREEN;
	                SetDirectionLights(DIR_HORIZONTAL, LIGHT_GREEN);
	                osEventFlagsClear(dirHorizontalEvents, DIR_EVENT_IS_STOP);
	                osEventFlagsClear(dirHorizontalEvents, DIR_EVENT_REQUEST_GO);
	            }
	            break;
	        default:
	        	break;
	    }
	}

	void carCtrlTask(void *argument) {
	    uint32_t xLastWakeTime = osKernelGetTickCount();
	    const uint32_t xTaskPeriod = 10;

        currentCarState = STATE_ACTIVE_V;
        verticalLightState = LIGHT_GREEN;
        horizontalLightState = LIGHT_RED;
        SetDirectionLights(DIR_VERTICAL, LIGHT_GREEN);
        SetDirectionLights(DIR_HORIZONTAL, LIGHT_RED);

        greenStartTime = osKernelGetTickCount();

	    for(;;) {
	    	ProcessVerticalDirection();
	    	ProcessHorizontalDirection();
	    	ProcessMainFSM();

	        xLastWakeTime += xTaskPeriod;
	        osDelayUntil(xLastWakeTime);
	    }
	}
