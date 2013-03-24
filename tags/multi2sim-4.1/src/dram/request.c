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
#include <lib/util/debug.h>

#include "request.h"


/*
 * Request
 */

struct dram_request_t *dram_request_create(void)
{
	struct dram_request_t *request;

	/* Initialize */
	request = xcalloc(1, sizeof(struct dram_request_t));

	/* Return */
	return request;
}


void dram_request_free(struct dram_request_t *request)
{
	free(request);
}


void dram_request_dump(struct dram_request_t *dram_request, FILE *f)
{
	fprintf(f, "\t\tRequest ID: %llu, type: ", dram_request->id);

	switch (dram_request->type)
	{

	case request_type_read:

		fprintf(f, "READ, ");
		break;

	case request_type_write:

		fprintf(f, "WRITE, ");
		break;

	default:

		fprintf(f, "???, ");
		break;

	}

	fprintf(f, "addr: %08X\n", dram_request->addr);
}
