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

#ifndef ARCH_X86_EMU_EMU_H
#define ARCH_X86_EMU_EMU_H

#include <pthread.h>
#include <stdio.h>


struct x86_emu_t
{
	/* pid & address_space_index assignment */
	int current_pid;

	/* Schedule next call to 'x86_emu_process_events()'.
	 * The call will only be effective if 'process_events_force' is set.
	 * This flag should be accessed thread-safely locking 'process_events_mutex'. */
	pthread_mutex_t process_events_mutex;
	int process_events_force;

	/* Counter of times that a context has been suspended in a
	 * futex. Used for FIFO wakeups. */
	long long futex_sleep_count;
	
	/* Flag set to force a call to the scheduler 'x86_cpu_schedule()' in the
	 * beginning of next cycle. This flag is set any time a context changes its
	 * state in any bit other than 'spec_mode'. It can be set anywhere in the
	 * code by directly assigning a value to 1. E.g.: when a system call is
	 * executed to change the context's affinity. */
	int schedule_signal;

	/* List of contexts */
	struct x86_ctx_t *context_list_head;
	struct x86_ctx_t *context_list_tail;
	int context_list_count;
	int context_list_max;

	/* List of running contexts */
	struct x86_ctx_t *running_list_head;
	struct x86_ctx_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of suspended contexts */
	struct x86_ctx_t *suspended_list_head;
	struct x86_ctx_t *suspended_list_tail;
	int suspended_list_count;
	int suspended_list_max;

	/* List of zombie contexts */
	struct x86_ctx_t *zombie_list_head;
	struct x86_ctx_t *zombie_list_tail;
	int zombie_list_count;
	int zombie_list_max;

	/* List of finished contexts */
	struct x86_ctx_t *finished_list_head;
	struct x86_ctx_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;
};


/* x86 emulator and architecture */
extern struct x86_emu_t *x86_emu;

extern long long x86_emu_max_cycles;
extern long long x86_emu_max_inst;
extern char x86_emu_last_inst_bytes[20];
extern int x86_emu_last_inst_size;
extern int x86_emu_process_prefetch_hints;



/*
 * Public Functions
 */

void x86_emu_init(void);
void x86_emu_done(void);

int x86_emu_run(void);

void x86_emu_dump(FILE *f);
void x86_emu_dump_summary(FILE *f);

void x86_emu_process_events(void);
void x86_emu_process_events_schedule(void);


#endif

