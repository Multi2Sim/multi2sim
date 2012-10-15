/*
 *  Multi2Sim
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

#ifndef LIB_ESIM_ESIM_H
#define LIB_ESIM_ESIM_H

#include <stdint.h>
#include <stdio.h>


/*
 * Event-Driven Simulation
 */


/* Reason for simulation end */
extern struct str_map_t esim_finish_map;

extern volatile enum esim_finish_t
{
	esim_finish_none,  /* Simulation not finished */
	esim_finish_ctx,  /* Contexts finished */

	esim_finish_x86_last_inst,  /* Last x86 instruction reached, as specified by user */
	esim_finish_x86_max_inst,  /* Maximum instruction count reached in x86 CPU */
	esim_finish_x86_max_cycles,  /* Maximum cycle count reached in x86 CPU */

	esim_finish_arm_max_inst,
	esim_finish_arm_max_cycles,
	esim_finish_arm_last_inst,

	esim_finish_evg_max_inst,  /* Maximum instruction count reached in Evergreen GPU */
	esim_finish_evg_max_cycles,  /* Maximum cycle count reached in Evergreen GPU */
	esim_finish_evg_max_kernels,  /* Maximum number of GPU kernels */
	esim_finish_evg_no_faults,  /* GPU-REL: no fault in '--evg-stack-faults' caused error */

	esim_finish_frm_max_inst,
	esim_finish_frm_max_cycles,
	esim_finish_frm_max_kernels,

	esim_finish_si_max_inst,
	esim_finish_si_max_cycles,
	esim_finish_si_max_kernels,

	esim_finish_max_time,  /* Maximum simulation time reached */
	esim_finish_signal,  /* Signal received */
	esim_finish_stall  /* Simulation stalled */
} esim_finish;

/* Variable to indicate event simulation cycle */
extern long long esim_cycle;

/* Empty event. When this event is scheduled, it will be ignored */
extern int ESIM_EV_NONE;

/* Procedure to handle an event */
typedef void (*esim_event_handler_t)(int event, void *data);

/* Initialization and finalization */
void esim_init(void);
void esim_done(void);

/* Dump information in event heap, to a maximum of 'max' events. If 'max' is 0,
 * all events in the heap are dumped. */
void esim_dump(FILE *f, int max);

/* Register an events */
int esim_register_event(esim_event_handler_t handler);
int esim_register_event_with_name(esim_event_handler_t handler, char *name);

/* Schedule an event in 'after' cycles from now. If several cycles are
 * scheduled for the same cycle, they will execute in the order they were
 * scheduled. */
void esim_schedule_event(int event, void *data, int after);

/* Schedule an event for the end of the simulation. This event will be executed
 * during the call to 'esim_process_all_events' at the end of the program. */
void esim_schedule_end_event(int event, void *data);

/* Execute *now* an event handler synchronously; this is not the same as
 * calling esim_schedule_event with after=0, where the event will be processed
 * after all pending events for current cycle completed */
void esim_execute_event(int event, void *data);

/* Advance event simulation one cycle and process all events for the new cycle.
 * This function should be called at the end of the main simulation loop body
 * of the main program. */
void esim_process_events(void);

/* Process all events in the heap. When the heap is empty, all finalization
 * events scheduled with 'esim_schedule_end_event' are processed. Since
 * these events could schedule new events, the function finally continues
 * simulation until the heap is empty again.
 * This function should be called after the main simulation loop finishes
 * in the main program. */
void esim_process_all_events(void);

/* Return number of events in the heap */
int esim_event_count(void);

/* Process esim events, without enabling the schedule of a new event;
 * when all events are processed, esim heap will be empty;
 * esim_cycle is not incremented */
void esim_empty(void);

/* Return the number of micro-seconds ellapsed since the beginning of the
 * simulation. */
long long esim_real_time(void);


#endif

