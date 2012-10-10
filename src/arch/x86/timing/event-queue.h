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

#ifndef X86_ARCH_TIMING_EVENT_QUEUE_H
#define X86_ARCH_TIMING_EVENT_QUEUE_H


void x86_event_queue_init(void);
void x86_event_queue_done(void);

int x86_event_queue_long_latency(int core, int thread);
int x86_event_queue_cache_miss(int core, int thread);
void x86_event_queue_insert(struct linked_list_t *event_queue, struct x86_uop_t *uop);
struct x86_uop_t *x86_event_queue_extract(struct linked_list_t *event_queue);
void x86_event_queue_recover(int core, int thread);


#endif

