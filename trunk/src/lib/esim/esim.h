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

#ifndef ESIM_H
#define ESIM_H

#include <stdint.h>
#include <stdio.h>



/*
 * Event-Driven Simulation
 */


/* Variable to indicate event simulation cycle */
extern long long esim_cycle;

/* Empty event. When this event is scheduled, it will be ignored */
extern int ESIM_EV_NONE;

/* Procedure to handle an event */
typedef void (*esim_event_handler_t)(int event, void *data);

/* Initialization and finalization */
void esim_init();
void esim_done();

/* Events */
int esim_register_event(esim_event_handler_t handler);
void esim_schedule_event(int event, void *data, int after);

/* Execute *now* an event handler synchronously; this is not the same as
 * calling esim_schedule_event with after=0, where the event will be processed
 * after all pending events for current cycle completed */
void esim_execute_event(int event, void *data);

/* Advance event simulation cycle and process events of the new cycle */
void esim_process_events();
void esim_process_all_events(int max);

/* Force event extraction; useful to empty event heap before
 * finalization; return false only when heap is empty and
 * extraction failed */
uint64_t esim_extract_event(int *event, void **data);

/* Return number of events in the heap */
int esim_pending();

/* Process esim events, without enabling the schedule of a new event;
 * when all events are processed, esim heap will be empty;
 * esim_cycle is not incremented */
void esim_empty();




/*
 * Simulation Trace
 */

void trace_init(char *file_name);
void trace_done(void);

int trace_new_category(void);

#define trace_status(category) ((category) ? __trace_status((category)) : 0)
int __trace_status(int category);

#define trace(category, ...) ((category) ? \
	__trace((category), 1, __VA_ARGS__) : (void) 0)
#define trace_header(category, ...) ((category) ? \
	__trace((category), 0, __VA_ARGS__) : (void) 0)
void __trace(int category, int print_cycle, char *fmt, ...)
	__attribute__ ((format (printf, 3, 4)));


#endif
