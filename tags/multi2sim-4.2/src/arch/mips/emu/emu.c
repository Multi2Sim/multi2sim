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


#include <arch/common/arch.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <mem-system/memory.h>

#include "context.h"
#include "emu.h"
#include "syscall.h"


/*
 * Class 'MIPSEmu'
 */

CLASS_IMPLEMENTATION(MIPSEmu);

void MIPSEmuCreate(MIPSEmu *self)
{
	/* Parent */
	EmuCreate(asEmu(self), "MIPS");

	/* Initialize */
	self->current_pid = 100;
	pthread_mutex_init(&self->process_events_mutex, NULL);

	/* Virtual functions */
	asObject(self)->Dump = MIPSEmuDump;
	asEmu(self)->DumpSummary = MIPSEmuDumpSummary;
	asEmu(self)->Run = MIPSEmuRun;
}


void MIPSEmuDestroy(MIPSEmu *self)
{
	struct mips_ctx_t *ctx;

	/* Finish all contexts */
	for (ctx = self->context_list_head; ctx; ctx = ctx->context_list_next)
		if (!mips_ctx_get_status(ctx, mips_ctx_finished))
			mips_ctx_finish(ctx, 0);

	/* Free contexts */
	while (self->context_list_head)
		mips_ctx_free(self->context_list_head);

}


void MIPSEmuDump(Object *self, FILE *f)
{
	/* Call parent */
	EmuDump(self, f);
}


void MIPSEmuDumpSummary(Emu *self, FILE *f)
{
	MIPSEmu *emu = asMIPSEmu(self);

	/* Call parent */
	EmuDumpSummary(self, f);

	/* More statistics */
	fprintf(f, "Contexts = %d\n", emu->running_list_max);
	fprintf(f, "Memory = %lu\n", mem_max_mapped_space);
}


void MIPSEmuListInsertHead(MIPSEmu *self, enum mips_emu_list_kind_t list,
		struct mips_ctx_t *ctx)
{
	assert(!MIPSEmuListMember(self, list, ctx));
	switch (list)
	{
	case mips_emu_list_context:

		DOUBLE_LINKED_LIST_INSERT_HEAD(self, context, ctx);
		break;

	case mips_emu_list_running:

		DOUBLE_LINKED_LIST_INSERT_HEAD(self, running, ctx);
		break;

	case mips_emu_list_finished:

		DOUBLE_LINKED_LIST_INSERT_HEAD(self, finished, ctx);
		break;

	case mips_emu_list_zombie:

		DOUBLE_LINKED_LIST_INSERT_HEAD(self, zombie, ctx);
		break;

	case mips_emu_list_suspended:

		DOUBLE_LINKED_LIST_INSERT_HEAD(self, suspended, ctx);
		break;

	case mips_emu_list_alloc:

		DOUBLE_LINKED_LIST_INSERT_HEAD(self, alloc, ctx);
		break;
	}
}


void MIPSEmuListInsertTail(MIPSEmu *self, enum mips_emu_list_kind_t list,
		struct mips_ctx_t *ctx)
{
	assert(!MIPSEmuListMember(self, list, ctx));
	switch (list) {
	case mips_emu_list_context: DOUBLE_LINKED_LIST_INSERT_TAIL(self, context, ctx); break;
	case mips_emu_list_running: DOUBLE_LINKED_LIST_INSERT_TAIL(self, running, ctx); break;
	case mips_emu_list_finished: DOUBLE_LINKED_LIST_INSERT_TAIL(self, finished, ctx); break;
	case mips_emu_list_zombie: DOUBLE_LINKED_LIST_INSERT_TAIL(self, zombie, ctx); break;
	case mips_emu_list_suspended: DOUBLE_LINKED_LIST_INSERT_TAIL(self, suspended, ctx); break;
	case mips_emu_list_alloc: DOUBLE_LINKED_LIST_INSERT_TAIL(self, alloc, ctx); break;
	}
}


void MIPSEmuListRemove(MIPSEmu *self, enum mips_emu_list_kind_t list,
		struct mips_ctx_t *ctx)
{
	assert(MIPSEmuListMember(self, list, ctx));
	switch (list) {
	case mips_emu_list_context: DOUBLE_LINKED_LIST_REMOVE(self, context, ctx); break;
	case mips_emu_list_running: DOUBLE_LINKED_LIST_REMOVE(self, running, ctx); break;
	case mips_emu_list_finished: DOUBLE_LINKED_LIST_REMOVE(self, finished, ctx); break;
	case mips_emu_list_zombie: DOUBLE_LINKED_LIST_REMOVE(self, zombie, ctx); break;
	case mips_emu_list_suspended: DOUBLE_LINKED_LIST_REMOVE(self, suspended, ctx); break;
	case mips_emu_list_alloc: DOUBLE_LINKED_LIST_REMOVE(self, alloc, ctx); break;
	}
}


int MIPSEmuListMember(MIPSEmu *self, enum mips_emu_list_kind_t list,
		struct mips_ctx_t *ctx)
{
	switch (list) {
	case mips_emu_list_context: return DOUBLE_LINKED_LIST_MEMBER(self, context, ctx);
	case mips_emu_list_running: return DOUBLE_LINKED_LIST_MEMBER(self, running, ctx);
	case mips_emu_list_finished: return DOUBLE_LINKED_LIST_MEMBER(self, finished, ctx);
	case mips_emu_list_zombie: return DOUBLE_LINKED_LIST_MEMBER(self, zombie, ctx);
	case mips_emu_list_suspended: return DOUBLE_LINKED_LIST_MEMBER(self, suspended, ctx);
	case mips_emu_list_alloc: return DOUBLE_LINKED_LIST_MEMBER(self, alloc, ctx);
	}
	return 0;
}


void MIPSEmuProcessEventsSchedule(MIPSEmu *self)
{
	pthread_mutex_lock(&self->process_events_mutex);
	self->process_events_force = 1;
	pthread_mutex_unlock(&self->process_events_mutex);
}


int MIPSEmuRun(Emu *self)
{
	MIPSEmu *emu = asMIPSEmu(self);

	struct mips_ctx_t *ctx;

	/* Stop if there is no context running */
	if (emu->finished_list_count >= emu->context_list_count)
		return FALSE;

	/* Stop if maximum number of CPU instructions exceeded */
	if (mips_emu_max_inst && asEmu(mips_emu)->instructions >= mips_emu_max_inst)
		esim_finish = esim_finish_mips_max_inst;

	/* Stop if any previous reason met */
	if (esim_finish)
		return TRUE;

	/* Run an instruction from every running process */
	for (ctx = emu->running_list_head; ctx; ctx = ctx->running_list_next)
		mips_ctx_execute(ctx);

	/* Free finished contexts */
	while (emu->finished_list_head)
		mips_ctx_free(emu->finished_list_head);

	/* Still running */
	return TRUE;
}




/*
 * Non-Class Stuff
 */


long long mips_emu_max_inst;
long long mips_emu_max_cycles;
long long mips_emu_max_time;
enum arch_sim_kind_t mips_emu_sim_kind = arch_sim_kind_functional;

MIPSEmu *mips_emu;


void mips_emu_init(void)
{
	/* Classes */
	CLASS_REGISTER(MIPSEmu);

	/* Initialization */
	mips_sys_init();
	mips_asm_init();

	/* Create emulator */
	mips_emu = new(MIPSEmu);
}


void mips_emu_done(void)
{
	/* Free emulator */
	delete(mips_emu);

	/* Free */
	mips_asm_done();

	/* End */
	mips_sys_done();
}

