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

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "fibers.h"
#include "mhandle.h"



void make_fiber(struct fiber_t *fiber, fiber_proc_t fiber_proc, int num_args, ...)
{
	int i;
	int *params;
	int stack_words;
	int *stack_bottom;

	params = &num_args + 1;

	assert(!((int) params % sizeof (int)));
	assert(!((int) fiber->stack_bottom % sizeof (int)));
	assert(!(fiber->stack_size % sizeof (int)));

	stack_words = fiber->stack_size / sizeof (int);
	stack_bottom = (int *) fiber->stack_bottom;

	for (i = 0; i < num_args; i++)
		stack_bottom[stack_words - num_args + i] = params[i];
	/* return address of fake 'calling function' */
	stack_bottom[stack_words - num_args - 1] = 0;

	fiber->eip = fiber_proc;
	fiber->esp = stack_bottom + stack_words - num_args - 1;
}

void make_fiber_ex(struct fiber_t *fiber, fiber_proc_t fiber_proc, return_address_t return_proc, int arg_size, void *args)
{
	arg_size *= sizeof (size_t);
	assert(!(arg_size % sizeof (size_t)));
	assert(!((size_t) fiber->stack_bottom % sizeof (size_t)));
	assert(!(fiber->stack_size % sizeof (size_t)));

	memcpy((char *) fiber->stack_bottom + fiber->stack_size - arg_size, args, arg_size);

	
	fiber->eip = fiber_proc;
	fiber->esp = (char *)fiber->stack_bottom + fiber->stack_size - arg_size - sizeof (size_t);
	*(size_t *) fiber->esp = (size_t)return_proc;
}

