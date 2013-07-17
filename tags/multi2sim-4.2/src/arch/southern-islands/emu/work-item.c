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

#include "work-item.h"



/*
 * Public Functions
 */

struct si_work_item_t *si_work_item_create()
{
	struct si_work_item_t *work_item;

	/* Initialize */
	work_item = xcalloc(1, sizeof(struct si_work_item_t));

	/* Return */
	return work_item;
}


void si_work_item_free(struct si_work_item_t *work_item)
{
	/* Free work_item */
	free(work_item);
}

