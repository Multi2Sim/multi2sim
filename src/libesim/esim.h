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

#ifndef ESIM_H
#define ESIM_H

#include <stdint.h>

/* Variable to indicate esim cycle */
extern uint64_t esim_cycle;

/* Empty event; when this event is scheduled, it will be ignored */
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

/* Advance esim cycle and process events of the new cycle */
void esim_process_events();

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


/* Debugging */
extern FILE *esim_debug_file;
int esim_debug_init(char *filename);
void esim_debug_done(void);
void esim_debug(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

#endif
