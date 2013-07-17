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
#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/string.h>
#include <mem-system/mem-system.h>

#include "compute-unit.h"
#include "gpu.h"
#include "mem-config.h"


void EvgGpuMemConfigDefault(Timing *self, struct config_t *config)
{
	char section[MAX_STRING_SIZE];
	char str[MAX_STRING_SIZE];

	int compute_unit_id;
	int l2_id;
	int mm_id;

	/* Cache geometry for L1 */
	snprintf(section, sizeof section, "CacheGeometry evg-geo-l1");
	config_write_int(config, section, "Sets", 32);
	config_write_int(config, section, "Assoc", 4);
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 1);
	config_write_string(config, section, "Policy", "LRU");

	/* Cache geometry for L2 */
	snprintf(section, sizeof section, "CacheGeometry evg-geo-l2");
	config_write_int(config, section, "Sets", 256);
	config_write_int(config, section, "Assoc", 8);
	config_write_int(config, section, "BlockSize", 256);
	config_write_int(config, section, "Latency", 10);
	config_write_string(config, section, "Policy", "LRU");

	/* L1 caches and entries */
	EVG_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		/* L1 cache */
		snprintf(section, sizeof section, "Module evg-l1-%d", compute_unit_id);
		config_write_string(config, section, "Type", "Cache");
		config_write_string(config, section, "Geometry", "evg-geo-l1");
		config_write_string(config, section, "LowNetwork", "evg-net-l1-l2");
		config_write_string(config, section, "LowModules", "evg-l2-0 evg-l2-1 evg-l2-2 evg-l2-3");

		/* Entry */
		snprintf(section, sizeof section, "Entry evg-cu-%d", compute_unit_id);
		snprintf(str, sizeof str, "evg-l1-%d", compute_unit_id);
		config_write_string(config, section, "Arch", "Evergreen");
		config_write_int(config, section, "ComputeUnit", compute_unit_id);
		config_write_string(config, section, "Module", str);
	}

	/* L2 caches */
	for (l2_id = 0; l2_id < 4; l2_id++) 
	{
		snprintf(section, sizeof section, "Module evg-l2-%d", l2_id);
		config_write_string(config, section, "Type", "Cache");
		config_write_string(config, section, "Geometry", "evg-geo-l2");
		config_write_string(config, section, "HighNetwork", "evg-net-l1-l2");
		config_write_string(config, section, "LowNetwork", "evg-net-l2-gm");
		config_write_string(config, section, "LowModules", "evg-gm-0 evg-gm-1 evg-gm-2 evg-gm-3");

		snprintf(str, sizeof str, "ADDR DIV 256 MOD 4 EQ %d", l2_id);
		config_write_string(config, section, "AddressRange", str);
	}

	/* Global memory */
	for (mm_id = 0; mm_id < 4; mm_id++) 
	{
		snprintf(section, sizeof section, "Module evg-gm-%d", mm_id);
		config_write_string(config, section, "Type", "MainMemory");
		config_write_string(config, section, "HighNetwork", "evg-net-l2-gm");
		config_write_int(config, section, "BlockSize", 256);
		config_write_int(config, section, "Latency", 100);

		snprintf(str, sizeof str, "ADDR DIV 256 MOD 4 EQ %d", mm_id);
		config_write_string(config, section, "AddressRange", str);
	}

	/* Network connecting L1 caches and L2 */
	snprintf(section, sizeof section, "Network evg-net-l1-l2");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);

	/* Network connecting L2 cache and global memory */
	snprintf(section, sizeof section, "Network evg-net-l2-gm");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);
}


void EvgGpuMemConfigParseEntry(Timing *self, struct config_t *config,
		char *section)
{
	char *file_name;
	char *module_name;

	int compute_unit_id;

	struct evg_compute_unit_t *compute_unit;

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
	if (compute_unit_id >= evg_gpu_num_compute_units)
	{
		warning("%s: section [%s] ignored, referring to Evergreen compute unit %d.\n"
			"\tThis section refers to a compute unit that does not currently exist.\n"
			"\tPlease review your Evergreen configuration file if this is not the\n"
			"\tdesired behavior.\n",
			file_name, section, compute_unit_id);
		return;
	}

	/* Check that entry has not been assigned before */
	compute_unit = evg_gpu->compute_units[compute_unit_id];
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
			"\tPlease run use '--mem-help' for more information on the\n"
			"\tconfiguration file format, or consult the Multi2Sim Guide.\n",
			file_name, section);
	
	/* Assign module */
	compute_unit->global_memory = mem_system_get_mod(module_name);
	if (!compute_unit->global_memory)
		fatal("%s: section [%s]: '%s' is not a valid module name.\n"
			"\tThe given module name must match a module declared in a section\n"
			"\t[Module <name>] in the memory configuration file.\n",
			file_name, section, module_name);
	
	/* Add modules to list of memory entries */
	linked_list_add(arch_evergreen->mem_entry_mod_list,
			compute_unit->global_memory);
	
	/* Debug */
	mem_debug("\tEvergreen compute unit %d\n", compute_unit_id);
	mem_debug("\t\tEntry -> %s\n", compute_unit->global_memory->name);
	mem_debug("\n");
}


void EvgGpuMemConfigCheck(Timing *self, struct config_t *config)
{
	struct evg_compute_unit_t *compute_unit;
	int compute_unit_id;
	char *file_name;

	/* Check that all compute units have an entry to the memory hierarchy. */
	file_name = config_get_file_name(config);
	EVG_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		compute_unit = evg_gpu->compute_units[compute_unit_id];
		if (!compute_unit->global_memory)
			fatal("%s: Evergreen compute unit %d has no entry to memory.\n"
				"\tPlease add a new [Entry <name>] section in your memory configuration\n"
				"\tfile to associate this compute unit with a memory module.\n",
				file_name, compute_unit_id);
	}
}

