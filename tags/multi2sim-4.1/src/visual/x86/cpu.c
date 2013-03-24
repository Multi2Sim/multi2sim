/*
 *  Multi2Sim Tools
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
#include <gtk/gtk.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <visual/common/state.h>
#include <visual/common/trace.h>

#include "context.h"
#include "core.h"
#include "cpu.h"
#include "inst.h"


/*
 * Trace Line Processing Functions
 */


/* Command 'x86.map_ctx'
 * 	ctx=<pid>
 * 	core=<core_id>
 * 	thread=<thread_id>
 */
static void vi_x86_cpu_map_context(struct vi_x86_cpu_t *cpu,
	struct vi_trace_line_t *trace_line)
{
	struct vi_x86_context_t *context;
	struct vi_x86_core_t *core;

	int context_id;
	int core_id;
	int thread_id;
	int parent_id;

	char context_name[MAX_STRING_SIZE];

	/* Get fields */
	context_id = vi_trace_line_get_symbol_int(trace_line, "ctx");
	core_id = vi_trace_line_get_symbol_int(trace_line, "core");
	thread_id = vi_trace_line_get_symbol_int(trace_line, "thread");
	parent_id = vi_trace_line_get_symbol_int(trace_line, "ppid");

	/* If context does not exist, create it */
	snprintf(context_name, sizeof context_name, "ctx-%d", context_id);
	context = hash_table_get(vi_x86_cpu->context_table, context_name);
	if (!context)
	{
		context = vi_x86_context_create(context_name, context_id, parent_id);
		if (!hash_table_insert(vi_x86_cpu->context_table, context_name, context))
			panic("%s: invalid context", __FUNCTION__);
	}

	/* Set fields */
	context->core_id = core_id;
	context->thread_id = thread_id;

	/* Get core */
	core = list_get(vi_x86_cpu->core_list, core_id);
	if (!core)
		panic("%s: invalid core", __FUNCTION__);

	/* Add to core */
	if (!hash_table_insert(core->context_table, context_name, VI_X86_CONTEXT_EMPTY))
		panic("%s: invalid context", __FUNCTION__);
}


/* Command 'x86.unmap_ctx'
 * 	ctx=<pid>
 * 	core=<core_id>
 * 	thread=<thread_id>
 */
static void vi_x86_cpu_unmap_context(struct vi_x86_cpu_t *cpu,
	struct vi_trace_line_t *trace_line)
{
	struct vi_x86_core_t *core;
	struct vi_x86_context_t *context;

	int context_id;
	int core_id;
	int thread_id;

	char context_name[MAX_STRING_SIZE];

	/* Get fields */
	context_id = vi_trace_line_get_symbol_int(trace_line, "ctx");
	core_id = vi_trace_line_get_symbol_int(trace_line, "core");
	thread_id = vi_trace_line_get_symbol_int(trace_line, "thread");

	/* Get core */
	core = list_get(vi_x86_cpu->core_list, core_id);
	if (!core)
		panic("%s: invalid core", __FUNCTION__);

	/* Get context */
	snprintf(context_name, sizeof context_name, "ctx-%d", context_id);
	context = hash_table_get(vi_x86_cpu->context_table, context_name);
	if (!context || context->core_id != core_id || context->thread_id != thread_id)
		panic("%s: invalid context", __FUNCTION__);

	/* Remove from core */
	if (!hash_table_remove(core->context_table, context_name))
		panic("%s: invalid context", __FUNCTION__);

}


/* Command 'x86.end_ctx'
 *	ctx=<pid>
 */
static void vi_x86_cpu_end_context(struct vi_x86_cpu_t *cpu,
	struct vi_trace_line_t *trace_line)
{
	struct vi_x86_context_t *context;

	char context_name[MAX_STRING_SIZE];

	int context_id;

	/* Get fields */
	context_id = vi_trace_line_get_symbol_int(trace_line, "ctx");

	/* Remove and free context */
	snprintf(context_name, sizeof context_name, "ctx-%d", context_id);
	context = hash_table_remove(vi_x86_cpu->context_table, context_name);
	if (!context)
		panic("%s: invalid context", __FUNCTION__);
	vi_x86_context_free(context);
}


/* Command 'x86.new_inst'
 *	id=<id>
 *	core=<core_id>
 *	spec="t" (optional)
 *	asm=<asm_code>
 *	uasm=<asm_micro_code>
 *	stg=<stage>
 */
static void vi_x86_cpu_new_inst(struct vi_x86_cpu_t *cpu,
	struct vi_trace_line_t *trace_line)
{
	struct vi_x86_core_t *core;
	struct vi_x86_inst_t *inst;

	long long id;

	int core_id;
	int spec_mode;

	char *asm_code;
	char *asm_micro_code;

	char name[MAX_STRING_SIZE];

	enum vi_x86_inst_stage_t stage;

	/* Fields */
	id = vi_trace_line_get_symbol_long_long(trace_line, "id");
	core_id = vi_trace_line_get_symbol_int(trace_line, "core");
	asm_code = vi_trace_line_get_symbol(trace_line, "asm");
	asm_micro_code = vi_trace_line_get_symbol(trace_line, "uasm");
	stage = str_map_string(&vi_x86_inst_stage_map, vi_trace_line_get_symbol(trace_line, "stg"));
	spec_mode = !strcmp(vi_trace_line_get_symbol(trace_line, "spec"), "t");

	/* Create instruction */
	snprintf(name, sizeof name, "i-%lld", id);
	inst = vi_x86_inst_create(id, name, asm_code, asm_micro_code, spec_mode, stage);

	/* Get core */
	core = list_get(vi_x86_cpu->core_list, core_id);
	if (!core)
		panic("%s: invalid core", __FUNCTION__);

	/* Insert in table */
	core->num_insts = MAX(core->num_insts, id + 1);
	if (!hash_table_insert(core->inst_table, name, inst))
		panic("%s: invalid instruction", __FUNCTION__);
}


/* Command 'x86.inst'
 *	id=<id>
 *	core=<core_id>
 *	stg=<stage>
 */
static void vi_x86_cpu_inst(struct vi_x86_cpu_t *cpu,
	struct vi_trace_line_t *trace_line)
{
	struct vi_x86_inst_t *inst;
	struct vi_x86_core_t *core;

	enum vi_x86_inst_stage_t stage;

	long long id;

	int core_id;

	char name[MAX_STRING_SIZE];

	/* Fields */
	id = vi_trace_line_get_symbol_long_long(trace_line, "id");
	core_id = vi_trace_line_get_symbol_int(trace_line, "core");
	stage = str_map_string(&vi_x86_inst_stage_map, vi_trace_line_get_symbol(trace_line, "stg"));

	/* Get core */
	core = list_get(vi_x86_cpu->core_list, core_id);
	if (!core)
		panic("%s: invalid core", __FUNCTION__);

	/* Get instruction */
	snprintf(name, sizeof name, "i-%lld", id);
	inst = hash_table_get(core->inst_table, name);
	if (!inst)
		panic("%s: invalid instruction - %s", __FUNCTION__, name);

	/* Update stage */
	inst->stage = stage;
}


/* Command 'x86.end_inst'
 *	id=<id>
 *	core=<core_id>
 */
static void vi_x86_cpu_end_inst(struct vi_x86_cpu_t *cpu,
	struct vi_trace_line_t *trace_line)
{
	struct vi_x86_inst_t *inst;
	struct vi_x86_core_t *core;

	long long id;

	int core_id;

	char name[MAX_STRING_SIZE];

	/* Fields */
	id = vi_trace_line_get_symbol_long_long(trace_line, "id");
	core_id = vi_trace_line_get_symbol_int(trace_line, "core");

	/* Get core */
	core = list_get(vi_x86_cpu->core_list, core_id);
	if (!core)
		panic("%s: invalid core", __FUNCTION__);

	/* Free instruction */
	snprintf(name, sizeof name, "i-%lld", id);
	inst = hash_table_remove(core->inst_table, name);
	if (!inst)
		panic("%s: invalid instruction - %s", __FUNCTION__, name);
	vi_x86_inst_free(inst);
}


static void vi_x86_cpu_read_checkpoint(struct vi_x86_cpu_t *cpu, FILE *f)
{
	struct vi_x86_core_t *core;
	struct vi_x86_context_t *context;

	int core_id;
	int num_contexts;
	int count;
	int i;

	char *context_name;

	/* Clear list of contexts */
	HASH_TABLE_FOR_EACH(vi_x86_cpu->context_table, context_name, context)
		vi_x86_context_free(context);
	hash_table_clear(vi_x86_cpu->context_table);

	/* Number of contexts */
	count = fread(&num_contexts, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot read checkpoint", __FUNCTION__);

	/* Contexts */
	for (i = 0; i < num_contexts; i++)
	{
		context = vi_x86_context_create(NULL, 0, 0);
		vi_x86_context_read_checkpoint(context, f);
		if (!hash_table_insert(vi_x86_cpu->context_table, context->name, context))
			panic("%s: invalid context", __FUNCTION__);
	}

	/* Cores */
	LIST_FOR_EACH(vi_x86_cpu->core_list, core_id)
	{
		core = list_get(vi_x86_cpu->core_list, core_id);
		vi_x86_core_read_checkpoint(core, f);
	}
}


static void vi_x86_cpu_write_checkpoint(struct vi_x86_cpu_t *cpu, FILE *f)
{
	struct vi_x86_core_t *core;
	struct vi_x86_context_t *context;

	int core_id;
	int count;
	int num_contexts;

	char *context_name;

	/* Number of contexts */
	num_contexts = hash_table_count(vi_x86_cpu->context_table);
	count = fwrite(&num_contexts, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot read checkpoint", __FUNCTION__);

	/* Contexts */
	HASH_TABLE_FOR_EACH(vi_x86_cpu->context_table, context_name, context)
		vi_x86_context_write_checkpoint(context, f);

	/* Cores */
	LIST_FOR_EACH(vi_x86_cpu->core_list, core_id)
	{
		core = list_get(vi_x86_cpu->core_list, core_id);
		vi_x86_core_write_checkpoint(core, f);
	}
}




/*
 * Public Functions
 */


struct vi_x86_cpu_t *vi_x86_cpu;

/* Version of the x86 trace consumer. The major version must match with the
 * trace producer, while the minor version should be equal or higher. See
 * 'glut.c' for the version number assignment, and code modification policies.
 * See 'src/arch/x86/timing/cpu.c' for x86 trace producer version.
 */

#define VI_X86_TRACE_VERSION_MAJOR	1
#define VI_X86_TRACE_VERSION_MINOR	671

static char *err_vi_x86_trace_version =
	"\tThe x86 trace file has been created with an incompatible version\n"
	"\tof Multi2Sim. Please rerun the simulation with the same Multi2Sim\n"
	"\tversion used to visualize the trace.\n";

void vi_x86_cpu_init(void)
{
	struct vi_trace_line_t *trace_line;

	/* Initialize */
	vi_x86_cpu = xcalloc(1, sizeof(struct vi_x86_cpu_t));
	vi_x86_cpu->core_list = list_create();
	vi_x86_cpu->context_table = hash_table_create(0, FALSE);

	/* State file */
	vi_state_new_category("x86 CPU",
		(vi_state_read_checkpoint_func_t) vi_x86_cpu_read_checkpoint,
		(vi_state_write_checkpoint_func_t) vi_x86_cpu_write_checkpoint,
		vi_x86_cpu);

	/* Commands */
	vi_state_new_command("x86.map_ctx",
		(vi_state_process_trace_line_func_t) vi_x86_cpu_map_context,
		vi_x86_cpu);
	vi_state_new_command("x86.unmap_ctx",
		(vi_state_process_trace_line_func_t) vi_x86_cpu_unmap_context,
		vi_x86_cpu);
	vi_state_new_command("x86.end_ctx",
		(vi_state_process_trace_line_func_t) vi_x86_cpu_end_context,
		vi_x86_cpu);
	vi_state_new_command("x86.new_inst",
		(vi_state_process_trace_line_func_t) vi_x86_cpu_new_inst,
		vi_x86_cpu);
	vi_state_new_command("x86.inst",
		(vi_state_process_trace_line_func_t) vi_x86_cpu_inst,
		vi_x86_cpu);
	vi_state_new_command("x86.end_inst",
		(vi_state_process_trace_line_func_t) vi_x86_cpu_end_inst,
		vi_x86_cpu);

	/* Parse header in state file */
	VI_STATE_FOR_EACH_HEADER(trace_line)
	{
		char *command;

		/* Get command */
		command = vi_trace_line_get_command(trace_line);
		assert(strcmp(command, "c"));

		if (!strcmp(command, "x86.init"))
		{
			struct vi_x86_core_t *core;

			int num_cores;
			int i;

			char name[MAX_STRING_SIZE];

			char *version;

			int version_major = 0;
			int version_minor = 0;

			/* Trace contains x86 */
			vi_x86_cpu->active = 1;

			/* Check version compatibility */
			version = vi_trace_line_get_symbol(trace_line, "version");
			if (version)
				sscanf(version, "%d.%d", &version_major, &version_minor);
			if (version_major != VI_X86_TRACE_VERSION_MAJOR ||
				version_minor > VI_X86_TRACE_VERSION_MINOR)
				fatal("incompatible x86 trace version.\n"
					"\tTrace generation v. %d.%d / Trace consumer v. %d.%d\n%s",
					version_major, version_minor, VI_X86_TRACE_VERSION_MAJOR,
					VI_X86_TRACE_VERSION_MINOR, err_vi_x86_trace_version);

			/* Create cores */
			num_cores = vi_trace_line_get_symbol_int(trace_line, "num_cores");
			for (i = 0; i < num_cores; i++)
			{
				snprintf(name, sizeof name, "Core-%d", i);
				core = vi_x86_core_create(name);
				list_add(vi_x86_cpu->core_list, core);
			}
		}
	}
}


void vi_x86_cpu_done(void)
{
	int i;

	struct vi_x86_context_t *context;

	char *context_name;

	/* Free cores */
	LIST_FOR_EACH(vi_x86_cpu->core_list, i)
		vi_x86_core_free(list_get(vi_x86_cpu->core_list, i));
	list_free(vi_x86_cpu->core_list);

	/* Free contexts */
	HASH_TABLE_FOR_EACH(vi_x86_cpu->context_table, context_name, context)
		vi_x86_context_free(context);
	hash_table_free(vi_x86_cpu->context_table);

	/* Free CPU */
	free(vi_x86_cpu);
}
