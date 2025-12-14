#include "shift_register.h"
#include "main.h"
#include "gpio.h"

void ShiftReg_Init(void) {
	HAL_GPIO_WritePin(Reset_GPIO_Port, Reset_Pin, GPIO_PIN_SET);
}
