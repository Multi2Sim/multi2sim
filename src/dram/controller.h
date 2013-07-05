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

#ifndef DRAM_CONTROLLER_H
#define DRAM_CONTROLLER_H

#include <stdio.h>

#define DRAM_TIMING_MATRIX_SIZE 7


/*
 * Local variable
 */

enum dram_controller_row_buffer_policy_t
{
	open_page_row_buffer_policy = 0,
	close_page_row_buffer_policy,
	hybird_page_row_buffer_policy
};

enum dram_controller_scheduling_policy_t
{
	rank_bank_round_robin = 0,
	bank_rank_round_robin
};


/*
 * DRAM bank info
 */

struct dram_bank_info_t
{
	/* IDs */
	unsigned int channel_id;
	unsigned int rank_id;
	unsigned int bank_id;

	/* Bank status */
	int row_buffer_valid;
	int active_row_id;

	/* Request queue*/
	unsigned int request_queue_depth;
	struct list_t *request_queue;

	/* Command queue */
	//unsigned int command_queue_depth;
	struct list_t *command_queue;

	/* Last scheduled command type*/
	struct dram_command_t *last_scheduled_command;

	/* Last scheduled command time matrix */
	unsigned long long dram_bank_info_last_scheduled_time_matrix[DRAM_TIMING_MATRIX_SIZE];
};

struct dram_bank_info_t *dram_bank_info_create(unsigned int channel_id,
						unsigned int rank_id,
						unsigned int bank_id,
						unsigned int request_queue_depth);
void dram_bank_info_free(struct dram_bank_info_t *info);
void dram_bank_info_dump(struct dram_bank_info_t *info, FILE *f);


/*
 * DRAM bus scheduler
 */

struct dram_command_scheduler_t
{
	unsigned int channel_id;
	unsigned int last_scheduled_rank_id;
	unsigned int last_scheduled_bank_id;
};

struct dram_command_scheduler_t *dram_command_scheduler_create(unsigned int channel_id);
void dram_command_scheduler_free(struct dram_command_scheduler_t *scheduler);
void dram_command_scheduler_dump(struct dram_command_scheduler_t *scheduler, FILE *f);


/*
 * Controller
 */

struct dram_controller_t
{
	char *name;

	unsigned int id;
	unsigned int lowest_addr, highest_addr;
	unsigned int num_physical_channels;
	unsigned int request_queue_depth;
	//unsigned int dram_controller_command_queue_depth;	//FIXME: figure out how to implement fixed size command queue
	enum dram_controller_row_buffer_policy_t rb_policy;
	enum dram_controller_scheduling_policy_t scheduling_policy;

	unsigned int dram_num_ranks;
	unsigned int dram_num_devices_per_rank;
	unsigned int dram_num_banks_per_device;
	unsigned int dram_num_rows_per_bank;
	unsigned int dram_num_columns_per_row;
	unsigned int dram_num_bits_per_column;

	unsigned int dram_addr_bits_rank;
	unsigned int dram_addr_bits_row;
	unsigned int dram_addr_bits_bank;
	unsigned int dram_addr_bits_column;
	unsigned int dram_addr_bits_physical_channel;
	unsigned int dram_addr_bits_byte;

	unsigned int dram_timing_tCAS;
	unsigned int dram_timing_tRCD;
	unsigned int dram_timing_tRP;
	unsigned int dram_timing_tRAS;
	unsigned int dram_timing_tCWL;

	unsigned int dram_timing_tCCD;

	unsigned int dram_timing_matrix[DRAM_TIMING_MATRIX_SIZE][DRAM_TIMING_MATRIX_SIZE];

	struct list_t *dram_list;
	struct list_t *dram_bank_info_list;
	struct list_t *dram_command_scheduler_list;
};


struct dram_controller_t *dram_controller_create(unsigned int dram_controller_request_queue_depth,
							enum dram_controller_row_buffer_policy_t rb_policy,
							enum dram_controller_scheduling_policy_t scheduling_policy);
void dram_controller_free(struct dram_controller_t *controller);
void dram_controller_dump(struct dram_controller_t *controller, FILE *f);
int dram_controller_add_dram(struct dram_controller_t *controller, struct dram_t *dram);
int dram_controller_get_request(struct dram_controller_t *controller, struct dram_request_t *request);
void dram_controller_process_request(struct dram_controller_t *controller);
void dram_controller_schedule_command(struct dram_controller_t *controller);


#endif
