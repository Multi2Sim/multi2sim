/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#if 0

#include <ctype.h>
#include <string.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "asm.h"
#include "inst.h"



/* 
 * Functions to print assembly output to file
 */

static int is_token(char *fmt_str, char *token_str, int *token_len)
{
	*token_len = strlen(token_str);
	return !strncmp(fmt_str, token_str, *token_len) &&
		!isalnum(fmt_str[*token_len]);
}


void operand_dump(char *str, int operand)
{
	/* Assume operand in range. */
	assert(operand >= 0 && operand <= 511);

	int str_size = MAX_STRING_SIZE;
	char *pstr = str;

	if (operand <= 103)
	{
		/* SGPR */
		str_printf(&pstr, &str_size, "s%d", operand);
	}
	else if (operand <= 127)
	{
		/* sdst special registers */
		str_printf(&pstr, &str_size, "%s", 
			str_map_value(&si_inst_sdst_map, operand - 104));
	}
	else if (operand <= 192)
	{
		/* Positive integer constant */
		str_printf(&pstr, &str_size, "%d", operand - 128);
	}
	else if (operand <= 208)
	{
		/* Negative integer constant */
		str_printf(&pstr, &str_size, "-%d", operand - 192);
	}
	else if (operand <= 239)
	{
		fatal("Operand code unused.");
	}
	else if (operand <= 255)
	{
		str_printf(&pstr, &str_size, "%s", 
			str_map_value(&si_inst_ssrc_map, operand - 240));
	}
	else if (operand <= 511)
	{
		/* VGPR */
		str_printf(&pstr, &str_size, "v%d", operand - 256);
	}
}

void operand_dump_series(char *str, int operand, int operand_end)
{
	assert(operand <= operand_end);
	if (operand == operand_end)
	{
		operand_dump(str, operand);
		return;
	}

	int str_size = MAX_STRING_SIZE;
	char *pstr = str;

	if (operand <= 103)
	{
		str_printf(&pstr, &str_size, "s[%d:%d]", operand, operand_end);
	}
	else if (operand <= 245)
	{
		if (operand >= 112 && operand <= 123)
		{
			assert(operand_end <= 123);
			str_printf(&pstr, &str_size, "ttmp[%d:%d]", 
				operand - 112, operand_end - 112);
		}
		else
		{
			assert(operand_end == operand + 1);
			switch (operand)
			{
				case 106:
					str_printf(&pstr, &str_size, "vcc");
					break;
				case 108:
					str_printf(&pstr, &str_size, "tba");
					break;
				case 110:
					str_printf(&pstr, &str_size, "tma");
					break;
				case 126:
					str_printf(&pstr, &str_size, "exec");
					break;
				case 128:
					str_printf(&pstr, &str_size, "0");
					break;
				case 131:
					str_printf(&pstr, &str_size, "3");
					break;
				case 208: 
					str_printf(&pstr, &str_size, "-16");
					break;
				case 240:
					str_printf(&pstr, &str_size, "0.5");
					break;
				case 242:
					str_printf(&pstr, &str_size, "1.0");
					break;
				case 243:
					str_printf(&pstr, &str_size, "-1.0");
					break;
				case 244:
					str_printf(&pstr, &str_size, "2.0");
					break;
				case 245:
					str_printf(&pstr, &str_size, "-2.0");
					break;
				default:
					fatal("Unimplemented operand series: "
						"[%d:%d]", operand, 
						operand_end);
			}
		}
	}
	else if (operand <= 255)
	{
		fatal("Illegal operand series: [%d:%d]", operand, operand_end);
	}
	else if (operand <= 511)
	{
		str_printf(&pstr, &str_size, "v[%d:%d]", operand - 256, 
			operand_end - 256);
	}
}

void operand_dump_scalar(char *str, int operand)
{
	operand_dump(str, operand);
}

void operand_dump_series_scalar(char *str, int operand, int operand_end)
{
	operand_dump_series(str, operand, operand_end);
}

void operand_dump_vector(char *str, int operand)
{
	operand_dump(str, operand + 256);
}

void operand_dump_series_vector(char *str, int operand, int operand_end)
{
	operand_dump_series(str, operand + 256, operand_end + 256);
}

void operand_dump_exp(char *str, int operand)
{
	/* Assume operand in range. */
	assert(operand >= 0 && operand <= 63);

	int str_size = MAX_STRING_SIZE;
	char *pstr = str;

	if (operand <= 7)
	{
		/* EXP_MRT */
		str_printf(&pstr, &str_size, "exp_mrt_%d", operand);
	}
	else if (operand == 8)
	{
		/* EXP_Z */
		str_printf(&pstr, &str_size, "exp_mrtz");
	}
	else if (operand == 9)
	{
		/* EXP_NULL */
		str_printf(&pstr, &str_size, "exp_null");
	}
	else if (operand < 12)
	{
		fatal("Operand code [%d] unused.", operand);
	}
	else if (operand <= 15)
	{
		/* EXP_POS */
		str_printf(&pstr, &str_size, "exp_pos_%d", operand - 12);
	}
	else if (operand < 32)
	{
		fatal("Operand code [%d] unused.", operand);
	}
	else if (operand <= 63)
	{
		/* EXP_PARAM */
		str_printf(&pstr, &str_size, "exp_prm_%d", operand - 32);
	}
}

void line_dump(char *inst_str, unsigned int rel_addr, void *buf, char *line, int line_size, int inst_size)
{
	int dat_str_size = MAX_STRING_SIZE;
	char inst_dat_str[MAX_STRING_SIZE];
	char *dat_str = &inst_dat_str[0];

	if (inst_size == 4)
	{
		str_printf(&dat_str, &dat_str_size, "// %08X: %08X", rel_addr,
			((unsigned int*)buf)[0]);
	}
	else
	{
		str_printf(&dat_str, &dat_str_size, "// %08X: %08X %08X", 
			rel_addr, ((unsigned int*)buf)[0], 
			((unsigned int*)buf)[1]);
	}

	if (strlen(inst_str) < 59)
	{
		str_printf(&line, &line_size, "%-59s%s\n", inst_str, 
			inst_dat_str);
	}
	else
	{
		str_printf(&line, &line_size, "%s %s\n", inst_str, 
			inst_dat_str);
	}
}



void SIInstCreate(SIInst *self, SIAsm *as)
{
	/* Initialize */
	self->as = as;
}


void SIInstDestroy(SIInst *self)
{
}


void SIInstDump_SSRC(SIInst *inst, unsigned int ssrc, 
	char *operand_str, char **inst_str, int str_size)
{
	if (ssrc == 0xFF)
	{
		str_printf(inst_str, &str_size, "0x%08x", 
			inst->bytes.sop2.lit_cnst);
	}
	else
	{
		operand_dump_scalar(operand_str, ssrc);
		str_printf(inst_str, &str_size, "%s", operand_str);
	}
}

void SIInstDump_64_SSRC(SIInst *inst, unsigned int ssrc, 
	char *operand_str, char **inst_str, int str_size)
{		
	if (ssrc == 0xFF)
	{
		str_printf(inst_str, &str_size, "0x%08x", 
			inst->bytes.sop2.lit_cnst);
	}
	else
	{
		operand_dump_series_scalar(operand_str, ssrc, ssrc + 1);
		str_printf(inst_str, &str_size, "%s", operand_str);
	}
}

void SIInstDump_VOP3_SRC(SIInst *inst, unsigned int src, int neg, 
	char *operand_str, char **inst_str, int str_size)
{
	operand_dump(operand_str, src);

	if (!(IN_RANGE(inst->bytes.vop3a.op, 293, 298)) && 
		!(IN_RANGE(inst->bytes.vop3a.op, 365, 366)))
	{
		if ((inst->bytes.vop3a.neg & neg) && 
			(inst->bytes.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "-abs(%s)", 
				operand_str);
		}
		else if ((inst->bytes.vop3a.neg & neg) && 
			!(inst->bytes.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "-%s", operand_str);
		}
		else if (!(inst->bytes.vop3a.neg & neg) && 
			(inst->bytes.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "abs(%s)", operand_str);
		}
		else if (!(inst->bytes.vop3a.neg & neg) && 
			!(inst->bytes.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "%s", operand_str);
		}
	}
	else
	{
		if (inst->bytes.vop3a.neg & neg)
		{
			str_printf(inst_str, &str_size, "-%s", operand_str);
		}
		else if (!(inst->bytes.vop3a.neg & neg))
		{
			str_printf(inst_str, &str_size, "%s", operand_str);
		}
	}
}

void SIInstDump_VOP3_64_SRC(SIInst *inst, unsigned int src, int neg, char *operand_str, char **inst_str, int str_size)
{
	operand_dump_series(operand_str, src, src + 1);
	
	if (!(IN_RANGE(inst->bytes.vop3a.op, 293, 298)) && 
		!(IN_RANGE(inst->bytes.vop3a.op, 365, 366)))
	{
		if ((inst->bytes.vop3a.neg & neg) && 
			(inst->bytes.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "-abs(%s)", 
				operand_str);
		}
		else if ((inst->bytes.vop3a.neg & neg) && 
			!(inst->bytes.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "-%s", operand_str);
		}
		else if (!(inst->bytes.vop3a.neg & neg) && 
			(inst->bytes.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "abs(%s)", operand_str);
		}
		else if (!(inst->bytes.vop3a.neg & neg) && 
			!(inst->bytes.vop3a.abs & neg))
		{
			str_printf(inst_str, &str_size, "%s", operand_str);
		}
	}
	else
	{
		if (inst->bytes.vop3a.neg & neg)
		{
			str_printf(inst_str, &str_size, "-%s", operand_str);
		}
		else if (!(inst->bytes.vop3a.neg & neg))
		{
			str_printf(inst_str, &str_size, "%s", operand_str);
		}
	}
}
void SIInstDump_SERIES_VDATA(unsigned int vdata, int op, char *operand_str, 
	char **inst_str, int str_size)
{
	int vdata_end;

	switch (op)
	{
		case 0:
		case 4:
		case 9:
		case 12:
		case 24:
		case 28:
		case 50:
			vdata_end = vdata + 0;
			break;
		case 1:
		case 5:
			vdata_end = vdata + 1;
			break;
		case 2:
		case 6:
			vdata_end = vdata + 2;
			break;
		case 3:
		case 7:
			vdata_end = vdata + 3;
			break;
		default:
			fatal("MUBUF/MTBUF opcode not recognized");
	}

	operand_dump_series_vector(operand_str, vdata, vdata_end);
	str_printf(inst_str, &str_size, "%s", operand_str);
}

void SIInstDump_MADDR(SIInst *inst, char *operand_str, 
	char **inst_str, int str_size)
{
	/* soffset */
	assert(inst->bytes.mtbuf.soffset <= 103 ||
		inst->bytes.mtbuf.soffset == 124 ||
		(inst->bytes.mtbuf.soffset >= 128 && 
		inst->bytes.mtbuf.soffset <= 208));
	operand_dump_scalar(operand_str, inst->bytes.mtbuf.soffset);
	str_printf(inst_str, &str_size, "%s", operand_str);

	/* offen */
	if (inst->bytes.mtbuf.offen)
		str_printf(inst_str, &str_size, " offen");

	/* index */
	if (inst->bytes.mtbuf.idxen)
		str_printf(inst_str, &str_size, " idxen");

	/* offset */
	if (inst->bytes.mtbuf.offset)
		str_printf(inst_str, &str_size, " offset:%d", 
			inst->bytes.mtbuf.offset);
}

void SIInstDump_DUG(SIInst *inst, char *operand_str, 
	char **inst_str, int str_size)
{
	/* DMASK */
	str_printf(inst_str, &str_size, " dmask:0x%01x", 
		inst->bytes.mimg.dmask);
	
	/* UNORM */
	if (inst->bytes.mimg.unorm)
		str_printf(inst_str, &str_size, " unorm");
	
	/* GLC */
	if (inst->bytes.mimg.glc)
		str_printf(inst_str, &str_size, " glc");
}

void SIInstDump(SIInst *self, unsigned int rel_addr, void *buf,
		char *line, int line_size)
{
	int str_size = MAX_STRING_SIZE;
	int token_len;
	
	char orig_inst_str[MAX_STRING_SIZE];
	char orig_operand_str[MAX_STRING_SIZE];
	
	char *operand_str = &orig_operand_str[0];
	char *inst_str = &orig_inst_str[0];
	char *fmt_str = self->info->fmt_str;

	while (*fmt_str)
	{
		/* Literal */
		if (*fmt_str != '%')
		{
			str_printf(&inst_str, &str_size, "%c", *fmt_str);
			fmt_str++;
			continue;
		}

		/* Token */
		fmt_str++;
		if (is_token(fmt_str, "WAIT_CNT", &token_len))
		{	
			SIInstBytesSOPP *sopp = &self->bytes.sopp;

			unsigned int and = 0;
			int vm_cnt = (sopp->simm16 & 0xF);

			if (vm_cnt != 0xF)
			{
				str_printf(&inst_str, &str_size, "vmcnt(%d)",
					vm_cnt);
				and = 1;
			}

			int lgkm_cnt = (sopp->simm16 & 0x1f00) >> 8;

			if (lgkm_cnt != 0x1f)
			{
				if (and)
				{
					str_printf(&inst_str, &str_size, " & ");
				}

				str_printf(&inst_str, &str_size, "lgkmcnt(%d)", 
					lgkm_cnt);
				and = 1;
			}

			int exp_cnt = (sopp->simm16 & 0x70) >> 4;

			if (exp_cnt != 0x7)
			{
				if (and)
				{
					str_printf(&inst_str, &str_size, " & ");
				}

				str_printf(&inst_str, &str_size, "expcnt(%d)",
					exp_cnt);
				and = 1;
			}
		}
		else if (is_token(fmt_str, "LABEL", &token_len))
		{		
			SIInstBytesSOPP *sopp = &self->bytes.sopp;
	
			short simm16 = sopp->simm16;
			int se_simm = simm16;

			str_printf(&inst_str, &str_size, "label_%04X", 
				(rel_addr + (se_simm * 4) + 4) / 4);
		}
		else if (is_token(fmt_str, "SSRC0", &token_len))
		{	
			SIInstDump_SSRC(self, self->bytes.sop2.ssrc0, 
				operand_str, &inst_str, str_size);
		}
		else if (is_token(fmt_str, "64_SSRC0", &token_len))
		{
			SIInstDump_64_SSRC(self, self->bytes.sop2.ssrc0, 
				operand_str, &inst_str, str_size);
		}
		else if (is_token(fmt_str, "SSRC1", &token_len))
		{
			SIInstDump_SSRC(self, self->bytes.sop2.ssrc1, 
				operand_str, &inst_str, str_size);
		}
		else if (is_token(fmt_str, "64_SSRC1", &token_len))
		{
			SIInstDump_64_SSRC(self, self->bytes.sop2.ssrc1, 
				operand_str, &inst_str, str_size);
		}
		else if (is_token(fmt_str, "SDST", &token_len))
		{	
			operand_dump_scalar(operand_str, 
				self->bytes.sop2.sdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "64_SDST", &token_len))
		{
			operand_dump_series_scalar(operand_str, 
				self->bytes.sop2.sdst, 
				self->bytes.sop2.sdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SIMM16", &token_len))
		{
			str_printf(&inst_str, &str_size, "0x%04x", 
				self->bytes.sopk.simm16);
		}
		else if (is_token(fmt_str, "SRC0", &token_len))
		{
			if (self->bytes.vopc.src0 == 0xFF)
			{
				str_printf(&inst_str, &str_size, "0x%08x", 
					self->bytes.vopc.lit_cnst);
			}
			else
			{
				operand_dump(operand_str, 
					self->bytes.vopc.src0);
				str_printf(&inst_str, &str_size, "%s", 
					operand_str);
			}
		}
		else if (is_token(fmt_str, "64_SRC0", &token_len))
		{
			assert(self->bytes.vopc.src0 != 0xFF);

			operand_dump_series(operand_str, 
				self->bytes.vopc.src0, 
				self->bytes.vopc.src0 + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VSRC1", &token_len))
		{
			operand_dump_vector(operand_str, 
				self->bytes.vopc.vsrc1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "64_VSRC1", &token_len))
		{
			assert(self->bytes.vopc.vsrc1 != 0xFF);

			operand_dump_series_vector(operand_str, 
				self->bytes.vopc.vsrc1, 
				self->bytes.vopc.vsrc1 + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VDST", &token_len))
		{
			operand_dump_vector(operand_str, 
				self->bytes.vop1.vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "64_VDST", &token_len))
		{
			operand_dump_series_vector(operand_str, 
				self->bytes.vop1.vdst, 
				self->bytes.vop1.vdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SVDST", &token_len))
		{
			operand_dump_scalar(operand_str, 
				self->bytes.vop1.vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VOP3_64_SVDST", &token_len))
		{
			/* VOP3a compare operations use the VDST field to 
			 * indicate the address of the scalar destination.*/
			operand_dump_series_scalar(operand_str, 
				self->bytes.vop3a.vdst, 
				self->bytes.vop3a.vdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VOP3_VDST", &token_len))
		{
			operand_dump_vector(operand_str, 
				self->bytes.vop3a.vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VOP3_64_VDST", &token_len))
		{
			operand_dump_series_vector(operand_str, 
				self->bytes.vop3a.vdst, 
				self->bytes.vop3a.vdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VOP3_64_SDST", &token_len))
		{
			operand_dump_series_scalar(operand_str, 
				self->bytes.vop3b.sdst, 
				self->bytes.vop3b.sdst + 1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VOP3_SRC0", &token_len))
		{
			SIInstDump_VOP3_SRC(self, 
				self->bytes.vop3a.src0, 1, operand_str, 
				&inst_str, str_size);
		}
		else if (is_token(fmt_str, "VOP3_64_SRC0", &token_len))
		{
			SIInstDump_VOP3_64_SRC(self, 
				self->bytes.vop3a.src0, 1, operand_str, 
				&inst_str, str_size);
		}
		else if (is_token(fmt_str, "VOP3_SRC1", &token_len))
		{
			SIInstDump_VOP3_SRC(self, 
				self->bytes.vop3a.src1, 2, operand_str, 
				&inst_str, str_size);
		}
		else if (is_token(fmt_str, "VOP3_64_SRC1", &token_len))
		{
			SIInstDump_VOP3_64_SRC(self, 
				self->bytes.vop3a.src1, 2, operand_str, 
				&inst_str, str_size);
		}
		else if (is_token(fmt_str, "VOP3_SRC2", &token_len))
		{
			SIInstDump_VOP3_SRC(self, self->bytes.vop3a.src2, 
				4, operand_str, &inst_str, str_size);
		}
		else if (is_token(fmt_str, "VOP3_64_SRC2", &token_len))
		{
			SIInstDump_VOP3_64_SRC(self, 
				self->bytes.vop3a.src2, 4, operand_str, 
				&inst_str, str_size);
		}
		else if (is_token(fmt_str, "VOP3_OP16", &token_len))
		{
			str_printf(&inst_str, &str_size, "%s", 
				str_map_value(&si_inst_OP16_map, 
					(self->bytes.vop3a.op & 15)));
		}
		else if (is_token(fmt_str, "VOP3_OP8", &token_len))
		{
			str_printf(&inst_str, &str_size, "%s", 
				str_map_value(&si_inst_OP8_map, 
					(self->bytes.vop3a.op & 15)));
		}
		else if (is_token(fmt_str, "SMRD_SDST", &token_len))
		{
			operand_dump_scalar(operand_str, 
				self->bytes.smrd.sdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "SERIES_SDST", &token_len))
		{
			
			/* The sbase field is missing the LSB, 
			 * so multiply by 2 */
			int sdst = self->bytes.smrd.sdst;
			int sdst_end;
			int op = self->bytes.smrd.op;

			/* S_LOAD_DWORD */
			if (IN_RANGE(op, 0, 4))
			{
				if (op != 0)
				{
					/* Multi-dword */
					switch (op)
					{
						case 1:
							sdst_end = sdst + 1;
							break;
						case 2:
							sdst_end = sdst + 3;
							break;
						case 3:
							sdst_end = sdst + 7;
							break;
						case 4:
							sdst_end = sdst + 15;
							break;
						default:
							fatal("Invalid smrd "
								"opcode");
					}
				}
			}
			/* S_BUFFER_LOAD_DWORD */
			else if (IN_RANGE(op, 8, 12))
			{	
				if (op != 8)
				{
					/* Multi-dword */
					switch (op)
					{
						case 9:
							sdst_end = sdst + 1;
							break;
						case 10:
							sdst_end = sdst + 3;
							break;
						case 11:
							sdst_end = sdst + 7;
							break;
						case 12:
							sdst_end = sdst + 15;
							break;
						default:
							fatal("Invalid smrd "
								"opcode");
					}
				}
			}
			/* S_MEMTIME */
			else if (op == 30)
			{
				fatal("S_MEMTIME instruction not currently" 
					"supported");
			}
			/* S_DCACHE_INV */
			else if (op == 31)
			{
				fatal("S_DCACHE_INV instruction not" 
					"currently supported");
			}
			else
			{
				fatal("Invalid smrd opcode");
			}

			operand_dump_series_scalar(operand_str, sdst, sdst_end);
			str_printf(&inst_str, &str_size, "%s", operand_str);

		}
		else if (is_token(fmt_str, "SERIES_SBASE", &token_len))
		{
			
			/* The sbase field is missing the LSB, 
			 * so multiply by 2 */
			int sbase = self->bytes.smrd.sbase * 2;
			int sbase_end;
			int op = self->bytes.smrd.op;

			/* S_LOAD_DWORD */
			if (IN_RANGE(op, 0, 4))
			{
				/* SBASE specifies two consecutive SGPRs */
				sbase_end = sbase + 1;
			}
			/* S_BUFFER_LOAD_DWORD */
			else if (IN_RANGE(op, 8, 12))
			{
				/* SBASE specifies four consecutive SGPRs */
				sbase_end = sbase + 3;
			}
			/* S_MEMTIME */
			else if (op == 30)
			{
				fatal("S_MEMTIME instruction not currently"
					" supported");
			}
			/* S_DCACHE_INV */
			else if (op == 31)
			{
				fatal("S_DCACHE_INV instruction not currently"
					" supported");
			}
			else
			{
				fatal("Invalid smrd opcode");
			}

			operand_dump_series_scalar(operand_str, sbase, 
				sbase_end);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VOP2_LIT", &token_len))
		{
			str_printf(&inst_str, &str_size, "0x%08x", 
				self->bytes.vop2.lit_cnst);
		}
		else if (is_token(fmt_str, "OFFSET", &token_len))
		{
			if (self->bytes.smrd.imm)
			{
				str_printf(&inst_str, &str_size, "0x%02x", 
					self->bytes.smrd.offset);
			}
			else
			{
				operand_dump_scalar(operand_str, 
					self->bytes.smrd.offset);
				str_printf(&inst_str, &str_size, "%s", 
					operand_str);
			}
		}
		else if (is_token(fmt_str, "DS_VDST", &token_len))
		{
			operand_dump_vector(operand_str, 
				self->bytes.ds.vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "ADDR", &token_len))
		{
			operand_dump_vector(operand_str, 
				self->bytes.ds.addr);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "DATA0", &token_len))
		{
			operand_dump_vector(operand_str, 
				self->bytes.ds.data0);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "DATA1", &token_len))
		{
			operand_dump_vector(operand_str, 
				self->bytes.ds.data1);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "OFFSET0", &token_len))
		{
			if(self->bytes.ds.offset0)
			{
				str_printf(&inst_str, &str_size, "offset0:%u ", 
					self->bytes.ds.offset0);
			}
		}
		else if (is_token(fmt_str, "DS_SERIES_VDST", &token_len))
		{
			operand_dump_series_vector(operand_str, 
				self->bytes.ds.vdst, 
				self->bytes.ds.vdst+ 1);
			str_printf(&inst_str, &str_size, "%s", 
				operand_str);
		}
		else if (is_token(fmt_str, "OFFSET1", &token_len))
		{
			if(self->bytes.ds.offset1)
			{
				str_printf(&inst_str, &str_size, "offset1:%u ", 
					self->bytes.ds.offset1);
			}
		}
		else if (is_token(fmt_str, "VINTRP_VDST", &token_len))
		{
			operand_dump_vector(operand_str, 
				self->bytes.vintrp.vdst);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "VSRC_I_J", &token_len))
		{
			operand_dump_vector(operand_str, 
				self->bytes.vintrp.vsrc);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "ATTR", &token_len))
		{
			str_printf(&inst_str, &str_size, "attr_%d", 
				self->bytes.vintrp.attr);
		}
		else if (is_token(fmt_str, "ATTRCHAN", &token_len))
		{
			switch (self->bytes.vintrp.attrchan)
			{
				case 0:
					str_printf(&inst_str, &str_size, "x");
					break;
				case 1:
					str_printf(&inst_str, &str_size, "y");
					break;
				case 2:
					str_printf(&inst_str, &str_size, "z");
					break;
				case 3:
					str_printf(&inst_str, &str_size, "w");
					break;
				default:
					break;
			}
		}
		else if (is_token(fmt_str, "MU_SERIES_VDATA", &token_len))
		{
			SIInstDump_SERIES_VDATA(self->bytes.mubuf.vdata, 
				self->bytes.mubuf.op, operand_str, 
				&inst_str, str_size);
		}
		else if (is_token(fmt_str, "MU_GLC", &token_len))
		{
			if (self->bytes.mubuf.glc)
				str_printf(&inst_str, &str_size, "glc");
		}
		else if (is_token(fmt_str, "VADDR", &token_len))
		{
			if (self->bytes.mtbuf.offen && 
				self->bytes.mtbuf.idxen)
			{
				operand_dump_series_vector(operand_str, 
					self->bytes.mtbuf.vaddr, 
					self->bytes.mtbuf.vaddr + 1);
				str_printf(&inst_str, &str_size, "%s", 
					operand_str);
			}
			else
			{
				operand_dump_vector(operand_str, 
					self->bytes.mtbuf.vaddr);
				str_printf(&inst_str, &str_size, "%s", 
					operand_str);
			}
		}
		else if (is_token(fmt_str, "MU_MADDR", &token_len))
		{
			SIInstDump_MADDR(self, operand_str, &inst_str, 
				str_size);
		}
		else if (is_token(fmt_str, "MT_SERIES_VDATA", &token_len))
		{
			SIInstDump_SERIES_VDATA(self->bytes.mtbuf.vdata, 
				self->bytes.mtbuf.op, operand_str, 
				&inst_str, str_size);
		}
		else if (is_token(fmt_str, "SERIES_SRSRC", &token_len))
		{
			assert((self->bytes.mtbuf.srsrc << 2) % 4 == 0);
			operand_dump_series_scalar(operand_str, 
				self->bytes.mtbuf.srsrc << 2, 
				(self->bytes.mtbuf.srsrc << 2) + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "MT_MADDR", &token_len))
		{
			SIInstDump_MADDR(self, operand_str, &inst_str, 
				str_size);
		
			/* Format */
			str_printf(&inst_str, &str_size, " format:[%s,%s]",
				str_map_value(&si_inst_buf_data_format_map, 
					self->bytes.mtbuf.dfmt),
				str_map_value(&si_inst_buf_num_format_map, 
					self->bytes.mtbuf.nfmt));
		}
		else if (is_token(fmt_str, "MIMG_SERIES_VDATA", &token_len))
		{
			operand_dump_series_vector(operand_str, 
				self->bytes.mimg.vdata, 
				self->bytes.mimg.vdata + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "MIMG_VADDR", &token_len))
		{
			operand_dump_series_vector(operand_str, 
				self->bytes.mimg.vaddr, 
				self->bytes.mimg.vaddr + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "MIMG_SERIES_SRSRC", &token_len))
		{
			assert((self->bytes.mimg.srsrc << 2) % 4 == 0);
			operand_dump_series_scalar(operand_str, 
				self->bytes.mimg.srsrc << 2, 
				(self->bytes.mimg.srsrc << 2) + 7);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "MIMG_DUG_SERIES_SRSRC", &token_len))
		{
			assert((self->bytes.mimg.srsrc << 2) % 4 == 0);
			operand_dump_series_scalar(operand_str, 
				self->bytes.mimg.srsrc << 2, 
				(self->bytes.mimg.srsrc << 2) + 7);
			str_printf(&inst_str, &str_size, "%s", operand_str);

			/* Call SIInstDump_DUG to print 
			 * dmask, unorm, and glc */
			SIInstDump_DUG(self, operand_str, &inst_str, 
				str_size);
		}
		else if (is_token(fmt_str, "MIMG_SERIES_SSAMP", &token_len))
		{
			assert((self->bytes.mimg.ssamp << 2) % 4 == 0);
			operand_dump_series_scalar(operand_str, 
				self->bytes.mimg.ssamp << 2, 
				(self->bytes.mimg.ssamp << 2) + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "MIMG_DUG_SERIES_SSAMP", 
			&token_len))
		{
			assert((self->bytes.mimg.ssamp << 2) % 4 == 0);
			operand_dump_series_scalar(operand_str, 
				self->bytes.mimg.ssamp << 2, 
				(self->bytes.mimg.ssamp << 2) + 3);
			str_printf(&inst_str, &str_size, "%s", operand_str);
			
			/* Call SIInstDump_DUG to print 
			 * dmask, unorm, and glc */
			SIInstDump_DUG(self, operand_str, &inst_str, 
				str_size);
		}
		else if (is_token(fmt_str, "TGT", &token_len))
		{
			operand_dump_exp(operand_str, 
				self->bytes.exp.tgt);
			str_printf(&inst_str, &str_size, "%s", operand_str);
		}
		else if (is_token(fmt_str, "EXP_VSRCs", &token_len))
		{
			if (self->bytes.exp.compr == 0 && 
				(self->bytes.exp.en && 0x0) == 0x0)
			{
				operand_dump_vector(operand_str, 
					self->bytes.exp.vsrc0);
				str_printf(&inst_str, &str_size, 
					"[%s ", operand_str);
				operand_dump_vector(operand_str, 
					self->bytes.exp.vsrc1);
				str_printf(&inst_str, &str_size, "%s ", 
					operand_str);
				operand_dump_vector(operand_str, 
					self->bytes.exp.vsrc2);
				str_printf(&inst_str, &str_size, "%s ", 
					operand_str);
				operand_dump_vector(operand_str, 
					self->bytes.exp.vsrc3);
				str_printf(&inst_str, &str_size, "%s]", 
					operand_str);
			}
			else if (self->bytes.exp.compr == 1 && 
				(self->bytes.exp.en && 0x0) == 0x0)
			{
				operand_dump_vector(operand_str, 
					self->bytes.exp.vsrc0);
				str_printf(&inst_str, &str_size, "[%s ", 
					operand_str);
				operand_dump_vector(operand_str, 
					self->bytes.exp.vsrc1);
				str_printf(&inst_str, &str_size, "%s]", 
					operand_str);
			}
		}
		else
		{
			fatal("%s: token not recognized.", fmt_str);
		}

		fmt_str += token_len;
	}
	line_dump(orig_inst_str, rel_addr, buf, line, line_size, self->size);
}


void SIInstClear(SIInst *self)
{
	self->info = NULL;
	self->bytes.dword = 0;
}


void SIInstDecode(SIInst *self, void *buf, unsigned int offset)
{
	SIAsm *as = self->as;

	/* Initialize instruction */
	SIInstClear(self);

	/* Instruction is at least 4 bytes */
	self->size = 4;
	self->bytes.word[0] = * (unsigned int *) buf;

	/* Use the encoding field to determine the instruction type */
	if (self->bytes.sopp.enc == 0x17F)
	{
		if (!as->inst_info_sopp[self->bytes.sopp.op])
		{
			fatal("Unimplemented Instruction: SOPP:%d  "
				"// %08X: %08X\n", self->bytes.sopp.op,
				offset, * (unsigned int *) buf);
		}

		self->info = as->inst_info_sopp[self->bytes.sopp.op];
	}
	else if (self->bytes.sopc.enc == 0x17E)
	{
		if (!as->inst_info_sopc[self->bytes.sopc.op])
		{
			fatal("Unimplemented Instruction: SOPC:%d  "
				"// %08X: %08X\n", self->bytes.sopc.op,
				offset, * (unsigned int *) buf);
		}

		self->info = as->inst_info_sopc[self->bytes.sopc.op];

		/* Only one source field may use a literal constant,
		 * which is indicated by 0xFF. */
		assert(!(self->bytes.sopc.ssrc0 == 0xFF &&
			self->bytes.sopc.ssrc1 == 0xFF));
		if (self->bytes.sopc.ssrc0 == 0xFF ||
			self->bytes.sopc.ssrc1 == 0xFF)
		{
			self->size = 8;
			self->bytes.dword = * (unsigned long long *) buf;
		}
	}
	else if (self->bytes.sop1.enc == 0x17D)
	{
		if (!as->inst_info_sop1[self->bytes.sop1.op])
		{
			fatal("Unimplemented Instruction: SOP1:%d  "
				"// %08X: %08X\n", self->bytes.sop1.op,
				offset, *(unsigned int*)buf);
		}

		self->info = as->inst_info_sop1[self->bytes.sop1.op];

		/* 0xFF indicates the use of a literal constant as a
		 * source operand. */
		if (self->bytes.sop1.ssrc0 == 0xFF)
		{
			self->size = 8;
			self->bytes.dword = * (unsigned long long *) buf;		}
	}
	else if (self->bytes.sopk.enc == 0xB)
	{
		if (!as->inst_info_sopk[self->bytes.sopk.op])
		{
			fatal("Unimplemented Instruction: SOPK:%d  "
				"// %08X: %08X\n", self->bytes.sopk.op,
				offset, * (unsigned int *) buf);
		}

		self->info = as->inst_info_sopk[self->bytes.sopk.op];
	}
	else if (self->bytes.sop2.enc == 0x2)
	{
		if (!as->inst_info_sop2[self->bytes.sop2.op])
		{
			fatal("Unimplemented Instruction: SOP2:%d  "
				"// %08X: %08X\n", self->bytes.sop2.op,
				offset, *(unsigned int *)buf);
		}

		self->info = as->inst_info_sop2[self->bytes.sop2.op];

		/* Only one source field may use a literal constant,
		 * which is indicated by 0xFF. */
		assert(!(self->bytes.sop2.ssrc0 == 0xFF &&
			self->bytes.sop2.ssrc1 == 0xFF));
		if (self->bytes.sop2.ssrc0 == 0xFF ||
			self->bytes.sop2.ssrc1 == 0xFF)
		{
			self->size = 8;
			self->bytes.dword = * (unsigned long long *) buf;		}
	}
	else if (self->bytes.smrd.enc == 0x18)
	{
		if (!as->inst_info_smrd[self->bytes.smrd.op])
		{
			fatal("Unimplemented Instruction: SMRD:%d  "
				"// %08X: %08X\n", self->bytes.smrd.op,
				offset, *(unsigned int *)buf);
		}

		self->info = as->inst_info_smrd[self->bytes.smrd.op];
	}
	else if (self->bytes.vop3a.enc == 0x34)
	{
		/* 64 bit instruction. */
		self->size = 8;
		self->bytes.dword = * (unsigned long long *) buf;

		if (!as->inst_info_vop3[self->bytes.vop3a.op])
		{
			fatal("Unimplemented Instruction: VOP3:%d  "
				"// %08X: %08X %08X\n",
				self->bytes.vop3a.op, offset,
				*(unsigned int *)buf,
				*(unsigned int *)(buf + 4));
		}

		self->info = as->inst_info_vop3[self->bytes.vop3a.op];
	}
	else if (self->bytes.vopc.enc == 0x3E)
	{
		if (!as->inst_info_vopc[self->bytes.vopc.op])
		{
			fatal("Unimplemented Instruction: VOPC:%d  "
				"// %08X: %08X\n",
				self->bytes.vopc.op, offset,
				*(unsigned int *)buf);
		}

		self->info = as->inst_info_vopc[self->bytes.vopc.op];

		/* 0xFF indicates the use of a literal constant as a
		 * source operand. */
		if (self->bytes.vopc.src0 == 0xFF)
		{
			self->size = 8;
			self->bytes.dword = * (unsigned long long *) buf;		}
	}
	else if (self->bytes.vop1.enc == 0x3F)
	{
		if (!as->inst_info_vop1[self->bytes.vop1.op])
		{
			fatal("Unimplemented Instruction: VOP1:%d  "
				"// %08X: %08X\n", self->bytes.vop1.op,
				offset, * (unsigned int *) buf);
		}

		self->info = as->inst_info_vop1[self->bytes.vop1.op];

		/* 0xFF indicates the use of a literal constant as a
		 * source operand. */
		if (self->bytes.vop1.src0 == 0xFF)
		{
			self->size = 8;
			self->bytes.dword = * (unsigned long long *) buf;		}
	}
	else if (self->bytes.vop2.enc == 0x0)
	{
		if (!as->inst_info_vop2[self->bytes.vop2.op])
		{
			fatal("Unimplemented Instruction: VOP2:%d  "
				"// %08X: %08X\n", self->bytes.vop2.op,
				offset, * (unsigned int *) buf);
		}

		self->info = as->inst_info_vop2[self->bytes.vop2.op];

		/* 0xFF indicates the use of a literal constant as a
		 * source operand. */
		if (self->bytes.vop2.src0 == 0xFF)
		{
			self->size = 8;
			self->bytes.dword = * (unsigned long long *) buf;		}

		/* Some opcodes define a 32-bit literal constant following
		 * the instruction */
		if (self->bytes.vop2.op == 32)
		{
			self->size = 8;
			self->bytes.dword = * (unsigned long long *) buf;		}
	}
	else if (self->bytes.vintrp.enc == 0x32)
	{
		if (!as->inst_info_vintrp[self->bytes.vintrp.op])
		{
			fatal("Unimplemented Instruction: VINTRP:%d  "
				"// %08X: %08X\n", self->bytes.vintrp.op,
				offset, * (unsigned int *) buf);
		}

		self->info = as->inst_info_vintrp[self->bytes.vintrp.op];

	}
	else if (self->bytes.ds.enc == 0x36)
	{
		/* 64 bit instruction. */
		self->size = 8;
		self->bytes.dword = * (unsigned long long *) buf;
		if (!as->inst_info_ds[self->bytes.ds.op])
		{
			fatal("Unimplemented Instruction: DS:%d  "
				"// %08X: %08X %08X\n", self->bytes.ds.op,
				offset, *(unsigned int *)buf,
				*(unsigned int *)(buf + 4));
		}

		self->info = as->inst_info_ds[self->bytes.ds.op];
	}
	else if (self->bytes.mtbuf.enc == 0x3A)
	{
		/* 64 bit instruction. */
		self->size = 8;
		self->bytes.dword = * (unsigned long long *) buf;

		if (!as->inst_info_mtbuf[self->bytes.mtbuf.op])
		{
			fatal("Unimplemented Instruction: MTBUF:%d  "
				"// %08X: %08X %08X\n",
				self->bytes.mtbuf.op, offset,
				*(unsigned int *)buf, *(unsigned int *)(buf+4));
		}

		self->info = as->inst_info_mtbuf[self->bytes.mtbuf.op];
	}
	else if (self->bytes.mubuf.enc == 0x38)
	{
		/* 64 bit instruction. */
		self->size = 8;
		self->bytes.dword = * (unsigned long long *) buf;

		if (!as->inst_info_mubuf[self->bytes.mubuf.op])
		{
			fatal("Unimplemented Instruction: MUBUF:%d  "
				"// %08X: %08X %08X\n",
				self->bytes.mubuf.op, offset,
				*(unsigned int *)buf,
				*(unsigned int *)(buf+4));
		}

		self->info = as->inst_info_mubuf[self->bytes.mubuf.op];
	}
	else if (self->bytes.mimg.enc == 0x3C)
	{
		/* 64 bit instruction. */
		self->size = 8;
		self->bytes.dword = * (unsigned long long *) buf;

		if(!as->inst_info_mimg[self->bytes.mimg.op])
		{
			fatal("Unimplemented Instruction: MIMG:%d  "
				"// %08X: %08X %08X\n",
				self->bytes.mimg.op, offset,
				*(unsigned int *)buf,
				*(unsigned int *)(buf+4));
		}

		self->info = as->inst_info_mimg[self->bytes.mimg.op];
	}
	else if (self->bytes.exp.enc == 0x3E)
	{
		/* 64 bit instruction. */
		self->size = 8;
		self->bytes.dword = * (unsigned long long *) buf;

		/* Export is the only instruction in its kind */
		if (!as->inst_info_exp[0])
			fatal("Unimplemented Instruction: EXP\n");

		self->info = as->inst_info_exp[0];
	}
	else
	{
		fatal("Unimplemented format. Instruction is:  // %08X: %08X\n",
				offset, ((unsigned int*)buf)[0]);
	}
}

#endif

