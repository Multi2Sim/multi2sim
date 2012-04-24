/*
 *  Multi2Sim Tools
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <visual-x86.h>


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

	char context_name[MAX_STRING_SIZE];

	/* Get fields */
	context_id = vi_trace_line_get_symbol_int(trace_line, "ctx");
	core_id = vi_trace_line_get_symbol_int(trace_line, "core");
	thread_id = vi_trace_line_get_symbol_int(trace_line, "thread");

	/* If context does not exist, create it */
	snprintf(context_name, sizeof context_name, "ctx-%d", context_id);
	context = hash_table_get(vi_x86_cpu->context_table, context_name);
	if (!context)
	{
		context = vi_x86_context_create(context_name, context_id);
		hash_table_insert(vi_x86_cpu->context_table, context_name, context);
	}

	/* Set fields */
	context->core_id = core_id;
	context->thread_id = thread_id;

	/* Get core */
	core = list_get(vi_x86_cpu->core_list, core_id);
	if (!core)
		panic("%s: invalid core", __FUNCTION__);

	/* Add to core */
	if (!hash_table_insert(core->context_table, context_name, context))
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
}


/* Command 'x86.end_ctx'
 *	ctx=<pid>
 */
static void vi_x86_cpu_end_context(struct vi_x86_cpu_t *cpu,
	struct vi_trace_line_t *trace_line)
{
}


static void vi_x86_cpu_read_checkpoint(struct vi_x86_cpu_t *cpu, FILE *f)
{
}


static void vi_x86_cpu_write_checkpoint(struct vi_x86_cpu_t *cpu, FILE *f)
{
}




/*
 * Public Functions
 */


struct vi_x86_cpu_t *vi_x86_cpu;


void vi_x86_cpu_init(void)
{
	struct vi_trace_line_t *trace_line;

	/* Allocate */
	vi_x86_cpu = calloc(1, sizeof(struct vi_x86_cpu_t));
	if (!vi_x86_cpu)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
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
