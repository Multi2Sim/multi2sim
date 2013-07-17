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

#include <lib/mhandle/mhandle.h>
#include <lib/util/string.h>

#include "core.h"
#include "cpu.h"
#include "event-queue.h"
#include "fu.h"
#include "rob.h"
#include "thread.h"


/*
 * Class 'X86Core'
 */

CLASS_IMPLEMENTATION(X86Core);

void X86CoreCreate(X86Core *self, X86Cpu *cpu)
{
	int i;

	/* Initialize */
	self->cpu = cpu;

	/* Create threads */
	self->threads = xcalloc(x86_cpu_num_threads, sizeof(X86Thread *));
	for (i = 0; i < x86_cpu_num_threads; i++)
		self->threads[i] = new(X86Thread, self);

	/* Prefetcher */
	self->prefetch_history = prefetch_history_create();

	/* Structures */
	X86CoreInitROB(self);
	X86CoreInitEventQueue(self);
	X86CoreInitFunctionalUnits(self);
}


void X86CoreDestroy(X86Core *self)
{
	int i;

	/* Name */
	self->name = str_free(self->name);

	/* Free threads */
	for (i = 0; i < x86_cpu_num_threads; i++)
		delete(self->threads[i]);
	free(self->threads);

	/* Prefetcher */
	prefetch_history_free(self->prefetch_history);

	/* Structures */
	X86CoreFreeROB(self);
	X86CoreFreeEventQueue(self);
	X86CoreFreeFunctionalUnits(self);
}


void X86CoreSetName(X86Core *self, char *name)
{
	self->name = str_set(self->name, name);
}
