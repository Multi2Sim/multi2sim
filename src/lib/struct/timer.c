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

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <mhandle.h>

#include "debug.h"
#include "list.h"
#include "timer.h"


struct timer_t
{
	char *name;

	/* Timer running or stopped */
	enum timer_state_t state;

	/* Timer value for last instant when timer started/resumed */
	long long start_value;

	/* Accumulated value until last time the timer was stopped */
	long long total_value;
};




/*
 * Private Functions
 */

/* Return the current time in micro-seconds */
static long long timer_now(void)
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

struct timer_t *timer_create(char *name)
{
	struct timer_t *timer;

	/* Allocate */
	timer = calloc(1, sizeof(struct timer_t));
	if (!timer)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Initialize */
	timer->state = timer_state_stopped;
	timer->name = strdup(name ? name : "");
	if (!timer->name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Return */
	return timer;
}


void timer_free(struct timer_t *timer)
{
	free(timer->name);
	free(timer);
}


void timer_start(struct timer_t *timer)
{
	/* Timer already running */
	if (timer->state == timer_state_running)
		return;
	
	/* Start timer */
	timer->state = timer_state_running;
	timer->start_value = timer_now();
}


void timer_stop(struct timer_t *timer)
{
	long long ellapsed;

	/* Timer already stopped */
	if (timer->state == timer_state_stopped)
		return;

	/* Stop timer */
	ellapsed = timer_now() - timer->start_value;
	timer->state = timer_state_stopped;
	timer->total_value += ellapsed;
}


void timer_reset(struct timer_t *timer)
{
	timer->total_value = 0;
	timer->start_value = timer_now();
}


long long timer_get_value(struct timer_t *timer)
{
	long long ellapsed;

	/* Timer is stopped */
	if (timer->state == timer_state_stopped)
		return timer->total_value;

	/* Timer is running */
	ellapsed = timer_now() - timer->start_value;
	return timer->total_value + ellapsed;
}


enum timer_state_t timer_get_state(struct timer_t *timer)
{
	return timer->state;
}


char *timer_get_name(struct timer_t *timer)
{
	return timer->name;
}

