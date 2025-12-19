/*
 * traffic_manager.c
 *
 *  Created on: Dec 4, 2025
 *      Author: karenli009
 */
#include "../Inc/config.h"
#include "main.h"
#include "../Inc/application/pedestrian_ctrl.h"
#include "../Inc/application/traffic_manager.h"
#include "../Inc/application/car_ctrl.h"
#include "main.h" //HAL definitions
#include "cmsis_os2.h" //osDelay (CMSIS-RTOS v2)
#include "../Inc/spi.h"
#include "usart.h"
#include <stdint.h>
#include <stdbool.h>

static PedestrianCrossing_t activePedestrian = PED_CROSSING_1;
static uint32_t lastCheckTime = 0;
static uint32_t coordinatorLastRun = 0;

void Task3_Coordinator(void) {
    uint32_t currentTime = osKernelGetTickCount();

    if ((currentTime - coordinatorLastRun) < 50) {
        return;
    }
    coordinatorLastRun = currentTime;

    // Get current pedestrian states
    CrossingState_t* crossing1State = GetCrossingState(PED_CROSSING_1);
    CrossingState_t* crossing2State = GetCrossingState(PED_CROSSING_2);

    bool crossing1Green = (crossing1State != NULL &&
                          crossing1State->state == STATE_PED_GREEN_CAR_RED);
    bool crossing2Green = (crossing2State != NULL &&
                          crossing2State->state == STATE_PED_GREEN_CAR_RED);

    // R3.3: Only one pedestrian crossing can be green at a time
    if (crossing1Green && crossing2Green) {
        if (activePedestrian == PED_CROSSING_1) {
            // Force crossing 2 to red
            SetSinglePedestrianLight(PED_CROSSING_2, LIGHT_RED);
            if (crossing2State) {
                crossing2State->state = STATE_INIT;
                crossing2State->isActive = false;
            }
            if (pedCrossingSemaphore != NULL) {
                osSemaphoreRelease(pedCrossingSemaphore);
            }
        } else {
            SetSinglePedestrianLight(PED_CROSSING_1, LIGHT_RED);
            if (crossing1State) {
                crossing1State->state = STATE_INIT;
                crossing1State->isActive = false;
            }
            // Release semaphore
            if (pedCrossingSemaphore != NULL) {
                osSemaphoreRelease(pedCrossingSemaphore);
            }
        }
    }
    if (crossing1Green) {
        activePedestrian = PED_CROSSING_1;
    } else if (crossing2Green) {
        activePedestrian = PED_CROSSING_2;
    }
}

bool Task3_CanPedestrianGoGreen(PedestrianCrossing_t crossing) {
    // Check if the other crossing is already green
    CrossingState_t* otherCrossing = (crossing == PED_CROSSING_1) ?
                                     GetCrossingState(PED_CROSSING_2) :
                                     GetCrossingState(PED_CROSSING_1);

    if (otherCrossing && otherCrossing->state == STATE_PED_GREEN_CAR_RED) {
        return false; // Other crossing is green, can't go green
    }

    return true; // Can go green
}

void Task3_ForcePedestrianRed(PedestrianCrossing_t crossing) {
    SetSinglePedestrianLight(crossing, LIGHT_RED);

    CrossingState_t* crossState = GetCrossingState(crossing);
    if (crossState) {
        crossState->state = STATE_INIT;
        crossState->isActive = false;
    }
}

PedestrianCrossing_t Task3_GetActivePedestrian(void) {
    return activePedestrian;
}
