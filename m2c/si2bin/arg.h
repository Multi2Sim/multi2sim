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

#ifndef M2C_SI2BIN_ARG_H
#define M2C_SI2BIN_ARG_H

#include <arch/southern-islands/asm/inst.h>
#include <lib/util/misc.h>

#include <stdio.h>


/* Forward declarations */
struct si2bin_symbol_t;

extern struct str_map_t si2bin_arg_type_map;
typedef enum
{
	Si2binArgInvalid = 0,

	Si2binArgScalarRegister,
	Si2binArgVectorRegister,
	Si2binArgScalarRegisterSeries,
	Si2binArgVectorRegisterSeries,
	Si2binArgMemRegister,
	Si2binArgSpecialRegister,
	Si2binArgLiteral,
	Si2binArgLiteralReduced,
	Si2binArgLiteralFloat,
	Si2binArgLiteralFloatReduced,
	Si2binArgWaitcnt,
	Si2binArgLabel,
	Si2binArgMaddr,
	Si2binArgMaddrQual
} Si2binArgType;

/* Return true if the argument type is any kind of literal */
#define Si2binArgIsConstant(arg) \
	((arg)->type == Si2binArgLiteral || \
	(arg)->type == Si2binArgLiteralReduced || \
	(arg)->type == Si2binArgLiteralFloat || \
	(arg)->type == Si2binArgLiteralFloatReduced)


CLASS_BEGIN(Si2binArg, Object)

	/* Argument type */
	Si2binArgType type;

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
	
		/* Used when 'type' is 'si2bin_arg_type_literal' or
		 * 'si2bin_arg_type_literal_reduced' */
		struct
		{
			int val;
		} literal;
		
		/* Used when 'type' is 'si2bin_arg_type_literal_float' or
		 * 'si2bin_arg_type_literal_float_reduced' */
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
			Si2binArg *soffset;

			/* Sub-argument of type 'maddr_qual'
			 * (memory address qualifier) */
			Si2binArg *qual;

			enum si_inst_buf_data_format_t data_format;
			enum si_inst_buf_num_format_t num_format;
		} maddr;

		struct
		{
			int offen;
			int idxen;
			int offset;
		} maddr_qual;
		
		struct
		{
			enum si_inst_special_reg_t reg;
		} special_register;

		struct
		{
			char *name;
		} label;
		
	} value;

CLASS_END(Si2binArg)


/* Constructors and destructor */
void Si2binArgCreate(Si2binArg *self);
void Si2binArgCreateLiteral(Si2binArg *self, int value);
void Si2binArgCreateLiteralFloat(Si2binArg *self, float value);
void Si2binArgCreateScalarRegister(Si2binArg *self, int id);
void Si2binArgCreateScalarRegisterSeries(Si2binArg *self, int low, int high);
void Si2binArgCreateVectorRegister(Si2binArg *self, int id);
void Si2binArgCreateVectorRegisterSeries(Si2binArg *self, int low, int high);
void Si2binArgCreateSpecialRegister(Si2binArg *self, enum si_inst_special_reg_t reg);
void Si2binArgCreateMemRegister(Si2binArg *self, int id);
void Si2binArgCreateMaddr(Si2binArg *self, Si2binArg *soffset,
		Si2binArg *qual,
		enum si_inst_buf_data_format_t data_format,
		enum si_inst_buf_num_format_t num_format);
void Si2binArgCreateMaddrQual(Si2binArg *self);
void Si2binArgCreateLabel(Si2binArg *self, char *name);
void Si2binArgDestroy(Si2binArg *self);

void Si2binArgDump(Si2binArg *self, FILE *f);
void Si2binArgDumpAssembly(Si2binArg *self, FILE *f);

int Si2binArgEncodeOperand(Si2binArg *self);


/* Check that an argument is of any of the types listed in the function. For
 * example, this function could be used like this:
 *	si2bin_arg_valid_types(arg, si2bin_arg_literal,
 *		si2bin_arg_scalar_register);
 */
#define Si2binArgValidTypes(arg, ...) \
	__Si2binArgValidTypes(arg, __FUNCTION__, \
		PP_NARG(__VA_ARGS__), __VA_ARGS__)
void __Si2binArgValidTypes(Si2binArg *self, const char *user_message,
		int num_args, ...);



/*
 * Public
 */

void Si2binArgSwap(Si2binArg **arg1_ptr, Si2binArg **arg2_ptr);


#endif

