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

#ifndef DRIVER_CUDA_OBJECT_H
#define DRIVER_CUDA_OBJECT_H


enum frm_cuda_obj_t
{
        FRM_CUDA_OBJ_DEVICE = 1,
        FRM_CUDA_OBJ_CONTEXT,
        FRM_CUDA_OBJ_MODULE,
        FRM_CUDA_OBJ_FUNCTION,
        FRM_CUDA_OBJ_MEMORY,
        FRM_CUDA_OBJ_STREAM
};

extern struct linked_list_t *frm_cuda_object_list;

void frm_cuda_object_add(void *object);
void frm_cuda_object_remove(void *object);
void *frm_cuda_object_get(enum frm_cuda_obj_t type, unsigned int id);
unsigned int frm_cuda_object_new_id(enum frm_cuda_obj_t type);
void frm_cuda_object_free_all(void);


#endif

