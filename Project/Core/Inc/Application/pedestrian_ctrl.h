//Handles the pedestrian crossings (upper & lower)
//Priority: normal(osPriorityNormal)

#ifndef PEDESTRIAN_H
#define PEDESTRIAN_H

#include <stdbool.h>
#include <FreeRTOSConfig.h>
#include <stm32l4xx_hal_conf.h>
#include <cmsis_os.h>

//Type definitions
typedef enum {
	STAT_INIT,
	STAT_CAR_GREEN_PED_RED,
	STATE_BLINKING_PED,
	STATE_CAR_ORANGE,
	STATE_PED_GREEN_CAR_RED,
} TrafficState_P;

//Function declarations

void pedestrian_ctrl_task(void);
void setPedestrianLight(char color[]);
void setCarLight(char color[]);
bool isButtonPressed(void);
void togglePedestrianIndicator(void);
bool isDelayPassed(int x, int y);
void


#endif
