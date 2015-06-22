/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_SOUTHERN_ISLANDS_FETCH_SHADER_H
#define ARCH_SOUTHERN_ISLANDS_FETCH_SHADER_H

struct opengl_si_shader_t;
struct si_fetch_shader_t
{
	unsigned int size; 
	void *isa; /* Final ISA buffer */

	/* List contains instruction objects defined in asm.h */
	struct list_t *isa_list; 
};

struct si_fetch_shader_t *si_fetch_shader_create(struct opengl_si_shader_t *shdr);
void si_fetch_shader_free(struct si_fetch_shader_t *fs);

#endif
