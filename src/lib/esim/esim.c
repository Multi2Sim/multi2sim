/*
 *  Libesim
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

#include <assert.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/heap.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/timer.h>

#include "esim.h"


/* Number of in-flight events before a warning is shown (10k events) */
#define ESIM_OVERLOAD_EVENTS  10000

/* Number of events to process in 'esim_drain_heap' to empty the event heap at
 * the end of the simulation before it is assumed that there is a recursive
 * queuing of events that will cause an infinite loop (1M events). */
#define ESIM_MAX_FINALIZATION_EVENTS  10000000


static char *esim_err_finalization =
	"\tThe finalization process of the event-driven simulation is trying to\n"
	"\tempty the event heap by scheduling all pending events. If the number of\n"
	"\tscheduled events during this process is too high, it could mean that\n"
	"\tsome event is recursively scheduling other events, avoiding correct\n"
	"\tfinalization of the simulation. Please contact development@multi2sim.org\n"
	"\tto report this error.\n";

static char *esim_err_overload =
	"\tAn excessive number of events are currently in-flight in the event-\n"
	"\tdriven simulation library. This is probably the result of a timing\n"
	"\tmodel uncontrollably issuing events (typically a processor pipeline\n"
	"\tissuing memory accesses without checking for a limit in the number\n"
	"\tof in-flight accesses).\n"
	"\tIf you still believe that this is an expected number of in-flight\n"
	"\tevents, please increase the value of macro ESIM_OVERLOAD_EVENTS to\n"
	"\tavoid this warning.\n";


/* Reason for simulation end. Declared as volatile, since it is might be modified
 * by signal handlers. This makes sure the variable resides in a memory location. */
volatile enum esim_finish_t esim_finish = esim_finish_none;

struct str_map_t esim_finish_map =
{
	21, {
		{ "ContextsFinished", esim_finish_ctx },

		{ "x86LastInst", esim_finish_x86_last_inst },
		{ "x86MaxInst", esim_finish_x86_max_inst },
		{ "x86MaxCycles", esim_finish_x86_max_cycles },

		{ "ArmMaxInst", esim_finish_arm_max_inst },
		{ "ArmMaxCycles", esim_finish_arm_max_cycles },

		{ "MipsMaxInst", esim_finish_mips_max_inst },
		{ "MipsMaxCycles", esim_finish_mips_max_cycles },

		{ "EvergreenMaxInst", esim_finish_evg_max_inst },
		{ "EvergreenMaxCycles", esim_finish_evg_max_cycles },
		{ "EvergreenMaxKernels", esim_finish_evg_max_kernels },
		{ "EvergreenNoFaults", esim_finish_evg_no_faults },  /* GPU-REL */

		{ "FermiMaxInst", esim_finish_frm_max_inst },
		{ "FermiMaxCycles", esim_finish_frm_max_cycles },
		{ "FermiMaxFunctions", esim_finish_frm_max_functions},

		{ "SouthernIslandsMaxInst", esim_finish_si_max_inst },
		{ "SouthernIslandsMaxCycles", esim_finish_si_max_cycles },
		{ "SouthernIslandsMaxKernels", esim_finish_si_max_kernels },

		{ "MaxTime", esim_finish_max_time },
		{ "Signal", esim_finish_signal },
		{ "Stall", esim_finish_stall }
	}
};


/* Events */

static int ESIM_EV_INVALID;
static int esim_lock_schedule = 0;

/* Flag indicating whether the overload warning has been already shown. This
 * will prevent it to be shown more than once during the execution. */
static int esim_overload_shown = 0;

/* Special event without any effect. */
int ESIM_EV_NONE;

/* Simulated time in picoseconds */
long long esim_time;

/* Frequency (MHz) and cycle time (psec) of the fastest frequency domain. */
long long esim_cycle_time;
int esim_frequency;

/* Number of main loop iterations with no forwarded time */
long long esim_no_forward_cycles;



/* List of registered events. Each element is of type 'struct
 * esim_event_info_t'. */
static struct list_t *esim_event_info_list;

/* Heap of events. Each element is of type 'struct esim_event_t' */
static struct heap_t *esim_event_heap;

/* List of events to be executed at the end of the simulation, when function
 * 'esim_process_all_events' is called. Each element in this list is of type
 * 'struct esim_event_t'. */
static struct linked_list_t *esim_end_event_list;

/* Global timer */
static struct m2s_timer_t *esim_timer;



/*
 * Frequency Domain
 */

/* List of frequency domains. Each domain is of type 'struct esim_domain_t'. The
 * list contains an element at position 0 set to NULL. */
static struct list_t *esim_domain_list;

struct esim_domain_t
{
	int frequency;
	long long cycle_time;
};


struct esim_domain_t *esim_domain_create(int frequency)
{
	struct esim_domain_t *domain;

	/* Check valid 'frequency' */
	if (!IN_RANGE(frequency, 1, ESIM_MAX_FREQUENCY))
		fatal("%s: frequency not in range [1, %d] (=%d)\n",
			__FUNCTION__, ESIM_MAX_FREQUENCY, frequency);

	/* Initialize */
	domain = xcalloc(1, sizeof(struct esim_domain_t));
	domain->frequency = frequency;
	domain->cycle_time = 1000000ll / frequency;  /* Picoseconds */

	/* Update 'esim_cycle_time' if needed */
	if (!esim_cycle_time || domain->cycle_time < esim_cycle_time)
	{
		esim_frequency = frequency;
		esim_cycle_time = domain->cycle_time;
	}

	/* Return */
	return domain;
}


void esim_domain_free(struct esim_domain_t *domain)
{
	free(domain);
}


int esim_new_domain(int frequency)
{
	struct esim_domain_t *domain;

	domain = esim_domain_create(frequency);
	list_add(esim_domain_list, domain);
	return list_count(esim_domain_list) - 1;
}


long long esim_domain_cycle(int domain_index)
{
	struct esim_domain_t *domain;

	/* Get domain */
	domain = list_get(esim_domain_list, domain_index);
	if (!domain)
		panic("%s: invalid domain index (%d)",
				__FUNCTION__, domain_index);

	/* Return current cycle */
	return esim_time / domain->cycle_time + 1;
}


long long esim_domain_cycle_time(int domain_index)
{
	struct esim_domain_t *domain;

	/* Get domain */
	domain = list_get(esim_domain_list, domain_index);
	if (!domain)
		panic("%s: invalid domain index (%d)",
				__FUNCTION__, domain_index);

	/* Return cycle time */
	return domain->cycle_time;
}


int esim_domain_frequency(int domain_index)
{
	struct esim_domain_t *domain;

	/* Get domain */
	domain = list_get(esim_domain_list, domain_index);
	if (!domain)
		panic("%s: invalid domain index (%d)",
				__FUNCTION__, domain_index);

	/* Return cycle time */
	return domain->frequency;
}


long long esim_cycle(void)
{
	return esim_time / esim_cycle_time + 1;
}




/*
 * Event Info
 */

struct esim_event_info_t
{
	int id;
	char *name;
	esim_event_handler_t handler;
	struct esim_domain_t *domain;
};


struct esim_event_info_t *esim_event_info_create(int id,
	char *name, esim_event_handler_t handler,
	struct esim_domain_t *domain)
{
	struct esim_event_info_t *event_info;

	/* Initialize */
	event_info = xcalloc(1, sizeof(struct esim_event_info_t));
	event_info->id = id;
	event_info->handler = handler;
	event_info->name = xstrdup(name);
	event_info->domain = domain;

	/* Return */
	return event_info;
}


void esim_event_info_free(struct esim_event_info_t *event_info)
{
	free(event_info->name);
	free(event_info);
}




/*
 * Event
 */

struct esim_event_t
{
	int id;
	void *data;
};


struct esim_event_t *esim_event_create(int id, void *data)
{
	struct esim_event_t *event;

	/* Initialize */
	event = xcalloc(1, sizeof(struct esim_event_t));
	event->id = id;
	event->data = data;
	
	/* Return */
	return event;
}


void esim_event_free(struct esim_event_t *event)
{
	free(event);
}




/*
 * Private Functions
 */

/* Drain events in the heap. If this operation stalls after exceeding a limit
 * in the number of events, return non-zero. Zero is success. */
static int esim_drain_heap(void)
{
	int count = 0;

	struct esim_event_t *event;
	struct esim_event_info_t *event_info;

	long long when;

	/* Extract all elements from heap */
	while (1)
	{
		/* Extract event */
		when = heap_extract(esim_event_heap, (void **) &event);
		if (heap_error(esim_event_heap))
			break;

		/* Process it */
		count++;
		esim_time = when;
		event_info = list_get(esim_event_info_list, event->id);
		assert(event_info && event_info->handler);
		event_info->handler(event->id, event->data);
		esim_event_free(event);

		/* Interrupt heap draining after exceeding a given number of
		 * events. This can happen if the event handlers of processed
		 * events keep scheduling new events, causing the heap to never
		 * finish draining. */
		if (count == ESIM_MAX_FINALIZATION_EVENTS)
		{
			esim_dump(stderr, 20);
			warning("%s: number of finalization events exceeds %d - stopped.\n%s",
				__FUNCTION__, ESIM_MAX_FINALIZATION_EVENTS,
				esim_err_finalization);
			return 1;
		}
	}

	/* Success */
	return 0;
}




/*
 * Public Functions
 */


void esim_init()
{
	/* Create structures */
	esim_event_info_list = list_create();
	esim_event_heap = heap_create(20);
	esim_end_event_list = linked_list_create();
	
	/* List of frequency domains */
	esim_domain_list = list_create();
	list_add(esim_domain_list, NULL);

	/* Initialize global timer */
	esim_timer = m2s_timer_create(NULL);
	m2s_timer_start(esim_timer);

	/* Register special events */
	ESIM_EV_INVALID = esim_register_event_with_name(NULL, 0, "Invalid");
	ESIM_EV_NONE = esim_register_event_with_name(NULL, 0, "None");
}


void esim_done()
{
	void *elem;
	int index;

	/* Free list of frequency domains */
	LIST_FOR_EACH(esim_domain_list, index)
	{
		elem = list_get(esim_domain_list, index);
		if (elem)
			esim_domain_free(elem);
	}
	list_free(esim_domain_list);

	/* Free list of event info items */
	LIST_FOR_EACH(esim_event_info_list, index)
		esim_event_info_free(list_get(esim_event_info_list, index));
	list_free(esim_event_info_list);

	/* Free lists of events */
	heap_free(esim_event_heap);
	linked_list_free(esim_end_event_list);

	/* Free global timer */
	m2s_timer_free(esim_timer);
}


/* Dump information in event heap, to a maximum of 'max' events. If 'max' is 0,
 * all events in the heap are dumped. */
void esim_dump(FILE *f, int max)
{
	struct heap_t *aux_event_heap;

	struct esim_event_info_t *event_info;
	struct esim_event_t *event;

	long long when;

	/* Create auxiliary heap to store extracted events. */
	aux_event_heap = heap_create(max);

	/* Dump events */
	fprintf(f, "\n");
	fprintf(f, "Event heap state in simulated time %lld picosedons\n",
			esim_time);
	while (1)
	{
		/* Stop dumping */
		if (max && aux_event_heap->count == max)
			break;
		if (!esim_event_heap->count)
			break;

		/* Transfer an event from main heap to auxiliary heap */
		when = heap_extract(esim_event_heap, (void **) &event);
		heap_insert(aux_event_heap, when, event);

		/* Dump event */
		event_info = list_get(esim_event_info_list, event->id);
		assert(event_info);
		fprintf(f, "\t{ event = '%s', time = %lld, rel. time = %lld }\n",
			event_info->name, when, when - esim_time);
	}

	/* Rest of events */
	if (esim_event_heap->count)
		fprintf(f, "\t\t+ %d more\n", esim_event_heap->count);
	fprintf(f, "Total: %d event(s)\n", esim_event_heap->count +
		aux_event_heap->count);
	fprintf(f, "\n");

	/* Bring events back from list to heap. */
	while (aux_event_heap->count)
	{
		when = heap_extract(aux_event_heap, (void **) &event);
		heap_insert(esim_event_heap, when, event);
	}

	/* Free auxiliary heap */
	heap_free(aux_event_heap);
}


int esim_register_event(esim_event_handler_t handler, int domain_index)
{
	char name[100];
	int index;

	/* Construct event name */
	index = list_count(esim_event_info_list);
	snprintf(name, sizeof name, "event_%d", index);

	/* Register event */
	return esim_register_event_with_name(handler, domain_index, name);
}


int esim_register_event_with_name(esim_event_handler_t handler,
		int domain_index, char *name)
{
	struct esim_event_info_t *event_info;
	struct esim_domain_t *domain;
	int index;

	/* Get frequency domain. Allow invalid domains for event 'Invalid'
	 * and 'None'. */
	domain = list_get(esim_domain_list, domain_index);
	if (!domain && esim_event_info_list->count >= 2)
		panic("%s: invalid domain index (%d)",
				__FUNCTION__, domain_index);

	/* Create event info item */
	index = list_count(esim_event_info_list);
	event_info = esim_event_info_create(index, name, handler, domain);

	/* Add to registered events list */
	list_add(esim_event_info_list, event_info);

	/* Return event ID */
	return index;
}


void esim_schedule_event(int event_index, void *data, int cycles)
{
	struct esim_event_t *event;
	struct esim_event_info_t *event_info;
	struct esim_domain_t *domain;
	long long when;

	/* Schedule locked? */
	if (esim_lock_schedule)
		return;
	
	/* Integrity */
	if (!event_index)
		panic("%s: invalid event - forgot to register?", __FUNCTION__);
	if (!IN_RANGE(event_index, 1, esim_event_info_list->count - 1))
		panic("%s: invalid event index", __FUNCTION__);
	
	/* Special event to be ignored */
	if (event_index == ESIM_EV_NONE)
		return;

	/* Get frequency domain */
	event_info = list_get(esim_event_info_list, event_index);
	domain = event_info->domain;

	/* Calculate time based on event's frequency domain. First, get the
	 * actual current time for the current frequency domain, then add the
	 * time after which the event should be scheduled. */
	when = esim_time / domain->cycle_time * domain->cycle_time;
	when += domain->cycle_time * cycles;
	
	/* Create event and insert in heap */
	event = esim_event_create(event_index, data);
	heap_insert(esim_event_heap, when, event);

	/* Warn when heap is overloaded */
	if (!esim_overload_shown && esim_event_heap->count >= ESIM_OVERLOAD_EVENTS)
	{
		esim_overload_shown = 1;
		warning("%s: number of in-flight events exceeds %d.\n%s",
			__FUNCTION__, ESIM_OVERLOAD_EVENTS, esim_err_overload);
	}
}


void esim_schedule_end_event(int id, void *data)
{
	struct esim_event_t *event;

	/* Schedule locked? */
	if (esim_lock_schedule)
		return;

	/* Integrity */
	if (id < 0 || id >= list_count(esim_event_info_list))
		panic("%s: unknown event", __FUNCTION__);
	if (!id)
		panic("%s: invalid event (forgot call to 'esim_register_event'?)", __FUNCTION__);

	/* If this is an empty event, ignore it */
	if (id == ESIM_EV_NONE)
		return;

	/* Initialize and insert */
	event = esim_event_create(id, data);
	linked_list_add(esim_end_event_list, event);
}


void esim_execute_event(int id, void *data)
{
	struct esim_event_info_t *event_info;
	
	/* Schedule locked */
	if (esim_lock_schedule)
		return;
		
	/* Integrity */
	if (id < 0 || id >= list_count(esim_event_info_list))
		panic("%s: unkown event", __FUNCTION__);
	if (!id)
		panic("%s: invalid event (forgot to call to 'esim_register_event'?)", __FUNCTION__);
	if (id == ESIM_EV_NONE)
		return;
	
	/* Execute event handler */
	event_info = list_get(esim_event_info_list, id);
	assert(event_info && event_info->handler);
	event_info->handler(id, data);
}


void esim_process_events(int forward)
{
	long long when;

	struct esim_event_t *event;
	struct esim_event_info_t *event_info;
	
	/* Check if any action is actually needed. Events will be checked and
	 * global time will be advanced only if argument 'forward' is set or
	 * there are any pending events to process. */
	if (!forward && !esim_event_heap->count)
	{
		esim_no_forward_cycles++;
		return;
	}

	/* Process events scheduled for this cycle */
	while (1)
	{
		/* Extract event from heap */
		when = heap_peek(esim_event_heap, (void **) &event);
		if (heap_error(esim_event_heap))
			break;
		
		/* Stop when we find the first event that should run in the future. */
		if (when > esim_time)
			break;
		
		/* Process it */
		heap_extract(esim_event_heap, NULL);
		event_info = list_get(esim_event_info_list, event->id);
		assert(event_info && event_info->handler);
		event_info->handler(event->id, event->data);
		esim_event_free(event);
	}
	
	/* Next simulation cycle */
	esim_time += esim_cycle_time;
}


void esim_process_all_events(void)
{
	struct esim_event_t *event;
	struct esim_event_info_t *event_info;
	int err;

	/* Drain all previous events */
	err = esim_drain_heap();

	/* An stall while draining heap stops further processing. */
	if (err)
		return;

	/* Schedule all events that were planned for the end of the simulation
	 * using calls to 'esim_schedule_end_event'. */
	while (linked_list_count(esim_end_event_list))
	{
		/* Extract one event */
		linked_list_head(esim_end_event_list);
		event = linked_list_get(esim_end_event_list);
		linked_list_remove(esim_end_event_list);

		/* Process it */
		event_info = list_get(esim_event_info_list, event->id);
		assert(event_info && event_info->handler);
		event_info->handler(event->id, event->data);
		esim_event_free(event);
	}
	
	/* Drain heap again with new events */
	esim_drain_heap();
}


void esim_empty(void)
{
	struct esim_event_t *event;
	struct esim_event_info_t *event_info;
	
	/* Lock event scheduling, so no event will be
	 * inserted into the heap */
	esim_lock_schedule = 1;
	
	/* Extract all elements from heap */
	while (1)
	{
		/* Extract event */
		heap_extract(esim_event_heap, (void **) &event);
		if (heap_error(esim_event_heap))
			break;
		
		/* Process it */
		event_info = list_get(esim_event_info_list, event->id);
		assert(event_info && event_info->handler);
		event_info->handler(event->id, event->data);
		esim_event_free(event);
	}
	
	/* Unlock event scheduling */
	esim_lock_schedule = 0;
}


int esim_event_count(void)
{
	return esim_event_heap->count;
}


long long esim_real_time(void)
{
	return m2s_timer_get_value(esim_timer);
}
