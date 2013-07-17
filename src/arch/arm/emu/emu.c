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


#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <mem-system/memory.h>

#include "context.h"
#include "emu.h"
#include "syscall.h"



/*
 * Class 'ARMEmu'
 */

CLASS_IMPLEMENTATION(ARMEmu);


void ARMEmuCreate(ARMEmu *self)
{
	/* Parent */
	EmuCreate(asEmu(self), "ARM");

	/* Initialize */
	self->current_pid = 100;
	pthread_mutex_init(&self->process_events_mutex, NULL);

	/* Virtual functions */
	asObject(self)->Dump = ARMEmuDump;
	asEmu(self)->DumpSummary = ARMEmuDumpSummary;
	asEmu(self)->Run = ARMEmuRun;
}


void ARMEmuDestroy(ARMEmu *self)
{
	struct arm_ctx_t *ctx;

	/* Finish all contexts */
	for (ctx = self->context_list_head; ctx; ctx = ctx->context_list_next)
		if (!arm_ctx_get_status(ctx, arm_ctx_finished))
			arm_ctx_finish(ctx, 0);

	/* Free contexts */
	while (self->context_list_head)
		arm_ctx_free(self->context_list_head);

}


void ARMEmuDump(Object *self, FILE *f)
{
	/* Call parent */
	EmuDump(self, f);
}


void ARMEmuDumpSummary(Emu *self, FILE *f)
{
	ARMEmu *emu = asARMEmu(self);

	/* Call parent */
	EmuDumpSummary(self, f);

	/* More statistics */
	fprintf(f, "Contexts = %d\n", emu->running_list_max);
	fprintf(f, "Memory = %lu\n", mem_max_mapped_space);
}


void ARMEmuListInsertHead(ARMEmu *self, enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx)
{
	assert(!ARMEmuListMember(arm_emu, list, ctx));
	switch (list)
	{
	case arm_emu_list_context:

		DOUBLE_LINKED_LIST_INSERT_HEAD(self, context, ctx);
		break;

	case arm_emu_list_running:

		DOUBLE_LINKED_LIST_INSERT_HEAD(self, running, ctx);
		break;

	case arm_emu_list_finished:

		DOUBLE_LINKED_LIST_INSERT_HEAD(self, finished, ctx);
		break;

	case arm_emu_list_zombie:

		DOUBLE_LINKED_LIST_INSERT_HEAD(self, zombie, ctx);
		break;

	case arm_emu_list_suspended:

		DOUBLE_LINKED_LIST_INSERT_HEAD(self, suspended, ctx);
		break;

	case arm_emu_list_alloc:

		DOUBLE_LINKED_LIST_INSERT_HEAD(self, alloc, ctx);
		break;
	}
}


void ARMEmuListInsertTail(ARMEmu *self, enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx)
{
	assert(!ARMEmuListMember(arm_emu, list, ctx));
	switch (list) {
	case arm_emu_list_context: DOUBLE_LINKED_LIST_INSERT_TAIL(self, context, ctx); break;
	case arm_emu_list_running: DOUBLE_LINKED_LIST_INSERT_TAIL(self, running, ctx); break;
	case arm_emu_list_finished: DOUBLE_LINKED_LIST_INSERT_TAIL(self, finished, ctx); break;
	case arm_emu_list_zombie: DOUBLE_LINKED_LIST_INSERT_TAIL(self, zombie, ctx); break;
	case arm_emu_list_suspended: DOUBLE_LINKED_LIST_INSERT_TAIL(self, suspended, ctx); break;
	case arm_emu_list_alloc: DOUBLE_LINKED_LIST_INSERT_TAIL(self, alloc, ctx); break;
	}
}


void ARMEmuListRemove(ARMEmu *self, enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx)
{
	assert(ARMEmuListMember(arm_emu, list, ctx));
	switch (list) {
	case arm_emu_list_context: DOUBLE_LINKED_LIST_REMOVE(self, context, ctx); break;
	case arm_emu_list_running: DOUBLE_LINKED_LIST_REMOVE(self, running, ctx); break;
	case arm_emu_list_finished: DOUBLE_LINKED_LIST_REMOVE(self, finished, ctx); break;
	case arm_emu_list_zombie: DOUBLE_LINKED_LIST_REMOVE(self, zombie, ctx); break;
	case arm_emu_list_suspended: DOUBLE_LINKED_LIST_REMOVE(self, suspended, ctx); break;
	case arm_emu_list_alloc: DOUBLE_LINKED_LIST_REMOVE(self, alloc, ctx); break;
	}
}


int ARMEmuListMember(ARMEmu *self, enum arm_emu_list_kind_t list, struct arm_ctx_t *ctx)
{
	switch (list) {
	case arm_emu_list_context: return DOUBLE_LINKED_LIST_MEMBER(self, context, ctx);
	case arm_emu_list_running: return DOUBLE_LINKED_LIST_MEMBER(self, running, ctx);
	case arm_emu_list_finished: return DOUBLE_LINKED_LIST_MEMBER(self, finished, ctx);
	case arm_emu_list_zombie: return DOUBLE_LINKED_LIST_MEMBER(self, zombie, ctx);
	case arm_emu_list_suspended: return DOUBLE_LINKED_LIST_MEMBER(self, suspended, ctx);
	case arm_emu_list_alloc: return DOUBLE_LINKED_LIST_MEMBER(self, alloc, ctx);
	}
	return 0;
}


void ARMEmuProcessEventsSchedule(ARMEmu *self)
{
	pthread_mutex_lock(&self->process_events_mutex);
	self->process_events_force = 1;
	pthread_mutex_unlock(&self->process_events_mutex);
}


int ARMEmuRun(Emu *self)
{
	ARMEmu *emu = asARMEmu(self);
	struct arm_ctx_t *ctx;

	/* Stop if there is no context running */
	if (emu->finished_list_count >= emu->context_list_count)
		return FALSE;

	/* Stop if maximum number of CPU instructions exceeded */
	if (arm_emu_max_inst && self->instructions >= arm_emu_max_inst)
		esim_finish = esim_finish_arm_max_inst;

	/* Stop if any previous reason met */
	if (esim_finish)
		return TRUE;

	/* Run an instruction from every running process */
	for (ctx = emu->running_list_head; ctx; ctx = ctx->running_list_next)
		arm_ctx_execute(ctx);

	/* Free finished contexts */
	while (emu->finished_list_head)
		arm_ctx_free(emu->finished_list_head);

	/* Still running */
	return TRUE;
}



/*
 * Non-Class
 */

/* Configuration parameters */
long long arm_emu_max_inst;
long long arm_emu_max_cycles;
long long arm_emu_max_time;


/* ARM emulator */
ARMEmu *arm_emu;


void arm_emu_init(void)
{
	/* Classes */
	CLASS_REGISTER(ARMEmu);

	/* Initialization */
	arm_sys_init();
	arm_disasm_init();
	arm_thumb16_disasm_init();
	arm_thumb32_disasm_init();

	/* Create ARM emulator */
	arm_emu = new(ARMEmu);
}


void arm_emu_done(void)
{
	/* End */
	arm_disasm_done();
	arm_sys_done();
	
	/* Free ARM emulator */
	delete(arm_emu);
}

