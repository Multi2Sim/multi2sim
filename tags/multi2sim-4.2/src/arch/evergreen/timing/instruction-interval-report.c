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


#include <arch/evergreen/emu/ndrange.h>
#include <arch/evergreen/emu/wavefront.h>
#include <arch/evergreen/emu/work-group.h>
#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/string.h>

#include "gpu.h"
#include "instruction-interval-report.h"
#include "uop.h"


/*
 * Public variables
 */

int evg_periodic_report_active;



/*
 * Private variables
 */

static char *evg_periodic_report_file_name;
static int evg_periodic_report_interval = 1000;

static enum evg_periodic_report_scope_t
{
	evg_periodic_report_scope_invalid = 0,

	evg_periodic_report_scope_first_wavefront_first_workgroup,
	evg_periodic_report_scope_first_wavefront_all_workgroups,
	evg_periodic_report_scope_all_wavefronts_first_workgroup,
	evg_periodic_report_scope_all_wavefronts_all_workgroups,

	evg_periodic_report_scope_count
} evg_periodic_report_scope;

static struct str_map_t evg_periodic_report_scope_map =
{
	evg_periodic_report_scope_count - 1,
	{
		{ "FirstWavefrontFirstWorkgroup", evg_periodic_report_scope_first_wavefront_first_workgroup },
		{ "FirstWavefrontAllWorkgroups", evg_periodic_report_scope_first_wavefront_all_workgroups },
		{ "AllWavefrontsFirstWorkgroup", evg_periodic_report_scope_all_wavefronts_first_workgroup },
		{ "AllWavefrontsAllWorkgroups", evg_periodic_report_scope_all_wavefronts_all_workgroups }
	}
};

static char *evg_periodic_report_scope_str = "FirstWavefrontFirstWorkgroup";

static char *evg_periodic_report_intro =
	"The fields contained in each record are listed next, in the same order of\n"
	"occurrence:\n"
	"\n"
	"<vliw_bundles>\n"
	"\tTotal number of instructions executed so far, considering a ALU instruction\n"
	"\tforming a VLIW bundle as one instruction. This value is a multiple of the\n"
	"\tinterval specified in the periodic report configuration.\n"
	"\n"
	"<cycles>\n"
	"\tNumber of cycles of that the interval took to execute.\n"
	"\n"
	"<vliw_slots>\n"
	"\tNumber of instructions executed in the interval, considering each slot in an\n"
	"\tALU VLIW bundle as a single instruction. This value should be equal or greater\n"
	"\tthan the interval specified in the periodic report configuration.\n"
	"\n"
	"<local_mem_accesses>\n"
	"\tNumber of local memory accesses performed in the interval, adding up all\n"
	"\taccesses performed by all work-items in the wavefront.\n"
	"\n"
	"<global_mem_reads>\n"
	"\tNumber of global memory reads performed in the interval, adding up all\n"
	"\taccesses performed by all work-items in the wavefront.\n"
	"\n"
	"<global_mem_writes>\n"
	"\tNumber of global memory writes performed in the interval, adding up all\n"
	"\taccesses performed by all work-items in the wavefront.\n"
	"\n";



/*
 * Public Functions
 */

void evg_periodic_report_init(void)
{
}


void evg_periodic_report_done(void)
{
	str_free(evg_periodic_report_file_name);
}


static char *evg_periodic_report_section_name = "PeriodicReport";

void evg_periodic_report_config_read(struct config_t *config)
{
	char *section;
	char *file_name;

	/* Nothing if section is not present */
	section = evg_periodic_report_section_name;
	if (!config_section_exists(config, section))
		return;

	/* Periodic reports are active */
	evg_periodic_report_active = 1;

	/* Interval */
	evg_periodic_report_interval = config_read_int(config, section,
		"Interval", evg_periodic_report_interval);

	/* File name */
	config_var_enforce(config, section, "File");
	file_name = config_read_string(config, section, "File", NULL);
	if (!file_name || !*file_name)
		fatal("%s: %s: invalid or missing value for 'File'",
			evg_gpu_config_file_name, section);
	evg_periodic_report_file_name = str_set(NULL, file_name);

	/* Wavefront set */
	evg_periodic_report_scope_str = config_read_string(config, section,
		"Scope", evg_periodic_report_scope_str);
	evg_periodic_report_scope = str_map_string(&evg_periodic_report_scope_map,
		evg_periodic_report_scope_str);
	if (!evg_periodic_report_scope)
		fatal("%s: [%s]: invalid value for 'Scope'",
			evg_gpu_config_file_name, section);
}


/* Initialize periodic report fields in a wavefront when the work-group
 * containing this wavefront is mapped to the compute unit.
 * This function must be called only if the periodic report has been activated
 * (evg_periodic_report_active = 1). */
void evg_periodic_report_wavefront_init(struct evg_wavefront_t *wavefront)
{
	struct evg_work_group_t *work_group = wavefront->work_group;
	char file_name[MAX_STRING_SIZE];

	/* Decide if wavefront should dump report, depending on the variable
	 * 'Scope' in the configuration file. */
	assert(evg_periodic_report_active);
	switch (evg_periodic_report_scope)
	{
	case evg_periodic_report_scope_first_wavefront_first_workgroup:

		if (wavefront->id)
			return;
		break;

	case evg_periodic_report_scope_first_wavefront_all_workgroups:

		if (wavefront->id_in_work_group)
			return;
		break;

	case evg_periodic_report_scope_all_wavefronts_first_workgroup:

		if (work_group->id)
			return;
		break;

	case evg_periodic_report_scope_all_wavefronts_all_workgroups:

		break;

	default:
		panic("%s: invalid scope", __FUNCTION__);
		break;
	}

	/* Create file for report */
	assert(evg_periodic_report_file_name);
	snprintf(file_name, sizeof file_name, "%s-wg%d-wf%d", evg_periodic_report_file_name,
		work_group->id, wavefront->id_in_work_group);
	wavefront->periodic_report_file = file_open_for_write(file_name);
	if (!wavefront->periodic_report_file)
		fatal("%s: could not open periodic report file", file_name);

	/* Record initial cycle */
	wavefront->periodic_report_cycle = asTiming(evg_gpu)->cycle;
}


/* Finalization actions related with periodic report when the work-group
 * containing this wavefront is unmapped from the compute unit.
 * This function must be called only if the periodic report has been activated
 * (evg_periodic_report_active = 1). */
void evg_periodic_report_wavefront_done(struct evg_wavefront_t *wavefront)
{
	/* Ignore if this wavefront is not dumping report */
	assert(evg_periodic_report_active);
	if (!wavefront->periodic_report_file)
		return;

	/* Close report file */
	fclose(wavefront->periodic_report_file);
	wavefront->periodic_report_file = NULL;
}


void evg_periodic_report_dump_entry(struct evg_wavefront_t *wavefront)
{
	FILE *f = wavefront->periodic_report_file;
	int i;

	assert(evg_periodic_report_active);
	assert(f);

	/* First entry - dump intro */
	if (wavefront->periodic_report_vliw_bundle_count <= evg_periodic_report_interval)
	{
		fprintf(f, "Periodic Report for WG-%d/WF-%d (%s)\n\n",
			wavefront->work_group->id, wavefront->id_in_work_group,
			wavefront->name);
		fprintf(f, "%s", evg_periodic_report_intro);
		for (i = 0; i < 80; i++)
			fprintf(f, "-");
		fprintf(f, "\n");
	}

	/* Dump entry */
	fprintf(f, "%8lld ", wavefront->periodic_report_vliw_bundle_count);
	fprintf(f, "%5lld ", asTiming(evg_gpu)->cycle - wavefront->periodic_report_cycle);
	fprintf(f, "%5d ", wavefront->periodic_report_inst_count);
	fprintf(f, "%5d ", wavefront->periodic_report_local_mem_accesses);
	fprintf(f, "%5d ", wavefront->periodic_report_global_mem_reads);
	fprintf(f, "%5d ", wavefront->periodic_report_global_mem_writes);
	fprintf(f, "\n");

	/* Reset statistics */
	wavefront->periodic_report_cycle = asTiming(evg_gpu)->cycle;
	wavefront->periodic_report_inst_count = 0;
	wavefront->periodic_report_local_mem_accesses = 0;
	wavefront->periodic_report_global_mem_reads = 0;
	wavefront->periodic_report_global_mem_writes = 0;
}



/* Update periodic interval report statistics. This function is called every time
 * a new instruction is fetched in the CF, ALU, or TEX engines. It should be only
 * called if the periodic report has been activated (evg_periodic_report_active = 1).
 */
void evg_periodic_report_new_inst(struct evg_uop_t *uop)
{
	struct evg_wavefront_t *wavefront = uop->wavefront;
	struct evg_work_item_t *work_item;

	int work_item_id;

	/* Ignore if this wavefront is not dumping report */
	assert(evg_periodic_report_active);
	if (!wavefront->periodic_report_file)
		return;

	/* Track number of VLIW slots (or non-ALU instructions) */
	wavefront->periodic_report_inst_count += uop->vliw_slots;


	/* Number of local memory accesses performed by this uop */
	if (uop->local_mem_read || uop->local_mem_write)
	{
		EVG_FOREACH_WORK_ITEM_IN_WAVEFRONT(wavefront, work_item_id)
		{
			work_item = evg_gpu->ndrange->work_items[work_item_id];
			wavefront->periodic_report_local_mem_accesses += work_item->local_mem_access_count;
		}
	}

	/* FIXME: if we decide to fix the read stage to access global memory only when
	 * a work-item is active, these counters need to be updated accordingly. */
	/* Number of Global memory read accesses performed by this uop */
	if (uop->global_mem_read)
		wavefront->periodic_report_global_mem_reads += wavefront->work_item_count;

	/* Number of Global memory write accesses performed by this uop */
	if (uop->global_mem_write)
		wavefront->periodic_report_global_mem_writes += wavefront->work_item_count;

	/* Dump report entry if interval reached */
	wavefront->periodic_report_vliw_bundle_count++;
	if (!(wavefront->periodic_report_vliw_bundle_count % evg_periodic_report_interval) || uop->wavefront_last)
		evg_periodic_report_dump_entry(wavefront);
}
