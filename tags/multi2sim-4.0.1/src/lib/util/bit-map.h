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

#ifndef LIB_UTIL_BIT_MAP_H
#define LIB_UTIL_BIT_MAP_H

#include <stdio.h>


struct bit_map_t;

struct bit_map_t *bit_map_create(unsigned int size);
void bit_map_free(struct bit_map_t *bit_map);

void bit_map_set(struct bit_map_t *bit_map, unsigned int where, unsigned int size,
	unsigned int value);
unsigned int bit_map_get(struct bit_map_t *bit_map, unsigned int where, unsigned int size);
int bit_map_count_ones(struct bit_map_t *bit_map, unsigned int where, unsigned int size);
void bit_map_copy(struct bit_map_t *dst, unsigned int dst_where,
	struct bit_map_t *src, unsigned int src_where, unsigned int size);
void bit_map_dump(struct bit_map_t *bit_map, unsigned int where, unsigned int size, FILE *f);

#endif

