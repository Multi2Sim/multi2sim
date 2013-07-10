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
#include <lib/util/elf-encode.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "arg.h"
#include "inner-bin.h"
#include "inst.h"
#include "inst-info.h"
#include "si2bin.h"
#include "symbol.h"
#include "task.h"
#include "token.h"


struct si2bin_inst_t *si2bin_inst_create(int opcode, struct list_t *arg_list)
{
	struct si2bin_inst_t *inst;
	struct si2bin_arg_t *arg;
	struct si2bin_token_t *token;
	struct si2bin_inst_info_t *info;
	int index;

	/* Initialize */
	inst = xcalloc(1, sizeof(struct si2bin_inst_t));
	inst->arg_list = arg_list;

	/* Check valid opcode */
	if (!IN_RANGE(opcode, 1, SI_INST_COUNT - 1))
		fatal("%s: invalid opcode (%d)", __FUNCTION__, opcode);

	/* Get instruction information */
	inst->info = list_get(si2bin_inst_info_list, opcode);
	info = inst->info;
	if (!info)
		fatal("%s: opcode %d not supported", __FUNCTION__, opcode);

	/* Check number of arguments */
	if (arg_list->count != info->token_list->count)
		fatal("%s: invalid number of arguments (%d given, %d expected)",
				__FUNCTION__, arg_list->count, info->token_list->count);

	/* Check argument types */
	LIST_FOR_EACH(arg_list, index)
	{
		/* Get actual argument */
		arg = list_get(arg_list, index);

		/* Get formal argument from instruction info */
		token = list_get(info->token_list, index);
		assert(token);

		/* Check that actual argument type is acceptable for token */
		if (!si2bin_token_is_arg_allowed(token, arg))
			fatal("%s: invalid type for argument %d", __FUNCTION__, index);
	}

	/* Return */
	return inst;
}


struct si2bin_inst_t *si2bin_inst_create_with_name(char *name, struct list_t *arg_list)
{
	struct si2bin_inst_t *inst;
	struct si2bin_inst_info_t *info;

	struct si2bin_arg_t *arg;
	struct si2bin_token_t *token;

	char err_str[MAX_STRING_SIZE];
	int index;
	
	/* Allocate */
	inst = xcalloc(1, sizeof(struct si2bin_inst_t));
	
	/* Initialize */
	if (!arg_list)
		arg_list = list_create();
	inst->arg_list = arg_list;
	
	/* Try to create the instruction following all possible encodings for
	 * the same instruction name. */
	snprintf(err_str, sizeof err_str, "invalid instruction: %s", name);
	for (info = hash_table_get(si2bin_inst_info_table, name);
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
			if (!si2bin_token_is_arg_allowed(token, arg))
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
		si2bin_yyerror(err_str);

	/* Initialize opcode */
	inst->info = info;
	inst->opcode = info->inst_info->opcode;

	/* Return */
	return inst;
}


void si2bin_inst_free(struct si2bin_inst_t *inst)
{
	int index;
	
	/* Free argument list */
	LIST_FOR_EACH(inst->arg_list, index)
		si2bin_arg_free(list_get(inst->arg_list, index));
	list_free(inst->arg_list);
	
	/* Rest */
	str_free(inst->comment);
	free(inst);
}


void si2bin_inst_dump(struct si2bin_inst_t *inst, FILE *f)
{
	struct si2bin_arg_t *arg;
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
		si2bin_arg_dump(arg, f);
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


void si2bin_inst_dump_assembly(struct si2bin_inst_t *inst, FILE *f)
{
        struct si2bin_arg_t *arg;

        int i;

        /* Comment attached to the instruction */
        if (inst->comment)
        	fprintf(f, "\n\t# %s\n", inst->comment);

        /* Dump instruction opcode */
        fprintf(f, "\t%s ", inst->info->name);

        /* Dump arguments */
        LIST_FOR_EACH(inst->arg_list, i)
        {
                arg = list_get(inst->arg_list, i);
		assert(arg);
                si2bin_arg_dump_assembly(arg, f);
                if (i < inst->arg_list->count - 1)
			fprintf(f, ", ");
			
	}

	/* New line */
	fprintf(f, "\n");
}


void si2bin_inst_add_comment(struct si2bin_inst_t *inst, char *comment)
{
	inst->comment = str_set(inst->comment, comment);
}


void si2bin_inst_gen(struct si2bin_inst_t *inst)
{
	union si_inst_microcode_t *inst_bytes;
	struct si_inst_info_t *inst_info;
	struct si2bin_inst_info_t *info;

	struct si2bin_arg_t *arg;
	struct si2bin_token_t *token;

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
	
	/* encoding in [31:26], op in [25:28] */
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

	/* encoding in [31:26], op in [25:17] */
	case SI_FMT_VOP3b:

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
		si2bin_yyerror_fmt("%s: unsupported format", __FUNCTION__);
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
		
		case si2bin_token_simm16:
		{
			int value;

			if (arg->type == si2bin_arg_literal || 
				arg->type == si2bin_arg_literal_reduced)
			{
				/* Literal constant other than [-16...64] */
				if (arg->value.literal.val > 0xff00)
					si2bin_yyerror_fmt("%s: Literal in simm16 needs to fit in 16 bit field",
						__FUNCTION__);
				
				inst_bytes->sopk.simm16 = arg->value.literal.val;
			}
			else
			{

				/* Encode */
				value = si2bin_arg_encode_operand(arg);
				if (!IN_RANGE(value, 0, 255))
					si2bin_yyerror("invalid argument type");
				inst_bytes->sopk.simm16 = value;
			}
			break;
		}
		
		case si2bin_token_64_sdst:
		{
			int low;
			int high;

			/* Check range if scalar register range given */
			if (arg->type == si2bin_arg_scalar_register_series)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be s[x:x+1]");
			}
			
			/* Encode */
			inst_bytes->sop2.sdst = si2bin_arg_encode_operand(arg);
			break;
		}

		case si2bin_token_64_ssrc0:
		{
			int value;

			if (arg->type == si2bin_arg_literal)
			{
				/* Literal constant other than [-16...64] is encoded by adding
				 * four more bits to the instruction. */
				if (inst->size == 8)
					si2bin_yyerror("only one literal allowed");
				inst->size = 8;
				inst_bytes->sop2.ssrc0 = 0xff;
				inst_bytes->sop2.lit_cnst = arg->value.literal.val;
			}
			else
			{
				/* Check range of scalar registers */
				if (arg->type == si2bin_arg_scalar_register_series &&
						arg->value.scalar_register_series.high !=
						arg->value.scalar_register_series.low + 1)
					si2bin_yyerror("invalid scalar register series, s[x:x+1] expected");

				/* Encode */
				value = si2bin_arg_encode_operand(arg);
				if (!IN_RANGE(value, 0, 255))
					si2bin_yyerror("invalid argument type");
				inst_bytes->sop2.ssrc0 = value;
			}
			break;
		}

		case si2bin_token_64_ssrc1:
		{
			int value;

			if (arg->type == si2bin_arg_literal)
			{
				/* Literal constant other than [-16...64] is encoded by adding
				 * four more bits to the instruction. */
				if (inst->size == 8)
					si2bin_yyerror("only one literal allowed");
				inst->size = 8;
				inst_bytes->sop2.ssrc1 = 0xff;
				inst_bytes->sop2.lit_cnst = arg->value.literal.val;
			}
			else
			{
				/* Check range of scalar registers */
				if (arg->type == si2bin_arg_scalar_register_series &&
						arg->value.scalar_register_series.high !=
						arg->value.scalar_register_series.low + 1)
					si2bin_yyerror("invalid scalar register series, s[x:x+1] expected");

				/* Encode */
				value = si2bin_arg_encode_operand(arg);
				if (!IN_RANGE(value, 0, 255))
					si2bin_yyerror("invalid argument type");
				inst_bytes->sop2.ssrc1 = value;
			}
			break;
		}
		
		case si2bin_token_mt_maddr:
		{
			struct si2bin_arg_t *qual;
			int soffset;

			/* Offset */
			soffset = si2bin_arg_encode_operand(arg->value.maddr.soffset);
			if (!IN_RANGE(soffset, 0, 253))
				si2bin_yyerror("invalid offset");
			inst_bytes->mtbuf.soffset = soffset;

			/* Data and number format */
			inst_bytes->mtbuf.dfmt = arg->value.maddr.data_format;
			inst_bytes->mtbuf.nfmt = arg->value.maddr.num_format;

			/* Qualifiers */
			qual = arg->value.maddr.qual;
			assert(qual->type == si2bin_arg_maddr_qual);
			inst_bytes->mtbuf.offen = qual->value.maddr_qual.offen;
			inst_bytes->mtbuf.idxen = qual->value.maddr_qual.idxen;
			inst_bytes->mtbuf.offset = qual->value.maddr_qual.offset;

			break;
		}

		case si2bin_token_label:
		{
			struct si2bin_symbol_t *symbol;
			struct si2bin_task_t *task;

			/* Search symbol in symbol table */
			assert(arg->type == si2bin_arg_label);
			symbol = hash_table_get(si2bin_symbol_table, arg->value.label.name);

			/* Create symbol if it doesn't exist */
			if (!symbol)
			{
				symbol = si2bin_symbol_create(arg->value.label.name);
				hash_table_insert(si2bin_symbol_table, symbol->name, symbol);
			}

			/* If symbol is defined, resolve label right away. Otherwise,
			 * program a deferred task to resolve it. */
			if (symbol->defined)
			{
				inst_bytes->sopp.simm16 = (symbol->value -
						si2bin_entry->text_section_buffer
						->offset) / 4 - 1;
			}
			else
			{
				task = si2bin_task_create(si2bin_entry->text_section_buffer
						->offset, symbol);
				list_add(si2bin_task_list, task);
			}
			break;
		}

		case si2bin_token_mt_series_vdata:
		{
			int low = 0;
			int high = 0;
			int high_must = 0;

			/* Get registers */
			switch (arg->type)
			{

			case si2bin_arg_vector_register:

				low = arg->value.vector_register.id;
				high = low;
				break;

			case si2bin_arg_vector_register_series:

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
				si2bin_yyerror_fmt("invalid register series: v[%d:%d]", low, high);

			/* Encode */
			inst_bytes->mtbuf.vdata = low;
			break;
		}

		case si2bin_token_offset:

			/* Depends of argument type */
			switch (arg->type)
			{

			case si2bin_arg_literal:

				if (arg->value.literal.val > 255)
					si2bin_yyerror_fmt("%s: offset needs to fit in 8 bit field",
						__FUNCTION__);
				
				inst_bytes->smrd.imm = 1;
				inst_bytes->smrd.offset = arg->value.literal.val;
				/* FIXME - check valid range of literal */
				break;

			case si2bin_arg_literal_reduced:
				
				inst_bytes->smrd.imm = 1;
				inst_bytes->smrd.offset = arg->value.literal.val;
				break;
			
			case si2bin_arg_scalar_register:

				inst_bytes->smrd.offset = arg->value.scalar_register.id;
				break;

			default:
				panic("%s: invalid argument type for token 'offset'",
					__FUNCTION__);
			}
			break;

		case si2bin_token_sdst:

			/* Encode */
			inst_bytes->sop2.sdst = si2bin_arg_encode_operand(arg);
			break;

		case si2bin_token_series_sbase:

			/* Check that low register is multiple of 2 */
			if (arg->value.scalar_register_series.low % 2)
				si2bin_yyerror("base register must be multiple of 2");

			/* Restrictions for high register */
			switch (inst_info->inst)
			{

			case SI_INST_S_LOAD_DWORDX2:
			case SI_INST_S_LOAD_DWORDX4:

				/* High register must be low plus 1 */
				if (arg->value.scalar_register_series.high !=
						arg->value.scalar_register_series.low + 1)
					si2bin_yyerror("register series must be s[x:x+1]");
				break;

			case SI_INST_S_BUFFER_LOAD_DWORD:
			case SI_INST_S_BUFFER_LOAD_DWORDX2:
			case SI_INST_S_BUFFER_LOAD_DWORDX4:

				/* High register must be low plus 3 */
				if (arg->value.scalar_register_series.high !=
						arg->value.scalar_register_series.low + 3)
					si2bin_yyerror("register series must be s[x:x+3]");
				break;

			default:
				si2bin_yyerror_fmt("%s: unsupported opcode for 'series_sbase' token: %s",
						__FUNCTION__, info->name);
			}

			/* Encode */
			inst_bytes->smrd.sbase = arg->value.scalar_register_series.low / 2;
			break;

		case si2bin_token_series_sdst:

			/* Restrictions for high register */
			switch (inst_info->inst)
			{

			case SI_INST_S_LOAD_DWORDX2:
			case SI_INST_S_BUFFER_LOAD_DWORDX2:

				/* High register must be low plus 1 */
				if (arg->value.scalar_register_series.high !=
						arg->value.scalar_register_series.low + 1)
					si2bin_yyerror("register series must be s[low:low+1]");
				break;

			case SI_INST_S_LOAD_DWORDX4:
			case SI_INST_S_BUFFER_LOAD_DWORDX4:

				/* High register must be low plus 3 */
				if (arg->value.scalar_register_series.high !=
						arg->value.scalar_register_series.low + 3)
					si2bin_yyerror("register series must be s[low:low+3]");
				break;

			default:
				si2bin_yyerror_fmt("%s: unsupported opcode for 'series_sdst' token: %s",
						__FUNCTION__, info->name);
			}

			/* Encode */
			inst_bytes->smrd.sdst = arg->value.scalar_register_series.low;
			break;

		case si2bin_token_series_srsrc:
		{
			int low = arg->value.scalar_register_series.low;
			int high = arg->value.scalar_register_series.high;

			/* Base register must be multiple of 4 */
			if (low % 4)
				si2bin_yyerror_fmt("low register must be multiple of 4 in s[%d:%d]",
						low, high);

			/* High register must be low + 3 */
			if (high != low + 3)
				si2bin_yyerror_fmt("register series must span 4 registers in s[%d:%d]",
						low, high);

			/* Encode */
			inst_bytes->mtbuf.srsrc = low >> 2;
			break;
		}

		case si2bin_token_smrd_sdst:

			/* Encode */
			inst_bytes->smrd.sdst = arg->value.scalar_register.id;
			break;

		case si2bin_token_src0:

			if (arg->type == si2bin_arg_literal)
			{
				/* Literal constant other than [-16...64] is encoded by adding
				 * four more bits to the instruction. */
				if (inst->size == 8)
					si2bin_yyerror("only one literal allowed");
				inst->size = 8;
				inst_bytes->vopc.src0 = 0xff;
				inst_bytes->vopc.lit_cnst = arg->value.literal.val;
			}
			else
			{
				inst_bytes->vopc.src0 = si2bin_arg_encode_operand(arg);
			}
			break;

		case si2bin_token_ssrc0:
		{
			int value;

			if (arg->type == si2bin_arg_literal)
			{
				/* Literal constant other than [-16...64] is encoded by adding
				 * four more bits to the instruction. */
				if (inst->size == 8)
					si2bin_yyerror("only one literal allowed");
				inst->size = 8;
				inst_bytes->sop2.ssrc0 = 0xff;
				inst_bytes->sop2.lit_cnst = arg->value.literal.val;
			}
			else
			{
				value = si2bin_arg_encode_operand(arg);
				if (!IN_RANGE(value, 0, 255))
					si2bin_yyerror("invalid argument type");
				inst_bytes->sop2.ssrc0 = value;
			}
			break;
		}

		case si2bin_token_ssrc1:
		{
			int value;

			if (arg->type == si2bin_arg_literal)
			{
				/* Literal constant other than [-16...64] is encoded by adding
				 * four more bits to the instruction. */
				if (inst->size == 8)
					si2bin_yyerror("only one literal allowed");
				inst->size = 8;
				inst_bytes->sop2.ssrc1 = 0xff;
				inst_bytes->sop2.lit_cnst = arg->value.literal.val;
			}
			else
			{
				value = si2bin_arg_encode_operand(arg);
				if (!IN_RANGE(value, 0, 255))
					si2bin_yyerror("invalid argument type");
				inst_bytes->sop2.ssrc1 = value;
			}
			break;
		}

		case si2bin_token_vaddr:

			switch (arg->type)
			{

			case si2bin_arg_vector_register:

				inst_bytes->mtbuf.vaddr = arg->value.vector_register.id;
				break;
			
			case si2bin_arg_vector_register_series:
				/* High register must be low plus 1 */
				if (arg->value.vector_register_series.high !=
						arg->value.vector_register_series.low + 1)
					si2bin_yyerror("register series must be v[x:x+1]");
				
				inst_bytes->mtbuf.vaddr = 
					arg->value.vector_register_series.low;

				/* FIXME - Find way to verify that idxen and offen are set */
				break;

			default:
				si2bin_yyerror_fmt("%s: invalid argument type for token 'vaddr'",
					__FUNCTION__);
			}
			break;

		case si2bin_token_vcc:

			/* Not encoded */
			break;

		case si2bin_token_svdst:

			/* Check for scalar register */
			assert(arg->type == si2bin_arg_scalar_register);
			
			/* Encode */
			inst_bytes->vop1.vdst = si2bin_arg_encode_operand(arg);
			break;

		case si2bin_token_vdst:

			/* Encode */
			inst_bytes->vop1.vdst = arg->value.vector_register.id;
			break;
		
		case si2bin_token_64_src0:
		{
			int low;
			int high;

			/* Check argument type */
			if (arg->type == si2bin_arg_scalar_register_series)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be s[low:low+1]");
			}
			else if (arg->type == si2bin_arg_vector_register_series)
			{
				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be v[low:low+1]");
			}

			/* Encode */
			inst_bytes->vop1.src0 = si2bin_arg_encode_operand(arg);
			break;
		}
	
		case si2bin_token_64_vdst:
		{
			int low;
			int high;

			/* Check argument type */
			if (arg->type == si2bin_arg_vector_register_series)
			{
				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be v[low:low+1]");
			}
			
			/* Encode */
			inst_bytes->vop1.vdst = arg->value.vector_register_series.low;
			break;
		}
		case si2bin_token_vop3_64_svdst:
		{
			int low;
			int high;

			/* If operand is a scalar register series, check range */
			if (arg->type == si2bin_arg_scalar_register_series)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be s[low:low+1]");
			}

			/* Encode */
			inst_bytes->vop3a.vdst = si2bin_arg_encode_operand(arg);
			break;
		}

		case si2bin_token_vop3_src0:

			inst_bytes->vop3a.src0 = si2bin_arg_encode_operand(arg);
			if (arg->abs)
				inst_bytes->vop3a.abs |= 0x1;
			if (arg->neg)
				inst_bytes->vop3a.neg |= 0x1;
			break;

		case si2bin_token_vop3_src1:

			inst_bytes->vop3a.src1 = si2bin_arg_encode_operand(arg);
			if (arg->abs)
				inst_bytes->vop3a.abs |= 0x2;
			if (arg->neg)
				inst_bytes->vop3a.neg |= 0x2;
			break;

		case si2bin_token_vop3_src2:

			inst_bytes->vop3a.src2 = si2bin_arg_encode_operand(arg);
			if (arg->abs)
				inst_bytes->vop3a.abs |= 0x4;
			if (arg->neg)
				inst_bytes->vop3a.neg |= 0x4;
			break;

		case si2bin_token_vop3_64_src0:
		{
			
			int low;
			int high;

			/* If operand is a scalar register series, check range */
			if (arg->type == si2bin_arg_scalar_register_series)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be s[low:low+1]");
			}
			else if (arg->type == si2bin_arg_vector_register_series)
			{
				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be v[low:low+1]");
			}

			/* Encode */
			inst_bytes->vop3a.src0 = si2bin_arg_encode_operand(arg);
			
			if (arg->abs)
				inst_bytes->vop3a.abs |= 0x1;
			if (arg->neg)
				inst_bytes->vop3a.neg |= 0x1;
			break;
		}
		case si2bin_token_vop3_64_src1:
		{	
			int low;
			int high;

			/* If operand is a scalar register series, check range */
			if (arg->type == si2bin_arg_scalar_register_series)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be s[low:low+1]");
			}
			else if (arg->type == si2bin_arg_vector_register_series)
			{
				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be v[low:low+1]");
			}

			/* Encode */
			inst_bytes->vop3a.src1 = si2bin_arg_encode_operand(arg);
			
			if (arg->abs)
				inst_bytes->vop3a.abs |= 0x2;
			if (arg->neg)
				inst_bytes->vop3a.neg |= 0x2;
			break;
		}
		case si2bin_token_vop3_64_src2:
		{
			int low;
			int high;

			/* If operand is a scalar register series, check range */
			if (arg->type == si2bin_arg_scalar_register_series)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be s[low:low+1]");
			}
			else if (arg->type == si2bin_arg_vector_register_series)
			{
				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be v[low:low+1]");
			}

			/* Encode */
			inst_bytes->vop3a.src2 = si2bin_arg_encode_operand(arg);
			
			if (arg->abs)
				inst_bytes->vop3a.abs |= 0x4;
			if (arg->neg)
				inst_bytes->vop3a.neg |= 0x4;
			break;
		}
		
		case si2bin_token_vop3_64_sdst:
			
			/* Encode */
			inst_bytes->vop3b.sdst = si2bin_arg_encode_operand(arg);
			
			break;

		case si2bin_token_vop3_vdst:
			
			/* Encode */
			inst_bytes->vop3a.vdst = arg->value.vector_register.id;
			break;

		case si2bin_token_vop3_64_vdst:
		{
			int low;
			int high;

			/* Check argument type */
			if (arg->type == si2bin_arg_vector_register_series)
			{
				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be v[low:low+1]");
			}
			
			/* Encode */
			inst_bytes->vop3a.vdst = arg->value.vector_register_series.low;
			break;
		}

		case si2bin_token_vsrc1:

			/* Make sure argument is a vector register */
			assert(arg->type == si2bin_arg_vector_register);

			/* Encode */
			inst_bytes->vopc.vsrc1 = arg->value.vector_register.id;
			
			break;

		case si2bin_token_wait_cnt:
			/* vmcnt(x) */
			if (arg->value.wait_cnt.vmcnt_active)
			{
				if (!IN_RANGE(arg->value.wait_cnt.vmcnt_value, 0, 0xe))
					si2bin_yyerror("invalid value for vmcnt");
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
					si2bin_yyerror("invalid value for lgkmcnt");
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
					si2bin_yyerror("invalid value for expcnt");
				inst_bytes->sopp.simm16 = SET_BITS_32(inst_bytes->sopp.simm16,
						6, 4, arg->value.wait_cnt.expcnt_value);
			}
			else
			{
				inst_bytes->sopp.simm16 = SET_BITS_32(inst_bytes->sopp.simm16,
						6, 4, 0x7);
			}
			break;

		case si2bin_token_addr:
			
			/* Make sure argument is a vector register */
			assert(arg->type == si2bin_arg_vector_register);
			
			/* Encode */
			inst_bytes->ds.addr = arg->value.vector_register.id;
			break;

		case si2bin_token_data0:

			/* Make sure argument is a vector register */
			assert(arg->type == si2bin_arg_vector_register);

			/* Encode */
			inst_bytes->ds.data0 = arg->value.vector_register.id;
			break;

		case si2bin_token_ds_vdst:

			/* Make sure argument is a vector register */
			assert(arg->type == si2bin_arg_vector_register);

			/* Encode */
			inst_bytes->ds.vdst = arg->value.vector_register.id;
			break;
		
		
		default:
			si2bin_yyerror_fmt("unsupported token for argument %d",
				index + 1);
		}
	}
}


