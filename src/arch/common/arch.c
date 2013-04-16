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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/timer.h>

#include "arch.h"


struct str_map_t arch_sim_kind_map =
{
	2,
	{
		{ "functional", arch_sim_kind_functional },
		{ "detailed", arch_sim_kind_detailed }
	}
};


struct arch_t *arch_create(char *name, char *prefix)
{
	struct arch_t *arch;

	/* Initialize */
	arch = xcalloc(1, sizeof(struct arch_t));
	arch->name = xstrdup(name);
	arch->prefix = xstrdup(prefix);
	arch->mem_entry_mod_list = linked_list_create();
	arch->timer = m2s_timer_create(name);

	/* Return */
	return arch;
}


void arch_free(struct arch_t *arch)
{
	free(arch->name);
	free(arch->prefix);
	linked_list_free(arch->mem_entry_mod_list);
	m2s_timer_free(arch->timer);
	free(arch);
}


void arch_dump(struct arch_t *arch, FILE *f)
{
	double time_in_sec;
	int i;

	/* Nothing to print if architecture was not active */
	if (!arch->inst_count)
		return;

	/* Header */
	for (i = 0; i < 80; i++)
		fprintf(f, "=");
	fprintf(f, "\nArchitecture '%s'\n", arch->name);
	for (i = 0; i < 80; i++)
		fprintf(f, "=");
	fprintf(f, "\n\n");

	/* Emulator */
	time_in_sec = (double) m2s_timer_get_value(arch->timer) / 1.0e6;
	fprintf(f, "SimKind = %s\n", str_map_value(&arch_sim_kind_map, arch->sim_kind));
	fprintf(f, "Time = %.2f\n", time_in_sec);
	fprintf(f, "Instructions = %lld\n", arch->inst_count);
	fprintf(f, "\n");
	if (arch->emu_dump_func)
		arch->emu_dump_func(f);


	/* Continue with timing simulator only it active */
	if (arch->sim_kind == arch_sim_kind_functional)
		return;

	/* Timing simulator */
	fprintf(f, "Cycles = %lld\n", arch->cycle);
	fprintf(f, "\n");
	if (arch->timing_dump_func)
		arch->timing_dump_func(f);
}


void arch_dump_summary(struct arch_t *arch, FILE *f)
{
	double time_in_sec;
	double inst_per_sec;
	double cycles_per_sec;

	/* If no instruction was run for this architecture, skip
	 * statistics summary. */
	if (!arch->inst_count)
		return;

	/* Standard emulation statistics */
	time_in_sec = (double) m2s_timer_get_value(arch->timer) / 1.0e6;
	inst_per_sec = time_in_sec > 0.0 ? (double) arch->inst_count / time_in_sec : 0.0;
	fprintf(f, "[ %s ]\n", arch->name);
	fprintf(f, "Time = %.2f\n", time_in_sec);
	fprintf(f, "Instructions = %lld\n", arch->inst_count);
	fprintf(f, "InstructionsPerSecond = %.0f\n", inst_per_sec);

	/* Architecture-specific emulation statistics */
	arch->emu_dump_summary_func(f);

	/* Timing simulation statistics */
	if (arch->sim_kind == arch_sim_kind_detailed)
	{
		/* Standard */
		cycles_per_sec = time_in_sec > 0.0 ? (double) arch->cycle
				/ time_in_sec : 0.0;
		fprintf(f, "Cycles = %lld\n", arch->cycle);
		fprintf(f, "CyclesPerSecond = %.0f\n", cycles_per_sec);

		/* Architecture-specific */
		arch->timing_dump_summary_func(f);
	}

	/* End */
	fprintf(f, "\n");
}



/*
 * Architecture List
 */

#define ARCH_LIST_MAX_SIZE  10

/* List defined as a vector for efficiency in simulation loop. We avoid using
 * the standard 'list_t' or 'linked_list_t' structures here. */
static struct arch_t *arch_list[ARCH_LIST_MAX_SIZE];
static int arch_list_count;  /* Number of registered architectures */


void arch_init(void)
{
}


void arch_done(void)
{
	struct arch_t *arch;
	int i;

	/* Free all architectures */
	for (i = 0; i < arch_list_count; i++)
	{
		/* Get architecture */
		arch = arch_list[i];

		/* Free functional/timing simulator */
		if (arch->sim_kind == arch_sim_kind_detailed)
			arch->timing_done_func();
		arch->emu_done_func();

		/* Free architecture */
		arch_free(arch);
	}
}


void arch_register(char *name, char *prefix,
		enum arch_sim_kind_t sim_kind,
		arch_emu_init_func_t emu_init_func,
		arch_emu_done_func_t emu_done_func,
		arch_emu_dump_func_t emu_dump_func,
		arch_emu_dump_summary_func_t emu_dump_summary_func,
		arch_run_func_t emu_run_func,
		arch_timing_init_func_t timing_init_func,
		arch_timing_done_func_t timing_done_func,
		arch_timing_dump_func_t timing_dump_func,
		arch_timing_dump_summary_func_t timing_dump_summary_func,
		arch_run_func_t timing_run_func)
{
	struct arch_t *arch;

	/* Check no duplicates */
	arch = arch_get(name);
	if (arch)
		panic("%s: duplicated architecture", __FUNCTION__);

	/* Check list size */
	if (arch_list_count == ARCH_LIST_MAX_SIZE)
		panic("%s: architecture list too big - increase ARCH_LIST_MAX_SIZE",
				__FUNCTION__);

	/* Initialize */
	arch = arch_create(name, prefix);
	arch->sim_kind = sim_kind;
	arch->emu_init_func = emu_init_func;
	arch->emu_done_func = emu_done_func;
	arch->emu_dump_func = emu_dump_func;
	arch->emu_dump_summary_func = emu_dump_summary_func;
	arch->emu_run_func = emu_run_func;
	arch->timing_init_func = timing_init_func;
	arch->timing_done_func = timing_done_func;
	arch->timing_dump_func = timing_dump_func;
	arch->timing_dump_summary_func = timing_dump_summary_func;
	arch->timing_run_func = timing_run_func;

	/* Choose functional/timing simulation loop iteration function */
	assert(sim_kind);
	arch->run_func = sim_kind == arch_sim_kind_functional ?
			emu_run_func : timing_run_func;

	/* Add architecture */
	arch_list[arch_list_count++] = arch;

	/* Call functional/timing simulator initialization */
	emu_init_func(arch);
	if (sim_kind == arch_sim_kind_detailed)
		timing_init_func();
}


void arch_for_each(arch_callback_func_t callback_func, void *user_data)
{
	int i;

	/* Call function for all architectures */
	for (i = 0; i < arch_list_count; i++)
		callback_func(arch_list[i], user_data);
}


struct arch_t *arch_get(char *name)
{
	int i;

	/* Search architecture */
	for (i = 0; i < arch_list_count; i++)
		if (!strcasecmp(arch_list[i]->name, name))
			return arch_list[i];

	/* Not found */
	return NULL;
}


/* Get a list of all possible names for architectures. This function is usefull
 * to print an error message with the valid values for architectures when
 * processing user input. */
void arch_get_names(char *str, int size)
{
	char *comma = "";
	int i;

	str_printf(&str, &size, "{");
	for (i = 0; i < arch_list_count; i++)
	{
		str_printf(&str, &size, "%s%s", comma,
				arch_list[i]->name);
		comma = "|";
	}
	str_printf(&str, &size, "}");
}


int arch_get_sim_kind_detailed_count(void)
{
	int count;
	int i;

	count = 0;
	for (i = 0; i < arch_list_count; i++)
		if (arch_list[i]->sim_kind == arch_sim_kind_detailed)
			count++;
	return count;
}

enum arch_sim_kind_t arch_run_all(void)
{
	enum arch_sim_kind_t ret = arch_sim_kind_invalid;
	enum arch_sim_kind_t arch_ret;
	int i;

	/* Run one iteration for all architectures */
	for (i = 0; i < arch_list_count; i++)
	{
		arch_ret = arch_list[i]->run_func();
		ret = MAX(ret, arch_ret);
	}

	/* Return maximum simulation level performed */
	return ret;
}
