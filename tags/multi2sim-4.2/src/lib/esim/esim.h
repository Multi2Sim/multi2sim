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
 * Global Variables
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

	esim_finish_mips_max_inst,
	esim_finish_mips_max_cycles,
	esim_finish_mips_last_inst,

	esim_finish_evg_max_inst,  /* Maximum instruction count reached in Evergreen GPU */
	esim_finish_evg_max_cycles,  /* Maximum cycle count reached in Evergreen GPU */
	esim_finish_evg_max_kernels,  /* Maximum number of GPU kernels */
	esim_finish_evg_no_faults,  /* GPU-REL: no fault in '--evg-stack-faults' caused error */

	esim_finish_frm_max_inst,
	esim_finish_frm_max_cycles,
	esim_finish_frm_max_functions,

	esim_finish_si_max_inst,
	esim_finish_si_max_cycles,
	esim_finish_si_max_kernels,

	esim_finish_max_time,  /* Maximum simulation time reached */
	esim_finish_signal,  /* Signal received */
	esim_finish_stall  /* Simulation stalled */
} esim_finish;


/* Simulated time in picoseconds */
extern long long esim_time;

/* Cycle time of one iteration of the main Multi2Sim loop. For every call to
 * 'esim_process_events()', 'esim_time' will advance in as many picoseconds as
 * this value indicates. The main loop cycle time is calculated as the minimum
 * of all frequency domains' cycle times, as registered with
 * 'esim_new_domain()'. These variables are read-only, updated every time a new
 * call to 'esim_new_domain()' is performed. */
extern int esim_frequency;
extern long long esim_cycle_time;

/* Counter keeping track of all calls to 'esim_process_events()' where the
 * global time 'esim_time' was not incremented (for example, as a result of
 * all architectures performing only a functional simulation. */
extern long long esim_no_forward_cycles;

/* Empty event. When this event is scheduled, it will be ignored */
extern int ESIM_EV_NONE;

/* Procedure to handle an event */
typedef void (*esim_event_handler_t)(int event, void *data);




/*
 * Functions
 */

/* Initialization and finalization */
void esim_init(void);
void esim_done(void);

/* Dump information in event heap, to a maximum of 'max' events. If 'max' is 0,
 * all events in the heap are dumped. */
void esim_dump(FILE *f, int max);

/* Create a new frequency domain. Argument 'frequency' specifies the frequency
 * in MHz. The function returns a domain identifier. */
#define ESIM_MAX_FREQUENCY  10000
int esim_new_domain(int frequency);

/* Functions returning the current cycle and the cycle time of a frequency
 * domain. As an argument, they take a domain identifier returned by
 * 'esim_new_domain'. The first cycle in any frequency domain is always 1. */
long long esim_domain_cycle(int domain_index);
long long esim_domain_cycle_time(int domain_index);
int esim_domain_frequency(int domain_index);

/* Return the current cycle of the fastest domain. */
long long esim_cycle(void);


/* Register an event, optionally giving an event name. These functions take an
 * additional argument 'domain_index', specifying the frequency domain that the
 * event is associated to. */
int esim_register_event(esim_event_handler_t handler, int domain_index);
int esim_register_event_with_name(esim_event_handler_t handler,
		int domain_index, char *name);

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

/* Call to be made in each iteration of the main simulation loop, moving the
 * event-driven simulation engine one cycle.
 * The argument 'forward' is a flag forcing the global simulation time
 * 'esim_time' to advance by 'esim_cycle_time' picoseconds. If not set,
 * the global time will only advance if there were any pending events to
 * process.
 * The caller should set flag 'forward' to FALSE if there is no need to
 * advance the global simulation time, e.g., if all architectures in the
 * system are just performing a functional simulation.
 * For each call to 'esim_process_events' where the global simulation time
 * did not effectively advance, global counter 'esim_no_forward_cycles' is
 * incremented. */
void esim_process_events(int forward);

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
 * when all events are processed, esim heap will be empty.
 * Value in 'esim_time' is not incremented */
void esim_empty(void);

/* Return the number of micro-seconds ellapsed since the beginning of the
 * simulation. */
long long esim_real_time(void);


#endif

