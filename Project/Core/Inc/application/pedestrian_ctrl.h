//Handles the pedestrian crossings (upper & lower)
//Priority: normal(osPriorityNormal)

#ifndef PEDESTRIAN_H
#define PEDESTRIAN_H

#include <FreeRTOSConfig.h>
#include <task.h>
#include <config.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif
	void PedestrianCtrlTask(void *argument); //Task function prototype, for FreeRTOS

	//Public API functions def.
	void PedestrinCtrl_Init(void);
	void PedestrainCtrl_Start(void);
	void PedestraianCtrl_SetConfig(const TrafficConfig_t *newConfig);

	//Debug functions
	const char* PedestrianCtrl_getStateString(PedestrianState_t state);
	uint32_t PedestrianCtrl_GetCycleCount(void);

#ifdef __cplusplus
}
#endif

#endif
