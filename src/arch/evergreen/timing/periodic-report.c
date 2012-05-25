/*
 *  Multi2Sim
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

#include <evergreen-timing.h>


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

static struct string_map_t evg_periodic_report_scope_map =
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
	evg_periodic_report_scope = map_string(&evg_periodic_report_scope_map,
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
	}

	/* Create file for report */
	assert(evg_periodic_report_file_name);
	snprintf(file_name, sizeof file_name, "%s-wg%d-wf%d", evg_periodic_report_file_name,
		work_group->id, wavefront->id_in_work_group);
	wavefront->periodic_report_file = open_write(file_name);
	if (!wavefront->periodic_report_file)
		fatal("%s: could not open periodic report file", file_name);
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


/* Update periodic interval report statistics. This function is called every time
 * a new instruction is fetched in the CF, ALU, or TEX engines. It should be only
 * called if the periodic report has been activated (evg_periodic_report_active = 1).
 */
void evg_periodic_report_new_inst(struct evg_uop_t *uop)
{
	struct evg_wavefront_t *wavefront = uop->wavefront;

	/* Ignore if this wavefront is not dumping report */
	assert(evg_periodic_report_active);
	if (!wavefront->periodic_report_file)
		return;

	/* Increase number of instructions, and ignore call if counter is not a
	 * multiple of the current interval. */
	wavefront->periodic_report_inst_count++;
	if (wavefront->periodic_report_inst_count % evg_periodic_report_interval)
		return;

	/* Record statistics */
	wavefront->periodic_report_inst_count = 0;
	wavefront->periodic_report_cycle = evg_gpu->cycle;
}
