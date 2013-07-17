/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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


#include <arch/southern-islands/asm/arg.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/list.h>

#include "metadata.h"


/*
* Metadata Object
*/

struct si2bin_metadata_t *si2bin_metadata_create(void)
{
       struct si2bin_metadata_t *metadata;

       metadata = xcalloc(1, sizeof(struct si2bin_metadata_t));

       metadata->arg_list = list_create();
       
	/* Return */
       return metadata;
}

void si2bin_metadata_free(struct si2bin_metadata_t *metadata)
{
	struct si_arg_t *arg;
	int i;
	
	LIST_FOR_EACH(metadata->arg_list, i)
	{
		arg = list_get(metadata->arg_list, i);
		si_arg_free(arg);
	}

	list_free(metadata->arg_list);

	free(metadata);
}

void si2bin_metadata_add_arg(struct si2bin_metadata_t *metadata, struct si_arg_t *arg)
{
	list_add(metadata->arg_list, arg);
}


