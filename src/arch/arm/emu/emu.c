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

#include <arch/arm/timing/cpu.h>
#include <arch/common/arch.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/timer.h>
#include <mem-system/memory.h>

#include "context.h"
#include "emu.h"
#include "syscall.h"

/*
 * Global variables
 */

/* Configuration parameters */
long long arm_emu_max_inst = 0;
long long arm_emu_max_cycles = 0;
long long arm_emu_max_time = 0;

/* ARM emulator */
struct arm_emu_t *arm_emu;




/*
 * Public functions
 */


/* Initialization */

void arm_emu_init(void)
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

	/* Initialization */
	arm_sys_init();
	arm_disasm_init();
	arm_thumb16_disasm_init();
	arm_thumb32_disasm_init();

	/* Allocate */
	arm_emu = xcalloc(1, sizeof(struct arm_emu_t));

	/* Initialize */
	arm_emu->current_pid = 1000;  /* Initial assigned pid */

	/* Initialize mutex for variables controlling calls to 'arm_emu_process_events()' */
	pthread_mutex_init(&arm_emu->process_events_mutex, NULL);
}


/* Finalization */
void arm_emu_done(void)
{
	struct arm_ctx_t *ctx;

	/* Finish all contexts */
	for (ctx = arm_emu->context_list_head; ctx; ctx = ctx->context_list_next)
		if (!arm_ctx_get_status(ctx, arm_ctx_finished))
			arm_ctx_finish(ctx, 0);

	/* Free contexts */
	while (arm_emu->context_list_head)
		arm_ctx_free(arm_emu->context_list_head);

	/* Finalize GPU */
	/*evg_emu_done();
	si_emu_done();*/
	/*frm_emu_done();*/

	/* Free */
	free(arm_emu);

	/* End */
	/*arm_isa_done();*/
	arm_disasm_done();
	arm_sys_done();
}


void arm_emu_dump(FILE *f)
{
}


void arm_emu_dump_summary(FILE *f)
{
	fprintf(f, "Contexts = %d\n", arm_emu->running_list_max);
	fprintf(f, "Memory = %lu\n", mem_max_mapped_space);
}


void arm_emu_list_insert_head(enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx)
{
	assert(!arm_emu_list_member(list, ctx));
	switch (list)
	{
	case arm_emu_list_context:

		DOUBLE_LINKED_LIST_INSERT_HEAD(arm_emu, context, ctx);
		break;

	case arm_emu_list_running:

		DOUBLE_LINKED_LIST_INSERT_HEAD(arm_emu, running, ctx);
		break;

	case arm_emu_list_finished:

		DOUBLE_LINKED_LIST_INSERT_HEAD(arm_emu, finished, ctx);
		break;

	case arm_emu_list_zombie:

		DOUBLE_LINKED_LIST_INSERT_HEAD(arm_emu, zombie, ctx);
		break;

	case arm_emu_list_suspended:

		DOUBLE_LINKED_LIST_INSERT_HEAD(arm_emu, suspended, ctx);
		break;

	case arm_emu_list_alloc:

		DOUBLE_LINKED_LIST_INSERT_HEAD(arm_emu, alloc, ctx);
		break;
	}
}


void arm_emu_list_insert_tail(enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx)
{
	assert(!arm_emu_list_member(list, ctx));
	switch (list) {
	case arm_emu_list_context: DOUBLE_LINKED_LIST_INSERT_TAIL(arm_emu, context, ctx); break;
	case arm_emu_list_running: DOUBLE_LINKED_LIST_INSERT_TAIL(arm_emu, running, ctx); break;
	case arm_emu_list_finished: DOUBLE_LINKED_LIST_INSERT_TAIL(arm_emu, finished, ctx); break;
	case arm_emu_list_zombie: DOUBLE_LINKED_LIST_INSERT_TAIL(arm_emu, zombie, ctx); break;
	case arm_emu_list_suspended: DOUBLE_LINKED_LIST_INSERT_TAIL(arm_emu, suspended, ctx); break;
	case arm_emu_list_alloc: DOUBLE_LINKED_LIST_INSERT_TAIL(arm_emu, alloc, ctx); break;
	}
}


void arm_emu_list_remove(enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx)
{
	assert(arm_emu_list_member(list, ctx));
	switch (list) {
	case arm_emu_list_context: DOUBLE_LINKED_LIST_REMOVE(arm_emu, context, ctx); break;
	case arm_emu_list_running: DOUBLE_LINKED_LIST_REMOVE(arm_emu, running, ctx); break;
	case arm_emu_list_finished: DOUBLE_LINKED_LIST_REMOVE(arm_emu, finished, ctx); break;
	case arm_emu_list_zombie: DOUBLE_LINKED_LIST_REMOVE(arm_emu, zombie, ctx); break;
	case arm_emu_list_suspended: DOUBLE_LINKED_LIST_REMOVE(arm_emu, suspended, ctx); break;
	case arm_emu_list_alloc: DOUBLE_LINKED_LIST_REMOVE(arm_emu, alloc, ctx); break;
	}
}


int arm_emu_list_member(enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx)
{
	switch (list) {
	case arm_emu_list_context: return DOUBLE_LINKED_LIST_MEMBER(arm_emu, context, ctx);
	case arm_emu_list_running: return DOUBLE_LINKED_LIST_MEMBER(arm_emu, running, ctx);
	case arm_emu_list_finished: return DOUBLE_LINKED_LIST_MEMBER(arm_emu, finished, ctx);
	case arm_emu_list_zombie: return DOUBLE_LINKED_LIST_MEMBER(arm_emu, zombie, ctx);
	case arm_emu_list_suspended: return DOUBLE_LINKED_LIST_MEMBER(arm_emu, suspended, ctx);
	case arm_emu_list_alloc: return DOUBLE_LINKED_LIST_MEMBER(arm_emu, alloc, ctx);
	}
	return 0;
}


/* Schedule a call to 'arm_emu_process_events' */
void arm_emu_process_events_schedule()
{
	pthread_mutex_lock(&arm_emu->process_events_mutex);
	arm_emu->process_events_force = 1;
	pthread_mutex_unlock(&arm_emu->process_events_mutex);
}





/*
 * Functional simulation loop
 */

/* Run one iteration of the ARM emulation loop. Return TRUE if the emulation is
 * still running. */
int arm_emu_run(void)
{
	struct arm_ctx_t *ctx;

	/* Stop if there is no context running */
	if (arm_emu->finished_list_count >= arm_emu->context_list_count)
		return FALSE;

	/* Stop if maximum number of CPU instructions exceeded */
	if (arm_emu_max_inst && arch_arm->inst_count >= arm_emu_max_inst)
		esim_finish = esim_finish_arm_max_inst;

	/* Stop if maximum number of cycles exceeded */
	if (arm_emu_max_cycles && arch_arm->cycle >= arm_emu_max_cycles)
		esim_finish = esim_finish_arm_max_cycles;

	/* Stop if any previous reason met */
	if (esim_finish)
		return TRUE;

	/* Run an instruction from every running process */
	for (ctx = arm_emu->running_list_head; ctx; ctx = ctx->running_list_next)
		arm_ctx_execute(ctx);

	/* Free finished contexts */
	while (arm_emu->finished_list_head)
		arm_ctx_free(arm_emu->finished_list_head);

	/* Still running */
	return TRUE;
}
