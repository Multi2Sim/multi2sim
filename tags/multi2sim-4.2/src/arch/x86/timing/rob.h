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

#ifndef X86_ARCH_TIMING_ROB_H
#define X86_ARCH_TIMING_ROB_H

#include <lib/util/class.h>

/*
 * Public
 */

extern char *x86_rob_kind_map[];
extern enum x86_rob_kind_t
{
	x86_rob_kind_private = 0,
	x86_rob_kind_shared
} x86_rob_kind;
extern int x86_rob_size;



/*
 * Class 'X86Core'
 */

void X86CoreInitROB(X86Core *self);
void X86CoreFreeROB(X86Core *self);

void X86CoreDumpROB(X86Core *self, FILE *f);
int X86CoreCanEnqueueInROB(X86Core *self, struct x86_uop_t *uop);
void X86CoreEnqueueInROB(X86Core *self, struct x86_uop_t *uop);


/*
 * Class 'X86Thread'
 */

int X86ThreadCanDequeueFromROB(X86Thread *self);
struct x86_uop_t *X86ThreadGetROBHead(X86Thread *self);
void X86ThreadRemoveROBHead(X86Thread *self);
struct x86_uop_t *X86ThreadGetROBTail(X86Thread *self);
void X86ThreadRemoveROBTail(X86Thread *self);
struct x86_uop_t *X86GetROBEntry(X86Thread *self, int index);


#endif

