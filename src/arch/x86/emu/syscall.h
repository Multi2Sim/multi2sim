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

#ifndef ARCH_X86_EMU_SYSCALL_H
#define ARCH_X86_EMU_SYSCALL_H

#include <stdio.h>

/* Forward type declarations */
struct x86_ctx_t;


#define x86_sys_debug(...) debug(x86_sys_debug_category, __VA_ARGS__)
#define x86_sys_debug_buffer(...) debug_buffer(x86_sys_debug_category, __VA_ARGS__)
extern int x86_sys_debug_category;

void x86_sys_init(void);
void x86_sys_done(void);
void x86_sys_dump(FILE *f);

void x86_sys_call(struct x86_ctx_t *ctx);
 

#endif

