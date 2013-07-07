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

#ifndef ARCH_COMMON_TIMING_H
#define ARCH_COMMON_TIMING_H

#include <lib/util/class.h>


/* Class macros */
#define TIMING_TYPE 0x29276c06
#define TIMING(p) (CLASS_REINTERPRET_CAST(p, TIMING_TYPE, struct timing_t))
#define TIMING_CLASS_OF(p) (CLASS_OF(p, TIMING_TYPE))

struct timing_t
{
	/* This is a class
	 * WARNING - must be the first field */
	struct class_t class_info;

	
	/*** Virtual methods ***/

	void (*free)(struct timing_t *timing);
};


struct timing_t *timing_create(void);
void timing_free(struct timing_t *timing);

#endif

