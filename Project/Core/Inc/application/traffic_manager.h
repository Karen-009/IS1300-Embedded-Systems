//The main coordinator: sync the pedestrian & vehicle logic
//Priority: high(osPriorityHigh)

#ifndef TRAFFIC_MANAGER_H
#define TRAFFIC_MANAGER_H

#include "config.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
	bool crossing1Green();
	bool crossing2Green();
	void Task3_EnforceSingleGreen(void);

	bool Task3_CanPedestrianGoGreen(PedestrianCrossing_t crossing);

	void Task3_CoordinatorInit(void);

	PedestrianCrossing_t Task3_GetActivePedestrian(void);

	void Task3_ForcePedestrianRed(PedestrianCrossing_t crossing);

	void Task3_Coordinator(void);

#ifdef __cplusplus
}
#endif

#endif /* TASK3_COORDINATOR_H */
