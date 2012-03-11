/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <cpuarch.h>


#define UOP_MAGIC  0x10101010U


struct uop_t *uop_create(void)
{
	struct uop_t *uop;

	uop = calloc(1, sizeof(struct uop_t));
	uop->magic = UOP_MAGIC;
	return uop;
}


void uop_free_if_not_queued(struct uop_t *uop)
{
	/* Do not free if 'uop' is still enqueued */
	if (uop->in_fetchq || uop->in_uopq || uop->in_iq ||
		uop->in_lq || uop->in_sq ||
		uop->in_rob || uop->in_eventq)
	{
		return;
	}

	/* Free */
	uop->magic = 0;
	x86_uinst_free(uop->uinst);
	free(uop);
}


/* Check whether this is a valid pointer to an allocated uop by checking
 * the magic number. */
int uop_exists(struct uop_t *uop)
{
	return uop->magic == UOP_MAGIC;
}


void uop_list_dump(struct list_t *uop_list, FILE *f)
{
	struct uop_t *uop;
	int i;
	
	for (i = 0; i < list_count(uop_list); i++)
	{
		uop = list_get(uop_list, i);
		fprintf(f, "%3d. ", i);
		x86_uinst_dump(uop->uinst, f);
		fprintf(f, "\n");
	}
}


void uop_lnlist_dump(struct linked_list_t *uop_list, FILE *f)
{
	struct uop_t *uop;
	
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
 * obtained by 'rf_ready'. The 'uop->ready' field is redundant and should always
 * match the return value of 'rf_ready' while an uop is in the ROB.
 * A debug message is dumped when the uop transitions to ready. */
void uop_lnlist_check_if_ready(struct linked_list_t *uop_list)
{
	struct uop_t *uop;
	linked_list_head(uop_list);
	for (linked_list_head(uop_list); !linked_list_is_end(uop_list); linked_list_next(uop_list)) {
		uop = linked_list_get(uop_list);
		if (uop->ready || !rf_ready(uop))
			continue;
		uop->ready = 1;
		esim_debug("uop action=\"update\", core=%d, seq=%lld, ready=1\n",
			uop->core, uop->di_seq);
	}
}

