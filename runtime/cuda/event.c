/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <time.h>

#include "../include/cuda.h"
#include "event.h"
#include "list.h"
#include "mhandle.h"


/* Create an event */
CUevent cuda_event_create(unsigned flags)
{
	CUevent event;

	/* Allocate event */
	event = xcalloc(1, sizeof(struct CUevent_st));

	/* Initialize */
	event->id = list_count(event_list);
	event->to_be_recorded = 0;
	event->recorded = 0;
	event->flags = flags;

	/* Add to event list */
	list_add(event_list, event);

	return event;
}

/* Free event */
void cuda_event_free(CUevent event)
{
	list_remove(event_list, event);

	free(event);
}

/* Record an event */
void cuda_event_record(CUevent event)
{
	event->t = clock();
}

