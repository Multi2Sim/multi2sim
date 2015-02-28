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
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <typeinfo>

#include <lib/cpp/Error.h>
#include <lib/cpp/Misc.h>

#include "Asm.h"
#include "Inst.h"


namespace Fermi
{


misc::StringMap _1d_map =
{
		{ ".1D", 0 },
		{ ".2D", 1 },
		{ ".3D", 2 },
		{ ".E2D", 3 }
};

misc::StringMap addsub_map =
{
		{ "ADD", 0 },
		{ "SUB", 1 },
		{ "SUB", 2 },
		{ "ADD", 3 }
};

misc::StringMap aoffi_map =
{
		{ "", 0 },
		{ ".AOFFI", 1 }
};

misc::StringMap atomicsize_map =
{
		{ ".U8", 0 },
		{ ".S8", 1 },
		{ ".U16", 2 },
		{ ".S16", 3 },
		{ "", 4 },
		{ ".U64", 5 },
		{ ".U128", 6 },
		{ ".S32", 7 },
		{ ".S64", 8 },
		{ ".S128", 9 },
		{ ".F16", 10 },
		{ ".F32.FTZ.RN", 11 },
		{ ".F64", 12 },
		{ ".INVALIDATOMICSIZE13", 13 },
		{ ".INVALIDATOMICSIZE14", 14 },
		{ ".INVALIDATOMICSIZE15", 15 }
};

misc::StringMap atomop_map =
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
		{ ".CAS", 9 },
		{ ".INVALIDATOMOP10", 10 },
		{ ".INVALIDATOMOP11", 11 },
		{ ".INVALIDATOMOP12", 12 },
		{ ".INVALIDATOMOP13", 13 },
		{ ".INVALIDATOMOP14", 14 },
		{ ".INVALIDATOMOP15", 15 },
};

misc::StringMap b1_map =
{
		{ ".B", 0 },
		{ ".P", 1 }
};

misc::StringMap b4_map =
{
		{ ".B", 0 },
		{ ".P", 1 },
		{ ".P", 2 },
		{ ".P", 3 },
		{ ".P", 4 },
		{ ".P", 5 },
		{ ".P", 6 },
		{ ".P", 7 },
		{ ".P", 8 },
		{ ".P", 9 },
		{ ".P", 10 },
		{ ".P", 11 },
		{ ".P", 12 },
		{ ".P", 13 },
		{ ".P", 14 },
		{ ".P", 15 }
};

misc::StringMap barmode_map =
{
		{ ".POPC", 0 },
		{ ".AND", 1 },
		{ ".OR", 2 },
		{ ".INVALIDBARMOD3", 3 }
};

misc::StringMap bf_map =
{
		{ "", 0 },
		{ ".BF", 1 }
};

misc::StringMap blod_map =
{
		{ "", 0 },
		{ ".LZ", 1 },
		{ ".LB", 2 },
		{ ".LL", 3 },
		{ ".INVALIDBLOD4", 4 },
		{ ".INVALIDBLOD5", 5 },
		{ ".LBA", 6 },
		{ ".LLA", 7 }
};

misc::StringMap bop_map =
{
		{ ".AND", 0 },
		{ ".OR", 1 },
		{ ".XOR", 2 },
		{ ".PASS_B", 3 }
};

misc::StringMap brev_map =
{
		{ "", 0 },
		{ ".BREV", 1 }
};

misc::StringMap btoff_map =
{
		{ "", 0 },
		{ ".AOFFI", 1 },
		{ ".PTP", 2 },
		{ ".INVALIDBTOFF03", 3 }
};

misc::StringMap cachectrl_map =
{
		{ "", 0 },
		{ "", 1 },
		{ "", 2 },
		{ "", 3 },
		{ "", 4 },
		{ "", 5 },
		{ "", 6 },
		{ ".BV", 7 },
		{ ".CG", 8 },
		{ ".CG", 9 },
		{ ".CG", 10 },
		{ ".CG", 11 },
		{ ".CG", 12 },
		{ ".CG", 13 },
		{ ".CG", 14 },
		{ ".BV.CG", 15 },
		{ ".CS", 16 },
		{ ".CS", 17 },
		{ ".CS", 18 },
		{ ".CS", 19 },
		{ ".CS", 20 },
		{ ".CS", 21 },
		{ ".CS", 22 },
		{ ".BV", 23 },
		{ ".CV", 24 },
		{ ".CV", 25 },
		{ ".CV", 26 },
		{ ".CV", 27 },
		{ ".CV", 28 },
		{ ".CV", 29 },
		{ ".CV", 30 },
		{ ".BV.CG", 31 }
};

misc::StringMap cctlop_map =
{
		{ ".QRY1", 0 },
		{ ".PF1", 1 },
		{ ".PF1_5", 2 },
		{ ".PR2", 3 },
		{ ".WB", 4 },
		{ ".IV", 5 },
		{ ".IVALL", 6 },
		{ ".RS", 7 },
		{ ".INVALIDCCTLOP8", 8 },
		{ ".INVALIDCCTLOP9", 9 },
		{ ".INVALIDCCTLOP10", 10 },
		{ ".INVALIDCCTLOP11", 11 },
		{ ".INVALIDCCTLOP12", 12 },
		{ ".INVALIDCCTLOP13", 13 },
		{ ".INVALIDCCTLOP14", 14 },
		{ ".INVALIDCCTLOP15", 15 },
		{ ".INVALIDCCTLOP16", 16 },
		{ ".INVALIDCCTLOP17", 17 },
		{ ".INVALIDCCTLOP18", 18 },
		{ ".INVALIDCCTLOP19", 19 },
		{ ".INVALIDCCTLOP20", 20 },
		{ ".INVALIDCCTLOP21", 21 },
		{ ".INVALIDCCTLOP22", 22 },
		{ ".INVALIDCCTLOP23", 23 },
		{ ".INVALIDCCTLOP24", 24 },
		{ ".INVALIDCCTLOP25", 25 },
		{ ".INVALIDCCTLOP26", 26 },
		{ ".INVALIDCCTLOP27", 27 },
		{ ".INVALIDCCTLOP28", 28 },
		{ ".INVALIDCCTLOP29", 29 },
		{ ".INVALIDCCTLOP30", 30 },
		{ ".INVALIDCCTLOP31", 31 }
};

misc::StringMap cl_map =
{
		{ "", 0 },
		{ ".CL", 1 }
};

misc::StringMap cop3_map =
{
		{ ".F", 0 },
		{ ".LT", 1 },
		{ ".EQ", 2 },
		{ ".LE", 3 },
		{ ".GT", 4 },
		{ ".NE", 5 },
		{ ".GE", 6 },
		{ ".T", 7 }
};

misc::StringMap cop4_map =
{
		{ ".F", 0 },
		{ ".LT", 1 },
		{ ".EQ", 2 },
		{ ".LE", 3 },
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
		{ ".T", 15 }
};

misc::StringMap cop_map =
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
		{ "", 15 },
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

misc::StringMap dc_map =
{
		{ "", 0 },
		{ ".DC", 1 }
};

misc::StringMap drain_map =
{
		{ ".DRAIN", 0 },
		{ ".CAL", 1 },
		{ ".PAUSE", 2 },
		{ ".TRAP", 3 }
};

misc::StringMap e_map =
{
		{ "", 0 },
		{ ".E", 1 }
};

misc::StringMap eu_map =
{
		{ "", 0 },
		{ ".EU", 1 }
};

misc::StringMap fma_map =
{
		{ "", 0 },
		{ ".FMA", 1 },
		{ ".FMA2", 2 },
		{ ".INVALID", 3 }
};

misc::StringMap fmz_map =
{
		{ "", 0 },
		{ ".FTZ", 1 },
		{ ".FMZ", 2 },
		{ ".INVALIDFMZ3", 3 }
};

misc::StringMap fpdest_map =
{
		{ ".INVALIDFPDEST0", 0 },
		{ ".F16", 1 },
		{ ".F32", 2 },
		{ ".F64", 3 }
};

misc::StringMap fpsrc_map =
{
		{ ".INVALIDFPSRC0", 0 },
		{ ".F16", 1 },
		{ ".F32", 2 },
		{ ".F64", 3 }
};

misc::StringMap frnd2_map =
{
		{ "", 0 },
		{ ".FLOOR", 1 },
		{ ".CEIL", 2 },
		{ ".TRUNC", 3 }
};

misc::StringMap frnd3_map =
{
		{ "", 0 },
		{ ".PASS", 1 },
		{ ".PASS", 2 },
		{ ".PASS", 3 },
		{ ".ROUND", 4 },
		{ ".FLOOR", 5 },
		{ ".CEIL", 6 },
		{ ".TRUNC", 7 }
};

misc::StringMap ftz_map =
{
		{ "", 0 },
		{ ".FTZ", 1 }
};

misc::StringMap geom_map =
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

misc::StringMap h1_map =
{
		{ "", 0 },
		{ ".H1", 1 }
};

misc::StringMap hi_map =
{
		{ "", 0 },
		{ ".HI", 1 }
};

misc::StringMap i_map =
{
		{ "", 0 },
		{ ".I", 1 }
};

misc::StringMap idest_map =
{
		{ ".U8", 0 },
		{ ".U16", 1 },
		{ ".U32", 2 },
		{ ".U64", 3 },
		{ ".S8", 4 },
		{ ".S16", 5 },
		{ ".S32", 6 },
		{ ".S64", 7 }
};

misc::StringMap il_map =
{
		{ "", 0 },
		{ ".IL", 1 },
		{ ".IS", 2 },
		{ ".ISL", 3 }
};

misc::StringMap irnd_map =
{
		{ "", 0 },
		{ ".RM", 1 },
		{ ".RP", 2 },
		{ ".RZ", 3 }
};

misc::StringMap isrc_map =
{
		{ ".U8", 0 },
		{ ".U16", 1 },
		{ ".U32", 2 },
		{ ".U64", 3 },
		{ ".S8", 4 },
		{ ".S16", 5 },
		{ ".S32", 6 },
		{ ".S64", 7 }
};

misc::StringMap ldcachectrl_map =
{
		{ ".CA", 0 },
		{ "", 1 },
		{ ".CS", 2 },
		{ ".CV", 3 }
};

misc::StringMap ldlcachectrl_map =
{
		{ "", 0 },
		{ ".CG", 1 },
		{ ".LU", 2 },
		{ ".CV", 3 }
};

misc::StringMap lmt_map =
{
		{ "", 0 },
		{ ".LMT", 1 }
};

misc::StringMap ls_map =
{
		{ "L", 0 },
		{ "S", 1 }
};

misc::StringMap lz_map =
{
		{ ".LZ", 0 },
		{ ".LL", 1 }
};

misc::StringMap membar_map =
{
		{ ".CTA", 0 },
		{ ".GL", 1 },
		{ ".SYS", 2 },
		{ ".INVALIDMEMBAR3", 3 }
};

misc::StringMap mode_map =
{
		{ "0", 0 },
		{ "TEX_HEADER_TEXTURE_TYPE", 1 },
		{ "2", 2 },
		{ "3", 3 },
		{ "4", 4 },
		{ "5", 5 },
		{ "6", 6 },
		{ "7", 7 },
		{ "TEX_SAMPLER_FILTER", 8 },
		{ "TEX_SAMPLER_LOD", 9 },
		{ "TEX_SAMPLER_WRAP", 10 },
		{ "TEX_SAMPLER_BORDER_COLOR", 11 },
		{ "12", 12 },
		{ "13", 13 },
		{ "14", 14 },
		{ "15", 15 },
		{ "16", 16 },
		{ "17", 17 },
		{ "18", 18 },
		{ "19", 19 },
		{ "20", 20 },
		{ "21", 21 },
		{ "22", 22 },
		{ "23", 23 },
		{ "24", 24 },
		{ "25", 25 },
		{ "26", 26 },
		{ "27", 27 },
		{ "28", 28 },
		{ "29", 29 },
		{ "30", 30 },
		{ "31", 31 }
};

misc::StringMap ms_map =
{
		{ "", 0 },
		{ ".MS", 1 }
};

misc::StringMap mufuopcode_map =
{
		{ ".COS", 0 },
		{ ".SIN", 1 },
		{ ".EX2", 2 },
		{ ".LG2", 3 },
		{ ".RCP", 4 },
		{ ".RSQ", 5 },
		{ ".RCP64H", 6 },
		{ ".RSQ64H", 7 },
		{ ".INVALIDMUFUOPCODE8", 8 },
		{ ".INVALIDMUFUOPCODE9", 9 },
		{ ".INVALIDMUFUOPCODEA", 10 },
		{ ".INVALIDMUFUOPCODEB", 11 },
		{ ".INVALIDMUFUOPCODEC", 12 },
		{ ".INVALIDMUFUOPCODED", 13 },
		{ ".INVALIDMUFUOPCODEE", 14 },
		{ ".INVALIDMUFUOPCODEF", 15 }
};

misc::StringMap ndv_map =
{
		{ "", 0 },
		{ ".NDV", 1 }
};

misc::StringMap nodep_map =
{
		{ "", 0 },
		{ ".NODEP", 1 }
};

misc::StringMap noinc_map =
{
		{ ".NOINC", 0 },
		{ "", 1 }
};

misc::StringMap op_map =
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
		{ ".INVALID9", 9 },
		{ ".INVALID0A", 10 },
		{ ".INVALID0B", 11 },
		{ ".INVALID0C", 12 },
		{ ".INVALID0D", 13 },
		{ ".INVALID0E", 14 },
		{ ".INVALID0F", 15 }
};

misc::StringMap phase_map =
{
		{ "", 0 },
		{ ".T", 1 },
		{ ".P", 2 },
		{ ".INVALIDPHASE3", 3 }
};

misc::StringMap pmode_map =
{
		{ "", 0 },
		{ ".F4E", 1 },
		{ ".B4E", 2 },
		{ ".RC8", 3 },
		{ ".ECL", 4 },
		{ ".ECR", 5 },
		{ ".EC16", 6 },
		{ ".INVALIDPMODE7", 7 },
		{ ".???8", 8 },
		{ ".???9", 9 },
		{ ".???10", 10 },
		{ ".???11", 11 },
		{ ".???12", 12 },
		{ ".???13", 13 },
		{ ".???14", 14 },
		{ ".???15", 15 }
};

misc::StringMap po_map =
{
		{ "", 0 },
		{ "", 1 },
		{ "", 2 },
		{ ".PO", 3 }
};

misc::StringMap r2_map =
{
		{ ".R", 0 },
		{ ".G", 1 },
		{ ".B", 2 },
		{ ".A", 3 }
};

misc::StringMap r4_map =
{
		{ "", 0 },
		{ ".R", 1 },
		{ ".G", 2 },
		{ ".RG", 3 },
		{ ".B", 4 },
		{ ".RB", 5 },
		{ ".GB", 6 },
		{ ".RGB", 7 },
		{ ".A", 8 },
		{ ".RA", 9 },
		{ ".GA", 10 },
		{ ".RGA", 11 },
		{ ".BA", 12 },
		{ ".RBA", 13 },
		{ ".GBA", 14 },
		{ "", 15 }
};

misc::StringMap redop_map =
{
		{ ".ADD", 0 },
		{ ".MIN", 1 },
		{ ".MAX", 2 },
		{ ".INC", 3 },
		{ ".DEC", 4 },
		{ ".AND", 5 },
		{ ".OR", 6 },
		{ ".XOR", 7 },
		{ ".INVALIDREDOP8", 8 },
		{ ".INVALIDREDOP9", 9 },
		{ ".INVALIDREDOP10", 10 },
		{ ".INVALIDREDOP11", 11 },
		{ ".INVALIDREDOP12", 12 },
		{ ".INVALIDREDOP13", 13 },
		{ ".INVALIDREDOP14", 14 },
		{ ".INVALIDREDOP15", 15 }
};

misc::StringMap rf2_map =
{
		{ "", 0 },
		{ ".INVALIDRF1", 1 },
		{ ".INVALIDRF2", 2 },
		{ ".INVALIDRF3", 3 }
};

misc::StringMap rf3_map =
{
		{ "", 0 },
		{ ".INVALIDRF1", 1 },
		{ ".INVALIDRF2", 2 },
		{ ".INVALIDRF3", 3 },
		{ ".32", 4 },
		{ ".64", 5 },
		{ ".128", 6 },
		{ ".INVALIDRF7", 7 }
};

misc::StringMap rnd_map =
{
		{ "", 0 },
		{ ".RM", 1 },
		{ ".RP", 2 },
		{ ".RZ", 3 }
};

misc::StringMap rroop_map =
{
		{ ".SINCOS", 0 },
		{ ".EX2", 1 }
};

misc::StringMap s_map =
{
		{ "", 0 },
		{ ".S", 1 }
};

misc::StringMap sat_map =
{
		{ "", 0 },
		{ ".SAT", 1 }
};

misc::StringMap scale3_map =
{
		{ "", 0 },
		{ ".D2", 1 },
		{ ".D4", 2 },
		{ ".D8", 3 },
		{ ".M8", 4 },
		{ ".M4", 5 },
		{ ".M2", 6 },
		{ ".INVALIDSCALE37", 7 }
};

misc::StringMap sh_map =
{
		{ "", 0 },
		{ ".SH", 1 }
};

misc::StringMap shrxmode_map =
{
		{ "", 0 },
		{ ".INVALIDSHRXMODE1", 1 },
		{ ".X", 2 },
		{ ".XHI", 3 }
};

misc::StringMap size3_map =
{
		{ ".U8", 0 },
		{ ".S8", 1 },
		{ ".U16", 2 },
		{ ".S16", 3 },
		{ "", 4 },
		{ ".64", 5 },
		{ ".128", 6 }
};

misc::StringMap size5_map =
{
		{ ".U8.U8", 0 },
		{ ".S8.S8", 1 },
		{ ".U8.U16", 2 },
		{ ".U8.S16", 3 },
		{ ".U8.32", 4 },
		{ ".S8.U8", 5 },
		{ ".S8.S8", 6 },
		{ ".S8.U16", 7 },
		{ ".S8.S16", 8 },
		{ ".S8.32", 9 },
		{ ".U16.U8", 10 },
		{ ".U16.S8", 11 },
		{ ".U16.U16", 12 },
		{ ".U16.S16", 13 },
		{ ".U16.32", 14 },
		{ ".S16.U8", 15 },
		{ ".S16.S8", 16 },
		{ ".S16.U16", 17 },
		{ ".S16.U16", 18 },
		{ ".S16.32", 19 },
		{ ".32.U8", 20 },
		{ ".32.S8", 21 },
		{ ".32.U16", 22 },
		{ ".32.S16", 23 },
		{ "", 24 },
		{ ".32.64", 25 },
		{ ".32.128", 26 },
		{ ".64.32", 27 },
		{ ".64.64", 28 },
		{ ".64.128", 29 },
		{ ".128.32", 30 },
		{ ".128.64", 31 },
};

misc::StringMap stcachectrl_map =
{
		{ "", 0 },
		{ ".CG", 1 },
		{ ".CS", 2 },
		{ ".WT", 3 }
};

misc::StringMap sucachectrl_map =
{
		{ ".WB", 0 },
		{ "", 1 },
		{ ".CS", 2 },
		{ ".WT", 3 }
};

misc::StringMap suqop_map =
{
		{ ".RANK", 0 },
		{ ".PIXFMT", 1 },
		{ ".SMPLSZ", 2 },
		{ ".DIM", 3 },
		{ ".RGBA", 4 },
		{ ".BLKSZ", 5 },
		{ ".INVALIDSUQOP6", 6 },
		{ ".INVALIDSUQOP7", 7 },
		{ ".INVALIDSUQOP8", 8 },
		{ ".INVALIDSUQOP9", 9 },
		{ ".INVALIDSUQOP10", 10 },
		{ ".INVALIDSUQOP11", 11 },
		{ ".INVALIDSUQOP12", 12 },
		{ ".INVALIDSUQOP13", 13 },
		{ ".INVALIDSUQOP14", 14 },
		{ ".INVALIDSUQOP15", 15 },
		{ ".INVALIDSUQOP16", 16 },
		{ ".INVALIDSUQOP17", 17 },
		{ ".INVALIDSUQOP18", 18 },
		{ ".INVALIDSUQOP19", 19 },
		{ ".INVALIDSUQOP20", 20 },
		{ ".INVALIDSUQOP21", 21 },
		{ ".INVALIDSUQOP22", 22 },
		{ ".INVALIDSUQOP23", 23 },
		{ ".INVALIDSUQOP24", 24 },
		{ ".INVALIDSUQOP25", 25 },
		{ ".INVALIDSUQOP26", 26 },
		{ ".INVALIDSUQOP27", 27 },
		{ ".INVALIDSUQOP28", 28 },
		{ ".INVALIDSUQOP29", 29 },
		{ ".INVALIDSUQOP30", 30 },
		{ ".INVALIDSUQOP31", 31 },
};

misc::StringMap suredop_map =
{
		{ ".ADD", 0 },
		{ ".MIN", 1 },
		{ ".MAX", 2 },
		{ ".INC", 3 },
		{ ".DEC", 4 },
		{ ".AND", 5 },
		{ ".OR", 6 },
		{ ".XOR", 7 },
		{ ".INVALIDSUREDOP8", 8 },
		{ ".INVALIDSUREDOP9", 9 },
		{ ".INVALIDSUREDOP10", 10 },
		{ ".INVALIDSUREDOP11", 11 },
		{ ".INVALIDSUREDOP12", 12 },
		{ ".INVALIDSUREDOP13", 13 },
		{ ".INVALIDSUREDOP14", 14 },
		{ ".INVALIDSUREDOP15", 15 }
};

misc::StringMap surfaceclamp_map =
{
		{ "", 1 },
		{ ".TRAP", 2 },
		{ ".INVALIDSURFACECLAMP3", 3 }
};

misc::StringMap swizzlemode_map =
{
		{ ".0000", 0 },
		{ ".1111", 1 },
		{ ".2222", 2 },
		{ ".3333", 3 },
		{ ".1032", 4 },
		{ ".2301", 5 },
		{ ".INVALIDSWIZZLEMODE6", 6 },
		{ ".INVALIDSWIZZLEMODE7", 7 }
};

misc::StringMap trig_map =
{
		{ "", 0 },
		{ ".TRIG", 1 }
};

misc::StringMap type_map =
{
		{ ".U32.U32", 0 },
		{ ".U32.S32", 1 },
		{ ".S32.U32", 2 },
		{ "", 3 }
};

misc::StringMap u_map =
{
		{ "", 0 },
		{ ".U", 1 },
		{ ".C", 2 },
		{ ".I", 3 },
};

misc::StringMap u32_map =
{
		{ ".U32", 0 },
		{ "", 1 }
};

misc::StringMap vmode_map =
{
		{ ".ALL", 0 },
		{ ".ANY", 1 },
		{ ".EQ", 2 },
		{ ".INVALIDVMODE3", 3 },
		{ ".INVALIDVMODE4", 4 },
		{ ".VTG.R", 5 },
		{ ".VTG.A", 6 },
		{ ".VTG.RA", 7 }
};

misc::StringMap w_map =
{
		{ "", 0 },
		{ ".W", 1 }
};

misc::StringMap x_map =
{
		{ "", 0 },
		{ ".X", 1 }
};

misc::StringMap xlo_map =
{
		{ "", 0 },
		{ ".XLO", 1 },
		{ ".XMED", 2 },
		{ ".XHI", 3 }
};

misc::StringMap xlu_map =
{
		{ "", 0 },
		{ ".XLU", 1 },
		{ ".ALU", 2 },
		{ ".INVALID", 3 }
};

// operand modifiers
misc::StringMap cc_map =
{
		{ "", 0 },
		{ ".CC", 1 }
};

misc::StringMap cccop_map =
{
		{ "CC.F", 0 },
		{ "CC.LT", 1 },
		{ "CC.EQ", 2 },
		{ "CC.LE", 3 },
		{ "CC.GT", 4 },
		{ "CC.NE", 5 },
		{ "CC.GE", 6 },
		{ "CC.NUM", 7 },
		{ "CC.NAN", 8 },
		{ "CC.LTU", 9 },
		{ "CC.EQU", 10 },
		{ "CC.LEU", 11 },
		{ "CC.GTU", 12 },
		{ "CC.NEU", 13 },
		{ "CC.GEU", 14 },
		{ "", 15 },
		{ "CC.OFF", 16 },
		{ "CC.LO", 17 },
		{ "CC.SFF", 18 },
		{ "CC.LS", 19 },
		{ "CC.HI", 20 },
		{ "CC.SFT", 21 },
		{ "CC.HS", 22 },
		{ "CC.OFT", 23 },
		{ "CC.CSM_TA", 24 },
		{ "CC.CSM_TR", 25 },
		{ "CC.CSM_MX", 26 },
		{ "CC.FCSM_TA", 27 },
		{ "CC.FCSM_TR", 28 },
		{ "CC.FCSM_MX", 29 },
		{ "CC.RLE", 30 },
		{ "CC.RGT", 31 }
};

enum
{
	SR_LANEID = 0,
	SR_CLOCK = 1,
	SR_VIRTCFG = 2,
	SR_VIRTID = 3,
	SR_PM0 = 4,
	SR_PM1 = 5,
	SR_PM2 = 6,
	SR_PM3 = 7,
	SR_PM4 = 8,
	SR_PM5 = 9,
	SR_PM6 = 10,
	SR_PM7 = 11,
	SR_PRIM_TYPE = 16,
	SR_INVOCATION_ID = 17,
	SR_Y_DIRECTION = 18,
	SR_THREAD_KILL = 19,
	SR_SHADER_TYPE = 20,
	SR_MACHINE_ID_0 = 24,
	SR_MACHINE_ID_1 = 25,
	SR_MACHINE_ID_2 = 26,
	SR_MACHINE_ID_3 = 27,
	SR_AFFINITY = 28,
	SR_TID = 32,
	SR_TID_X = 33,
	SR_TID_Y = 34,
	SR_TID_Z = 35,
	SR_CTA_PARAM = 36,
	SR_CTAID_X = 37,
	SR_CTAID_Y = 38,
	SR_CTAID_Z = 39,
	SR_NTID = 40,
	SR_NTID_X = 41,
	SR_NTID_Y = 42,
	SR_NTID_Z = 43,
	SR_GRIDPARAM = 44,
	SR_NCTAID_X = 45,
	SR_NCTAID_Y = 46,
	SR_NCTAID_Z = 47,
	SR_SWINLO = 48,
	SR_SWINSZ = 49,
	SR_SMEMSZ = 50,
	SR_SMEMBANKS = 51,
	SR_LWINLO = 52,
	SR_LWINSZ = 53,
	SR_LMEMLOSZ = 54,
	SR_LMEMHIOFF = 55,
	SR_EQMASK = 56,
	SR_LTMASK = 57,
	SR_LEMASK = 58,
	SR_GTMASK = 59,
	SR_GEMASK = 60,
	SR_GLOBALERRORSTATUS = 64,
	SR_WARPERRORSTATUS = 66,
	SR_WARPERRORSTATUSCLEAR = 67,
	SR_CLOCKLO = 80,
	SR_CLOCKHI = 81
};

misc::StringMap ssrc_map =
{
		{ "SR_LANEID", SR_LANEID },
		{ "SR_CLOCK", SR_CLOCK },
		{ "SR_VIRTCFG", SR_VIRTCFG },
		{ "SR_VIRTID", SR_VIRTID },
		{ "SR_PM0", SR_PM0 },
		{ "SR_PM1", SR_PM1 },
		{ "SR_PM2", SR_PM2 },
		{ "SR_PM3", SR_PM3 },
		{ "SR_PM4", SR_PM4 },
		{ "SR_PM5", SR_PM5 },
		{ "SR_PM6", SR_PM6 },
		{ "SR_PM7", SR_PM7 },
		{ "SR12", 12 },
		{ "SR13", 13 },
		{ "SR14", 14 },
		{ "SR15", 15 },
		{ "SR_PRIM_TYPE", SR_PRIM_TYPE },
		{ "SR_INVOCATION_ID", SR_INVOCATION_ID },
		{ "SR_Y_DIRECTION", SR_Y_DIRECTION },
		{ "SR_THREAD_KILL", SR_THREAD_KILL },
		{ "SR_SHADER_TYPE", SR_SHADER_TYPE },
		{ "SR21", 21 },
		{ "SR22", 22 },
		{ "SR23", 23 },
		{ "SR_MACHINE_ID_0", SR_MACHINE_ID_0 },
		{ "SR_MACHINE_ID_1", SR_MACHINE_ID_1 },
		{ "SR_MACHINE_ID_2", SR_MACHINE_ID_2 },
		{ "SR_MACHINE_ID_3", SR_MACHINE_ID_3 },
		{ "SR_AFFINITY", SR_AFFINITY },
		{ "SR29", 29 },
		{ "SR30", 30 },
		{ "SR31", 31 },
		{ "SR_TID", SR_TID },
		{ "SR_TID.X", SR_TID_X },
		{ "SR_TID.Y", SR_TID_Y },
		{ "SR_TID.Z", SR_TID_Z },
		{ "SR_CTA_PARAM", SR_CTA_PARAM },
		{ "SR_CTAID.X", SR_CTAID_X },
		{ "SR_CTAID.Y", SR_CTAID_Y },
		{ "SR_CTAID.Z", SR_CTAID_Z },
		{ "SR_NTID", SR_NTID },
		{ "SR_NTID.X", SR_NTID_X },
		{ "SR_NTID.Y", SR_NTID_Y },
		{ "SR_NTID.Z", SR_NTID_Z },
		{ "SR_GRIDPARAM", SR_GRIDPARAM },
		{ "SR_NCTAID.X", SR_NCTAID_X },
		{ "SR_NCTAID.Y", SR_NCTAID_Y },
		{ "SR_NCTAID.Z", SR_NCTAID_Z },
		{ "SR_SWINLO", SR_SWINLO },
		{ "SR_SWINSZ", SR_SWINSZ },
		{ "SR_SMEMSZ", SR_SMEMSZ },
		{ "SR_SMEMBANKS", SR_SMEMBANKS },
		{ "SR_LWINLO", SR_LWINLO },
		{ "SR_LWINSZ", SR_LWINSZ },
		{ "SR_LMEMLOSZ", SR_LMEMLOSZ },
		{ "SR_LMEMHIOFF", SR_LMEMHIOFF },
		{ "SR_EQMASK", SR_EQMASK },
		{ "SR_LTMASK", SR_LTMASK },
		{ "SR_LEMASK", SR_LEMASK },
		{ "SR_GTMASK", SR_GTMASK },
		{ "SR_GEMASK", SR_GEMASK },
		{ "SR61", 61 },
		{ "SR62", 62 },
		{ "SR63", 63 },
		{ "SR_GLOBALERRORSTATUS", SR_GLOBALERRORSTATUS },
		{ "SR65", 65 },
		{ "SR_WARPERRORSTATUS", SR_WARPERRORSTATUS },
		{ "SR_WARPERRORSTATUSCLEAR", SR_WARPERRORSTATUSCLEAR },
		{ "SR68", 68 },
		{ "SR69", 69 },
		{ "SR70", 70 },
		{ "SR71", 71 },
		{ "SR72", 72 },
		{ "SR73", 73 },
		{ "SR74", 74 },
		{ "SR75", 75 },
		{ "SR76", 76 },
		{ "SR77", 77 },
		{ "SR78", 78 },
		{ "SR79", 79 },
		{ "SR_CLOCKLO", SR_CLOCKLO },
		{ "SR_CLOCKHI", SR_CLOCKHI }
};


Inst::Inst()
{
	this->as = Asm::getInstance();
}


void Inst::DumpPC(std::ostream &os)
{
	os << misc::fmt("/*%04x*/", addr);
}


void Inst::DumpToBufWithFmtReg(void)
{
	FmtReg fmt;
	const char *fmt_str;
	int len;

	// Get instruction format/encoding
	fmt = bytes.fmt_reg;

	// Get format string
	fmt_str = info->fmt_str.c_str();

	// Process format string
	while (*fmt_str)
	{
		std::stringstream ss;

		// Character is a literal symbol
		if (*fmt_str != '%')
		{
			if (fmt_str == info->fmt_str.c_str())
				str += "        ";
			str += *fmt_str;
			++fmt_str;
			continue;
		}

		// Character is a token
		++fmt_str;

		if (comm::Asm::isToken(fmt_str, "pred", len))
		{
			unsigned pred;
			pred = fmt.pred;
			if (pred < 7)
				ss << "    @P" << pred;
			else if (pred > 7)
				ss << "   @!P" << pred - 8;
			else
				ss << "       ";
		}
		else if (comm::Asm::isToken(fmt_str, "addsub", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 4) & 0x3;
			ss << addsub_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "bf", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x1;
			ss << bf_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "bop", len))
		{
			unsigned v;
			if (info->op == InstOpLOP)
				v = (fmt.fmod0 >> 2) & 0x3;
			else
				v = (fmt.fmod1_srco >> 4) & 0x3;
			ss << bop_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "brev", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 4) & 0x1;
			ss << brev_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "cop3", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 6) & 0x7;
			ss << cop3_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "cop4", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 6) & 0xf;
			if (v != 4)
				ss << cop4_map.MapValue(v);
			else
			{
				if ((info->op == InstOpFSETP) || (info->op == InstOpDSETP))
					ss << ".GT";
			}
		}
		else if (comm::Asm::isToken(fmt_str, "fma", len))
		{
			unsigned v;
			if (info->op == InstOpFADD || info->op == InstOpFFMA)
				v = (fmt.fmod1_srco >> 8) & 0x3;
			else
				v = (fmt.fmod0 >> 4) & 0x3;
			ss << fma_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "fmz", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 2) & 0x3;
			ss << fmz_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "ftz", len))
		{
			unsigned v;
			if (info->op == InstOpFSETP)
				v = fmt.func & 0x1;
			else
				v = (fmt.fmod0 >> 1) & 0x1;
			ss << ftz_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "hi", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 2) & 0x1;
			ss << hi_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "mufuopcode", len))
		{
			unsigned v;
			v = fmt.src2 & 0xf;
			ss << mufuopcode_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "ndv", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 5) & 0x1;
			ss << ndv_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "po", len))
		{
			unsigned v;
			if (info->op == InstOpISCADD)
				v = (fmt.fmod1_srco >> 6) & 0x3;
			else
				v = (fmt.fmod0 >> 4) & 0x3;
			ss << po_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "rnd", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 6) & 0x3;
			ss << rnd_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "rroop", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x1;
			ss << rroop_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "sat", len))
		{
			unsigned v;
			if (info->op == InstOpFADD)
				v = fmt.fmod1_srco & 0x1;
			else if (info->op == InstOpIMAD)
				v = (fmt.fmod1_srco >> 7) & 0x1;
			else
				v = (fmt.fmod0 >> 1) & 0x1;
			ss << sat_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "scale3", len))
		{
			unsigned v;
			v = fmt.fmod1_srco & 0x7;
			ss << scale3_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "shrxmode", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 2) & 0x3;
			ss << shrxmode_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "sh", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 2) & 0x1;
			ss << sh_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "swizzlemode", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 2) & 0x7;
			ss << swizzlemode_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "s", len))
		{
			unsigned v;
			v = fmt.fmod0 & 0x1;
			ss << s_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "type", len))
		{
			unsigned v;
			v = (((fmt.fmod0 >> 3) & 0x1) << 1) | ((fmt.fmod0 >> 1) & 0x1);
			ss << type_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "u32", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x1;
			ss << u32_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "w", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 5) & 0x1;
			ss << w_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "xlo", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 2) & 0x3;
			ss << xlo_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "xlu", len))
		{
			unsigned v;
			if (info->op == InstOpISETP)
				v = (fmt.fmod0 >> 4) & 0x3;
			else
				v = (fmt.fmod1_srco >> 6) & 0x3;
			ss << xlu_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "x", len))
		{
			unsigned v;
			if (info->op == InstOpLOP)
				v = (fmt.fmod0 >> 1) & 0x1;
			else if (info->op == InstOpIMAD)
				v = (fmt.fmod1_srco >> 6) & 0x1;
			else
				v = (fmt.fmod0 >> 2) & 0x1;
			ss << x_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "dst", len))
		{
			unsigned dst;
			dst = fmt.dst;
			ss << "R";
			if (dst != 63)
				ss << dst;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "cc", len))
		{
			unsigned v;
			v = fmt.dmod & 0x1;
			ss << cc_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "pdst1", len))
		{
			unsigned p;
			p = (fmt.dst >> 3) & 0x7;
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else if (comm::Asm::isToken(fmt_str, "pdst2", len))
		{
			unsigned p;
			p = fmt.dst & 0x7;
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else if (comm::Asm::isToken(fmt_str, "src1src2", len))
		{
			unsigned src1;
			union {unsigned i; float f;} src2;
			unsigned s2mode;
			unsigned mode;

			src1 = fmt.src1;
			src2.i = fmt.src2;
			s2mode = fmt.s2mod;
			mode = (fmt.fmod0 >> 4) & 0x3;

			if (mode == 2)
			{
				if (s2mode == 0)
				{
					ss << "R";
					if (src2.i != 63)
						ss << src2.i;
					else
						ss << "Z";
				}
				else if (s2mode == 1 || s2mode == 2)
				{
					unsigned bank = ((src2.i & 0x1) << 4) | (src2.i >> 16);
					int offset = src2.i & 0xfffe;
					ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
				}
				else if (s2mode == 3)
				{
					InstCategory cat = info->category;
					if (cat == InstCategorySpFp || cat == InstCategoryDpFp)
					{
						src2.i = fmt.src2 << 12;
						if (std::abs(src2.f) < 1e-4 || std::abs(src2.f) > 1e9)
							ss << misc::fmt("%.20e", src2.f);
						else
							ss << misc::fmt("%.20g", src2.f);
					}
					else if (cat == InstCategoryInt)
					{
						src2.i = fmt.src2;
						if (src2.i >> 19 == 0)  // positive value
							ss << misc::fmt("0x%x", src2.i);
						else  // negative value
							ss << misc::fmt("-0x%x", 0x100000 - src2.i);
					}
				}
				ss << ", R";
				if (src1 != 63)
					ss << src1;
				else
					ss << "Z";
			}
			else
			{
				ss << "R";
				if (src1 != 63)
					ss << src1;
				else
					ss << "Z";
				ss << ", ";
				if (s2mode == 0)
				{
					ss << "R";
					if (src2.i != 63)
						ss << src2.i;
					else
						ss << "Z";
				}
				else if (s2mode == 1 || s2mode == 2)
				{
					unsigned bank = ((src2.i & 0x1) << 4) | (src2.i >> 16);
					int offset = src2.i & 0xfffe;
					ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
				}
				else if (s2mode == 3)
				{
					InstCategory cat = info->category;
					if (cat == InstCategorySpFp || cat == InstCategoryDpFp)
					{
						src2.i = fmt.src2 << 12;
						if (std::abs(src2.f) < 1e-4 || std::abs(src2.f) > 1e9)
							ss << misc::fmt("%.20e", src2.f);
						else
							ss << misc::fmt("%.20g", src2.f);
					}
					else if (cat == InstCategoryInt)
					{
						src2.i = fmt.src2;
						if (src2.i >> 19 == 0)  // positive value
							ss << misc::fmt("0x%x", src2.i);
						else  // negative value
							ss << misc::fmt("-0x%x", 0x100000 - src2.i);
					}
				}
			}
		}
		else if (comm::Asm::isToken(fmt_str, "src1", len))
		{
			unsigned src;
			src = fmt.src1;
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "isrc1", len))
		{
			unsigned src;
			unsigned i;
			src = fmt.src1;
			i = (fmt.fmod0 >> 5) & 0x1;
			if (i == 1)
				ss << "~";
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "n1src1", len))
		{
			unsigned src;
			unsigned n;
			src = fmt.src1;
			if (info->op == InstOpFMUL)
				n = (fmt.fmod1_srco >> 8) & 0x1;
			else
				n = (fmt.fmod0 >> 5) & 0x1;
			if (n == 1)
				ss << "-";
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "n2src1", len))
		{
			unsigned src;
			unsigned n;
			src = fmt.src1;
			if (info->op == InstOpIMAD)
				n = (fmt.fmod0 >> 4) & 0x3;
			else
				n = (fmt.fmod1_srco >> 6) & 0x3;
			if (n == 2)
				ss << "-";
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "nasrc1", len))
		{
			unsigned src;
			unsigned n;
			unsigned a;
			src = fmt.src1;
			n = (fmt.fmod0 >> 5) & 0x1;
			a = (fmt.fmod0 >> 3) & 0x1;
			if (n == 1)
				ss << "-";
			if (a == 1)
				ss << "|";
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
			if (a == 1)
				ss << "|";
		}
		else if (comm::Asm::isToken(fmt_str, "src2n1src3", len))
		{
			union {unsigned i; float f;} src2;
			unsigned mode;
			unsigned src3;
			unsigned n;

			src2.i = fmt.src2;
			mode = fmt.s2mod;
			src3 = fmt.fmod1_srco & 0x3f;
			n = (fmt.fmod0 >> 4) & 0x1;

			if (mode == 0)
			{
				ss << "R";
				if (src2.i != 63)
					ss << src2.i;
				else
					ss << "Z";
				ss << ", ";
				if (n == 1)
					ss << "-";
				ss << "R";
				if (src3 != 63)
					ss << src3;
				else
					ss << "Z";
			}
			else if (mode == 1)
			{
				unsigned bank = ((src2.i & 0x1) << 4) | (src2.i >> 16);
				int offset = src2.i & 0xfffe;
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
				ss << ", ";
				ss << std::dec;
				if (n == 1)
					ss << "-";
				ss << "R";
				if (src3 != 63)
					ss << src3;
				else
					ss << "Z";
			}
			else if (mode == 2)
			{
				unsigned bank = ((src2.i & 0x1) << 4) | (src2.i >> 16);
				int offset = src2.i & 0xfffe;
				if (n == 1)
					ss << "-";
				ss << "R";
				if (src3 != 63)
					ss << src3;
				else
					ss << "Z";
				ss << ", ";
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
			}
			else if (mode == 3)
			{
				InstCategory cat = info->category;
				if (cat == InstCategorySpFp || cat == InstCategoryDpFp)
				{
					src2.i = fmt.src2 << 12;
					if (std::abs(src2.f) < 1e-4)
						ss << misc::fmt("%.19e", src2.f);
					else if (std::abs(src2.f) > 1e9)
						ss << misc::fmt("%.20e", src2.f);
					else
						ss << misc::fmt("%.20g", src2.f);
				}
				else if (cat == InstCategoryInt)
				{
					src2.i = fmt.src2;
					if (src2.i >> 19 == 0)  // positive value
						ss << misc::fmt("0x%x", src2.i);
					else  // negative value
						ss << misc::fmt("-0x%x", 0x100000 - src2.i);
				}
				ss << ", ";
				if (n == 1)
					ss << "-";
				ss << "R";
				if (src3 != 63)
					ss << src3;
				else
					ss << "Z";
			}
		}
		else if (comm::Asm::isToken(fmt_str, "src2n2src3", len))
		{
			union {unsigned i; float f;} src2;
			unsigned mode;
			unsigned src3;
			unsigned n;

			src2.i = fmt.src2;
			mode = fmt.s2mod;
			src3 = fmt.fmod1_srco & 0x3f;
			n = (fmt.fmod0 >> 4) & 0x3;

			if (mode == 0)
			{
				ss << "R";
				if (src2.i != 63)
					ss << src2.i;
				else
					ss << "Z";
				ss << ", ";
				if (n == 1)
					ss << "-";
				ss << "R";
				if (src3 != 63)
					ss << src3;
				else
					ss << "Z";
			}
			else if (mode == 1)
			{
				unsigned bank = ((src2.i & 0x1) << 4) | (src2.i >> 16);
				int offset = src2.i & 0xfffe;
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
				ss << ", ";
				ss << std::dec;
				if (n == 1)
					ss << "-";
				ss << "R";
				if (src3 != 63)
					ss << src3;
				else
					ss << "Z";
			}
			else if (mode == 2)
			{
				unsigned bank = ((src2.i & 0x1) << 4) | (src2.i >> 16);
				int offset = src2.i & 0xfffe;
				if (n == 1)
					ss << "-";
				ss << "R";
				if (src3 != 63)
					ss << src3;
				else
					ss << "Z";
				ss << ", ";
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
			}
			else if (mode == 3)
			{
				InstCategory cat = info->category;
				if (cat == InstCategorySpFp || cat == InstCategoryDpFp)
				{
					src2.i = fmt.src2 << 12;
					if (std::abs(src2.f) < 1e-4 || std::abs(src2.f) > 1e9)
						ss << misc::fmt("%.20e", src2.f);
					else
						ss << misc::fmt("%.20g", src2.f);
				}
				else if (cat == InstCategoryInt)
				{
					src2.i = fmt.src2;
					if (src2.i >> 19 == 0)  // positive value
						ss << misc::fmt("0x%x", src2.i);
					else  // negative value
						ss << misc::fmt("-0x%x", 0x100000 - src2.i);
				}
				ss << ", ";
				if (n == 1)
					ss << "-";
				ss << "R";
				if (src3 != 63)
					ss << src3;
				else
					ss << "Z";
			}
		}
		else if (comm::Asm::isToken(fmt_str, "src2", len))
		{
			union {unsigned i; float f; unsigned long long l; double d;} src;
			unsigned mode;

			src.i = fmt.src2;
			mode = fmt.s2mod;

			if (mode == 0)
			{
				ss << "R";
				if (src.i != 63)
					ss << src.i;
				else
					ss << "Z";
			}
			else if (mode == 1 || mode == 2)
			{
				unsigned bank = ((src.i & 0x1) << 4) | (src.i >> 16);
				int offset = src.i & 0xfffe;
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
			}
			else if (mode == 3)
			{
				InstCategory cat = info->category;
				if (cat == InstCategorySpFp)
				{
					src.i = fmt.src2 << 12;
					if ((std::abs(src.f) < 1e-4) || (std::abs(src.f) > 1e9))
					{
						if (info->op == InstOpFMUL)
							ss << misc::fmt("%.19e", src.f);
						else
							ss << misc::fmt("%.20e", src.f);
					}
					else
						ss << misc::fmt("%.20g", src.f);
				}
				else if (cat == InstCategoryDpFp)
				{
					src.l = (unsigned long long)fmt.src2 << 44;
					if (std::abs(src.d) < 1e-4)
						ss << misc::fmt("%.11e", src.d);
					else if (std::abs(src.d) > 1e9)
						ss << misc::fmt("%.20e", src.d);
					else
						ss << misc::fmt("%.11g", src.d);
				}
				else if (cat == InstCategoryInt)
				{
					src.i = fmt.src2;
					if (src.i >> 19 == 0)  // positive value
						ss << misc::fmt("0x%x", src.i);
					else  // negative value
						ss << misc::fmt("-0x%x", 0x100000 - src.i);
				}
			}
		}
		else if (comm::Asm::isToken(fmt_str, "isrc2", len))
		{
			union {unsigned i; float f;} src;
			unsigned mode;
			unsigned i;

			src.i = fmt.src2;
			mode = fmt.s2mod;
			i = (fmt.fmod0 >> 4) & 0x1;

			if (mode == 0)
			{
				if (i == 1)
					ss << "~";
				ss << "R";
				if (src.i != 63)
					ss << src.i;
				else
					ss << "Z";
			}
			else if (mode == 1 || mode == 2)
			{
				unsigned bank = ((src.i & 0x1) << 4) | (src.i >> 16);
				int offset = src.i & 0xfffe;
				if (i == 1)
					ss << "~";
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
			}
			else if (mode == 3)
			{
				InstCategory cat = info->category;
				if (cat == InstCategorySpFp || cat == InstCategoryDpFp)
				{
					src.i = fmt.src2 << 12;
					if (std::abs(src.f) < 1e-4 || std::abs(src.f) > 1e9)
						ss << misc::fmt("%.20e", src.f);
					else
						ss << misc::fmt("%.20g", src.f);
				}
				else if (cat == InstCategoryInt)
				{
					src.i = fmt.src2;
					if (src.i >> 19 == 0)  // positive value
						ss << misc::fmt("0x%x", src.i);
					else  // negative value
						ss << misc::fmt("-0x%x", 0x100000 - src.i);
				}
				if (i == 1)
					ss << ".INV";
			}
		}
		else if (comm::Asm::isToken(fmt_str, "n2src2", len))
		{
			union {unsigned i; float f;} src;
			unsigned mode;
			unsigned n;

			src.i = fmt.src2;
			mode = fmt.s2mod;
			n = (fmt.fmod1_srco >> 6) & 0x3;

			if (mode == 0)
			{
				if (n == 1)
					ss << "-";
				ss << "R";
				if (src.i != 63)
					ss << src.i;
				else
					ss << "Z";
			}
			else if (mode == 1 || mode == 2)
			{
				unsigned bank = ((src.i & 0x1) << 4) | (src.i >> 16);
				int offset = src.i & 0xfffe;
				if (n == 1)
					ss << "-";
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
			}
			else if (mode == 3)
			{
				InstCategory cat = info->category;
				if (cat == InstCategorySpFp || cat == InstCategoryDpFp)
				{
					src.i = fmt.src2 << 12;
					if (std::abs(src.f) < 1e-4 || std::abs(src.f) > 1e9)
						ss << misc::fmt("%.20e", src.f);
					else
						ss << misc::fmt("%.20g", src.f);
				}
				else if (cat == InstCategoryInt)
				{
					src.i = fmt.src2;
					if (src.i >> 19 == 0)  // positive value
						ss << misc::fmt("0x%x", src.i);
					else  // negative value
						ss << misc::fmt("-0x%x", 0x100000 - src.i);
				}
				if (n == 1)
					ss << ".NEG";
			}
		}
		else if (comm::Asm::isToken(fmt_str, "nasrc2", len))
		{
			union {unsigned i; float f; unsigned long long l; double d;} src;
			unsigned mode;
			unsigned n, a;

			src.i = fmt.src2;
			mode = fmt.s2mod;
			n = (fmt.fmod0 >> 4) & 0x1;
			a = (fmt.fmod0 >> 2) & 0x1;

			if (mode == 0)
			{
				if (n == 1)
					ss << "-";
				if (a == 1)
					ss << "|";
				ss << "R";
				if (src.i != 63)
					ss << src.i;
				else
					ss << "Z";
				if (a == 1)
					ss << "|";
			}
			else if (mode == 1 || mode == 2)
			{
				unsigned bank = ((src.i & 0x1) << 4) | (src.i >> 16);
				int offset = src.i & 0xfffe;
				if (n == 1)
					ss << "-";
				if (a == 1)
					ss << "|";
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
				if (a == 1)
					ss << "|";
			}
			else if (mode == 3)
			{
				InstCategory cat = info->category;
				if (cat == InstCategorySpFp)
				{
					src.i = fmt.src2 << 12;
					if (std::isinf(src.f))
					{
						if (src.f > 0)
							ss << "+";
						else
							ss << "-";
						ss << "INF";
					}
					else
					{
						if (std::abs(src.f) < 1e-4 || std::abs(src.f) > 1e9)
						{
							if (info->op == InstOpFSETP)
								ss << misc::fmt("%.18e", src.f);
							else
								ss << misc::fmt("%.20e", src.f);
						}
						else
							ss << misc::fmt("%.20g", src.f);
					}
				}
				if (cat == InstCategoryDpFp)
				{
					src.l = (unsigned long long)fmt.src2 << 44;
					if (std::isinf(src.d))
					{
						if (src.d > 0)
							ss << "+";
						else
							ss << "-";
						ss << "INF";
					}
					else
					{
						if ((std::abs(src.d) < 1e-4 || std::abs(src.d) > 1e9) && (src.d != 0))
							ss << misc::fmt("%.20e", src.d);
						else
							ss << misc::fmt("%.20g", src.d);
					}
				}
				else if (cat == InstCategoryInt)
				{
					src.i = fmt.src2;
					if (src.i >> 19 == 0)  // positive value
						ss << misc::fmt("0x%x", src.i);
					else  // negative value
						ss << misc::fmt("-0x%x", 0x100000 - src.i);
				}
				if (n == 1)
					ss << ".NEG";
				if (a == 1)
					ss << ".ABS";
			}
		}
		else if (comm::Asm::isToken(fmt_str, "src3", len))
		{
			unsigned src;
			src = fmt.fmod1_srco & 0x3f;
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "n1src3", len))
		{
			unsigned src;
			unsigned n;
			src = fmt.fmod1_srco & 0x3f;
			n = (fmt.fmod0 >> 4) & 0x1;
			if (n == 1)
				ss << "-";
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "n2src3", len))
		{
			unsigned src;
			unsigned n;
			src = fmt.fmod1_srco & 0x3f;
			n = (fmt.fmod0 >> 4) & 0x3;
			if (n == 1)
				ss << "-";
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "psrc3", len))
		{
			unsigned p;
			p = fmt.fmod1_srco & 0x7;
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else if (comm::Asm::isToken(fmt_str, "npsrc3", len))
		{
			unsigned p;
			unsigned n;
			p = fmt.fmod1_srco & 0x7;
			n = (fmt.fmod1_srco >> 3) & 0x1;
			if (n == 1)
				ss << "!";
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else if (comm::Asm::isToken(fmt_str, "shamt", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x1f;
			ss << misc::fmt("0x%x", v);
		}
		else if (comm::Asm::isToken(fmt_str, "pat", len))
		{
			unsigned v;
			v = (fmt.src2 >> 6) & 0x1ff;
			if (v < 256)
			{
				unsigned digit;
				for (int i = 3; i >= 0; --i)
				{
					digit = (v >> (i * 2)) & 0x3;
					if (digit == 0)
						ss << "PP";
					else if (digit == 1)
						ss << "NP";
					else if (digit == 2)
						ss << "PN";
					else if (digit == 3)
						ss << "ZP";
				}
			}
			else
				ss << "   ???" << v;
		}
		else
		{
			throw misc::Panic(misc::fmt("Unknown token: %s",
					fmt_str));
		}

		str += ss.str();

		// Skip processed token
		fmt_str += len;
	}
	str += ";";
}


void Inst::DumpToBufWithFmtImm(void)
{
	FmtImm fmt;
	const char *fmt_str;
	int len;

	// Get instruction format/encoding
	fmt = bytes.fmt_imm;

	// Get format string
	fmt_str = info->fmt_str.c_str();

	// Process format string
	while (*fmt_str)
	{
		std::stringstream ss;

		// Character is a literal symbol
		if (*fmt_str != '%')
		{
			if (fmt_str == info->fmt_str.c_str())
				str += "        ";
			str += *fmt_str;
			++fmt_str;
			continue;
		}

		// Character is a token
		++fmt_str;

		if (comm::Asm::isToken(fmt_str, "pred", len))
		{
			unsigned pred;
			pred = fmt.pred;
			if (pred < 7)
				ss << "    @P" << pred;
			else if (pred > 7)
				ss << "   @!P" << pred - 8;
			else
				ss << "       ";
		}
		else if (comm::Asm::isToken(fmt_str, "bop", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 2) & 0x3;
			ss << bop_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "fma", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 4) & 0x3;
			ss << fma_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "fmz", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 2) & 0x3;
			ss << fmz_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "ftz", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x1;
			ss << ftz_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "hi", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 2) & 0x1;
			ss << hi_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "po", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 4) & 0x3;
			ss << po_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "sat", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x1;
			ss << sat_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "sat", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x1;
			ss << sat_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "s", len))
		{
			unsigned v;
			v = fmt.fmod0 & 0x1;
			ss << s_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "type", len))
		{
			unsigned v;
			v = (((fmt.fmod0 >> 3) & 0x1) << 1) | ((fmt.fmod0 >> 1) & 0x1);
			ss << type_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "x", len))
		{
			unsigned v;
			if (info->op == InstOpIADD32I)
				v = (fmt.fmod0 >> 2) & 0x1;
			else
				v = (fmt.fmod0 >> 1) & 0x1;
			ss << x_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "dst", len))
		{
			unsigned dst;
			dst = fmt.dst;
			ss << "R";
			if (dst != 63)
				ss << dst;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "cc", len))
		{
			unsigned v;
			v = fmt.dmod & 0x1;
			ss << cc_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "n1dst", len))
		{
			unsigned dst;
			unsigned n;
			dst = fmt.dst;
			n = (fmt.fmod0 >> 4) & 0x1;
			if (n == 1)
				ss << "-";
			ss << "R";
			if (dst != 63)
				ss << dst;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "n2dst", len))
		{
			unsigned dst;
			unsigned n;
			dst = fmt.dst;
			n = (fmt.fmod0 >> 4) & 0x3;
			if (n == 1)
				ss << "-";
			ss << "R";
			if (dst != 63)
				ss << dst;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "src1", len))
		{
			unsigned src;
			src = fmt.src1;
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "isrc1", len))
		{
			unsigned src;
			unsigned i;
			src = fmt.src1;
			i = (fmt.fmod0 >> 5) & 0x1;
			if (i == 1)
				ss << "~";
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "n1src1", len))
		{
			unsigned src;
			unsigned n;
			src = fmt.src1;
			n = (fmt.fmod0 >> 5) & 0x1;
			if (n == 1)
				ss << "-";
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "n2src1", len))
		{
			unsigned src;
			unsigned n;
			src = fmt.src1;
			n = (fmt.fmod0 >> 4) & 0x3;
			if (n == 2)
				ss << "-";
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "nasrc1", len))
		{
			unsigned src;
			unsigned n;
			unsigned a;
			src = fmt.src1;
			n = (fmt.fmod0 >> 5) & 0x1;
			a = (fmt.fmod0 >> 3) & 0x1;
			if (n == 1)
				ss << "-";
			if (a == 1)
				ss << "|";
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
			if (a == 1)
				ss << "|";
		}
		else if (comm::Asm::isToken(fmt_str, "imm32imm4", len))
		{
			unsigned imm32;
			unsigned imm4;
			imm32 = fmt.imm32;
			imm4 = (fmt.fmod0 >> 1) & 0xf;
			ss << misc::fmt("0x%x", imm32);
			if (imm4 != 15)
				ss << misc::fmt(", 0x%x", imm4);
		}
		else if (comm::Asm::isToken(fmt_str, "uimm32", len))
		{
			unsigned imm32;
			imm32 = fmt.imm32;
			ss << misc::fmt("0x%x", imm32);
		}
		else if (comm::Asm::isToken(fmt_str, "imm32", len))
		{
			union {unsigned i; float f;} imm32;
			unsigned s;
			imm32.i = fmt.imm32;
			s = imm32.i >> 31;
			if (info->op == InstOpFFMA32I || info->op == InstOpFADD32I || info->op == InstOpFMUL32I)
			{
				if (std::abs(imm32.f) < 1e-4 || std::abs(imm32.f) > 1e9)
					ss << misc::fmt("%.20e", imm32.f);
				else
					ss << misc::fmt("%.20g", imm32.f);
			}
			else
			{
				if (s == 0)
					ss << misc::fmt("0x%x", imm32.i);
				else
					ss << misc::fmt("-0x%x", ~imm32.i + 1);
			}
		}
		else if (comm::Asm::isToken(fmt_str, "neg", len))
		{
			unsigned n;
			n = (fmt.fmod0 >> 4) & 0x1;
			if (n == 1)
				ss << ".NEG";
		}
		else if (comm::Asm::isToken(fmt_str, "abs", len))
		{
			unsigned a;
			a = (fmt.fmod0 >> 2) & 0x1;
			if (a == 1)
				ss << ".ABS";
		}
		else if (comm::Asm::isToken(fmt_str, "inv", len))
		{
			unsigned i;
			i = (fmt.fmod0 >> 4) & 0x1;
			if (i == 1)
				ss << ".INV";
		}
		else if (comm::Asm::isToken(fmt_str, "shamt", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x1f;
			ss << misc::fmt("0x%x", v);
		}
		else
		{
			throw misc::Panic(misc::fmt("Unknown token: %s",
					fmt_str));
		}

		str += ss.str();

		// Skip processed token
		fmt_str += len;
	}
	str += ";";
}


void Inst::DumpToBufWithFmtOther(void)
{
	FmtOther fmt;
	const char *fmt_str;
	int len;

	// Get instruction format/encoding
	fmt = bytes.fmt_other;

	// Get format string
	fmt_str = info->fmt_str.c_str();

	// Process format string
	while (*fmt_str)
	{
		std::stringstream ss;

		// Character is a literal symbol
		if (*fmt_str != '%')
		{
			if (fmt_str == info->fmt_str.c_str())
				str += "        ";
			str += *fmt_str;
			++fmt_str;
			continue;
		}

		// Character is a token
		++fmt_str;

		if (comm::Asm::isToken(fmt_str, "pred", len))
		{
			unsigned pred;
			pred = fmt.pred;
			if (pred < 7)
				ss << "    @P" << pred;
			else if (pred > 7)
				ss << "   @!P" << pred - 8;
			else
				ss << "       ";
		}
		else if (comm::Asm::isToken(fmt_str, "barmode", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x3;
			ss << barmode_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "bf", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x1;
			ss << bf_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "bop1", len))
		{
			unsigned v;
			v = (fmt.src2 >> 4) & 0x3;
			ss << bop_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "bop2", len))
		{
			unsigned v;
			v = (fmt.fmod2_srco >> 4) & 0x3;
			ss << bop_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "cop", len))
		{
			unsigned v;
			v = fmt.fmod1_src1 & 0x1f;
			ss << cop_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "fpdest", len))
		{
			unsigned v;
			v = fmt.fmod1_src1 & 0x3;
			ss << fpdest_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "fpsrc", len))
		{
			unsigned v;
			v = (fmt.fmod1_src1 >> 3) & 0x3;
			ss << fpsrc_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "frnd2", len))
		{
			unsigned v;
			v = fmt.fmod2_srco & 0x3;
			ss << frnd2_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "frnd3", len))
		{
			unsigned v;
			v = (((fmt.fmod0 >> 3) & 0x1) << 2) | (fmt.fmod2_srco & 0x3);
			ss << frnd3_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "ftz", len))
		{
			unsigned v;
			v = (fmt.fmod2_srco >> 6) & 0x1;
			ss << ftz_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "h1", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x1;
			ss << h1_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "idest", len))
		{
			unsigned v;
			v = (((fmt.fmod0 >> 3) & 0x1) << 2) | (fmt.fmod1_src1 & 0x3);
			ss << idest_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "irnd", len))
		{
			unsigned v;
			v = fmt.fmod2_srco & 0x3;
			ss << irnd_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "isrc", len))
		{
			unsigned v;
			v = (((fmt.fmod0 >> 5) & 0x1) << 2) | ((fmt.fmod1_src1 >> 3) & 0x3);
			ss << isrc_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "op", len))
		{
			unsigned v;
			v = (fmt.fmod1_src1 >> 2) & 0xf;
			ss << op_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "pmode", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0xf;
			ss << pmode_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "sat", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x1;
			ss << sat_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "s", len))
		{
			unsigned v;
			v = fmt.fmod0 & 0x1;
			ss << s_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "trig", len))
		{
			unsigned v;
			v = (fmt.fmod2_srco >> 1) & 0x1;
			ss << trig_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "vmode", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x7;
			ss << vmode_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "xlu", len))
		{
			unsigned v;
			if (info->op == InstOpMOV)
				v = (fmt.fmod2_srco >> 6) & 0x3;
			else
				v = (fmt.fmod0 >> 4) & 0x3;
			ss << xlu_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "cccopsrc2", len))
		{
			unsigned cccop;
			unsigned imm16;
			cccop = (fmt.fmod0 >> 1) & 0x1f;
			imm16 = fmt.src2 & 0xffff;
			if (cccop != 15)
				ss << cop_map.MapValue(cccop);
			if (imm16 != 0)
				ss << misc::fmt(", 0x%x", imm16);
		}
		else if (comm::Asm::isToken(fmt_str, "dstpdst", len))
		{
			unsigned dst, p;
			dst = fmt.dst;
			p = (fmt.fmod2_srco >> 4) & 0x7;
			ss << "R";
			if (dst != 63)
				ss << dst;
			else
				ss << "Z";
			if (p != 7)
			{
				ss << ", P";
				if (p != 7)
					ss << p;
				else
					ss << "T";
			}
		}
		else if (comm::Asm::isToken(fmt_str, "dstpsrc4", len))
		{
			unsigned dst, p;
			dst = fmt.dst;
			p = (fmt.fmod2_srco >> 5) & 0x7;
			if (dst != 63)
				ss << "R" << dst << ", ";
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else if (comm::Asm::isToken(fmt_str, "dst", len))
		{
			unsigned dst;
			dst = fmt.dst;
			ss << "R";
			if (dst != 63)
				ss << dst;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "cc", len))
		{
			unsigned v;
			v = fmt.dmod & 0x1;
			ss << cc_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "pdst1", len))
		{
			unsigned p;
			p = (fmt.dst >> 3) & 0x7;
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else if (comm::Asm::isToken(fmt_str, "pdst2", len))
		{
			unsigned p;
			p = fmt.dst & 0x7;
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else if (comm::Asm::isToken(fmt_str, "src1", len))
		{
			unsigned src;
			src = fmt.fmod1_src1;
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "isrc1", len))
		{
			unsigned src;
			unsigned i;
			src = fmt.fmod1_src1;
			i = (fmt.fmod0 >> 5) & 0x1;
			if (i == 1)
				ss << "~";
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "nasrc1", len))
		{
			unsigned src;
			unsigned n;
			unsigned a;
			src = fmt.fmod1_src1;
			n = (fmt.fmod0 >> 5) & 0x1;
			a = (fmt.fmod0 >> 3) & 0x1;
			if (n == 1)
				ss << "-";
			if (a == 1)
				ss << "|";
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
			if (a == 1)
				ss << "|";
		}
		else if (comm::Asm::isToken(fmt_str, "src2imm4", len))
		{
			union {unsigned i; float f;} src;
			unsigned i;
			unsigned mode;

			src.i = fmt.src2;
			i = (fmt.fmod0 >> 1) & 0xf;
			mode = fmt.s2mod;

			if (mode == 0)
			{
				ss << "R";
				if (src.i != 63)
					ss << src.i;
				else
					ss << "Z";
			}
			else if (mode == 1 || mode == 2)
			{
				unsigned bank = ((src.i & 0x1) << 4) | (src.i >> 16);
				int offset = src.i & 0xfffe;
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
			}
			else if (mode == 3)
			{
				InstCategory cat = info->category;
				if (cat == InstCategorySpFp || cat == InstCategoryDpFp)
				{
					src.i = fmt.src2 << 12;
					if (std::abs(src.f) < 1e-4 || std::abs(src.f) > 1e9)
						ss << misc::fmt("%.20e", src.f);
					else
						ss << misc::fmt("%.20g", src.f);
				}
				else if (cat == InstCategoryInt)
				{
					src.i = fmt.src2;
					if (src.i >> 19 == 0)  // positive value
						ss << misc::fmt("0x%x", src.i);
					else  // negative value
						ss << misc::fmt("-0x%x", 0x100000 - src.i);
				}
			}

			if (i != 15)
				ss << ", " << i;
		}
		else if (comm::Asm::isToken(fmt_str, "src2", len))
		{
			union {unsigned i; float f;} src;
			unsigned mode;

			src.i = fmt.src2;
			mode = fmt.s2mod;

			if (mode == 0)
			{
				ss << "R";
				if (src.i != 63)
					ss << src.i;
				else
					ss << "Z";
			}
			else if (mode == 1 || mode == 2)
			{
				unsigned bank = ((src.i & 0x1) << 4) | (src.i >> 16);
				int offset = src.i & 0xfffe;
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
			}
			else if (mode == 3)
			{
				src.i = fmt.src2;
				if (src.i >> 19 == 0)  // positive value
					ss << misc::fmt("0x%x", src.i);
				else  // negative value
					ss << misc::fmt("-0x%x", 0x100000 - src.i);
			}
		}
		else if (comm::Asm::isToken(fmt_str, "isrc2", len))
		{
			union {unsigned i; float f;} src;
			unsigned mode;
			unsigned i;

			src.i = fmt.src2;
			mode = fmt.s2mod;
			i = (fmt.fmod0 >> 4) & 0x1;

			if (mode == 0)
			{
				if (i == 1)
					ss << "~";
				ss << "R";
				if (src.i != 63)
					ss << src.i;
				else
					ss << "Z";
			}
			else if (mode == 1 || mode == 2)
			{
				unsigned bank = ((src.i & 0x1) << 4) | (src.i >> 16);
				int offset = src.i & 0xfffe;
				if (i == 1)
					ss << "~";
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
			}
			else if (mode == 3)
			{
				InstCategory cat = info->category;
				if (cat == InstCategorySpFp || cat == InstCategoryDpFp)
				{
					src.i = fmt.src2 << 12;
					if (std::abs(src.f) < 1e-4 || std::abs(src.f) > 1e9)
						ss << misc::fmt("%.20e", src.f);
					else
						ss << misc::fmt("%.20g", src.f);
				}
				else if (cat == InstCategoryInt)
				{
					src.i = fmt.src2;
					if (src.i >> 19 == 0)  // positive value
						ss << misc::fmt("0x%x", src.i);
					else  // negative value
						ss << misc::fmt("-0x%x", 0x100000 - src.i);
				}
				if (i == 1)
					ss << ".INV";
			}
		}
		else if (comm::Asm::isToken(fmt_str, "nasrc2", len))
		{
			union {unsigned i; float f;} src;
			unsigned mode;
			unsigned n, a;

			src.i = fmt.src2;
			mode = fmt.s2mod;
			n = (fmt.fmod0 >> 4) & 0x1;
			a = (fmt.fmod0 >> 2) & 0x1;

			if (mode == 0)
			{
				if (n == 1)
					ss << "-";
				if (a == 1)
					ss << "|";
				ss << "R";
				if (src.i != 63)
					ss << src.i;
				else
					ss << "Z";
				if (a == 1)
					ss << "|";
			}
			else if (mode == 1 || mode == 2)
			{
				unsigned bank = ((src.i & 0x1) << 4) | (src.i >> 16);
				int offset = src.i & 0xfffe;
				if (n == 1)
					ss << "-";
				if (a == 1)
					ss << "|";
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
				if (a == 1)
					ss << "|";
			}
			else if (mode == 3)
			{
				InstCategory cat = info->category;
				if (cat == InstCategorySpFp || cat == InstCategoryDpFp)
				{
					src.i = fmt.src2 << 12;
					if (std::abs(src.f) < 1e-4 || std::abs(src.f) > 1e9)
						ss << misc::fmt("%.20e", src.f);
					else
						ss << misc::fmt("%.20g", src.f);
				}
				else if (cat == InstCategoryInt)
				{
					src.i = fmt.src2;
					if (src.i >> 19 == 0)  // positive value
						ss << misc::fmt("0x%x", src.i);
					else  // negative value
						ss << misc::fmt("-0x%x", 0x100000 - src.i);
				}
				if (n == 1)
					ss << ".NEG";
				if (a == 1)
					ss << ".ABS";
			}
		}
		else if (comm::Asm::isToken(fmt_str, "nabsrc2", len))
		{
			union {unsigned i; float f;} src;
			unsigned mode;
			unsigned n, a, b;

			src.i = fmt.src2;
			mode = fmt.s2mod;
			n = (fmt.fmod0 >> 4) & 0x1;
			a = (fmt.fmod0 >> 2) & 0x1;
			b = (fmt.fmod2_srco >> 6) & 0x3;

			if (mode == 0)
			{
				if (n == 1)
					ss << "-";
				if (a == 1)
					ss << "|";
				ss << "R";
				if (src.i != 63)
					ss << src.i;
				else
					ss << "Z";
				if (b != 0)
					ss << ".B" << b;
				if (a == 1)
					ss << "|";
			}
			else if (mode == 1 || mode == 2)
			{
				unsigned bank = ((src.i & 0x1) << 4) | (src.i >> 16);
				int offset = src.i & 0xfffe;
				if (n == 1)
					ss << "-";
				if (a == 1)
					ss << "|";
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
				if (b != 0)
					ss << ".B" << b;
				if (a == 1)
					ss << "|";
			}
			else if (mode == 3)
			{
				InstCategory cat = info->category;
				if (cat == InstCategorySpFp || cat == InstCategoryDpFp)
				{
					src.i = fmt.src2 << 12;
					if (std::abs(src.f) < 1e-4 || std::abs(src.f) > 1e9)
						ss << misc::fmt("%.20e", src.f);
					else
						ss << misc::fmt("%.20g", src.f);
				}
				else if (cat == InstCategoryInt)
				{
					src.i = fmt.src2;
					if (src.i >> 19 == 0)  // positive value
						ss << misc::fmt("0x%x", src.i);
					else  // negative value
						ss << misc::fmt("-0x%x", 0x100000 - src.i);
				}
				if (b != 0)
					ss << ".B" << b;
				if (n == 1)
					ss << ".NEG";
				if (a == 1)
					ss << ".ABS";
			}
		}
		else if (comm::Asm::isToken(fmt_str, "nahisrc2", len))
		{
			union {unsigned i; float f;} src;
			unsigned mode;
			unsigned n, a, hi;

			src.i = fmt.src2;
			mode = fmt.s2mod;
			n = (fmt.fmod0 >> 4) & 0x1;
			a = (fmt.fmod0 >> 2) & 0x1;
			hi = (fmt.fmod2_srco >> 7) & 0x1;

			if (mode == 0)
			{
				if (n == 1)
					ss << "-";
				if (a == 1)
					ss << "|";
				ss << "R";
				if (src.i != 63)
					ss << src.i;
				else
					ss << "Z";
				if (a == 1)
					ss << "|";
			}
			else if (mode == 1 || mode == 2)
			{
				unsigned bank = ((src.i & 0x1) << 4) | (src.i >> 16);
				int offset = src.i & 0xfffe;
				if (n == 1)
					ss << "-";
				if (a == 1)
					ss << "|";
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
				if (a == 1)
					ss << "|";
			}
			else if (mode == 3)
			{
				InstCategory cat = info->category;
				if (cat == InstCategorySpFp || cat == InstCategoryDpFp)
				{
					src.i = fmt.src2 << 12;
					if (std::abs(src.f) < 1e-4 || std::abs(src.f) > 1e9)
						ss << misc::fmt("%.20e", src.f);
					else
						ss << misc::fmt("%.20g", src.f);
				}
				else if (cat == InstCategoryInt)
				{
					src.i = fmt.src2;
					if (src.i >> 19 == 0)  // positive value
						ss << misc::fmt("0x%x", src.i);
					else  // negative value
						ss << misc::fmt("-0x%x", 0x100000 - src.i);
				}
				if (n == 1)
					ss << ".NEG";
				if (a == 1)
					ss << ".ABS";
			}

			if (hi == 1)
				ss << ".HI";
		}
		else if (comm::Asm::isToken(fmt_str, "src3", len))
		{
			unsigned src;
			src = fmt.fmod2_srco & 0x3f;
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "ssrc", len))
		{
			unsigned src;
			src = fmt.src2 & 0xff;
			ss << ssrc_map.MapValue(src);
		}
		else if (comm::Asm::isToken(fmt_str, "imm6", len))
		{
			unsigned i;
			i = fmt.fmod1_src1;
			ss << misc::fmt("0x%x", i);
		}
		else if (comm::Asm::isToken(fmt_str, "psrc1", len))
		{
			unsigned p, i;
			p = fmt.fmod1_src1 & 0x7;
			i = (fmt.fmod1_src1 >> 3) & 0x1;
			if (i == 1)
				ss << "!";
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else if (comm::Asm::isToken(fmt_str, "psrc2", len))
		{
			unsigned p, i;
			p = fmt.src2 & 0x7;
			i = (fmt.src2 >> 3) & 0x1;
			if (i == 1)
				ss << "!";
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else if (comm::Asm::isToken(fmt_str, "psrc3", len))
		{
			unsigned p, i;
			p = fmt.fmod2_srco & 0x7;
			i = (fmt.fmod2_srco >> 3) & 0x1;
			if (i == 1)
				ss << "!";
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else if (comm::Asm::isToken(fmt_str, "psrc4", len))
		{
			unsigned p;
			p = (fmt.fmod2_srco >> 5) & 0x7;
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else
		{
			throw misc::Panic(misc::fmt("Unknown token: %s",
					fmt_str));
		}

		str += ss.str();

		// Skip processed token
		fmt_str += len;
	}
	str += ";";
}


void Inst::DumpToBufWithFmtLdSt(void)
{
	FmtLdSt fmt;
	const char *fmt_str;
	int len;

	// Get instruction format/encoding
	fmt = bytes.fmt_ldst;

	// Get format string
	fmt_str = info->fmt_str.c_str();

	// Process format string
	while (*fmt_str)
	{
		std::stringstream ss;

		// Character is a literal symbol
		if (*fmt_str != '%')
		{
			if (fmt_str == info->fmt_str.c_str())
				str += "        ";
			str += *fmt_str;
			++fmt_str;
			continue;
		}

		// Character is a token
		++fmt_str;

		if (comm::Asm::isToken(fmt_str, "pred", len))
		{
			unsigned pred;
			pred = fmt.pred;
			if (pred < 7)
				ss << "    @P" << pred;
			else if (pred > 7)
				ss << "   @!P" << pred - 8;
			else
				ss << "       ";
		}
		else if (comm::Asm::isToken(fmt_str, "1d", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 18) & 0x3;
			ss << _1d_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "aoffi", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 28) & 0x1;
			ss << aoffi_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "atomicsize", len))
		{
			unsigned v;
			v = (fmt.func & 0xe) | ((fmt.fmod0 >> 5) & 0x1);
			ss << atomicsize_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "atomop", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0xf;
			ss << atomop_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "b1", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 4) & 0x1;
			ss << b1_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "b4", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 23) & 0xf;
			ss << b4_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "blod", len))
		{
			unsigned v;
			v = ((fmt.func & 0x3) << 1) | ((fmt.fmod1_srco >> 31) & 0x1);
			ss << blod_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "btoff", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 28) & 0x3;
			ss << btoff_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "cachectrl", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x1f;
			ss << cachectrl_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "cctlop", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x1f;
			ss << cctlop_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "cl", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 30) & 0x1;
			ss << cl_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "dc", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 30) & 0x1;
			ss << dc_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "eu", len))
		{
			unsigned v;
			v = fmt.func & 0x1;
			ss << eu_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "e", len))
		{
			unsigned v;
			v = fmt.func & 0x1;
			ss << e_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "geom", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 25) & 0x7;
			ss << geom_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "il", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 4) & 0x1;
			ss << il_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "i", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 24) & 0x1;
			ss << i_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "ldcachectrl", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 4) & 0x3;
			ss << ldcachectrl_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "ldlcachectrl", len))
		{
			if (((fmt.fmod1_srco >> 30) & 0x1) == 0)  // LDL
			{
				unsigned v;
				v = (fmt.fmod0 >> 4) & 0x3;
				ss << ldlcachectrl_map.MapValue(v);
			}
		}
		else if (comm::Asm::isToken(fmt_str, "ls", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 30) & 0x1;
			ss << ls_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "lz", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 31) & 0x1;
			ss << lz_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "membar", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x3;
			ss << membar_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "mode", len))
		{
			unsigned v;
			v = ((fmt.func & 0x1) << 4) | ((fmt.fmod1_srco >> 28) & 0xf);
			ss << mode_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "ms", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 29) & 0x1;
			ss << ms_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "ndv", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 19) & 0x1;
			ss << ndv_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "nodep", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 5) & 0x1;
			ss << nodep_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "phase", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 3) & 0x3;
			ss << phase_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "r2", len))
		{
			unsigned v;
			v = fmt.fmod0 & 0x3;
			ss << r2_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "r4", len))
		{
			unsigned v;
			if (info->op == InstOpSURED)
				v = (fmt.fmod1_srco >> 6) & 0xf;
			else
				v = (fmt.fmod1_srco >> 23) & 0xf;
			ss << r4_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "redop", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0xf;
			ss << redop_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "rf2", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 29) & 0x3;
			ss << rf2_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "rf3", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x7;
			ss << rf3_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "size3", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x7;
			if (v != 7)
				ss << size3_map.MapValue(v);
			else
			{
				if (info->op == InstOpLD)
				{
					if (((fmt.fmod0 >> 5) & 0x1) == 0)
						ss << ".64";
					else
						ss << ".128";
				}
				else if (info->op == InstOpLDX && ((fmt.fmod1_srco >> 30) & 0x1) == 1)  // LDS
				{
					if (((fmt.fmod0 >> 4) & 0x3) == 0)
						ss << ".BV.64";
					else if (((fmt.fmod0 >> 4) & 0x3) == 2)
						ss << ".BV.128";
				}
				else
					ss << ".INVALIDSIZE7";
			}
		}
		else if (comm::Asm::isToken(fmt_str, "size5", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 27) & 0x1f;
			ss << size5_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "stcachectrl", len))
		{
			unsigned v;
			if (!((info->op == InstOpSTX) && (((fmt.fmod1_srco >> 30) & 0x1) == 1)))  // !STS
			{
				v = (fmt.fmod0 >> 4) & 0x3;
				ss << stcachectrl_map.MapValue(v);
			}
		}
		else if (comm::Asm::isToken(fmt_str, "sucachectrl", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 4) & 0x3;
			ss << sucachectrl_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "suqop", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x1f;
			ss << suqop_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "suredop", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0xf;
			ss << suredop_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "surfaceclamp", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 21) & 0x3;
			ss << surfaceclamp_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "s", len))
		{
			unsigned v;
			v = fmt.fmod0 & 0x1;
			ss << s_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "u", len))
		{
			unsigned v;
			v = fmt.fmod1_srco & 0x3;
			ss << u_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "dstsrc4", len))
		{
			unsigned dst, src;
			dst = fmt.dst;
			src = (fmt.fmod1_srco >> 23) & 0x3f;
			ss << "R";
			if (dst != 63)
				ss << dst;
			else
				ss << "Z";
			if (src != 63)
				ss << ", R" << src;
		}
		else if (comm::Asm::isToken(fmt_str, "dst", len))
		{
			unsigned dst;
			dst = fmt.dst;
			ss << "R";
			if (dst != 63)
				ss << dst;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "pdst", len))
		{
			unsigned p, dst;
			p = (fmt.fmod1_srco >> 28) & 0x7;
			dst = fmt.dst;
			if (p != 7)
				ss << "P" << p << ", ";
			ss << "R";
			if (dst != 63)
				ss << dst;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "src1src2", len))
		{
			unsigned src1, src2;
			src1 = fmt.src1;
			src2 = fmt.fmod1_srco & 0x3f;
			ss << "R";
			if (src1 != 63)
				ss << src1;
			else
				ss << "Z";
			if (src2 != 63)
			{
				ss << ", R";
				if (src2 != 63)
					ss << src2;
				else
					ss << "Z";
			}
		}
		else if (comm::Asm::isToken(fmt_str, "src1off16", len))
		{
			unsigned src, off16, s;
			src = fmt.src1;
			off16 = fmt.fmod1_srco & 0xffff;
			s = off16 >> 15;
			if (src != 63 && off16 != 0)
			{
				ss << "R" << src << "+";
				if (s == 0)
					ss << misc::fmt("0x%x", off16);
				else
					ss << misc::fmt("-0x%x", 0x1000000 - off16);
			}
			else if (src == 63 && off16 != 0)
				ss << misc::fmt("0x%x", off16);
			else if (src != 63 && off16 == 0)
				ss << "R" << src;
			else
				ss << "0x0";
		}
		else if (comm::Asm::isToken(fmt_str, "src1off24", len))
		{
			unsigned src, off24, s;
			src = fmt.src1;
			off24 = fmt.fmod1_srco & 0xffffff;
			s = off24 >> 23;
			if (src != 63 && off24 != 0)
			{
				ss << "R" << src;
				if (s == 0)
					ss << misc::fmt("+0x%x", off24);
				else
					ss << misc::fmt("+-0x%x", 0x1000000 - off24);
			}
			else if (src == 63 && off24 != 0)
				ss << misc::fmt("0x%x", off24);
			else if (src != 63 && off24 == 0)
				ss << "R" << src;
			else
				ss << "RZ";
		}
		else if (comm::Asm::isToken(fmt_str, "src1imm41imm42", len))
		{
			unsigned src, imm41, imm42;
			src = fmt.src1;
			imm41 = (fmt.fmod1_srco >> 6) & 0xf;
			imm42 = (fmt.fmod1_srco >> 10) & 0xf;
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
			if (imm41 != 0 || imm42 != 0)
			{
				ss << misc::fmt(", 0x%x, 0x%x", imm41, imm42);
			}
		}
		else if (comm::Asm::isToken(fmt_str, "src1", len))
		{
			unsigned src;
			src = fmt.src1;
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "src2imm8", len))
		{
			unsigned src;
			unsigned i;
			src = fmt.fmod1_srco & 0x3f;
			i = (fmt.fmod1_srco >> 6) & 0xff;
			if (src != 63)
				ss << "R" << src << ", ";
			ss << misc::fmt("0x%x", i);
		}
		else if (comm::Asm::isToken(fmt_str, "src2", len))
		{
			unsigned src;
			src = fmt.fmod1_srco & 0x3f;
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "risrc2", len))
		{
			unsigned src;
			unsigned mode;
			src = fmt.fmod1_srco & 0x3f;
			mode = (fmt.fmod1_srco >> 20) & 0x1;
			if (mode == 0)
			{
				ss << "R";
				if (src != 63)
					ss << src;
				else
					ss << "Z";
			}
			else
				ss << misc::fmt("0x%x", src);
		}
		else if (comm::Asm::isToken(fmt_str, "src3", len))
		{
			unsigned src;
			if (info->op == InstOpATOM)
				src = (fmt.fmod1_srco >> 17) & 0x3f;
			else
				src = (fmt.fmod1_srco >> 6) & 0x3f;
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "off141", len))
		{
			unsigned s, v;
			s = (fmt.fmod1_srco >> 27) & 0x3f;
			v = (fmt.fmod1_srco >> 17) & 0x3ff;
			if (s <= 9)
				ss << v;
			else if (s <= 19)
				ss << (v << 1);
			else if (s <= 26)
				ss << (v << 2);
			else if (s <= 29)
				ss << (v << 3);
			else
				ss << (v << 4);
		}
		else if (comm::Asm::isToken(fmt_str, "off142", len))
		{
			unsigned s, v;
			s = (fmt.fmod1_srco >> 27) & 0x3f;
			v = (((fmt.fmod1_srco >> 12) & 0x1f) << 5) | ((fmt.fmod0 >> 1) & 0x1f);
			if (s == 0 || s == 1 || s == 5 || s == 6 || s == 10 || s == 11 || s == 15 || s == 16 || s == 20 || s == 21)
				ss << v;
			else if (s == 2 || s == 3 || s == 7 || s == 8 || s == 12 || s == 13 || s == 17 || s == 18 || s == 22 || s == 23)
				ss << (v << 1);
			else if (s == 4 || s == 9 || s == 14 || s == 19 || s == 24 || s == 27 || s == 30)
				ss << (v << 2);
			else if (s == 25 || s == 28 || s == 31)
				ss << (v << 3);
			else
				ss << (v << 4);
		}
		else if (comm::Asm::isToken(fmt_str, "off16", len))
		{
			int v, s;
			v = fmt.fmod1_srco & 0xffff;
			s = v >> 15;
			if (v != 0)
			{
				if (s == 0)
					ss << misc::fmt("+0x%x", v);
				else
					ss << misc::fmt("-0x%x", 0x10000 - v);
			}
		}
		else if (comm::Asm::isToken(fmt_str, "off20", len))
		{
			int v, s;
			v = (((fmt.fmod1_srco >> 29) & 0x7) << 17) | (fmt.fmod1_srco & 0x1ffff);
			s = v >> 19;
			if (v != 0)
			{
				if (s == 0)
					ss << misc::fmt("+0x%x", v);
				else
					ss << misc::fmt("-0x%x", 0x100000 - v);
			}
		}
		else if (comm::Asm::isToken(fmt_str, "off24", len))
		{
			int v, s;
			if (info->op == InstOpCCTLL)
				v = fmt.fmod1_srco & 0xfffffc;
			else
				v = fmt.fmod1_srco & 0xffffff;
			s = v >> 23;
			if (v != 0)
			{
				if (s == 0)
					ss << misc::fmt("+0x%x", v);
				else
					ss << misc::fmt("-0x%x", 0x1000000 - v);
			}
		}
		else if (comm::Asm::isToken(fmt_str, "off32", len))
		{
			int v;
			if (info->op == InstOpCCTL)
				v = fmt.fmod1_srco & 0xfffffffc;
			else
				v = fmt.fmod1_srco & 0xffffffff;
			if (v > 0)
				ss << misc::fmt("+0x%x", v);
			else if (v < 0)
				ss << misc::fmt("+-0x%x", ~v + 1);
		}
		else if (comm::Asm::isToken(fmt_str, "imm41", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 6) & 0xf;
			ss << misc::fmt("0x%x", v);
		}
		else if (comm::Asm::isToken(fmt_str, "imm42", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 10) & 0xf;
			ss << misc::fmt("0x%x", v);
		}
		else if (comm::Asm::isToken(fmt_str, "imm43", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 20) & 0xf;
			ss << misc::fmt("0x%x", v);
		}
		else if (comm::Asm::isToken(fmt_str, "imm5", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 14) & 0x1f;
			ss << misc::fmt("0x%x", v);
		}
		else if (comm::Asm::isToken(fmt_str, "imm8", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 6) & 0xff;
			ss << misc::fmt("0x%x", v);
		}
		else if (comm::Asm::isToken(fmt_str, "bank", len))
		{
			unsigned v;
			v = (fmt.fmod1_srco >> 16) & 0x1f;
			ss << misc::fmt("0x%x", v);
		}
		else if (comm::Asm::isToken(fmt_str, "psrc", len))
		{
			unsigned p;
			if (info->op == InstOpLDLK)
				p = ((fmt.func & 0x1) << 2) | ((fmt.fmod0 >> 4) & 0x3);
			else
				p = (fmt.fmod1_srco >> 24) & 0x7;
			ss << "P";
			if (p != 7)
				ss << p;
			else
				ss << "T";
		}
		else
		{
			throw misc::Panic(misc::fmt("Unknown token: %s",
					fmt_str));
		}

		str += ss.str();

		// Skip processed token
		fmt_str += len;
	}
	str += ";";
}


void Inst::DumpToBufWithFmtCtrl(void)
{
	FmtCtrl fmt;
	const char *fmt_str;
	int len;

	// Get instruction format/encoding
	fmt = bytes.fmt_ctrl;

	// Get format string
	fmt_str = info->fmt_str.c_str();

	// Process format string
	while (*fmt_str)
	{
		std::stringstream ss;

		// Character is a literal symbol
		if (*fmt_str != '%')
		{
			if (fmt_str == info->fmt_str.c_str())
				str += "        ";
			str += *fmt_str;
			++fmt_str;
			continue;
		}

		// Character is a token
		++fmt_str;

		if (comm::Asm::isToken(fmt_str, "pred", len))
		{
			unsigned pred;
			pred = fmt.pred;
			if (pred < 7)
				ss << "    @P" << pred;
			else if (pred > 7)
				ss << "   @!P" << pred - 8;
			else
				ss << "       ";
		}
		else if (comm::Asm::isToken(fmt_str, "drain", len))
		{
			unsigned v;
			v = fmt.mmod & 0x3;
			ss << drain_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "lmt", len))
		{
			unsigned v;
			v = (fmt.mmod >> 2) & 0x1;
			ss << lmt_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "noinc", len))
		{
			unsigned v;
			v = (fmt.mmod >> 2) & 0x1;
			ss << noinc_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "u", len))
		{
			unsigned v;
			v = (fmt.mmod >> 1) & 0x1;
			ss << u_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "cccopatarget", len))
		{
			unsigned cccop;
			unsigned mode;
			unsigned target;
			unsigned bank, offset;

			cccop = (fmt.fmod0 >> 1) & 0x1f;
			mode = fmt.mmod & 0x1;
			target = fmt.imm32;
			bank = (fmt.imm32 >> 16) & 0x1f;
			offset = fmt.imm32 & 0xffff;

			if (cccop != 15)
				ss << cccop_map.MapValue(cccop) << ", ";
			if (mode == 0)
				ss << misc::fmt("0x%x", target);
			else
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
		}
		else if (comm::Asm::isToken(fmt_str, "cccopaitarget24", len))
		{
			unsigned cccop;
			unsigned mode;
			unsigned src;
			unsigned target, s;
			unsigned bank, offset;

			cccop = (fmt.fmod0 >> 1) & 0x1f;
			mode = fmt.mmod & 0x1;
			src = fmt.src1;
			target = fmt.imm32 & 0xffffff;
			s = target >> 23;
			bank = (fmt.imm32 >> 16) & 0x1f;
			offset = fmt.imm32 & 0xffff;

			if (cccop != 15)
				ss << cccop_map.MapValue(cccop) << ", ";
			if (mode == 0)
			{
				ss << "R" << src << " ";
				if (s == 0)
					ss << misc::fmt("0x%x", target);
				else
					ss << misc::fmt("-0x%x", 0x1000000 - target);
			}
			else
			{
				ss << misc::fmt("c[R%d+0x%x][0x%x]", src, bank, offset);
			}
		}
		else if (comm::Asm::isToken(fmt_str, "cccoprtarget", len))
		{
			unsigned cccop;
			unsigned mode;
			unsigned target, s;
			unsigned bank, offset;

			cccop = (fmt.fmod0 >> 1) & 0x1f;
			mode = fmt.mmod & 0x1;
			target = fmt.imm32 & 0xffffff;
			s = target >> 23;
			bank = (fmt.imm32 >> 16) & 0x1f;
			offset = fmt.imm32 & 0xffff;

			if (cccop != 15)
				ss << cccop_map.MapValue(cccop) << ", ";
			if (mode == 0)
			{
				if (s == 0)
					ss << misc::fmt("0x%x", target + addr + 8);
				else
					ss << misc::fmt("0x%x", -(0x1000000 - target) + addr + 8);
			}
			else
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
		}
		else if (comm::Asm::isToken(fmt_str, "cccopsrc1", len))
		{
			unsigned cccop;
			unsigned src;

			cccop = (fmt.fmod0 >> 1) & 0x1f;
			src = fmt.src1;

			if (cccop != 15)
				ss << cccop_map.MapValue(cccop) << ", ";
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "cccop", len))
		{
			unsigned v;
			v = (fmt.fmod0 >> 1) & 0x1f;
			ss << cccop_map.MapValue(v);
		}
		else if (comm::Asm::isToken(fmt_str, "src1", len))
		{
			unsigned src;
			src = fmt.src1;
			ss << "R";
			if (src != 63)
				ss << src;
			else
				ss << "Z";
		}
		else if (comm::Asm::isToken(fmt_str, "atarget", len))
		{
			unsigned mode;
			unsigned target;
			unsigned bank, offset;
			mode = fmt.mmod & 0x1;
			target = fmt.imm32;
			bank = (fmt.imm32 >> 16) & 0x1f;
			offset = fmt.imm32 & 0xffff;
			if (mode == 0)
				ss << misc::fmt("0x%x", target);
			else
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
		}
		else if (comm::Asm::isToken(fmt_str, "aitarget", len))
		{
			unsigned mode;
			unsigned src;
			unsigned target;
			unsigned bank, offset;
			mode = fmt.mmod & 0x1;
			src = fmt.src1;
			target = fmt.imm32;
			bank = (fmt.imm32 >> 16) & 0x1f;
			offset = fmt.imm32 & 0xffff;
			if (mode == 0)
			{
				ss << "R" << src << " ";
				ss << misc::fmt("0x%x", target);
			}
			else
			{
				ss << misc::fmt("c[R%d+0x%x][0x%x]", src, bank, offset);
			}
		}
		else if (comm::Asm::isToken(fmt_str, "rtarget", len))
		{
			unsigned mode;
			unsigned target, s;
			unsigned bank, offset;
			mode = fmt.mmod & 0x1;
			target = fmt.imm32 & 0xffffff;
			s = target >> 23;
			bank = (fmt.imm32 >> 16) & 0x1f;
			offset = fmt.imm32 & 0xffff;
			if (mode == 0)
			{
				if (s == 0)
					ss << misc::fmt("0x%x", target + addr + 8);
				else
					ss << misc::fmt("0x%x", -(0x1000000 - target) + addr + 8);
			}
			else
				ss << misc::fmt("c[0x%x][0x%x]", bank, offset);
		}
		else if (comm::Asm::isToken(fmt_str, "ritarget", len))
		{
			unsigned mode;
			unsigned src;
			unsigned target, s;
			unsigned bank, offset;
			mode = fmt.mmod & 0x1;
			src = fmt.src1;
			target = fmt.imm32 & 0xffffff;
			s = target >> 23;
			bank = (fmt.imm32 >> 16) & 0x1f;
			offset = fmt.imm32 & 0xffff;
			if (mode == 0)
			{
				ss << "R" << src << " ";
				if (s == 0)
					ss << misc::fmt("0x%x", target + addr + 8);
				else
					ss << misc::fmt("0x%x", -(0x1000000 - target) + addr + 8);
			}
			else
			{
				ss << misc::fmt("c[R%d+0x%x][0x%x]", src, bank, offset);
			}
		}
		else if (comm::Asm::isToken(fmt_str, "imm20", len))
		{
			unsigned i;
			i = fmt.imm32 & 0xfffff;
			if (i != 0)
				ss << misc::fmt("0x%x", i);
		}
		else
		{
			throw misc::Panic(misc::fmt("Unknown token: %s",
					fmt_str));
		}

		str += ss.str();

		// Skip processed token
		fmt_str += len;
	}
	str += ";";
}


void Inst::DumpToBuf(void)
{
	int cat;

	// Check if instruction is supported
	if (!info)
		throw Asm::Error(misc::fmt("Instruction not supported at 0x%x",
				addr));

	// Get instruction category
	cat = bytes.bytes[0] & 0xf;

	// Dump to buffer based on format
	if (cat == InstCategorySpFp || cat == InstCategoryDpFp || cat == InstCategoryInt)
		DumpToBufWithFmtReg();
	else if (cat == InstCategoryImm)
		DumpToBufWithFmtImm();
	else if (cat == InstCategoryOther)
		DumpToBufWithFmtOther();
	else if (cat == InstCategoryLdSt || cat == InstCategoryLdRO)
		DumpToBufWithFmtLdSt();
	else if (cat == InstCategoryCtrl)
		DumpToBufWithFmtCtrl();
	else
		throw Asm::Error(misc::fmt("Invalid instruction category %d "
				"at offset 0x%x", cat, addr));
}


void Inst::Dump(std::ostream &os, unsigned width)
{
	os << str;
	for (unsigned i = str.length(); i <= width; ++i)
		os << " ";
}


void Inst::DumpHex(std::ostream &os)
{
	os << misc::fmt("/* 0x%016llx */", bytes.dword);
}


void Inst::Decode(unsigned addr, const char *ptr)
{
	unsigned cat;
	unsigned op_in_cat;

	// Get instruction category
	bytes.dword = * (unsigned long long *) ptr;
	cat = bytes.bytes[0] & 0xf;

	// Get opcode bits
	if (cat <= 3)
		op_in_cat = bytes.bytes[7] >> 3;  // 5-bit
	else
		op_in_cat = bytes.bytes[7] >> 2;  // 6-bit

	// Get opcode bits: special cases
	if (cat == InstCategorySpFp && ((op_in_cat >> 1) & 0xf) == 2)  // FSETP
		op_in_cat = op_in_cat & 0x1e;
	else if (cat == InstCategoryLdSt && ((op_in_cat & 0x30) >> 4) == 0)  // RED
		op_in_cat = op_in_cat & 0x30;
	else if (cat == InstCategoryLdSt && ((op_in_cat & 0x30) >> 4) == 1)  // ATOM
		op_in_cat = op_in_cat & 0x30;

	// Get instruction information
	info = as->GetInstInfo(cat, op_in_cat);

	// Set virtual address
	this->addr = addr;
}


}  // namespace Fermi


