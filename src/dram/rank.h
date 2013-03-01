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

#ifndef DRAM_RANK_H
#define DRAM_RANK_H

#include <stdio.h>


/*
 * Rank
 */

struct dram_rank_t
{
	/* DRAM object that the rank belongs to */
	struct dram_t *dram;

	/* Array of banks */
	struct dram_bank_t **bank_array;
};

struct dram_rank_t *dram_rank_create(struct dram_t *dram);
void dram_rank_free(struct dram_rank_t *rank);
void dram_rank_dump(struct dram_rank_t *rank, FILE *f);


#endif
