/*
 * trigen.h
 *
 *  Created on: Nov 17, 2021
 *      Author: InterImpv
 */

#ifndef INC_TRIGEN_H_
#define INC_TRIGEN_H_

#include <stdint.h>

enum GEN_ERR {
	GEN_EOK = 0,
	GEN_EARG = 1,
	GEN_EOVF = 2
};

enum GEN_MODE {
	GEN_8BIT = 0xff,
	GEN_12BIT = 0xfff
};

/* global generator struct */
typedef struct __trigen
{
	uint32_t step;

	uint32_t count_overflow;	// ~ maximum amplitude
	uint32_t count;				// current waveform counter value

	uint32_t tick_overflow;		// ~ frequency divider
	uint32_t tick;				// divider counter tick

	uint32_t norm_maximum;
} trigen;

/* generator functions */
enum GEN_ERR gen_init(trigen *generator, uint32_t amp_max, uint32_t step, uint32_t tick);

enum GEN_ERR gen_set_amp(trigen *generator, uint32_t amplitude);
enum GEN_ERR gen_set_tick(trigen *generator, uint32_t tick);
enum GEN_ERR gen_set_step(trigen *generator, uint32_t step);

enum GEN_ERR gen_count(trigen *generator);
enum GEN_ERR gen_tick(trigen *generator);

const uint32_t gen_get_norm_amp(trigen *generator);

#endif /* INC_TRIGEN_H_ */
