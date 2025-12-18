#ifndef TRAFFIC_MANAGER_H
#define TRAFFIC_MANAGER_H

#include "config.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    // R3.3: Only one pedestrian crossing is green at a time
    void Task3_Coordinator(void);

    // Helper functions
    bool Task3_CanPedestrianGoGreen(PedestrianCrossing_t crossing);
    void Task3_ForcePedestrianRed(PedestrianCrossing_t crossing);
    PedestrianCrossing_t Task3_GetActivePedestrian(void);

    // Check current states
    bool crossing1Green(void);
    bool crossing2Green(void);

    // Right-turn logic (R3.5)
    bool CanCarTurnRight(TrafficLane_t lane);

#ifdef __cplusplus
}
#endif

#endif /* TRAFFIC_MANAGER_H */
