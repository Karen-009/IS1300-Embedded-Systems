#include "config.h"
#include "main.h"

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
