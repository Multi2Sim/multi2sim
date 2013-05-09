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

#include <assert.h>

#include <arch/mips/timing/cpu.h>
#include <arch/common/arch.h>
#include <arch/common/arch.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/timer.h>
#include <mem-system/memory.h>

#include "context.h"
#include "emu.h"
#include "syscall.h"


/*
 * Global variables
 */

/* Configuration parameters */
long long mips_emu_max_inst = 0;
long long mips_emu_max_cycles = 0;
long long mips_emu_max_time = 0;
enum arch_sim_kind_t mips_emu_sim_kind = arch_sim_kind_functional;

/* MIPS emulator and architecture */
struct mips_emu_t *mips_emu;
struct arch_t *mips_emu_arch;




/*
 * Public functions
 */


/* Initialization */

void mips_emu_init(void)
{
	union
	{
		unsigned int as_uint;
		unsigned char as_uchar[4];
	} endian;

	/* Endian check */
	endian.as_uint = 0x33221100;
	if (endian.as_uchar[0])
		fatal("%s: host machine is not little endian", __FUNCTION__);

	/* Host types */
	M2S_HOST_GUEST_MATCH(sizeof(long long), 8);
	M2S_HOST_GUEST_MATCH(sizeof(int), 4);
	M2S_HOST_GUEST_MATCH(sizeof(short), 2);

	/* Register architecture */
	//mips_emu_arch = arch_list_register("MIPS", "mips");
	//mips_emu_arch->sim_kind = mips_emu_sim_kind;

	/* Initialization */
	mips_sys_init();
	mips_asm_init();

	/* Allocate */
	mips_emu = xcalloc(1, sizeof(struct mips_emu_t));

	/* Initialize */
	mips_emu->current_pid = 1000;  /* Initial assigned pid */
	//mips_emu->timer = m2s_timer_create("mips emulation timer");

	/* Initialize mutex for variables controlling calls to 'mips_emu_process_events()' */
	pthread_mutex_init(&mips_emu->process_events_mutex, NULL);
}


/* Finalization */
void mips_emu_done(void)
{
	struct mips_ctx_t *ctx;

	/* Finish all contexts */
	for (ctx = mips_emu->context_list_head; ctx; ctx = ctx->context_list_next)
		if (!mips_ctx_get_status(ctx, mips_ctx_finished))
			mips_ctx_finish(ctx, 0);

	/* Free contexts */
	while (mips_emu->context_list_head)
		mips_ctx_free(mips_emu->context_list_head);

	/* Finalize GPU */
	/*evg_emu_done();
	si_emu_done();*/
	/*frm_emu_done();*/

	/* Free */
	//m2s_timer_free(mips_emu->timer);
	free(mips_emu);
	mips_asm_done();

	/* End */
//	mips_isa_done();
	mips_sys_done();
}

void mips_emu_dump(FILE *f)
{
}

void mips_emu_dump_summary(FILE *f)
{
	/*	double time_in_sec;
		double inst_per_sec;*/

	/* No statistic dump if there was no Mips simulation */
	//if (!mips_emu->inst_count)
		//	return;

	/* Functional simulation */
	//time_in_sec = (double) m2s_timer_get_value(mips_emu->timer) / 1.0e6;
	//inst_per_sec = time_in_sec > 0.0 ? (double) mips_emu->inst_count / time_in_sec : 0.0;
	//fprintf(f, "[ Mips ]\n");
	//fprintf(f, "SimType = %s\n", mips_emu_sim_kind == arch_sim_kind_functional ?
	//		"Functional" : "Detailed");
	//fprintf(f, "Time = %.2f\n", time_in_sec);
	//fprintf(f, "Instructions = %lld\n", mips_emu->inst_count);
	//fprintf(f, "InstructionsPerSecond = %.0f\n", inst_per_sec);
	fprintf(f, "Contexts = %d\n", mips_emu->running_list_max);
	fprintf(f, "Memory = %lu\n", mem_max_mapped_space);

	/* Detailed simulation */
	//if (mips_emu_sim_kind == arch_sim_kind_detailed)
	//	mips_cpu_dump_summary(f);

	/* End */
	//fprintf(f, "\n");
}


void mips_emu_list_insert_head(enum mips_emu_list_kind_t list, struct mips_ctx_t *ctx)
{
	assert(!mips_emu_list_member(list, ctx));
	switch (list)
	{
	case mips_emu_list_context:

		DOUBLE_LINKED_LIST_INSERT_HEAD(mips_emu, context, ctx);
		break;

	case mips_emu_list_running:

		DOUBLE_LINKED_LIST_INSERT_HEAD(mips_emu, running, ctx);
		break;

	case mips_emu_list_finished:

		DOUBLE_LINKED_LIST_INSERT_HEAD(mips_emu, finished, ctx);
		break;

	case mips_emu_list_zombie:

		DOUBLE_LINKED_LIST_INSERT_HEAD(mips_emu, zombie, ctx);
		break;

	case mips_emu_list_suspended:

		DOUBLE_LINKED_LIST_INSERT_HEAD(mips_emu, suspended, ctx);
		break;

	case mips_emu_list_alloc:

		DOUBLE_LINKED_LIST_INSERT_HEAD(mips_emu, alloc, ctx);
		break;
	}
}


void mips_emu_list_insert_tail(enum mips_emu_list_kind_t list, struct mips_ctx_t *ctx)
{
	assert(!mips_emu_list_member(list, ctx));
	switch (list) {
	case mips_emu_list_context: DOUBLE_LINKED_LIST_INSERT_TAIL(mips_emu, context, ctx); break;
	case mips_emu_list_running: DOUBLE_LINKED_LIST_INSERT_TAIL(mips_emu, running, ctx); break;
	case mips_emu_list_finished: DOUBLE_LINKED_LIST_INSERT_TAIL(mips_emu, finished, ctx); break;
	case mips_emu_list_zombie: DOUBLE_LINKED_LIST_INSERT_TAIL(mips_emu, zombie, ctx); break;
	case mips_emu_list_suspended: DOUBLE_LINKED_LIST_INSERT_TAIL(mips_emu, suspended, ctx); break;
	case mips_emu_list_alloc: DOUBLE_LINKED_LIST_INSERT_TAIL(mips_emu, alloc, ctx); break;
	}
}


void mips_emu_list_remove(enum mips_emu_list_kind_t list, struct mips_ctx_t *ctx)
{
	assert(mips_emu_list_member(list, ctx));
	switch (list) {
	case mips_emu_list_context: DOUBLE_LINKED_LIST_REMOVE(mips_emu, context, ctx); break;
	case mips_emu_list_running: DOUBLE_LINKED_LIST_REMOVE(mips_emu, running, ctx); break;
	case mips_emu_list_finished: DOUBLE_LINKED_LIST_REMOVE(mips_emu, finished, ctx); break;
	case mips_emu_list_zombie: DOUBLE_LINKED_LIST_REMOVE(mips_emu, zombie, ctx); break;
	case mips_emu_list_suspended: DOUBLE_LINKED_LIST_REMOVE(mips_emu, suspended, ctx); break;
	case mips_emu_list_alloc: DOUBLE_LINKED_LIST_REMOVE(mips_emu, alloc, ctx); break;
	}
}


int mips_emu_list_member(enum mips_emu_list_kind_t list, struct mips_ctx_t *ctx)
{
	switch (list) {
	case mips_emu_list_context: return DOUBLE_LINKED_LIST_MEMBER(mips_emu, context, ctx);
	case mips_emu_list_running: return DOUBLE_LINKED_LIST_MEMBER(mips_emu, running, ctx);
	case mips_emu_list_finished: return DOUBLE_LINKED_LIST_MEMBER(mips_emu, finished, ctx);
	case mips_emu_list_zombie: return DOUBLE_LINKED_LIST_MEMBER(mips_emu, zombie, ctx);
	case mips_emu_list_suspended: return DOUBLE_LINKED_LIST_MEMBER(mips_emu, suspended, ctx);
	case mips_emu_list_alloc: return DOUBLE_LINKED_LIST_MEMBER(mips_emu, alloc, ctx);
	}
	return 0;
}


/* Schedule a call to 'mips_emu_process_events' */
void mips_emu_process_events_schedule()
{
	pthread_mutex_lock(&mips_emu->process_events_mutex);
	mips_emu->process_events_force = 1;
	pthread_mutex_unlock(&mips_emu->process_events_mutex);
}





/*
 * Functional simulation loop
 */

/* Run one iteration of the MIPS emulation loop. Return TRUE if still running. */
int mips_emu_run(void)
{
	struct mips_ctx_t *ctx;

	/* Stop if there is no context running */
	if (mips_emu->finished_list_count >= mips_emu->context_list_count)
		return FALSE;

	/* Stop if maximum number of CPU instructions exceeded */
	if (mips_emu_max_inst && arch_mips->inst_count >= mips_emu_max_inst)
		esim_finish = esim_finish_mips_max_inst;

	/* Stop if maximum number of cycles exceeded */
	if (mips_emu_max_cycles && arch_mips->cycle >= mips_emu_max_cycles)
		esim_finish = esim_finish_mips_max_cycles;

	/* Stop if any previous reason met */
	if (esim_finish)
		return TRUE;

	/* Run an instruction from every running process */
	for (ctx = mips_emu->running_list_head; ctx; ctx = ctx->running_list_next)
		mips_ctx_execute(ctx);

	/* Free finished contexts */
	while (mips_emu->finished_list_head)
		mips_ctx_free(mips_emu->finished_list_head);

	/* Still running */
	return TRUE;
}
