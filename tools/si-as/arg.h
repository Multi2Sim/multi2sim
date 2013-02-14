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

#include <stdio.h>

enum si_arg_type_t 
{
	si_arg_invalid = 0,
	si_arg_scalar_register,
	si_arg_vector_register,
	si_arg_register_range,
	si_arg_mtype_register,
	si_arg_special_register,
	si_arg_literal,
	si_arg_waitcnt,
	si_arg_label,
	si_arg_format	
};

enum si_arg_special_register_type_t
{
	si_arg_special_register_vcc = 0,
	si_arg_special_register_scc
};

struct si_arg_t 
{
	enum si_arg_type_t type;
	
	union
	{
	
		struct
		{
			int id;
		} scalar_register;
		
		struct
		{
			int id_low;
			int id_high;
		} register_range;
		
		struct
		{
			int id;
		} 
		vector_register;
		
		struct
		{
			unsigned int val;
		} literal;
		
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
		} mtype_register;
		
		struct
		{
			char *data_format;
			char *num_format;
			
			int offen;
			int offset;
		} format;
		
		struct
		{
			enum si_arg_special_register_type_t type;
		} special_register;
		
	} value;
};

struct si_arg_t *si_arg_create(void);
void si_arg_free(struct si_arg_t *inst_arg);
void si_arg_dump(struct si_arg_t *inst_arg, FILE *f);

