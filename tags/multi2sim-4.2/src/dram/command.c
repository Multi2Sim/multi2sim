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

#include "command.h"
#include "dram-system.h"


/*
 * Command
 */

static long long command_id_counter;

struct dram_command_t *dram_command_create(void)
{
	struct dram_command_t *command;
	long long cycle = esim_domain_cycle(dram_domain_index);
	
	/* Initialize */
	command = xcalloc(1, sizeof(struct dram_command_t));
	command->id = ++command_id_counter;
	command->cycle = cycle;

	/* Return */
	return command;
}


void dram_command_free(struct dram_command_t *dram_command)
{
	/* Free */
	free(dram_command);
}


void dram_command_dump(struct dram_command_t *dram_command, FILE *f)
{
	fprintf(f, "\t\tCommand ID: %llu, type: ", dram_command->id);

	switch (dram_command->type)
	{

	case dram_command_invalid:

		fprintf(f, "Invalid Command\n");
		break;

	case dram_command_refresh:

		fprintf(f, "REFRESH, Rank ID: %u, Bank ID: %u, Row ID: %u\n",
			dram_command->u.refresh.rank_id,
			dram_command->u.refresh.bank_id,
			dram_command->u.refresh.row_id);
		break;

	case dram_command_precharge:

		fprintf(f, "PRECHARGE, Rank ID: %u, Bank ID: %u\n",
			dram_command->u.precharge.rank_id,
			dram_command->u.precharge.bank_id);
		break;

	case dram_command_activate:

		fprintf(f, "ACTIVATE, Rank ID: %u, Bank ID: %u, Row ID: %u\n",
			dram_command->u.activate.rank_id,
			dram_command->u.activate.bank_id,
			dram_command->u.activate.row_id);
		break;

	case dram_command_read:

		fprintf(f, "READ, Rank ID: %u, Bank ID: %u, Column ID: %u\n",
			dram_command->u.read.rank_id,
			dram_command->u.read.bank_id,
			dram_command->u.read.column_id);
		break;
		
	case dram_command_write:

		fprintf(f, "WRITE, Rank ID: %u, Bank ID: %u, Column ID: %u\n",
			dram_command->u.write.rank_id,
			dram_command->u.write.bank_id,
			dram_command->u.write.column_id);
		break;

	case dram_command_nop:

		fprintf(f, "NOP\n");
		break;

	default:

		fprintf(f, "???\n");
		break;
	
	}
}

