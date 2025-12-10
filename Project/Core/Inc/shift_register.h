#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

void ShiftReg_Init(void); //Initialize shift register control pins
void ShiftReg_WriteByte(uint8_t data); //Write a single byte to the first shift register
void ShiftReg_WriteChain(const uint8_t *data, uint8_t count); //Write data to all daisy-chained shift registers
void ShiftReg_SetPedestrianSimple(bool ped_red, bool ped_green, bool ped_blue); //Set pedestrian light states
void ShiftReg_SetAllLights(uint8_t reg1_data, uint8_t reg2_data, uint8_t reg3_data); //Set all traffic lights at once
void ShiftReg_ClearAll(void); //Turns off all LEDs
void ShiftReg_TestSequence(void); //For testing

#ifdef __cplusplus
}
#endif

#endif
