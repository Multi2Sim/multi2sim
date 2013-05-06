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

#ifndef TOOLS_CLCC_SI2BIN_ARG_H
#define TOOLS_CLCC_SI2BIN_ARG_H

#include <stdio.h>

/* Forward declarations */
struct si2bin_symbol_t;


enum si2bin_arg_type_t 
{
	si2bin_arg_invalid = 0,

	si2bin_arg_scalar_register,
	si2bin_arg_vector_register,
	si2bin_arg_scalar_register_series,
	si2bin_arg_vector_register_series,
	si2bin_arg_mem_register,
	si2bin_arg_special_register,
	si2bin_arg_literal,
	si2bin_arg_literal_float,
	si2bin_arg_waitcnt,
	si2bin_arg_label,
	si2bin_arg_maddr,
	si2bin_arg_maddr_qual
};

/* NOTE: modify string map 'si_arg_special_register_map' in asm.c together
 * with this enumeration. */
enum si2bin_arg_special_register_type_t
{
	si2bin_arg_special_register_invalid = 0,
	si2bin_arg_special_register_vcc,
	si2bin_arg_special_register_scc,
	si2bin_arg_special_register_exec
};

struct si2bin_arg_t 
{
	enum si2bin_arg_type_t type;
	int abs;  /* Absolute value */
	int neg;  /* Negation */

	union
	{
	
		struct
		{
			int id;
		} scalar_register;
		
		struct
		{
			int low;
			int high;
		} scalar_register_series;

		struct
		{
			int low;
			int high;
		} vector_register_series;
		
		struct
		{
			int id;
		} 
		vector_register;
		
		struct
		{
			int val;
		} literal;
		
		struct
		{
			float val;
		} literal_float;

		struct
		{
			int vmcnt_active;
			int vmcnt_value;

			int lgkmcnt_active;
			int lgkmcnt_value;

			int expcnt_active;
			int expcnt_value;
		} wait_cnt;
		
		struct
		{
			int id;
		} mem_register;
		
		struct
		{
			/* Sub-argument of type 'vector', 'scalar', 'literal',
			 * or 'literal_float'. */
			struct si2bin_arg_t *soffset;

			/* Sub-argument of type 'maddr_qual'
			 * (memory address qualifier) */
			struct si2bin_arg_t *qual;

			char *data_format;
			char *num_format;
		} maddr;

		struct
		{
			int offen;
			int idxen;
			int offset;
		} maddr_qual;
		
		struct
		{
			enum si2bin_arg_special_register_type_t type;
		} special_register;

		struct
		{
			struct si2bin_symbol_t *symbol;
		} label;
		
	} value;
};

struct si2bin_arg_t *si2bin_arg_create(void);
void si2bin_arg_free(struct si2bin_arg_t *inst_arg);

struct si2bin_arg_t *si2bin_arg_create_literal(int value);
struct si2bin_arg_t *si2bin_arg_create_literal_float(float value);
struct si2bin_arg_t *si2bin_arg_create_scalar_register(char *name);
struct si2bin_arg_t *si2bin_arg_create_vector_register(char *name);
struct si2bin_arg_t *si2bin_arg_create_special_register(char *name);
struct si2bin_arg_t *si2bin_arg_create_maddr(struct si2bin_arg_t *soffset,
		struct si2bin_arg_t *qual, char *data_format, char *num_format);
struct si2bin_arg_t *si2bin_arg_create_maddr_qual(void);
struct si2bin_arg_t *si2bin_arg_create_label(struct si2bin_symbol_t *symbol);

int si2bin_arg_encode_operand(struct si2bin_arg_t *arg);

void si2bin_arg_dump(struct si2bin_arg_t *inst_arg, FILE *f);


#endif

