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

#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <mem-system/mem-system.h>

#include "mem-config.h"
#include "timing.h"


void si_mem_config_default(struct config_t *config)
{
}


void si_mem_config_parse_entry(struct config_t *config, char *section)
{
	char *file_name;
	char *module_name;

	int compute_unit_id;

	struct si_compute_unit_t *compute_unit;

	/* Get configuration file name */
	file_name = config_get_file_name(config);

	/* Allow these sections in case we quit before reading them. */
	config_var_allow(config, section, "Module");

	/* Read compute unit */
	compute_unit_id = config_read_int(config, section, "ComputeUnit", -1);
	if (compute_unit_id < 0)
		fatal("%s: section [%s]: invalid or missing value for 'ComputeUnit'",
			file_name, section);

	/* Check compute unit boundaries */
	if (compute_unit_id >= si_gpu_num_compute_units)
	{
		warning("%s: section [%s] ignored, referring to Southern Islands compute unit %d.\n"
			"\tThis section refers to a compute unit that does not currently exist.\n"
			"\tPlease review your Southern Islands configuration file if this is not the\n"
			"\tdesired behavior.\n",
			file_name, section, compute_unit_id);
		return;
	}

	/* Check that entry has not been assigned before */
	compute_unit = si_gpu->compute_units[compute_unit_id];
	if (compute_unit->global_memory)
		fatal("%s: section [%s]: entry from compute unit %d already assigned.\n"
			"\tA different [Entry <name>] section in the memory configuration file has already\n"
			"\tassigned an entry for this particular compute unit. Please review your\n"
			"\tconfiguration file to avoid duplicates.\n",
			file_name, section, compute_unit_id);

	/* Read module */
	module_name = config_read_string(config, section, "Module", NULL);
	if (!module_name)
		fatal("%s: section [%s]: variable 'Module' missing.\n"
			"\tPlease run use '--help-mem-config' for more information on the\n"
			"\tconfiguration file format, or consult the Multi2Sim Guide.\n",
			file_name, section);
	
	/* Assign module */
	compute_unit->global_memory = mem_system_get_mod(module_name);
	if (!compute_unit->global_memory)
		fatal("%s: section [%s]: '%s' is not a valid module name.\n"
			"\tThe given module name must match a module declared in a section\n"
			"\t[Module <name>] in the memory configuration file.\n",
			file_name, section, module_name);
	
	/* Debug */
	mem_debug("\tSouthern Islands compute unit %d\n", compute_unit_id);
	mem_debug("\t\tEntry -> %s\n", compute_unit->global_memory->name);
	mem_debug("\n");
}


void si_mem_config_check(struct config_t *config)
{
	struct si_compute_unit_t *compute_unit;
	int compute_unit_id;
	char *file_name;

	/* Check that all compute units have an entry to the memory hierarchy. */
	file_name = config_get_file_name(config);
	SI_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		compute_unit = si_gpu->compute_units[compute_unit_id];
		if (!compute_unit->global_memory)
			fatal("%s: Southern Islands compute unit %d has no entry to memory.\n"
				"\tPlease add a new [Entry <name>] section in your memory configuration\n"
				"\tfile to associate this compute unit with a memory module.\n",
				file_name, compute_unit_id);
	}
}

