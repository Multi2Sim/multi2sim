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

map<string, int> cmp_map =
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

map<string, int> logic_map =
{
	{ ".AND", 0},
	{ ".OR", 1},
	{ ".XOR", 2},
};

map<string, int> rnd_map =
{
	{ ".RM", 1},
	{ ".RP", 2},
	{ ".RZ", 3},
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

map<string, int> ld_cop_map =
{
	{ ".CG", 1},
	{ ".CS", 2},
	{ ".CV", 3}
};

map<string, int> st_cop_map =
{
	{ ".CG", 1},
	{ ".CS", 2},
	{ ".WT", 3}
};

map<string, int> type_map =
{
	{ ".U8", 0},
	{ ".S8", 1},
	{ ".U16", 2},
	{ ".S16", 3},
	{ ".64", 5},
	{ ".128", 6}
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


map<string, int> red_op_map =
{
	{ ".ADD", 0},
	{ ".MIN", 1},
	{ ".MAX", 2},
	{ ".INC", 3},
	{ ".DEC", 4},
	{ ".AND", 5},
	{ ".OR", 6},
	{ ".XOR", 7}
};

map<string, int> red_type_map =
{
	{ ".U64", 5},
	{ ".S32", 7},
	{ ".F32.FTZ.RN", 11}
};

map<string, int> atom_op_map =
{
	{ ".ADD", 0},
	{ ".MIN", 1},
	{ ".MAX", 2},
	{ ".INC", 3},
	{ ".DEC", 4},
	{ ".AND", 5},
	{ ".OR", 6},
	{ ".XOR", 7},
	{ ".EXCH", 8},
	{ ".CAS", 9},
};

map<string, int> atom_type_map =
{
	{ ".U64", 5},
	{ ".S32", 7},
	{ ".F32.FTZ.RN", 11}
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

		if (Common::Asm::IsToken(fmt_str, "pred", len))
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
		else if (Common::Asm::IsToken(fmt_str, "cc", len))
		{
			unsigned int cc;
			cc = fmt.dst_mod;
			ss << ((cc == 1) ? ".CC" : "");
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
		else if (Common::Asm::IsToken(fmt_str, "esrc1", len))
		{
			unsigned int src1;
			unsigned int neg;
			unsigned int abs;
			src1 = fmt.src1;
			neg = fmt.mixed0 & 0x20;
			abs = fmt.mixed0 & 0x8;
			ss << ((neg != 0) ? "-" : "");
			ss << ((abs != 0) ? "|" : "");
			ss << "R";
			if (src1 != 63)
				ss << src1;
			else
				ss << "Z";
			ss << ((abs != 0) ? "|" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "ftz", len))
		{
			unsigned int ftz;
			ftz = fmt.mixed0 & 0x2;
			ss << ((ftz != 0) ? ".FTZ" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "hi", len))
		{
			unsigned int hi;
			hi = fmt.mixed0 & 0x4;
			ss << ((hi != 0) ? ".HI" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "imad_sat", len))
		{
			unsigned int sat;
			sat = fmt.mixed1 & 0x40;
			ss << ((sat != 0) ? ".SAT" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "logic", len))
		{
			unsigned int logic;
			string logic_str;
			bool found;
			logic = (fmt.mixed1 & 0x30) >> 4;
			logic_str = StringMapValue(logic_map, logic, found);
			ss << (found ? logic_str : "");
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
			r = fmt.mixed1 & 0x7;
			ss << "P";
			if (r != 7)
				ss << r;
			else
				ss << "T";
		}
		else if (Common::Asm::IsToken(fmt_str, "rnd", len))
		{
			unsigned int rnd;
			string rnd_str;
			bool found;
			rnd = (fmt.mixed1 & 0xc0) >> 6;
			rnd_str = StringMapValue(rnd_map, rnd, found);
			ss << (found ? rnd_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "s_", len))
		{
			unsigned int s;
			s = fmt.mixed0 & 0x1;
			ss << ((s != 0) ? ".S" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "s32", len))
		{
			unsigned int s32;
			s32 = fmt.mixed0 & 0x2;
			ss << ((s32 != 0) ? ".S32" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "sat", len))
		{
			unsigned int sat;
			sat = fmt.mixed1 & 0x2;
			ss << ((sat != 0) ? ".SAT" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "shamt", len))
		{
			unsigned int shamt;
			shamt = (fmt.mixed0 & 0x1f) >> 1;
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
			unsigned int src2;
			unsigned int bank_id;
			unsigned int offset_in_bank;
			unsigned int neg;
			unsigned int abs;

			src2_mod = fmt.src2_mod;
			src2 = fmt.src2;
			neg = fmt.mixed0 & 0x10;
			abs = fmt.mixed0 & 0x4;

			if (src2_mod == 0)
			{
				ss << ((neg != 0) ? "-" : "");
				ss << ((abs != 0) ? "|" : "");
				ss << "R";
				if (src2 != 63)
					ss << src2;
				else
					ss << "Z";
				ss << ((abs != 0) ? "|" : "");
			}
			else if (src2_mod == 1 || src2_mod == 2)
			{
				bank_id = ((src2 & 0x1) << 4) |	(src2 >> 16);
				offset_in_bank = src2 &	0xfffc;
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
			unsigned int u32;
			u32 = fmt.mixed0 & 0x2;
			ss << ((u32 != 0) ? "" : "U32");
		}
		else if (Common::Asm::IsToken(fmt_str, "w", len))
		{
			unsigned int w;
			w = fmt.mixed0 & 0x20;
			ss << ((w != 0) ? ".W" : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "type1", len))
		{
			unsigned int type1;
			type1 = fmt.mixed0 & 0x8;
			ss << ((type1 != 0) ? "" : ".S32");
		}
		else if (Common::Asm::IsToken(fmt_str, "type2", len))
		{
			unsigned int type2;
			type2 = fmt.mixed0 & 0x2;
			ss << ((type2 != 0) ? "" : ".S32");
		}
		else if (Common::Asm::IsToken(fmt_str, "x", len))
		{
			unsigned int x;
			x = fmt.mixed0 & 0x4;
			ss << ((x != 0) ? ".X" : "");
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

		if (Common::Asm::IsToken(fmt_str, "pred", len))
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
		else if (Common::Asm::IsToken(fmt_str, "imm32", len))
		{
			unsigned int imm32;
			imm32 = fmt.imm32;
			ss << hex << "0x" << imm32;
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

		if (Common::Asm::IsToken(fmt_str, "pred", len))
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
		else if (Common::Asm::IsToken(fmt_str, "s_", len))
		{
			unsigned int s;
			s = fmt.mixed0 & 0x1;
			ss << ((s != 0) ? ".S" : "");
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

		if (Common::Asm::IsToken(fmt_str, "pred", len))
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
		else if (Common::Asm::IsToken(fmt_str, "atom_op", len))
		{
			unsigned int op;
			string op_str;
			bool found;
			op = (fmt.suffix0 & 0x1e) >> 1;
			op_str = StringMapValue(atom_op_map, op, found);
			ss << (found ? op_str : ".INVALID");
		}
		else if (Common::Asm::IsToken(fmt_str, "atom_type", len))
		{
			unsigned int type;
			string type_str;
			bool found;
			type = (fmt.func & 0xe) | ((fmt.suffix0 & 0x20) >> 5);
			type_str = StringMapValue(atom_type_map, type, found);
			ss << (found ? type_str : "");
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
		else if (Common::Asm::IsToken(fmt_str, "ld_cop", len))
		{
			unsigned int ld_cop;
			string ld_cop_str;
			bool found;
			ld_cop = ((fmt.suffix0 & 0x30) >> 4);
			ld_cop_str = StringMapValue(ld_cop_map, ld_cop, found);
			ss << (found ? ld_cop_str : "");
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
		else if (Common::Asm::IsToken(fmt_str, "red_op", len))
		{
			unsigned int op;
			string op_str;
			bool found;
			op = (fmt.suffix0 & 0x1e) >> 1;
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
		else if (Common::Asm::IsToken(fmt_str, "src3", len))
		{
			unsigned int src3;
			src3 = (fmt.suffix1 & 0x7e) >> 1;
			ss << "R";
			if (src3 != 63)
				ss << src3;
			else
				ss << "Z";
		}
		else if (Common::Asm::IsToken(fmt_str, "src4", len))
		{
			unsigned int src4;
			src4 = (fmt.suffix1 & 0x1f80) >> 7;
			if (src4 != 63)
				ss << ", R" << src4;
		}
		else if (Common::Asm::IsToken(fmt_str, "st_cop", len))
		{
			unsigned int st_cop;
			string st_cop_str;
			bool found;
			st_cop = ((fmt.suffix0 & 0x30) >> 4);
			st_cop_str = StringMapValue(st_cop_map, st_cop, found);
			ss << (found ? st_cop_str : "");
		}
		else if (Common::Asm::IsToken(fmt_str, "type", len))
		{
			unsigned int type;
			string type_str;
			bool found;
			type = ((fmt.suffix0 & 0xe) >> 1);
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

		if (Common::Asm::IsToken(fmt_str, "pred", len))
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
		else if (Common::Asm::IsToken(fmt_str, "bra_target", len))
		{
			unsigned int target;
			target = fmt.target;
			ss << hex << "0x" << addr + 8 + target;
		}
		else if (Common::Asm::IsToken(fmt_str, "lmt", len))
		{
			unsigned int lmt;
			lmt = fmt.suffix1 & 0x2;
			ss << ((lmt != 0) ? ".LMT" : "");
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


	//		else if (Common::Asm::IsToken(fmt_str,"dst", len))
	//		{
	//			unsigned long long int dst;
	//			dst = bytes.general0.dst;
	//			if (dst != 63)
	//				str += "R" + to_string(dst);
	//			else
	//				str += "RZ";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str,"src1_abs_neg", len))
	//		{
	//			unsigned long long int src1;
	//			unsigned long long int src1_abs;
	//
	//			src1 = bytes.general0.src1;
	//			src1_abs = bytes.mod0_A.abs_src1;
	//			if (src1 != 63 && src1_abs == 1)
	//				str += "|R" + to_string(src1) + "|";
	//			else if (src1 != 63 && src1_abs == 0)
	//				str += "R" + to_string(src1);
	//			else
	//				str += "RZ";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "src1_neg", len))
	//		{
	//			unsigned long long int src1;
	//			unsigned long long int src_mod;
	//
	//			src1 = bytes.general0.src1;
	//			src_mod = bytes.mod0_D.fma_mod;
	//			if (src1 != 63)
	//			{
	//				if (src_mod == 0x2)
	//					str += "-R" + to_string(src1);
	//				else
	//					str += "R" + to_string(src1);
	//			}
	//			else
	//				str += "RZ";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "src1_offs", len))
	//		{
	//			unsigned long long int src1;
	//			unsigned long long int offs;
	//
	//			src1 = bytes.general0.src1;
	//			offs = bytes.offs.offset;
	//
	//			if (src1 != 63)
	//			{
	//				str += "[R" + to_string(src1);
	//
	//				if (offs != 0)
	//				{
	//					stringstream ss;
	//					ss << "+0x"  << hex << offs;
	//					str += ss.str();
	//				}
	//			}
	//
	//			else
	//			{
	//				stringstream ss;
	//				ss << "[0x"  << hex << offs;
	//				str += ss.str();
	//			}
	//
	//			str += "]";
	//		}
	//

	//
	//		else if (Common::Asm::IsToken(fmt_str,"src23", len))
	//		{
	//			unsigned long long int bank_id;
	//			unsigned long long int offset_in_bank;
	//			unsigned long long int src3;
	//			unsigned long long int src2_neg;
	//			unsigned long long int src3_neg;
	//
	//			src3 = bytes.general0_mod1_B.src3;
	//			src2_neg = bytes.mod0_A.neg_src1;
	//			src3_neg = bytes.mod0_A.neg_src2;
	//
	//			if (bytes.general0.src2_mod == 0)
	//			{
	//				if (bytes.general0.src2 != 0x3f)
	//				{
	//					if (src2_neg)
	//						str += "-R" + to_string(bytes.general0.src2 & 0x3f) + ", ";
	//					else
	//						str += "R" + to_string(bytes.general0.src2 & 0x3f) + ", ";
	//				}
	//				else
	//					str += "RZ, ";
	//				if (src3 != 63)
	//				{
	//					if (src3_neg)
	//						str += "-R" + to_string(src3);
	//					else
	//						str += "R" + to_string(src3);
	//				}
	//				else
	//					str += "RZ";
	//			}
	//			else if (bytes.general0.src2_mod == 1)
	//			{
	//				bank_id = ((bytes.general0.src2 & 0x1) << 4) |
	//					(bytes.general0.src2 >> 16);
	//				offset_in_bank= bytes.general0.src2 &
	//					0xfffc;
	//				stringstream ss;
	//				ss << "c[0x" << hex << bank_id << "][0x" << hex << offset_in_bank << "], ";
	//				str += ss.str();
	//				if (src3 != 63)
	//				{
	//					if (src3_neg)
	//						str += "-R" + to_string(src3);
	//					else
	//						str += "R" + to_string(src3);
	//				}
	//				else
	//					str += "RZ";
	//			}
	//			else if (bytes.general0.src2_mod == 2)
	//			{
	//				if (src3 != 63)
	//				{
	//					if (src3_neg)
	//						str += "-R" + to_string(src3) + ", ";
	//					else
	//						str += "R" + to_string(src3) + ", ";
	//				}
	//				else
	//					str += "RZ, ";
	//				bank_id = ((bytes.general0.src2 & 0x1) << 4) |
	//					(bytes.general0.src2 >> 16);
	//				offset_in_bank= bytes.general0.src2 &
	//					0xfffc;
	//				stringstream ss;
	//				ss << "c[0x" << hex << bank_id << "][0x" << hex << offset_in_bank << "]";
	//				str += ss.str();
	//			}
	//			else if (bytes.general0.src2_mod == 3)
	//			{
	//				stringstream ss;
	//				ss << "0x" << hex << bytes.general0.src2 << ", ";
	//				str += ss.str();
	//				if (src3 != 63)
	//				{
	//					if (src3_neg)
	//						str += "-R" + to_string(src3);
	//					else
	//						str += "R" + to_string(src3);
	//				}
	//				else
	//					str += "RZ";
	//			}
	//		}
	//
	//		/* This is a special case for src2 and src3. For FFMA,
	//		   the sequence of output from cuobjdump is somehow depends on the src2_mod
	//		   it prints src3 first when src2_mod > 2, however prints src2 first when src2_mod < 2 */
	//		else if (Common::Asm::IsToken(fmt_str,"src2_FFMA", len))
	//		{
	//			unsigned long long int bank_id;
	//			unsigned long long int offset_in_bank;
	//			unsigned long long int src3;
	//
	//			bank_id = bytes.general0.src2 >> 16;
	//			offset_in_bank= bytes.general0.src2 & 0xffff;
	//			src3 = bytes.general0_mod1_B.src3;
	//
	//			/* print out src2 */
	//			if (bytes.general0.src2_mod < 2)
	//			{
	//				if (bytes.general0.src2_mod == 0)
	//					str += "R" + to_string(bytes.general0.src2 & 0x3f);
	//				else if (bytes.general0.src2_mod == 1)
	//				{
	//					stringstream ss;
	//					ss << "c[0x" << hex << bank_id << "][0x" << hex << offset_in_bank << "]";
	//					str += ss.str();
	//				}
	//			}
	//
	//			/* print out src3 */
	//			else
	//			{
	//				if (src3 != 63)
	//					str += ", R" + to_string(src3);
	//				else
	//					str += ", RZ";
	//			}
	//		}
	//
	//		/* This is a special case for src2.
	//		 * For LDC, src2 is always a constant memory address, no matter
	//		 * what the value of src2_mod is. */
	//		else if (Common::Asm::IsToken(fmt_str,"src2_LDC", len))
	//		{
	//			unsigned long long int bank_id;
	//			unsigned long long int src1;
	//			unsigned long long int offset_in_bank;
	//
	//			bank_id = bytes.general0.src2 >> 16;
	//			src1= bytes.general0.src1;
	//			offset_in_bank= bytes.general0.src2 & 0xffff;
	//
	//			/* print out src2 */
	//			stringstream ss;
	//			ss << "c[0x" << hex << bank_id << "][R" << src1;
	//			if (offset_in_bank != 0)
	//				ss << "+0x" << hex << offset_in_bank;
	//			ss << "]";
	//			str += ss.str();
	//		}
	//

	//
	//		else if (Common::Asm::IsToken(fmt_str,"src2_abs_neg", len))
	//		{
	//			unsigned long long int src2;
	//			unsigned long long int src2_abs;
	//
	//			src2 = bytes.general0.src2;
	//			src2_abs = bytes.mod0_A.abs_src2;
	//			if (src2 != 63 && src2_abs == 1)
	//				str += "|R" + to_string(src2) + "|";
	//			else if (src2 != 63 && src2_abs == 0)
	//				str += "R" + to_string(src2);
	//			else
	//				str += "RZ";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str,"src2_neg", len))
	//		{
	//			unsigned long long int bank_id;
	//			unsigned long long int offset_in_bank;
	//
	//			stringstream ss;
	//			if (bytes.general0.src2_mod == 0)
	//			{
	//				if (bytes.mod0_A.neg_src2)
	//					str += "-";
	//				str += "R" + to_string(bytes.general0.src2 & 0x3f);
	//			}
	//			else if (bytes.general0.src2_mod == 1)
	//			{
	//				bank_id = bytes.general0.src2 >> 16;
	//				offset_in_bank= bytes.general0.src2 & 0xffff;
	//				ss << "c[0x" << hex << bank_id << "][0x" << hex << offset_in_bank << "]";
	//				str += ss.str();
	//			}
	//			else if (bytes.general0.src2_mod == 2)
	//			{
	//				ss << "0x" << hex << bytes.general0.src2;
	//				str += ss.str();
	//			}
	//			else if (bytes.general0.src2_mod == 3)
	//			{
	//				ss << "0x" << hex << bytes.general0.src2;
	//				str += ss.str();
	//			}
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str,"src2", len))
	//		{
	//			unsigned long long int bank_id;
	//			unsigned long long int offset_in_bank;
	//
	//			stringstream ss;
	//			if (bytes.general0.src2_mod == 0)
	//			{
	//				if (bytes.general0.src2 != 0x3f)
	//					str += "R" + to_string(bytes.general0.src2 & 0x3f);
	//				else
	//					str += "RZ";
	//			}
	//			else if (bytes.general0.src2_mod == 1 ||
	//					bytes.general0.src2_mod == 2)
	//			{
	//				bank_id = ((bytes.general0.src2 & 0x1) << 4) |
	//					(bytes.general0.src2 >> 16);
	//				offset_in_bank= bytes.general0.src2 &
	//					0xfffc;
	//				ss << "c[0x" << hex << bank_id << "][0x" << hex << offset_in_bank << "]";
	//				str += ss.str();
	//			}
	//			else if (bytes.general0.src2_mod == 3)
	//			{
	//				ss << "0x" << hex << bytes.general0.src2;
	//				str += ss.str();
	//			}
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "src3_FFMA", len))
	//		{
	//			unsigned long long int src3;
	//			unsigned long long int bit26;
	//			unsigned long long int bit28;
	//
	//			src3 = bytes.general0_mod1_B.src3;
	//			bit26 = bytes.general0.src2 & 0x1;
	//			bit28 = bytes.general0.src2 >> 2 &0x1;
	//
	//			/* print out src2 */
	//			if (bytes.general0.src2_mod < 2)
	//			{
	//				if (src3 != 63)
	//					str += "R" + to_string(src3);
	//				else
	//					str += "RZ";
	//			}
	//
	//			else
	//			{
	//				/* FIXME : we need to figure out how bit26 and bit28 control src2*/
	//				if (bit26 == 0 && bit28 == 0)
	//					str += "c[0x0][0x0]";
	//				else if (bit26 == 1 && bit28 == 0)
	//					str += "c[0x10][0x0]";
	//				else if (bit26 == 0 && bit28 == 1)
	//					str += "c[0x0][0x4]";
	//				else
	//					str += "c[0x10][0x4]";
	//			}
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str,"src3", len))
	//		{
	//			unsigned long long int src3;
	//			src3 = bytes.general0_mod1_B.src3;
	//			if (src3 != 63)
	//				str += "R" + to_string(src3);
	//			else
	//				str += "RZ";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str,"tgt_lmt", len))
	//		{
	//			if (bytes.tgt.noinc)
	//				str += ".LMT";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str,"tgt_noinc", len))
	//		{
	//			if (!bytes.tgt.noinc)
	//				str += ".noinc";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str,"tgt_u", len))
	//		{
	//			if (bytes.tgt.u)
	//				str += ".U";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str,"abs_target", len))
	//		{
	//			unsigned long long int target;
	//
	//			target = bytes.tgt.target;
	//			stringstream ss;
	//			ss << "0x" << hex << target;
	//			str += ss.str();
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str,"tgt", len))
	//		{
	//			unsigned long long int target;
	//
	//			target = bytes.tgt.target;
	//			target = SignExtend64(target, 24);
	//			target += addr + 8;
	//			stringstream ss;
	//			ss << "0x" << hex << target;
	//			str += ss.str();
	//		}
	//

	//

	//
	//		else if (Common::Asm::IsToken(fmt_str,"gen1_logic", len))
	//		{
	//			str += StringMapValue(inst_logic_map, bytes.general1.logic);
	//		}
	//
	//		/* 2nd level token such as mod0, mod1, P, Q*/
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_A_ftz", len))
	//		{
	//			if (bytes.mod0_A.satftz)
	//				str += ".FTZ";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_A_op", len))
	//		{
	//			unsigned long long int op;
	//			op = bytes.mod0_A.abs_src2 << 1 || bytes.mod0_A.satftz;
	//			str += StringMapValue(inst_op56_map,op);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_A_redarv", len))
	//		{
	//			if (bytes.mod0_A.abs_src1)
	//				str += ".ARV";
	//			else
	//				str += ".RED";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_A_s", len))
	//		{
	//			if (bytes.mod0_A.s)
	//				str += ".S";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_A_u32", len))
	//		{
	//			if (!bytes.mod0_A.satftz)
	//				str += ".U32";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_A_w", len))
	//		{
	//			if (bytes.mod0_A.neg_src1)
	//				str += ".W";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_B_brev", len))
	//		{
	//			if (bytes.mod0_B.cop)
	//				str += ".brev";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_B_cop", len))
	//		{
	//			str += StringMapValue(inst_cop_map, bytes.mod0_B.cop);
	//		}
	//

	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_B_u32", len))
	//		{
	//			if (!bytes.mod0_B.type)
	//				str += ".U32";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_C_ccop", len))
	//		{
	//			str += StringMapValue(inst_ccop_map, bytes.mod0_C.shamt);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_C_shamt", len))
	//		{
	//			unsigned long long int shamt;
	//			shamt = bytes.mod0_C.shamt;
	//			stringstream ss;
	//			ss << "0x" << hex << shamt;
	//			str += ss.str();
	//		}
	//
	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_D_ftzfmz", len))
	//		{
	//			str += StringMapValue(inst_ftzfmz_map, bytes.mod0_D.ftzfmz);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_D_op67", len))
	//		{
	//			str += StringMapValue(inst_op67_map, bytes.mod0_D.ftzfmz);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_D_op", len))
	//		{
	//			str += StringMapValue(inst_op_map, bytes.mod0_D.ftzfmz);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_D_round", len))
	//		{
	//			str += StringMapValue(inst_round_map, bytes.mod0_D.ftzfmz >> 1);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_D_sat", len))
	//		{
	//			str += StringMapValue(inst_sat_map, bytes.mod0_D.sat);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "mod0_D_x", len))
	//		{
	//			unsigned long long int x;
	//			x = bytes.mod0_D.ftzfmz & 0x1;
	//			if (x)
	//				str += ".X";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "gen0_mod1_B_rnd", len))
	//		{
	//			str += StringMapValue(inst_rnd_map, bytes.general0_mod1_B.rnd);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "gen0_mod1_C_rnd", len))
	//		{
	//			str += StringMapValue(inst_rnd1_map, bytes.general0_mod1_C.rnd);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "gen0_mod1_D_cmp", len))
	//		{
	//			str += StringMapValue(inst_cmp_map, bytes.general0_mod1_D.cmp);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "gen0_src1_dtype_n", len))
	//		{
	//			unsigned long long int dtype_n;
	//			dtype_n = bytes.general0.src1 & 0x3;
	//			str += StringMapValue(inst_dtype_n_map, dtype_n);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "gen0_src1_dtype", len))
	//		{
	//			unsigned long long int dtype;
	//			dtype = bytes.general0.src1 & 0x3;
	//			str += StringMapValue(inst_dtype_map, dtype);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "gen0_src1_stype_n", len))
	//		{
	//			unsigned long long int stype_n;
	//			stype_n = bytes.general0.src1 >> 3 & 0x3;
	//			str += StringMapValue(inst_stype_n_map, stype_n);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "gen0_src1_stype", len))
	//		{
	//			unsigned long long int stype;
	//			stype = bytes.general0.src1 >> 3 & 0x3;
	//			str += StringMapValue(inst_stype_map, stype);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "dtype_sn", len))
	//		{
	//			unsigned long long int dtype_n;
	//			if (bytes.mod0_A.abs_src1)
	//			        str += ".S";
	//			else
	//			        str += ".U";
	//			dtype_n = bytes.general0.src1 & 0x3;
	//			str += StringMapValue(inst_stype_n_map, dtype_n);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "stype_sn", len))
	//		{
	//			unsigned long long int stype_n;
	//			if (bytes.mod0_A.neg_src1)
	//			        str += ".S";
	//		        else
	//			        str += ".U";
	//			stype_n = bytes.general0.src1 >> 3 & 0x3;
	//			str += StringMapValue(inst_stype_n_map, stype_n);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "offs_op1_e", len))
	//		{
	//			unsigned long long int e;
	//			e = bytes.offs.op1 & 0x1;
	//			if (e)
	//				str += ".e";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str,"offs", len))
	//		{
	//			unsigned long long int offs;
	//			offs = bytes.offs.offset;
	//			stringstream ss;
	//			if (offs)
	//				ss << "+0x" << hex << offs;
	//			str += ss.str();
	//		}



	//		else if (Common::Asm::IsToken(fmt_str, "P", len))
	//		{
	//			unsigned long long int P;
	//			P = bytes.general1.dst >> 3;
	//			str += "P" + to_string(P);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "Q", len))
	//		{
	//			unsigned long long int Q;
	//			Q = bytes.general1.dst & 0x7;
	//			if (Q != 7)
	//				str += "P" + to_string(Q);
	//			else
	//				str += "PT";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "R", len))
	//		{
	//			unsigned long long int R;
	//			R = bytes.general1.R;
	//			if (R < 7)
	//				str += "P" + R;
	//			else if (R == 8)
	//				str += "!P0";
	//			else
	//				str += "PT";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "FADD_sat", len))
	//		{
	//			unsigned long long int sat;
	//			sat = bytes.general0_mod1_B.src3 & 0x1;
	//			if (sat)
	//				str += ".SAT";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "MUFU_op", len))
	//		{
	//			unsigned long long int op;
	//			op = bytes.imm.imm32;
	//			str += StringMapValue(inst_op_map, op);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "NOP_op", len))
	//		{
	//			unsigned long long int op;
	//			op = bytes.offs.mod1 >> 9 & 0x4;
	//			str += StringMapValue(inst_NOP_op_map, op);
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "IMAD_hi", len))
	//		{
	//			unsigned long long int hi;
	//			hi= bytes.mod0_D.ftzfmz & 0x1;
	//			if (hi)
	//				str += ".HI";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "IMAD_mod1", len))
	//		{
	//			unsigned long long int mod1;
	//			mod1 = bytes.mod0_D.ftzfmz >> 1;
	//			if (!mod1)
	//				str += ".U32";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "IMAD_mod2", len))
	//		{
	//			unsigned long long int mod2;
	//			mod2 = bytes.mod0_D.sat;
	//			if (!mod2)
	//				str += ".U32";
	//		}
	//
	//		else if (Common::Asm::IsToken(fmt_str, "IMAD_sat", len))
	//		{
	//			unsigned long long int sat;
	//			sat = bytes.general0_mod1_B.rnd >> 1;
	//			if (sat)
	//				str += ".SAT";
	//		}

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
