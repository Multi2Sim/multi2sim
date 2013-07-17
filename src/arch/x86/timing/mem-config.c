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

#include "core.h"
#include "cpu.h"
#include "mem-config.h"
#include "thread.h"


void X86CpuMemConfigDefault(Timing *self, struct config_t *config)
{
	char section[MAX_STRING_SIZE];
	char str[MAX_STRING_SIZE];

	int i;
	int j;

	/* Cache geometry for L1 */
	snprintf(section, sizeof section, "CacheGeometry x86-geo-l1");
	config_write_int(config, section, "Sets", 16);
	config_write_int(config, section, "Assoc", 2);
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 1);
	config_write_string(config, section, "Policy", "LRU");

	/* Cache geometry for L2 */
	snprintf(section, sizeof section, "CacheGeometry x86-geo-l2");
	config_write_int(config, section, "Sets", 64);
	config_write_int(config, section, "Assoc", 4);
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 10);
	config_write_string(config, section, "Policy", "LRU");

	/* L1 caches and entries */
	for (i = 0; i < x86_cpu_num_cores; i++)
	{
		/* L1 cache */
		snprintf(section, sizeof section, "Module x86-l1-%d", i);
		config_write_string(config, section, "Type", "Cache");
		config_write_string(config, section, "Geometry", "x86-geo-l1");
		config_write_string(config, section, "LowNetwork", "x86-net-l1-l2");
		config_write_string(config, section, "LowModules", "x86-l2");

		/* Entry */
		snprintf(str, sizeof str, "x86-l1-%d", i);
		for (j = 0; j < x86_cpu_num_threads; j++)
		{
			snprintf(section, sizeof section, "Entry x86-core-%d-thread-%d",
				i, j);
			config_write_string(config, section, "Arch", "x86");
			config_write_int(config, section, "Core", i);
			config_write_int(config, section, "Thread", j);
			config_write_string(config, section, "Module", str);
		}
	}

	/* L2 cache */
	snprintf(section, sizeof section, "Module x86-l2");
	config_write_string(config, section, "Type", "Cache");
	config_write_string(config, section, "Geometry", "x86-geo-l2");
	config_write_string(config, section, "HighNetwork", "x86-net-l1-l2");
	config_write_string(config, section, "LowNetwork", "x86-net-l2-mm");
	config_write_string(config, section, "LowModules", "x86-mm");

	/* Main memory */
	snprintf(section, sizeof section, "Module x86-mm");
	config_write_string(config, section, "Type", "MainMemory");
	config_write_string(config, section, "HighNetwork", "x86-net-l2-mm");
	config_write_int(config, section, "BlockSize", 64);
	config_write_int(config, section, "Latency", 100);

	/* Network connecting L1 caches and L2 */
	snprintf(section, sizeof section, "Network x86-net-l1-l2");
	config_write_int(config, section, "DefaultInputBufferSize", 144);
	config_write_int(config, section, "DefaultOutputBufferSize", 144);
	config_write_int(config, section, "DefaultBandwidth", 72);

	/* Network connecting L2 cache and global memory */
	snprintf(section, sizeof section, "Network x86-net-l2-mm");
	config_write_int(config, section, "DefaultInputBufferSize", 528);
	config_write_int(config, section, "DefaultOutputBufferSize", 528);
	config_write_int(config, section, "DefaultBandwidth", 264);
}


void X86CpuMemConfigParseEntry(Timing *self, struct config_t *config, char *section)
{
	X86Cpu *cpu = asX86Cpu(self);
	X86Core *core;
	X86Thread *thread;

	char *file_name;

	int core_index;
	int thread_index;

	int unified_present;
	int data_inst_present;

	char *data_module_name;
	char *inst_module_name;

	/* Get configuration file name */
	file_name = config_get_file_name(config);

	/* Allow these sections in case we quit before reading them. */
	config_var_allow(config, section, "DataModule");
	config_var_allow(config, section, "InstModule");
	config_var_allow(config, section, "Module");

	/* Check right presence of sections */
	unified_present = config_var_exists(config, section, "Module");
	data_inst_present = config_var_exists(config, section, "DataModule") &&
		config_var_exists(config, section, "InstModule");
	if (!(unified_present ^ data_inst_present))
		fatal("%s: section [%s]: invalid combination of modules.\n"
			"\tAn x86 entry to the memory hierarchy needs to specify either a unified\n"
			"\tentry for data and instructions (variable 'Module'), or two separate\n"
			"\tentries for data and instructions (variables 'DataModule' and 'InstModule'),\n"
			"\tbut not both.\n",
			file_name, section);

	/* Read core */
	core_index = config_read_int(config, section, "Core", -1);
	if (core_index < 0)
		fatal("%s: section [%s]: invalid or missing value for 'Core'",
			file_name, section);

	/* Read thread */
	thread_index = config_read_int(config, section, "Thread", -1);
	if (thread_index < 0)
		fatal("%s: section [%s]: invalid or missing value for 'Thread'",
			file_name, section);

	/* Check bounds */
	if (core_index >= x86_cpu_num_cores || thread_index >= x86_cpu_num_threads)
	{
		warning("%s: section [%s] ignored, referring to x86 Core %d, Thread %d.\n"
			"\tThis section refers to a core or thread that does not currently exists.\n"
			"\tPlease review your x86 configuration file if this behavior is not desired.\n",
			file_name, section, core_index, thread_index);
		return;
	}

	/* Check that entry has not been assigned before */
	core = cpu->cores[core_index];
	thread = core->threads[thread_index];
	if (thread->data_mod || thread->inst_mod)
	{
		assert(thread->data_mod && thread->inst_mod);
		fatal("%s: section [%s]: entry from Core %d, Thread %d already assigned.\n"
			"\tA different [Entry <name>] section in the memory configuration file has already\n"
			"\tassigned an entry for this particular core and thread. Please review your\n"
			"\tconfiguration file to avoid duplicates.\n",
			file_name, section, core_index, thread_index);
	}

	/* Read modules */
	if (data_inst_present)
	{
		data_module_name = config_read_string(config, section, "DataModule", NULL);
		inst_module_name = config_read_string(config, section, "InstModule", NULL);
		assert(data_module_name);
		assert(inst_module_name);
	}
	else
	{
		data_module_name = inst_module_name =
			config_read_string(config, section, "Module", NULL);
		assert(data_module_name);
	}

	/* Assign data module */
	thread->data_mod = mem_system_get_mod(data_module_name);
	if (!thread->data_mod)
		fatal("%s: section [%s]: '%s' is not a valid module name.\n"
			"\tThe given module name must match a module declared in a section\n"
			"\t[Module <name>] in the memory configuration file.\n",
			file_name, section, data_module_name);

	/* Assign instruction module */
	thread->inst_mod = mem_system_get_mod(inst_module_name);
	if (!thread->inst_mod)
		fatal("%s: section [%s]: '%s' is not a valid module name.\n"
			"\tThe given module name must match a module declared in a section\n"
			"\t[Module <name>] in the memory configuration file.\n",
			file_name, section, inst_module_name);
	
	/* Add modules to entry list */
	linked_list_add(arch_x86->mem_entry_mod_list, thread->data_mod);
	if (thread->data_mod != thread->inst_mod)
		linked_list_add(arch_x86->mem_entry_mod_list, thread->inst_mod);

	/* Debug */
	mem_debug("\tx86 Core %d, Thread %d\n", core_index, thread_index);
	mem_debug("\t\tEntry for instructions -> %s\n", thread->inst_mod->name);
	mem_debug("\t\tEntry for data -> %s\n", thread->data_mod->name);
	mem_debug("\n");
}


void X86CpuMemConfigCheck(Timing *self, struct config_t *config)
{
	X86Cpu *cpu = asX86Cpu(self);
	X86Core *core;
	X86Thread *thread;

	int i;
	int j;

	char *file_name;

	/* Check that all cores/threads have an entry to the memory hierarchy. */
	file_name = config_get_file_name(config);
	for (i = 0; i < x86_cpu_num_cores; i++)
	{
		core = cpu->cores[i];
		for (j = 0; j < x86_cpu_num_threads; j++)
		{
			thread = core->threads[j];
			if (!thread->data_mod || !thread->inst_mod)
				fatal("%s: x86 Core %d, Thread %d lacks a data/instruction entry to memory.\n"
						"\tPlease add a new [Entry <name>] section in your memory configuration\n"
						"\tfile to associate this hardware thread with a memory module.\n",
						file_name, i, j);
		}
	}
}

