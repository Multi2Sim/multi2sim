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

#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>

#include <lib/cpp/Misc.h>

#include "Asm.h"
#include "Inst.h"


using namespace misc;
using namespace std;

namespace Fermi
{

/*
 * Class 'Inst'
 */

Inst::Inst(Asm *as)
{
	/* Initialize */
	this->as = as;
}


void Inst::Decode(unsigned int addr, const char *ptr)
{
	unsigned int cat;
	unsigned int func;

	/* Get instruction category bits */
	bytes.dword = * (unsigned long long *) ptr;
	cat = bytes.bytes[0] & 0xf;

	/* Get function bits */
	if (cat <= 3)
		func = bytes.bytes[7] >> 3;  /* 5-bit func */
	else
		func = bytes.bytes[7] >> 2;  /* 6-bit func */

	info = as->GetDecTable(cat, func);
	this->addr = addr;
}


StringMap inst_ftzfmz_map =
{
	{"", 0},
	{ ".FTZ", 1 },
	{ ".FMZ", 2 },
	{ NULL, 0 }
};

StringMap inst_rnd_map =
{
	{ "", 0},
	{ ".RM", 1 },
	{ ".RP", 2 },
	{ ".RZ", 3},
	{ NULL, 0 }
};

StringMap inst_rnd1_map =
{
	{ "", 0},
	{ ".FLOOR", 1 },
	{ ".CEIL", 2 },
	{ ".TRUNC", 3 },
	{ NULL, 0 }
};

StringMap inst_cmp_map =
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
	{ ".GEU", 14},
	{ NULL, 0 }
};

StringMap inst_round_map =
{
	{ "", 0},
	{ ".ROUND", 1 },
	{ NULL, 0 }
};

StringMap inst_sat_map =
{
	{ "", 0},
	{ ".SAT", 1 },
	{ NULL, 0 }
};

StringMap inst_logic_map =
{
	{ ".AND", 0},
	{ ".OR", 1},
	{ ".XOR", 2},
	{ 0, 0 }
};


StringMap inst_op_map =
{
	{ ".COS", 0},
	{ ".SIN", 1},
	{ ".EX2", 2},
	{ ".LG2", 3},
	{ ".RCP", 4},
	{ ".RSQ", 5},
	{ ".RCP64H", 6},
	{ ".RSQ64H", 7},
	{ 0, 0 }
};

StringMap inst_op56_map =
{
	{ ".POPC", 0},
	{ ".AND", 1},
	{ ".OR", 2},
	{ 0, 0 }
};

StringMap inst_op67_map =
{
	{ ".AND", 0},
	{ ".OR", 1},
	{ ".XOR", 2},
	{ ".PASS_B", 3},
	{ 0, 0 }
};

StringMap inst_dtype_n_map =
{
	{ "", 0},
	{ ".F16", 1},
	{ ".F32", 2},
	{ ".F64", 3},
	{ 0, 0, }
};

StringMap inst_dtype_map =
{
	{ "", 0},
	{ ".F16", 1},
	{ ".F32", 2},
	{ ".F64", 3},
	{ 0, 0 }
};

StringMap inst_stype_n_map =
{
	{ "", 0},
	{ "16", 1},
	{ "32", 2},
	{ "64", 3},
	{ 0, 0 }
};

StringMap inst_stype_map =
{
	{ "", 0},
	{ ".F16", 1},
	{ ".F32", 2},
	{ ".F64", 3},
	{ 0, 0 }
};

StringMap inst_type_map =
{
	{ ".U8", 0},
	{ ".S8", 1},
	{ ".U16", 2},
	{ ".S16", 3},
	{ "", 4},
	{ ".64", 5},
	{ ".128", 6},
	{ 0, 0 }
};

StringMap inst_cop_map =
{
	{ "", 0},
	{ ".CG", 1},
	{ ".CS", 2},
	{ ".CV", 3},
	{ 0, 0 }
};

StringMap inst_NOP_op_map =
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
	{ 0, 0 }
};

StringMap inst_ccop_map =  // more needs to be added (look over fermi isa NOP)
{
	{ ".F", 0 },
	{ ".LT", 1 },
	{ ".EQ", 2 },
	{ ".T", 15 },
	{ 0, 0 }
};


StringMap inst_sreg_map =
{
	{ "SR_Laneld", InstSRegLaneld },
	{ "SR_VirtCfg", InstSRegVirtCfg },
	{ "SR_VirtId", InstSRegVirtId },
	{ "SR_PM0", InstSRegPM0 },
	{ "SR_PM1", InstSRegPM1 },
	{ "SR_PM2", InstSRegPM2 },
	{ "SR_PM3", InstSRegPM3 },
	{ "SR_PM4", InstSRegPM4 },
	{ "SR_PM5", InstSRegPM5 },
	{ "SR_PM6", InstSRegPM6 },
	{ "SR_PM7", InstSRegPM7 },
	{ "SR_PRIM_TYPE", InstSRegPrimType },
	{ "SR_INVOCATION_ID", InstSRegInvocationID },
	{ "SR_Y_DIRECTION", InstSRegYDirection },
	{ "SR_MACHINE_ID_0", InstSRegMachineID0 },
	{ "SR_MACHINE_ID_1", InstSRegMachineID1 },
	{ "SR_MACHINE_ID_2", InstSRegMachineID2 },
	{ "SR_MACHINE_ID_3", InstSRegMachineID3 },
	{ "SR_AFFINITY", InstSRegAffinity },
	{ "SR_TID", InstSRegTid },
	{ "SR_TID.X", InstSRegTidX },
	{ "SR_TID.Y", InstSRegTidY },
	{ "SR_TID.Z", InstSRegTidZ },
	{ "SR_CTAParam", InstSRegCTAParam },
	{ "SR_CTAID.X", InstSRegCTAidX },
	{ "SR_CTAID.Y", InstSRegCTAidY },
	{ "SR_CTAID.Z", InstSRegCTAidZ },
	{ "SR_NTID", InstSRegNTid },
	{ "SR_NTID.X", InstSRegNTidX },
	{ "SR_NTID.Y", InstSRegNTidY },
	{ "SR_NTID.Z", InstSRegNTidZ },
	{ "SR_GridParam", InstSRegGridParam },
	{ "SR_NCTAID.X", InstSRegNCTAidX },
	{ "SR_NCTAID.Y", InstSRegNCTAidY },
	{ "SR_NCTAID.Z", InstSRegNCTAidZ },
	{ "SR_SWinLo", InstSRegSWinLo },
	{ "SR_SWINSZ", InstSRegSWINSZ },
	{ "SR_SMemSz", InstSRegSMemSz },
	{ "SR_SMemBanks", InstSRegSMemBanks },
	{ "SR_LWinLo", InstSRegLWinLo },
	{ "SR_LWINSZ", InstSRegLWINSZ },
	{ "SR_LMemLoSz", InstSRegLMemLoSz },
	{ "SR_LMemHiOff", InstSRegLMemHiOff },
	{ "SR_EqMask", InstSRegEqMask },
	{ "SR_LtMask", InstSRegLtMask },
	{ "SR_LeMask", InstSRegLeMask },
	{ "SR_GtMask", InstSRegGtMask },
	{ "SR_GeMask", InstSRegGeMask },
	{ "SR_ClockLo", InstSRegClockLo },
	{ "SR_ClockHi", InstSRegClockHi },
	{ NULL, 0 }
};


void Inst::DumpPC(ostream &os)
{
	os << "/*" << setfill('0') << setw(4) << right << hex << addr << "*/";
}


void Inst::DumpPred(ostream &os)
{
	const char *fmt_str;
	int len;

	/* Check if instruction is supported */
	if (!info)
		fatal("%s: instruction not supported (offset=0x%x)",
			__FUNCTION__, addr);

	/* Store copy of format string and original destination buffer */
	fmt_str = info->fmt_str.c_str();

	if (*fmt_str == '%' && Common::Asm::IsToken(++fmt_str, "pred", len))
	{
		unsigned long long int pred;
		pred = bytes.general0.pred;
		if (pred < 7)
			os << "    @P" << pred << " ";
		else if (pred > 7)
			os << "   @!P" << pred - 8 << " ";
		else
			os << setfill(' ') << setw(8) << "";
	}
	else
		os << setfill(' ') << setw(8) << "";
}


void Inst::DumpToBuf(void)
{
	const char *fmt_str;
	int len;
	int size = 200;

	/* Check if instruction is supported */
	if (!info)
		fatal("%s: instruction not supported (offset=0x%x)",
			__FUNCTION__, addr);

	/* Store copy of format string and original destination buffer */
	fmt_str = info->fmt_str.c_str();

	/* Process format string */
	while (*fmt_str && size)
	{
		/* Character is a literal symbol */
		if (*fmt_str != '%')
		{
			if (fmt_str == info->fmt_str.c_str())
				str += "        ";
			str += *fmt_str;
			++fmt_str;
			continue;
		}

		/* Character is a token */
		++fmt_str;

		/* 1st level token such as pred, dst, src1, src2, src2_mod, imm, offs*/
		if (Common::Asm::IsToken(fmt_str, "pred", len))
		{
			unsigned long long int pred;
			pred = bytes.general0.pred;
			if (pred < 7)
				str += "    @P" + to_string(pred);
			else if (pred > 7)
				str += "   @!P" + to_string(pred - 8);
			else
				str += "       ";
		}
		else if (Common::Asm::IsToken(fmt_str,"dst", len))
		{
			unsigned long long int dst;
			dst = bytes.general0.dst;
			if (dst != 63)
				str += "R" + to_string(dst);
			else
				str += "RZ";
		}

		else if (Common::Asm::IsToken(fmt_str,"src1_abs_neg", len))
		{
			unsigned long long int src1;
			unsigned long long int src1_abs;

			src1 = bytes.general0.src1;
			src1_abs = bytes.mod0_A.abs_src1;
			if (src1 != 63 && src1_abs == 1)
				str += "|R" + to_string(src1) + "|";
			else if (src1 != 63 && src1_abs == 0)
				str += "R" + to_string(src1);
			else
				str += "RZ";
		}

		else if (Common::Asm::IsToken(fmt_str, "src1_neg", len))
		{
			unsigned long long int src1;
			unsigned long long int src_mod;

			src1 = bytes.general0.src1;
			src_mod = bytes.mod0_D.fma_mod;
			if (src1 != 63)
			{
				if (src_mod == 0x2)
					str += "-R" + to_string(src1);
				else
					str += "R" + to_string(src1);
			}
			else
				str += "RZ";
		}

		else if (Common::Asm::IsToken(fmt_str, "src1_offs", len))
		{
			unsigned long long int src1;
			unsigned long long int offs;

			src1 = bytes.general0.src1;
			offs = bytes.offs.offset;

			if (src1 != 63)
			{
				str += "[R" + to_string(src1);

				if (offs != 0)
				{
					stringstream ss;
					ss << "+0x"  << hex << offs;
					str += ss.str();
				}
			}

			else
			{
				stringstream ss;
				ss << "[0x"  << hex << offs;
				str += ss.str();
			}

			str += "]";
		}

		else if (Common::Asm::IsToken(fmt_str,"src1", len))
		{
			unsigned long long int src1;

			src1 = bytes.general0.src1;
			if (src1 != 63)
				str += "R" + to_string(src1);
			else
				str += "RZ";
		}

		else if (Common::Asm::IsToken(fmt_str,"src23", len))
		{
			unsigned long long int bank_id;
			unsigned long long int offset_in_bank;
			unsigned long long int src3;
			unsigned long long int src2_neg;
			unsigned long long int src3_neg;

			src3 = bytes.general0_mod1_B.src3;
			src2_neg = bytes.mod0_A.neg_src1;
			src3_neg = bytes.mod0_A.neg_src2;

			if (bytes.general0.src2_mod == 0)
			{
				if (bytes.general0.src2 != 0x3f)
				{
					if (src2_neg)
						str += "-R" + to_string(bytes.general0.src2 & 0x3f) + ", ";
					else
						str += "R" + to_string(bytes.general0.src2 & 0x3f) + ", ";
				}
				else
					str += "RZ, ";
				if (src3 != 63)
				{
					if (src3_neg)
						str += "-R" + to_string(src3);
					else
						str += "R" + to_string(src3);
				}
				else
					str += "RZ";
			}
			else if (bytes.general0.src2_mod == 1)
			{
				bank_id = ((bytes.general0.src2 & 0x1) << 4) | 
					(bytes.general0.src2 >> 16);
				offset_in_bank= bytes.general0.src2 &
					0xfffc;
				stringstream ss;
				ss << "c[0x" << hex << bank_id << "][0x" << hex << offset_in_bank << "], ";
				str += ss.str();
				if (src3 != 63)
				{
					if (src3_neg)
						str += "-R" + to_string(src3);
					else
						str += "R" + to_string(src3);
				}
				else
					str += "RZ";
			}
			else if (bytes.general0.src2_mod == 2)
			{
				if (src3 != 63)
				{
					if (src3_neg)
						str += "-R" + to_string(src3) + ", ";
					else
						str += "R" + to_string(src3) + ", ";
				}
				else
					str += "RZ, ";
				bank_id = ((bytes.general0.src2 & 0x1) << 4) | 
					(bytes.general0.src2 >> 16);
				offset_in_bank= bytes.general0.src2 &
					0xfffc;
				stringstream ss;
				ss << "c[0x" << hex << bank_id << "][0x" << hex << offset_in_bank << "]";
				str += ss.str();
			}
			else if (bytes.general0.src2_mod == 3)
			{
				stringstream ss;
				ss << "0x" << hex << bytes.general0.src2 << ", ";
				str += ss.str();
				if (src3 != 63)
				{
					if (src3_neg)
						str += "-R" + to_string(src3);
					else
						str += "R" + to_string(src3);
				}
				else
					str += "RZ";
			}
		}

		/* This is a special case for src2 and src3. For FFMA,
		   the sequence of output from cuobjdump is somehow depends on the src2_mod
		   it prints src3 first when src2_mod > 2, however prints src2 first when src2_mod < 2 */
		else if (Common::Asm::IsToken(fmt_str,"src2_FFMA", len))
		{
			unsigned long long int bank_id;
			unsigned long long int offset_in_bank;
			unsigned long long int src3;

			bank_id = bytes.general0.src2 >> 16;
			offset_in_bank= bytes.general0.src2 & 0xffff;
			src3 = bytes.general0_mod1_B.src3;

			/* print out src2 */
			if (bytes.general0.src2_mod < 2)
			{
				if (bytes.general0.src2_mod == 0)
					str += "R" + to_string(bytes.general0.src2 & 0x3f);
				else if (bytes.general0.src2_mod == 1)
				{
					stringstream ss;
					ss << "c[0x" << hex << bank_id << "][0x" << hex << offset_in_bank << "]";
					str += ss.str();
				}
			}

			/* print out src3 */
			else
			{
				if (src3 != 63)
					str += ", R" + to_string(src3);
				else
					str += ", RZ";
			}
		}

		/* This is a special case for src2. 
		 * For LDC, src2 is always a constant memory address, no matter
		 * what the value of src2_mod is. */
		else if (Common::Asm::IsToken(fmt_str,"src2_LDC", len))
		{
			unsigned long long int bank_id;
			unsigned long long int src1;
			unsigned long long int offset_in_bank;

			bank_id = bytes.general0.src2 >> 16;
			src1= bytes.general0.src1;
			offset_in_bank= bytes.general0.src2 & 0xffff;

			/* print out src2 */
			stringstream ss;
			ss << "c[0x" << hex << bank_id << "][R" << src1;
			if (offset_in_bank != 0)
				ss << "+0x" << hex << offset_in_bank;
			ss << "]";
			str += ss.str();
		}

		else if (Common::Asm::IsToken(fmt_str,"src2_frm_sr", len))
		{
			const char *sreg;
			sreg = StringMapValue(inst_sreg_map, bytes.general0.src2 & 0xff);
			str += sreg;

		}

		else if (Common::Asm::IsToken(fmt_str,"src2_abs_neg", len))
		{
			unsigned long long int src2;
			unsigned long long int src2_abs;

			src2 = bytes.general0.src2;
			src2_abs = bytes.mod0_A.abs_src2;
			if (src2 != 63 && src2_abs == 1)
				str += "|R" + to_string(src2) + "|";
			else if (src2 != 63 && src2_abs == 0)
				str += "R" + to_string(src2);
			else
				str += "RZ";
		}

		else if (Common::Asm::IsToken(fmt_str,"src2_neg", len))
		{
			unsigned long long int bank_id;
			unsigned long long int offset_in_bank;

			stringstream ss;
			if (bytes.general0.src2_mod == 0)
			{
				if (bytes.mod0_A.neg_src2)
					str += "-";
				str += "R" + to_string(bytes.general0.src2 & 0x3f);
			}
			else if (bytes.general0.src2_mod == 1)
			{
				bank_id = bytes.general0.src2 >> 16;
				offset_in_bank= bytes.general0.src2 & 0xffff;
				ss << "c[0x" << hex << bank_id << "][0x" << hex << offset_in_bank << "]";
				str += ss.str();
			}
			else if (bytes.general0.src2_mod == 2)
			{
				ss << "0x" << hex << bytes.general0.src2;
				str += ss.str();
			}
			else if (bytes.general0.src2_mod == 3)
			{
				ss << "0x" << hex << bytes.general0.src2;
				str += ss.str();
			}
		}

		else if (Common::Asm::IsToken(fmt_str,"src2", len))
		{
			unsigned long long int bank_id;
			unsigned long long int offset_in_bank;

			stringstream ss;
			if (bytes.general0.src2_mod == 0)
			{
				if (bytes.general0.src2 != 0x3f)
					str += "R" + to_string(bytes.general0.src2 & 0x3f);
				else
					str += "RZ";
			}
			else if (bytes.general0.src2_mod == 1 || 
					bytes.general0.src2_mod == 2)
			{
				bank_id = ((bytes.general0.src2 & 0x1) << 4) | 
					(bytes.general0.src2 >> 16);
				offset_in_bank= bytes.general0.src2 &
					0xfffc;
				ss << "c[0x" << hex << bank_id << "][0x" << hex << offset_in_bank << "]";
				str += ss.str();
			}
			else if (bytes.general0.src2_mod == 3)
			{
				ss << "0x" << hex << bytes.general0.src2;
				str += ss.str();
			}
		}

		else if (Common::Asm::IsToken(fmt_str, "src3_FFMA", len))
		{
			unsigned long long int src3;
			unsigned long long int bit26;
			unsigned long long int bit28;

			src3 = bytes.general0_mod1_B.src3;
			bit26 = bytes.general0.src2 & 0x1;
			bit28 = bytes.general0.src2 >> 2 &0x1;

			/* print out src2 */
			if (bytes.general0.src2_mod < 2)
			{
				if (src3 != 63)
					str += "R" + to_string(src3);
				else
					str += "RZ";
			}

			else
			{
				/* FIXME : we need to figure out how bit26 and bit28 control src2*/
				if (bit26 == 0 && bit28 == 0)
					str += "c[0x0][0x0]";
				else if (bit26 == 1 && bit28 == 0)
					str += "c[0x10][0x0]";
				else if (bit26 == 0 && bit28 == 1)
					str += "c[0x0][0x4]";
				else
					str += "c[0x10][0x4]";
			}
		}

		else if (Common::Asm::IsToken(fmt_str,"src3", len))
		{
			unsigned long long int src3;
			src3 = bytes.general0_mod1_B.src3;
			if (src3 != 63)
				str += "R" + to_string(src3);
			else
				str += "RZ";
		}

		else if (Common::Asm::IsToken(fmt_str,"tgt_lmt", len))
		{
			if (bytes.tgt.noinc)
				str += ".LMT";
		}

		else if (Common::Asm::IsToken(fmt_str,"tgt_noinc", len))
		{
			if (!bytes.tgt.noinc)
				str += ".noinc";
		}

		else if (Common::Asm::IsToken(fmt_str,"tgt_u", len))
		{
			if (bytes.tgt.u)
				str += ".U";
		}

		else if (Common::Asm::IsToken(fmt_str,"abs_target", len))
		{
			unsigned long long int target;

			target = bytes.tgt.target;
			stringstream ss;
			ss << "0x" << hex << target;
			str += ss.str();
		}

		else if (Common::Asm::IsToken(fmt_str,"tgt", len))
		{
			unsigned long long int target;

			target = bytes.tgt.target;
			target = SignExtend64(target, 24);
			target += addr + 8;
			stringstream ss;
			ss << "0x" << hex << target;
			str += ss.str();
		}

		else if (Common::Asm::IsToken(fmt_str,"imm32", len))
		{
			long long int imm32;
			/* FIXME
			 * Careful - We need to check whether each instruction encodes the immediate
			 * value as a signed or unsigned number. */
				imm32 = (signed)bytes.imm.imm32;
			/* Print sign */
			if (imm32 < 0)
			{
				str += "-";
				imm32 = -imm32;
			}
			stringstream ss;
			ss << "0x" << hex << imm32;
			str += ss.str();
		}

		else if (Common::Asm::IsToken(fmt_str,"gen1_cmp", len))
		{
			str += StringMapValue(inst_cmp_map, bytes.general1.cmp);
		}

		else if (Common::Asm::IsToken(fmt_str,"gen1_logicftz", len))
		{
			str += ".FTZ";
			str += StringMapValue(inst_logic_map, bytes.general1.logic);
		}

		else if (Common::Asm::IsToken(fmt_str,"gen1_logic", len))
		{
			str += StringMapValue(inst_logic_map, bytes.general1.logic);
		}

		/* 2nd level token such as mod0, mod1, P, Q*/
		else if (Common::Asm::IsToken(fmt_str, "mod0_A_ftz", len))
		{
			if (bytes.mod0_A.satftz)
				str += ".FTZ";
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_A_op", len))
		{
			unsigned long long int op;
			op = bytes.mod0_A.abs_src2 << 1 || bytes.mod0_A.satftz;
			str += StringMapValue(inst_op56_map,op);
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_A_redarv", len))
		{
			if (bytes.mod0_A.abs_src1)
				str += ".ARV";
			else
				str += ".RED";
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_A_s", len))
		{
			if (bytes.mod0_A.s)
				str += ".S";
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_A_u32", len))
		{
			if (!bytes.mod0_A.satftz)
				str += ".U32";
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_A_w", len))
		{
			if (bytes.mod0_A.neg_src1)
				str += ".W";
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_B_brev", len))
		{
			if (bytes.mod0_B.cop)
				str += ".brev";
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_B_cop", len))
		{
			str += StringMapValue(inst_cop_map, bytes.mod0_B.cop);
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_B_type", len))
		{
			str += StringMapValue(inst_type_map, bytes.mod0_B.type);
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_B_u32", len))
		{
			if (!bytes.mod0_B.type)
				str += ".U32";
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_C_ccop", len))
		{
			str += StringMapValue(inst_ccop_map, bytes.mod0_C.shamt);
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_C_shamt", len))
		{
			unsigned long long int shamt;
			shamt = bytes.mod0_C.shamt;
			stringstream ss;
			ss << "0x" << hex << shamt;
			str += ss.str();
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_C_s", len))
		{
			if (bytes.mod0_C.s)
				str += ".S";
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_D_ftzfmz", len))
		{
			str += StringMapValue(inst_ftzfmz_map, bytes.mod0_D.ftzfmz);
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_D_op67", len))
		{
			str += StringMapValue(inst_op67_map, bytes.mod0_D.ftzfmz);
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_D_op", len))
		{
			str += StringMapValue(inst_op_map, bytes.mod0_D.ftzfmz);
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_D_round", len))
		{
			str += StringMapValue(inst_round_map, bytes.mod0_D.ftzfmz >> 1);
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_D_sat", len))
		{
			str += StringMapValue(inst_sat_map, bytes.mod0_D.sat);
		}

		else if (Common::Asm::IsToken(fmt_str, "mod0_D_x", len))
		{
			unsigned long long int x;
			x = bytes.mod0_D.ftzfmz & 0x1;
			if (x)
				str += ".X";
		}

		else if (Common::Asm::IsToken(fmt_str, "gen0_mod1_B_rnd", len))
		{
			str += StringMapValue(inst_rnd_map, bytes.general0_mod1_B.rnd);
		}

		else if (Common::Asm::IsToken(fmt_str, "gen0_mod1_C_rnd", len))
		{
			str += StringMapValue(inst_rnd1_map, bytes.general0_mod1_C.rnd);
		}

		else if (Common::Asm::IsToken(fmt_str, "gen0_mod1_D_cmp", len))
		{
			str += StringMapValue(inst_cmp_map, bytes.general0_mod1_D.cmp);
		}

		else if (Common::Asm::IsToken(fmt_str, "gen0_src1_dtype_n", len))
		{
			unsigned long long int dtype_n;
			dtype_n = bytes.general0.src1 & 0x3;
			str += StringMapValue(inst_dtype_n_map, dtype_n);
		}

		else if (Common::Asm::IsToken(fmt_str, "gen0_src1_dtype", len))
		{
			unsigned long long int dtype;
			dtype = bytes.general0.src1 & 0x3;
			str += StringMapValue(inst_dtype_map, dtype);
		}

		else if (Common::Asm::IsToken(fmt_str, "gen0_src1_stype_n", len))
		{
			unsigned long long int stype_n;
			stype_n = bytes.general0.src1 >> 3 & 0x3;
			str += StringMapValue(inst_stype_n_map, stype_n);
		}

		else if (Common::Asm::IsToken(fmt_str, "gen0_src1_stype", len))
		{
			unsigned long long int stype;
			stype = bytes.general0.src1 >> 3 & 0x3;
			str += StringMapValue(inst_stype_map, stype);
		}

		else if (Common::Asm::IsToken(fmt_str, "dtype_sn", len))
		{
			unsigned long long int dtype_n;
			if (bytes.mod0_A.abs_src1)
			        str += ".S";
			else
			        str += ".U";
			dtype_n = bytes.general0.src1 & 0x3;
			str += StringMapValue(inst_stype_n_map, dtype_n);
		}

		else if (Common::Asm::IsToken(fmt_str, "stype_sn", len))
		{
			unsigned long long int stype_n;
			if (bytes.mod0_A.neg_src1)
			        str += ".S";
		        else
			        str += ".U";
			stype_n = bytes.general0.src1 >> 3 & 0x3;
			str += StringMapValue(inst_stype_n_map, stype_n);
		}

		else if (Common::Asm::IsToken(fmt_str, "offs_op1_e", len))
		{
			unsigned long long int e;
			e = bytes.offs.op1 & 0x1;
			if (e)
				str += ".e";
		}

		else if (Common::Asm::IsToken(fmt_str,"offs", len))
		{
			unsigned long long int offs;
			offs = bytes.offs.offset;
			stringstream ss;
			if (offs)
				ss << "+0x" << hex << offs;
			str += ss.str();
		}

		else if (Common::Asm::IsToken(fmt_str, "P", len))
		{
			unsigned long long int P;
			P = bytes.general1.dst >> 3;
			str += "P" + to_string(P);
		}

		else if (Common::Asm::IsToken(fmt_str, "Q", len))
		{
			unsigned long long int Q;
			Q = bytes.general1.dst & 0x7;
			if (Q != 7)
				str += "P" + to_string(Q);
			else
				str += "PT";
		}

		else if (Common::Asm::IsToken(fmt_str, "R", len))
		{
			unsigned long long int R;
			R = bytes.general1.R;
			if (R < 7)
				str += "P" + R;
			else if (R == 8)
				str += "!P0";
			else
				str += "PT";
		}

		else if (Common::Asm::IsToken(fmt_str, "FADD_sat", len))
		{
			unsigned long long int sat;
			sat = bytes.general0_mod1_B.src3 & 0x1;
			if (sat)
				str += ".SAT";
		}

		else if (Common::Asm::IsToken(fmt_str, "MUFU_op", len))
		{
			unsigned long long int op;
			op = bytes.imm.imm32;
			str += StringMapValue(inst_op_map, op);
		}

		else if (Common::Asm::IsToken(fmt_str, "NOP_op", len))
		{
			unsigned long long int op;
			op = bytes.offs.mod1 >> 9 & 0x4;
			str += StringMapValue(inst_NOP_op_map, op);
		}

		else if (Common::Asm::IsToken(fmt_str, "IMAD_hi", len))
		{
			unsigned long long int hi;
			hi= bytes.mod0_D.ftzfmz & 0x1;
			if (hi)
				str += ".HI";
		}

		else if (Common::Asm::IsToken(fmt_str, "IMAD_mod1", len))
		{
			unsigned long long int mod1;
			mod1 = bytes.mod0_D.ftzfmz >> 1;
			if (!mod1)
				str += ".U32";
		}

		else if (Common::Asm::IsToken(fmt_str, "IMAD_mod2", len))
		{
			unsigned long long int mod2;
			mod2 = bytes.mod0_D.sat;
			if (!mod2)
				str += ".U32";
		}

		else if (Common::Asm::IsToken(fmt_str, "IMAD_sat", len))
		{
			unsigned long long int sat;
			sat = bytes.general0_mod1_B.rnd >> 1;
			if (sat)
				str += ".SAT";
		}
		else
		{
			fatal("%s: unknown token: %s", __FUNCTION__,
					fmt_str);
		}

		/* Skip processed token */
		fmt_str += len;
	}
	str += ";";
}


void Inst::Dump(ostream &os, unsigned int max_inst_len)
{
	os << setfill(' ') << setw(max_inst_len + 1) << left;
	os << str;
}


void Inst::DumpHex(ostream &os)
{
	os << "/* 0x";
	os << setfill('0') << setw(8) << hex << bytes.dword;
	os << " */";
}

}  /* namespace Fermi */


/*
 * C Wrapper
 */

struct FrmInstWrap *FrmInstWrapCreate(struct FrmAsmWrap *as)
{
	return (FrmInstWrap *) new Fermi::Inst((Fermi::Asm *) as);
}


void FrmInstWrapFree(struct FrmInstWrap *self)
{
	delete (Fermi::Inst *) self;
}


void FrmInstWrapCopy(struct FrmInstWrap *left, struct frmInstWrap *right)
{
	Fermi::Inst *ileft = (Fermi::Inst *) left;
	Fermi::Inst *iright = (Fermi::Inst *) right;
	*ileft = *iright;
}


void FrmInstWrapDecode(struct FrmInstWrap *self, unsigned int addr, void *ptr)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	inst->Decode(addr, (const char *) ptr);
}


FrmInstBytes *FrmInstWrapGetBytes(struct FrmInstWrap *self)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	return (FrmInstBytes *) inst->GetBytes();
}

const char *FrmInstWrapGetName(struct FrmInstWrap *self)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	return inst->GetName().c_str();
}


FrmInstOpcode FrmInstWrapGetOpcode(struct FrmInstWrap *self)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	return (FrmInstOpcode) inst->GetOpcode();
}


FrmInstCategory FrmInstWrapGetCategory(struct FrmInstWrap *self)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	return (FrmInstCategory) inst->GetCategory();
}
