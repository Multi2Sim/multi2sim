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

#include "../include/cuda.h"
#include "list.h"
#include "mhandle.h"
#include "stream.h"


struct list_t *stream_list;

/* Create a stream */
CUstream cuda_stream_create(void)
{
	CUstream stream;

	/* Initialize */
	stream = (CUstream)xcalloc(1, sizeof(struct CUstream_st));
	stream->id = list_count(stream_list);
	stream->ref_count = 1;

	list_add(stream_list, stream);

	return stream;
}

/* Free stream */
void cuda_stream_free(CUstream stream)
{
	list_remove(stream_list, stream);

	stream->ref_count--;
	free(stream);
}

