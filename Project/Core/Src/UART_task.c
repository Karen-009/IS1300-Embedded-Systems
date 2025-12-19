
#include "UART_task.h"
#include "stm32l4xx_it.h"
#include "config.h"
#include "main.h"


extern UART_HandleTypeDef huart2;

uint8_t list [4];
uint16_t values;
uint8_t i [1];

void Uart_Init(){
	HAL_UART_Receive_IT(&huart2, list, 4);

}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
	if(huart != &huart2) return

	UNUSED(huart);

	// Route the value to the correct struct member in config.h

	values = (list[2] << 8) | list [3];


	switch(list[0]) {
	            case 0x01:
	            	if (values >= 50 && values <= 3000){
	            			config.toggleFreq = values;
	            			i[0] = 0x11; //visa att den är på
	            	}
	            	break;

	            case 0x02:
	            	  if (values >= 50 && values <= 3000){
	            	        config.pedestrianDelay = values;
	            	        i[0] = 0x11; //visa att den är på
	            	  }
	            	  break;

	            case 0x03:
	            	  if (values >= 50 && values <= 3000){
	            	       config.orangeDelay = values;
	            	       i[0] = 0x11; //visa att den är på
	            	  }
	            	  break;

	            case 0x04:
	            	  if (values >= 50 && values <= 3000){
	            	       config.walkingDelay = values;
	            	       i[0] = 0x11; //visa att den är på
	            	  }
	            	  break;

	            case 0x05:
	            	  if (values >= 50 && values <= 3000){
	            	        config.greenDelay = values;
	            	        i[0] = 0x11; //visa att den är på
	            	  }
	            	  break;

	            case 0x06:
	            	 if (values >= 50 && values <= 3000){
	            	        config.orangeDelay = values;
	            	        i[0] = 0x11; //visa att den är på
	            	 }
	            	 break;
	            default:
	            	i[0] = 0x00;
	            	break;
	        }
	HAL_UART_Transmit_IT(&huart2, i, 1);
	HAL_UART_Receive_IT(&huart2, list, 4);




  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_UART_RxCpltCallback can be implemented in the user file.
   */


}

