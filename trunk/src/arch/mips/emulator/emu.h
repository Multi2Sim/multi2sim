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

#ifndef ARCH_MIPS_EMU_EMU_H
#define ARCH_MIPS_EMU_EMU_H

#include <pthread.h>

#include <arch/common/emu.h>
#include <arch/mips/asm/Wrapper.h>


/*
 * Class 'MIPSEmu'
 */


CLASS_BEGIN(MIPSEmu, Emu)

	/* Disassembler */
	struct MIPSAsmWrap *as;

	/* PID counter */
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
	MIPSContext *context_list_head;
	MIPSContext *context_list_tail;
	int context_list_count;
	int context_list_max;

	/* List of running contexts */
	MIPSContext *running_list_head;
	MIPSContext *running_list_tail;
	int running_list_count;
	int running_list_max;

	/* List of suspended contexts */
	MIPSContext *suspended_list_head;
	MIPSContext *suspended_list_tail;
	int suspended_list_count;
	int suspended_list_max;

	/* List of zombie contexts */
	MIPSContext *zombie_list_head;
	MIPSContext *zombie_list_tail;
	int zombie_list_count;
	int zombie_list_max;

	/* List of finished contexts */
	MIPSContext *finished_list_head;
	MIPSContext *finished_list_tail;
	int finished_list_count;
	int finished_list_max;

	/* List of allocated contexts */
	MIPSContext *alloc_list_head;
	MIPSContext *alloc_list_tail;
	int alloc_list_count;
	int alloc_list_max;

	/* Stats */
	long long inst_count;  /* Number of emulated instructions */

CLASS_END(MIPSEmu)


void MIPSEmuCreate(MIPSEmu *self, struct MIPSAsmWrap *as);
void MIPSEmuDestroy(MIPSEmu *self);

void MIPSEmuDump(Object *self, FILE *f);
void MIPSEmuDumpSummary(Emu *self, FILE *f);

/* Virtual function from class 'Emu' */
int MIPSEmuRun(Emu *self);

void MIPSEmuProcessEventsSchedule(MIPSEmu *self);

void MIPSEmuLoadContextFromCommandLine(MIPSEmu *self,
		int argc, char **argv);
void MIPSEmuLoadContextsFromIniFile(MIPSEmu *self,
		struct config_t *config, char *section);


/*
 * Non-Class Functions
 */

extern enum arch_sim_kind_t mips_emu_sim_kind;

extern long long mips_emu_max_cycles;
extern long long mips_emu_max_inst;
extern long long mips_emu_max_time;


#endif
