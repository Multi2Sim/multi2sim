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

#ifndef X86_ARCH_TIMING_TIMING_H
#define X86_ARCH_TIMING_TIMING_H

#include <time.h>
#include <signal.h>

#include <arch/x86/emu/context.h>
#include <arch/x86/emu/emu.h>
#include <arch/x86/emu/regs.h>
#include <arch/x86/emu/uinst.h>
#include <lib/struct/list.h>
#include <lib/struct/linked-list.h>
#include <lib/struct/repos.h>
#include <mem-system/mem-system.h>



/* Environment variables */
extern char **environ;



/*
 * Fetch Queue
 */

extern int x86_fetch_queue_size;

void x86_fetch_queue_init(void);
void x86_fetch_queue_done(void);

void x86_fetch_queue_recover(int core, int thread);
struct x86_uop_t *x86_fetch_queue_remove(int core, int thread, int index);




/*
 * Uop Queue
 */

extern int x86_uop_queue_size;

void x86_uop_queue_init(void);
void x86_uop_queue_done(void);

void x86_uop_queue_recover(int core, int thread);




/*
 * Instruction Queue
 */

extern char *x86_iq_kind_map[];
extern enum x86_iq_kind_t
{
	x86_iq_kind_shared = 0,
	x86_iq_kind_private
} x86_iq_kind;
extern int x86_iq_size;

void x86_iq_init(void);
void x86_iq_done(void);

int x86_iq_can_insert(struct x86_uop_t *uop);
void x86_iq_insert(struct x86_uop_t *uop);
void x86_iq_remove(int core, int thread);
void x86_iq_recover(int core, int thread);




/*
 * Load/Store Queue
 */

extern char *x86_lsq_kind_map[];
extern enum x86_lsq_kind_t
{
	x86_lsq_kind_shared = 0,
	x86_lsq_kind_private
} x86_lsq_kind;
extern int x86_lsq_size;

void x86_lsq_init(void);
void x86_lsq_done(void);

int x86_lsq_can_insert(struct x86_uop_t *uop);
void x86_lsq_insert(struct x86_uop_t *uop);
void x86_lsq_recover(int core, int thread);

void x86_lq_remove(int core, int thread);
void x86_sq_remove(int core, int thread);




/*
 * Event Queue
 */

void x86_event_queue_init(void);
void x86_event_queue_done(void);

int x86_event_queue_long_latency(int core, int thread);
int x86_event_queue_cache_miss(int core, int thread);
void x86_event_queue_insert(struct linked_list_t *event_queue, struct x86_uop_t *uop);
struct x86_uop_t *x86_event_queue_extract(struct linked_list_t *event_queue);
void x86_event_queue_recover(int core, int thread);





#endif

