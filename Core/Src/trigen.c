/*
 * trigen.c
 *
 *  Created on: Nov 17, 2021
 *      Author: InterImpv
 */

#include "trigen.h"

static const uint32_t MAX_AMPLITUDE = 0xfff;

/* functions */

/* - forward declarations */
enum GEN_ERR gen_set_amp(trigen *generator, uint32_t amp);
enum GEN_ERR gen_set_tick(trigen *generator, uint32_t tick);
enum GEN_ERR gen_set_step(trigen *generator, uint32_t step);

/* initializer */
enum GEN_ERR gen_init(trigen *generator, uint32_t amp_max, uint32_t step, uint32_t tick)
{
	generator->count = 0;
	generator->tick = 0;

	if (GEN_EOK != gen_set_amp(generator, amp_max)) {
		return GEN_EARG;
	}

	if (GEN_EOK != gen_set_step(generator, step)) {
		return GEN_EARG;
	}

	if (GEN_EOK != gen_set_tick(generator, tick)) {
		return GEN_EARG;
	}

	return GEN_EOK;
}

/* set methods */
enum GEN_ERR gen_set_amp(trigen *generator, uint32_t amplitude)
{
	switch(amplitude)
	{
		case GEN_8BIT:
			generator->count_overflow = GEN_8BIT;
			return GEN_EOK;
			break;

		case GEN_12BIT:
			generator->count_overflow = GEN_12BIT;
			return GEN_EOK;
			break;

		default:
			generator->count_overflow = 1;
			return GEN_EARG;
			break;
	};

	return GEN_EARG;
}

enum GEN_ERR gen_set_tick(trigen *generator, uint32_t tick)
{
	generator->tick_overflow = tick;
	return GEN_EOK;
}

enum GEN_ERR gen_set_step(trigen *generator, uint32_t step)
{
	if (step != 0 && step < MAX_AMPLITUDE) {
		generator->step = step;
		generator->norm_maximum = generator->step * ( generator->count_overflow / generator->step);
		return GEN_EOK;
	} else {
		return GEN_EARG;
	}
}

/* generation helper function */
enum GEN_ERR gen_count(trigen *generator)
{
	generator->count += generator->step;

	if (generator->count > generator->count_overflow) {
		generator->count = 0;
		return GEN_EOVF;
	}

	return GEN_EOK;
}

/* main generation function */
enum GEN_ERR gen_tick(trigen *generator)
{
	generator->tick++;

	if (generator->tick > generator->tick_overflow) {
		gen_count(generator);
		generator->tick = 0;
		return GEN_EOVF;
	}

	return GEN_EOK;
}

inline const uint32_t gen_get_norm_amp(trigen *generator)
{
	//uint32_t max_amplitude = generator->step * ( generator->count_overflow / generator->step);
	return (generator->count * generator->count_overflow) / generator->norm_maximum;
}
