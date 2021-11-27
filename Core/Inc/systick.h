/*
 * systick.h
 *
 *  Created on: Nov 24, 2021
 *      Author: krypton
 */

#ifndef INC_SYSTICK_H_
#define INC_SYSTICK_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

/*
 * sys_delay:	@resolution = 1 ms
 */
static void sys_delay(uint32_t ms)
{
	uint32_t delay = HAL_GetTick() + ms;
	while(HAL_GetTick() < delay);
}

#endif /* INC_SYSTICK_H_ */
