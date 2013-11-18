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

#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <typeinfo>

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
	unsigned int func_idx;

	/* Get instruction category bits */
	bytes.dword = * (unsigned long long *) ptr;
	cat = bytes.bytes[0] & 0xf;

	/* Get function bits */
	if (cat <= 3)
		func = bytes.bytes[7] >> 3;  /* 5-bit func */
	else
		func = bytes.bytes[7] >> 2;  /* 6-bit func */
	func_idx = func;

	/* Special cases */
	if (cat == 5 && ((func & 0x30) >> 4) == 0)  /* RED */
		func_idx = func & 0x30;
	else if (cat == 5 && ((func & 0x30) >> 4) == 1)  /* ATOM */
		func_idx = func & 0x30;

	info = as->GetDecTable(cat, func_idx);
	this->addr = addr;
}


map<string, int> atom_op_map =
{
	{ ".ADD", 0 },
	{ ".MIN", 1 },
	{ ".MAX", 2 },
	{ ".INC", 3 },
	{ ".DEC", 4 },
	{ ".AND", 5 },
	{ ".OR", 6 },
	{ ".XOR", 7 },
	{ ".EXCH", 8 },
	{ ".CAS", 9 }
};

map<string, int> atom_type_map =
{
	{ ".U64", 5 },
	{ ".S32", 7 },
	{ ".F32.FTZ.RN", 11 }
};

map<string, int> b2r_op_map =
{
	{ ".XLU", 1 },
	{ ".ALU", 2 }
};

map<string, int> bar_op_map =
{
	{ ".POPC", 0 },
	{ ".AND", 1 },
	{ ".OR", 2 }
};

map<string, int> cc_op_map =
{
	{ ".F", 0 },
	{ ".LT", 1 },
	{ ".EQ", 2 },
	{ ".LE", 3 },
	{ ".GT", 4 },
	{ ".NE", 5 },
	{ ".GE", 6 },
	{ ".NUM", 7 },
	{ ".NAN", 8 },
	{ ".LTU", 9 },
	{ ".EQU", 10 },
	{ ".LEU", 11 },
	{ ".GTU", 12 },
	{ ".NEU", 13 },
	{ ".GEU", 14 },
	{ ".T", 15 },
	{ ".OFF", 16 },
	{ ".LO", 17 },
	{ ".SFF", 18 },
	{ ".LS", 19 },
	{ ".HI", 20 },
	{ ".SFT", 21 },
	{ ".HS", 22 },
	{ ".OFT", 23 },
	{ ".CSM_TA", 24 },
	{ ".CSM_TR", 25 },
	{ ".CSM_MX", 26 },
	{ ".FCSM_TA", 27 },
	{ ".FCSM_TR", 28 },
	{ ".FCSM_MX", 29 },
	{ ".RLE", 30 },
	{ ".RGT", 31 }
};

map<string, int> btoff03_map =
{
	{ "", 0 },
	{ ".AOFFI", 1 },
	{ ".PTP", 2 }
};

map<string, int> cc_cop_map =
{
	{ ".QRY1", 0 },
	{ ".PF1", 1 },
	{ ".PF1_5", 2 },
	{ ".PR2", 3 },
	{ ".WB", 4 },
	{ ".IV", 5 },
	{ ".IVALL", 6 },
	{ ".RS", 7 }
};

map<string, int> cctl_op_map =
{
	{ ".U", 1 },
	{ ".C", 2 },
	{ ".I", 3 }
};

map<string, int> cmp_map =
{
	{ ".LT", 1 },
	{ ".EQ", 2 },
	{ ".LE", 3 },
	{ ".GT", 4 },
	{ ".NE", 5 },
	{ ".GE", 6 },
	{ ".NUM", 7 },
	{ ".NAN", 8 },
	{ ".LTU", 9 },
	{ ".EQU", 10 },
	{ ".LEU", 11 },
	{ ".GTU", 12 },
	{ ".NEU", 13 },
	{ ".GEU", 14 }
};

map<string, int> ftype_map =
{
	{ ".F16", 1 },
	{ ".F32", 2 },
	{ ".F64", 3 }
};

map<string, int> ftzfmz_map =
{
	{ ".FTZ", 1 },
	{ ".FMZ", 2 },
	{ ".INVALID", 3 }
};

map<string, int> geom_map =
{
	{ "1D", 0 },
	{ "ARRAY_1D", 1 },
	{ "2D", 2 },
	{ "ARRAY_2D", 3 },
	{ "3D", 4 },
	{ "ARRAY_3D", 5 },
	{ "CUBE", 6 },
	{ "ARRAY_CUBE", 7 }
};

map<string, int> itype_map =
{
	{ "16", 1 },
	{ "32", 2 },
	{ "64", 3 }
};

map<string, int> ld_cop_map =
{
	{ ".CG", 1 },
	{ ".CS", 2 },
	{ ".CV", 3 }
};

map<string, int> ldx_cop_map =
{
	{ ".CA", 0 },
	{ ".CG", 1 },
	{ ".LU", 2 },
	{ ".CV", 3 }
};

map<string, int> logic_map =
{
	{ ".AND", 0 },
	{ ".OR", 1 },
	{ ".XOR", 2 }
};

map<string, int> lop_op_map =
{
	{ ".AND", 0 },
	{ ".OR", 1 },
	{ ".XOR", 2 },
	{ ".PASS_B", 3 }
};

map<string, int> lvl_map =
{
	{ ".CTA", 0 },
	{ ".GL", 1 },
	{ ".SYS", 2 }
};

map<string, int> mufu_op_map =
{
	{ ".COS", 0 },
	{ ".SIN", 1 },
	{ ".EX2", 2 },
	{ ".LG2", 3 },
	{ ".RCP", 4 },
	{ ".RSQ", 5 },
	{ ".RCP64H", 6 },
	{ ".RSQ64H", 7 }
};

map<string, int> nop_op_map =
{
	{ ".FMA64", 1 },
	{ ".FMA32", 2 },
	{ ".XLU", 3 },
	{ ".ALU", 4 },
	{ ".AGU", 5 },
	{ ".SU", 6 },
	{ ".FU", 7 },
	{ ".FMUL", 8 }
};

map<string, int> phase3_map =
{
	{ "", 0 },
	{ ".T", 1 },
	{ ".P", 2 }
};

map<string, int> red_op_map =
{
	{ ".ADD", 0 },
	{ ".MIN", 1 },
	{ ".MAX", 2 },
	{ ".INC", 3 },
	{ ".DEC", 4 },
	{ ".AND", 5 },
	{ ".OR", 6 },
	{ ".XOR", 7 }
};

map<string, int> red_type_map =
{
	{ ".U64", 5 },
	{ ".S32", 7 },
	{ ".F32.FTZ.RN", 11 }
};

map<string, int> rnd_f_map =
{
	{ ".FLOOR", 1 },
	{ ".CEIL", 2 },
	{ ".TRUNC", 3 }
};

map<string, int> rnd_i_map =
{
	{ ".RM", 1 },
	{ ".RP", 2 },
	{ ".RZ", 3 }
};

map<string, int> sreg_map =
{
	{ "_LANEID", 0 },
	{ "_CLOCK", 1 },
	{ "_VIRTCFG", 2 },
	{ "_VIRTID", 3 },
	{ "_PM0", 4 },
	{ "_PM1", 5 },
	{ "_PM2", 6 },
	{ "_PM3", 7 },
	{ "_PM4", 8 },
	{ "_PM5", 9 },
	{ "_PM6", 10 },
	{ "_PM7", 11 },
	{ "_PRIM_TYPE", 16 },
	{ "_INVOCATION_ID", 17 },
	{ "_Y_DIRECTION", 18 },
	{ "_THREAD_KILL", 19 },
	{ "_SHADER_TYPE", 20 },
	{ "_MACHINE_ID_0", 24 },
	{ "_MACHINE_ID_1", 25 },
	{ "_MACHINE_ID_2", 26 },
	{ "_MACHINE_ID_3", 27 },
	{ "_AFFINITY", 28 },
	{ "_TID", 32 },
	{ "_TID.X", 33 },
	{ "_TID.Y", 34 },
	{ "_TID.Z", 35 },
	{ "_CTA_PARAM", 36 },
	{ "_CTAID.X", 37 },
	{ "_CTAID.Y", 38 },
	{ "_CTAID.Z", 39 },
	{ "_NTID", 40 },
	{ "_NTID.X", 41 },
	{ "_NTID.Y", 42 },
	{ "_NTID.Z", 43 },
	{ "_GRIDPARAM", 44 },
	{ "_NCTAID.X", 45 },
	{ "_NCTAID.Y", 46 },
	{ "_NCTAID.Z", 47 },
	{ "_SWINLO", 48 },
	{ "_SWINSZ", 49 },
	{ "_SMEMSZ", 50 },
	{ "_SMEMBANKS", 51 },
	{ "_LWINLO", 52 },
	{ "_LWINSZ", 53 },
	{ "_LMEMLOSZ", 54 },
	{ "_LMEMHIOFF", 55 },
	{ "_EQMASK", 56 },
	{ "_LTMASK", 57 },
	{ "_LEMASK", 58 },
	{ "_GTMASK", 59 },
	{ "_GEMASK", 60 },
	{ "_GLOBALERRORSTATUS", 64 },
	{ "_WARPERRORSTATUS", 66 },
	{ "_WARPERRORSTATUSCLEAR", 67 },
	{ "_CLOCKLO", 80 },
	{ "_CLOCKHI", 81 }
};

map<string, int> st_cop_map =
{
	{ ".CG", 1 },
	{ ".CS", 2 },
	{ ".WT", 3 }
};

map<string, int> stx_cop_map =
{
	{ ".CG", 1 },
	{ ".CS", 2 },
	{ ".WT", 3 }
};

map<string, int> type_map =
{
	{ ".U8", 0 },
	{ ".S8", 1 },
	{ ".U16", 2 },
	{ ".S16", 3 },
	{ ".64", 5 },
	{ ".128", 6 }
};

map<string, int> vote_op_map =
{
	{ ".ALL", 0 },
	{ ".ANY", 1 },
	{ ".EQ", 2 },
	{ ".VTG.R", 5 },
	{ ".VTG.A", 6 },
	{ ".VTG.RA", 7 }
};


const string StringMapValue(map<string, int> &suffix_map, int value, bool &found)
{
	/* Find value */
	found = true;
	for (map<string, int>::iterator i = suffix_map.begin(); i != suffix_map.end(); ++i)
		if (i->second == value)
			return i->first;

	/* Not found */
	found = false;
	return "";
}


void Inst::DumpPC(ostream &os)
{
	os << "/*" << setfill('0') << setw(4) << right << hex << addr << "*/";
}


void Inst::DumpToBufWithFmtReg(void)
{
	FmtReg fmt;
	const char *fmt_str;
	int len;

	/* Get instruction format/encoding */
	fmt = bytes.fmt_reg;

	/* Get format string */
	fmt_str = info->fmt_str.c_str();

	/* Process format string */
	while (*fmt_str)
	{
		stringstream ss;

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

		if (Common::Asm::IsToken(fmt_str, "ADD_SUB", len))
		{
			unsigned int sign_src2;
			sign_src2 = ((fmt.mixed0 & 0x10) != 0);
			ss << (sign_src2 ? "SUB" : "ADD");
		}
		else if (Common::Asm::IsToken(fmt_str, "P", len))
		{
			unsigned int p;
			p = (fmt.dst & 0x38) >> 3;
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else if (Common::Asm::IsToken(fmt_str, "Q", len))
		{
			unsigned int q;
			q = fmt.dst & 0x7;
			ss << "P";
			if (q != 7)
				ss << q;
			else
				ss << "T";
		}
		else if (Common::Asm::IsToken(fmt_str, "R", len))
		{
			unsigned int r;
			bool sign;
			r = fmt.mixed1 & 0x7;
			sign = (fmt.mixed1 & 0x8) == 0;
			ss << (sign ? "" : "!");
			ss << "P";
			if (r != 7)
				ss << r;
			else
				ss << "T";
		}
		else if (Common::Asm::IsToken(fmt_str, "brev", len))
		{
			bool brev;
			brev = fmt.mixed0 & 0x10;
			ss << (brev ? ".BREV" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "cc", len))
		{
			bool cc;
			cc = (fmt.dst_mod != 0);
			ss << (cc ? ".CC" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "cmp", len))
		{
			unsigned int cmp;
			string cmp_str;
			bool found;
			cmp = (fmt.mixed1 & 0x3c0) >> 6;
			cmp_str = StringMapValue(cmp_map, cmp, found);
			ss << (found ? cmp_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "dst", len))
		{
			unsigned int dst;
			dst = fmt.dst;
			ss << "R";
			if (dst != 63)
				ss << dst;
			else
				ss << "Z";
		}
		else if (Common::Asm::IsToken(fmt_str, "ftz_", len))
		{
			bool ftz;
			ftz = ((fmt.mixed0 & 0x2) != 0);
			ss << (ftz ? ".FTZ" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "ftzfmz", len))
		{
			bool ftz;
			bool fmz;
			ftz = ((fmt.mixed0 & 0x4) != 0);
			fmz = ((fmt.mixed0 & 0x8) != 0);
			ss << (ftz ? ".FTZ" : "");
			ss << (fmz ? ".FMZ" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "hi", len))
		{
			bool hi;
			hi = ((fmt.mixed0 & 0x4) != 0);
			ss << (hi ? ".HI" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "logic", len))
		{
			unsigned int logic;
			string logic_str;
			bool found;
			logic = (fmt.mixed1 >> 4) & 0x3;
			logic_str = StringMapValue(logic_map, logic, found);
			ss << (found ? logic_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "lop_op", len))
		{
			unsigned int op;
			string op_str;
			bool found;
			op = (fmt.mixed0 >> 2) & 0x3;
			op_str = StringMapValue(lop_op_map, op, found);
			ss << (found ? op_str : ".INVALID");
		}
		else if (Common::Asm::IsToken(fmt_str, "msrc", len))
		{
			unsigned int src2_mod;
			union {unsigned int i; float f;} src2;
			unsigned int src3;
			unsigned int bank_id;
			unsigned int offset_in_bank;
			bool neg_src3;
			unsigned int cat;

			src2_mod = fmt.src2_mod;
			src3 = fmt.mixed1 & 0x3f;
			neg_src3 = ((fmt.mixed0 & 0x10) != 0);

			if (src2_mod == 0)
			{
				src2.i = fmt.src2;
				ss << "R";
				if (src2.i != 63)
					ss << src2.i;
				else
					ss << "Z";
				ss << ", ";
				ss << (neg_src3 ? "-" : "");
				ss << "R";
				if (src3 != 63)
					ss << src3;
				else
					ss << "Z";
			}
			else if (src2_mod == 1)
			{
				src2.i = fmt.src2;
				bank_id = ((src2.i & 0x1) << 4) | (src2.i >> 16);
				offset_in_bank = src2.i & 0xfffc;
				ss << hex << "c[0x" << bank_id << "][0x" << offset_in_bank << "]";
				ss << ", ";
				ss << (neg_src3 ? "-" : "");
				ss << "R";
				if (src3 != 63)
					ss << src3;
				else
					ss << "Z";
			}
			else if (src2_mod == 2)
			{
				ss << (neg_src3 ? "-" : "");
				ss << "R";
				if (src3 != 63)
					ss << src3;
				else
					ss << "Z";
				ss << ", ";
				src2.i = fmt.src2;
				bank_id = ((src2.i & 0x1) << 4) | (src2.i >> 16);
				offset_in_bank = src2.i & 0xfffc;
				ss << hex << "c[0x" << bank_id << "][0x" << offset_in_bank << "]";
			}
			else if (src2_mod == 3)
			{
				cat = bytes.bytes[0] & 0xf;
				if (cat == 0 || cat == 1)
				{
					src2.i = fmt.src2 << 12;
					if (src2.f > 1e9)
						ss << scientific << setprecision(20);
					ss << src2.f;
				}
				else if (cat == 3)
				{
					src2.i = fmt.src2;
					ss << hex;
					if (src2.i >> 19 == 0)  /* positive value */
						ss << "0x" << src2.i;
					else  /* negative value */
						ss << "-0x" << (0x100000 - src2.i);
				}
				ss << ", ";
				ss << (neg_src3 ? "-" : "");
				ss << "R";
				if (src3 != 63)
					ss << src3;
				else
					ss << "Z";
			}
		}
		else if (Common::Asm::IsToken(fmt_str, "mufu_op", len))
		{
			unsigned int op;
			string op_str;
			bool found;
			op = fmt.src2 & 0xf;
			op_str = StringMapValue(mufu_op_map, op, found);
			ss << (found ? op_str : ".INVALID");
		}
		else if (Common::Asm::IsToken(fmt_str, "nasrc1", len))
		{
			unsigned int src1;
			bool neg;
			bool abs;
			src1 = fmt.src1;
			neg = ((fmt.mixed0 & 0x20) != 0);
			abs = ((fmt.mixed0 & 0x8) != 0);
			ss << (neg ? "-" : "");
			ss << (abs ? "|" : "");
			ss << "R";
			if (src1 != 63)
				ss << src1;
			else
				ss << "Z";
			ss << (abs ? "|" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "nasrc2", len))
		{
			unsigned int src2_mod;
			union {unsigned int i; float f;} src2;
			unsigned int bank_id;
			unsigned int offset_in_bank;
			bool neg;
			bool abs;
			unsigned int cat;

			src2_mod = fmt.src2_mod;
			neg = ((fmt.mixed0 & 0x10) != 0);
			abs = ((fmt.mixed0 & 0x4) != 0);

			if (src2_mod == 0)
			{
				src2.i = fmt.src2;
				ss << (neg ? "-" : "");
				ss << (abs ? "|" : "");
				ss << "R";
				if (src2.i != 63)
					ss << src2.i;
				else
					ss << "Z";
				ss << (abs ? "|" : "");
			}
			else if (src2_mod == 1 || src2_mod == 2)
			{
				src2.i = fmt.src2;
				bank_id = ((src2.i & 0x1) << 4) | (src2.i >> 16);
				offset_in_bank = src2.i & 0xfffc;
				ss << hex << "c[0x" << bank_id << "][0x" << offset_in_bank << "]";
			}
			else if (src2_mod == 3)
			{
				cat = bytes.bytes[0] & 0xf;
				if (cat == 0 || cat == 1)
				{
					src2.i = fmt.src2 << 12;
					if (isinf(src2.f))
						ss << showpos << uppercase;
					ss << src2.f;
				}
				else if (cat == 3)
				{
					src2.i = fmt.src2;
					ss << showpos << uppercase << hex;
					if (src2.i >> 19 == 0)  /* positive value */
						ss << "0x" << src2.i;
					else  /* negative value */
						ss << "-0x" << (0x100000 - src2.i);
				}
			}
		}
		else if (Common::Asm::IsToken(fmt_str, "nsrc1_05", len))
		{
			unsigned int src1;
			bool sign;
			src1 = fmt.src1;
			sign = ((fmt.mixed0 & 0x20) != 0);
			ss << (sign ? "-" : "");
			ss << "R";
			if (src1 != 63)
				ss << src1;
			else
				ss << "Z";
		}
		else if (Common::Asm::IsToken(fmt_str, "nsrc1_176", len))
		{
			unsigned int src1;
			bool neg;
			src1 = fmt.src1;
			neg = ((fmt.mixed1 & 0x80) != 0) && ((fmt.mixed1 & 0x40) == 0);
			ss << (neg ? "-" : "");
			ss << "R";
			if (src1 != 63)
				ss << src1;
			else
				ss << "Z";
		}
		else if (Common::Asm::IsToken(fmt_str, "nsrc1_18", len))
		{
			unsigned int src1;
			bool sign;
			src1 = fmt.src1;
			sign = ((fmt.mixed1 & 0x100) != 0);
			ss << (sign ? "-" : "");
			ss << "R";
			if (src1 != 63)
				ss << src1;
			else
				ss << "Z";
		}
		else if (Common::Asm::IsToken(fmt_str, "nsrc2", len))
		{
			unsigned int src2_mod;
			union {unsigned int i; float f;} src2;
			unsigned int bank_id;
			unsigned int offset_in_bank;
			bool neg;
			unsigned int cat;

			src2_mod = fmt.src2_mod;
			neg = ((fmt.mixed0 & 0x20) != 0);

			if (src2_mod == 0)
			{
				src2.i = fmt.src2;
				ss << (neg ? "-" : "");
				ss << "R";
				if (src2.i != 63)
					ss << src2.i;
				else
					ss << "Z";
			}
			else if (src2_mod == 1 || src2_mod == 2)
			{
				src2.i = fmt.src2;
				bank_id = ((src2.i & 0x1) << 4) | (src2.i >> 16);
				offset_in_bank = src2.i & 0xfffc;
				ss << hex << "c[0x" << bank_id << "][0x" << offset_in_bank << "]";
			}
			else if (src2_mod == 3)
			{
				cat = bytes.bytes[0] & 0xf;
				if (cat == 0 || cat == 1)
				{
					src2.i = fmt.src2 << 12;
					ss.precision(6);
					ss << src2.f;
				}
				else if (cat == 3)
				{
					src2.i = fmt.src2;
					ss << hex;
					if (src2.i >> 19 == 0)  /* positive value */
						ss << "0x" << src2.i;
					else  /* negative value */
						ss << "-0x" << (0x100000 - src2.i);
				}
			}
		}
		else if (Common::Asm::IsToken(fmt_str, "neg_176", len))
		{
			bool neg;
			neg = ((fmt.mixed1 & 0x40) != 0) && ((fmt.mixed1 & 0x80) == 0);
			ss << (neg ? "-" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "nsrc3", len))
		{
			unsigned int src3;
			bool neg;
			src3 = fmt.mixed1 & 0x3f;
			neg = ((fmt.mixed0 & 0x10) != 0);
			ss << (neg ? "-" : "");
			ss << "R";
			if (src3 != 63)
				ss << src3;
			else
				ss << "Z";
		}
		else if (Common::Asm::IsToken(fmt_str, "po", len))
		{
			bool po;
			po = ((fmt.mixed1 & 0xc0) == 0xc0);
			ss << (po ? ".PO" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "pred", len))
		{
			unsigned int pred;
			pred = fmt.pred;
			if (pred < 7)
				ss << "    @P" << pred;
			else if (pred > 7)
				ss << "   @!P" << pred - 8;
			else
				ss << "       ";
		}
		else if (Common::Asm::IsToken(fmt_str, "rnd", len))
		{
			unsigned int rnd;
			string rnd_str;
			bool found;
			rnd = (fmt.mixed1 >> 6) & 0x3;
			rnd_str = StringMapValue(rnd_i_map, rnd, found);
			ss << (found ? rnd_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "s_", len))
		{
			bool s;
			s = fmt.mixed0 & 0x1;
			ss << (s ? ".S" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "s32", len))
		{
			bool s32;
			s32 = ((fmt.mixed0 & 0x2) != 0);
			ss << (s32 ? ".S32" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "sat_01", len))
		{
			bool sat;
			sat = ((fmt.mixed0 & 0x2) != 0);
			ss << (sat ? ".SAT" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "sat_10", len))
		{
			bool sat;
			sat = ((fmt.mixed1 & 0x1) != 0);
			ss << (sat ? ".SAT" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "sat_17", len))
		{
			bool sat;
			sat = ((fmt.mixed1 & 0x80) != 0);
			ss << (sat ? ".SAT" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "shamt", len))
		{
			unsigned int shamt;
			shamt = (fmt.mixed0 >> 1) & 0x1f;
			ss << hex << "0x" << shamt;
		}
		else if (Common::Asm::IsToken(fmt_str, "src1", len))
		{
			unsigned int src1;
			src1 = fmt.src1;
			ss << "R";
			if (src1 != 63)
				ss << src1;
			else
				ss << "Z";
		}
		else if (Common::Asm::IsToken(fmt_str, "src2", len))
		{
			unsigned int src2_mod;
			union {unsigned int i; float f;} src2;
			unsigned int bank_id;
			unsigned int offset_in_bank;
			unsigned int cat;

			src2_mod = fmt.src2_mod;

			if (src2_mod == 0)
			{
				src2.i = fmt.src2;
				ss << "R";
				if (src2.i != 63)
					ss << src2.i;
				else
					ss << "Z";
			}
			else if (src2_mod == 1 || src2_mod == 2)
			{
				src2.i = fmt.src2;
				bank_id = ((src2.i & 0x1) << 4) | (src2.i >> 16);
				offset_in_bank = src2.i & 0xfffc;
				ss << hex << "c[0x" << bank_id << "][0x" << offset_in_bank << "]";
			}
			else if (src2_mod == 3)
			{
				cat = bytes.bytes[0] & 0xf;
				if (cat == 0 || cat == 1)
				{
					src2.i = fmt.src2 << 12;
					ss << src2.f;
				}
				else if (cat == 3)
				{
					src2.i = fmt.src2;
					ss << hex;
					if (src2.i >> 19 == 0)  /* positive value */
						ss << "0x" << src2.i;
					else  /* negative value */
						ss << "-0x" << (0x100000 - src2.i);
				}
			}
		}
		else if (Common::Asm::IsToken(fmt_str, "src3", len))
		{
			unsigned int src3;
			src3 = fmt.mixed1 & 0x3f;
			ss << "R";
			if (src3 != 63)
				ss << src3;
			else
				ss << "Z";
		}
		else if (Common::Asm::IsToken(fmt_str, "u32", len))
		{
			bool u32;
			u32 = ((fmt.mixed0 & 0x2) == 0);
			ss << (u32 ? ".U32" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "w", len))
		{
			bool w;
			w = ((fmt.mixed0 & 0x20) != 0);
			ss << (w ? ".W" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "type1", len))
		{
			bool type1;
			type1 = ((fmt.mixed0 & 0x8) == 0);
			ss << (type1 ? ".U32" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "type2", len))
		{
			bool type2;
			type2 = ((fmt.mixed0 & 0x2) == 0);
			ss << (type2 ? ".U32" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "x", len))
		{
			bool x;
			x = ((fmt.mixed0 & 0x4) != 0);
			ss << (x ? ".X" : "");
		}
		else
		{
			fatal("%s: unknown token: %s", __FUNCTION__,
					fmt_str);
		}

		str += ss.str();

		/* Skip processed token */
		fmt_str += len;
	}
	str += ";";
}


void Inst::DumpToBufWithFmtImm(void)
{
	FmtImm fmt;
	const char *fmt_str;
	int len;

	/* Get instruction format/encoding */
	fmt = bytes.fmt_imm;

	/* Get format string */
	fmt_str = info->fmt_str.c_str();

	/* Process format string */
	while (*fmt_str)
	{
		stringstream ss;

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

		if (Common::Asm::IsToken(fmt_str, "dst", len))
		{
			unsigned int dst;
			dst = fmt.dst;
			ss << "R";
			if (dst != 63)
				ss << dst;
			else
				ss << "Z";
		}
		else if (Common::Asm::IsToken(fmt_str, "imm32", len))
		{
			unsigned int imm32;
			imm32 = fmt.imm32;
			ss << hex;
			if (imm32 <= 0x80000000)  /* positive value */
				ss << "0x" << imm32;
			else  /* negative value */
				ss << "-0x" << ((unsigned long long int)0x100000000 - imm32);
		}
		else if (Common::Asm::IsToken(fmt_str, "lop_op", len))
		{
			unsigned int op;
			string op_str;
			bool found;
			op = (fmt.suffix0 >> 2) & 0x3;
			op_str = StringMapValue(lop_op_map, op, found);
			ss << (found ? op_str : ".INVALID");
		}
		else if (Common::Asm::IsToken(fmt_str, "nsrc1", len))
		{
			unsigned int src1;
			bool neg;
			src1 = fmt.src1;
			neg = ((fmt.suffix0 & 0x20) != 0);
			ss << (neg ? "-" : "");
			ss << "R";
			if (src1 != 63)
				ss << src1;
			else
				ss << "Z";
		}
		else if (Common::Asm::IsToken(fmt_str, "pred", len))
		{
			unsigned int pred;
			pred = fmt.pred;
			if (pred < 7)
				ss << "    @P" << pred;
			else if (pred > 7)
				ss << "   @!P" << pred - 8;
			else
				ss << "       ";
		}
		else if (Common::Asm::IsToken(fmt_str, "sat", len))
		{
			bool sat;
			sat = ((fmt.suffix0 & 0x2) != 0);
			ss << (sat ? ".SAT" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "src1", len))
		{
			unsigned int src1;
			src1 = fmt.src1;
			ss << "R";
			if (src1 != 63)
				ss << src1;
			else
				ss << "Z";
		}
		else if (Common::Asm::IsToken(fmt_str, "uimm32", len))
		{
			unsigned int imm32;
			imm32 = fmt.imm32;
			ss << hex << "0x" << imm32;
		}
		else if (Common::Asm::IsToken(fmt_str, "x", len))
		{
			bool x;
			x = ((fmt.suffix0 & 0x4) != 0);
			ss << (x ? ".X" : "");
		}
		else
		{
			fatal("%s: unknown token: %s", __FUNCTION__,
					fmt_str);
		}

		str += ss.str();

		/* Skip processed token */
		fmt_str += len;
	}
	str += ";";
}


void Inst::DumpToBufWithFmtOther(void)
{
	FmtOther fmt;
	const char *fmt_str;
	int len;

	/* Get instruction format/encoding */
	fmt = bytes.fmt_other;

	/* Get format string */
	fmt_str = info->fmt_str.c_str();

	/* Process format string */
	while (*fmt_str)
	{
		stringstream ss;

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

		if (Common::Asm::IsToken(fmt_str, "P__", len))
		{
			unsigned int p;
			p = (fmt.dst & 0x38) >> 3;
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else if (Common::Asm::IsToken(fmt_str, "P_75", len))
		{
			unsigned int p;
			p = (fmt.mixed1 >> 5) & 0x7;
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else if (Common::Asm::IsToken(fmt_str, "P_64", len))
		{
			unsigned int p;
			p = (fmt.mixed1 >> 4) & 0x7;
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else if (Common::Asm::IsToken(fmt_str, "P1", len))
		{
			unsigned int p1;
			p1 = fmt.src1 & 0x7;
			ss << "P";
			if (p1 != 7)
				ss << p1;
			else
				ss << "T";
		}
		else if (Common::Asm::IsToken(fmt_str, "P2", len))
		{
			unsigned int p2;
			p2 = fmt.src2 & 0x7;
			ss << "P";
			if (p2 != 7)
				ss << p2;
			else
				ss << "T";
		}
		else if (Common::Asm::IsToken(fmt_str, "Q__", len))
		{
			unsigned int q;
			q = fmt.dst & 0x7;
			ss << "P";
			if (q != 7)
				ss << q;
			else
				ss << "T";
		}
		else if (Common::Asm::IsToken(fmt_str, "Q_m", len))
		{
			unsigned int q;
			q = fmt.mixed1 & 0x7;
			ss << "P";
			if (q != 7)
				ss << q;
			else
				ss << "T";
		}
		else if (Common::Asm::IsToken(fmt_str, "Q_s", len))
		{
			unsigned int q;
			q = fmt.src1 & 0x7;
			ss << "P";
			if (q != 7)
				ss << q;
			else
				ss << "T";
		}
		else if (Common::Asm::IsToken(fmt_str, "R", len))
		{
			unsigned int r;
			bool sign;
			r = fmt.mixed1 & 0x7;
			sign = (fmt.mixed1 & 0x8) == 0;
			ss << (sign ? "" : "!");
			ss << "P";
			if (r != 7)
				ss << r;
			else
				ss << "T";
		}
		else if (Common::Asm::IsToken(fmt_str, "b_", len))
		{
			unsigned int b;
			b = (fmt.mixed1 >> 6) & 0x3;
			if (b != 0)
				ss << ".B" << b;
		}
		else if (Common::Asm::IsToken(fmt_str, "b2r_op", len))
		{
			unsigned int op;
			string op_str;
			bool found;
			op = (fmt.mixed0 >> 4) & 0x3;
			op_str = StringMapValue(b2r_op_map, op, found);
			ss << (found ? op_str : ".INVALID");
		}
		else if (Common::Asm::IsToken(fmt_str, "bar_mode", len))
		{
			bool mode;
			mode = ((fmt.mixed0 & 0x8) == 0);
			ss << (mode ? ".RED" : ".ARV");
		}
		else if (Common::Asm::IsToken(fmt_str, "bar_op", len))
		{
			unsigned int op;
			string op_str;
			bool found;
			op = (fmt.mixed0 >> 1) & 0x3;
			op_str = StringMapValue(bar_op_map, op, found);
			ss << (found ? op_str : ".INVALID");
		}
		else if (Common::Asm::IsToken(fmt_str, "cc__", len))
		{
			bool cc;
			cc = (fmt.dst_mod != 0);
			ss << (cc ? ".CC" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "cc_op", len))
		{
			unsigned int op;
			string op_str;
			bool found;
			op = (fmt.mixed0 >> 1) & 0x1f;
			op_str = StringMapValue(cc_op_map, op, found);
			ss << (found ? op_str : ".INVALID");
		}
		else if (Common::Asm::IsToken(fmt_str, "cmp", len))
		{
			unsigned int cmp;
			string cmp_str;
			bool found;
			cmp = (fmt.src2 >> 4) & 0x3;
			cmp_str = StringMapValue(logic_map, cmp, found);
			ss << (found ? cmp_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "dst", len))
		{
			unsigned int dst;
			dst = fmt.dst;
			ss << "R";
			if (dst != 63)
				ss << dst;
			else
				ss << "Z";
		}
		else if (Common::Asm::IsToken(fmt_str, "dtype_f", len))
		{
			unsigned int dtype;
			string dtype_str;
			bool found;
			dtype = fmt.src1 & 0x3;
			dtype_str = StringMapValue(ftype_map, dtype, found);
			ss << (found ? dtype_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "dtype_i", len))
		{
			bool sign;
			unsigned int dtype;
			string dtype_str;
			bool found;
			sign = ((fmt.mixed0 & 0x8) == 0);
			ss << (sign ? ".U" : ".S");
			dtype = fmt.src1 & 0x3;
			dtype_str = StringMapValue(itype_map, dtype, found);
			ss << (found ? dtype_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "ftz", len))
		{
			bool ftz;
			ftz = ((fmt.mixed1 & 0x40) != 0);
			ss << (ftz ? ".FTZ" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "logic", len))
		{
			unsigned int logic;
			string logic_str;
			bool found;
			logic = (fmt.mixed1 >> 4) & 0x3;
			logic_str = StringMapValue(logic_map, logic, found);
			ss << (found ? logic_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "nasrc2", len))
		{
			unsigned int src2_mod;
			union {unsigned int i; float f;} src2;
			unsigned int bank_id;
			unsigned int offset_in_bank;
			bool neg;
			bool abs;
			unsigned int cat;

			src2_mod = fmt.src2_mod;
			neg = ((fmt.mixed0 & 0x10) != 0);
			abs = ((fmt.mixed0 & 0x4) != 0);

			if (src2_mod == 0)
			{
				src2.i = fmt.src2;
				ss << (neg ? "-" : "");
				ss << (abs ? "|" : "");
				ss << "R";
				if (src2.i != 63)
					ss << src2.i;
				else
					ss << "Z";
				ss << (abs ? "|" : "");
			}
			else if (src2_mod == 1 || src2_mod == 2)
			{
				src2.i = fmt.src2;
				bank_id = (src2.i & 0xf0000) >> 16;
				offset_in_bank = src2.i & 0xffff;
				ss << hex << "c[0x" << bank_id << "][0x" << offset_in_bank << "]";
			}
			else if (src2_mod == 3)
			{
				cat = bytes.bytes[0] & 0xf;
				if (cat == 0 || cat == 1)
				{
					src2.i = fmt.src2 << 12;
					ss << src2.f;
				}
				else if (cat == 3)
				{
					src2.i = fmt.src2;
					ss << hex;
					if (src2.i >> 19 == 0)  /* positive value */
						ss << "0x" << src2.i;
					else  /* negative value */
						ss << "-0x" << (0x100000 - src2.i);
				}
			}
		}
		else if (Common::Asm::IsToken(fmt_str, "nop_op", len))
		{
			unsigned int op;
			string op_str;
			bool found;
			op = (fmt.mixed1 >> 2) & 0x7;
			op_str = StringMapValue(nop_op_map, op, found);
			ss << (found ? op_str : ".INVALID");
		}
		else if (Common::Asm::IsToken(fmt_str, "pred", len))
		{
			unsigned int pred;
			pred = fmt.pred;
			if (pred < 7)
				ss << "    @P" << pred;
			else if (pred > 7)
				ss << "   @!P" << pred - 8;
			else
				ss << "       ";
		}
		else if (Common::Asm::IsToken(fmt_str, "rnd_f", len))
		{
			unsigned int rnd;
			string rnd_str;
			bool found;
			rnd = fmt.mixed1 & 0x3;
			rnd_str = StringMapValue(rnd_f_map, rnd, found);
			ss << (found ? rnd_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "rnd_i", len))
		{
			unsigned int rnd;
			string rnd_str;
			bool found;
			rnd = fmt.mixed1 & 0x3;
			rnd_str = StringMapValue(rnd_i_map, rnd, found);
			ss << (found ? rnd_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "rop", len))
		{
			bool rop;
			rop = ((fmt.mixed0 & 0x8) != 0);
			ss << (rop ? ".ROUND" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "s_", len))
		{
			bool s;
			s = ((fmt.mixed0 & 0x1) != 0);
			ss << (s ? ".S" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "sat", len))
		{
			bool sat;
			sat = ((fmt.mixed0 & 0x2) != 0);
			ss << (sat ? ".SAT" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "src2", len))
		{
			unsigned int src2_mod;
			unsigned int src2;
			unsigned int bank_id;
			unsigned int offset_in_bank;

			src2_mod = fmt.src2_mod;
			src2 = fmt.src2;

			if (src2_mod == 0)
			{
				ss << "R";
				if (src2 != 63)
					ss << src2;
				else
					ss << "Z";
			}
			else if (src2_mod == 1 || src2_mod == 2)
			{
				bank_id = (src2 & 0xf0000) >> 16;
				offset_in_bank = src2 &	0xffff;
				ss << hex << "c[0x" << bank_id << "][0x" << offset_in_bank << "]";
			}
			else if (src2_mod == 3)
			{
				ss << hex;
				if (src2 >> 19 == 0)  /* positive value */
					ss << "0x" << src2;
				else  /* negative value */
					ss << "-0x" << (0x100000 - src2);
			}
		}
		else if (Common::Asm::IsToken(fmt_str, "sreg", len))
		{
			unsigned int sreg_idx;
			string sreg_str;
			bool found;
			sreg_idx = fmt.src2 & 0xff;
			sreg_str = StringMapValue(sreg_map, sreg_idx, found);
			ss << "SR";
			if (found)
				ss << sreg_str;
			else
				ss << sreg_idx;
		}
		else if (Common::Asm::IsToken(fmt_str, "stype_f", len))
		{
			unsigned int stype;
			string stype_str;
			bool found;
			stype = fmt.src1 & 0x3;
			stype_str = StringMapValue(ftype_map, stype, found);
			ss << (found ? stype_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "stype_i", len))
		{
			unsigned int sign;
			unsigned int stype;
			string stype_str;
			bool found;
			sign = fmt.mixed0 & 0x20;
			ss << (sign == 0 ? ".U" : ".S");
			stype = fmt.src1 & 0x3;
			stype_str = StringMapValue(itype_map, stype, found);
			ss << (found ? stype_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "trig", len))
		{
			bool trig;
			trig = ((fmt.mixed1 & 0x2) != 0);
			ss << (trig ? ".TRIG" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "vote_op", len))
		{
			unsigned int op;
			string op_str;
			bool found;
			op = (fmt.mixed0 >> 1) & 0x7;
			op_str = StringMapValue(vote_op_map, op, found);
			ss << (found ? op_str : ".INVALID");
		}
		else
		{
			fatal("%s: unknown token: %s", __FUNCTION__,
					fmt_str);
		}

		str += ss.str();

		/* Skip processed token */
		fmt_str += len;
	}
	str += ";";
}


void Inst::DumpToBufWithFmtLdSt(void)
{
	FmtLdSt fmt;
	const char *fmt_str;
	int len;

	/* Get instruction format/encoding */
	fmt = bytes.fmt_ldst;

	/* Get format string */
	fmt_str = info->fmt_str.c_str();

	/* Process format string */
	while (*fmt_str)
	{
		stringstream ss;

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

		if (Common::Asm::IsToken(fmt_str, "Q", len))
		{
			unsigned int q;
			q = ((fmt.func & 0x1) << 2) | ((fmt.suffix0 >> 4) & 0x3);
			ss << "P";
			if (q != 7)
				ss << q;
			else
				ss << "T";
		}
		else if (Common::Asm::IsToken(fmt_str, "X", len))
		{
			bool l;
			l = ((fmt.suffix1 & 0x4000) == 0);
			ss << (l ? "L" : "S");
		}
		else if (Common::Asm::IsToken(fmt_str, "atom_op", len))
		{
			unsigned int op;
			string op_str;
			bool found;
			op = (fmt.suffix0 >> 1) & 0xf;
			op_str = StringMapValue(atom_op_map, op, found);
			ss << (found ? op_str : ".INVALID");
		}
		else if (Common::Asm::IsToken(fmt_str, "atom_type", len))
		{
			unsigned int type;
			string type_str;
			bool found;
			type = (fmt.func & 0xe) | ((fmt.suffix0 >> 5) & 0x1);
			type_str = StringMapValue(atom_type_map, type, found);
			ss << (found ? type_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "bank", len))
		{
			unsigned int bank;
			bank = fmt.suffix1 & 0xf;
			if (bank)
				ss << hex << "0x" << bank;
		}
		else if (Common::Asm::IsToken(fmt_str, "btoff03", len))
		{
			unsigned int btoff03;
			string btoff03_str;
			bool found;
			btoff03 = (fmt.suffix1 >> 12) & 0x3;
			btoff03_str = StringMapValue(btoff03_map, btoff03, found);
			ss << (found ? btoff03_str : ".INVALIDBTOFF03");
		}
		else if (Common::Asm::IsToken(fmt_str, "cc_cop", len))
		{
			unsigned int op;
			string op_str;
			bool found;
			op = (fmt.suffix0 >> 1) & 0x7;
			op_str = StringMapValue(cc_cop_map, op, found);
			ss << (found ? op_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "cctl_op", len))
		{
			unsigned int op;
			string op_str;
			bool found;
			op = fmt.off16 & 0x3;
			op_str = StringMapValue(cctl_op_map, op, found);
			ss << (found ? op_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "dc", len))
		{
			unsigned int dc;
			dc = ((fmt.suffix1 & 0x8000) != 0);
			ss << ((dc != 0) ? ".DC" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "dst", len))
		{
			unsigned int dst;
			dst = fmt.dst;
			ss << "R";
			if (dst != 63)
				ss << dst;
			else
				ss << "Z";
		}
		else if (Common::Asm::IsToken(fmt_str, "e", len))
		{
			unsigned int e;
			e = fmt.func & 0x1;
			ss << ((e != 0) ? ".E" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "geom", len))
		{
			unsigned int geom;
			string geom_str;
			bool found;
			geom = (fmt.suffix1 >> 9) & 0x7;
			geom_str = StringMapValue(geom_map, geom, found);
			ss << (found ? geom_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "i_", len))
		{
			unsigned int i;
			i = ((fmt.suffix1 & 0x100) != 0);
			ss << ((i != 0) ? ".I" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "imm4", len))
		{
			unsigned int imm4;
			imm4 = (fmt.suffix1 >> 4) & 0xf;
			ss << hex << "0x" << imm4;
		}
		else if (Common::Asm::IsToken(fmt_str, "imm5", len))
		{
			unsigned int imm5;
			imm5 = ((fmt.suffix1 & 0x7) << 2) | ((fmt.off16 >> 14) & 0x3);
			ss << hex << "0x" << imm5;
		}
		else if (Common::Asm::IsToken(fmt_str, "imm8", len))
		{
			unsigned int imm8;
			imm8 = (fmt.off16 >> 6) & 0xff;
			ss << hex << "0x" << imm8;
		}
		else if (Common::Asm::IsToken(fmt_str, "ld_cop", len))
		{
			unsigned int cop;
			string cop_str;
			bool found;
			cop = (fmt.suffix0 >> 4) & 0x3;
			cop_str = StringMapValue(ld_cop_map, cop, found);
			ss << (found ? cop_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "ldx_cop", len))
		{
			unsigned int cop;
			string cop_str;
			bool found;
			cop = (fmt.suffix0 >> 4) & 0x3;
			cop_str = StringMapValue(ldx_cop_map, cop, found);
			ss << (found ? cop_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "lvl", len))
		{
			unsigned int lvl;
			string lvl_str;
			bool found;
			lvl = (fmt.suffix0 >> 1) & 0x3;
			lvl_str = StringMapValue(lvl_map, lvl, found);
			ss << (found ? lvl_str : ".INVALID");
		}
		else if (Common::Asm::IsToken(fmt_str, "lz", len))
		{
			unsigned int lz;
			lz = ((fmt.suffix1 & 0x4000) != 0);
			ss << ((lz != 0) ? ".LZ" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "ndv", len))
		{
			unsigned int ndv;
			ndv = ((fmt.suffix1 & 0x8) != 0);
			ss << ((ndv != 0) ? ".NDV" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "nodep", len))
		{
			unsigned int nodep;
			nodep = ((fmt.suffix0 & 0x20) != 0);
			ss << ((nodep != 0) ? ".NODEP" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "off16", len))
		{
			unsigned int off16;
			off16 = fmt.off16;
			if (off16)
				ss << hex << "+0x" << off16;
		}
		else if (Common::Asm::IsToken(fmt_str, "off20", len))
		{
			unsigned int off20;
			off20 = ((fmt.suffix1 & 0xe000) << 4) | fmt.off16;
			if (off20)
				ss << hex << "+0x" << off20;
		}
		else if (Common::Asm::IsToken(fmt_str, "off24", len))
		{
			unsigned int off24;
			off24 = ((fmt.suffix1 & 0xff) << 16) | fmt.off16;
			if (off24)
				ss << hex << "+0x" << off24;
		}
		else if (Common::Asm::IsToken(fmt_str, "off30", len))
		{
			unsigned int off30;
			off30 = ((fmt.suffix1 & 0xffff) << 14) | (fmt.off16 >> 2);
			if (off30)
				ss << hex << "+0x" << off30;
		}
		else if (Common::Asm::IsToken(fmt_str, "phase3", len))
		{
			unsigned int phase3;
			string phase3_str;
			bool found;
			phase3 = (fmt.suffix0 >> 3) & 0x3;
			phase3_str = StringMapValue(phase3_map, phase3, found);
			ss << (found ? phase3_str : ".INVALIDPHASE3");
		}
		else if (Common::Asm::IsToken(fmt_str, "pred", len))
		{
			unsigned int pred;
			pred = fmt.pred;
			if (pred < 7)
				ss << "    @P" << pred;
			else if (pred > 7)
				ss << "   @!P" << pred - 8;
			else
				ss << "       ";
		}
		else if (Common::Asm::IsToken(fmt_str, "red_op", len))
		{
			unsigned int op;
			string op_str;
			bool found;
			op = (fmt.suffix0 >> 1) & 0xf;
			op_str = StringMapValue(red_op_map, op, found);
			ss << (found ? op_str : ".INVALID");
		}
		else if (Common::Asm::IsToken(fmt_str, "red_type", len))
		{
			unsigned int type;
			string type_str;
			bool found;
			type = (fmt.func & 0xe) | ((fmt.suffix0 & 0x20) >> 5);
			type_str = StringMapValue(red_type_map, type, found);
			ss << (found ? type_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "s_", len))
		{
			bool s;
			s = ((fmt.suffix0 & 0x1) != 0);
			ss << (s ? ".S" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "src1", len))
		{
			unsigned int src1;
			src1 = fmt.src1;
			ss << "R";
			if (src1 != 63)
				ss << src1;
			else
				ss << "Z";
		}
		else if (Common::Asm::IsToken(fmt_str, "src2", len))
		{
			unsigned int src2;
			src2 = fmt.off16 & 0x3f;
			if (src2 != 63)
				ss << "R" << src2;
			else
				ss << "0x0";
		}
		else if (Common::Asm::IsToken(fmt_str, "src3", len))
		{
			unsigned int src3;
			src3 = (fmt.suffix1 >> 1) & 0x3f;
			ss << "R";
			if (src3 != 63)
				ss << src3;
			else
				ss << "Z";
		}
		else if (Common::Asm::IsToken(fmt_str, "src4", len))
		{
			unsigned int src4;
			src4 = (fmt.suffix1 >> 7) & 0x3f;
			if (src4 != 63)
				ss << ", R" << src4;
		}
		else if (Common::Asm::IsToken(fmt_str, "st_cop", len))
		{
			unsigned int cop;
			string cop_str;
			bool found;
			cop = (fmt.suffix0 >> 4) & 0x3;
			cop_str = StringMapValue(st_cop_map, cop, found);
			ss << (found ? cop_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "stx_cop", len))
		{
			unsigned int cop;
			string cop_str;
			bool found;
			cop = (fmt.suffix0 >> 4) & 0x3;
			cop_str = StringMapValue(stx_cop_map, cop, found);
			ss << (found ? cop_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "type", len))
		{
			unsigned int type;
			string type_str;
			bool found;
			type = (fmt.suffix0 >> 1) & 0x7;
			type_str = StringMapValue(type_map, type, found);
			ss << (found ? type_str : "");
		}
		else
		{
			fatal("%s: unknown token: %s", __FUNCTION__,
					fmt_str);
		}

		str += ss.str();

		/* Skip processed token */
		fmt_str += len;
	}
	str += ";";
}


void Inst::DumpToBufWithFmtCtrl(void)
{
	FmtCtrl fmt;
	const char *fmt_str;
	int len;

	/* Get instruction format/encoding */
	fmt = bytes.fmt_ctrl;

	/* Get format string */
	fmt_str = info->fmt_str.c_str();

	/* Process format string */
	while (*fmt_str)
	{
		stringstream ss;

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

		if (Common::Asm::IsToken(fmt_str, "lmt", len))
		{
			unsigned int lmt;
			lmt = fmt.suffix1 & 0x2;
			ss << ((lmt != 0) ? ".LMT" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "noinc", len))
		{
			unsigned int noinc;
			noinc = fmt.suffix1 & 0x20;
			ss << ((noinc != 0) ? ".NOINC" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "offset", len))
		{
			unsigned int target;
			target = fmt.target;
			ss << hex << "0x" << addr + 8 + target;
		}
		else if (Common::Asm::IsToken(fmt_str, "pred", len))
		{
			unsigned int pred;
			pred = fmt.pred;
			if (pred < 7)
				ss << "    @P" << pred;
			else if (pred > 7)
				ss << "   @!P" << pred - 8;
			else
				ss << "       ";
		}
		else if (Common::Asm::IsToken(fmt_str, "s", len))
		{
			unsigned int s;
			s = fmt.suffix0 & 0x1;
			ss << ((s != 0) ? ".S" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "target", len))
		{
			unsigned int target;
			target = fmt.target;
			ss << hex << "0x" << target;
		}
		else if (Common::Asm::IsToken(fmt_str, "u", len))
		{
			unsigned int u;
			u = fmt.suffix1 & 0x1;
			ss << ((u != 0) ? ".U" : "");
		}
		else
		{
			fatal("%s: unknown token: %s", __FUNCTION__,
					fmt_str);
		}

		str += ss.str();

		/* Skip processed token */
		fmt_str += len;
	}
	str += ";";
}


void Inst::DumpToBuf(void)
{
	int cat;

	/* Check if instruction is supported */
	if (!info)
		fatal("%s: instruction not supported (offset=0x%x)",
				__FUNCTION__, addr);

	/* Get instruction category */
	cat = bytes.bytes[0] & 0xf;

	/* Dump to buffer based on format */
	if (cat == 0 || cat == 1 || cat == 3)
		DumpToBufWithFmtReg();
	else if (cat == 2)
		DumpToBufWithFmtImm();
	else if (cat == 4)
		DumpToBufWithFmtOther();
	else if (cat == 5 || cat == 6)
		DumpToBufWithFmtLdSt();
	else if (cat == 7)
		DumpToBufWithFmtCtrl();
	else
		fatal("%s: instruction category %d (offset=0x%x)",
				__FUNCTION__, addr, cat);
}


void Inst::Dump(ostream &os, unsigned int max_inst_len)
{
	os << setfill(' ') << setw(max_inst_len + 1) << left;
	os << str;
}


void Inst::DumpHex(ostream &os)
{
	os << "/* 0x";
	os << setfill('0') << setw(16) << right << hex << bytes.dword;
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
