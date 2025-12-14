//Basic configuration and function definitions

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

	typedef enum {
		LIGHT_RED,
		LIGHT_ORANGE,
		LIGHT_GREEN,
		LIGHT_BLUE,
		LIGHT_OFF
	} LightState_t;


	typedef enum {
		STATE_INIT,
		STATE_CAR_GREEN_PED_RED,
		STATE_WAIT_BUTTON,
		STATE_BLINKING_PED,
		STATE_CAR_ORANGE,
		STATE_PED_GREEN_CAR_RED,
		STATE_CAR_ORANGE_2
	} PedestrianState_t;

	typedef struct {
		uint32_t toggleFreq; //Blinking freq.
		uint32_t pedestrianDelay; //Delay before crossing
		uint32_t walkingDelay; //Time for pedestrian walking
		uint32_t orangeDelay; // Orange light duration
	} TrafficConfig_t;

	static uint8_t shiftRegData[3] = {0, 0, 0}; //U1, U2, U3

	#define DEFAULT_TOGGLE_FREQ 500
	#define DEFAULT_PEDESTRIAN_DELAY 5000
	#define DEFAULT_WALKING_DELAY 10000
	#define DEFAULT_ORANGE_DELAY 2000


	//Bit definitions for shift register U1
	#define BIT_TL1_RED (1 << 0) //U1, Q0
	#define BIT_TL1_ORANGE (1 << 1) //U1, Q1
	#define BIT_TL1_GREEN (1 << 2) //U1, Q2

	#define BIT_PL1_RED (1 << 3) //U1, Q3
	#define BIT_PL1_GREEN (1 << 4) //U1, Q4
	#define BIT_PL1_BLUE (1 << 5) //U1, Q5



#endif
