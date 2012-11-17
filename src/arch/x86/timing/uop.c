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
#include <lib/util/list.h>
#include <lib/util/linked-list.h>

#include "reg-file.h"
#include "uop.h"


#define UOP_MAGIC  0x10101010U


struct x86_uop_t *x86_uop_create(void)
{
	struct x86_uop_t *uop;

	/* Initialize */
	uop = xcalloc(1, sizeof(struct x86_uop_t));
	uop->magic = UOP_MAGIC;

	/* Return */
	return uop;
}


void x86_uop_free_if_not_queued(struct x86_uop_t *uop)
{
	/* Do not free if 'uop' is still enqueued */
	if (uop->in_fetch_queue || uop->in_uop_queue || uop->in_iq ||
		uop->in_lq || uop->in_sq || uop->in_preq ||
		uop->in_rob || uop->in_event_queue ||
		uop->in_uop_trace_list)
	{
		return;
	}

	/* Free */
	uop->magic = 0;
	x86_uinst_free(uop->uinst);
	free(uop);
}


void x86_uop_dump(struct x86_uop_t *uop, FILE *f)
{
	/* Fields */
	fprintf(f, "id=%lld, ", uop->id);
	fprintf(f, "eip=0x%x, ", uop->eip);
	fprintf(f, "spec_mode=%c, ", uop->specmode ? 't' : 'f');
	fprintf(f, "trace_cache=%c, ", uop->trace_cache ? 't' : 'f');

	/* Micro-instruction */
	fprintf(f, "uinst='");
	x86_uinst_dump(uop->uinst, f);
	fprintf(f, "'");
}


/* Check whether this is a valid pointer to an allocated uop by checking
 * the magic number. */
int x86_uop_exists(struct x86_uop_t *uop)
{
	return uop->magic == UOP_MAGIC;
}


void x86_uop_list_dump(struct list_t *list, FILE *f)
{
	struct x86_uop_t *uop;
	int i;
	
	for (i = 0; i < list_count(list); i++)
	{
		fprintf(f, "%3d. ", i);
		uop = list_get(list, i);
		x86_uop_dump(uop, f);
		fprintf(f, "\n");
	}
}


void x86_uop_linked_list_dump(struct linked_list_t *uop_list, FILE *f)
{
	struct x86_uop_t *uop;
	
	linked_list_head(uop_list);
	while (!linked_list_is_end(uop_list))
	{
		uop = linked_list_get(uop_list);
		fprintf(f, "%3d. ", linked_list_current(uop_list));
		x86_uinst_dump(uop->uinst, f);
		fprintf(f, "\n");
		linked_list_next(uop_list);
	}
}


/* Update 'uop->ready' field of all instructions in a list as per the result
 * obtained by 'x86_reg_file_ready'. The 'uop->ready' field is redundant and should always
 * match the return value of 'x86_reg_file_ready' while an uop is in the ROB.
 * A debug message is dumped when the uop transitions to ready. */
void x86_uop_linked_list_check_if_ready(struct linked_list_t *uop_list)
{
	struct x86_uop_t *uop;

	LINKED_LIST_FOR_EACH(uop_list)
	{
		uop = linked_list_get(uop_list);
		if (uop->ready || !x86_reg_file_ready(uop))
			continue;
		uop->ready = 1;
	}
}

