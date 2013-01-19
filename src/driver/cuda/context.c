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

#include "context.h"
#include "object.h"


/* Create a context */
struct frm_cuda_context_t *frm_cuda_context_create()
{
	struct frm_cuda_context_t *context;

	/* Initialize */
	context = xcalloc(1, sizeof(struct frm_cuda_context_t));
	context->id = frm_cuda_object_new_id(FRM_CUDA_OBJ_CONTEXT);
	context->ref_count = 1;

	/* Return */
	frm_cuda_object_add(context);
	return context;
}


/* Free context */
void frm_cuda_context_free(struct frm_cuda_context_t *context)
{
	frm_cuda_object_remove(context);
	free(context);
}

