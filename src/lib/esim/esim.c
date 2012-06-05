/*
 *  Libesim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include <debug.h>
#include <esim.h>
#include <heap.h>
#include <linked-list.h>
#include <list.h>
#include <mhandle.h>


/* Number of in-flight events before a warning is shown. */
#define ESIM_OVERLOAD_EVENTS  10000


static char *err_esim_finalization =
	"\tThe finalization process of the event-driven simulation is trying to\n"
	"\tempty the event heap by scheduling all pending events. If the number of\n"
	"\tscheduled events during this process is too high, it could mean that\n"
	"\tsome event is recursively scheduling other events, avoiding correct\n"
	"\tfinalization of the simulation. Please contact development@multi2sim.org\n"
	"\tto report this error.\n";

static char *err_esim_overload =
	"\tAn excessive number of events are currently in-flight in the event-\n"
	"\tdriven simulation library. This is probably the result of a timing\n"
	"\tmodel uncontrollably issuing events (typically a processor pipeline\n"
	"\tissuing memory accesses without checking for a limit in the number\n"
	"\tof in-flight accesses).\n"
	"\tIf you still believe that this is an expected number of in-flight\n"
	"\tevents, please increase the value of macro ESIM_OVERLOAD_EVENTS to\n"
	"\tavoid this warning.\n";


static int curr_event = 0;
static int ESIM_EV_INVALID;
static int esim_lock_schedule = 0;

/* Flag indicating whether the overload warning has been already shown. This
 * will prevent it to be shown more than once during the execution. */
static int esim_overload_shown = 0;

long long esim_cycle = 1;
int ESIM_EV_NONE;

static struct list_t *esim_event_handler_list;
static struct heap_t *esim_event_heap;
static struct linked_list_t *esim_end_event_list;


struct esim_event_t
{
	int event;
	void *data;
};




/*
 * Private Functions
 */

static void esim_drain_heap(void)
{
	int count = 0;

	struct esim_event_t *event;
	esim_event_handler_t handler;

	long long cycle;

	/* Extract all elements from heap */
	while (1)
	{
		/* Extract event */
		cycle = heap_extract(esim_event_heap, (void **) &event);
		if (heap_error(esim_event_heap))
			break;

		/* Process it */
		count++;
		esim_cycle = cycle;
		handler = list_get(esim_event_handler_list, event->event);
		assert(handler);
		handler(event->event, event->data);

		/* Free event */
		free(event);

		/* Issue a warning if more than 10k events are processed. This could
		 * mean that the simulator is recursively issuing events, incurring
		 * an infinite loop execution in this code. */
		if (count == 10000)
			warning("%s: too many finalization events.\n%s",
				__FUNCTION__, err_esim_finalization);
	}
}




/*
 * Public Functions
 */


void esim_init()
{
	/* Create structures */
	esim_event_handler_list = list_create();
	esim_event_heap = heap_create(20);
	esim_end_event_list = linked_list_create();

	/* Register special events */
	ESIM_EV_INVALID = esim_register_event(NULL);
	ESIM_EV_NONE = esim_register_event(NULL);
}


void esim_done()
{
	list_free(esim_event_handler_list);
	heap_free(esim_event_heap);
	linked_list_free(esim_end_event_list);
}


int esim_register_event(esim_event_handler_t handler)
{
	list_add(esim_event_handler_list, handler);
	return curr_event++;
}


void esim_schedule_event(int event, void *data, int after)
{
	struct esim_event_t *e;
	
	long long when = esim_cycle + after;

	/* Schedule locked? */
	if (esim_lock_schedule)
		return;
	
	/* Integrity */
	if (event < 0 || event >= list_count(esim_event_handler_list))
		panic("%s: unknown event", __FUNCTION__);
	if (when < esim_cycle)
		panic("%s: event scheduled in the past", __FUNCTION__);
	if (!event)
		panic("%s: invalid event (forgot call to 'esim_register_event'?)", __FUNCTION__);
	
	/* If this is an empty event, ignore it */
	if (event == ESIM_EV_NONE)
		return;
	
	/* Create event */
	e = malloc(sizeof(struct esim_event_t));
	if (!e)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize and insert */
	e->event = event;
	e->data = data;
	heap_insert(esim_event_heap, when, e);

	/* Warn when heap is overloaded */
	if (!esim_overload_shown && heap_count(esim_event_heap) >= ESIM_OVERLOAD_EVENTS)
	{
		esim_overload_shown = 1;
		warning("%s: number of in-flight events exceeds %d.\n%s",
			__FUNCTION__, ESIM_OVERLOAD_EVENTS, err_esim_overload);
	}
}


void esim_schedule_end_event(int event, void *data)
{
	struct esim_event_t *e;

	/* Schedule locked? */
	if (esim_lock_schedule)
		return;

	/* Integrity */
	if (event < 0 || event >= list_count(esim_event_handler_list))
		panic("%s: unknown event", __FUNCTION__);
	if (!event)
		panic("%s: invalid event (forgot call to 'esim_register_event'?)", __FUNCTION__);

	/* If this is an empty event, ignore it */
	if (event == ESIM_EV_NONE)
		return;

	/* Create event */
	e = malloc(sizeof(struct esim_event_t));
	if (!e)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize and insert */
	e->event = event;
	e->data = data;
	linked_list_add(esim_end_event_list, e);
}


void esim_execute_event(int event, void *data)
{
	esim_event_handler_t handler;
	
	/* Schedule locked */
	if (esim_lock_schedule)
		return;
		
	/* Integrity */
	if (event < 0 || event >= list_count(esim_event_handler_list))
		panic("%s: unkown event", __FUNCTION__);
	if (!event)
		panic("%s: invalid event (forgot to call to 'esim_register_event'?)", __FUNCTION__);
	if (event == ESIM_EV_NONE)
		return;
	
	/* execute event handler */
	handler = list_get(esim_event_handler_list, event);
	assert(handler);
	handler(event, data);
}


/* New cycle. Process activated events */
void esim_process_events()
{
	long long when;
	struct esim_event_t *e;
	esim_event_handler_t handler;
	
	/* Process events scheduled for this cycle */
	while (1)
	{
		/* Extract event from heap */
		when = heap_peek(esim_event_heap, (void **) &e);
		if (heap_error(esim_event_heap))
			break;
		
		/* Must process it? */
		assert(when >= esim_cycle);
		if (when != esim_cycle)
			break;
		
		/* Process it */
		heap_extract(esim_event_heap, NULL);
		handler = list_get(esim_event_handler_list, e->event);
		assert(handler);
		handler(e->event, e->data);

		/* Free event */
		free(e);
	}
	
	/* advance cycle counter */
	esim_cycle++;
}


void esim_process_all_events(void)
{
	struct esim_event_t *e;
	esim_event_handler_t handler;

	/* Drain all previous events */
	esim_drain_heap();

	/* Schedule all events that were planned for the end of the simulation
	 * using calls to 'esim_schedule_end_event'. */
	while (linked_list_count(esim_end_event_list))
	{
		/* Extract one event */
		linked_list_head(esim_end_event_list);
		e = linked_list_get(esim_end_event_list);
		linked_list_remove(esim_end_event_list);

		/* Process it */
		handler = list_get(esim_event_handler_list, e->event);
		assert(handler);
		handler(e->event, e->data);

		/* Free event */
		free(e);
	}
	
	/* Drain heap again with new events */
	esim_drain_heap();
}


void esim_empty()
{
	struct esim_event_t *event;
	esim_event_handler_t handler;
	
	/* Lock event scheduling, so no event will be
	 * inserted into the heap */
	esim_lock_schedule = 1;
	
	/* extract all elements from heap */
	while (1)
	{
		/* Extract event */
		heap_extract(esim_event_heap, (void **) &event);
		if (heap_error(esim_event_heap))
			break;
		
		/* Process it */
		handler = list_get(esim_event_handler_list, event->event);
		assert(handler);
		handler(event->event, event->data);

		/* Free event */
		free(event);
	}
	
	/* Unlock event scheduling */
	esim_lock_schedule = 0;
}


int esim_event_count()
{
	return heap_count(esim_event_heap);
}
