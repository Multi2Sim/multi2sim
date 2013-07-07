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
 *  You should have received as copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_COMMON_ASM_H
#define ARCH_COMMON_ASM_H

#include <lib/util/class.h>


/* Class macros */
#define ASM_TYPE 0x46c0ba31
#define ASM(p) (CLASS_REINTERPRET_CAST(p, ASM_TYPE, struct asm_t))
#define ASM_CLASS_OF(p) (CLASS_OF(p, ASM_TYPE))


/* Class:	asm_t
 * Inherits:	-
 */
struct asm_t
{
	/* This is as class
	 * WARNING - must be the first field */
	struct class_t class_info;

	
	/*** Virtual methods ***/

	void (*decode_binary)(char *path);

	void (*decode_buffer)(void *ptr, int size);

	void (*free)(struct asm_t *as);
};


struct asm_t *asm_create(void);
void asm_free(struct asm_t *as);

#endif
