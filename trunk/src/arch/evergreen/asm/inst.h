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

#ifndef EVERGREEN_ASM_INST_H
#define EVERGREEN_ASM_INST_H

#include <lib/class/class.h>

/*
 * Class 'EvgInst'
 */


typedef enum
{
	EvgInstOpcodeInvalid = 0,

#define DEFINST(_name, _fmt_str, _fmt0, _fmt1, _fmt2, _category, _cf_inst, _flags) \
	EVG_INST_##_name,
#include "asm.dat"
#undef DEFINST

	/* Max */
	EvgInstOpcodeCount
} EvgInstOpcode;


typedef enum
{
	EvgInstCategoryInvalid = 0,

	EvgInstCategoryCF,  /* Control-flow instructions */
	EvgInstCategoryALU,  /* ALU clause instructions */
	EvgInstCategoryLDS,  /* LDS clause instructions */
	EvgInstCategoryVTX,  /* Instructions for a fetch through a vertex cache clause */
	EvgInstCategoryTEX,  /* Instructions for a fetch through a texture cache clause */
	EvgInstCategoryMemRd,  /* Memory read instructions */
	EvgInstCategoryMemGDS, /* Global data-share read/write instructions */

	EvgInstCategoryCount
} EvgInstCategory;


/* Microcode Formats */
typedef enum
{
	EvgInstFormatInvalid = 0,

	/* Control flow instructions */
	EvgInstFormatCfWord0,
	EvgInstFormatCfGwsWord0,
	EvgInstFormatCfWord1,

	EvgInstFormatCfAluWord0,
	EvgInstFormatCfAluWord1,

	EvgInstFormatCfAluWord0Ext,
	EvgInstFormatCfAluWord1Ext,

	EvgInstFormatCfAllocExportWord0,
	EvgInstFormatCfAllocExportWord0Rat,
	EvgInstFormatCfAllocExportWord1Buf,
	EvgInstFormatCfAllocExportWord1Swiz,

	/* ALU Clause Instructions */
	EvgInstFormatAluWord0,
	EvgInstFormatAluWord1Op2,
	EvgInstFormatAluWord1Op3,

	/* LDS Clause Instructions */
	EvgInstFormatAluWord0LdsIdxOp,
	EvgInstFormatAluWord1LdsIdxOp,
	EvgInstFormatAluWord1LdsDirectLiteralLo,
	EvgInstFormatAluWord1LdsDirectLiteralHi,

	/* Instructions for a Fetch Through a Vertex Cache Clause */
	EvgInstFormatVtxWord0,
	EvgInstFormatVtxWord1Gpr,
	EvgInstFormatVtxWord1Sem,
	EvgInstFormatVtxWord2,

	/* Instructions for a Fetch Through a Texture Cache Clause */
	EvgInstFormatTexWord0,
	EvgInstFormatTexWord1,
	EvgInstFormatTexWord2,

	/* Memory Read Instructions */
	EvgInstFormatMemRdWord0,
	EvgInstFormatMemRdWord1,
	EvgInstFormatMemRdWord2,

	/* Global Data-Share Read/Write Instructions */
	EvgInstFormatMemGdsWord0,
	EvgInstFormatMemGdsWord1,
	EvgInstFormatMemGdsWord2,

	/* Max */
	EvgInstFormatCount
} EvgInstFormat;


typedef enum
{
	EvgInstFlagInvalid = 0x0000,
	EvgInstFlagTransOnly = 0x0001,  /* Only executable in transcendental unit */
	EvgInstFlagIncLoopIdx = 0x0002,  /* CF inst increasing loop depth index */
	EvgInstFlagDecLoopIdx = 0x0004,  /* CF inst decreasing loop index */
	EvgInstFlagDstInt = 0x0008,  /* Inst with integer dest operand */
	EvgInstFlagDstUint = 0x0010,  /* Inst with unsigned int dest op */
	EvgInstFlagDstFloat = 0x0020,  /* Inst with float dest op */
	EvgInstFlagActMask = 0x0040,  /* Inst affects the active mask (control flow) */
	EvgInstFlagLDS = 0x0080,  /* Access to local memory */
	EvgInstFlagMem = 0x0100,  /* Access to global memory */
	EvgInstFlagMemRead = 0x0200,  /* Read to global memory */
	EvgInstFlagMemWrite = 0x0400,  /* Write to global memory */
	EvgInstFlagPredMask = 0x0800   /* Inst affects the predicate mask */
} EvgInstFlag;


#define EVG_INST_MAX_WORDS  3
typedef struct
{
	EvgInstOpcode opcode;
	EvgInstCategory category;
	char *name;
	char *fmt_str;
	EvgInstFormat fmt[EVG_INST_MAX_WORDS];  /* Word formats */

	/* Instruction bits identifying the instruction */
	int op;

	EvgInstFlag flags;  /* Flag bitmap */
	int size;  /* Number of words (32-bit) */
} EvgInstInfo;


CLASS_BEGIN(EvgInst, Object)

CLASS_END(EvgInst)


void EvgInstCreate(EvgInst *self, EvgAsm *as);
void EvgInstDestroy(EvgInst *self);

#endif

