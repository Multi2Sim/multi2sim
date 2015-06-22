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

#include <unistd.h>

#include <arch/common/arch.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <memory/memory.h>

#include "context.h"
#include "emu.h"
#include "syscall.h"


/*
 * Class 'MIPSEmu'
 */

void MIPSEmuCreate(MIPSEmu *self, struct MIPSAsmWrap *as)
{
	/* Parent */
	EmuCreate(asEmu(self), "MIPS");

	/* Initialize */
	self->as = as;
	self->current_pid = 100;
	pthread_mutex_init(&self->process_events_mutex, NULL);

	/* Virtual functions */
	asObject(self)->Dump = MIPSEmuDump;
	asEmu(self)->DumpSummary = MIPSEmuDumpSummary;
	asEmu(self)->Run = MIPSEmuRun;
}


void MIPSEmuDestroy(MIPSEmu *self)
{
	MIPSContext *ctx;

	/* Finish all contexts */
	for (ctx = self->context_list_head; ctx; ctx = ctx->context_list_next)
		if (!MIPSContextGetState(ctx, MIPSContextFinished))
			MIPSContextFinish(ctx, 0);

	/* Free contexts */
	while (self->context_list_head)
		delete(self->context_list_head);

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

void MIPSEmuProcessEventsSchedule(MIPSEmu *self)
{
	pthread_mutex_lock(&self->process_events_mutex);
	self->process_events_force = 1;
	pthread_mutex_unlock(&self->process_events_mutex);
}


int MIPSEmuRun(Emu *self)
{
	MIPSEmu *emu = asMIPSEmu(self);
	MIPSContext *ctx;

	/* Stop if there is no context running */
	if (emu->finished_list_count >= emu->context_list_count)
		return FALSE;

	/* Stop if maximum number of CPU instructions exceeded */
	if (mips_emu_max_inst && asEmu(self)->instructions >= mips_emu_max_inst)
		esim_finish = esim_finish_mips_max_inst;

	/* Stop if any previous reason met */
	if (esim_finish)
		return TRUE;

	/* Run an instruction from every running process */
	for (ctx = emu->running_list_head; ctx; ctx = ctx->running_list_next)
		MIPSContextExecute(ctx);

	/* Free finished contexts */
	while (emu->finished_list_head)
		delete(emu->finished_list_head);

	/* Still running */
	return TRUE;
}


/* Look for a context matching pid in the list of existing
 * contexts of the kernel. */
MIPSContext *MIPSEmuGetContext(MIPSEmu *self, int pid)
{
	MIPSContext *context;

	context = self->context_list_head;
	while (context && context->pid != pid)
		context = context->context_list_next;
	return context;
}


void MIPSEmuLoadContextFromCommandLine(MIPSEmu *self, int argc, char **argv)
{
	MIPSContext *context;

	char buf[MAX_STRING_SIZE];

	/* Create context */
	MIPSContextDebug("Creating context\n");
	context = new(MIPSContext, self);
	MIPSContextDebug("Context Created\n");

	/* Arguments and environment */
	MIPSContextAddArgsVector(context, argc, argv);
	MIPSContextAddEnviron(context, "");

	/* Get current directory */
	context->cwd = getcwd(buf, sizeof buf);
	if (!context->cwd)
		panic("%s: buffer too small", __FUNCTION__);

	/* Redirections */
	context->cwd = xstrdup(context->cwd);
	context->stdin_file = xstrdup("");
	context->stdout_file = xstrdup("");

	/* Load executable */
	mips_loader_debug("Loading executable\n");
	MIPSContextLoadExecutable(context, argv[0]);
}


void MIPSEmuLoadContextsFromIniFile(MIPSEmu *self,
		struct config_t *config, char *section)
{
}



/*
 * Public
 */


long long mips_emu_max_inst;
long long mips_emu_max_cycles;
long long mips_emu_max_time;
enum arch_sim_kind_t mips_emu_sim_kind = arch_sim_kind_functional;
