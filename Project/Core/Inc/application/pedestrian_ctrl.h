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
	void TogglePedestrianIndicator(PedestrianCrossing_t crossing);
	bool IsDelayPassed(uint32_t startTime, uint32_t delayMs);
	void ProcessCrossingState(PedestrianCrossing_t crossing, CrossingState_t *cross);
	void PedestrianCtrl_Init(void);
	void PedestrianCtrl_SetConfig(const TrafficConfig_t *newConfig);
	const char* PedestrianCtrl_GetStateString(PedestrianState_t state);
	uint32_t PedestrianCtrl_GetCycleCount(void);
	void pedestrianCtrlTask(void *argument);


#ifdef __cplusplus
}
#endif

#endif
