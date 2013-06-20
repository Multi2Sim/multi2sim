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

#ifndef ARCH_SOUTHERN_ISLANDS_ARG_H
#define ARCH_SOUTHERN_ISLANDS_ARG_H

#include <stdio.h>


/* String Maps */

extern struct str_map_t si_arg_dimension_map;
extern struct str_map_t si_arg_access_type_map;
extern struct str_map_t si_arg_data_type_map;
extern struct str_map_t si_arg_scope_map;
extern struct str_map_t si_arg_reflection_map;

enum si_arg_type_t
{
	si_arg_type_invalid = 0,
	si_arg_value,
	si_arg_pointer,
	si_arg_image,
	si_arg_sampler
};

enum si_arg_access_type_t
{
	si_arg_access_type_invalid = 0,
	si_arg_read_only,
	si_arg_write_only,
	si_arg_read_write
};

enum si_arg_scope_t
{
	si_arg_scope_invalid = 0,
	si_arg_global,
	si_arg_emu_private,
	si_arg_emu_local,
	si_arg_uav,
	si_arg_emu_constant,
	si_arg_emu_gds,
	si_arg_hw_local,
	si_arg_hw_private,
	si_arg_hw_constant,
	si_arg_hw_gds
};

enum si_arg_data_type_t
{
	si_arg_data_type_invalid = 0,
	si_arg_i1,
	si_arg_i8,
	si_arg_i16,
	si_arg_i32,
	si_arg_i64,
	si_arg_u1,
	si_arg_u8,
	si_arg_u16,
	si_arg_u32,
	si_arg_u64,
	si_arg_float,
	si_arg_double,
	si_arg_struct,
	si_arg_union,
	si_arg_event,
	si_arg_opaque
};

struct si_arg_value_t
{
	/* Metadata info */
	enum si_arg_data_type_t data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;

	/* Value set by user */
	void *value_ptr;
};

struct si_arg_pointer_t
{
	/* Metadata info */
	enum si_arg_data_type_t data_type;
	int num_elems;
	int constant_buffer_num;
	int constant_offset;
	enum si_arg_scope_t scope;
	int buffer_num;
	int alignment;
	enum si_arg_access_type_t access_type;

	/* Value set by user */
	unsigned int device_ptr;
};

struct si_arg_image_t
{
	int dimension;  /* 2 or 3 */
	enum si_arg_access_type_t access_type;
	int uav;
	int constant_buffer_num;
	int constant_offset;
};

struct si_arg_sampler_t
{
	int id;
	unsigned int location;
	int value;
};

struct si_arg_t
{
	enum si_arg_type_t type;
	char *name;
	int set;  /* Set to true when it is assigned */
	int size; /* Inferred from metadata or user calls */
	int constarg; /*Set to true when argument is constant */

	union
	{
		struct si_arg_value_t value;
		struct si_arg_pointer_t pointer;
		struct si_arg_image_t image;
		struct si_arg_sampler_t sampler;
	};
};


struct si_arg_t *si_arg_create(enum si_arg_type_t type,
		char *name);
void si_arg_free(struct si_arg_t *arg);

void si_arg_name_set(struct si_arg_t *arg, char *name);

int si_arg_get_data_size(enum si_arg_data_type_t data_type);

#endif
