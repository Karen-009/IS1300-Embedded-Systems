//Basic configuration and function definitions

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h" //osDelay (CMSIS-RTOS v2)

	//All possible states of LEDs
	typedef enum {
		LIGHT_RED,
		LIGHT_ORANGE,
		LIGHT_GREEN,
		LIGHT_BLUE,
		LIGHT_OFF
	} LightState_t;

	//States for the state machine in task 1
	typedef enum {
		STATE_INIT,
		STATE_WAIT_BUTTON,
		STATE_BLINKING_PED,
		STATE_PED_GREEN_CAR_RED,
		STATE_CAR_ORANGE_TO_RED,
		STATE_CAR_RED,
		STATE_CAR_ORANGE_2,
		STATE_WAIT_CAR_ORANGE,
		STATE_WAIT_CAR_TRANSITION
	} PedestrianState_t;

	//States for pedestrian crossing in task 1
	typedef struct {
	    PedestrianState_t state;
	    uint32_t blinkStartTime;
	    uint32_t lastBlinkTime;
	    uint32_t orangeStartTime;
	    uint32_t walkingStartTime;
	    bool buttonPressed;
	    bool isActive;  // If the crossing still is in use
	} CrossingState_t;

	//Possible Delays & toggle
	typedef struct {
	    uint32_t greenDelay;
	    uint32_t redDelayMax;
	    uint32_t toggleFreq;
	    uint32_t pedestrianDelay;
	    uint32_t walkingDelay;
	    uint32_t orangeDelay;
	} TrafficConfig_t;

	//Define the two different pedestrian crossings
	typedef enum {
	    PED_CROSSING_1,
	    PED_CROSSING_2
	} PedestrianCrossing_t;

	//Define the two different car lanes
	typedef enum {
	    DIR_VERTICAL,
	    DIR_HORIZONTAL
	} TrafficDirection_t;

	//Define the 4 different car lanes
	typedef enum {
	    LANE_1 = 1,   // Traffic Light 1
	    LANE_2 = 2,   // Traffic Light 2
	    LANE_3 = 3,    // Traffic Light 3
	    LANE_4 = 4     // Traffic Light 4
	} TrafficLane_t;

	//States for the state machine in task 2
	typedef enum {
	    STATE_ACTIVE_V, //MAIN_STATE_FLOW_
	    STATE_ACTIVE_H, //MAIN_STATE_FLOW_H
	    STATE_TRANSITION_V, //MAIN_STATE_TRANSITION_V
	    STATE_TRANSITION_H //MAIN_STATE_TRANSITION_H
	} CarState_t; //MainState_t

	extern TrafficConfig_t config;

	//Definition of Delay & toggle values
	#define DEFAULT_TOGGLE_FREQ 500
	#define DEFAULT_PEDESTRIAN_DELAY 5000
	#define DEFAULT_WALKING_DELAY 10000
	#define DEFAULT_ORANGE_DELAY 2000
	#define DEFAULT_REDDELAYMAX 5000



	//Bit definitions for shift registers, TL for TL1, TL2 and TL4
	#define BIT_TL_RED (1 << 0)
	#define BIT_TL_ORANGE (1 << 1)
	#define BIT_TL_GREEN (1 << 2)

	#define BIT_TL3_RED (1 << 3)
	#define BIT_TL3_ORANGE (1 << 4)
	#define BIT_TL3_GREEN (1 << 5)

	#define BIT_PL_RED (1 << 3)
	#define BIT_PL_GREEN (1 << 4)
	#define BIT_PL_BLUE (1 << 5)

	void UpdateShiftRegisters(void);
	void SetCarLaneLight(int lane, LightState_t state);
	void SetSinglePedestrianLight(PedestrianCrossing_t crossing, LightState_t state);
	void SetAllCarLights(LightState_t state);
	bool IsCarPresent(TrafficLane_t lane);
	bool IsButtonPressed(PedestrianCrossing_t crossing);
	LightState_t GetCarLaneState(TrafficLane_t lane);
	bool AreCarsPresent(TrafficDirection_t direction);
	bool AreCrossingCarLights(TrafficDirection_t crossingDir, LightState_t state);
	bool CanPedestrianCross(PedestrianCrossing_t crossing);
	bool AreCrossingCarLight(PedestrianCrossing_t crossing, LightState_t state);
	bool CanTurnRight(TrafficLane_t lane);


	extern osEventFlagsId_t pedEventFlags;
	extern osEventFlagsId_t dirVerticalEvents;
	extern osEventFlagsId_t dirHorizontalEvents;

	#define DIR_EVENT_REQUEST_STOP   (1U << 0)
	#define DIR_EVENT_REQUEST_GO     (1U << 1)
	#define DIR_EVENT_IS_STOP        (1U << 2)

	#define PED_EVENT_REQUEST_1      (1U << 0)
	#define PED_EVENT_REQUEST_2      (1U << 1)
	#define PED_EVENT_TIMEOUT_1      (1U << 2)
	#define PED_EVENT_TIMEOUT_2      (1U << 3)

	extern uint8_t shiftRegData[3];
	extern uint32_t lastPressTime1;
	extern uint32_t lastPressTime2;
	extern const uint32_t debounceDelay;


	extern uint32_t currentRedWaitTime;
	extern uint32_t greenStartTime;

	extern osMutexId_t shiftRegMutex;
	extern osSemaphoreId_t pedCrossingSemaphore;
	extern osEventFlagsId_t carSensorEvents;

	extern LightState_t verticalLightState;
	extern LightState_t horizontalLightState;
	extern CarState_t currentCarState;

	extern CrossingState_t crossing1;
	extern CrossingState_t crossing2;

	//Adding new event flag to the sensor
	#define CAR_SENSOR_1_EVENT  (1U << 4)
	#define CAR_SENSOR_2_EVENT  (1U << 5)
	#define CAR_SENSOR_3_EVENT  (1U << 6)
	#define CAR_SENSOR_4_EVENT  (1U << 7)

#endif
