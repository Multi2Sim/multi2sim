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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "asm.h"
#include "inst.h"


/*
 * Class 'KplInst'
 */

void KplInstCreate(KplInst *self, KplAsm *as)
{
	self->as = as;
}


void KplInstDestroy(KplInst *self)
{
}


void KplInstDecode(KplInst *self, void *buf, unsigned int addr)
{
	KplAsm *as = self->as;
	KplInstTableEntry *table;

	int index;
	int low;
	int high;

	/* Populate */
	self->bytes.as_dword = * (unsigned long long *) buf;
	self->addr = addr;

	/* Start with master table */
	table = as->dec_table;
	low = 0;
	high = 1;

	/* Traverse tables */
	while (1)
	{
		index = BITS64(self->bytes.as_dword, high, low);
		if (!table[index].next_table)
		{
			self->info = table[index].info;
			return;
		}

		/* Go to next table */
		low = table[index].next_table_low;
		high = table[index].next_table_high;
		table = table[index].next_table;
	}
}


void KplInstDumpHex(KplInst *self, FILE *f)
{
	fprintf(f, "\n\t/*%04x*/     /*%08x%08x*/ \t",
			self->addr,
			self->bytes.as_uint[0],
			self->bytes.as_uint[1]);
}


struct str_map_t kpl_inst_sat_map =
{
	2,
	{
		{ "", 0},
		{ ".SAT", 1}
	}
};

struct str_map_t kpl_inst_x_map =
{
	2,
	{
		{ "", 0},
		{ ".X", 1}
	}
};

struct str_map_t kpl_inst_cc2_map =
{
	32,
	{
                { "CC.F", 0},
		{ "CC.LT", 1},
		{ "CC.EQ", 2},
		{ "CC.LE", 3},
		{ "CC.GT", 4},
		{ "CC.NE", 5},
		{ "CC.GE", 6},
		{ "CC.NUM", 7},
		{ "CC.NAN", 8},
		{ "CC.LTU", 9},
		{ "CC.EQU", 10},
		{ "CC.LEU", 11},
		{ "CC.GTU", 12},
		{ "CC.NEU", 13},
		{ "CC.GEU", 14},
		{ "", 15},
		{ "CC.OFF", 16},
		{ "CC.LO", 17},
		{ "CC.SFF", 18},
		{ "CC.LS", 19},
		{ "CC.HI", 20},
		{ "CC.SFT", 21},
		{ "CC.HS", 22},
		{ "CC.OFT", 23},
		{ "CC.CSM_TA", 24},
		{ "CC.CSM_TR", 25},
		{ "CC.CSM_MX", 26},
		{ "CC.FCSM_TA", 27},
		{ "CC.FCSM_TR", 28},
		{ "CC.FCSM_MX", 29},
		{ "CC.RLE", 30},
		{ "CC.RGT", 31}
	}
};

struct str_map_t kpl_inst_cc_map =
{
	2,
	{
		{ "", 0},
		{ ".CC", 1}
	}
};

struct str_map_t kpl_inst_hi_map =
{
	2,
	{
		{ "", 0},
		{ ".HI", 1}
	}
};

struct str_map_t kpl_inst_keeprefcount_map =
{
	2,
	{
		{ "", 0},
		{ ".KEEPREFCOUNT", 1}
	}
};

struct str_map_t kpl_inst_s_map =
{
	2,
	{
		{ "", 0},
		{ ".S", 1}
	}
};

struct str_map_t kpl_inst_and_map =
{
	4,
	{
		{ ".AND", 0},
		{ ".OR", 1},
		{ ".XOR", 2},
		{ ".INVALIDBOP3", 3}
	}
};

struct str_map_t kpl_inst_cv_map =
{
	4,
	{
		{ "", 0},
		{ ".CG", 1},
		{ ".CS", 2},
		{ ".CV", 3}
	}
};

struct str_map_t kpl_inst_wt_map =
{
	4,
	{
		{ "", 0},
		{ ".CG", 1},
		{ ".CS", 2},
		{ ".WT", 3}
	}
};

struct str_map_t kpl_inst_po_map =
{
	2,
	{
		{ "", 0},
		{ ".PO", 1}
	}
};

struct str_map_t kpl_inst_u1_map =
{
	2,
	{
		{ ".U32.U32", 0},
		{ ".U32.S32", 1}
	}
};

struct str_map_t kpl_inst_f_map=
{
	8,
	{
		{ ".F", 0},
		{ ".LT", 1},
		{ ".EQ", 2},
		{ ".LE", 3},
		{ ".GT", 4},
		{ ".NE", 5},
		{ ".GE", 6},
		{ ".T", 7}
	}
};

struct str_map_t kpl_inst_u_map =
{
	2,
	{
		{ "", 0},
		{ ".U", 1}
	}
};

struct str_map_t kpl_inst_lmt_map =
{
	2,
	{
		{ "", 0},
		{ ".LMT", 1}
	}
};

struct str_map_t kpl_inst_e_map =
{
	2,
	{
		{ "", 0},
		{ ".E", 1}
	}
};

struct str_map_t kpl_inst_u32_map =
{
	2,
	{
		{ ".U32", 0},
		{ "", 1}
	}
};

struct str_map_t kpl_inst_rm_map =
{
	4,
	{
		{ "", 0},
		{ ".RM", 1},
		{ ".RP", 2},
		{ ".RZ", 3}
	}
};

struct str_map_t kpl_inst_us_map =
{
	2,
	{
		{ ".U32", 0},
		{ ".S32", 1},
	}
};


static void KplInstDumpPredShort(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
        int value;

        value = BITS64(self->bytes.as_dword, high, low);
        if (value == 7)
		str_printf(buf_ptr, size_ptr, "PT");
	else
                str_printf(buf_ptr, size_ptr, "P%d", value & 7);
}


static void KplInstDumpPredNoat(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
        int value;

        value = BITS64(self->bytes.as_dword, high, low);
        if (value != 7)
        {
                if (value >> 3)
                        str_printf(buf_ptr, size_ptr, "!");
                if (value == 15)
                        str_printf(buf_ptr, size_ptr, "PT");
                else
                        str_printf(buf_ptr, size_ptr, "P%d", value & 7);
	}
	else
		str_printf(buf_ptr, size_ptr, "PT");
}


static void KplInstDumpPred(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(self->bytes.as_dword, high, low);
	if (value != 7)
	{
		str_printf(buf_ptr, size_ptr, "@");
		if (value >> 3)
			str_printf(buf_ptr, size_ptr, "!");
		if (value == 15)
			str_printf(buf_ptr, size_ptr, "PT ");
		else
			str_printf(buf_ptr, size_ptr, "P%d ", value & 7);
	}
}


static void KplInstDumpReg(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(self->bytes.as_dword, high, low);
	if(value == 255)
		str_printf(buf_ptr, size_ptr, "RZ");
	else
		str_printf(buf_ptr, size_ptr, "R%d", value);
}


static void KplInstDumpSpecReg(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
        int value;

        value = BITS64(self->bytes.as_dword, high, low);
        if(value == 61)
                str_printf(buf_ptr, size_ptr, "SR_RegAlloc");
	else if(value == 62)
		str_printf(buf_ptr, size_ptr, "SR_CtxAddr");
        else
                str_printf(buf_ptr, size_ptr, "SR%d", value);
}


static void KplInstDumpS(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(self->bytes.as_dword, high, low);
	str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_s_map, value));
}


static void KplInstDumpF(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(self->bytes.as_dword, high, low);
	str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_f_map, value));
}


static void KplInstDumpAnd(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(self->bytes.as_dword, high, low);
	str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_and_map, value));
}


static void KplInstDumpX(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(self->bytes.as_dword, high, low);
	str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_x_map, value));
}


static void KplInstDumpU32(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(self->bytes.as_dword, high, low);
	str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_u32_map, value));
}


static void KplInstDumpHi(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(self->bytes.as_dword, high, low);
	str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_hi_map, value));
}


static void KplInstDumpSat(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(self->bytes.as_dword, high, low);
	str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_sat_map, value));
}


static void KplInstDumpPo(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
	int value;

	value = BITS64(self->bytes.as_dword, high, low);
	str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_po_map, value));
}


static void KplInstDumpUs(KplInst *self, char **buf_ptr, int *size_ptr, int high0, int low0, int high1, int low1)
{
	int value0;
	int value1;

	value0 = BITS64(self->bytes.as_dword, high0, low0);
	value1 = BITS64(self->bytes.as_dword, high1, low1);

	if(value0 == 1 && value1 == 1)
		str_printf(buf_ptr, size_ptr, "%s", "");
	else
	{
		str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_us_map, value1));
		str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_us_map, value0));
	}
}


static void KplInstDumpCc(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
        int value;

        value = BITS64(self->bytes.as_dword, high, low);
        str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_cc_map, value));
}


void KplInstDumpE(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
        int value;

        value = BITS64(self->bytes.as_dword, high, low);
        str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_e_map, value));
}


static void KplInstDumpCv(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
        int value;

        value = BITS64(self->bytes.as_dword, high, low);
        str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_cv_map, value));
}


static void KplInstDumpLmt(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
        int value;

        value = BITS64(self->bytes.as_dword, high, low);
        str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_lmt_map, value));
}


static void KplInstDumpU(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
        int value;

        value = BITS64(self->bytes.as_dword, high, low);
        str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_u_map, value));
}


static void KplInstDumpRm(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
        int value;

        value = BITS64(self->bytes.as_dword, high, low);
        str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_rm_map, value));
}


static void KplInstDumpKeepRefCount(KplInst *self, char **buf_ptr, int *size_ptr, int high, int low)
{
        int value;

        value = BITS64(self->bytes.as_dword, high, low);
        str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_keeprefcount_map, value));
}


static void KplInstDumpCc2(KplInst *self, char **buf_ptr, int *size_ptr, int high, int   low)
{
        int value;

        value = BITS64(self->bytes.as_dword, high, low);
        str_printf(buf_ptr, size_ptr, "%s", str_map_value(&kpl_inst_cc2_map, value));
}


void KplInstDumpBuf(KplInst *self, char *buf, int size)
{
	char *fmt_str;

	/* Invalid instruction */
	if (!self->info || !self->info->fmt_str)
	{
		snprintf(buf, size, "<unknown>");
		return;
	}

	/* Print entire format string temporarily */
	fmt_str = self->info->fmt_str;
	while (*fmt_str)
	{
                if (str_prefix(fmt_str, "%srcB"))
                {
                        KplInstDumpReg(self, &buf, &size, 30, 23);
                        fmt_str += 5;
                        continue;
                }

	        if (str_prefix(fmt_str, "%src_spec"))
                {
                        KplInstDumpSpecReg(self, &buf, &size, 30, 23);
                        fmt_str += 9;
                        continue;
                }

	        if (str_prefix(fmt_str, "%cc"))
                {
                        KplInstDumpCc2(self, &buf, &size, 6, 2);
                        fmt_str += 3;
                        continue;
                }

                if (str_prefix(fmt_str, "%pred1"))
                {
                        KplInstDumpPredShort(self, &buf, &size, 7, 5);
                        fmt_str += 6;
                        continue;
                }

                if (str_prefix(fmt_str, "%pred2"))
                {
                        KplInstDumpPredShort(self, &buf, &size, 4, 2);
                        fmt_str += 6;
                        continue;
                }

                if (str_prefix(fmt_str, "%pred3"))
                {
                        KplInstDumpPredNoat(self, &buf, &size, 45, 42);
                        fmt_str += 6;
                        continue;
                }

                if (str_prefix(fmt_str, "%_keeprefcount"))
                {
                        KplInstDumpKeepRefCount(self, &buf, &size, 7, 7);
                        fmt_str += 14;
                        continue;
                }

                if (str_prefix(fmt_str, "%_rm"))
                {
                        KplInstDumpRm(self, &buf, &size, 43, 42);
                        fmt_str += 4;
                        continue;
                }

                if (str_prefix(fmt_str, "%_LMT"))
                {
                        KplInstDumpLmt(self, &buf, &size, 8, 8);
                        fmt_str += 5;
                        continue;
                }

	       if (str_prefix(fmt_str, "%_cg"))
                {
                        KplInstDumpCv(self, &buf, &size, 50, 50);
                        fmt_str += 4;
                        continue;
                }

                if (str_prefix(fmt_str, "%_e"))
                {
                        KplInstDumpCc(self, &buf, &size, 60, 59);
                        fmt_str += 3;
                        continue;
                }

		if (str_prefix(fmt_str, "%_cc"))
                {
                        KplInstDumpCc(self, &buf, &size, 50, 50);
                        fmt_str += 4;
                        continue;
                }

		if (str_prefix(fmt_str, "%_us"))
		{
			KplInstDumpUs(self, &buf, &size, 56, 56, 51, 51);
			fmt_str += 4;
			continue;
		}

		if (str_prefix(fmt_str, "%_po"))
		{
			KplInstDumpPo(self, &buf, &size, 55, 55);
			fmt_str += 4;
			continue;
		}

		if (str_prefix(fmt_str, "%_sat"))
		{
			KplInstDumpSat(self, &buf, &size, 53, 53);
			fmt_str += 5;
			continue;
		}

		if (str_prefix(fmt_str, "%_hi"))
		{
			KplInstDumpHi(self, &buf, &size, 57, 57);
			fmt_str += 4;
			continue;
		}

		if (str_prefix(fmt_str, "%_U32"))
		{
			KplInstDumpU32(self, &buf, &size, 51, 51);
			fmt_str += 5;
			continue;
		}

		if (str_prefix(fmt_str, "%_x"))
		{
			KplInstDumpX(self, &buf, &size, 46, 46);
			fmt_str += 3;
			continue;
		}

		if (str_prefix(fmt_str, "%_and"))
		{
			KplInstDumpAnd(self, &buf, &size, 49, 48);
			fmt_str += 5;
			continue;
		}

		if (str_prefix(fmt_str, "%_f"))
		{
			KplInstDumpF(self, &buf, &size, 54, 52);
			fmt_str += 3;
			continue;
		}

		if (str_prefix(fmt_str, "%_s"))
		{
			KplInstDumpS(self, &buf, &size, 22, 22);
			fmt_str += 3;
			continue;
		}

                if (str_prefix(fmt_str, "%_u"))
                {
                        KplInstDumpU(self, &buf, &size, 9, 9);
                        fmt_str += 3;
                        continue;
                }

		if (str_prefix(fmt_str, "%srcA"))
		{
			KplInstDumpReg(self, &buf, &size, 17, 10);
			fmt_str += 5;
			continue;
		}

		if (str_prefix(fmt_str, "%dst"))
		{
			KplInstDumpReg(self, &buf, &size, 9, 2);
			fmt_str += 4;
			continue;
		}

		if (str_prefix(fmt_str, "%pred0"))
		{
			KplInstDumpPred(self, &buf, &size, 21, 18);
			fmt_str += 6;
			continue;
		}

		if (str_prefix(fmt_str, "%pred"))
		{
			KplInstDumpPred(self, &buf, &size, 21, 18);
			fmt_str += 5;
			continue;
		}

		/* Print literal character */
		str_printf(&buf, &size, "%c", *fmt_str);
		++fmt_str;
	}
}


void KplInstDump(KplInst *self, FILE *f)
{
	char buf[200];

	KplInstDumpBuf(self, buf, sizeof buf);
	fprintf(f, "%s", buf);
}

