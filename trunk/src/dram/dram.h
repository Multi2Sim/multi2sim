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

#ifndef DRAM_DRAM_H
#define DRAM_DRAM_H

#include <stdio.h>


/*
 * Global Variables
 */


extern int EV_DRAM_COMMAND_RECEIVE;
extern int EV_DRAM_COMMAND_COMPLETE;



/*
 * DRAM
 */

struct dram_t
{
	/* Structure parameter */
	unsigned int num_ranks;
	unsigned int num_devices_per_rank;
	unsigned int num_banks_per_device;
	unsigned int num_rows_per_bank;
	unsigned int num_columns_per_row;
	unsigned int num_bits_per_column;

	/* Timing parameter */
	unsigned int timing_tCAS;
	unsigned int timing_tRCD;
	unsigned int timing_tRP;
	unsigned int timing_tRAS;
	unsigned int timing_tCWL;

	/* Array of ranks */
	struct dram_rank_t **rank_array;
};


void dram_init(void);
void dram_done(void);
struct dram_t *dram_create(unsigned int num_ranks,
				unsigned int num_devices_per_rank,
				unsigned int num_banks_per_device,
				unsigned int num_rows_per_bank,
				unsigned int num_columns_per_row,
				unsigned int num_bits_per_column);
//void dram_timing_config(); //FIXME: decide whether to use this function or not.
void dram_free(struct dram_t *dram);
void dram_dump(struct dram_t *dram, FILE *f);
void dram_event_handler(int event, void *data);


#endif

