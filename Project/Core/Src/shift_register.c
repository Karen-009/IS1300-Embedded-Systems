#include "shift_register.h"
#include "main.h"
#include "gpio.h"

void ShiftReg_Init(void) {
	HAL_GPIO_WritePin(, SHIFT_RESET_Pin, GPIO_PIN_SET);
}
