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

#ifndef ARCH_ARM_EMU_EMU_H
#define ARCH_ARM_EMU_EMU_H

#include <pthread.h>
#include <stdio.h>

#include <arch/common/emu.h>


/*
 * Class 'ARMEmu'
 */

enum arm_emu_list_kind_t
{
	arm_emu_list_context = 0,
	arm_emu_list_running,
	arm_emu_list_suspended,
	arm_emu_list_zombie,
	arm_emu_list_finished,
	arm_emu_list_alloc
};


CLASS_BEGIN(ARMEmu, Emu)

	/* PID assigned */
	int current_pid;

	/* Schedule next call to 'x86_emu_process_events()'.
	 * The call will only be effective if 'process_events_force' is set.
	 * This flag should be accessed thread-safely locking 'process_events_mutex'. */
	pthread_mutex_t process_events_mutex;
	int process_events_force;

	/* Counter of times that a context has been suspended in a
	 * futex. Used for FIFO wakeups. */
	long long futex_sleep_count;

	/* Flag set when any context changes any status other than 'specmode' */
	int context_reschedule;

	/* List of contexts */
	struct arm_ctx_t *context_list_head;
	struct arm_ctx_t *context_list_tail;
	int context_list_count;
	int context_list_max;

	/* List of running contexts */
	struct arm_ctx_t *running_list_head;
	struct arm_ctx_t *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of suspended contexts */
	struct arm_ctx_t *suspended_list_head;
	struct arm_ctx_t *suspended_list_tail;
	int suspended_list_count;
	int suspended_list_max;

	/* List of zombie contexts */
	struct arm_ctx_t *zombie_list_head;
	struct arm_ctx_t *zombie_list_tail;
	int zombie_list_count;
	int zombie_list_max;

	/* List of finished contexts */
	struct arm_ctx_t *finished_list_head;
	struct arm_ctx_t *finished_list_tail;
	int finished_list_count;
	int finished_list_max;

	/* List of allocated contexts */
	struct arm_ctx_t *alloc_list_head;
	struct arm_ctx_t *alloc_list_tail;
	int alloc_list_count;
	int alloc_list_max;

CLASS_END(ARMEmu)


void ARMEmuCreate(ARMEmu *self);
void ARMEmuDestroy(ARMEmu *self);

/* Virtual function from class 'Object' */
void ARMEmuDump(Object *self, FILE *f);

/* Virtual function from class 'Emu' */
void ARMEmuDumpSummary(Emu *self, FILE *f);

/* Virtual function from class 'Emu' */
int ARMEmuRun(Emu *self);

void ARMEmuProcessEventsSchedule(ARMEmu *self);

/* FIXME - Ugly functions below to be removed */
int ARMEmuListMember(ARMEmu *emu, enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx);
void ARMEmuListRemove(ARMEmu *emu, enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx);
void ARMEmuListInsertTail(ARMEmu *emu, enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx);
void ARMEmuListInsertHead(ARMEmu *emu, enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx);




/*
 * Non-Class
 */

extern ARMEmu *arm_emu;

extern long long arm_emu_max_cycles;
extern long long arm_emu_max_inst;
extern long long arm_emu_max_time;

void arm_emu_init(void);
void arm_emu_done(void);


#endif
