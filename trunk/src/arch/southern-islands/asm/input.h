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

#ifndef ARCH_SOUTHERN_ISLANDS_INPUT_H
#define ARCH_SOUTHERN_ISLANDS_INPUT_H

enum si_input_data_type_t
{
	si_input_data_type_invalid = 0,
	si_input_byte,
	si_input_ubyte,
	si_input_short,
	si_input_ushort,
	si_input_int,
	si_input_uint,
	si_input_hfloat,
	si_input_float,
	si_input_double,
	si_input_fixed,
	si_input_int_2_10_10_10_rev,
	si_input_uint_2_10_10_10_rev
};

struct si_input_t
{
	unsigned int usage_index; /* The associated vertex attribute index */
	int set;  /* Set to true when it is assigned */

	enum si_input_data_type_t type;
	int num_elems;
	int size;
	unsigned int device_ptr; 
};

struct si_input_t *si_input_create();
void si_input_free(struct si_input_t *input);
void si_input_set_usage_index(struct si_input_t *input, unsigned int usage_index);
unsigned int si_input_get_type(unsigned int gl_data_type);
int si_input_get_data_size(enum si_input_data_type_t data_type);

#endif
