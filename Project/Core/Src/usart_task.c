#include "cmsis_os2.h"
#include "usart.h"
#include "config.h"
#include "main.h"

extern UART_HandleTypeDef huart2;

void UART(void *argument)
{
    uint8_t rx[4]; //the 4 values that can change
    uint8_t tx;
    uint16_t value;

    for (;;)
    {
        // 1. Wait for exactly 4 bytes (blocking, safe)
        HAL_UART_Receive(&huart2, rx, 4, HAL_MAX_DELAY);

        // 2. Decode value
        value = (rx[2] << 8) | rx[3];
        tx = 0x00; // default = reject

        // 3. Validate & apply
        switch (rx[0])
        {
            case 0x01: // toggleFreq
                if (value >= 10) {
                    config.toggleFreq = value;
                    tx = 0x01;
                }
                break;

            case 0x02: // pedestrianDelay
                if (value >= 1000) {
                    config.pedestrianDelay = value;
                    tx = 0x01;
                }
                break;

            case 0x03: // walkingDelay
                if (value >= 3000) {
                    config.walkingDelay = value;
                    tx = 0x01;
                }
                break;

            case 0x04: // orangeDelay
                if (value >= 1000) {
                    config.orangeDelay = value;
                    tx = 0x01;
                }
                break;

            default:
                tx = 0x00;
                break;
        }

        // 4. Send response
        HAL_UART_Transmit(&huart2, &tx, 1, HAL_MAX_DELAY);
    }
}
