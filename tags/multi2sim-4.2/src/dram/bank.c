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


/*
 * Bank
 */

struct dram_bank_t *dram_bank_create(struct dram_rank_t *rank)
{
	struct dram_bank_t *bank;

	/* Initialize */
	bank = xcalloc(1, sizeof(struct dram_bank_t));
	bank->rank = rank;

	/* Return */
	return bank;
}


void dram_bank_free(struct dram_bank_t *bank)
{
	/* Free */
	free(bank);
}


void dram_bank_dump(struct dram_bank_t *bank, FILE *f)
{
	/* Active row */
	fprintf(f, "\t\tActive row: ");
	if (bank->row_buffer_valid)
		fprintf(f, "%d\n", bank->active_row_id);
	else
		fprintf(f, "none\n");

	/* Last line */
	fprintf(f, "\n");
}
