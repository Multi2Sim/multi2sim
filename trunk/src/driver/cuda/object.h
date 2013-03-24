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

enum cuda_obj_t
{
        CUDA_OBJ_DEVICE = 1,
        CUDA_OBJ_CONTEXT,
        CUDA_OBJ_MODULE,
        CUDA_OBJ_FUNCTION,
        CUDA_OBJ_MEMORY,
        CUDA_OBJ_STREAM
};

extern struct linked_list_t *cuda_object_list;

void cuda_object_add(void *object);
void cuda_object_remove(void *object);
void *cuda_object_get(enum cuda_obj_t type, unsigned int id);
unsigned int cuda_object_new_id(enum cuda_obj_t type);
void cuda_object_free_all(void);

#endif

