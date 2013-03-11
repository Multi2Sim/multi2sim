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

#include <arch/common/arch.h>
#include <arch/fermi/emu/emu.h>
#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/string.h>
#include <mem-system/mem-system.h>
#include <mem-system/module.h>

#include "mem-config.h"
#include "gpu.h"
#include "compute-unit.h"


void frm_mem_config_default(struct config_t *config)
{
	char section[MAX_STRING_SIZE];
	char str[MAX_STRING_SIZE];

	int sm_id;

	/* Only detailed simulation */
	assert(frm_emu_sim_kind == arch_sim_kind_detailed);

	/* Cache geometry for L1 */
	snprintf(section, sizeof section, "CacheGeometry si-geo-l1");
	config_write_int(config, section, "Sets", 64);
	config_write_int(config, section, "Assoc", 4);
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 1);
	config_write_string(config, section, "Policy", "LRU");

	/* Cache geometry for L2 */
	snprintf(section, sizeof section, "CacheGeometry si-geo-l2");
	config_write_int(config, section, "Sets", 128);
	config_write_int(config, section, "Assoc", 16);
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 10);
	config_write_string(config, section, "Policy", "LRU");

	/* L1 caches */
	FRM_GPU_FOREACH_COMPUTE_UNIT(sm_id)
	{
		/* L1 cache */
		snprintf(section, sizeof section, "Module si-l1-%d", sm_id);
		config_write_string(config, section, "Type", "Cache");
		config_write_string(config, section, "Geometry", "si-geo-l1");
		config_write_string(config, section, "LowNetwork", "si-net-l1-l2");
		config_write_string(config, section, "LowModules", 
			"si-l2-0 si-l2-1 si-l2-2 si-l2-3 si-l2-4 si-l2-5");

		/* Entry */
		snprintf(section, sizeof section, "Entry si-cu-%d", sm_id);
		snprintf(str, sizeof str, "si-l1-%d", sm_id);
		config_write_string(config, section, "Arch", "SouthernIslands");
		config_write_int(config, section, "ComputeUnit", sm_id);
		config_write_string(config, section, "Module", str);
	}

	/* L2 caches */
	snprintf(section, sizeof section, "Module si-l2-0");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "si-geo-l2");
	config_write_string(config, section, "HighNetwork", "si-net-l1-l2");
	config_write_string(config, section, "LowNetwork", "si-net-l2-0-gm-0");
	config_write_string(config, section, "LowModules", "si-gm-0");
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 0");

	snprintf(section, sizeof section, "Module si-l2-1");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "si-geo-l2");
	config_write_string(config, section, "HighNetwork", "si-net-l1-l2");
	config_write_string(config, section, "LowNetwork", "si-net-l2-1-gm-1");
	config_write_string(config, section, "LowModules", "si-gm-1");
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 1");
	
	snprintf(section, sizeof section, "Module si-l2-2");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "si-geo-l2");
	config_write_string(config, section, "HighNetwork", "si-net-l1-l2");
	config_write_string(config, section, "LowNetwork", "si-net-l2-2-gm-2");
	config_write_string(config, section, "LowModules", "si-gm-2");
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 2");

	snprintf(section, sizeof section, "Module si-l2-3");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "si-geo-l2");
	config_write_string(config, section, "HighNetwork", "si-net-l1-l2");
	config_write_string(config, section, "LowNetwork", "si-net-l2-3-gm-3");
	config_write_string(config, section, "LowModules", "si-gm-3");
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 3");

	snprintf(section, sizeof section, "Module si-l2-4");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "si-geo-l2");
	config_write_string(config, section, "HighNetwork", "si-net-l1-l2");
	config_write_string(config, section, "LowNetwork", "si-net-l2-4-gm-4");
	config_write_string(config, section, "LowModules", "si-gm-4");
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 4");

	snprintf(section, sizeof section, "Module si-l2-5");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "si-geo-l2");
	config_write_string(config, section, "HighNetwork", "si-net-l1-l2");
	config_write_string(config, section, "LowNetwork", "si-net-l2-5-gm-5");
	config_write_string(config, section, "LowModules", "si-gm-5");
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 5");

	/* Global memory */
	snprintf(section, sizeof section, "Module si-gm-0");
	config_write_string(config, section, "Type", "MainMemory");
	config_write_string(config, section, "HighNetwork", "si-net-l2-0-gm-0");
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 100);
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 0");

	snprintf(section, sizeof section, "Module si-gm-1");
	config_write_string(config, section, "Type", "MainMemory");
	config_write_string(config, section, "HighNetwork", "si-net-l2-1-gm-1");
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 100);
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 1");

	snprintf(section, sizeof section, "Module si-gm-2");
	config_write_string(config, section, "Type", "MainMemory");
	config_write_string(config, section, "HighNetwork", "si-net-l2-2-gm-2");
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 100);
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 2");

	snprintf(section, sizeof section, "Module si-gm-3");
	config_write_string(config, section, "Type", "MainMemory");
	config_write_string(config, section, "HighNetwork", "si-net-l2-3-gm-3");
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 100);
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 3");

	snprintf(section, sizeof section, "Module si-gm-4");
	config_write_string(config, section, "Type", "MainMemory");
	config_write_string(config, section, "HighNetwork", "si-net-l2-4-gm-4");
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 100);
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 4");

	snprintf(section, sizeof section, "Module si-gm-5");
	config_write_string(config, section, "Type", "MainMemory");
	config_write_string(config, section, "HighNetwork", "si-net-l2-5-gm-5");
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 100);
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 5");

	/* Network connecting L1s and L2s */
	snprintf(section, sizeof section, "Network si-net-l1-l2");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);

	/* Networks connecting memory controllers and global memory banks */
	snprintf(section, sizeof section, "Network si-net-l2-0-gm-0");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);

	snprintf(section, sizeof section, "Network si-net-l2-1-gm-1");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);

	snprintf(section, sizeof section, "Network si-net-l2-2-gm-2");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);

	snprintf(section, sizeof section, "Network si-net-l2-3-gm-3");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);

	snprintf(section, sizeof section, "Network si-net-l2-4-gm-4");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);

	snprintf(section, sizeof section, "Network si-net-l2-5-gm-5");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);
}


void frm_mem_config_parse_entry(struct config_t *config, char *section)
{
	char *file_name;
	char *module_name;

	int sm_id;

	struct frm_sm_t *sm;

	/* Get configuration file name */
	file_name = config_get_file_name(config);

	/* Allow these sections in case we quit before reading them. */
	config_var_allow(config, section, "Module");

	/* Read compute unit */
	sm_id = config_read_int(config, section, "ComputeUnit", -1);
	if (sm_id < 0)
		fatal("%s: section [%s]: invalid or missing value for 'ComputeUnit'",
			file_name, section);

	/* Check compute unit boundaries */
	if (sm_id >= frm_gpu_num_sms)
	{
		warning("%s: section [%s] ignored, referring to Southern Islands compute unit %d.\n"
			"\tThis section refers to a compute unit that does not currently exist.\n"
			"\tPlease review your Southern Islands configuration file if this is not the\n"
			"\tdesired behavior.\n",
			file_name, section, sm_id);
		return;
	}

	/* Check that entry has not been assigned before */
	sm = frm_gpu->sms[sm_id];
	if (sm->global_memory)
		fatal("%s: section [%s]: entry from compute unit %d already assigned.\n"
			"\tA different [Entry <name>] section in the memory configuration file has already\n"
			"\tassigned an entry for this particular compute unit. Please review your\n"
			"\tconfiguration file to avoid duplicates.\n",
			file_name, section, sm_id);

	/* Read module */
	module_name = config_read_string(config, section, "Module", NULL);
	if (!module_name)
		fatal("%s: section [%s]: variable 'Module' missing.\n"
			"\tPlease run use '--mem-help' for more information on the\n"
			"\tconfiguration file format, or consult the Multi2Sim Guide.\n",
			file_name, section);
	
	/* Assign module */
	sm->global_memory = mem_system_get_mod(module_name);
	if (!sm->global_memory)
		fatal("%s: section [%s]: '%s' is not a valid module name.\n"
			"\tThe given module name must match a module declared in a section\n"
			"\t[Module <name>] in the memory configuration file.\n",
			file_name, section, module_name);
	
	/* Add modules to list of memory entries */
	linked_list_add(frm_emu_arch->mem_entry_mod_list, sm->global_memory);
	
	/* Debug */
	mem_debug("\tSouthern Islands compute unit %d\n", sm_id);
	mem_debug("\t\tEntry -> %s\n", sm->global_memory->name);
	mem_debug("\n");
}


void frm_mem_config_check(struct config_t *config)
{
	struct frm_sm_t *sm;
	int sm_id;
	char *file_name;

	/* Check that all compute units have an entry to the memory hierarchy. */
	file_name = config_get_file_name(config);
	FRM_GPU_FOREACH_COMPUTE_UNIT(sm_id)
	{
		sm = frm_gpu->sms[sm_id];
		if (!sm->global_memory)
			fatal("%s: Southern Islands compute unit %d has no entry to memory.\n"
				"\tPlease add a new [Entry <name>] section in your memory configuration\n"
				"\tfile to associate this compute unit with a memory module.\n",
				file_name, sm_id);
	}
}

