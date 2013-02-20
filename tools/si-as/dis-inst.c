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
#include "dis-inst-info.h"
#include "main.h"
#include "token.h"


struct si_dis_inst_t *si_dis_inst_create(char *name, struct list_t *arg_list)
{
	struct si_dis_inst_t *inst;
	struct si_dis_inst_info_t *info;

	struct si_arg_t *arg;
	struct si_token_t *token;

	char err_str[MAX_STRING_SIZE];
	int index;
	
	/* Allocate */
	inst = xcalloc(1, sizeof(struct si_dis_inst_t));
	
	/* Initialize */
	if (!arg_list)
		arg_list = list_create();
	inst->arg_list = arg_list;
	
	/* Try to create the instruction following all possible encodings for
	 * the same instruction name. */
	snprintf(err_str, sizeof err_str, "invalid instruction: %s", name);
	for (info = hash_table_get(si_dis_inst_info_table, name);
			info; info = info->next)
	{
		/* Check number of arguments */
		if (arg_list->count != info->token_list->count)
		{
			snprintf(err_str, sizeof err_str,
				"invalid number of arguments for %s (%d given, %d expected)",
				name, arg_list->count, info->token_list->count - 1);
			continue;
		}

		/* Check arguments */
		err_str[0] = '\0';
		LIST_FOR_EACH(arg_list, index)
		{
			/* Get actual argument */
			arg = list_get(arg_list, index);

			/* Get formal argument from instruction info */
			token = list_get(info->token_list, index);
			assert(token);

			/* Check that actual argument type is acceptable for token */
			if (!si_token_is_arg_allowed(token, arg))
			{
				snprintf(err_str, sizeof err_str,
					"invalid type for argument %d", index + 1);
				break;
			}
		}

		/* Error while processing arguments */
		if (err_str[0])
			continue;
	
		/* All checks passed, instruction identified correctly as that
		 * represented by 'info'. */
		break;
	}

	/* Error identifying instruction */
	if (!info)
		yyerror(err_str);

	/* Initialize opcode */
	inst->info = info;
	inst->opcode = info->inst_info->opcode;

	/* Return */
	return inst;
}


void si_dis_inst_free(struct si_dis_inst_t *inst)
{
	int index;
	struct si_arg_t *arg;
	
	/* Free all argument object in the argument list */
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
	unsigned int word;
	
	int i;
	int j;
	
	/* Dump instruction opcode */
	fprintf(f, "Instruction %s\n", inst->info->name);

	/* Dump arguments */
	LIST_FOR_EACH(inst->arg_list, i)
	{
		arg = list_get(inst->arg_list, i);
		fprintf(f, "\targ %d:\n", i);
		si_arg_dump(arg, f);
	}

	/* Empty instruction bits */
	if (!inst->size)
		return;

	/* Print words */
	for (i = 0; i < inst->size / 4; i++)
	{
		word = * (int *) inst->inst_bytes.bytes;
		printf("\tword %d:  hex = { %08x },  bin = {", i, word);
		for (j = 0; j < 32; j++)
			printf("%s%d", j % 4 ? "" : " ", (word >> (31 - j)) & 1);
		printf(" }\n");
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

	

	
void si_dis_inst_gen(struct si_dis_inst_t *inst)
{
	union si_inst_microcode_t *inst_bytes;
	struct si_inst_info_t *inst_info;
	struct si_dis_inst_info_t *info;

	struct si_arg_t *arg;
	struct si_token_t *token;

	int index;

	/* Initialize */
	inst_bytes = &inst->inst_bytes;
	info = inst->info;
	assert(info);
	inst_info = info->inst_info;

	/* By default, the instruction has 4 bytes. It will be extended upon
	 * the presence of a literal constant in its arguments. */
	inst->size = 4;

	/* Instruction opcode */
	switch (inst_info->fmt)
	{

	/* Opcode in [26:22], encoding in [31:27] */
	case SI_FMT_SMRD:
		inst_bytes->smrd.enc = 0x18;
		inst_bytes->smrd.op = inst_info->opcode;
		break;

	default:
		fatal("%s: unsupported format", __FUNCTION__);
	}

	/* Arguments */
	assert(inst->arg_list->count == info->token_list->count);
	LIST_FOR_EACH(inst->arg_list, index)
	{
		/* Get argument */
		arg = list_get(inst->arg_list, index);
		token = list_get(info->token_list, index);
		assert(arg);
		assert(token);

		/* Check token */
		switch (token->type)
		{
		
		case si_token_offset:

			/* Depends of argument type */
			switch (arg->type)
			{

			case si_arg_literal:

				inst_bytes->smrd.imm = 1;
				inst_bytes->smrd.offset = arg->value.literal.val;
				/* FIXME - check valid range of literal */
				break;
			
			case si_arg_scalar_register:

				inst_bytes->smrd.offset = arg->value.scalar_register.id;
				break;

			default:
				panic("%s: invalid argument type for 'offset' token",
					__FUNCTION__);
			}
			break;

		case si_token_series_sbase:

			/* Check that low register is multiple of 2 */
			if (arg->value.scalar_register_series.low % 2)
				yyerror_fmt("base register must be multiple of 2");

			/* Restrictions for high register */
			switch (inst_info->inst)
			{
			case SI_INST_S_BUFFER_LOAD_DWORD:

				/* High register must be low plus 3 */
				if (arg->value.scalar_register_series.high !=
						arg->value.scalar_register_series.low + 3)
					yyerror("register series must be s[low:low+3]");
				break;
			default:
				fatal("%s: unsupported opcode for 'series_sbase' token: %d",
					__FUNCTION__, inst_info->inst);
			}

			/* Encode */
			inst_bytes->smrd.sbase = arg->value.scalar_register_series.low / 2;
			break;

		case si_token_smrd_sdst:
			inst_bytes->smrd.sdst = arg->value.scalar_register.id;
			break;

		default:
			fatal("%s: unsupported token", __FUNCTION__);
		}
	}
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
}

