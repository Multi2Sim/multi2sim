/*
 *  Multi2Sim
 *  Copyright (C) 2013  Chris Barton (barton-ch@husky.neu.edu)
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

#ifndef M2C_CL2LLVM_VECTOR_OPS_H
#define M2C_CL2LLVM_VECTOR_OPS_H

#include <llvm-c/Core.h>

#include "val.h"

/* Expands any vectors present in a list of cl2llvm_val_t's */

/* This function takes a list of values and returns the same list with each vector expanded into its components */
void expand_vectors(struct list_t *elem_list);

/* This function takes a string referencing the components of a vector and returns the index of each referenced component in an array */
void cl2llvm_get_vector_indices(struct cl2llvm_val_t *, char *);

/* Takes a pointer to a component referenced vector and performs the necessary assignment. The rvalue may be either a vector or scalar value */
struct cl2llvm_val_t *cl2llvm_build_component_wise_assignment(struct cl2llvm_val_t *lvalue_addr, 
	struct cl2llvm_val_t *rvalue);

/* Take a referenced vector and emits and error if any of the vector's components are referenced more that once */
void cl2llvm_no_repeated_component_references(struct cl2llvm_val_t *lvalue);

#endif
