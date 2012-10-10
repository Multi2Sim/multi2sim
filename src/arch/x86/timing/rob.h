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


extern char *x86_rob_kind_map[];
extern enum x86_rob_kind_t
{
	x86_rob_kind_private = 0,
	x86_rob_kind_shared
} x86_rob_kind;
extern int x86_rob_size;

void x86_rob_init(void);
void x86_rob_done(void);
void x86_rob_dump(int core, FILE *f);

int x86_rob_can_enqueue(struct x86_uop_t *uop);
void x86_rob_enqueue(struct x86_uop_t *uop);
int x86_rob_can_dequeue(int core, int thread);
struct x86_uop_t *x86_rob_head(int core, int thread);
void x86_rob_remove_head(int core, int thread);
struct x86_uop_t *x86_rob_tail(int core, int thread);
void x86_rob_remove_tail(int core, int thread);
struct x86_uop_t *x86_rob_get(int core, int thread, int index);

#endif

