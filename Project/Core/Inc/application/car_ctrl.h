//Handles the car traffic signals
//Priority: normal

//Handles the car traffic signals
//Priority: normal
// car_ctrl.h should be:
#ifndef CAR_CTRL_H
#define CAR_CTRL_H

#include <stdbool.h>
#include <stdint.h>
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

void ProcessMainFSM(void);
void RequestDirectionChange(TrafficDirection_t fromDir, TrafficDirection_t toDir);
void SetDirectionLights(TrafficDirection_t direction, LightState_t state);

#ifdef __cplusplus
}
#endif

#endif
