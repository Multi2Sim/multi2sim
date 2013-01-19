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

#include <lib/mhandle/mhandle.h>

#include "stream.h"
#include "object.h"


/* Create a stream */
struct frm_cuda_stream_t *frm_cuda_stream_create(void)
{
	struct frm_cuda_stream_t *stream;

	/* Initialize */
	stream = xcalloc(1, sizeof(struct frm_cuda_stream_t));
	stream->id = frm_cuda_object_new_id(FRM_CUDA_OBJ_STREAM);
	stream->ref_count = 1;

	/* Return */
	frm_cuda_object_add(stream);
	return stream;
}


/* Free stream */
void frm_cuda_stream_free(struct frm_cuda_stream_t *stream)
{
	frm_cuda_object_remove(stream);
	free(stream);
}

