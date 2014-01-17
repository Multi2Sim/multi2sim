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

#ifndef DRAM_BANK_H
#define DRAM_BANK_H

#include <stdio.h>


/*
 * Bank
 */

struct dram_bank_t
{
	/* Bank object that the rank belongs to */
	struct dram_rank_t *rank;

	/* Active row */
	int row_buffer_valid;
	int active_row_id;

	/* Bank busy */
	int bank_busy;
};

struct dram_bank_t *dram_bank_create(struct dram_rank_t *rank);

void dram_bank_free(struct dram_bank_t *bank);
void dram_bank_dump(struct dram_bank_t *bank, FILE *f);


#endif
