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
#include <mem-system/module.h>

#include "mem-config.h"
#include "gpu.h"
#include "compute-unit.h"

void SIGpuMemConfigDefault(Timing *self, struct config_t *config)
{
	char section[MAX_STRING_SIZE];
	char str[MAX_STRING_SIZE];

	int i;

	/* Cache geometry for vector L1 */
	snprintf(section, sizeof section, "CacheGeometry si-geo-vector-l1");
	config_write_int(config, section, "Sets", 64);
	config_write_int(config, section, "Assoc", 4);
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 1);
	config_write_string(config, section, "Policy", "LRU");

	/* Cache geometry for scalar L1 */
	snprintf(section, sizeof section, "CacheGeometry si-geo-scalar-l1");
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

	/* Create scalar L1 caches */
	for (i = 0; i < (si_gpu_num_compute_units + 3) / 4; i++)
	{
		snprintf(section, sizeof section, "Module si-scalar-l1-%d", i);
		config_write_string(config, section, "Type", "Cache");
		config_write_string(config, section, "Geometry", 
			"si-geo-scalar-l1");
		config_write_string(config, section, "LowNetwork", 
			"si-net-l1-l2");
		config_write_string(config, section, "LowModules", 
			"si-l2-0 si-l2-1 si-l2-2 si-l2-3 si-l2-4 si-l2-5");
	}

	/* Create vector L1 caches */
	for (i = 0; i < si_gpu_num_compute_units; i++)
	{
		snprintf(section, sizeof section, "Module si-vector-l1-%d", i);
		config_write_string(config, section, "Type", "Cache");
		config_write_string(config, section, "Geometry", 
			"si-geo-vector-l1");
		config_write_string(config, section, "LowNetwork", 
			"si-net-l1-l2");
		config_write_string(config, section, "LowModules", 
			"si-l2-0 si-l2-1 si-l2-2 si-l2-3 si-l2-4 si-l2-5");
	}

	/* Create entries from compute units to L1s */
	for (i = 0; i < si_gpu_num_compute_units; i++)
	{
		/* Entry */
		snprintf(section, sizeof section, "Entry si-cu-%d", i);
		config_write_string(config, section, "Arch", "SouthernIslands");
		config_write_int(config, section, "ComputeUnit", i);
		snprintf(str, sizeof str, "si-vector-l1-%d", i);
		config_write_string(config, section, "DataModule", str);
		snprintf(str, sizeof str, "si-scalar-l1-%d", i / 4);
		config_write_string(config, section, "ConstantDataModule", str);
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

void SIGpuMemConfigFused(Timing *self, struct config_t *config)
{
	char section[MAX_STRING_SIZE];
	char str[MAX_STRING_SIZE];

	int i;

	/* Cache geometry for vector L1 */
	snprintf(section, sizeof section, "CacheGeometry si-geo-vector-l1");
	config_write_int(config, section, "Sets", 64);
	config_write_int(config, section, "Assoc", 4);
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 1);
	config_write_string(config, section, "Policy", "LRU");

	/* Cache geometry for scalar L1 */
	snprintf(section, sizeof section, "CacheGeometry si-geo-scalar-l1");
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

	/* Create scalar L1 caches */
	for (i = 0; i < (si_gpu_num_compute_units + 3) / 4; i++)
	{
		snprintf(section, sizeof section, "Module si-scalar-l1-%d", i);
		config_write_string(config, section, "Type", "Cache");
		config_write_string(config, section, "Geometry", 
			"si-geo-scalar-l1");
		config_write_string(config, section, "LowNetwork", 
			"si-net-l1-l2");
		config_write_string(config, section, "LowModules", 
			"si-l2-0 si-l2-1 si-l2-2 si-l2-3 si-l2-4 si-l2-5");
	}

	/* Create vector L1 caches */
	for (i = 0; i < si_gpu_num_compute_units; i++)
	{
		snprintf(section, sizeof section, "Module si-vector-l1-%d", i);
		config_write_string(config, section, "Type", "Cache");
		config_write_string(config, section, "Geometry", 
			"si-geo-vector-l1");
		config_write_string(config, section, "LowNetwork", 
			"si-net-l1-l2");
		config_write_string(config, section, "LowModules", 
			"si-l2-0 si-l2-1 si-l2-2 si-l2-3 si-l2-4 si-l2-5");
	}

	/* Create entries from compute units to L1s */
	for (i = 0; i < si_gpu_num_compute_units; i++)
	{
		/* Entry */
		snprintf(section, sizeof section, "Entry si-cu-%d", i);
		config_write_string(config, section, "Arch", "SouthernIslands");
		config_write_int(config, section, "ComputeUnit", i);
		snprintf(str, sizeof str, "si-vector-l1-%d", i);
		config_write_string(config, section, "DataModule", str);
		snprintf(str, sizeof str, "si-scalar-l1-%d", i / 4);
		config_write_string(config, section, "ConstantDataModule", str);
	}

	/* L2 caches */
	snprintf(section, sizeof section, "Module si-l2-0");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "si-geo-l2");
	config_write_string(config, section, "HighNetwork", "si-net-l1-l2");
	config_write_string(config, section, "LowNetwork", "x86-net-l2-mm");
	config_write_string(config, section, "LowModules", "x86-mm");
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 0");

	snprintf(section, sizeof section, "Module si-l2-1");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "si-geo-l2");
	config_write_string(config, section, "HighNetwork", "si-net-l1-l2");
	config_write_string(config, section, "LowNetwork", "x86-net-l2-mm");
	config_write_string(config, section, "LowModules", "x86-mm");
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 1");
	
	snprintf(section, sizeof section, "Module si-l2-2");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "si-geo-l2");
	config_write_string(config, section, "HighNetwork", "si-net-l1-l2");
	config_write_string(config, section, "LowNetwork", "x86-net-l2-mm");
	config_write_string(config, section, "LowModules", "x86-mm");
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 2");

	snprintf(section, sizeof section, "Module si-l2-3");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "si-geo-l2");
	config_write_string(config, section, "HighNetwork", "si-net-l1-l2");
	config_write_string(config, section, "LowNetwork", "x86-net-l2-mm");
	config_write_string(config, section, "LowModules", "x86-mm");
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 3");

	snprintf(section, sizeof section, "Module si-l2-4");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "si-geo-l2");
	config_write_string(config, section, "HighNetwork", "si-net-l1-l2");
	config_write_string(config, section, "LowNetwork", "x86-net-l2-mm");
	config_write_string(config, section, "LowModules", "x86-mm");
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 4");

	snprintf(section, sizeof section, "Module si-l2-5");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "si-geo-l2");
	config_write_string(config, section, "HighNetwork", "si-net-l1-l2");
	config_write_string(config, section, "LowNetwork", "x86-net-l2-mm");
	config_write_string(config, section, "LowModules", "x86-mm");
	config_write_string(config, section, "AddressRange", 
		"ADDR DIV 64 MOD 6 EQ 5");

	/* Network connecting L1s and L2s */
	snprintf(section, sizeof section, "Network si-net-l1-l2");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);

	/* Networks connecting memory controllers and global memory banks */
	snprintf(section, sizeof section, "Network x86-net-l2-mm");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);
}


void SIGpuMemConfigParseEntry(Timing *self, struct config_t *config, char *section)
{
	char *file_name;
	char *vector_module_name;
	char *scalar_module_name;

	int unified_present;
	int separate_present;

	int compute_unit_id;

	struct si_compute_unit_t *compute_unit;

	/* Get configuration file name */
	file_name = config_get_file_name(config);

	/* Allow these sections in case we quit before reading them. */
	config_var_allow(config, section, "DataModule");
	config_var_allow(config, section, "ConstantDataModule");
	config_var_allow(config, section, "Module");

	unified_present = config_var_exists(config, section, "Module");
	separate_present = config_var_exists(config, section, 
		"DataModule") && config_var_exists(config, section, 
		"ConstantDataModule");

	if (!unified_present && !separate_present)
	{
		fatal(
	"%s: section [%s]: variable 'Module' missing.\n"
	"\tPlease run use '--mem-help' for more information on the\n"
	"\tconfiguration file format, or consult the Multi2Sim Guide.\n",
			file_name, section);
	}

	if (!(unified_present ^ separate_present))
	{
		fatal(
	"%s: section [%s]: invalid combination of modules.\n"
	"\tA Southern Islands entry to the memory hierarchy needs to specify\n"
	"\teither a unified entry for vector and scalar caches (variable \n"
	"\t'Module'), or two separate entries for data and scalar (constant)\n"
	"\tdata (variables 'DataModule' and 'ConstantDataModule'), but not\n"
	"\tboth.\n",
			file_name, section);
	}

	/* Read compute unit */
	compute_unit_id = config_read_int(config, section, "ComputeUnit", -1);
	if (compute_unit_id < 0)
	{
		fatal("%s: section [%s]: invalid or missing value for "
			"'ComputeUnit'", file_name, section);
	}

	/* Check compute unit boundaries */
	if (compute_unit_id >= si_gpu_num_compute_units)
	{
		warning(
	"%s: section [%s] ignored, referring to Southern Islands \n"
	"\tcompute unit %d. This section refers to a compute unit that\n" 
	"\tdoes not currently exist. Please review your Southern Islands\n" 
	"\tconfiguration file if this is not the desired behavior.\n",
			file_name, section, compute_unit_id);
		return;
	}

	/* Check that entry has not been assigned before */
	compute_unit = si_gpu->compute_units[compute_unit_id];
	if (compute_unit->vector_cache)
	{
		fatal(
	"%s: section [%s]: entry from compute unit %d already assigned.\n"
	"\tA different [Entry <name>] section in the memory configuration\n" 
	"\tfile has already assigned an entry for this particular compute \n"
	"\tunit. Please review your tconfiguration file to avoid duplicates.\n",
			file_name, section, compute_unit_id);
	}

	/* Read modules */
	if (separate_present)
	{
		vector_module_name = config_read_string(config, section, 
			"DataModule", NULL);
		scalar_module_name = config_read_string(config, section, 
			"ConstantDataModule", NULL);
	}
	else
	{
		vector_module_name = scalar_module_name =
			config_read_string(config, section, "Module", NULL);
	}
	assert(vector_module_name);
	assert(scalar_module_name);
	
	/* Assign modules */
	compute_unit->vector_cache = mem_system_get_mod(vector_module_name);
	if (!compute_unit->vector_cache)
	{
		fatal(
	"%s: section [%s]: '%s' is not a valid module name.\n"
	"\tThe given module name must match a module declared in a section\n"
	"\t[Module <name>] in the memory configuration file.\n",
			file_name, section, vector_module_name);
	}
	compute_unit->scalar_cache = mem_system_get_mod(scalar_module_name);
	if (!compute_unit->scalar_cache)
	{
		fatal(
	"%s: section [%s]: '%s' is not a valid module name.\n"
	"\tThe given module name must match a module declared in a section\n"
	"\t[Module <name>] in the memory configuration file.\n",
			file_name, section, scalar_module_name);
	}
	
	/* Add modules to list of memory entries */
	linked_list_add(arch_southern_islands->mem_entry_mod_list, 
		compute_unit->vector_cache);
	linked_list_add(arch_southern_islands->mem_entry_mod_list, 
		compute_unit->scalar_cache);
	
	/* Debug */
	mem_debug("\tSouthern Islands compute unit %d\n", compute_unit_id);
	mem_debug("\t\tEntry for vector mem -> %s\n", 
		compute_unit->vector_cache->name);
	mem_debug("\t\tEntry for scalar mem -> %s\n", 
		compute_unit->scalar_cache->name);
	mem_debug("\n");
}


void SIGpuMemConfigCheck(Timing *self, struct config_t *config)
{
	struct si_compute_unit_t *compute_unit;
	int compute_unit_id;
	char *file_name;

	/* Check that all compute units have an entry to the 
	 * memory hierarchy. */
	file_name = config_get_file_name(config);
	SI_GPU_FOREACH_COMPUTE_UNIT(compute_unit_id)
	{
		compute_unit = si_gpu->compute_units[compute_unit_id];
		if (!compute_unit->vector_cache)
		{
			fatal(
	"%s: Southern Islands compute unit %d has no entry to memory.\n"
	"\tPlease add a new [Entry <name>] section in your memory\n"
	"\tconfiguration file to associate this compute unit with a memory\n"
	"\tmodule.\n",
				file_name, compute_unit_id);
		}
	}
}

