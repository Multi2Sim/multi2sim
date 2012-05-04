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
#include <list.h>
#include <mhandle.h>


static int curr_event = 0;
static int ESIM_EV_INVALID;
static int esim_lock_schedule = 0;

long long esim_cycle = 1;
int ESIM_EV_NONE;

static struct list_t *event_handler_list;
static struct heap_t *event_heap;


struct esim_event_t
{
	int event;
	void *data;
};


void esim_init()
{
	event_handler_list = list_create();
	event_heap = heap_create(20);
	ESIM_EV_INVALID = esim_register_event(NULL);
	ESIM_EV_NONE = esim_register_event(NULL);
}


void esim_done()
{
	list_free(event_handler_list);
	heap_free(event_heap);
}


int esim_register_event(esim_event_handler_t handler)
{
	list_add(event_handler_list, handler);
	return curr_event++;
}


void esim_schedule_event(int event, void *data, int after)
{
	struct esim_event_t *e;
	uint64_t when = esim_cycle + after;
	
	/* Schedule locked? */
	if (esim_lock_schedule)
		return;
	
	/* Integrity */
	if (event < 0 || event >= list_count(event_handler_list))
		panic("%s: unknown event", __FUNCTION__);
	if (when < esim_cycle)
		panic("%s: event scheduled in the past", __FUNCTION__);
	if (!event)
		panic("%s: invalid event (forgot to call to 'esim_register_event'?)", __FUNCTION__);
	
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
	heap_insert(event_heap, when, e);
}


void esim_execute_event(int event, void *data)
{
	esim_event_handler_t handler;
	
	/* Schedule locked */
	if (esim_lock_schedule)
		return;
		
	/* Integrity */
	if (event < 0 || event >= list_count(event_handler_list))
		panic("%s: unkown event", __FUNCTION__);
	if (!event)
		panic("%s: invalid event (forgot to call to 'esim_register_event'?)", __FUNCTION__);
	if (event == ESIM_EV_NONE)
		return;
	
	/* execute event handler */
	handler = list_get(event_handler_list, event);
	assert(handler);
	handler(event, data);
}


/* New cycle. Process activated events */
void esim_process_events()
{
	uint64_t when;
	struct esim_event_t *e;
	esim_event_handler_t handler;
	
	/* Process events scheduled for this cycle */
	while (1)
	{
		/* Extract event from heap */
		when = heap_peek(event_heap, (void **) &e);
		if (heap_error(event_heap))
			break;
		
		/* Must process it? */
		assert(when >= esim_cycle);
		if (when != esim_cycle)
			break;
		
		/* Process it */
		heap_extract(event_heap, NULL);
		handler = list_get(event_handler_list, e->event);
		assert(handler);
		handler(e->event, e->data);

		/* Free event */
		free(e);
	}
	
	/* advance cycle counter */
	esim_cycle++;
}


/* Process all events, until the maximum 'max' is reached, or until the event
 * heap is empty. If 'max' is 0, no maximum is considered. */
void esim_process_all_events(int max)
{
	int count = 0;
	struct esim_event_t *e;
	esim_event_handler_t handler;
	long long cycle;
	
	/* Extract all elements from heap */
	while (!max || count < max)
	{
		/* Extract event */
		cycle = heap_extract(event_heap, (void **) &e);
		if (heap_error(event_heap))
			break;
		
		/* Process it */
		esim_cycle = cycle;
		count++;
		handler = list_get(event_handler_list, e->event);
		assert(handler);
		handler(e->event, e->data);

		/* Free event */
		free(e);
	}
}


void esim_empty()
{
	struct esim_event_t *e;
	esim_event_handler_t handler;
	
	/* Lock event scheduling, so no event will be
	 * inserted into the heap */
	esim_lock_schedule = 1;
	
	/* extract all elements from heap */
	while (1)
	{
		/* Extract event */
		heap_extract(event_heap, (void **) &e);
		if (heap_error(event_heap))
			break;
		
		/* Process it */
		handler = list_get(event_handler_list, e->event);
		assert(handler);
		handler(e->event, e->data);

		/* Free event */
		free(e);
	}
	
	/* Unlock event scheduling */
	esim_lock_schedule = 0;
}


uint64_t esim_extract_event(int *pkind, void **pdata)
{
	struct esim_event_t *e;
	uint64_t when;
	
	/* Extract head of the heap */
	assert(pkind && pdata);
	when = heap_extract(event_heap, (void **) &e);
	if (heap_error(event_heap)) {
		*pkind = 0;
		*pdata = NULL;
		return 0;
	}
	
	/* Return event fields */
	*pkind = e->event;
	*pdata = e->data;
	
	/* Free event and return success */
	free(e);
	return when;
}


int esim_pending()
{
	return heap_count(event_heap);
}
