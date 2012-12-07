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

#ifndef X86_ARCH_TIMING_INST_QUEUE_H
#define X86_ARCH_TIMING_INST_QUEUE_H

#include "uop.h"


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


#endif

