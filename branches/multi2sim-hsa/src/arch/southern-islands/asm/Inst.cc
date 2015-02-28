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

#include <cassert>
#include <iomanip>
#include <sstream>

#include <arch/common/Asm.h>
#include <lib/cpp/Misc.h>

#include "Asm.h"
#include "Inst.h"


namespace SI
{

misc::StringMap inst_format_map =
{
	{ "<invalid>", InstFormatInvalid },
	{ "sop2", InstFormatSOP2 },
	{ "sopk", InstFormatSOPK },
	{ "sop1", InstFormatSOP1 },
	{ "sopc", InstFormatSOPC },
	{ "sopp", InstFormatSOPP },
	{ "smrd", InstFormatSMRD },
	{ "vop2", InstFormatVOP2 },
	{ "vop1", InstFormatVOP1 },
	{ "vopc", InstFormatVOPC },
	{ "vop3a", InstFormatVOP3a },
	{ "vop3b", InstFormatVOP3b },
	{ "vintrp", InstFormatVINTRP },
	{ "ds", InstFormatDS },
	{ "mubuf", InstFormatMUBUF },
	{ "mtbuf", InstFormatMTBUF },
	{ "mimg", InstFormatMIMG },
	{ "exp", InstFormatEXP }
};

/* String maps for assembly dump. */
static misc::StringMap inst_sdst_map =
{
	{"reserved", 0},
	{"reserved", 1},
	{"vcc_lo", 2},
	{"vcc_hi", 3},
	{"tba_lo", 4},
	{"tba_hi", 5},
	{"tma_lo", 6},
	{"tma_hi", 7},
	{"ttmp0", 8},
	{"ttmp1", 9},
	{"ttmp2", 10},
	{"ttmp3", 11},
	{"ttmp4", 12},
	{"ttmp5", 13},
	{"ttmp6", 14},
	{"ttmp7", 15},
	{"ttmp8", 16},
	{"ttmp9", 17},
	{"ttmp10", 18},
	{"ttmp11", 19},
	{"m0", 20},
	{"reserved", 21},
	{"exec_lo", 22},
	{"exec_hi", 23}
};

static misc::StringMap inst_ssrc_map =
{
	{"0.5", 0},
	{"-0.5", 1},
	{"1.0", 2},
	{"-1.0", 3},
	{"2.0", 4},
	{"-2.0", 5},
	{"4.0", 6},
	{"-4.0", 7},
	{"reserved", 8},
	{"reserved", 9},
	{"reserved", 10},
	{"vccz", 11},
	{"execz", 12},
	{"scc", 13},
	{"reserved", 14},
	{"literal constant", 15}
};

misc::StringMap inst_buf_data_format_map =
{
	{"invalid", InstBufDataFormatInvalid },
	{"BUF_DATA_FORMAT_8", InstBufDataFormat8 },
	{"BUF_DATA_FORMAT_16", InstBufDataFormat16 },
	{"BUF_DATA_FORMAT_8_8", InstBufDataFormat8_8 },
	{"BUF_DATA_FORMAT_32", InstBufDataFormat32 },
	{"BUF_DATA_FORMAT_16_16", InstBufDataFormat16_16 },
	{"BUF_DATA_FORMAT_10_11_11", InstBufDataFormat10_11_11 },
	{"BUF_DATA_FORMAT_11_10_10", InstBufDataFormat11_10_10 },
	{"BUF_DATA_FORMAT_10_10_10_2", InstBufDataFormat10_10_10_2 },
	{"BUF_DATA_FORMAT_2_10_10_10", InstBufDataFormat2_10_10_10 },
	{"BUF_DATA_FORMAT_8_8_8_8", InstBufDataFormat8_8_8_8 },
	{"BUF_DATA_FORMAT_32_32", InstBufDataFormat32_32 },
	{"BUF_DATA_FORMAT_16_16_16_16", InstBufDataFormat16_16_16_16 },
	{"BUF_DATA_FORMAT_32_32_32", InstBufDataFormat32_32_32 },
	{"BUF_DATA_FORMAT_32_32_32_32", InstBufDataFormat32_32_32_32 },
	{"reserved", InstBufDataFormatReserved }
};

misc::StringMap inst_buf_num_format_map =
{
	{"BUF_NUM_FORMAT_UNORM", InstBufNumFormatUnorm },
	{"BUF_NUM_FORMAT_SNORM", InstBufNumFormatSnorm },
	{"BUF_NUM_FORMAT_UNSCALED", InstBufNumFormatUnscaled },
	{"BUF_NUM_FORMAT_SSCALED", InstBufNumFormatSscaled },
	{"BUF_NUM_FORMAT_UINT", InstBufNumFormatUint },
	{"BUF_NUM_FORMAT_SINT", InstBufNumFormatSint },
	{"BUF_NUM_FORMAT_SNORM_NZ", InstBufNumFormatSnormNz },
	{"BUF_NUM_FORMAT_FLOAT", InstBufNumFormatFloat },
	{"reserved", InstBufNumFormatReserved },
	{"BUF_NUM_FORMAT_SRGB", InstBufNumFormatSrgb },
	{"BUF_NUM_FORMAT_UBNORM", InstBufNumFormatUbnorm },
	{"BUF_NUM_FORMAT_UBNORM_NZ", InstBufNumFormatUbnormNz },
	{"BUF_NUM_FORMAT_UBINT", InstBufNumFormatUbint },
	{"BUF_NUM_FORMAT_UBSCALED", InstBufNumFormatUbscaled }
};

static misc::StringMap inst_OP16_map =
{
	{"f", 0},
	{"lt", 1},
	{"eq", 2},
	{"le", 3},
	{"gt", 4},
	{"lg", 5},
	{"ge", 6},
	{"o", 7},
	{"u", 8},
	{"nge", 9},
	{"nlg", 10},
	{"ngt", 11},
	{"nle", 12},
	{"neq", 13},
	{"nlt", 14},
	{"tru", 15}
};

static misc::StringMap inst_OP8_map =
{
	{"f", 0},
	{"lt", 1},
	{"eq", 2},
	{"le", 3},
	{"gt", 4},
	{"lg", 5},
	{"ge", 6},
	{"tru", 7}
};

misc::StringMap inst_special_reg_map =
{
	{ "vcc", InstSpecialRegVcc },
	{ "scc", InstSpecialRegScc },
	{ "exec", InstSpecialRegExec },
	{ "tma", InstSpecialRegTma }
};


void Inst::DumpOperand(std::ostream& os, int operand)
{
	assert(operand >= 0 && operand <= 511);
	if (operand <= 103)
	{
		/* SGPR */
		os << "s" << operand;
	}
	else if (operand <= 127)
	{
		/* sdst special registers */
		os << inst_sdst_map.MapValue(operand - 104);
	}
	else if (operand <= 192)
	{
		/* Positive integer constant */
		os << operand - 128;
	}
	else if (operand <= 208)
	{
		/* Negative integer constant */
		os << '-' << operand - 192;
	}
	else if (operand <= 239)
	{
		throw Asm::Error(misc::fmt("Unused operand code (%d)",
				operand));
	}
	else if (operand <= 255)
	{
		os << inst_ssrc_map.MapValue(operand - 240);
	}
	else if (operand <= 511)
	{
		/* VGPR */
		os << "v" << operand - 256;
	}
}


void Inst::DumpOperandSeries(std::ostream& os, int start, int end)
{
	assert(start <= end);
	if (start == end)
	{
		DumpOperand(os, start);
		return;
	}

	if (start <= 103)
	{
		os << "s[" << start << ':' << end << ']';
	}
	else if (start <= 245)
	{
		if (start >= 112 && start <= 123)
		{
			assert(end <= 123);
			os << "ttmp[" << start - 112 << ':' << end - 112 << ']';
		}
		else
		{
			assert(end == start + 1);
			switch (start)
			{
			case 106:
				os << "vcc";
				break;
			case 108:
				os << "tba";
				break;
			case 110:
				os << "tma";
				break;
			case 126:
				os << "exec";
				break;
			case 128:
				os << "0";
				break;
			case 131:
				os << "3";
				break;
			case 208: 
				os << "-16";
				break;
			case 240:
				os << "0.5";
				break;
			case 242:
				os << "1.0";
				break;
			case 243:
				os << "-1.0";
				break;
			case 244:
				os << "2.0";
				break;
			case 245:
				os << "-2.0";
				break;
			default:
				throw Asm::Error(misc::fmt(
						"Unimplemented series: "
						"[%d:%d]", start, end));
			}
		}
	}
	else if (start <= 255)
	{
		throw Asm::Error(misc::fmt("Illegal operand series: [%d:%d]",
				start, end));
	}
	else if (start <= 511)
	{
		os << "v[" << start - 256 << ':' << end - 256 << ']';
	}
}


void Inst::DumpScalar(std::ostream& os, int operand)
{
	DumpOperand(os, operand);
}


void Inst::DumpScalarSeries(std::ostream& os, int start, int end)
{
	DumpOperandSeries(os, start, end);
}


void Inst::DumpVector(std::ostream& os, int operand)
{
	DumpOperand(os, operand + 256);
}


void Inst::DumpVectorSeries(std::ostream& os, int start, int end)
{
	DumpOperandSeries(os, start + 256, end + 256);
}


void Inst::DumpOperandExp(std::ostream& os, int operand)
{
	assert(operand >= 0 && operand <= 63);
	if (operand <= 7)
	{
		/* EXP_MRT */
		os << "exp_mrt_" << operand;
	}
	else if (operand == 8)
	{
		/* EXP_Z */
		os << "exp_mrtz";
	}
	else if (operand == 9)
	{
		/* EXP_NULL */
		os << "exp_null";
	}
	else if (operand < 12)
	{
		throw Asm::Error(misc::fmt("Operand code [%d] unused.",
				operand));
	}
	else if (operand <= 15)
	{
		/* EXP_POS */
		os << "exp_pos_" << operand - 12;
	}
	else if (operand < 32)
	{
		throw Asm::Error(misc::fmt("Operand code [%d] unused.",
				operand));
	}
	else if (operand <= 63)
	{
		/* EXP_PARAM */
		os << "exp_prm_" << operand - 32;
	}
}


void Inst::DumpSeriesVdata(std::ostream& os, unsigned int vdata, int op)
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

			throw Asm::Error("MUBUF/MTBUF opcode not recognized");
	}

	DumpVectorSeries(os, vdata, vdata_end);
}


void Inst::DumpSsrc(std::ostream& os, unsigned int ssrc) const
{
	if (ssrc == 0xff)
		os << misc::fmt("0x%08x", bytes.sop2.lit_cnst);
	else
		DumpScalar(os, ssrc);
}


void Inst::Dump64Ssrc(std::ostream& os, unsigned int ssrc) const
{		
	if (ssrc == 0xff)
		os << misc::fmt("0x%08x", bytes.sop2.lit_cnst);
	else
		DumpScalarSeries(os, ssrc, ssrc + 1);
}


void Inst::DumpVop3Src(std::ostream& os, unsigned int src, int neg) const
{
	std::stringstream ss;

	DumpOperand(ss, src);
	if (!(misc::inRange(bytes.vop3a.op, 293, 298)) && 
		!(misc::inRange(bytes.vop3a.op, 365, 366)))
	{
		if ((bytes.vop3a.neg & neg) && 
				(bytes.vop3a.abs & neg))
			os << "-abs(" << ss.str() << ")";
		else if ((bytes.vop3a.neg & neg) && 
				!(bytes.vop3a.abs & neg))
			os << '-' << ss.str();
		else if (!(bytes.vop3a.neg & neg) && 
				(bytes.vop3a.abs & neg))
			os << "abs(" << ss.str() << ')';
		else if (!(bytes.vop3a.neg & neg) && 
				!(bytes.vop3a.abs & neg))
			os << ss.str();
	}
	else
	{
		if (bytes.vop3a.neg & neg)
			os << '-' << ss.str();
		else if (!(bytes.vop3a.neg & neg))
			os << ss.str();
	}
}


void Inst::DumpVop364Src(std::ostream& os, unsigned int src, int neg) const
{
	std::stringstream ss;

	DumpOperandSeries(ss, src, src + 1);
	if (!(misc::inRange(bytes.vop3a.op, 293, 298)) && 
		!(misc::inRange(bytes.vop3a.op, 365, 366)))
	{
		if ((bytes.vop3a.neg & neg) && 
				(bytes.vop3a.abs & neg))
			os << "-abs(" << ss.str() << ')';
		else if ((bytes.vop3a.neg & neg) && 
				!(bytes.vop3a.abs & neg))
			os << "-" << ss.str();
		else if (!(bytes.vop3a.neg & neg) && 
				(bytes.vop3a.abs & neg))
			os << "abs(" << ss.str() << ")";
		else if (!(bytes.vop3a.neg & neg) && 
				!(bytes.vop3a.abs & neg))
			os << ss.str();
	}
	else
	{
		if (bytes.vop3a.neg & neg)
			os << '-' << ss.str();
		else if (!(bytes.vop3a.neg & neg))
			os << ss.str();
	}
}


void Inst::DumpMaddr(std::ostream& os) const
{
	/* soffset */
	assert(bytes.mtbuf.soffset <= 103 ||
			bytes.mtbuf.soffset == 124 ||
			(bytes.mtbuf.soffset >= 128 && 
			bytes.mtbuf.soffset <= 208));
	DumpScalar(os, bytes.mtbuf.soffset);

	/* offen */
	if (bytes.mtbuf.offen)
		os << " offen";

	/* index */
	if (bytes.mtbuf.idxen)
		os << " idxen";

	/* offset */
	if (bytes.mtbuf.offset)
		os << " offset:" << bytes.mtbuf.offset;
}


void Inst::DumpDug(std::ostream& os) const
{
	/* DMASK */
	os << misc::fmt(" dmask:0x%x", bytes.mimg.dmask);
	
	/* UNORM */
	if (bytes.mimg.unorm)
		os << " unorm";
	
	/* GLC */
	if (bytes.mimg.glc)
		os << " glc";
}


Inst::Inst(Asm *as)
{
	/* Initialize */
	this->as = as;
	Clear();
}


void Inst::Dump(std::ostream &os) const
{
	int token_len;
	const char *fmt_str;
	
	/* Traverse format string */
	fmt_str = info ? info->fmt_str : "";
	while (*fmt_str)
	{
		/* Literal */
		if (*fmt_str != '%')
		{
			os << *fmt_str;
			fmt_str++;
			continue;
		}

		/* Token */
		fmt_str++;
		if (comm::Asm::isToken(fmt_str, "WAIT_CNT", token_len))
		{	
			const InstBytesSOPP *sopp = &bytes.sopp;

			unsigned int more = 0;
			int vm_cnt = (sopp->simm16 & 0xF);

			if (vm_cnt != 0xF)
			{
				os << "vmcnt(" << vm_cnt << ")";
				more = 1;
			}

			int lgkm_cnt = (sopp->simm16 & 0x1f00) >> 8;
			if (lgkm_cnt != 0x1f)
			{
				if (more)
					os << " & ";
				os << "lgkmcnt(" << lgkm_cnt << ")";
				more = 1;
			}

			int exp_cnt = (sopp->simm16 & 0x70) >> 4;
			if (exp_cnt != 0x7)
			{
				if (more)
					os << " & ";
				os << "expcnt(" << exp_cnt << ")";
				more = 1;
			}
		}
		else if (comm::Asm::isToken(fmt_str, "LABEL", token_len))
		{		
			const InstBytesSOPP *sopp = &bytes.sopp;
	
			short simm16 = sopp->simm16;
			int se_simm = simm16;

			os << misc::fmt("label_%04X",
					(address + (se_simm * 4) + 4) / 4);
		}
		else if (comm::Asm::isToken(fmt_str, "SSRC0", token_len))
		{	
			DumpSsrc(os, bytes.sop2.ssrc0);
		}
		else if (comm::Asm::isToken(fmt_str, "64_SSRC0", token_len))
		{
			Dump64Ssrc(os, bytes.sop2.ssrc0);
		}
		else if (comm::Asm::isToken(fmt_str, "SSRC1", token_len))
		{
			DumpSsrc(os, bytes.sop2.ssrc1);
		}
		else if (comm::Asm::isToken(fmt_str, "64_SSRC1", token_len))
		{
			Dump64Ssrc(os, bytes.sop2.ssrc1);
		}
		else if (comm::Asm::isToken(fmt_str, "SDST", token_len))
		{	
			DumpScalar(os, bytes.sop2.sdst);
		}
		else if (comm::Asm::isToken(fmt_str, "64_SDST", token_len))
		{
			DumpScalarSeries(os, bytes.sop2.sdst, bytes.sop2.sdst + 1);
		}
		else if (comm::Asm::isToken(fmt_str, "SIMM16", token_len))
		{
			os << misc::fmt("0x%04x", bytes.sopk.simm16);
		}
		else if (comm::Asm::isToken(fmt_str, "SRC0", token_len))
		{
			if (bytes.vopc.src0 == 0xFF)
				os << misc::fmt("0x%08x", bytes.vopc.lit_cnst);
			else
				DumpOperand(os, bytes.vopc.src0);
		}
		else if (comm::Asm::isToken(fmt_str, "64_SRC0", token_len))
		{
			assert(bytes.vopc.src0 != 0xFF);
			DumpOperandSeries(os, bytes.vopc.src0, bytes.vopc.src0 + 1);
		}
		else if (comm::Asm::isToken(fmt_str, "VSRC1", token_len))
		{
			DumpVector(os, bytes.vopc.vsrc1);
		}
		else if (comm::Asm::isToken(fmt_str, "64_VSRC1", token_len))
		{
			assert(bytes.vopc.vsrc1 != 0xFF);
			DumpVectorSeries(os, bytes.vopc.vsrc1, bytes.vopc.vsrc1 + 1);
		}
		else if (comm::Asm::isToken(fmt_str, "VDST", token_len))
		{
			DumpVector(os, bytes.vop1.vdst);
		}
		else if (comm::Asm::isToken(fmt_str, "64_VDST", token_len))
		{
			DumpVectorSeries(os, bytes.vop1.vdst, bytes.vop1.vdst + 1);
		}
		else if (comm::Asm::isToken(fmt_str, "SVDST", token_len))
		{
			DumpScalar(os, bytes.vop1.vdst);
		}
		else if (comm::Asm::isToken(fmt_str, "VOP3_64_SVDST", token_len))
		{
			/* VOP3a compare operations use the VDST field to 
			 * indicate the address of the scalar destination.*/
			DumpScalarSeries(os, bytes.vop3a.vdst, bytes.vop3a.vdst + 1);
		}
		else if (comm::Asm::isToken(fmt_str, "VOP3_VDST", token_len))
		{
			DumpVector(os, bytes.vop3a.vdst);
		}
		else if (comm::Asm::isToken(fmt_str, "VOP3_64_VDST", token_len))
		{
			DumpVectorSeries(os, bytes.vop3a.vdst, bytes.vop3a.vdst + 1);
		}
		else if (comm::Asm::isToken(fmt_str, "VOP3_64_SDST", token_len))
		{
			DumpScalarSeries(os, bytes.vop3b.sdst, bytes.vop3b.sdst + 1);
		}
		else if (comm::Asm::isToken(fmt_str, "VOP3_SRC0", token_len))
		{
			DumpVop3Src(os, bytes.vop3a.src0, 1);
		}
		else if (comm::Asm::isToken(fmt_str, "VOP3_64_SRC0", token_len))
		{
			DumpVop364Src(os, bytes.vop3a.src0, 1);
		}
		else if (comm::Asm::isToken(fmt_str, "VOP3_SRC1", token_len))
		{
			DumpVop3Src(os, bytes.vop3a.src1, 2);
		}
		else if (comm::Asm::isToken(fmt_str, "VOP3_64_SRC1", token_len))
		{
			DumpVop364Src(os, bytes.vop3a.src1, 2);
		}
		else if (comm::Asm::isToken(fmt_str, "VOP3_SRC2", token_len))
		{
			DumpVop3Src(os, bytes.vop3a.src2, 4);
		}
		else if (comm::Asm::isToken(fmt_str, "VOP3_64_SRC2", token_len))
		{
			DumpVop364Src(os, bytes.vop3a.src2, 4);
		}
		else if (comm::Asm::isToken(fmt_str, "VOP3_OP16", token_len))
		{
			os << inst_OP16_map.MapValue(bytes.vop3a.op & 15);
		}
		else if (comm::Asm::isToken(fmt_str, "VOP3_OP8", token_len))
		{
			os << inst_OP8_map.MapValue(bytes.vop3a.op & 15);
		}
		else if (comm::Asm::isToken(fmt_str, "SMRD_SDST", token_len))
		{
			DumpScalar(os, bytes.smrd.sdst);
		}
		else if (comm::Asm::isToken(fmt_str, "SERIES_SDST", token_len))
		{
			/* The sbase field is missing the LSB, 
			 * so multiply by 2 */
			int sdst = bytes.smrd.sdst;
			int sdst_end;
			int op = bytes.smrd.op;

			/* S_LOAD_DWORD */
			if (misc::inRange(op, 0, 4))
			{
				/* Multi-dword */
				switch (op)
				{
				case 0:
					break;
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
					throw Asm::Error("Invalid smrd opcode");
				}
			}
			/* S_BUFFER_LOAD_DWORD */
			else if (misc::inRange(op, 8, 12))
			{	
				/* Multi-dword */
				switch (op)
				{
				case 8:
					break;
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
					throw Asm::Error("Invalid smrd opcode");
				}
			}
			/* S_MEMTIME */
			else if (op == 30)
			{
				throw misc::Panic("S_MEMTIME instruction not "
						"supported");
			}
			/* S_DCACHE_INV */
			else if (op == 31)
			{
				throw misc::Panic("S_DCACHE_INV instruction not" 
					"currently supported");
			}
			else
			{
				throw Asm::Error("Invalid smrd opcode");
			}

			DumpScalarSeries(os, sdst, sdst_end);

		}
		else if (comm::Asm::isToken(fmt_str, "SERIES_SBASE", token_len))
		{
			
			/* The sbase field is missing the LSB, 
			 * so multiply by 2 */
			int sbase = bytes.smrd.sbase * 2;
			int sbase_end;
			int op = bytes.smrd.op;

			/* S_LOAD_DWORD */
			if (misc::inRange(op, 0, 4))
			{
				/* SBASE specifies two consecutive SGPRs */
				sbase_end = sbase + 1;
			}
			/* S_BUFFER_LOAD_DWORD */
			else if (misc::inRange(op, 8, 12))
			{
				/* SBASE specifies four consecutive SGPRs */
				sbase_end = sbase + 3;
			}
			/* S_MEMTIME */
			else if (op == 30)
			{
				throw misc::Panic("S_MEMTIME instruction not "
						" supported");
			}
			/* S_DCACHE_INV */
			else if (op == 31)
			{
				throw misc::Panic("S_DCACHE_INV instruction "
						"not supported");
			}
			else
			{
				throw Asm::Error("Invalid smrd opcode");
			}

			DumpScalarSeries(os, sbase, sbase_end);
		}
		else if (comm::Asm::isToken(fmt_str, "VOP2_LIT", token_len))
		{
			os << misc::fmt("0x%08x", bytes.vop2.lit_cnst);
		}
		else if (comm::Asm::isToken(fmt_str, "OFFSET", token_len))
		{
			if (bytes.smrd.imm)
				os << misc::fmt("0x%02x", bytes.smrd.offset);
			else
				DumpScalar(os, bytes.smrd.offset);
		}
		else if (comm::Asm::isToken(fmt_str, "DS_VDST", token_len))
		{
			DumpVector(os, bytes.ds.vdst);
		}
		else if (comm::Asm::isToken(fmt_str, "ADDR", token_len))
		{
			DumpVector(os, bytes.ds.addr);
		}
		else if (comm::Asm::isToken(fmt_str, "DATA0", token_len))
		{
			DumpVector(os, bytes.ds.data0);
		}
		else if (comm::Asm::isToken(fmt_str, "DATA1", token_len))
		{
			DumpVector(os, bytes.ds.data1);
		}
		else if (comm::Asm::isToken(fmt_str, "OFFSET0", token_len))
		{
			if (bytes.ds.offset0)
				os << "offset0:" << bytes.ds.offset0 << ' ';
		}
		else if (comm::Asm::isToken(fmt_str, "DS_SERIES_VDST", token_len))
		{
			DumpVectorSeries(os, bytes.ds.vdst, bytes.ds.vdst + 1);
		}
		else if (comm::Asm::isToken(fmt_str, "OFFSET1", token_len))
		{
			if (bytes.ds.offset1)
				os << "offset1:" << bytes.ds.offset1 << ' ';
		}
		else if (comm::Asm::isToken(fmt_str, "VINTRP_VDST", token_len))
		{
			DumpVector(os, bytes.vintrp.vdst);
		}
		else if (comm::Asm::isToken(fmt_str, "VSRC_I_J", token_len))
		{
			DumpVector(os, bytes.vintrp.vsrc);
		}
		else if (comm::Asm::isToken(fmt_str, "ATTR", token_len))
		{
			os << "attr_" << bytes.vintrp.attr;
		}
		else if (comm::Asm::isToken(fmt_str, "ATTRCHAN", token_len))
		{
			switch (bytes.vintrp.attrchan)
			{
			case 0:
				os << 'x';
				break;
			case 1:
				os << 'y';
				break;
			case 2:
				os << 'z';
				break;
			case 3:
				os << 'w';
				break;
			default:
				break;
			}
		}
		else if (comm::Asm::isToken(fmt_str, "MU_SERIES_VDATA_DST", token_len)  ||
				comm::Asm::isToken(fmt_str, "MU_SERIES_VDATA_SRC", token_len))
		{
			DumpSeriesVdata(os, bytes.mubuf.vdata, bytes.mubuf.op);
		}
		else if (comm::Asm::isToken(fmt_str, "MU_GLC", token_len))
		{
			if (bytes.mubuf.glc)
				os << "glc";
		}
		else if (comm::Asm::isToken(fmt_str, "VADDR", token_len))
		{
			if (bytes.mtbuf.offen && bytes.mtbuf.idxen)
				DumpVectorSeries(os, bytes.mtbuf.vaddr, 
						bytes.mtbuf.vaddr + 1);
			else
				DumpVector(os, bytes.mtbuf.vaddr);
		}
		else if (comm::Asm::isToken(fmt_str, "MU_MADDR", token_len))
		{
			DumpMaddr(os);
		}
		else if (comm::Asm::isToken(fmt_str, "MT_SERIES_VDATA_DST", token_len) ||
				comm::Asm::isToken(fmt_str, "MT_SERIES_VDATA_SRC", token_len))
		{
			DumpSeriesVdata(os, bytes.mtbuf.vdata, bytes.mtbuf.op);
		}
		else if (comm::Asm::isToken(fmt_str, "SERIES_SRSRC", token_len))
		{
			assert((bytes.mtbuf.srsrc << 2) % 4 == 0);
			DumpScalarSeries(os, bytes.mtbuf.srsrc << 2, 
					(bytes.mtbuf.srsrc << 2) + 3);
		}
		else if (comm::Asm::isToken(fmt_str, "MT_MADDR", token_len))
		{
			DumpMaddr(os);
			os << " format:["
					<< inst_buf_data_format_map.MapValue(
					bytes.mtbuf.dfmt) << ','
					<< inst_buf_num_format_map.MapValue(
					bytes.mtbuf.nfmt) << ']';
		}
		else if (comm::Asm::isToken(fmt_str, "MIMG_SERIES_VDATA_SRC", token_len) ||
				comm::Asm::isToken(fmt_str, "MIMG_SERIES_VDATA_DST", token_len))
		{
			DumpVectorSeries(os, bytes.mimg.vdata,
					bytes.mimg.vdata + 3);
		}
		else if (comm::Asm::isToken(fmt_str, "MIMG_VADDR", token_len))
		{
			DumpVectorSeries(os, bytes.mimg.vaddr, 
					bytes.mimg.vaddr + 3);
		}
		else if (comm::Asm::isToken(fmt_str, "MIMG_SERIES_SRSRC", token_len))
		{
			assert((bytes.mimg.srsrc << 2) % 4 == 0);
			DumpScalarSeries(os, bytes.mimg.srsrc << 2, 
					(bytes.mimg.srsrc << 2) + 7);
		}
		else if (comm::Asm::isToken(fmt_str, "MIMG_DUG_SERIES_SRSRC", token_len))
		{
			assert((bytes.mimg.srsrc << 2) % 4 == 0);
			DumpScalarSeries(os, bytes.mimg.srsrc << 2, 
					(bytes.mimg.srsrc << 2) + 7);
			DumpDug(os);
		}
		else if (comm::Asm::isToken(fmt_str, "MIMG_SERIES_SSAMP", token_len))
		{
			assert((bytes.mimg.ssamp << 2) % 4 == 0);
			DumpScalarSeries(os, bytes.mimg.ssamp << 2, 
					(bytes.mimg.ssamp << 2) + 3);
		}
		else if (comm::Asm::isToken(fmt_str, "MIMG_DUG_SERIES_SSAMP", 
			token_len))
		{
			assert((bytes.mimg.ssamp << 2) % 4 == 0);
			DumpScalarSeries(os, bytes.mimg.ssamp << 2, 
					(bytes.mimg.ssamp << 2) + 3);
			DumpDug(os);
		}
		else if (comm::Asm::isToken(fmt_str, "TGT", token_len))
		{
			DumpOperandExp(os, bytes.exp.tgt);
		}
		else if (comm::Asm::isToken(fmt_str, "EXP_VSRCs", token_len))
		{
			if (bytes.exp.compr == 0 && 
					(bytes.exp.en && 0x0) == 0x0)
			{
				os << '[';
				DumpVector(os, bytes.exp.vsrc0);
				os << ' ';
				DumpVector(os, bytes.exp.vsrc1);
				os << ' ';
				DumpVector(os, bytes.exp.vsrc2);
				os << ' ';
				DumpVector(os, bytes.exp.vsrc3);
				os << ']';
			}
			else if (bytes.exp.compr == 1 && 
				(bytes.exp.en && 0x0) == 0x0)
			{
				os << '[';
				DumpVector(os, bytes.exp.vsrc0);
				os << ' ';
				DumpVector(os, bytes.exp.vsrc1);
				os << ']';
			}
		}
		else
		{
			throw misc::Panic(misc::fmt("%s: token not recognized.",
					fmt_str));
		}

		fmt_str += token_len;
	}
}


void Inst::Clear()
{
	info = NULL;
	bytes.dword = 0;
	size = 0;
	address = 0;
}


void Inst::Decode(const char *buf, unsigned int address)
{
	/* Initialize instruction */
	info = NULL;
	size = 4;
	bytes.word[0] = * (unsigned int *) buf;
	bytes.word[1] = 0;
	this->address = address;

	/* Use the encoding field to determine the instruction type */
	if (bytes.sopp.enc == 0x17F)
	{
		if (!as->getDecTableSopp(bytes.sopp.op))
		{
			throw misc::Panic(misc::fmt(
					"Unimplemented Instruction: SOPP:%d  "
					"// %08X: %08X\n", bytes.sopp.op,
					address, * (unsigned int *) buf));
		}

		info = as->getDecTableSopp(bytes.sopp.op);
	}
	else if (bytes.sopc.enc == 0x17E)
	{
		if (!as->getDecTableSopc(bytes.sopc.op))
		{
			throw misc::Panic(misc::fmt(
					"Unimplemented Instruction: SOPC:%d  "
					"// %08X: %08X\n", bytes.sopc.op,
					address, * (unsigned int *) buf));
		}

		info = as->getDecTableSopc(bytes.sopc.op);

		/* Only one source field may use a literal constant,
		 * which is indicated by 0xFF. */
		assert(!(bytes.sopc.ssrc0 == 0xFF &&
			bytes.sopc.ssrc1 == 0xFF));
		if (bytes.sopc.ssrc0 == 0xFF ||
			bytes.sopc.ssrc1 == 0xFF)
		{
			size = 8;
			bytes.dword = * (unsigned long long *) buf;
		}
	}
	else if (bytes.sop1.enc == 0x17D)
	{
		if (!as->getDecTableSop1(bytes.sop1.op))
		{
			throw misc::Panic(misc::fmt(
					"Unimplemented Instruction: SOP1:%d  "
					"// %08X: %08X\n", bytes.sop1.op,
					address, *(unsigned int *) buf));
		}

		info = as->getDecTableSop1(bytes.sop1.op);

		/* 0xFF indicates the use of a literal constant as a
		 * source operand. */
		if (bytes.sop1.ssrc0 == 0xFF)
		{
			size = 8;
			bytes.dword = * (unsigned long long *) buf;
		}
	}
	else if (bytes.sopk.enc == 0xB)
	{
		if (!as->getDecTableSopk(bytes.sopk.op))
		{
			throw misc::Panic(misc::fmt(
					"Unimplemented Instruction: SOPK:%d  "
					"// %08X: %08X\n", bytes.sopk.op,
					address, * (unsigned int *) buf));
		}

		info = as->getDecTableSopk(bytes.sopk.op);
	}
	else if (bytes.sop2.enc == 0x2)
	{
		if (!as->getDecTableSop2(bytes.sop2.op))
		{
			throw misc::Panic(misc::fmt(
					"Unimplemented Instruction: SOP2:%d  "
					"// %08X: %08X\n", bytes.sop2.op,
					address, *(unsigned int *) buf));
		}

		info = as->getDecTableSop2(bytes.sop2.op);

		/* Only one source field may use a literal constant,
		 * which is indicated by 0xFF. */
		assert(!(bytes.sop2.ssrc0 == 0xFF &&
			bytes.sop2.ssrc1 == 0xFF));
		if (bytes.sop2.ssrc0 == 0xFF ||
			bytes.sop2.ssrc1 == 0xFF)
		{
			size = 8;
			bytes.dword = * (unsigned long long *) buf;
		}
	}
	else if (bytes.smrd.enc == 0x18)
	{
		if (!as->getDecTableSmrd(bytes.smrd.op))
		{
			throw misc::Panic(misc::fmt(
					"Unimplemented Instruction: SMRD:%d  "
					"// %08X: %08X\n", bytes.smrd.op,
					address, *(unsigned int *) buf));
		}

		info = as->getDecTableSmrd(bytes.smrd.op);
	}
	else if (bytes.vop3a.enc == 0x34)
	{
		/* 64 bit instruction. */
		size = 8;
		bytes.dword = * (unsigned long long *) buf;

		if (!as->getDecTableVop3(bytes.vop3a.op))
		{
			throw misc::Panic(misc::fmt(
					"Unimplemented Instruction: VOP3:%d  "
					"// %08X: %08X %08X\n",
					bytes.vop3a.op, address,
					*(unsigned int *) buf,
					*(unsigned int *) (buf + 4)));
		}

		info = as->getDecTableVop3(bytes.vop3a.op);
	}
	else if (bytes.vopc.enc == 0x3E)
	{
		if (!as->getDecTableVopc(bytes.vopc.op))
		{
			throw misc::Panic(misc::fmt(
					"Unimplemented Instruction: VOPC:%d  "
					"// %08X: %08X\n",
					bytes.vopc.op, address,
					*(unsigned int *) buf));
		}

		info = as->getDecTableVopc(bytes.vopc.op);

		/* 0xFF indicates the use of a literal constant as a
		 * source operand. */
		if (bytes.vopc.src0 == 0xFF)
		{
			size = 8;
			bytes.dword = * (unsigned long long *) buf;
		}
	}
	else if (bytes.vop1.enc == 0x3F)
	{
		if (!as->getDecTableVop1(bytes.vop1.op))
		{
			throw misc::Panic(misc::fmt(
					"Unimplemented Instruction: VOP1:%d  "
					"// %08X: %08X\n", bytes.vop1.op,
					address, * (unsigned int *) buf));
		}

		info = as->getDecTableVop1(bytes.vop1.op);

		/* 0xFF indicates the use of a literal constant as a
		 * source operand. */
		if (bytes.vop1.src0 == 0xFF)
		{
			size = 8;
			bytes.dword = * (unsigned long long *) buf;
		}
	}
	else if (bytes.vop2.enc == 0x0)
	{
		if (!as->getDecTableVop2(bytes.vop2.op))
		{
			throw misc::Panic(misc::fmt(
					"Unimplemented Instruction: VOP2:%d  "
					"// %08X: %08X\n", bytes.vop2.op,
					address, * (unsigned int *) buf));
		}

		info = as->getDecTableVop2(bytes.vop2.op);

		/* 0xFF indicates the use of a literal constant as a
		 * source operand. */
		if (bytes.vop2.src0 == 0xFF)
		{
			size = 8;
			bytes.dword = * (unsigned long long *) buf;
		}

		/* Some opcodes define a 32-bit literal constant following
		 * the instruction */
		if (bytes.vop2.op == 32)
		{
			size = 8;
			bytes.dword = * (unsigned long long *) buf;
		}
	}
	else if (bytes.vintrp.enc == 0x32)
	{
		if (!as->getDecTableVintrp(bytes.vintrp.op))
		{
			throw misc::Panic(misc::fmt(
					"Unimplemented Instruction: VINTRP:%d  "
					"// %08X: %08X\n", bytes.vintrp.op,
					address, * (unsigned int *) buf));
		}

		info = as->getDecTableVintrp(bytes.vintrp.op);

	}
	else if (bytes.ds.enc == 0x36)
	{
		/* 64 bit instruction. */
		size = 8;
		bytes.dword = * (unsigned long long *) buf;
		if (!as->getDecTableDs(bytes.ds.op))
		{
			throw misc::Panic(misc::fmt(
					"Unimplemented Instruction: DS:%d  "
					"// %08X: %08X %08X\n", bytes.ds.op,
					address, *(unsigned int *)buf,
					*(unsigned int *) (buf + 4)));
		}

		info = as->getDecTableDs(bytes.ds.op);
	}
	else if (bytes.mtbuf.enc == 0x3A)
	{
		/* 64 bit instruction. */
		size = 8;
		bytes.dword = * (unsigned long long *) buf;

		if (!as->getDecTableMtbuf(bytes.mtbuf.op))
		{
			throw misc::Panic(misc::fmt(
					"Unimplemented Instruction: MTBUF:%d  "
					"// %08X: %08X %08X\n",
					bytes.mtbuf.op, address,
					*(unsigned int *) buf,
					*(unsigned int *) (buf+4)));
		}

		info = as->getDecTableMtbuf(bytes.mtbuf.op);
	}
	else if (bytes.mubuf.enc == 0x38)
	{
		/* 64 bit instruction. */
		size = 8;
		bytes.dword = * (unsigned long long *) buf;

		if (!as->getDecTableMubuf(bytes.mubuf.op))
		{
			throw misc::Panic(misc::fmt(
					"Unimplemented Instruction: MUBUF:%d  "
					"// %08X: %08X %08X\n",
					bytes.mubuf.op, address,
					*(unsigned int *) buf,
					*(unsigned int *) (buf+4)));
		}

		info = as->getDecTableMubuf(bytes.mubuf.op);
	}
	else if (bytes.mimg.enc == 0x3C)
	{
		/* 64 bit instruction. */
		size = 8;
		bytes.dword = * (unsigned long long *) buf;

		if(!as->getDecTableMimg(bytes.mimg.op))
		{
			throw misc::Panic(misc::fmt(
					"Unimplemented Instruction: MIMG:%d  "
					"// %08X: %08X %08X\n",
					bytes.mimg.op, address,
					*(unsigned int *) buf,
					*(unsigned int *) (buf + 4)));
		}

		info = as->getDecTableMimg(bytes.mimg.op);
	}
	else if (bytes.exp.enc == 0x3E)
	{
		/* 64 bit instruction. */
		size = 8;
		bytes.dword = * (unsigned long long *) buf;

		/* Export is the only instruction in its kind */
		if (!as->getDecTableExp(0))
			throw misc::Panic("Unimplemented Instruction: EXP\n");

		info = as->getDecTableExp(0);
	}
	else
	{
		throw misc::Panic(misc::fmt(
				"Unimplemented format. Instruction is:  "
				"// %08X: %08X\n",
				address, ((unsigned int*) buf)[0]));
	}
}


}  // namespace SI

