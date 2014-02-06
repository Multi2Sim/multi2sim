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

#include <lib/class/array.h>
#include <lib/class/elf-writer.h>
#include <lib/class/hash-table.h>
#include <lib/class/list.h>
#include <lib/class/string.h>
#include <lib/cpp/Wrapper.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "arg.h"
#include "inner-bin.h"
#include "inst.h"
#include "inst-info.h"
#include "PVars.h"
#include "si2bin.h"
#include "symbol.h"
#include "task.h"
#include "token.h"


/*
 * Class 'Si2binInst'
 */

void Si2binInstCreate(Si2binInst *self, SIInstOpcode opcode, List *arg_list)
{
	Si2binArg *arg;
	Si2binToken *token;
	Si2binInstInfo *info;
	int index;

	/* Initialize */
	self->arg_list = arg_list;
	self->comment = new(String, "");

	/* Check valid opcode */
	self->opcode = opcode;
	if (!IN_RANGE(opcode, 1, SIInstOpcodeCount - 1))
		fatal("%s: invalid opcode (%d)", __FUNCTION__, opcode);

	/* Get instruction information */
	self->info = asSi2binInstInfo(ArrayGet(si2bin->inst_info_array, opcode));
	info = self->info;
	if (!info)
		fatal("%s: opcode %d not supported", __FUNCTION__, opcode);

	/* Check number of arguments */
	if (arg_list->count != info->token_list->count)
		fatal("%s: invalid number of arguments (%d given, %d expected)",
				__FUNCTION__, arg_list->count, info->token_list->count);

	/* Check argument types */
	index = 0;
	ListHead(info->token_list);
	ListForEach(arg_list, arg, Si2binArg)
	{
		/* Get formal argument from instruction info. Associate token with the
		 * instruction argument. */
		token = asSi2binToken(ListGet(info->token_list));
		arg->token = token;
		assert(token);

		/* Check that actual argument type is acceptable for token */
		if (!Si2binTokenIsArgAllowed(token, arg))
			fatal("%s: invalid type for argument %d", __FUNCTION__, index);

		/* Next */
		ListNext(info->token_list);
		index++;
	}
}


void Si2binInstCreateWithName(Si2binInst *self, char *name, List *arg_list)
{
	Si2binInstInfo *info;

	Si2binArg *arg;
	Si2binToken *token;

	char err_str[MAX_STRING_SIZE];
	int index;

	/* Create argument list if null */
	if (!arg_list)
		arg_list = new(List);
	
	/* Initialize */
	self->arg_list = arg_list;
	self->comment = new(String, "");
	
	/* Try to create the instruction following all possible encodings for
	 * the same instruction name. */
	snprintf(err_str, sizeof err_str, "invalid instruction: %s", name);
	for (info = asSi2binInstInfo(HashTableGetString(si2bin->inst_info_table,
			name)); info; info = info->next)
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
		index = 0;
		ListHead(info->token_list);
		ListForEach(arg_list, arg, Si2binArg)
		{
			/* Get formal argument from instruction info. We associate the
			 * instruction argument with the token. */
			token = asSi2binToken(ListGet(info->token_list));
			arg->token = token;
			assert(token);

			/* Check that actual argument type is acceptable for token */
			if (!Si2binTokenIsArgAllowed(token, arg))
			{
				snprintf(err_str, sizeof err_str,
					"invalid type for argument %d", index + 1);
				break;
			}

			/* Next */
			ListNext(info->token_list);
			index++;
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
	self->info = info;
	self->opcode = info->inst_info->op;
}


void Si2binInstDestroy(Si2binInst *self)
{
	ListDeleteObjects(self->arg_list);
	delete(self->arg_list);
	delete(self->comment);
}


void Si2binInstDump(Si2binInst *self, FILE *f)
{
	Si2binArg *arg;
	unsigned int word;
	
	int i;
	int j;
	
	/* Dump instruction opcode */
	fprintf(f, "Instruction %s\n", self->info->name->text);
	fprintf(f, "\tformat=%s, size=%d\n",
			StringMapValueWrap(si_inst_format_map,
			self->info->inst_info->fmt),
			self->size);

	/* Dump arguments */
	i = 0;
	ListForEach(self->arg_list, arg, Si2binArg)
	{
		fprintf(f, "\targ %d: ", i);
		Si2binArgDump(arg, f);
		fprintf(f, "\n");
		i++;
	}

	/* Empty instruction bits */
	if (!self->size)
		return;

	/* Print words */
	for (i = 0; i < self->size / 4; i++)
	{
		word = * (int *) &self->bytes.byte[i * 4];
		printf("\tword %d:  hex = { %08x },  bin = {", i, word);
		for (j = 0; j < 32; j++)
			printf("%s%d", j % 4 ? "" : " ", (word >> (31 - j)) & 1);
		printf(" }\n");
	}
}


void Si2binInstDumpAssembly(Si2binInst *self, FILE *f)
{
        Si2binArg *arg;

        int i;

        /* Comment attached to the instruction */
        if (self->comment->length)
        	fprintf(f, "\n\t# %s\n", self->comment->text);

        /* Dump instruction opcode */
        fprintf(f, "\t%s ", self->info->name->text);

        /* Dump arguments */
	i = 0;
	ListForEach(self->arg_list, arg, Si2binArg)
        {
		assert(arg);
                Si2binArgDumpAssembly(arg, f);
                if (i < self->arg_list->count - 1)
			fprintf(f, ", ");
		i++;
			
	}

	/* New line */
	fprintf(f, "\n");
}


void Si2binInstAddComment(Si2binInst *self, char *comment)
{
	StringSet(self->comment, "%s", comment);
}


void Si2binInstGenerate(Si2binInst *self)
{
	SIInstBytes *inst_bytes;
	SIInstInfo *inst_info;
	Si2binInstInfo *info;

	Si2binArg *arg;
	Si2binToken *token;

	int index;

	/* Initialize */
	inst_bytes = &self->bytes;
	info = self->info;
	assert(info);
	inst_info = info->inst_info;

	/* By default, the instruction has the number of bytes specified by its
	 * format. 4-bit instructions could be extended later to 8 bits upon
	 * the presence of a literal constant. */
	self->size = inst_info->size;

	/* Instruction opcode */
	switch (inst_info->fmt)
	{

	/* encoding in [31:26], op in [18:16] */
	case SIInstFormatMTBUF:

		inst_bytes->mtbuf.enc = 0x3a;
		inst_bytes->mtbuf.op = inst_info->op;
		break;
	
	/* encoding in [:], op in [] */
	case SIInstFormatMUBUF:
		
		inst_bytes->mubuf.enc = 0x38;
		inst_bytes->mubuf.op = inst_info->op;
		break;

	/* encoding in [:], op in [] */
	case SIInstFormatMIMG:
		
		inst_bytes->mimg.enc = 0x3c;
		inst_bytes->mimg.op = inst_info->op;
		break;

	/* encoding in [31:27], op in [26:22] */
	case SIInstFormatSMRD:

		inst_bytes->smrd.enc = 0x18;
		inst_bytes->smrd.op = inst_info->op;
		break;
	
	/* encoding in [31:26], op in [25:28] */
	case SIInstFormatDS:
		
		inst_bytes->ds.enc = 0x36;
		inst_bytes->ds.op = inst_info->op;
		break;

	/* encoding in [31:23], op in [22:16] */
	case SIInstFormatSOPC:
		
		inst_bytes->sopc.enc = 0x17e;
		inst_bytes->sopc.op = inst_info->op;
		break;

	/* encoding in [31:23], op in [15:8] */
	case SIInstFormatSOP1:

		inst_bytes->sop1.enc = 0x17d;
		inst_bytes->sop1.op = inst_info->op;
		break;

	/* encoding in [31:30], op in [29:23] */
	case SIInstFormatSOP2:

		inst_bytes->sop2.enc = 0x2;
		inst_bytes->sop2.op = inst_info->op;
		break;

	/* encoding in [31:23], op in [22:16] */
	case SIInstFormatSOPP:

		inst_bytes->sopp.enc = 0x17f;
		inst_bytes->sopp.op = inst_info->op;
		break;
	
	/* encoding in [:], op in [] */
	case SIInstFormatSOPK:
		
		inst_bytes->sopk.enc = 0xb;
		inst_bytes->sopk.op = inst_info->op;
		break;

	/* encoding in [:], op in [] */
	case SIInstFormatVOPC:
		
		inst_bytes->vopc.enc = 0x3e;
		inst_bytes->vopc.op = inst_info->op;
		break;

	/* encoding in [31:25], op in [16:9] */
	case SIInstFormatVOP1:

		inst_bytes->vop1.enc = 0x3f;
		inst_bytes->vop1.op = inst_info->op;
		break;

	/* encoding in [31], op in [30:25] */
	case SIInstFormatVOP2:

		inst_bytes->vop2.enc = 0x0;
		inst_bytes->vop2.op = inst_info->op;
		break;

	/* encoding in [31:26], op in [25:17] */
	case SIInstFormatVOP3a:

		inst_bytes->vop3a.enc = 0x34;
		inst_bytes->vop3a.op = inst_info->op;
		break;

	/* encoding in [31:26], op in [25:17] */
	case SIInstFormatVOP3b:

		inst_bytes->vop3a.enc = 0x34;
		inst_bytes->vop3a.op = inst_info->op;
		break;

	/* encoding in [:], op in [] */
	case SIInstFormatVINTRP:
		
		inst_bytes->vintrp.enc = 0x32;
		inst_bytes->vintrp.op = inst_info->op;
		break;

	/* encoding in [:], op in [] */
	case SIInstFormatEXP:
		
		inst_bytes->exp.enc = 0x3e;
		/* No opcode: only 1 instruction */
		break;

	default:
		si2bin_yyerror_fmt("%s: unsupported format", __FUNCTION__);
	}

	/* Arguments */
	assert(self->arg_list->count == info->token_list->count);
	index = 0;
	ListHead(info->token_list);
	ListForEach(self->arg_list, arg, Si2binArg)
	{
		/* Get argument */
		token = asSi2binToken(ListGet(info->token_list));
		assert(arg);
		assert(token);

		/* Check token */
		switch (token->type)
		{
		
		case Si2binTokenSimm16:
		{
			int value;

			if (arg->type == Si2binArgLiteral || 
				arg->type == Si2binArgLiteralReduced)
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
				value = Si2binArgEncodeOperand(arg);
				if (!IN_RANGE(value, 0, 255))
					si2bin_yyerror("invalid argument type");
				inst_bytes->sopk.simm16 = value;
			}
			break;
		}
		
		case Si2binToken64Sdst:
		{
			int low;
			int high;

			/* Check range if scalar register range given */
			if (arg->type == Si2binArgScalarRegisterSeries)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be s[x:x+1]");
			}
			
			/* Encode */
			inst_bytes->sop2.sdst = Si2binArgEncodeOperand(arg);
			break;
		}

		case Si2binToken64Ssrc0:
		{
			int value;

			if (arg->type == Si2binArgLiteral)
			{
				/* Literal constant other than [-16...64] is encoded by adding
				 * four more bits to the instruction. */
				if (self->size == 8)
					si2bin_yyerror("only one literal allowed");
				self->size = 8;
				inst_bytes->sop2.ssrc0 = 0xff;
				inst_bytes->sop2.lit_cnst = arg->value.literal.val;
			}
			else
			{
				/* Check range of scalar registers */
				if (arg->type == Si2binArgScalarRegisterSeries &&
						arg->value.scalar_register_series.high !=
						arg->value.scalar_register_series.low + 1)
					si2bin_yyerror("invalid scalar register series, s[x:x+1] expected");

				/* Encode */
				value = Si2binArgEncodeOperand(arg);
				if (!IN_RANGE(value, 0, 255))
					si2bin_yyerror("invalid argument type");
				inst_bytes->sop2.ssrc0 = value;
			}
			break;
		}

		case Si2binToken64Ssrc1:
		{
			int value;

			if (arg->type == Si2binArgLiteral)
			{
				/* Literal constant other than [-16...64] is encoded by adding
				 * four more bits to the instruction. */
				if (self->size == 8)
					si2bin_yyerror("only one literal allowed");
				self->size = 8;
				inst_bytes->sop2.ssrc1 = 0xff;
				inst_bytes->sop2.lit_cnst = arg->value.literal.val;
			}
			else
			{
				/* Check range of scalar registers */
				if (arg->type == Si2binArgScalarRegisterSeries &&
						arg->value.scalar_register_series.high !=
						arg->value.scalar_register_series.low + 1)
					si2bin_yyerror("invalid scalar register series, s[x:x+1] expected");

				/* Encode */
				value = Si2binArgEncodeOperand(arg);
				if (!IN_RANGE(value, 0, 255))
					si2bin_yyerror("invalid argument type");
				inst_bytes->sop2.ssrc1 = value;
			}
			break;
		}
		
		case Si2binTokenMtMaddr:
		{
			Si2binArg *qual;
			int soffset;

			/* Offset */
			soffset = Si2binArgEncodeOperand(arg->value.maddr.soffset);
			if (!IN_RANGE(soffset, 0, 253))
				si2bin_yyerror("invalid offset");
			inst_bytes->mtbuf.soffset = soffset;

			/* Data and number format */
			inst_bytes->mtbuf.dfmt = arg->value.maddr.data_format;
			inst_bytes->mtbuf.nfmt = arg->value.maddr.num_format;

			/* Qualifiers */
			qual = arg->value.maddr.qual;
			assert(qual->type == Si2binArgMaddrQual);
			inst_bytes->mtbuf.offen = qual->value.maddr_qual.offen;
			inst_bytes->mtbuf.idxen = qual->value.maddr_qual.idxen;
			inst_bytes->mtbuf.offset = qual->value.maddr_qual.offset;

			break;
		}

		case Si2binTokenLabel:
		{
			Si2binSymbol *symbol;
			Si2binTask *task;

			/* Search symbol in symbol table */
			assert(arg->type == Si2binArgLabel);
			symbol = asSi2binSymbol(HashTableGetString(si2bin->symbol_table,
					arg->value.label.name));

			/* Create symbol if it doesn't exist */
			if (!symbol)
			{
				symbol = new(Si2binSymbol, arg->value.label.name);
				HashTableInsert(si2bin->symbol_table, asObject(symbol->name),
						asObject(symbol));
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
				task = new(Si2binTask, si2bin_entry->text_section_buffer
						->offset, symbol);
				ListAdd(si2bin->task_list, asObject(task));
			}
			break;
		}
		
//		case Si2binTokenMtSeriesVdataSrc:
//		{
//			/* Encode */
//			inst_bytes->mtbuf.vdata = arg->value.vector_register.id;
//			break;
//		}
//
//		case Si2binTokenMtSeriesVdataDst:
//		{
//			/* Encode */
//			inst_bytes->mtbuf.vdata = arg->value.vector_register.id;
//			break;
//		}

		case Si2binTokenMtSeriesVdataDst:
		case Si2binTokenMtSeriesVdataSrc:
		{
			int low = 0;
			int high = 0;
			int high_must = 0;

			/* Get registers */
			switch (arg->type)
			{

			case Si2binArgVectorRegister:

				low = arg->value.vector_register.id;
				high = low;
				break;

			case Si2binArgVectorRegisterSeries:

				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				break;

			default:
				panic("%s: invalid argument type for token 'mt_series_vdata'",
						__FUNCTION__);
			}

			/* Restriction in vector register range */
			switch (inst_info->opcode)
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
						__FUNCTION__, info->name->text);
			}

			/* Check range */
			if (high != high_must)
				si2bin_yyerror_fmt("invalid register series: v[%d:%d]", low, high);

			/* Encode */
			inst_bytes->mtbuf.vdata = low;
			break;
		}

		case Si2binTokenOffset:

			/* Depends of argument type */
			switch (arg->type)
			{

			case Si2binArgLiteral:

				if (arg->value.literal.val > 255)
					si2bin_yyerror_fmt("%s: offset needs to fit in 8 bit field",
						__FUNCTION__);
				
				inst_bytes->smrd.imm = 1;
				inst_bytes->smrd.offset = arg->value.literal.val;
				/* FIXME - check valid range of literal */
				break;

			case Si2binArgLiteralReduced:
				
				inst_bytes->smrd.imm = 1;
				inst_bytes->smrd.offset = arg->value.literal.val;
				break;
			
			case Si2binArgScalarRegister:

				inst_bytes->smrd.offset = arg->value.scalar_register.id;
				break;

			default:
				panic("%s: invalid argument type for token 'offset'",
					__FUNCTION__);
			}
			break;

		case Si2binTokenSdst:

			/* Encode */
			inst_bytes->sop2.sdst = Si2binArgEncodeOperand(arg);
			break;

		case Si2binTokenSeriesSbase:

			/* Check that low register is multiple of 2 */
			if (arg->value.scalar_register_series.low % 2)
				si2bin_yyerror("base register must be multiple of 2");

			/* Restrictions for high register */
			switch (inst_info->opcode)
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
						__FUNCTION__, info->name->text);
			}

			/* Encode */
			inst_bytes->smrd.sbase = arg->value.scalar_register_series.low / 2;
			break;

		case Si2binTokenSeriesSdst:

			/* Restrictions for high register */
			switch (inst_info->opcode)
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
						__FUNCTION__, info->name->text);
			}

			/* Encode */
			inst_bytes->smrd.sdst = arg->value.scalar_register_series.low;
			break;

		case Si2binTokenSeriesSrsrc:
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

		case Si2binTokenSmrdSdst:

			/* Encode */
			inst_bytes->smrd.sdst = arg->value.scalar_register.id;
			break;

		case Si2binTokenSrc0:

			if (arg->type == Si2binArgLiteral)
			{
				/* Literal constant other than [-16...64] is encoded by adding
				 * four more bits to the instruction. */
				if (self->size == 8)
					si2bin_yyerror("only one literal allowed");
				self->size = 8;
				inst_bytes->vopc.src0 = 0xff;
				inst_bytes->vopc.lit_cnst = arg->value.literal.val;
			}
			else
			{
				inst_bytes->vopc.src0 = Si2binArgEncodeOperand(arg);
			}
			break;

		case Si2binTokenSsrc0:
		{
			int value;

			if (arg->type == Si2binArgLiteral)
			{
				/* Literal constant other than [-16...64] is encoded by adding
				 * four more bits to the instruction. */
				if (self->size == 8)
					si2bin_yyerror("only one literal allowed");
				self->size = 8;
				inst_bytes->sop2.ssrc0 = 0xff;
				inst_bytes->sop2.lit_cnst = arg->value.literal.val;
			}
			else
			{
				value = Si2binArgEncodeOperand(arg);
				if (!IN_RANGE(value, 0, 255))
					si2bin_yyerror("invalid argument type");
				inst_bytes->sop2.ssrc0 = value;
			}
			break;
		}

		case Si2binTokenSsrc1:
		{
			int value;

			if (arg->type == Si2binArgLiteral)
			{
				/* Literal constant other than [-16...64] is encoded by adding
				 * four more bits to the instruction. */
				if (self->size == 8)
					si2bin_yyerror("only one literal allowed");
				self->size = 8;
				inst_bytes->sop2.ssrc1 = 0xff;
				inst_bytes->sop2.lit_cnst = arg->value.literal.val;
			}
			else
			{
				value = Si2binArgEncodeOperand(arg);
				if (!IN_RANGE(value, 0, 255))
					si2bin_yyerror("invalid argument type");
				inst_bytes->sop2.ssrc1 = value;
			}
			break;
		}

		case Si2binTokenVaddr:

			switch (arg->type)
			{

			case Si2binArgVectorRegister:

				inst_bytes->mtbuf.vaddr = arg->value.vector_register.id;
				break;
			
			case Si2binArgVectorRegisterSeries:
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

		case Si2binTokenVcc:

			/* Not encoded */
			break;

		case Si2binTokenSvdst:

			/* Check for scalar register */
			assert(arg->type == Si2binArgScalarRegister);
			
			/* Encode */
			inst_bytes->vop1.vdst = Si2binArgEncodeOperand(arg);
			break;

		case Si2binTokenVdst:

			/* Encode */
			inst_bytes->vop1.vdst = arg->value.vector_register.id;
			break;
		
		case Si2binToken64Src0:
		{
			int low;
			int high;

			/* Check argument type */
			if (arg->type == Si2binArgScalarRegisterSeries)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be s[low:low+1]");
			}
			else if (arg->type == Si2binArgVectorRegisterSeries)
			{
				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be v[low:low+1]");
			}

			/* Encode */
			inst_bytes->vop1.src0 = Si2binArgEncodeOperand(arg);
			break;
		}
	
		case Si2binToken64Vdst:
		{
			int low;
			int high;

			/* Check argument type */
			if (arg->type == Si2binArgVectorRegisterSeries)
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
		case Si2binTokenVop364Svdst:
		{
			int low;
			int high;

			/* If operand is a scalar register series, check range */
			if (arg->type == Si2binArgScalarRegisterSeries)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be s[low:low+1]");
			}

			/* Encode */
			inst_bytes->vop3a.vdst = Si2binArgEncodeOperand(arg);
			break;
		}

		case Si2binTokenVop3Src0:

			inst_bytes->vop3a.src0 = Si2binArgEncodeOperand(arg);
			if (arg->abs)
				inst_bytes->vop3a.abs |= 0x1;
			if (arg->neg)
				inst_bytes->vop3a.neg |= 0x1;
			break;

		case Si2binTokenVop3Src1:

			inst_bytes->vop3a.src1 = Si2binArgEncodeOperand(arg);
			if (arg->abs)
				inst_bytes->vop3a.abs |= 0x2;
			if (arg->neg)
				inst_bytes->vop3a.neg |= 0x2;
			break;

		case Si2binTokenVop3Src2:

			inst_bytes->vop3a.src2 = Si2binArgEncodeOperand(arg);
			if (arg->abs)
				inst_bytes->vop3a.abs |= 0x4;
			if (arg->neg)
				inst_bytes->vop3a.neg |= 0x4;
			break;

		case Si2binTokenVop364Src0:
		{
			
			int low;
			int high;

			/* If operand is a scalar register series, check range */
			if (arg->type == Si2binArgScalarRegisterSeries)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be s[low:low+1]");
			}
			else if (arg->type == Si2binArgVectorRegisterSeries)
			{
				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be v[low:low+1]");
			}

			/* Encode */
			inst_bytes->vop3a.src0 = Si2binArgEncodeOperand(arg);
			
			if (arg->abs)
				inst_bytes->vop3a.abs |= 0x1;
			if (arg->neg)
				inst_bytes->vop3a.neg |= 0x1;
			break;
		}
		case Si2binTokenVop364Src1:
		{	
			int low;
			int high;

			/* If operand is a scalar register series, check range */
			if (arg->type == Si2binArgScalarRegisterSeries)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be s[low:low+1]");
			}
			else if (arg->type == Si2binArgVectorRegisterSeries)
			{
				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be v[low:low+1]");
			}

			/* Encode */
			inst_bytes->vop3a.src1 = Si2binArgEncodeOperand(arg);
			
			if (arg->abs)
				inst_bytes->vop3a.abs |= 0x2;
			if (arg->neg)
				inst_bytes->vop3a.neg |= 0x2;
			break;
		}
		case Si2binTokenVop364Src2:
		{
			int low;
			int high;

			/* If operand is a scalar register series, check range */
			if (arg->type == Si2binArgScalarRegisterSeries)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be s[low:low+1]");
			}
			else if (arg->type == Si2binArgVectorRegisterSeries)
			{
				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be v[low:low+1]");
			}

			/* Encode */
			inst_bytes->vop3a.src2 = Si2binArgEncodeOperand(arg);
			
			if (arg->abs)
				inst_bytes->vop3a.abs |= 0x4;
			if (arg->neg)
				inst_bytes->vop3a.neg |= 0x4;
			break;
		}
		
		case Si2binTokenVop364Sdst:
			
			/* Encode */
			inst_bytes->vop3b.sdst = Si2binArgEncodeOperand(arg);
			
			break;

		case Si2binTokenVop3Vdst:
			
			/* Encode */
			inst_bytes->vop3a.vdst = arg->value.vector_register.id;
			break;

		case Si2binTokenVop364Vdst:
		{
			int low;
			int high;

			/* Check argument type */
			if (arg->type == Si2binArgVectorRegisterSeries)
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

		case Si2binTokenVsrc1:

			/* Make sure argument is a vector register */
			assert(arg->type == Si2binArgVectorRegister);

			/* Encode */
			inst_bytes->vopc.vsrc1 = arg->value.vector_register.id;
			
			break;

		case Si2binTokenWaitCnt:
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

		case Si2binTokenAddr:
			
			/* Make sure argument is a vector register */
			assert(arg->type == Si2binArgVectorRegister);
			
			/* Encode */
			inst_bytes->ds.addr = arg->value.vector_register.id;
			break;

		case Si2binTokenData0:

			/* Make sure argument is a vector register */
			assert(arg->type == Si2binArgVectorRegister);

			/* Encode */
			inst_bytes->ds.data0 = arg->value.vector_register.id;
			break;

		case Si2binTokenDsVdst:

			/* Make sure argument is a vector register */
			assert(arg->type == Si2binArgVectorRegister);

			/* Encode */
			inst_bytes->ds.vdst = arg->value.vector_register.id;
			break;
		
		
		default:
			si2bin_yyerror_fmt("unsupported token for argument %d: %d",
				index + 1, token->type);
		}

		/* Next */
		ListNext(info->token_list);
		index++;
	}
}


