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
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "arg.h"
#include "dis-inst.h"
#include "dis-inst-info.h"
#include "main.h"
#include "stream.h"
#include "symbol.h"
#include "task.h"
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
	fprintf(f, "\tformat=%s, size=%d\n",
			str_map_value(&si_inst_fmt_map, inst->info->inst_info->fmt),
			inst->size);

	/* Dump arguments */
	LIST_FOR_EACH(inst->arg_list, i)
	{
		arg = list_get(inst->arg_list, i);
		fprintf(f, "\targ %d: ", i);
		si_arg_dump(arg, f);
		fprintf(f, "\n");
	}

	/* Empty instruction bits */
	if (!inst->size)
		return;

	/* Print words */
	for (i = 0; i < inst->size / 4; i++)
	{
		word = * (int *) &inst->inst_bytes.bytes[i * 4];
		printf("\tword %d:  hex = { %08x },  bin = {", i, word);
		for (j = 0; j < 32; j++)
			printf("%s%d", j % 4 ? "" : " ", (word >> (31 - j)) & 1);
		printf(" }\n");
	}
}


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

	/* By default, the instruction has the number of bytes specified by its
	 * format. 4-bit instructions could be extended later to 8 bits upon
	 * the presence of a literal constant. */
	inst->size = inst_info->size;

	/* Instruction opcode */
	switch (inst_info->fmt)
	{

	/* encoding in [31:26], op in [18:16] */
	case SI_FMT_MTBUF:

		inst_bytes->mtbuf.enc = 0x3a;
		inst_bytes->mtbuf.op = inst_info->opcode;
		break;
	
	/* encoding in [:], op in [] */
	case SI_FMT_MUBUF:
		
		inst_bytes->mubuf.enc = 0x38;
		inst_bytes->mubuf.op = inst_info->opcode;
		break;

	/* encoding in [:], op in [] */
	case SI_FMT_MIMG:
		
		inst_bytes->mimg.enc = 0x3c;
		inst_bytes->mimg.op = inst_info->opcode;
		break;

	/* encoding in [31:27], op in [26:22] */
	case SI_FMT_SMRD:

		inst_bytes->smrd.enc = 0x18;
		inst_bytes->smrd.op = inst_info->opcode;
		break;
	
	/* encoding in [:], op in [] */
	case SI_FMT_DS:
		
		inst_bytes->ds.enc = 0x36;
		inst_bytes->ds.op = inst_info->opcode;
		break;

	/* encoding in [31:23], op in [22:16] */
	case SI_FMT_SOPC:
		
		inst_bytes->sopc.enc = 0x17e;
		inst_bytes->sopc.op = inst_info->opcode;
		break;

	/* encoding in [31:23], op in [15:8] */
	case SI_FMT_SOP1:

		inst_bytes->sop1.enc = 0x17d;
		inst_bytes->sop1.op = inst_info->opcode;
		break;

	/* encoding in [31:30], op in [29:23] */
	case SI_FMT_SOP2:

		inst_bytes->sop2.enc = 0x2;
		inst_bytes->sop2.op = inst_info->opcode;
		break;

	/* encoding in [31:23], op in [22:16] */
	case SI_FMT_SOPP:

		inst_bytes->sopp.enc = 0x17f;
		inst_bytes->sopp.op = inst_info->opcode;
		break;
	
	/* encoding in [:], op in [] */
	case SI_FMT_SOPK:
		
		inst_bytes->sopk.enc = 0xb;
		inst_bytes->sopk.op = inst_info->opcode;
		break;

	/* encoding in [:], op in [] */
	case SI_FMT_VOPC:
		
		inst_bytes->vopc.enc = 0x3e;
		inst_bytes->vopc.op = inst_info->opcode;
		break;

	/* encoding in [31:25], op in [16:9] */
	case SI_FMT_VOP1:

		inst_bytes->vop1.enc = 0x3f;
		inst_bytes->vop1.op = inst_info->opcode;
		break;

	/* encoding in [31], op in [30:25] */
	case SI_FMT_VOP2:

		inst_bytes->vop2.enc = 0x0;
		inst_bytes->vop2.op = inst_info->opcode;
		break;

	/* encoding in [31:26], op in [25:17] */
	case SI_FMT_VOP3a:

		inst_bytes->vop3a.enc = 0x34;
		inst_bytes->vop3a.op = inst_info->opcode;
		break;

	/* encoding in [:], op in [] */
	case SI_FMT_VINTRP:
		
		inst_bytes->vintrp.enc = 0x32;
		inst_bytes->vintrp.op = inst_info->opcode;
		break;

	/* encoding in [:], op in [] */
	case SI_FMT_EXP:
		
		inst_bytes->exp.enc = 0x3e;
		/* No opcode: only 1 instruction */
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
		
		case si_token_simm16:
		{
			inst_bytes->sopk.simm16 = si_arg_encode_operand(arg);
			break;
		}
		
		case si_token_64_sdst:
		{
			int low;
			int high;

			/* Check range if scalar register range given */
			if (arg->type == si_arg_scalar_register_series)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					yyerror("register series must be s[x:x+1]");
			}
			
			/* Encode */
			inst_bytes->sop2.sdst = si_arg_encode_operand(arg);
			break;
		}

		case si_token_64_ssrc0:
		{
			int value;

			if (arg->type == si_arg_literal && !IN_RANGE(arg->value.literal.val, -16, 64))
			{
				/* Literal constant other than [-16...64] is encoded by adding
				 * four more bits to the instruction. */
				if (inst->size == 8)
					yyerror("only one literal allowed");
				inst->size = 8;
				inst_bytes->sop2.ssrc0 = 0xff;
				inst_bytes->sop2.lit_cnst = arg->value.literal.val;
			}
			else
			{
				/* Check range of scalar registers */
				if (arg->type == si_arg_scalar_register_series &&
						arg->value.scalar_register_series.high !=
						arg->value.scalar_register_series.low + 1)
					yyerror("invalid scalar register series, s[x:x+1] expected");

				/* Encode */
				value = si_arg_encode_operand(arg);
				if (!IN_RANGE(value, 0, 255))
					yyerror("invalid argument type");
				inst_bytes->sop2.ssrc0 = value;
			}
			break;
		}

		case si_token_64_ssrc1:
		{
			int value;

			if (arg->type == si_arg_literal && !IN_RANGE(arg->value.literal.val, -16, 64))
			{
				/* Literal constant other than [-16...64] is encoded by adding
				 * four more bits to the instruction. */
				if (inst->size == 8)
					yyerror("only one literal allowed");
				inst->size = 8;
				inst_bytes->sop2.ssrc1 = 0xff;
				inst_bytes->sop2.lit_cnst = arg->value.literal.val;
			}
			else
			{
				/* Check range of scalar registers */
				if (arg->type == si_arg_scalar_register_series &&
						arg->value.scalar_register_series.high !=
						arg->value.scalar_register_series.low + 1)
					yyerror("invalid scalar register series, s[x:x+1] expected");

				/* Encode */
				value = si_arg_encode_operand(arg);
				if (!IN_RANGE(value, 0, 255))
					yyerror("invalid argument type");
				inst_bytes->sop2.ssrc1 = value;
			}
			break;
		}
		
		case si_token_mt_maddr:
		{
			struct si_arg_t *qual;

			int err;
			int soffset;

			/* Offset */
			soffset = si_arg_encode_operand(arg->value.maddr.soffset);
			if (!IN_RANGE(soffset, 0, 253))
				yyerror("invalid offset");
			inst_bytes->mtbuf.soffset = soffset;

			/* Data format */
			inst_bytes->mtbuf.dfmt = str_map_string_err(&si_inst_dfmt_map,
					arg->value.maddr.data_format, &err);
			if (err)
				yyerror_fmt("invalid data format: %s", arg->value.maddr.data_format);

			/* Number format */
			inst_bytes->mtbuf.nfmt = str_map_string_err(&si_inst_nfmt_map,
					arg->value.maddr.num_format, &err);
			if (err)
				yyerror_fmt("invalid number format: %s", arg->value.maddr.num_format);

			/* Qualifiers */
			qual = arg->value.maddr.qual;
			assert(qual->type == si_arg_maddr_qual);
			inst_bytes->mtbuf.offen = qual->value.maddr_qual.offen;
			inst_bytes->mtbuf.idxen = qual->value.maddr_qual.idxen;
			inst_bytes->mtbuf.offset = qual->value.maddr_qual.offset;

			break;
		}

		case si_token_label:
		{
			struct si_symbol_t *label;
			struct si_task_t *task;

			assert(arg->type == si_arg_label);
			label = arg->value.label.symbol;
			if (label->defined)
			{
				inst_bytes->sopp.simm16 = (label->value -
						si_out_stream->offset) / 4 - 1;
			}
			else
			{
				/* We create a task to complete this instruction once the
				 * label is defined. */
				task = si_task_create(si_out_stream->offset, label);
				list_add(si_task_list, task);
			}
			break;
		}

		case si_token_mt_series_vdata:
		{
			int low = 0;
			int high = 0;
			int high_must = 0;

			/* Get registers */
			switch (arg->type)
			{

			case si_arg_vector_register:

				low = arg->value.vector_register.id;
				high = low;
				break;

			case si_arg_vector_register_series:

				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				break;

			default:
				panic("%s: invalid argument type for token 'mt_series_vdata'",
						__FUNCTION__);
			}

			/* Restriction in vector register range */
			switch (inst_info->inst)
			{

			case SI_INST_TBUFFER_LOAD_FORMAT_X:
			case SI_INST_TBUFFER_STORE_FORMAT_X:

				high_must = low;
				break;

			case SI_INST_TBUFFER_LOAD_FORMAT_XY:
			case SI_INST_TBUFFER_STORE_FORMAT_XY:

				high_must = low + 1;
				break;

			case SI_INST_TBUFFER_LOAD_FORMAT_XYZW:
			case SI_INST_TBUFFER_STORE_FORMAT_XYZW:

				high_must = low + 3;
				break;

			default:
				fatal("%s: MUBUF/MTBUF instruction not recognized: %s",
						__FUNCTION__, info->name);
			}

			/* Check range */
			if (high != high_must)
				yyerror_fmt("invalid register series: v[%d:%d]", low, high);

			/* Encode */
			inst_bytes->mtbuf.vdata = low;
			break;
		}

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
				panic("%s: invalid argument type for token 'offset'",
					__FUNCTION__);
			}
			break;

		case si_token_sdst:

			/* Encode */
			inst_bytes->sop2.sdst = arg->value.scalar_register.id;
			break;

		case si_token_series_sbase:

			/* Check that low register is multiple of 2 */
			if (arg->value.scalar_register_series.low % 2)
				yyerror("base register must be multiple of 2");

			/* Restrictions for high register */
			switch (inst_info->inst)
			{

			case SI_INST_S_LOAD_DWORDX2:
			case SI_INST_S_LOAD_DWORDX4:

				/* High register must be low plus 1 */
				if (arg->value.scalar_register_series.high !=
						arg->value.scalar_register_series.low + 1)
					yyerror("register series must be s[x:x+1]");
				break;

			case SI_INST_S_BUFFER_LOAD_DWORD:
			case SI_INST_S_BUFFER_LOAD_DWORDX2:

				/* High register must be low plus 3 */
				if (arg->value.scalar_register_series.high !=
						arg->value.scalar_register_series.low + 3)
					yyerror("register series must be s[x:x+3]");
				break;

			default:
				fatal("%s: unsupported opcode for 'series_sbase' token: %s",
						__FUNCTION__, info->name);
			}

			/* Encode */
			inst_bytes->smrd.sbase = arg->value.scalar_register_series.low / 2;
			break;

		case si_token_series_sdst:

			/* Restrictions for high register */
			switch (inst_info->inst)
			{

			case SI_INST_S_LOAD_DWORDX2:
			case SI_INST_S_BUFFER_LOAD_DWORDX2:

				/* High register must be low plus 1 */
				if (arg->value.scalar_register_series.high !=
						arg->value.scalar_register_series.low + 1)
					yyerror("register series must be s[low:low+1]");
				break;

			case SI_INST_S_LOAD_DWORDX4:

				/* High register must be low plus 3 */
				if (arg->value.scalar_register_series.high !=
						arg->value.scalar_register_series.low + 3)
					yyerror("register series must be s[low:low+3]");
				break;

			default:
				fatal("%s: unsupported opcode for 'series_sdst' token: %s",
						__FUNCTION__, info->name);
			}

			/* Encode */
			inst_bytes->smrd.sdst = arg->value.scalar_register_series.low;
			break;

		case si_token_series_srsrc:
		{
			int low = arg->value.scalar_register_series.low;
			int high = arg->value.scalar_register_series.high;

			/* Base register must be multiple of 4 */
			if (low % 4)
				yyerror_fmt("low register must be multiple of 4 in s[%d:%d]",
						low, high);

			/* High register must be low + 3 */
			if (high != low + 3)
				yyerror_fmt("register series must span 4 registers in s[%d:%d]",
						low, high);

			/* Encode */
			inst_bytes->mtbuf.srsrc = low >> 2;
			break;
		}

		case si_token_smrd_sdst:

			/* Encode */
			inst_bytes->smrd.sdst = arg->value.scalar_register.id;
			break;

		case si_token_src0:

			if (arg->type == si_arg_literal && !IN_RANGE(arg->value.literal.val, -16, 64))
			{
				/* Literal constant other than [-16...64] is encoded by adding
				 * four more bits to the instruction. */
				if (inst->size == 8)
					yyerror("only one literal allowed");
				inst->size = 8;
				inst_bytes->vopc.src0 = 0xff;
				inst_bytes->vopc.lit_cnst = arg->value.literal.val;
			}
			else
			{
				inst_bytes->vopc.src0 = si_arg_encode_operand(arg);
			}
			break;

		case si_token_ssrc0:
		{
			int value;

			if (arg->type == si_arg_literal && !IN_RANGE(arg->value.literal.val, -16, 64))
			{
				/* Literal constant other than [-16...64] is encoded by adding
				 * four more bits to the instruction. */
				if (inst->size == 8)
					yyerror("only one literal allowed");
				inst->size = 8;
				inst_bytes->sop2.ssrc0 = 0xff;
				inst_bytes->sop2.lit_cnst = arg->value.literal.val;
			}
			else
			{
				value = si_arg_encode_operand(arg);
				if (!IN_RANGE(value, 0, 255))
					yyerror("invalid argument type");
				inst_bytes->sop2.ssrc0 = value;
			}
			break;
		}

		case si_token_ssrc1:
		{
			int value;

			if (arg->type == si_arg_literal && !IN_RANGE(arg->value.literal.val, -16, 64))
			{
				/* Literal constant other than [-16...64] is encoded by adding
				 * four more bits to the instruction. */
				if (inst->size == 8)
					yyerror("only one literal allowed");
				inst->size = 8;
				inst_bytes->sop2.ssrc1 = 0xff;
				inst_bytes->sop2.lit_cnst = arg->value.literal.val;
			}
			else
			{
				value = si_arg_encode_operand(arg);
				if (!IN_RANGE(value, 0, 255))
					yyerror("invalid argument type");
				inst_bytes->sop2.ssrc1 = value;
			}
			break;
		}

		case si_token_vaddr:

			switch (arg->type)
			{

			case si_arg_vector_register:

				inst_bytes->mtbuf.vaddr = arg->value.vector_register.id;
				break;

			default:
				panic("%s: invalid argument type for token 'ssrc0'",
					__FUNCTION__);
			}
			break;

		case si_token_vcc:

			/* Not encoded */
			break;

		case si_token_vdst:

			/* Encode */
			inst_bytes->vop1.vdst = arg->value.vector_register.id;
			break;

		case si_token_vop3_64_svdst:
		{
			int low;
			int high;

			/* If operand is a scalar register series, check range */
			if (arg->type == si_arg_scalar_register_series)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					yyerror("register series must be s[low:low+1]");
			}

			/* Encode */
			inst_bytes->vop3a.vdst = si_arg_encode_operand(arg);
			break;
		}

		case si_token_vop3_src0:

			inst_bytes->vop3a.src0 = si_arg_encode_operand(arg);
			if (arg->abs)
				inst_bytes->vop3a.abs |= 0x1;
			if (arg->neg)
				inst_bytes->vop3a.neg |= 0x1;
			break;

		case si_token_vop3_src1:

			inst_bytes->vop3a.src1 = si_arg_encode_operand(arg);
			if (arg->abs)
				inst_bytes->vop3a.abs |= 0x2;
			if (arg->neg)
				inst_bytes->vop3a.neg |= 0x2;
			break;

		case si_token_vop3_src2:

			inst_bytes->vop3a.src2 = si_arg_encode_operand(arg);
			if (arg->abs)
				inst_bytes->vop3a.abs |= 0x4;
			if (arg->neg)
				inst_bytes->vop3a.neg |= 0x4;
			break;

		case si_token_vop3_vdst:
			
			inst_bytes->vop3a.vdst = arg->value.vector_register.id;
			break;

		case si_token_vsrc1:

			/* Encode */
			assert(arg->type == si_arg_vector_register);
			inst_bytes->vopc.vsrc1 = si_arg_encode_operand(arg);
			break;

		case si_token_wait_cnt:
			/* vmcnt(x) */
			if (arg->value.wait_cnt.vmcnt_active)
			{
				if (!IN_RANGE(arg->value.wait_cnt.vmcnt_value, 0, 0xe))
					yyerror("invalid value for vmcnt");
				inst_bytes->sopp.simm16 = SET_BITS_32(inst_bytes->sopp.simm16,
						3, 0, arg->value.wait_cnt.vmcnt_value);
			}
			else
			{
				inst_bytes->sopp.simm16 = SET_BITS_32(inst_bytes->sopp.simm16,
						3, 0, 0xf);
			}

			/* lgkmcnt(x) */
			if (arg->value.wait_cnt.lgkmcnt_active)
			{
				if (!IN_RANGE(arg->value.wait_cnt.lgkmcnt_value, 0, 0x1e))
					yyerror("invalid value for lgkmcnt");
				inst_bytes->sopp.simm16 = SET_BITS_32(inst_bytes->sopp.simm16,
						12, 8, arg->value.wait_cnt.lgkmcnt_value);
			}
			else
			{
				inst_bytes->sopp.simm16 = SET_BITS_32(inst_bytes->sopp.simm16,
						12, 8, 0x1f);
			}

			/* expcnt(x) */
			if (arg->value.wait_cnt.expcnt_active)
			{
				if (!IN_RANGE(arg->value.wait_cnt.expcnt_value, 0, 0x6))
					yyerror("invalid value for expcnt");
				inst_bytes->sopp.simm16 = SET_BITS_32(inst_bytes->sopp.simm16,
						6, 4, arg->value.wait_cnt.expcnt_value);
			}
			else
			{
				inst_bytes->sopp.simm16 = SET_BITS_32(inst_bytes->sopp.simm16,
						6, 4, 0x7);
			}
			break;

		default:
			yyerror_fmt("unsupported token for argument %d",
				index + 1);
		}
	}
}

