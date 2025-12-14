//Basic configuration and function definitions

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

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
