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
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/string.h>
#include <lib/util/timer.h>

#include "arch.h"
#include "asm.h"
#include "emu.h"
#include "timing.h"


/*
 * Object 'arch_t' Functions
 */

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

	/* Return */
	return arch;
}


void arch_free(struct arch_t *arch)
{
	free(arch->name);
	free(arch->prefix);
	linked_list_free(arch->mem_entry_mod_list);
	free(arch);
}


void arch_dump(struct arch_t *arch, FILE *f)
{
	double time_in_sec;
	int i;

	Emu *emu;
	Timing *timing;

	/* Get objects */
	emu = arch->emu;
	timing = arch->timing;

	/* Nothing to print if architecture was not active */
	if (!emu->instructions)
		return;

	/* Header */
	for (i = 0; i < 80; i++)
		fprintf(f, "=");
	fprintf(f, "\nArchitecture '%s'\n", arch->name);
	for (i = 0; i < 80; i++)
		fprintf(f, "=");
	fprintf(f, "\n\n");

	/* Emulator */
	time_in_sec = (double) m2s_timer_get_value(emu->timer) / 1.0e6;
	fprintf(f, "SimKind = %s\n", str_map_value(&arch_sim_kind_map, arch->sim_kind));
	fprintf(f, "Time = %.2f\n", time_in_sec);
	fprintf(f, "Instructions = %lld\n", emu->instructions);
	fprintf(f, "\n");
	assert(emu->DumpSummary);
	emu->DumpSummary(emu, f);

	/* Continue with timing simulator only it active */
	if (arch->sim_kind == arch_sim_kind_functional)
		return;

	/* Timing simulator */
	fprintf(f, "Cycles = %lld\n", timing->cycle);
	fprintf(f, "\n");
	assert(timing->DumpSummary);
	timing->DumpSummary(timing, f);
}


void arch_dump_summary(struct arch_t *arch, FILE *f)
{
	Emu *emu = arch->emu;
	Timing *timing = arch->timing;

	/* If no instruction was run for this architecture, skip
	 * statistics summary. */
	if (!emu->instructions)
		return;

	/* Architecture-specific emulation statistics */
	assert(emu->DumpSummary);
	emu->DumpSummary(emu, f);

	/* Timing simulation statistics */
	if (arch->sim_kind == arch_sim_kind_detailed)
	{
		/* Architecture-specific */
		assert(timing->DumpSummary);
		timing->DumpSummary(timing, f);
	}

	/* End */
	fprintf(f, "\n");
}


void arch_set_asm(struct arch_t *arch, Asm *as)
{
	arch->as = as;
	if (as)
		as->arch = arch;
}


void arch_set_emu(struct arch_t *arch, Emu *emu)
{
	arch->emu = emu;
	if (emu)
		emu->arch = arch;
}


void arch_set_timing(struct arch_t *arch, Timing *timing)
{
	arch->timing = timing;
	if (timing)
		timing->arch = arch;
}




/*
 * Global Variables
 */

struct arch_t *arch_arm;
struct arch_t *arch_evergreen;
struct arch_t *arch_fermi;
struct arch_t *arch_mips;
struct arch_t *arch_southern_islands;
struct arch_t *arch_x86;




/*
 * Public Functions
 */

#define ARCH_LIST_MAX_SIZE  10

/* List defined as a vector for efficiency in simulation loop. We avoid using
 * the standard 'list_t' or 'linked_list_t' structures here. */
static struct arch_t *arch_list[ARCH_LIST_MAX_SIZE];
static int arch_list_count;  /* Number of registered architectures */


struct arch_t *arch_register(char *name, char *prefix,
		enum arch_sim_kind_t sim_kind,
		arch_emu_init_func_t emu_init_func,
		arch_emu_done_func_t emu_done_func,
		arch_timing_read_config_func_t timing_read_config_func,
		arch_timing_init_func_t timing_init_func,
		arch_timing_done_func_t timing_done_func)
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
	assert(sim_kind);
	arch = arch_create(name, prefix);
	arch->sim_kind = sim_kind;
	arch->emu_init_func = emu_init_func;
	arch->emu_done_func = emu_done_func;
	arch->timing_read_config_func = timing_read_config_func;
	arch->timing_init_func = timing_init_func;
	arch->timing_done_func = timing_done_func;

	/* Add architecture and return it */
	arch_list[arch_list_count++] = arch;
	return arch;
}


void arch_init(void)
{
	struct arch_t *arch;
	int i;

	/* Initialize all architectures */
	for (i = 0; i < arch_list_count; i++)
	{
		/* Get architecture */
		arch = arch_list[i];

		/* Timing simulation */
		if (arch->sim_kind == arch_sim_kind_detailed)
		{
			/* Read configuration file */
			if (arch->timing_read_config_func)
				arch->timing_read_config_func();

		}

		/* Initialize emulator */
		if (arch->emu_init_func)
			arch->emu_init_func();

		/* Initialize timing simulator */
		if (arch->sim_kind == arch_sim_kind_detailed)
		{
			/* Register frequency domain */
			if (arch->timing_init_func)
				arch->timing_init_func();
		}
	}
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
		{
			if (arch->timing_done_func)
				arch->timing_done_func();
		}
		if (arch->emu_done_func)
			arch->emu_done_func();

		/* Free architecture */
		arch_free(arch);
	}
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


void arch_run(int *num_emu_active_ptr, int *num_timing_active_ptr)
{
	struct arch_t *arch;
	long long cycle;

	int run;
	int i;

	Emu *emu;
	Timing *timing;

	/* Reset active emulation and timing simulation counters */
	*num_emu_active_ptr = 0;
	*num_timing_active_ptr = 0;

	/* Run one iteration for all architectures */
	for (i = 0; i < arch_list_count; i++)
	{
		/* Get architecture */
		arch = arch_list[i];
		if (arch->sim_kind == arch_sim_kind_functional)
		{
			/* Emulation iteration */
			emu = arch->emu;
			assert(emu && emu->Run);
			arch->active = emu->Run(emu);

			/* Increase number of active emulations if the architecture
			 * actually performed a useful emulation iteration. */
			*num_emu_active_ptr += arch->active;
		}
		else
		{
			/* Check whether the architecture should actually run an
			 * iteration. If it is working at a slower frequency than
			 * the main simulation loop, we must skip this call. */
			timing = arch->timing;
			assert(timing);
			assert(timing->frequency_domain);
			cycle = esim_domain_cycle(timing->frequency_domain);
			run = cycle != arch->last_timing_cycle;

			/* Timing simulation iteration */
			if (run)
			{
				/* Do it... */
				arch->active = timing->Run(timing);

				/* ... but only update the last timing
				 * simulation cycle if there was an effective
				 * execution of the iteration loop. Otherwise,
				 * there is a deadlock: 'esim_time' will not
				 * advance (no call to 'esim_process_events')
				 * because no architecture ran, and no
				 * architecture will run because 'esim_time'
				 * did not advance. */
				if (arch->active)
					arch->last_timing_cycle = cycle;
			}

			/* Increase number of active timing simulations if the
			 * architecture actually performance a useful iteration. */
			*num_timing_active_ptr += arch->active;
		}
	}
}
