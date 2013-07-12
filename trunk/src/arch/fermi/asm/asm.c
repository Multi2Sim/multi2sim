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


#include <ctype.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/elf-encode.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "asm.h"  



/* 
 * Initialization of disassembler
 */

 
/* Table containing information of all instructions */
struct frm_inst_info_t frm_inst_info[FRM_INST_COUNT];

/* Pointers to 'frm_inst_info' table indexed by instruction opcode */
#define FRM_ISNT_INFO_LONG_SIZE 1024

/* Table containing information of all instructions. Indexed by instruction opcode */
static struct frm_inst_info_t *frm_inst_info_long[FRM_ISNT_INFO_LONG_SIZE];

/* Table containing names of all special registers */
static char *frm_sr[FRM_SR_COUNT];

void frm_disasm_init()
{
	struct frm_inst_info_t *info;
	int i;

	/* Read information about all instructions */
#define DEFINST(_name, _fmt_str, _fmt, _category, _opcode) \
	info = &frm_inst_info[FRM_INST_##_name]; \
	info->inst = FRM_INST_##_name; \
	info->category = FRM_INST_CAT_##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->fmt = FRM_FMT_##_fmt; \
	info->opcode = _opcode; \
	info->size = 8;
#include "asm.dat"
#undef DEFINST

	for (i = 1; i < FRM_INST_COUNT; ++i)
	{
		info = &frm_inst_info[i];
		frm_inst_info_long[info->opcode] = info;
	}

	/* Special registers */
	frm_sr[FRM_SR_Laneld] = "SR_Laneld";
	frm_sr[FRM_SR_VirtCfg] = "SR_VirtCfg";
	frm_sr[FRM_SR_VirtId] = "SR_VirtId";
	frm_sr[FRM_SR_PM0] = "SR_PM0";
	frm_sr[FRM_SR_PM1] = "SR_PM1";
	frm_sr[FRM_SR_PM2] = "SR_PM2";
	frm_sr[FRM_SR_PM3] = "SR_PM3";
	frm_sr[FRM_SR_PM4] = "SR_PM4";
	frm_sr[FRM_SR_PM5] = "SR_PM5";
	frm_sr[FRM_SR_PM6] = "SR_PM6";
	frm_sr[FRM_SR_PM7] = "SR_PM7";
	frm_sr[FRM_SR_PRIM_TYPE] = "SR_PRIM_TYPE";
	frm_sr[FRM_SR_INVOCATION_ID] = "SR_INVOCATION_ID";
	frm_sr[FRM_SR_Y_DIRECTION] = "SR_Y_DIRECTION";
	frm_sr[FRM_SR_MACHINE_ID_0] = "SR_MACHINE_ID_0";
	frm_sr[FRM_SR_MACHINE_ID_1] = "SR_MACHINE_ID_1";
	frm_sr[FRM_SR_MACHINE_ID_2] = "SR_MACHINE_ID_2";
	frm_sr[FRM_SR_MACHINE_ID_3] = "SR_MACHINE_ID_3";
	frm_sr[FRM_SR_AFFINITY] = "SR_AFFINITY";
	frm_sr[FRM_SR_Tid] = "SR_Tid";
	frm_sr[FRM_SR_Tid_X] = "SR_Tid_X";
	frm_sr[FRM_SR_Tid_Y] = "SR_Tid_Y";
	frm_sr[FRM_SR_Tid_Z] = "SR_Tid_Z";
	frm_sr[FRM_SR_CTAParam] = "SR_CTAParam";
	frm_sr[FRM_SR_CTAid_X] = "SR_CTAid_X";
	frm_sr[FRM_SR_CTAid_Y] = "SR_CTAid_Y";
	frm_sr[FRM_SR_CTAid_Z] = "SR_CTAid_Z";
	frm_sr[FRM_SR_NTid] = "SR_NTid";
	frm_sr[FRM_SR_NTid_X] = "SR_NTid_X";
	frm_sr[FRM_SR_NTid_Y] = "SR_NTid_Y";
	frm_sr[FRM_SR_NTid_Z] = "SR_NTid_Z";
	frm_sr[FRM_SR_GridParam] = "SR_GridParam";
	frm_sr[FRM_SR_NCTAid_X] = "SR_NCTAid_X";
	frm_sr[FRM_SR_NCTAid_Y] = "SR_NCTAid_Y";
	frm_sr[FRM_SR_NCTAid_Z] = "SR_NCTAid_Z";
	frm_sr[FRM_SR_SWinLo] = "SR_SWinLo";
	frm_sr[FRM_SR_SWINSZ] = "SR_SWINSZ";
	frm_sr[FRM_SR_SMemSz] = "SR_SMemSz";
	frm_sr[FRM_SR_SMemBanks] = "SR_SMemBanks";
	frm_sr[FRM_SR_LWinLo] = "SR_LWinLo";
	frm_sr[FRM_SR_LWINSZ] = "SR_LWINSZ";
	frm_sr[FRM_SR_LMemLoSz] = "SR_LMemLoSz";
	frm_sr[FRM_SR_LMemHiOff] = "SR_LMemHiOff";
	frm_sr[FRM_SR_EqMask] = "SR_EqMask";
	frm_sr[FRM_SR_LtMask] = "SR_LtMask";
	frm_sr[FRM_SR_LeMask] = "SR_LeMask";
	frm_sr[FRM_SR_GtMask] = "SR_GtMask";
	frm_sr[FRM_SR_GeMask] = "SR_GeMask";
	frm_sr[FRM_SR_ClockLo] = "SR_ClockLo";
	frm_sr[FRM_SR_ClockHi] = "SR_ClockHi";
}

void frm_disasm_done()
{

}

void frm_inst_hex_dump(FILE *f, unsigned char *buf, int inst_index)
{
	/* Print offset */
	fprintf(f, "\t/*%04x*/     ", inst_index * 8);

	/* Dump instruction hexadecimal bytes */
	fprintf(f, "/*0x");
	fprintf(f, "%08x", * (unsigned int *) (buf + 8 * inst_index));
	fprintf(f, "%08x", * (unsigned int *) (buf + 8 * inst_index + 4));
	fprintf(f, "*/ \t");
}


/* 
 * Decoder
 */

void frm_inst_decode(struct frm_inst_t *inst)
{
	unsigned int op;

	/* 10-bit opcode by default */
	op = ((inst->dword.bytes[7] & 0xfc) << 2) | ((inst->dword.bytes[0]) & 0xf);
	/* 9-bit opcode */
	if (op == 0x0e0 || op == 0x0f0 || /* FCMP */
		op == 0x0a0 || op == 0x0b0 ||op == 0x080 || op == 0x090 || /* FSETP */
		op == 0x061 || op == 0x071 || /* DSETP */
		op == 0x022 || op == 0x032 || /* IADD32I */
		op == 0x063 || op == 0x073 || /* ISETP */
		op == 0x043 || op == 0x053 || /* ISET */
		op == 0x205 || op == 0x215 || /* LD */
		op == 0x245 || op == 0x255)   /* ST */
		op = ((inst->dword.bytes[7] & 0xf8) << 1) |
			((inst->dword.bytes[0]) & 0xf);

	inst->info = frm_inst_info_long[op];
}


static struct str_map_t frm_inst_ftzfmz_map =
{
	3,
	{
		{"", 0},		
		{ ".FTZ", 1 },
		{ ".FMZ", 2 }
	}
};

static struct str_map_t frm_inst_rnd_map =
{
	4,
	{
		{ "", 0},		
		{ ".RM", 1 },
		{ ".RP", 2 },
		{ ".RZ", 3}
	}
};

static struct str_map_t frm_inst_cmp_map =
{
	14,
	{
		{ ".LT", 1},		
		{ ".EQ", 2},
		{ ".LE", 3},
		{ ".GT", 4},
		{ ".NE", 5},
		{ ".GE", 6},
		{ ".NUM", 7},
		{ ".NAN", 8},
		{ ".LTU", 9},
		{ ".EQU", 10},
		{ ".LEU", 11},
		{ ".GTU", 12},
		{ ".NEU", 13},
		{ ".GEU", 14}
	}
};

static struct str_map_t frm_inst_sat_map =
{
	2,
	{
		{ "", 0},		
		{ ".SAT", 1 }
	}
};

static struct str_map_t frm_inst_logic_map =
{
	3,
	{
		{ ".AND", 0},		
		{ ".OR", 1},
		{ ".XOR", 2}
	}
};


static struct str_map_t frm_inst_op_map =
{
	8,
	{
		{ ".COS", 0},		
		{ ".SIN", 1},
		{ ".EX2", 2},
		{ ".LG2", 3},
		{ ".RCP", 4},
		{ ".RSQ", 5},
		{ ".RCP64H", 6},
		{ ".RSQ64H", 7}
	}
};

static struct str_map_t frm_inst_op56_map =
{
	3,
	{
		{ ".POPC", 0},		
		{ ".AND", 1},
		{ ".OR", 2},
	}
};

static struct str_map_t frm_inst_op67_map =
{
	4,
	{
		{ ".AND", 0},		
		{ ".OR", 1},
		{ ".XOR", 2},
		{ ".PASS_B", 3}
	}
};

static struct str_map_t frm_inst_dtype_map =
{
	4,
	{
		{ "", 0},		
		{ ".F16", 1},
		{ ".F32", 2},
		{ ".F64", 3}
	}
};

/*static struct str_map_t frm_inst_stype_map =
{
	4,
	{
		{ "", 0},		
		{ ".F16", 1},
		{ ".F32", 2},
		{ ".F64", 3}
	}
};*/

static struct str_map_t frm_inst_stype_n_map =
{
	4,
	{
		{ "8", 0},		
		{ "16", 1},
		{ "32", 2},
		{ "64", 3}
	}
};

static struct str_map_t frm_inst_type_map =
{
	7,
	{
		{ ".U8", 0},		
		{ ".S8", 1},
		{ ".U16", 2},
		{ ".S16", 3},
		{ "", 4},
		{ ".64", 5},
		{ ".128", 6}
	}
};

static struct str_map_t frm_inst_cop_map =
{
	4,
	{
		{ "", 0},
		{ ".CG", 1},
		{ ".CS", 2},
		{ ".CV", 3}
	}
};

static struct str_map_t frm_inst_NOP_op_map =
{
	9,
	{
		{ "", 0 },
		{ ".FMA64", 1 },
		{ ".FMA32", 2 },
		{ ".XLU", 3 },
		{ ".ALU", 4 },
		{ ".AGU", 5 },
		{ ".SU", 6 },
		{ ".FU", 7 },
		{ ".FMUL", 8 },
	}
};

static struct str_map_t frm_inst_ccop_map =  // more needs to be added (look over fermi isa NOP)
{
	4,
	{
		{ ".F", 0 },
		{ ".LT", 1 },
		{ ".EQ", 2 },
		{ ".T", 15 }
	}
};

static int inst_is_token(char *fmt_str, char *token_str, int *len_ptr)
{
	int len;

	len = strlen(token_str);
	*len_ptr = len;
	return !strncmp(fmt_str, token_str, len) &&
		!isalnum(fmt_str[len]) && fmt_str[len] != '_';
}


/* 
 * Dump an instrution
 */

void frm_inst_dump(char *str, int size, void *buf, int inst_index)
{
	struct frm_inst_t inst;
	char *fmt_str;
	char *orig_str;
	int len;

	inst.addr = 8 * inst_index;
	memcpy(inst.dword.bytes, buf + 8 * inst_index, 8);
	frm_inst_decode(&inst);
	
	/* Check if instruction is supported */
	if (!inst.info)
		fatal("%s: instruction not supported (offset=0x%x)",
			__FUNCTION__, inst.addr);
	
	/* Store copy of format string and original destination buffer */
	fmt_str = inst.info->fmt_str;
	orig_str = str;

	/* Process format string */
	while (*fmt_str && size)
	{
		/* Character is a literal symbol */
		if (*fmt_str != '%' && *fmt_str != '+')
		{
			if (*fmt_str != ' ' || str != orig_str)
				str_printf(&str, &size, "%c", *fmt_str);
			++fmt_str;
			continue;
		}

		if (*fmt_str == '+')
			++fmt_str;
		
		/* Character is a token */
		++fmt_str;

		/* 1st level token such as pred, dst, src1, src2, src2_mod, imm, offs*/
		if (inst_is_token(fmt_str, "pred", &len))
		{
			unsigned long long int pred;
			pred = inst.dword.general0.pred;
			if (pred < 7)
				str_printf(&str, &size, "@P%lld", pred);
			else if (pred > 7)
				str_printf(&str, &size, "@!P%lld", pred - 8);
		}
		
		else if (inst_is_token(fmt_str, "pred_no@P0", &len))
		{
			unsigned long long int pred;
			pred = inst.dword.general0.pred;
			if (pred < 7 && pred != 0)
				str_printf(&str, &size, "@P%lld", pred);
			else if (pred > 7)
				str_printf(&str, &size, "@!P%lld", pred - 8);
		}
		
		else if (inst_is_token(fmt_str,"dst", &len))
		{
			unsigned long long int dst;
			dst = inst.dword.general0.dst;
			if (dst != 63)
				str_printf(&str, &size, "R%lld", dst);
			else
				str_printf(&str, &size, "RZ");
		}

		else if (inst_is_token(fmt_str,"src1", &len))
		{
			unsigned long long int src1;
			src1 = inst.dword.general0.src1;
			if (src1 != 63)
				str_printf(&str, &size, "R%lld", src1);
			else 	
				str_printf(&str, &size, "RZ");
		}
		
		else if (inst_is_token(fmt_str, "src1_offs", &len))
		{
			unsigned long long int src1;
			unsigned long long int offs;
				
			src1 = inst.dword.general0.src1;
			offs = inst.dword.offs.offset;
			
			if (src1 != 63)
			{
				str_printf(&str, &size, "[R%lld", src1);
			//else 
			//	str_printf(&str, &size, "[RZ");

				if (offs != 0)
					str_printf(&str, &size, "+%#llx", offs);
			}

			else
				str_printf(&str, &size, "[0x%#llx", offs);

			str_printf(&str, &size, "]");
		}

		else if (inst_is_token(fmt_str, "src1_neg", &len))
		{
			unsigned long long int src1;
			unsigned long long int src_mod;
			
			src1 = inst.dword.general0.src1;
			src_mod = inst.dword.mod0_D.fma_mod;
			if (src1 != 63)
			{
				if (src_mod == 0x2)
					str_printf(&str, &size, "-R%lld", src1);
				else
					str_printf(&str, &size, "R%lld", src1);
			}
			else 	
				str_printf(&str, &size, "RZ");
		}

		else if (inst_is_token(fmt_str,"src2", &len)) 
		{
			unsigned long long int bank_id;
			unsigned long long int offset_in_bank;

			if (inst.dword.general0.src2_mod == 0)
			{
				if (inst.dword.general0.src2 != 0x3f)
					str_printf(&str, &size, "R%d", inst.dword.general0.src2 & 0x3f);
				else 
					str_printf(&str, &size, "RZ");
			}
			else if (inst.dword.general0.src2_mod == 1)
			{
				bank_id = inst.dword.general0.src2 >> 16;
				offset_in_bank= inst.dword.general0.src2 & 0xffff;
				if (bank_id == 0 && offset_in_bank == 0)
					str_printf(&str, &size, "c [0x0] [0x0]");
				else if (bank_id == 0 && offset_in_bank != 0)
					str_printf(&str, &size, "c [0x0] [%#llx]", offset_in_bank);
				else if (bank_id != 0 && offset_in_bank == 0)
					str_printf(&str, &size, "c [%#llx] [0x0]", bank_id);
				else
					str_printf(&str, &size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
			}
			else if (inst.dword.general0.src2_mod == 2)
				str_printf(&str, &size, "0x%x", inst.dword.general0.src2);
			else if (inst.dword.general0.src2_mod == 3)
				str_printf(&str, &size, "0x%x", inst.dword.general0.src2);			
		}

		else if (inst_is_token(fmt_str,"src2_neg", &len)) 
		{
			unsigned long long int bank_id;
			unsigned long long int offset_in_bank;

			if (inst.dword.general0.src2_mod == 0)
			{
				if (inst.dword.mod0_A.neg_src2)
					str_printf(&str, &size, "-");
				str_printf(&str, &size, "R%d", inst.dword.general0.src2 & 0x3f);
			}
			else if (inst.dword.general0.src2_mod == 1)
			{
				bank_id = inst.dword.general0.src2 >> 16;
				offset_in_bank= inst.dword.general0.src2 & 0xffff;
				if (bank_id == 0 && offset_in_bank == 0)
					str_printf(&str, &size, "c [0x0] [0x0]");
				else if (bank_id == 0 && offset_in_bank != 0)
					str_printf(&str, &size, "c [0x0] [%#llx]", offset_in_bank);
				else if (bank_id != 0 && offset_in_bank == 0)
					str_printf(&str, &size, "c [%#llx] [0x0]", bank_id);
				else
					str_printf(&str, &size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
			}
			else if (inst.dword.general0.src2_mod == 2)
				str_printf(&str, &size, "0x%x", inst.dword.general0.src2);
			else if (inst.dword.general0.src2_mod == 3)
				str_printf(&str, &size, "0x%x", inst.dword.general0.src2);
		}

		/* This is a special case for src2 and src3. For FFMA,
		   the sequence of output from cuobjdump is somehow depends on the src2_mod  
		   it prints src3 first when src2_mod > 2, however prints src2 first when src2_mod < 2 */
		else if (inst_is_token(fmt_str,"src2_FFMA", &len)) 
		{
			unsigned long long int bank_id;
			unsigned long long int offset_in_bank;
			unsigned long long int src3;

			bank_id = inst.dword.general0.src2 >> 16;
			offset_in_bank= inst.dword.general0.src2 & 0xffff;
			src3 = inst.dword.general0_mod1_B.src3;

			/* print out src2 */
			if (inst.dword.general0.src2_mod < 2)
			{
				if (inst.dword.general0.src2_mod == 0)
					str_printf(&str, &size, "R%d", inst.dword.general0.src2 & 0x3f);
				else if (inst.dword.general0.src2_mod == 1)
				{
					if (bank_id == 0 && offset_in_bank == 0)
						str_printf(&str, &size, "c [0x0] [0x0]");
					else if (bank_id == 0 && offset_in_bank != 0)
						str_printf(&str, &size, "c [0x0] [%#llx]", offset_in_bank);
					else if (bank_id != 0 && offset_in_bank == 0)
						str_printf(&str, &size, "c [%#llx] [0x0]", bank_id);
					else
						str_printf(&str, &size, "c [%#llx] [%#llx]", bank_id, offset_in_bank);
				}
			}

			/* print out src3 */
			else
			{
				if (src3 != 63)			
					str_printf(&str, &size, ", R%lld", src3);
				else 
					str_printf(&str, &size, ", RZ");
			}
		}
		
		else if (inst_is_token(fmt_str, "src3_FFMA", &len))
		{
			unsigned long long int src3;
			unsigned long long int bit26;
			unsigned long long int bit28;
			
			src3 = inst.dword.general0_mod1_B.src3;
			bit26 = inst.dword.general0.src2 & 0x1;
			bit28 = inst.dword.general0.src2 >> 2 &0x1;
		
			/* print out src2 */
			if (inst.dword.general0.src2_mod < 2)
			{
				if (src3 != 63)			
					str_printf(&str, &size, "R%lld", src3);
				else 
					str_printf(&str, &size, "RZ");
			}

			else
			{
				/* FIXME : we need to figure out how bit26 and bit28 control src2*/
				if (bit26 == 0 && bit28 == 0)
					str_printf(&str, &size, "c [0x0] [0x0]");
				else if (bit26 == 1 && bit28 == 0)
					str_printf(&str, &size, "c [0x10] [0x0]");
				else if (bit26 == 0 && bit28 == 1)
					str_printf(&str, &size, "c [0x0] [0x4]");
				else
					str_printf(&str, &size, "c [0x10] [0x4]");
			}
		}
		
		else if (inst_is_token(fmt_str,"src2_frm_sr", &len))
		{
			char* sreg;
			sreg = frm_sr[inst.dword.general0.src2 & 0xff];
			str_printf (&str, &size, "%s", sreg);

		}

		else if (inst_is_token(fmt_str,"tgt", &len))
		{
			unsigned long long int target;

			target = inst.dword.tgt.target;	
			target = SEXT64(target, 24);
			target += inst.addr + 8;
			str_printf (&str,&size, "%#llx", target);
		}

		else if (inst_is_token(fmt_str,"tgt_noinc", &len))
		{
			if (!inst.dword.tgt.noinc)
				str_printf(&str,&size, ".noinc");
		}

		else if (inst_is_token(fmt_str,"tgt_u", &len))
		{
			if (inst.dword.tgt.u)
				str_printf(&str,&size, ".U");
		}

		else if (inst_is_token(fmt_str,"tgt_lmt", &len))
		{
			if (inst.dword.tgt.noinc)
				str_printf(&str,&size, ".LMT");
		}

		else if (inst_is_token(fmt_str,"imm32", &len))
		{
			long long int imm32;
			/* FIXME
			 * Careful - We need to check whether each instruction encodes the immediate
			 * value as a signed or unsigned number. */
				imm32 = (signed)inst.dword.imm.imm32;
			/* Print sign */
			if (imm32 < 0)
			{
				str_printf(&str, &size, "-");
				imm32 = -imm32;
			}
			str_printf(&str, &size, "%#llx", imm32);
		}

		else if (inst_is_token(fmt_str,"offs", &len))
		{
			unsigned long long int offs;
			offs = inst.dword.offs.offset;
			if (offs)
				str_printf(&str, &size, "+%#llx", offs);
		}

		else if (inst_is_token(fmt_str,"gen1_cmp", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_cmp_map,
						inst.dword.general1.cmp));
		}

		else if (inst_is_token(fmt_str,"gen1_logic", &len)) 
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_logic_map,
						inst.dword.general1.logic));
		}

		else if (inst_is_token(fmt_str,"gen1_logicftz", &len)) 
		{
			str_printf(&str, &size, ".FTZ%s", str_map_value(&frm_inst_logic_map,
						inst.dword.general1.logic));
		}

		else if (inst_is_token(fmt_str,"R", &len))
		{	
			unsigned long long int R;
			R = inst.dword.general1.R;
			if (R < 7)
				str_printf(&str, &size, "P%lld", R);
			else if (R == 8)
				str_printf(&str, &size, "!P0");
			else
				str_printf(&str, &size, "pt");
		}

		/* 2nd level token such as mod0, mod1, P, Q*/
		else if (inst_is_token(fmt_str,"mod0_A_ftz", &len))
		{
			if (inst.dword.mod0_A.satftz)
				str_printf(&str, &size, ".FTZ");
		}
		
		else if (inst_is_token(fmt_str, "mod0_A_s", &len))
		{
			if (inst.dword.mod0_A.s)
				str_printf(&str, &size, ".S");
		}

		else if (inst_is_token(fmt_str, "mod0_A_redarv", &len))
		{
			if (inst.dword.mod0_A.abs_src1)
				str_printf(&str, &size, ".ARV");
			else 
				str_printf(&str, &size, ".RED");
		}
	
		else if (inst_is_token(fmt_str, "mod0_A_u32", &len))
		{
			if (!inst.dword.mod0_A.satftz)
				str_printf(&str, &size, ".U32");
		}

		else if (inst_is_token(fmt_str,"mod0_A_w", &len))
		{
			if (inst.dword.mod0_A.neg_src1)
				str_printf(&str, &size, ".W");
		}

		else if (inst_is_token(fmt_str,"mod0_A_op", &len))
		{
			unsigned long long int op;
			op = inst.dword.mod0_A.abs_src2 << 1 || inst.dword.mod0_A.satftz;
			str_printf(&str, &size, "%s", str_map_value (&frm_inst_op56_map,op));
		}

		else if (inst_is_token(fmt_str,"mod0_B_u32", &len))
		{
			if (!inst.dword.mod0_B.type)
				str_printf(&str, &size, ".U32");
		}

		else if (inst_is_token(fmt_str,"mod0_B_brev", &len))
		{
			if (inst.dword.mod0_B.cop)	
				str_printf(&str, &size, ".brev");
		}	

		else if (inst_is_token(fmt_str,"mod0_B_cop", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_cop_map, inst.dword.mod0_B.cop));
		}

		else if (inst_is_token(fmt_str,"mod0_B_type", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_type_map, inst.dword.mod0_B.type));
		}

		else if (inst_is_token(fmt_str,"mod0_C_s", &len))
		{
			if (inst.dword.mod0_C.s)
				str_printf(&str, &size, ".S");
		}

		else if (inst_is_token(fmt_str,"mod0_C_shamt", &len))
		{
			unsigned long long int shamt;
			shamt = inst.dword.mod0_C.shamt;
			str_printf(&str, &size, "%#llx", shamt);
		}

		else if (inst_is_token(fmt_str,"mod0_C_ccop", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_ccop_map, inst.dword.mod0_C.shamt));
		}		

		else if (inst_is_token(fmt_str,"mod0_D_ftzfmz", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_ftzfmz_map, inst.dword.mod0_D.ftzfmz));
		}

		else if (inst_is_token(fmt_str,"mod0_D_sat", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_sat_map, inst.dword.mod0_D.sat));
		}

		else if (inst_is_token(fmt_str,"mod0_D_op", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_op_map, inst.dword.mod0_D.ftzfmz));
		}

		else if (inst_is_token(fmt_str,"mod0_D_op67", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_op67_map, inst.dword.mod0_D.ftzfmz));
		}		
		
		else if (inst_is_token(fmt_str,"gen0_mod1_B_rnd", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_rnd_map, inst.dword.general0_mod1_B.rnd));
		}

		else if (inst_is_token(fmt_str,"gen0_mod1_D_cmp", &len))
		{
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_cmp_map, inst.dword.general0_mod1_D.cmp));
		}

		else if (inst_is_token(fmt_str,"gen0_src1_dtype", &len))
		{
			unsigned long long int dtype;
			dtype = inst.dword.general0.src1 & 0x3;
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_dtype_map, dtype));
		}

		else if (inst_is_token(fmt_str,"gen0_src1_dtype_n", &len))
		{
			unsigned long long int dtype_n;
			dtype_n = inst.dword.general0.src1 & 0x3;
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_stype_n_map, dtype_n));
		}
		
		else if (inst_is_token(fmt_str,"gen0_src1_stype_n", &len))
		{
			unsigned long long int stype_n;
			stype_n = inst.dword.general0.src1 >> 3 & 0x3;
			str_printf(&str, &size, "%s", str_map_value (&frm_inst_stype_n_map, stype_n));
		}

		else if (inst_is_token(fmt_str,"stype_sn", &len))
		{
			unsigned long long int stype_n;
			if (inst.dword.mod0_A.neg_src1)
				str_printf(&str, &size, ".S");
		        else
			        str_printf(&str, &size, ".U");
			stype_n = inst.dword.general0.src1 >> 3 & 0x3;
				str_printf(&str, &size, "%s", str_map_value (&frm_inst_stype_n_map, stype_n));
		}	

		else if (inst_is_token(fmt_str,"dtype_sn", &len))
		{
			unsigned long long int dtype_n;
			if (inst.dword.mod0_A.abs_src1)
				str_printf(&str, &size, ".S");
			else
				str_printf(&str, &size, ".U");
			dtype_n = inst.dword.general0.src1 & 0x3;
			str_printf(&str, &size, "%s", str_map_value (&frm_inst_stype_n_map, dtype_n));
		}

		else if (inst_is_token(fmt_str,"offs_op1_e", &len))
		{
			unsigned long long int e;
			e = inst.dword.offs.op1 & 0x1;	
			if (e)
				str_printf(&str, &size, ".e");
		}

		else if (inst_is_token(fmt_str,"P", &len))
		{
			unsigned long long int P;
			P = inst.dword.general1.dst >> 3;	
			str_printf(&str, &size, "P%lld", P);
		}

		else if (inst_is_token(fmt_str,"Q", &len))
		{
			unsigned long long int Q;
			Q = inst.dword.general1.dst & 0x7;
			if (Q != 7)
				str_printf(&str, &size, "p%lld", Q);
			else
				str_printf(&str, &size, "pt");
		}

		else if (inst_is_token(fmt_str,"src3", &len))
		{
			unsigned long long int src3;
			src3 = inst.dword.general0_mod1_B.src3;
			if (src3 != 63)			
				str_printf(&str, &size, "R%lld", src3);
			else 
				str_printf(&str, &size, "RZ");
		}

		else if (inst_is_token(fmt_str,"FADD_sat", &len))
		{
			unsigned long long int sat;
			sat = inst.dword.general0_mod1_B.src3 & 0x1;
			if (sat)
				str_printf(&str, &size, ".SAT");
		}

		else if (inst_is_token(fmt_str,"MUFU_op", &len))
		{
			unsigned long long int op;
			op = inst.dword.imm.imm32;
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_op_map, op));	
		}

		else if (inst_is_token(fmt_str,"NOP_op", &len))
		{
			unsigned long long int op;
			op = inst.dword.offs.mod1 >> 9 & 0x4;
			str_printf(&str, &size, "%s", str_map_value(&frm_inst_NOP_op_map, op));	
		}

		else if (inst_is_token(fmt_str,"mod0_D_x", &len))
		{
			unsigned long long int x;
			x = inst.dword.mod0_D.ftzfmz & 0x1;
			if (x)
				str_printf(&str, &size, ".X");
		}

		else if (inst_is_token(fmt_str,"IMAD_mod1", &len))
		{
			unsigned long long int mod1;
			mod1 = inst.dword.mod0_D.ftzfmz >> 1;
			if (!mod1)
				str_printf(&str, &size, ".U32");
		}

		else if (inst_is_token(fmt_str,"IMAD_mod2", &len))
		{
			unsigned long long int mod2;
			mod2 = inst.dword.mod0_D.sat;
			if (!mod2)
				str_printf(&str, &size, ".U32");
		}

		else if (inst_is_token(fmt_str,"IMAD_hi", &len))
		{
			unsigned long long int hi;
			hi= inst.dword.mod0_D.ftzfmz & 0x1;
			if (hi)
				str_printf(&str, &size, ".HI");
		}

		else if (inst_is_token(fmt_str,"IMAD_sat", &len))
		{
			unsigned long long int sat;
			sat = inst.dword.general0_mod1_B.rnd >> 1;
			if (sat)
				str_printf(&str, &size, ".SAT");
		}
		else
		{
			fatal("%s: unknown token: %s", __FUNCTION__,
					fmt_str);
		}
		
		/* Skip processed token */
		fmt_str += len;
	}
}


void frm_disasm(char *path)
{
	struct elf_file_t *elf_file;
	struct elf_section_t *section;
	int inst_index;
	char inst_str[MAX_STRING_SIZE];
	int i;

	/* Initialization */
	frm_disasm_init();

	/* Load cubin file */
	elf_file = elf_file_create_from_path(path);
	printf("\n\tcode for sm_20\n");

	for (i = 0; i < list_count(elf_file->section_list); ++i)
	{
		section = (struct elf_section_t *)list_get(elf_file->section_list, i);

		/* Determine if section is .text.kernel_name */
		if (!strncmp(section->name, ".text.", 6))
		{
			/* Decode and dump instructions */
			printf("\t\tFunction : %s\n", section->name + 6);
			for (inst_index = 0; inst_index < section->buffer.size/8; ++inst_index)
			{
				frm_inst_hex_dump(stdout, (unsigned char*)(section->buffer.ptr), inst_index);
				frm_inst_dump(inst_str, sizeof inst_str, section->buffer.ptr, inst_index);
				printf("%s;\n", inst_str);
			}
			printf("\t\t....................................................\n\n\n");
		}
		if (!strncmp(section->name, ".rodata", 7))
		{
			FILE *fp = fopen(".rodata", "wb");
			fwrite(section->buffer.ptr, 1, section->buffer.size, fp);
			fclose(fp);
		}
	}

	/* Free external ELF */
	elf_file_free(elf_file);

        
	/*End */
        mhandle_done();
        exit(0);
}

void frm_disasm_text_section_buffer(struct elf_enc_buffer_t *buffer)
{
	int inst_index;
	char inst_str[MAX_STRING_SIZE];

	for (inst_index = 0; inst_index < buffer->size/8; ++inst_index)
	{
		frm_inst_hex_dump(stdout, (unsigned char*)(buffer->ptr),
			inst_index);
		frm_inst_dump(inst_str, sizeof inst_str, buffer->ptr,
			inst_index);
		printf("%s;\n", inst_str);
	}
}

