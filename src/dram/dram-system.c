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


#include <lib/mhandle/mhandle.h>
#include <lib/util/config.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>

#include "dram.h"
#include "request.h"
#include "controller.h"
#include "dram-system.h"


/*
 * Global Variable
 */

long long dram_system_max_cycles;


/*
 * Local Variable
 */


/*
 * Dram system
 */

struct dram_system_t *dram_system_create(void)
{
	struct dram_system_t *system;

	/* Initialize */
	system = xcalloc(1, sizeof(struct dram_system_t));
	system->dram_controller_list = list_create();

	/* Return */
	return system;
}


void dram_system_free(struct dram_system_t *system)
{
	int i;

	/* Free controller list */
	for (i = 0; i < system->num_logical_channels; i++)
		dram_controller_free(list_get(system->dram_controller_list, i));
	list_free(system->dram_controller_list);

	/* Free */
	free(system);
}


void dram_system_dump(struct dram_system_t *system, FILE *f)
{
	int i;

	/* Print */
	fprintf(f, "\n******************************\n"
			"Dumping DRAM system structure:\n"
			"******************************\n");

	/* Dump controllers */
	for (i = 0; i < system->num_logical_channels; i++)
		dram_controller_dump(list_get(system->dram_controller_list, i), f);
}


int dram_system_config_with_file(struct dram_system_t *system, char *file_name)
{
	int i, j;
	unsigned int highest_addr = 0;
	char section[16];
	char *row_buffer_policy_map[] = {"OpenPage", "ClosePage", "hybird"};
	char *scheduling_policy_map[] = {"RankBank", "BankRank"};
	struct config_t *config;

	/* Controller parameters */
	unsigned int num_physical_channels;
	unsigned int request_queue_depth;
	enum dram_controller_row_buffer_policy_t rb_policy;
	enum dram_controller_scheduling_policy_t scheduling_policy;

	unsigned int dram_num_ranks;
	unsigned int dram_num_devices_per_rank;
	unsigned int dram_num_banks_per_device;
	unsigned int dram_num_rows_per_bank;
	unsigned int dram_num_columns_per_row;
	unsigned int dram_num_bits_per_column;

	unsigned int dram_timing_tCAS;
	unsigned int dram_timing_tRCD;
	unsigned int dram_timing_tRP;
	unsigned int dram_timing_tRAS;
	unsigned int dram_timing_tCWL;
	unsigned int dram_timing_tCCD;

	/* Nothing to do if empty file name passed */
	if (!file_name || !file_name[0])
		return 0;

	/* Open configuration file */
	config = config_create(file_name);
	config_load(config);

	/* DRAM system configuration */
	sprintf(section, "DRAMsystem");
	dram_system_max_cycles = config_read_int(config, section, "NumCycle", dram_system_max_cycles);
	system->num_logical_channels = config_read_int(config, section, "NumLogicalChannels", system->num_logical_channels);

	/* Create controllers */
	for (i = 0; i < system->num_logical_channels; i++)
	{
		/* Section check */
		sprintf(section, "Controller-%u", i);
		if(!config_section_exists(config, section))
			return 0;

		/* Read configuration */
		num_physical_channels = config_read_int(config, section, "NumPhysicalChannels", num_physical_channels);
		dram_num_ranks = config_read_int(config, section, "NumRanks", dram_num_ranks);
		dram_num_devices_per_rank = config_read_int(config, section, "NumDevicesPerRank", dram_num_devices_per_rank);
		dram_num_banks_per_device = config_read_int(config, section, "NumBanksPerDevice", dram_num_banks_per_device);
		dram_num_rows_per_bank = config_read_int(config, section, "NumRowsPerBank", dram_num_rows_per_bank);
		dram_num_columns_per_row = config_read_int(config, section, "NumColumnPerRow", dram_num_columns_per_row);
		dram_num_bits_per_column = config_read_int(config, section, "NumBitsPerColumn", dram_num_bits_per_column);
		request_queue_depth = config_read_int(config, section, "RequestQueueDepth", request_queue_depth);
		rb_policy = config_read_enum(config, section, "RowBufferPolicy", rb_policy, row_buffer_policy_map, 3);
		scheduling_policy = config_read_enum(config, section, "SchedulingPolicy", scheduling_policy, scheduling_policy_map, 2);
		dram_timing_tCAS = config_read_int(config, section, "tCAS", dram_timing_tCAS);
		dram_timing_tRCD = config_read_int(config, section, "tRCD", dram_timing_tRCD);
		dram_timing_tRP = config_read_int(config, section, "tRP", dram_timing_tRP);
		dram_timing_tRAS = config_read_int(config, section, "tRAS", dram_timing_tRAS);
		dram_timing_tCWL = config_read_int(config, section, "tCWL", dram_timing_tCWL);
		dram_timing_tCCD = config_read_int(config, section, "tCCD", dram_timing_tCCD);

		/* Create controller */
		struct dram_controller_t *controller;
		controller = dram_controller_create(request_queue_depth, rb_policy, scheduling_policy);

		/* Assign controller parameters */
		controller->id = i;
		if (!i)
			controller->lowest_addr = 0;
		else
			controller->lowest_addr = highest_addr + 1;

		controller->highest_addr = controller->lowest_addr + ((dram_num_bits_per_column * dram_num_devices_per_rank) / 8 * dram_num_columns_per_row * dram_num_rows_per_bank * dram_num_banks_per_device * dram_num_ranks * num_physical_channels) - 1;

		controller->dram_addr_bits_rank = log_base2(dram_num_ranks);
		controller->dram_addr_bits_row = log_base2(dram_num_rows_per_bank);
		controller->dram_addr_bits_bank = log_base2(dram_num_banks_per_device);
		controller->dram_addr_bits_column = log_base2(dram_num_columns_per_row);
		controller->dram_addr_bits_physical_channel = log_base2(num_physical_channels);
		controller->dram_addr_bits_byte = log_base2(dram_num_bits_per_column * dram_num_devices_per_rank / 8);
		controller->dram_timing_tCAS = dram_timing_tCAS;
		controller->dram_timing_tRCD = dram_timing_tRCD;
		controller->dram_timing_tRP = dram_timing_tRP;
		controller->dram_timing_tRAS = dram_timing_tRAS;
		controller->dram_timing_tCWL = dram_timing_tCWL;
		controller->dram_timing_tCCD = dram_timing_tCCD;

		/* Update the highest address in memory system */
		highest_addr = controller->highest_addr;

		/* Add controller to system */
		list_add(system->dram_controller_list, controller);

		/* Create and add DRAM*/
		for (j = 0; j < num_physical_channels; j++)
		{
			struct dram_t *dram;
			dram = dram_create(dram_num_ranks,
						dram_num_devices_per_rank,
						dram_num_banks_per_device,
						dram_num_rows_per_bank,
						dram_num_columns_per_row,
						dram_num_bits_per_column);

			dram->timing_tCAS = dram_timing_tCAS;
			dram->timing_tRCD = dram_timing_tRCD;
			dram->timing_tRP = dram_timing_tRP;
			dram->timing_tRAS = dram_timing_tRAS;
			dram->timing_tCWL = dram_timing_tCWL;

			dram_controller_add_dram(list_get(system->dram_controller_list, i), dram);
		}
	}

	/* Config free */
	config_free(config);

	/* Return 1 on success */
	return 1;
}


int dram_system_get_request(struct dram_system_t *system, struct dram_request_t *request)
{
	unsigned int logical_channel_id;

	struct dram_controller_t *controller;

	/* Do nothing if no request is passed */
	if (!request)
		return 0;

	/* Assign pointer */
	request->system = system;

	/* Decode request address */
	dram_decode_address(system, request->addr, &logical_channel_id,
				NULL, NULL, NULL, NULL, NULL);

	/* Send request to the request queue*/
	/* return 0 if request queue cannot take any request at this moment */
	controller = list_get(system->dram_controller_list, logical_channel_id);
	if (!dram_controller_get_request(controller, request))
		return 0;

	/* Return */
	return 1;
}


void dram_system_process(struct dram_system_t *system)
{
	int i;
	struct dram_controller_t *controller;

	for (i = 0; i < system->num_logical_channels; i++)
	{
		/* Locate controller */
		controller = list_get(system->dram_controller_list, i);

		/* Schedule command to the bus*/
		dram_controller_schedule_command(controller);

		/* Process request */
		dram_controller_process_request(controller);
	}
}


void dram_decode_address(struct dram_system_t *system,
			unsigned int addr,
			unsigned int *logical_channel_id_ptr,
			unsigned int *rank_id_ptr,
			unsigned int *row_id_ptr,
			unsigned int *bank_id_ptr,
			unsigned int *column_id_ptr,
			unsigned int *physical_channel_id_ptr)
{
	int i;
	unsigned int local_addr;
	struct dram_controller_t *controller;

	/* Look for the corresponding controller */
	for (i = 0; i < system->num_logical_channels; i++)
	{
		controller = list_get(system->dram_controller_list, i);
		if ((addr > controller->lowest_addr) && (addr < controller->highest_addr))
		{
			local_addr = addr - controller->lowest_addr;

			break;
		}
	}

	/* Address decode */
	if (logical_channel_id_ptr)
		*logical_channel_id_ptr = i;
	i = controller->dram_addr_bits_byte;
	if (physical_channel_id_ptr)
		*physical_channel_id_ptr = (local_addr >> i) & (controller->num_physical_channels - 1);
	i += controller->dram_addr_bits_physical_channel;
	if (column_id_ptr)
		*column_id_ptr = (local_addr >> i) & (controller->dram_num_columns_per_row - 1);
	i += controller->dram_addr_bits_column;
	if (bank_id_ptr)
		*bank_id_ptr = (local_addr >> i) & (controller->dram_num_banks_per_device - 1);
	i += controller->dram_addr_bits_bank;
	if (row_id_ptr)
		*row_id_ptr = (local_addr >> i) & (controller->dram_num_rows_per_bank - 1);
	i += controller->dram_addr_bits_row;
	if (rank_id_ptr)
		*rank_id_ptr = (local_addr >> i) & (controller->dram_num_ranks - 1);
}


unsigned int dram_encode_address(struct dram_system_t *system,
			unsigned int logical_channel_id,
			unsigned int rank_id,
			unsigned int row_id,
			unsigned int bank_id,
			unsigned int column_id,
			unsigned int physical_channel_id)
{
	unsigned int addr;
	struct dram_controller_t *controller;

	/* Locate controller */
	controller = list_get(system->dram_controller_list, logical_channel_id);

	/* Address encode */
	addr = rank_id;
	addr <<= controller->dram_addr_bits_row;
	addr += row_id;
	addr <<= controller->dram_addr_bits_bank;
	addr += bank_id;
	addr <<= controller->dram_addr_bits_column;
	addr += column_id;
	addr <<= controller->dram_addr_bits_physical_channel;
	addr += physical_channel_id;
	addr <<= controller->dram_addr_bits_byte;
	addr += controller->lowest_addr;

	/* Return address */
	return addr;
}
