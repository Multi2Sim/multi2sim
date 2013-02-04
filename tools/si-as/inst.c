#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "inst.h"
#include "inst-arg.h"

struct str_map_t si_inst_opcode_map =
{
	39,
	{
		{ "s_mov_b32", si_inst_opcode_s_mov_b32 },
		{ "s_waitcnt", si_inst_opcode_s_waitcnt },
		{ "v_cvt_f32_u32", si_inst_opcode_v_cvt_f32_u32 },
		{ "v_rcp_f32", si_inst_opcode_v_rcp_f32 },
		{ "v_mul_f32", si_inst_opcode_v_mul_f32 },
		{ "s_buffer_load_dwordx2", si_inst_opcode_s_buffer_load_dwordx2 },
		{ "v_cvt_u32_f32", si_inst_opcode_v_cvt_u32_f32 },
		{ "v_mul_lo_u32", si_inst_opcode_v_mul_lo_u32 },
		{ "v_mul_hi_u32", si_inst_opcode_v_mul_hi_u32 },
		{ "s_buffer_load_dword", si_inst_opcode_s_buffer_load_dword },
		{ "v_sub_i32", si_inst_opcode_v_sub_i32 },
		{ "v_cmp_ne_i32", si_inst_opcode_v_cmp_ne_i32 },
		{ "v_cndmask_b32", si_inst_opcode_v_cndmask_b32 },
		{ "s_min_u32", si_inst_opcode_s_min_u32 },
		{ "v_sub_i32", si_inst_opcode_v_sub_i32 },
		{ "v_add_i32", si_inst_opcode_v_add_i32 },
		{ "v_mov_b32", si_inst_opcode_v_mov_b32 },
		{ "v_mul_i32_i24", si_inst_opcode_v_mul_i32_i24 },
		{ "s_load_dwordx4", si_inst_opcode_s_load_dwordx4 },
		{ "v_mul_lo_i32", si_inst_opcode_v_mul_lo_i32 },
		{ "v_cmp_ge_u32", si_inst_opcode_v_cmp_ge_u32 },
		{ "s_and_b64", si_inst_opcode_s_and_b64 },
		{ "v_lshlrev_b32", si_inst_opcode_v_lshlrev_b32 },
		{ "v_addc_u32", si_inst_opcode_v_addc_u32 },
		{ "tbuffer_store_format_x", si_inst_opcode_tbuffer_store_format_x },
		{ "s_cbranch_vccz", si_inst_opcode_s_cbranch_vccz },
		{ "s_mul_i32", si_inst_opcode_s_mul_i32 },
		{ "s_lshl_b32", si_inst_opcode_s_lshl_b32 },
		{ "v_readfirstlane_b32", si_inst_opcode_v_readfirstlane_b32 },
		{ "tbuffer_load_format_x", si_inst_opcode_tbuffer_load_format_x },
		{ "ds_write_b32", si_inst_opcode_ds_write_b32 },
		{ "s_barrier", si_inst_opcode_s_barrier },
		{ "s_cmp_eq_i32", si_inst_opcode_s_cmp_eq_i32 },
		{ "s_cbranch_scc1", si_inst_opcode_s_cbranch_scc1 },
		{ "ds_read_b32", si_inst_opcode_ds_read_b32 },
		{ "s_add_i32", si_inst_opcode_s_add_i32 },
		{ "v_mac_f32", si_inst_opcode_v_mac_f32 },
		{ "s_branch", si_inst_opcode_s_branch },
		{ "s_endpgm", si_inst_opcode_s_endpgm }
	}
};


struct si_inst_t *si_inst_create(char *inst_str)
{
	struct si_inst_t *inst;
	
	/* Allocate */
	inst = calloc(1, sizeof(struct si_inst_t));
	if (!inst)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Initialize */
	inst->arg_list = list_create();
	
	/* Look up our instruction string
	 * int the string map and find the
	 * the corresponding enumeration. Then,
	 * set si_inst_t's opcode value to the
	 * correct enumeration.				   */
	inst->opcode = str_map_string(&si_inst_opcode_map, inst_str);
	
	/* Return */
	return inst;
}


void si_inst_free(struct si_inst_t *inst)
{
	int index;
	
	/* Free all argument object in the argument list */
	for (index = 0; index < inst->arg_list->count; index++)
		si_inst_arg_free(list_get(inst->arg_list, index));
	
	/* Free argument list and instruction object */
	list_free(inst->arg_list);
	free(inst);
}


void si_inst_dump(struct si_inst_t *inst, FILE *f)
{
	int index;
	
	/* Dump instruction opcode */
	fprintf(f, "Instruction op-code %d: %s\n", inst->opcode,
		str_map_value(&si_inst_opcode_map, inst->opcode));

	/* Dump arguments */
	for (index = 0; index < (inst->arg_list->count); index++)
	{
		fprintf(f, "Argument %d:\n", index);
		si_inst_arg_dump(list_get(inst->arg_list, index), stdout);
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

	

	
int si_inst_code_gen(struct si_inst_t *inst, unsigned long long *inst_bytes)
{
	struct si_inst_arg_t *arg;
	switch (inst->opcode)
	{
	/*----------------SOP1 Instructions----------------*/
	case si_inst_opcode_s_mov_b32:

		/* SOP1 op-code = 101111101 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 31, 23, 381);

		arg = list_get(inst->arg_list, 0);
		/* If m0 is register, set to 124 */
		if (arg->type == si_inst_arg_mtype_register)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 22, 16, 124);
		}
		/* other cases here TBD else if () */
		
		/* smov_b32 op-code = 3 */ 	
		*inst_bytes = SET_BITS_64(*inst_bytes, 15, 8, 3);
		
		arg = list_get(inst->arg_list, 1);
		/* If second arg is a constant, set to 255 */
		if (arg->type == si_inst_arg_literal)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 7, 0, 255);
			*inst_bytes = SET_BITS_64(*inst_bytes, 63, 32, arg->value.literal.val);
		}
		
		si_inst_arg_free(arg);
		
		/* 64-bit */ 
		return 8;
		break;
	/*--------------END SOP1 Instructions--------------*/
	/*----------------SMRD Instructions----------------*/
	case si_inst_opcode_s_buffer_load_dword:
		
		/* SMRD op-code = 11000 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 31, 27, 24);
		
		/* s_buffer_load_dword = 8 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 26, 22, 8);
		
		arg = list_get(inst->arg_list, 0);
		/* SDST */ 
		if (arg->type == si_inst_arg_scalar_register)
		{	
			*inst_bytes = SET_BITS_64(*inst_bytes, 21, 15, arg->value.scalar_register.id);
		} /* else other cases */
		
		arg = list_get(inst->arg_list, 1);
		/* SBASE */
		if (arg->type == si_inst_arg_register_range)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 14, 9, ((arg->value.register_range.id_low & arg->value.register_range.id_high)>>1));
		}
		
		/* Last arg is offset*/
		arg = list_get(inst->arg_list, 2);
		if (arg->type == si_inst_arg_literal)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 8, 8, 1);
			*inst_bytes = SET_BITS_64(*inst_bytes, 7, 0, arg->value.literal.val);
		}
		
		si_inst_arg_free(arg);
		
		/* 32-bit */ 
		return 4;
		break;
		
	case si_inst_opcode_s_buffer_load_dwordx2:
		
		/* SMRD op-code = 11000 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 31, 27, 24);
		
		/* s_buffer_load_dwordx2 op-code = 9 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 26, 22, 9);
		
		/* SDST */ 
		arg = list_get(inst->arg_list, 0);
		if (arg->type == si_inst_arg_scalar_register)
		{	
			*inst_bytes = SET_BITS_64(*inst_bytes, 21, 15, arg->value.scalar_register.id);
		} else if ( arg->type == si_inst_arg_register_range)
		{
			/* SGPR 96 = Range[14:15] ?? */
			*inst_bytes = SET_BITS_64(*inst_bytes, 21, 15, 96);
		}
		
		/* SBASE */
		arg = list_get(inst->arg_list, 1);
		if (arg->type == si_inst_arg_register_range)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 14, 9, ((arg->value.register_range.id_low & arg->value.register_range.id_high)>>1));
		}
		
		/* SBASE offset */
		arg = list_get(inst->arg_list, 2);
		if (arg->type == si_inst_arg_literal)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 8, 8, 1);
			*inst_bytes = SET_BITS_64(*inst_bytes, 7, 0, arg->value.literal.val);
		}
		
		si_inst_arg_free(arg);
		
		/* 32-bit */ 
		return 4;
		break;
	
	/*--------------END SMRD Instructions--------------*/
	/*----------------SOPP Instructions----------------*/
	case si_inst_opcode_s_waitcnt:
		
		/* SOPP Encoding = 101111111 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 31, 23, 383);
		
		/* s_waitcnt op-code = 12 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 22, 16, 12);
		
		
		arg = list_get(inst->arg_list, 0);
		if (arg->type == si_inst_arg_waitcnt)
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
		
		si_inst_arg_free(arg);
		
		/* 32-bit */ 
		return 4;
	/*--------------END SOPP Instructions--------------*/
	/*----------------VOP1 Instructions----------------*/
	case si_inst_opcode_v_cvt_f32_u32:
		
		/* VOP1 Encoding = 0111111 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 31, 25, 63);
		
		/* v_cvt_f32_u32 op-code =  6 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 16, 9, 6);
		
		/* Destinsation VGPR (Vector General-Purpose Register) */
		arg = list_get(inst->arg_list, 0);
		if (arg->type == si_inst_arg_vector_register)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 24, 17, arg->value.vector_register.id);
		}
		
		/* Source */
		arg = list_get(inst->arg_list, 1);
		if (arg->type == si_inst_arg_scalar_register)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 8, 0, arg->value.scalar_register.id);
		}
		
		si_inst_arg_free(arg);
		
		/* 32-bit */ 
		return 4;
		break;
		
	case si_inst_opcode_v_rcp_f32:
		
		/* VOP1 Encoding = 0111111 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 31, 25, 63);
		
		/* v_rcp_f32 op-code =  42 */
		*inst_bytes = SET_BITS_64(*inst_bytes, 16, 9, 42);
		
		/* Destinsation VGPR (Vector General-Purpose Register) */
		arg = list_get(inst->arg_list, 0);
		if (arg->type == si_inst_arg_vector_register)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 24, 17, arg->value.vector_register.id);
		}
		
		/* Source */
		arg = list_get(inst->arg_list, 1);
		if (arg->type == si_inst_arg_vector_register)
		{
			*inst_bytes = SET_BITS_64(*inst_bytes, 8, 0, arg->value.vector_register.id + 256);
		}
		si_inst_arg_free(arg);
		
		/* 32-bit */ 
		return 4;
		break;
	/*--------------END VOP1 Instructions--------------*/
	/*----------------VOP2 Instructions---------------*/
	case si_inst_opcode_v_mul_f32:
	
		arg = list_get(inst->arg_list, 1);
		if (arg->type == si_inst_arg_literal)
		{
			/* Literal constant = 255 */
			*inst_bytes = SET_BITS_64(*inst_bytes, 8, 0, 255);
			
			/* Literal Value */
			*inst_bytes = SET_BITS_64(*inst_bytes, 63, 32, arg->value.literal.val);
			
			/* Second src for instruction */
			arg = list_get(inst->arg_list, 2);
			if (arg->type == si_inst_arg_vector_register)
			{
				*inst_bytes = SET_BITS_64(*inst_bytes, 16, 9, arg->value.vector_register.id);
			}
			
			arg = list_get(inst->arg_list, 0);
			/* Destinsation VGPR (Vector General-Purpose Register) */
			if (arg->type == si_inst_arg_vector_register)
			{
				*inst_bytes = SET_BITS_64(*inst_bytes, 24, 17, arg->value.vector_register.id);
			}
			
			/* v_mul_f32 op-code = 8 */
			*inst_bytes = SET_BITS_64(*inst_bytes, 30, 25, 8);
		
			/* Must be 0 */
			*inst_bytes = SET_BITS_64(*inst_bytes, 31, 31, 0);
			
			si_inst_arg_free(arg);
		
			/* 64-bit */ 
			return 8;
		} else {
			return 0;
		}
		break;
	/*--------------END VOP3a Instructions-------------*/
	case si_inst_opcode_s_cbranch_vccz:
	
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
	
	
}


