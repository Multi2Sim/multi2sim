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

#ifndef ARCH_MIPS_EMU_SYSCALL_H
#define ARCH_MIPS_EMU_SYSCALL_H


//#define ARM_set_tls 0xF0005
//#define ARM_exit_group 248

#define mips_sys_debug(...) debug(mips_sys_debug_category, __VA_ARGS__)
#define mips_sys_debug_buffer(...) debug_buffer(mips_sys_debug_category, __VA_ARGS__)
extern int mips_sys_debug_category;

void mips_sys_init(void);
void mips_sys_done(void);

void mips_sys_call(struct mips_ctx_t *ctx);




#endif
