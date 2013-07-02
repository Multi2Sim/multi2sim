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

#include "memory.h"


struct cuda_memory_t *cuda_memory_create(void)
{
        struct cuda_memory_t *mem;

        /* Initialize */
        mem = xcalloc(1, sizeof(struct cuda_memory_t));
        //mem->id = cuda_object_new_id(CUDA_OBJ_MEMORY);
        mem->ref_count = 1;

        //cuda_object_add(mem);

        return mem;
}


void cuda_memory_free(struct cuda_memory_t *mem)
{
        //cuda_object_remove(mem);

        free(mem);
}

