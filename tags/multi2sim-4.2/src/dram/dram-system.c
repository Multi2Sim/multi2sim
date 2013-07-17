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
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/hash-table.h>
#include <lib/util/string.h>


#include "dram.h"
#include "request.h"
#include "controller.h"
#include "dram-system.h"


/*
 * Global Variable
 */

long long dram_system_max_cycles;

int dram_debug_category;
int dram_domain_index;
int dram_frequency = 1000;

char *dram_config_file_name = "";
char *dram_report_file_name = "";
char *dram_request_file_name = "";
FILE *dram_report_file;

char *dram_sim_system_name = "";
char *dram_config_help =
		"The DRAM configuration file is a plain-text file following the\n"
		"IniFile format.  \n" "\n" "\n";

char *dram_err_config =
		"\tA DRAM system is being loaded from an IniFile configuration file.\n"
		"\tHowever, some feature of the provided file does not comply with the\n"
		"\texpected format. Please run 'm2s --help-dram-config' for a list of\n"
		"\tpossible sections/variables in the DRAM configuration file.\n";


/*
 * Local Variable
 */

static struct hash_table_t *dram_system_table;

static void dram_config_request_create(struct dram_system_t *system, struct config_t *config,
		char *section)
{
	char *request_line;
	char request_var[MAX_STRING_SIZE];

	int request_var_id;

	/* Read Requests */
	request_var_id = 0;

	/* Register events for request handler*/
	EV_DRAM_REQUEST = esim_register_event_with_name(dram_request_handler,
			dram_domain_index, "dram_request");

	while (1)
	{
		/* Get request */
		snprintf(request_var, sizeof request_var, "Request[%d]", request_var_id);
		request_line = config_read_string(config, section, request_var, NULL);
		if (!request_line)
			break;

		/* Schedule event to process request */
		struct request_stack_t *stack;
		stack = dram_request_stack_create();

		request_line = xstrdup(request_line);
		stack->request_line = request_line;
		stack->system = system;

		esim_schedule_event(EV_DRAM_REQUEST, stack, 0);

		/* Next command */
		request_var_id++;
	}
}


/*
 * Dram system
 */


struct dram_system_t *dram_system_create(char *name)
{
	struct dram_system_t *system;

	/* Initialize */
	system = xcalloc(1, sizeof(struct dram_system_t));
	system->name = xstrdup(name);
	system->dram_controller_list = list_create();
	system->dram_request_list = list_create();

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

	/* Freeing the DRAM request list */
	for (i = 0; i < list_count(system->dram_request_list); i++)
		dram_request_free(list_get(system->dram_request_list, i));
	list_free(system->dram_request_list);

	/* Free */
	free(system->name);
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


struct dram_system_t *dram_system_config_with_file(struct config_t *config, char *system_name)
{
	int j;
	int controller_sections = 0;
	unsigned int highest_addr = 0;
	char *section;
	char section_str[MAX_STRING_SIZE];
	char *row_buffer_policy_map[] = {"OpenPage", "ClosePage", "hybird"};
	char *scheduling_policy_map[] = {"RankBank", "BankRank"};
	struct dram_system_t *system;

	/* Controller parameters
	 * FIXME: we should create a default variation for times this values
	 * are not assigned. For now we set it as DRAM DDR3 Micron
	 * */
	unsigned int num_physical_channels = 1;
	unsigned int request_queue_depth = 32;
	enum dram_controller_row_buffer_policy_t rb_policy = open_page_row_buffer_policy;
	enum dram_controller_scheduling_policy_t scheduling_policy = rank_bank_round_robin;

	unsigned int dram_num_ranks = 8;
	unsigned int dram_num_devices_per_rank = 1;
	unsigned int dram_num_banks_per_device = 1;
	unsigned int dram_num_rows_per_bank = 8192;
	unsigned int dram_num_columns_per_row = 1024;
	unsigned int dram_num_bits_per_column = 16;

	unsigned int dram_timing_tCAS = 24;
	unsigned int dram_timing_tRCD = 10;
	unsigned int dram_timing_tRP = 10;
	unsigned int dram_timing_tRAS = 24;
	unsigned int dram_timing_tCWL = 9;
	unsigned int dram_timing_tCCD = 4;

	system = dram_system_create(system_name);
	/* DRAM system configuration */
	snprintf(section_str, sizeof section_str, "DRAMsystem.%s", system_name);
	for (section = config_section_first(config); section;
			section = config_section_next(config))
	{
		if (strcasecmp(section, section_str))
			continue;
		system->num_logical_channels = config_read_int(config, section,
				"NumLogicalChannels", system->num_logical_channels);
	}



	/* Create controllers */

	for (section = config_section_first(config); section;
			section = config_section_next(config))
	{
		char *delim = ".";
		char *token;
		char *controller_name;

		/* First token must be 'Network' */
		snprintf(section_str, sizeof section_str, "%s", section);
		token = strtok(section_str, delim);
		if (!token || strcasecmp(token, "DRAMsystem"))
			continue;

		/* Second token must be the name of the network */
		token = strtok(NULL, delim);
		if (!token || strcasecmp(token, system_name))
			continue;

		/* Third token must be 'Node' */
		token = strtok(NULL, delim);
		if (!token || strcasecmp(token, "Controller"))
			continue;

		/* Get name */
		controller_name = strtok(NULL, delim);
		token = strtok(NULL, delim);
		if (!controller_name || token)
			fatal("%s:%s: wrong format for controller name .\n%s",
					system->name, section, dram_err_config);

		/* Read Properties */

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
		controller->id = controller_sections;
		if (!controller_sections)
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

			dram_controller_add_dram(list_get(system->dram_controller_list, controller_sections), dram);
		}
		controller_sections++;
	}

	if (controller_sections != system->num_logical_channels)
		fatal("%s: number of controllers should match the number of logical"
				"channels \n%s", system->name, dram_err_config);

	/* Request Section */
	for (section = config_section_first(config); section;
			section = config_section_next(config))
	{
		char *delim = ".";

		char *token;
		char *token_endl;

		/* First token must be 'Network' */
		snprintf(section_str, sizeof section_str, "%s", section);
		token = strtok(section_str, delim);
		if (!token || strcasecmp(token, "DRAMsystem"))
			continue;

		/* Second token must be the name of the network */
		token = strtok(NULL, delim);
		if (!token || strcasecmp(token, system_name))
			continue;

		/* Third token must be 'Commands' */
		token = strtok(NULL, delim);
		if (!token || strcasecmp(token, "Requests"))
			continue;

		token_endl = strtok(NULL, delim);
		if (token_endl)
			fatal("%s: %s: bad format for Commands section.\n%s",
					system_name, section, dram_err_config);

		/* Requests */
		dram_config_request_create(system, config, section);
		config_check(config);
	}

	/* Return dram_system on success */
	return system;
}


int dram_system_get_request(struct dram_system_t *system)
{
	unsigned int logical_channel_id;

	struct dram_controller_t *controller;

	struct dram_request_t *request;
	request = list_get(system->dram_request_list, 0);
	/* Do nothing if no request is passed */
	if (!request)
		return 0;


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

void dram_system_init(void)
{
	dram_system_read_config();

	/* Register events */
	EV_DRAM_COMMAND_RECEIVE = esim_register_event(dram_event_handler, dram_domain_index);
	EV_DRAM_COMMAND_COMPLETE = esim_register_event(dram_event_handler, dram_domain_index);


	if (*dram_report_file_name)
	{
		dram_report_file = file_open_for_write(dram_report_file_name);
		if (!dram_report_file)
			fatal("%s: cannot write on DRAM report file", dram_report_file_name);
	}



}
void dram_system_sim (char *debug_file_name)
{
	struct dram_system_t *dram_system;

	/* Initialize */
	debug_init();
	esim_init();
	dram_system_init();
	dram_debug_category = debug_new_category(debug_file_name);

	/* Getting the simulation name */
	if (!*dram_sim_system_name)
		panic("%s: no DRAM simulation name", __FUNCTION__);
	dram_system = dram_system_find(dram_sim_system_name);
	if (!dram_system)
		fatal("%s: DRAM system does not exist", dram_sim_system_name);

	esim_process_events(TRUE);
	while (1)
	{
		long long cycle;

		cycle = esim_domain_cycle(dram_domain_index);

		if (cycle >= dram_system_max_cycles)
			break;

		if ((list_count(dram_system->dram_request_list)) &&
				dram_system_get_request(dram_system))
			list_dequeue(dram_system->dram_request_list);
		dram_system_process(dram_system);

		/* Next Cycle */
		dram_debug("___cycle %lld___\n", cycle);
		esim_process_events(TRUE);
	}

	dram_system_done();
	esim_done();
	debug_done();

	mhandle_done();
	exit(0);
}

void dram_system_read_config(void)
{
	int i ;
	struct config_t *config;
	struct list_t *dram_system_list;
	char *section;

	if (!*dram_config_file_name)
	{
		dram_domain_index = esim_new_domain(dram_frequency);
		return;
	}

	config = config_create(dram_config_file_name);
	if (*dram_config_file_name)
		config_load(config);

	/* Section with Generic Configuration Parameters */
	section = "General";

	/* Frequency */
	dram_frequency = config_read_int(config, section, "Frequency", dram_frequency);
	if (!IN_RANGE(dram_frequency, 1, ESIM_MAX_FREQUENCY))
		fatal("%s: Invalid value for 'Frequency'", dram_config_file_name);

	/* Creating the Frequency Domain */
	dram_domain_index = esim_new_domain(dram_frequency);

	/* Create a temporary List of all Dram Systems found in
	 * the configuration file */
	dram_system_list = list_create();
	for (section = config_section_first(config); section;
			section = config_section_next(config))
	{
		char *delim = ".";

		char section_str[MAX_STRING_SIZE];
		char *token;
		char *dram_system_name;

		/*Creating a copy of the name of the section */
		snprintf(section_str, sizeof section_str, "%s", section);
		section = section_str;

		/* First Token Must be 'DRAMsystem' */
		token = strtok(section, delim);
		if (strcasecmp(token, "DRAMsystem"))
			continue;

		/* Second Token must be the system Name */
		dram_system_name = strtok(NULL, delim);
		if (!dram_system_name)
			continue;

		/* No third term is required */
		token = strtok(NULL, delim);
		if (token)
			continue;

		/* Insert the new DRAM system name */
		dram_system_name = xstrdup(dram_system_name);
		list_add(dram_system_list, dram_system_name);
	}

	/* Print DRAM system Names in debug */
	dram_debug("%s: loading DRAM system configuration file \n",
			dram_config_file_name);
	dram_debug("DRAM systems found:\n");
	for (i = 0; i < dram_system_list->count; i++)
		dram_debug("\t%s\n", (char *) list_get(dram_system_list, i));
	dram_debug("\n");

	/* Load DRAM systems */
	dram_system_table = hash_table_create(0, 0);
	for ( i = 0; i < dram_system_list->count; i++)
	{
		struct dram_system_t *system;
		char *dram_system_name;

		dram_system_name = list_get(dram_system_list, i);
		system = dram_system_config_with_file(config, dram_system_name);

		hash_table_insert(dram_system_table, dram_system_name, system);
	}
	while (dram_system_list->count)
		free(list_remove_at(dram_system_list, 0));
	list_free(dram_system_list);
	config_free(config);
}

struct dram_system_t *dram_system_find(char *dram_system_name)
{
	if (!dram_system_table)
		return NULL;
	return hash_table_get(dram_system_table, dram_system_name);
}

void dram_system_done(void)
{
	struct dram_system_t *system;

	/* Free list of dram systems */
	if (dram_system_table)
	{
		for (hash_table_find_first(dram_system_table, (void **) &system);
				system ; hash_table_find_next(dram_system_table,
						(void **) &system))
		{
			/* Dump Report for DRAM system */
			if (dram_report_file)
				dram_system_dump(system, dram_report_file);

			dram_system_free(system);

		}
	}
	hash_table_free(dram_system_table);

	/* Close report File */
	file_close(dram_report_file);

}
