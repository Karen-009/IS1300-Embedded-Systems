//Handles the pedestrian crossings (upper & lower)
//Priority: normal(osPriorityNormal)

#ifndef PEDESTRIAN_CTRL_H
#define PEDESTRIAN_CTRL_H

#include <FreeRTOSConfig.h>
#include <FreeRTOS.h>
#include <task.h>
#include "./config.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif
	void pedestrianCtrlTask(void *argument); //Task function prototype, for FreeRTOS

	//Public API functions def.
	void PedestrianCtrl_Init(void);
	void PedestrianCtrl_SetConfig(const TrafficConfig_t *newConfig);

	//Debug functions
	const char* PedestrianCtrl_GetStateString(PedestrianState_t state);
	uint32_t PedestrianCtrl_GetCycleCount(void);

#ifdef __cplusplus
}
#endif

#endif
