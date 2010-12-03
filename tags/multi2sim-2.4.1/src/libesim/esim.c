/*
 *  Libesim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <list.h>
#include <heap.h>
#include <repos.h>
#include <mhandle.h>
#include "esim.h"

static int curr_event = 0;
static int ESIM_EV_INVALID;
static int esim_lock_schedule = 0;

uint64_t esim_cycle = 0;
int ESIM_EV_NONE;

static struct list_t *event_procs;
static struct heap_t *event_heap;
static struct repos_t *event_repos;


struct event_t {
	int event;
	void *data;
};


void esim_init()
{
	event_procs = list_create(10);
	event_heap = heap_create(20);
	event_repos = repos_create(sizeof(struct event_t), "event_repos");
	ESIM_EV_INVALID = esim_register_event(NULL);
	ESIM_EV_NONE = esim_register_event(NULL);
}


void esim_done()
{
	list_free(event_procs);
	heap_free(event_heap);
	repos_free(event_repos);
}


int esim_register_event(esim_event_handler_t handler)
{
	list_add(event_procs, handler);
	return curr_event++;
}


void esim_schedule_event(int event, void *data, int after)
{
	struct event_t *e;
	uint64_t when = esim_cycle + after;
	
	/* Schedule locked? */
	if (esim_lock_schedule)
		return;
	
	/* integrity */
	if (event < 0 || event >= list_count(event_procs)) {
		fprintf(stderr, "esim: unknown scheduled event\n");
		abort();
	}
	if (when < esim_cycle) {
		fprintf(stderr, "esim: cycle %lld: event scheduled in the past\n",
			(long long) esim_cycle);
		abort();
	}
	if (!event) {
		fprintf(stderr, "esim: scheduled event of kind 0; esim_register_event forgotten?\n");
		abort();
	}
	
	/* if this is an empty event, ignore it */
	if (event == ESIM_EV_NONE)
		return;
	
	/* create and insert event */
	e = repos_create_object(event_repos);
	assert(e);
	e->event = event;
	e->data = data;
	heap_insert(event_heap, when, e);
}


void esim_execute_event(int event, void *data)
{
	esim_event_handler_t handler;
	
	/* schedule locked? */
	if (esim_lock_schedule)
		return;
		
	/* integrity */
	if (event < 0 || event >= list_count(event_procs)) {
		fprintf(stderr, "esim: unknown scheduled event\n");
		abort();
	}
	if (!event) {
		fprintf(stderr, "esim: scheduled event of kind 0; esim_register_event forgotten?\n");
		abort();
	}
	if (event == ESIM_EV_NONE)
		return;
	
	/* execute event handler */
	handler = list_get(event_procs, event);
	assert(!list_error(event_procs));
	assert(handler);
	handler(event, data);
}


/* new cycle; process activated events */
void esim_process_events()
{
	uint64_t when;
	struct event_t *e;
	esim_event_handler_t handler;
	
	/* process events scheduled for this cycle */
	while (1) {
		/* extract event from heap */
		when = heap_peek(event_heap, (void **) &e);
		if (heap_error(event_heap))
			break;
		
		/* must we process it? */
		assert(when >= esim_cycle);
		if (when != esim_cycle)
			break;
		
		/* ok, process it */
		heap_extract(event_heap, NULL);
		handler = list_get(event_procs, e->event);
		assert(!list_error(event_procs));
		assert(handler);
		handler(e->event, e->data);
		repos_free_object(event_repos, e);
	}
	
	/* advance cycle counter */
	esim_cycle++;
}


void esim_empty()
{
	struct event_t *e;
	esim_event_handler_t handler;
	
	/* lock event scheduling, so no event will be
	 * inserted into the heap */
	esim_lock_schedule = 1;
	
	/* extract all elements from heap */
	while (1) {
		
		/* extract event */
		heap_extract(event_heap, (void **) &e);
		if (heap_error(event_heap))
			break;
		
		/* process it */
		handler = list_get(event_procs, e->event);
		assert(!list_error(event_procs));
		assert(handler);
		handler(e->event, e->data);
		repos_free_object(event_repos, e);
	}
	
	/* unlock event scheduling */
	esim_lock_schedule = 0;
}


uint64_t esim_extract_event(int *pkind, void **pdata)
{
	struct event_t *e;
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
	
	/* free event and return success */
	repos_free_object(event_repos, e);
	return when;
}


int esim_pending()
{
	return heap_count(event_heap);
}




/* Debugging */

FILE *esim_debug_file;
static uint64_t esim_debug_cycle;

int esim_debug_init(char *filename)
{
	esim_debug_file = strcmp(filename, "stdout") ?
		(strcmp(filename, "stderr") ?
		fopen(filename, "wt") : stderr) : stdout;
	return esim_debug_file != NULL;
}

void esim_debug_done()
{
	if (esim_debug_file)
		fclose(esim_debug_file);
}

void esim_debug(char *fmt, ...)
{
	va_list va;

	if (!esim_debug_file)
		return;
	if (esim_cycle >= esim_debug_cycle)
		fprintf(esim_debug_file, "clk c=%llu\n",
			(unsigned long long) esim_cycle);
	va_start(va, fmt);
	vfprintf(esim_debug_file, fmt, va);
	esim_debug_cycle = esim_cycle + 1;
}

