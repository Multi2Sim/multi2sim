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

#include "bank.h"
#include "dram.h"
#include "rank.h"


/*
 * Rank
 */

struct dram_rank_t *dram_rank_create(struct dram_t *dram)
{
	struct dram_rank_t *rank;
	int i;

	/* Initialize */
	rank = xcalloc(1, sizeof(struct dram_rank_t));
	rank->dram = dram;

	/* Initialize bank array */
	rank->bank_array = xcalloc(dram->num_banks_per_device, sizeof(struct dram_bank_t *));
	for (i = 0; i < dram->num_banks_per_device; i++)
		rank->bank_array[i] = dram_bank_create(rank);

	/* Return */
	return rank;
}


void dram_rank_free(struct dram_rank_t *rank)
{
	int i;

	/* Free all banks */
	for (i = 0; i < rank->dram->num_banks_per_device; i++)
		dram_bank_free(rank->bank_array[i]);

	/* Free bank array */
	free(rank->bank_array);

	/* Free */
	free(rank);
}


void dram_rank_dump(struct dram_rank_t *rank, FILE *f)
{
	int i;

	/* Print fields */
	fprintf(f, "\tExampleVariable = 1000\n");

	/* Dump all banks */
	for (i = 0; i < rank->dram->num_banks_per_device; i++)
	{
		fprintf(f, "\tBank %d:\n", i);
		dram_bank_dump(rank->bank_array[i], f);
	}
	
	/* Last line */
	fprintf(f, "\n");
}

