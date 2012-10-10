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

#ifndef X86_ARCH_TIMING_FETCH_QUEUE_H
#define X86_ARCH_TIMING_FETCH_QUEUE_H

extern int x86_fetch_queue_size;

void x86_fetch_queue_init(void);
void x86_fetch_queue_done(void);

void x86_fetch_queue_recover(int core, int thread);
struct x86_uop_t *x86_fetch_queue_remove(int core, int thread, int index);


#endif

