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

#include <assert.h>

#include <arch/southern-islands/asm/asm.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "arg.h"
#include "dis-inst.h"
#include "main.h"


/* Hash table indexed by an instruction name, returning the associated entry in
 * 'si_inst_info' of type 'si_inst_info_t'. The name of the instruction is
 * extracted from the first token of the format string. */
struct hash_table_t *si_dis_inst_table;


void si_dis_inst_init(void)
{
	struct si_inst_info_t *info;
	struct list_t *tokens;

	char *name;
	int i;

	/* Initialize hash table with instruction names. */
	si_dis_inst_table = hash_table_create(SI_INST_COUNT, 1);
	for (i = 0; i < SI_INST_COUNT; i++)
	{
		/* Get instruction info */
		info = &si_inst_info[i];
		if (!info->name || !info->fmt_str)
			continue;

		/* Split format string in tokens */
		tokens = str_token_list_create(info->fmt_str, " ");
		if (!list_count(tokens))
			continue;

		/* Insert */
		name = list_get(tokens, 0);
		hash_table_insert(si_dis_inst_table, name, info);
		str_token_list_free(tokens);
	}
}


void si_dis_inst_done(void)
{
	hash_table_free(si_dis_inst_table);
}


struct si_dis_inst_t *si_dis_inst_create(char *name, struct list_t *arg_list)
{
	struct si_dis_inst_t *inst;
	struct si_inst_info_t *info;
	struct list_t *tokens;
	
	/* Allocate */
	inst = xcalloc(1, sizeof(struct si_dis_inst_t));
	
	/* Initialize */
	if (!arg_list)
		arg_list = list_create();
	inst->arg_list = arg_list;
	
	/* Look up instruction name */
	info = hash_table_get(si_dis_inst_table, name);
	if (!info)
		yyerror_fmt("invalid instruction: %s", name);

	/* Initialize opcode */
	inst->info = info;
	inst->opcode = info->opcode;
	
	/* Split format string in tokens, discarding instruction name */
	tokens = str_token_list_create(info->fmt_str, ", ");
	assert(tokens->count);
	str_token_list_shift(tokens);

	/* Check number of arguments */
	if (arg_list->count != tokens->count)
		yyerror_fmt("wrong number of arguments for %s", name);

	/* Free token list for format string */
	str_token_list_free(tokens);

	/* Return */
	return inst;
}


void si_dis_inst_free(struct si_dis_inst_t *inst)
{
	int index;
	struct si_arg_t *arg;
	
	/* Free all argument object in the argument list */
	for (index = 0; index < inst->arg_list->count; index++)
	LIST_FOR_EACH(inst->arg_list, index)
	{
		arg = list_get(inst->arg_list, index);
		si_arg_free(arg);
	}
	
	/* Free argument list and instruction object */
	list_free(inst->arg_list);
	free(inst);
}


void si_dis_inst_dump(struct si_dis_inst_t *inst, FILE *f)
{
	struct si_arg_t *arg;
	int index;
	
	/* Dump instruction opcode */
	fprintf(f, "Instruction %s\n", inst->info->name);

	/* Dump arguments */
	LIST_FOR_EACH(inst->arg_list, index)
	{
		arg = list_get(inst->arg_list, index);
		fprintf(f, "\targ %d: ", index);
		si_arg_dump(arg, f);
	}
}

#define CLEAR_BITS_64(X, HI, LO) \
	((unsigned long long) (X) & (((1ull << (LO)) - 1) \
	| ~(((HI) < 63 ? 1ull << ((HI) + 1) : 0ull) - 1)))

#define TRUNCATE_BITS_64(X, NUM) \
	((unsigned long long) (X) & ((1ull << (NUM)) - 1))

#define SET_BITS_64(X, HI, LO, V) \
	(CLEAR_BITS_64((X), (HI), (LO)) | \
	(TRUNCATE_BITS_64((V), (HI) - (LO) + 1) << (LO)))

	

	
int si_dis_inst_code_gen(struct si_dis_inst_t *inst, unsigned long long *inst_bytes)
{
#if 0
	struct si_arg_t *arg;
	switch (inst->opcode)
	{
	/*----------------SOP1 Instructions----------------*/
	case si_dis_inst_opcode_s_mov_b32:

		/* SOP1 op-code = 101111101 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 31, 23, 381);

		arg = list_get(inst->arg_list, 0);
		/* If m0 is register, set to 124 */
		if (arg->type == si_arg_mtype_register)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 22, 16, 124);
		}
		/* other cases here TBD else if () */
		
		/* smov_b32 op-code = 3 */ 	
		*inst_bytes = SET_BITS_64(*inst_bytes, 15, 8, 3);
		
		arg = list_get(inst->arg_list, 1);
		/* If second arg is a constant, set to 255 */
		if (arg->type == si_arg_literal)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 7, 0, 255);
			*inst_bytes = SET_BITS_64(*inst_bytes, 63, 32, arg->value.literal.val);
		}
		
		si_arg_free(arg);
		
		/* 64-bit */ 
		return 8;
		break;
	/*--------------END SOP1 Instructions--------------*/
	/*----------------SMRD Instructions----------------*/
	case si_dis_inst_opcode_s_buffer_load_dword:
		
		/* SMRD op-code = 11000 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 31, 27, 24);
		
		/* s_buffer_load_dword = 8 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 26, 22, 8);
		
		arg = list_get(inst->arg_list, 0);
		/* SDST */ 
		if (arg->type == si_arg_scalar_register)
		{	
			*inst_bytes = SET_BITS_64(*inst_bytes, 21, 15, arg->value.scalar_register.id);
		} /* else other cases */
		
		arg = list_get(inst->arg_list, 1);
		/* SBASE */
		if (arg->type == si_arg_register_range)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 14, 9, ((arg->value.register_range.id_low & arg->value.register_range.id_high)>>1));
		}
		
		/* Last arg is offset*/
		arg = list_get(inst->arg_list, 2);
		if (arg->type == si_arg_literal)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 8, 8, 1);
			*inst_bytes = SET_BITS_64(*inst_bytes, 7, 0, arg->value.literal.val);
		}
		
		si_arg_free(arg);
		
		/* 32-bit */ 
		return 4;
		break;
		
	case si_dis_inst_opcode_s_buffer_load_dwordx2:
		
		/* SMRD op-code = 11000 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 31, 27, 24);
		
		/* s_buffer_load_dwordx2 op-code = 9 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 26, 22, 9);
		
		/* SDST */ 
		arg = list_get(inst->arg_list, 0);
		if (arg->type == si_arg_scalar_register)
		{	
			*inst_bytes = SET_BITS_64(*inst_bytes, 21, 15, arg->value.scalar_register.id);
		} else if ( arg->type == si_arg_register_range)
		{
			/* SGPR 96 = Range[14:15] ?? */
			*inst_bytes = SET_BITS_64(*inst_bytes, 21, 15, 96);
		}
		
		/* SBASE */
		arg = list_get(inst->arg_list, 1);
		if (arg->type == si_arg_register_range)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 14, 9, ((arg->value.register_range.id_low & arg->value.register_range.id_high)>>1));
		}
		
		/* SBASE offset */
		arg = list_get(inst->arg_list, 2);
		if (arg->type == si_arg_literal)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 8, 8, 1);
			*inst_bytes = SET_BITS_64(*inst_bytes, 7, 0, arg->value.literal.val);
		}
		
		si_arg_free(arg);
		
		/* 32-bit */ 
		return 4;
		break;
	
	/*--------------END SMRD Instructions--------------*/
	/*----------------SOPP Instructions----------------*/
	case si_dis_inst_opcode_s_waitcnt:
		
		/* SOPP Encoding = 101111111 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 31, 23, 383);
		
		/* s_waitcnt op-code = 12 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 22, 16, 12);
		
		
		arg = list_get(inst->arg_list, 0);
		if (arg->type == si_arg_waitcnt)
		{
			if (arg->value.wait_cnt.lgkmcnt_active)
			{
				/* unused */
				*inst_bytes = SET_BITS_64(*inst_bytes, 15, 13, 0);
				/* unused */
				*inst_bytes = SET_BITS_64(*inst_bytes, 7, 7, 0);
				/* lgkmcnt input */
				*inst_bytes = SET_BITS_64(*inst_bytes, 12, 8, 0);
				/* vm count ! how is this found? ! */
				*inst_bytes = SET_BITS_64(*inst_bytes, 3, 0, 15);
				/* export/mem-write-data count */
				*inst_bytes = SET_BITS_64(*inst_bytes, 6, 4, 7);
			}
				
		}
		
		si_arg_free(arg);
		
		/* 32-bit */ 
		return 4;
	/*--------------END SOPP Instructions--------------*/
	/*----------------VOP1 Instructions----------------*/
	case si_dis_inst_opcode_v_cvt_f32_u32:
		
		/* VOP1 Encoding = 0111111 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 31, 25, 63);
		
		/* v_cvt_f32_u32 op-code =  6 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 16, 9, 6);
		
		/* Destinsation VGPR (Vector General-Purpose Register) */
		arg = list_get(inst->arg_list, 0);
		if (arg->type == si_arg_vector_register)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 24, 17, arg->value.vector_register.id);
		}
		
		/* Source */
		arg = list_get(inst->arg_list, 1);
		if (arg->type == si_arg_scalar_register)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 8, 0, arg->value.scalar_register.id);
		}
		
		si_arg_free(arg);
		
		/* 32-bit */ 
		return 4;
		break;
		
	case si_dis_inst_opcode_v_rcp_f32:
		
		/* VOP1 Encoding = 0111111 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 31, 25, 63);
		
		/* v_rcp_f32 op-code =  42 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 16, 9, 42);
		
		/* Destinsation VGPR (Vector General-Purpose Register) */
		arg = list_get(inst->arg_list, 0);
		if (arg->type == si_arg_vector_register)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 24, 17, arg->value.vector_register.id);
		}
		
		/* Source */
		arg = list_get(inst->arg_list, 1);
		if (arg->type == si_arg_vector_register)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 8, 0, arg->value.vector_register.id + 256);
		}
		si_arg_free(arg);
		
		/* 32-bit */ 
		return 4;
		break;
	/*--------------END VOP1 Instructions--------------*/
	/*----------------VOP2 Instructions---------------*/
	case si_dis_inst_opcode_v_mul_f32:
	
		arg = list_get(inst->arg_list, 1);
		if (arg->type == si_arg_literal)
		{
			/* Literal constant = 255 */
			*inst_bytes = SET_BITS_64(*inst_bytes, 8, 0, 255);
			
			/* Literal Value */
			*inst_bytes = SET_BITS_64(*inst_bytes, 63, 32, arg->value.literal.val);
			
			/* Second src for instruction */
			arg = list_get(inst->arg_list, 2);
			if (arg->type == si_arg_vector_register)
			{
				*inst_bytes = SET_BITS_64(*inst_bytes, 16, 9, arg->value.vector_register.id);
			}
			
			arg = list_get(inst->arg_list, 0);
			/* Destinsation VGPR (Vector General-Purpose Register) */
			if (arg->type == si_arg_vector_register)
			{
				*inst_bytes = SET_BITS_64(*inst_bytes, 24, 17, arg->value.vector_register.id);
			}
			
			/* v_mul_f32 op-code = 8 */
			*inst_bytes = SET_BITS_64(*inst_bytes, 30, 25, 8);
		
			/* Must be 0 */
			*inst_bytes = SET_BITS_64(*inst_bytes, 31, 31, 0);
			
			si_arg_free(arg);
		
			/* 64-bit */ 
			return 8;
		} else {
			return 0;
		}
		break;
	/*--------------END VOP3a Instructions-------------*/
	case si_dis_inst_opcode_s_cbranch_vccz:
	
		/* To Be Finished */
	
		/* SOPP op-code = 101111111 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 31, 23, 383);
		
		//arg = list_get(inst->arg_list, 0);
		
		/* s_cbranch_vccz op-code = 6 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 22, 16, 6);
		
		/* The arg should always be a label which 
		   will be */
		return 0;
		break;
	default:
		return 0;
		break;
		
	};
#endif
	return 0;
	
}


