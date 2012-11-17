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

#ifndef ARCH_X86_EMU_CLRT_H
#define ARCH_X86_EMU_CLRT_H

/* Forward type declaration */
struct x86_ctx_t;


#define x86_clrt_debug(...) debug(x86_clrt_debug_category, __VA_ARGS__)
extern int x86_clrt_debug_category;

void x86_clrt_init(void);
void x86_clrt_done(void);

int x86_clrt_call(struct x86_ctx_t *ctx);


#endif

