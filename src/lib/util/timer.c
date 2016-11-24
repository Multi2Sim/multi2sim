/*
 *  Libstruct
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <sys/time.h>

#include <lib/mhandle/mhandle.h>

#include "timer.h"


struct m2s_timer_t
{
	char *name;

	/* Timer running or stopped */
	enum m2s_timer_state_t state;

	/* Timer value for last instant when timer started/resumed */
	long long start_value;

	/* Accumulated value until last time the timer was stopped */
	long long total_value;
};




/*
 * Private Functions
 */

/* Return the current time in micro-seconds */
static long long m2s_timer_now(void)
{
	struct timeval tv;
	long long value;

	gettimeofday(&tv, NULL);
	value = (long long) tv.tv_sec * 1000000 + tv.tv_usec;
	return value;
}




/*
 * Public Functions
 */

struct m2s_timer_t *m2s_timer_create(char *name)
{
	struct m2s_timer_t *timer;

	/* Initialize */
	timer = xcalloc(1, sizeof(struct m2s_timer_t));
	timer->state = m2s_timer_state_stopped;
	timer->name = xstrdup(name ? name : "");

	/* Return */
	return timer;
}


void m2s_timer_free(struct m2s_timer_t *timer)
{
	free(timer->name);
	free(timer);
}


void m2s_timer_start(struct m2s_timer_t *timer)
{
	/* Timer already running */
	if (timer->state == m2s_timer_state_running)
		return;
	
	/* Start timer */
	timer->state = m2s_timer_state_running;
	timer->start_value = m2s_timer_now();
}


void m2s_timer_stop(struct m2s_timer_t *timer)
{
	long long ellapsed;

	/* Timer already stopped */
	if (timer->state == m2s_timer_state_stopped)
		return;

	/* Stop timer */
	ellapsed = m2s_timer_now() - timer->start_value;
	timer->state = m2s_timer_state_stopped;
	timer->total_value += ellapsed;
}


void m2s_timer_reset(struct m2s_timer_t *timer)
{
	timer->total_value = 0;
	timer->start_value = m2s_timer_now();
}


long long m2s_timer_get_value(struct m2s_timer_t *timer)
{
	long long ellapsed;

	/* Timer is stopped */
	if (timer->state == m2s_timer_state_stopped)
		return timer->total_value;

	/* Timer is running */
	ellapsed = m2s_timer_now() - timer->start_value;
	return timer->total_value + ellapsed;
}


enum m2s_timer_state_t m2s_timer_get_state(struct m2s_timer_t *timer)
{
	return timer->state;
}


char *m2s_timer_get_name(struct m2s_timer_t *timer)
{
	return timer->name;
}

