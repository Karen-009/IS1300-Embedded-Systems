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

void Task3_Coordinator(void) {
    uint32_t currentTime = osKernelGetTickCount();
    static bool coordinatorInitialized = false;

    // Check only every 100ms (save CPU)
    if ((currentTime - lastCheckTime) < 100) {
        return;
    }
    lastCheckTime = currentTime;

    // Check current pedestrian states using YOUR EXISTING variables
    bool crossing1Green = (crossing1.state == STATE_PED_GREEN_CAR_RED);
    bool crossing2Green = (crossing2.state == STATE_PED_GREEN_CAR_RED);

    // R3.3: Only one can be green
    if (crossing1Green && crossing2Green) {
        // Both think they're green - fix it
        if (activePedestrian == PED_CROSSING_1) {
            // Force crossing 2 to red using YOUR EXISTING function
            SetSinglePedestrianLight(PED_CROSSING_2, LIGHT_RED);
            crossing2.state = STATE_INIT;
            crossing2.isActive = false;
        } else {
            SetSinglePedestrianLight(PED_CROSSING_1, LIGHT_RED);
            crossing1.state = STATE_INIT;
            crossing1.isActive = false;
        }
    } else if (crossing1Green) {
        activePedestrian = PED_CROSSING_1;
    } else if (crossing2Green) {
        activePedestrian = PED_CROSSING_2;
    }
}
