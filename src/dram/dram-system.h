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

#ifndef DRAM_DRAM_SYSTEM_H
#define DRAM_DRAM_SYSTEM_H

#include <stdio.h>


/*
 * Global variable
 */

extern long long dram_system_max_cycles;


/*
 * Local variable
 */


/*
 * Dram system
 */

struct dram_system_t
{
	unsigned int num_logical_channels;
	struct list_t *dram_controller_list;
};

struct dram_system_t *dram_system_create(void);
void dram_system_free(struct dram_system_t *system);
void dram_system_dump(struct dram_system_t *system, FILE *f);
int dram_system_config_with_file(struct dram_system_t *system, char *file_name);
int dram_system_get_request(struct dram_system_t *system, struct dram_request_t *request);
void dram_system_process(struct dram_system_t *system);
void dram_decode_address(struct dram_system_t *system,
			unsigned int addr,
			unsigned int *logical_channel_id_ptr,
			unsigned int *rank_id_ptr,
			unsigned int *row_id_ptr,
			unsigned int *bank_id_ptr,
			unsigned int *column_id_ptr,
			unsigned int *physical_channel_id_ptr);
unsigned int dram_encode_address(struct dram_system_t *system,
			unsigned int logical_channel_id,
			unsigned int rank_id,
			unsigned int row_id,
			unsigned int bank_id,
			unsigned int column_id,
			unsigned int physical_channel_id);


#endif
