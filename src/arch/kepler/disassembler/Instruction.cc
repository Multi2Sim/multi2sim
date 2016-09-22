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

#include "Instruction.h"

#include <cassert>
#include <iomanip>
#include <iostream>

#include <arch/common/Disassembler.h>
#include <arch/kepler/timing/Uop.h>
#include <lib/cpp/Error.h>
#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "Disassembler.h"



namespace Kepler
{


misc::StringMap inst_sat_map =
{
	{ "", 0},
	{ ".SAT", 1}
};

misc::StringMap inst_x_map =
{
	{ "", 0},
	{ ".X", 1}
};

misc::StringMap inst_cc2_map =
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
};

misc::StringMap inst_cc_map =
{
	{ "", 0},
	{ ".CC", 1}
};

misc::StringMap inst_hi_map =
{
	{ "", 0},
	{ ".HI", 1}
};

misc::StringMap inst_keeprefcount_map =
{
	{ "", 0},
	{ ".KEEPREFCOUNT", 1}
};

misc::StringMap inst_s_map =
{
	{ "", 0},
	{ ".S", 1}
};

misc::StringMap inst_and_map =
{
	{ ".AND", 0},
	{ ".OR", 1},
	{ ".XOR", 2},
	{ ".INVALIDBOP3", 3}
};

misc::StringMap inst_cv_map =
{
	{ "", 0},
	{ ".CG", 1},
	{ ".CS", 2},
	{ ".CV", 3}
};

misc::StringMap inst_wt_map =
{
	{ "", 0},
	{ ".CG", 1},
	{ ".CS", 2},
	{ ".WT", 3}
};

misc::StringMap inst_po_map =
{
	{ "", 0},
	{ ".PO", 1}
};

misc::StringMap inst_u1_map =
{
	{ ".U32.U32", 0},
	{ ".U32.S32", 1}
};

misc::StringMap inst_f_map=
{
	{ ".F", 0},
	{ ".LT", 1},
	{ ".EQ", 2},
	{ ".LE", 3},
	{ ".GT", 4},
	{ ".NE", 5},
	{ ".GE", 6},
	{ ".T", 7}
};

misc::StringMap inst_u_map =
{
	{ "", 0},
	{ ".U", 1}
};

misc::StringMap inst_lmt_map =
{
	{ "", 0},
	{ ".LMT", 1}
};

misc::StringMap inst_e_map =
{
	{ "", 0},
	{ ".E", 1}
};

misc::StringMap inst_u32_map =
{
	{ ".U32", 0},
	{ "", 1}
};

misc::StringMap inst_rm_map =
{
	{ "", 0},
	{ ".RM", 1},
	{ ".RP", 2},
	{ ".RZ", 3}
};

misc::StringMap inst_us_map =
{
	{ ".U32", 0},
	{ ".S32", 1}
};

misc::StringMap inst_u8_map =
{
	{ ".U8", 0},
	{ ".S8", 1},
	{ ".U16", 2},
	{ ".S16", 3},
	{ "", 4},
	{ ".64", 5},
	{ ".128", 6},
	{ ".U.128", 7}
};


Instruction::DecodeInfo::DecodeInfo()
{
	next_table_low = 0;
	next_table_high = 0;
	next_table = nullptr;
	info = nullptr;
}


Instruction::Instruction()
{
	disassembler = Disassembler::getInstance();
}


void Instruction::Decode(const char *buffer, unsigned int address)
{
	// Populate
	if (!address % 64)
		info = nullptr;
	else
	{
		this->address = address;
		bytes.as_dword = * (unsigned long long *) buffer;

		// Start with master table
		const DecodeInfo *table = disassembler->getDecTable();
		int low = 0;
		int high = 1;

		// Traverse tables
		while (1)
		{
			int index = misc::getBits64(bytes.as_dword, high, low);
			if (!table[index].next_table)
			{
				info = table[index].info;
				return;
			}

			// Go to next table
			low = table[index].next_table_low;
			high = table[index].next_table_high;
			table = table[index].next_table;
		}
	}
}


void Instruction::ReadRegistersIndex(Opcode opcode, Uop *uop)
{
	switch (opcode)
	{
	case (Opcode) (OpcodeInvalid):
	{
		instruction_format = FormatInvalid;
		break;
	}
	case (Opcode)(INST_MOV_A):
	{
		instruction_format = FormatBit1;
		break;
	}

	case (Opcode)(INST_MOV_B):
	{
		BytesGeneral0 format = bytes.general0;

		// Set source and destination register index
		if (format.srcB_mod == 1)
			uop->setSourceRegisterIndex(0, format.srcB);
		uop->setDestinationRegisterIndex(0, format.dst);

		// Set predicate register index p7 is always 1
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		instruction_format = FormatBit1;
		break;
	}

	case (Opcode)(INST_IADD_A):
	{
		BytesIADD format = bytes.iadd;
		uop->setSourceRegisterIndex(0, format.src1);
		uop->setDestinationRegisterIndex(0, format.dst);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		instruction_format = FormatInteger1;
		break;
	}

	case (Opcode)(INST_IADD_B):
	{
		BytesIADD format = bytes.iadd;
		uop->setSourceRegisterIndex(0, format.src1);
		// Register mode
		if (format.op2 == 3)
			uop->setSourceRegisterIndex(1, format.src2);
		uop->setDestinationRegisterIndex(0, format.dst);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		instruction_format = FormatInteger1;
		break;
	}

	case (Opcode)(INST_IMUL_A):
	{
		BytesIMUL format = bytes.imul;
		uop->setSourceRegisterIndex(0, format.src1);
		uop->setDestinationRegisterIndex(0, format.dst);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		instruction_format = FormatInteger2;
		break;
	}

	case (Opcode)(INST_IMUL_B):
	{
		BytesIMUL format = bytes.imul;
		uop->setSourceRegisterIndex(0, format.src1);
		// Register mode
		if (format.op2 == 1)
			uop->setSourceRegisterIndex(1, format.src2);
		uop->setDestinationRegisterIndex(0, format.dst);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		instruction_format = FormatInteger2;
		break;
	}

	case (Opcode)(INST_ISCADD_A):
	{
		BytesISCADD format = bytes.iscadd;
		uop->setSourceRegisterIndex(0, format.src1);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatInteger3;
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		break;
	}

	case (Opcode)(INST_ISCADD_B):
	{
		BytesISCADD format = bytes.iscadd;
		uop->setSourceRegisterIndex(0, format.src1);
		uop->setDestinationRegisterIndex(0, format.dst);
		if (format.op2 == 3)
			uop->setSourceRegisterIndex(1, format.src2);
		instruction_format = FormatInteger3;
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		break;
	}

	case (Opcode)(INST_ISETP_A):
	{
		BytesGeneral0 format = bytes.general0;
		uop->setSourceRegisterIndex(0, format.mod0);
		uop->setSourceRegisterIndex(1, format.srcB);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		if ((format.mod1 & 0x7) != 7)
			uop->setSourcePredicateIndex(1, format.mod1 & 0x7);
		uop->setDestinationPredicateIndex(0,(format.dst >> 3) & 0x7);
		uop->setDestinationPredicateIndex(1,format.dst & 0x7);
		instruction_format = FormatInteger1;
		break;
	}

	case (Opcode)(INST_ISETP_B):
	{
		BytesGeneral0 format = bytes.general0;
		uop->setSourceRegisterIndex(0, format.mod0);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		if ((format.mod1 & 0x7) != 7)
			uop->setSourcePredicateIndex(1, format.mod1 & 0x7);
		uop->setDestinationPredicateIndex(0,(format.dst >> 3) & 0x7);
			uop->setDestinationPredicateIndex(1,format.dst & 0x7);
		instruction_format = FormatInteger1;
		break;
	}

	case (Opcode)(INST_BRA):
	{
		instruction_format = FormatControl;
		break;
	}

	case (Opcode)(INST_EXIT):
	{
		instruction_format = FormatControl;
		break;
	}

	case (Opcode)(INST_LD):
	{
		BytesGeneral0 format = bytes.general0;
		uop->setSourceRegisterIndex(0, format.mod0);
		uop->setDestinationRegisterIndex(0, format.dst);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);

		// Set uop memory read flag
		uop->memory_read = true;
		instruction_format = FormatLS;
		break;
	}

	case (Opcode)(INST_SHL_A):
	{
		BytesSHL format = bytes.shl;
		uop->setSourceRegisterIndex(0, format.src1);
		uop->setDestinationRegisterIndex(0, format.dst);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		instruction_format = FormatInteger3;
		break;
	}

	case (Opcode)(INST_SHL_B):
	{
		BytesSHL format = bytes.shl;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.op2 == 3)
			uop->setDestinationRegisterIndex(1, format.src2);
		uop->setDestinationRegisterIndex(0, format.dst);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		instruction_format = FormatInteger3;
		break;
	}

	case (Opcode)(INST_SHR_A):
	{
		BytesSHR format = bytes.shr;
		uop->setSourceRegisterIndex(0, format.src1);
		uop->setDestinationRegisterIndex(0, format.dst);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		instruction_format = FormatInteger3;
		break;
	}

	case (Opcode)(INST_SHR_B):
	{
		BytesSHR format = bytes.shr;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.op2 == 3)
			uop->setDestinationRegisterIndex(1, format.src2);
		uop->setDestinationRegisterIndex(0, format.dst);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		instruction_format = FormatInteger3;
		break;
	}

	case (Opcode)(INST_S2R):
	{
		BytesGeneral0 format = bytes.general0;
		uop->setDestinationRegisterIndex(0, format.dst);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		instruction_format = FormatBit2;

		break;
	}

	case (Opcode)(INST_PSETP):
	{
		BytesPSETP format = bytes.psetp;
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		if (format.pred2 != 7)
			uop->setSourcePredicateIndex(1, format.pred2);
		if (format.pred3 != 7)
			uop->setSourcePredicateIndex(2, format.pred3);
		if (format.pred4 != 7)
			uop->setSourcePredicateIndex(3, format.pred4);

		if (format.pred0 != 7)
			uop->setDestinationPredicateIndex(0, format.pred0);
		if (format.pred1 != 7)
			uop->setDestinationPredicateIndex(1, format.pred1);
		instruction_format = FormatBit2;
		break;
	}

	case (Opcode)(INST_IMAD):
	{
		BytesGeneral0 format = bytes.general0;
		uop->setSourceRegisterIndex(0,format.mod0);
		uop->setSourceRegisterIndex(1,format.mod1 & 0xff);
		if (format.srcB_mod == 1)
			uop->setSourceRegisterIndex(2,format.srcB & 0x1ff);
		uop->setDestinationRegisterIndex(0, format.dst);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		instruction_format = FormatInteger2;
		break;
	}

	case (Opcode)(INST_ISAD_A):
	{
		BytesISAD format = bytes.isad;
		uop->setSourceRegisterIndex(0, format.src1);
		uop->setSourceRegisterIndex(1, format.src3);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatInteger3;
		break;
	}

	case (Opcode)(INST_ISAD_B):
	{
		BytesISAD format = bytes.isad;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.op2 == 3)
		{
			uop->setSourceRegisterIndex(1, format.src2);
			uop->setSourceRegisterIndex(2, format.src3);
		}
		else
			uop->setSourceRegisterIndex(1, format.src3);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatInteger3;
		break;
	}

	case (Opcode)(INST_BFI_A):
	{
		BytesBFI format = bytes.bfi;
		uop->setSourceRegisterIndex(0, format.src1);
		uop->setSourceRegisterIndex(0, format.src3);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatInteger3;
		break;
	}

	case (Opcode)(INST_BFI_B):
	{
		break;
	}

	case (Opcode)(INST_BFE_A):
	{
		BytesBFE format = bytes.bfe;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatInteger3;
		break;
	}

	case (Opcode)(INST_BFE_B):
	{
		BytesBFE format = bytes.bfe;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.op2 == 3)
			uop->setSourceRegisterIndex(1, format.src2);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatInteger3;
		break;
	}

	case (Opcode)(INST_IMADSP_A):
	{
		break;
	}

	case (Opcode)(INST_IMADSP_B):
	{
		break;
	}

	case (Opcode)(INST_IADD32I):
	{
		BytesIADD32I format = bytes.iadd32i;
		uop->setSourceRegisterIndex(0, format.src);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatInteger1;
		break;
	}

	case (Opcode)(INST_ISET_A):
	{
		break;
	}

	case (Opcode)(INST_ISET_B):
	{
		BytesISET format = bytes.iset;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.op2 == 3)
			uop->setSourceRegisterIndex(1, format.src2);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatInteger1;
		break;
	}

	case (Opcode)(INST_LOP_A):
	{
		BytesLOP format = bytes.lop;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatInteger1;
		break;
	}

	case (Opcode)(INST_LOP_B):
	{
		BytesLOP format = bytes.lop;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.op2 == 3)
			uop->setSourceRegisterIndex(1, format.src2);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatInteger1;
		break;
	}

	case (Opcode)(INST_LOP32I):
	{
		BytesLOP32I format = bytes.lop32i;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatInteger1;
		break;
	}

	case (Opcode)(INST_ICMP_A):
	{
		break;
	}

	case (Opcode)(INST_ICMP_B):
	{
		BytesICMP format = bytes.icmp;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.op2 == 3)
		{
			uop->setSourceRegisterIndex(1, format.src2);
			uop->setSourceRegisterIndex(2, format.src3);
		}
		else
			uop->setSourceRegisterIndex(1, format.src3);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatInteger3;
		break;
	}

	case (Opcode)(INST_MOV32I):
	{
		BytesImm format = bytes.immediate;
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatBit1;
		break;
	}

	case (Opcode)(INST_SEL_A):
	{
		BytesSEL format = bytes.sel;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		if (format.pred_src != 7)
			uop->setSourcePredicateIndex(0, format.pred_src);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatBit2;
		break;
	}

	case (Opcode)(INST_SEL_B):
	{
		BytesSEL format = bytes.sel;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.op2 == 3)
			uop->setSourceRegisterIndex(1, format.src2);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		if (format.pred_src != 7)
			uop->setSourcePredicateIndex(0, format.pred_src);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatBit2;
		break;
	}

	case (Opcode)(INST_I2F_A):
	{
		BytesI2F format = bytes.i2f;
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatConversion;
		break;
	}

	case (Opcode)(INST_I2F_B):
	{
		BytesI2F format = bytes.i2f;
		if (format.op2 == 3)
			uop->setSourceRegisterIndex(0, format.src);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatConversion;
		break;
	}

	case (Opcode)(INST_I2I_A):
	{
		BytesI2I format = bytes.i2i;
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatConversion;
		break;
	}

	case (Opcode)(INST_I2I_B):
	{
		BytesI2I format = bytes.i2i;
		if (format.op2 == 3)
			uop->setSourceRegisterIndex(0, format.src);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatConversion;
		break;
	}

	case (Opcode)(INST_F2I_A):
	{
		BytesF2I format = bytes.f2i;
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatConversion;
		break;
	}

	case (Opcode)(INST_F2I_B):
	{
		BytesF2I format = bytes.f2i;
		if (format.op2 == 3)
			uop->setSourceRegisterIndex(0, format.src);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatConversion;
		break;
	}

	case (Opcode)(INST_F2F_A):
	{
		break;
	}

	case (Opcode)(INST_F2F_B):
	{
		BytesF2F format = bytes.f2f;
		if (format.op2 == 3)
			uop->setSourceRegisterIndex(0, format.src);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatConversion;
		break;
	}

	case (Opcode)(INST_LDS):
	{
		break;
	}

	case (Opcode)(INST_LDC):
	{
		BytesLDC format = bytes.ldc;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		uop->memory_read = true;
		instruction_format = FormatLS;
		break;
	}

	case (Opcode)(INST_ST):
	{
		BytesGeneral0 format = bytes.general0;
		uop->setSourceRegisterIndex(0, format.dst);
		uop->setDestinationPredicateIndex(0, format.mod0);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);

		// Set uop memory write flag
		uop->memory_write = true;
		instruction_format = FormatLS;
		break;
	}

	case (Opcode)(INST_STS):
	{
		break;
	}

	case (Opcode)(INST_DADD):
	{
		break;
	}

	case (Opcode)(INST_FMUL):
	{
		BytesGeneral0 format = bytes.general0;
		uop->setSourceRegisterIndex(0,format.mod0);
		if (format.srcB_mod == 1 && format.op0 == 2)
			uop->setSourceRegisterIndex(1, format.srcB);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatFP32_1;
		break;
	}

	case (Opcode)(INST_FADD_A):
	{
		instruction_format = FormatFP32_1;
		break;
	}

	case (Opcode)(INST_FADD_B):
	{
		BytesFADD format = bytes.fadd;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.op2 == 3)
			uop->setSourceRegisterIndex(1, format.src2);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatFP32_1;
		break;
	}

	case (Opcode)(INST_MUFU):
	{
		BytesMUFU format = bytes.mufu;
		uop->setSourceRegisterIndex(0, format.src);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatSFU;
		break;
	}

	case (Opcode)(INST_FFMA_A):
	{
		break;
	}

	case (Opcode)(INST_FFMA_B):
	{
		BytesFFMA format = bytes.ffma;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.op2 == 3)
		{
			uop->setSourceRegisterIndex(1, format.src2);
			uop->setSourceRegisterIndex(2, format.src3);
		}
		else
			uop->setSourceRegisterIndex(1, format.src3);
		if (format.pred != 7)
			uop->setSourcePredicateIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatFP32_1;
		break;
	}

	case (Opcode)(INST_FSET_A):
	{
		break;
	}

	case (Opcode)(INST_FSET_B):
	{
		BytesFSET format = bytes.fset;
		uop->setSourceRegisterIndex(0, format.src1);
		if (format.op2 == 3)
			uop->setSourceRegisterIndex(1, format.src2);
		if (format.pred != 7)
			uop->setDestinationRegisterIndex(0, format.pred);
		uop->setDestinationRegisterIndex(0, format.dst);
		instruction_format = FormatFP32_1;
		break;
	}

	case (Opcode)(INST_FFMA32I):
	{
		break;
	}

	case (Opcode)(INST_NOP):
	{
		break;
	}

	case (Opcode)(INST_SHF):
	{
		break;
	}

	case (Opcode)(INST_BAR):
	{
		BytesGeneral0 format = bytes.general0;
		if (format.pred != 7)
			uop->setDestinationRegisterIndex(0, format.pred);
		// For now set invalid format
		instruction_format = FormatInvalid;
		break;
	}

	case (Opcode)(INST_BPT):
	{
		break;
	}

	case (Opcode)(INST_JMX):
	{
		break;
	}

	case (Opcode)(INST_JMP):
	{
		break;
	}

	case (Opcode)(INST_JCAL):
	{
		break;
	}

	case (Opcode)(INST_BRX):
	{
		break;
	}

	case (Opcode)(INST_CAL):
	{
		instruction_format = FormatControl;
		break;
	}

	case (Opcode)(INST_PRET):
	{
		break;
	}

	case (Opcode)(INST_PLONGJMP):
	{
		break;
	}

	case (Opcode)(INST_SSY):
	{
		instruction_format = FormatControl;
		break;
	}

	case (Opcode)(INST_PBK):
	{
		instruction_format = FormatControl;
		break;
	}

	case (Opcode)(INST_PCNT):
	{
		break;
	}

	case (Opcode)(INST_GETCRSPTR):
	{
		break;
	}

	case (Opcode)(INST_GETLMEMBASE):
	{
		break;
	}

	case (Opcode)(INST_SETCRSPTR):
	{
		break;
	}

	case (Opcode)(INST_SETLMEMBASE):
	{
		break;
	}

	case (Opcode)(INST_LONGJMP):
	{
		break;
	}

	case (Opcode)(INST_RET):
	{
		instruction_format = FormatControl;
		break;
	}

	case (Opcode)(INST_KIL):
	{
		break;
	}

	case (Opcode)(INST_BRK):
	{
		instruction_format = FormatControl;
		break;
	}

	case (Opcode)(INST_CONT):
	{
		break;
	}

	case (Opcode)(INST_RTT):
	{
		break;
	}

	case (Opcode)(INST_SAM):
	{
		break;
	}

	case (Opcode)(INST_RAM):
	{
		break;
	}

	case (Opcode)(INST_IDE):
	{
		break;
	}

	case (Opcode)(INST_FADD32I):
	{
		break;
	}

	case (Opcode)(INST_IMAD32I):
	{
		break;
	}

	case (Opcode)(INST_ISCADD32I):
	{
		break;
	}

	case (Opcode)(OpcodeCount):
	{
		break;
	}
	}
}

void Instruction::DumpHex(std::ostream &os) const
{
	os << misc::fmt("\n\t/*%04x*/     /*0x%08x%08x*/ \t",
			address, bytes.as_uint[0], bytes.as_uint[1]);
}


void Instruction::DumpPredShort(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	if (value == 7)
		os << "PT";
	else
		os << "P" << (value & 7);
}


void Instruction::DumpPredNoat(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	if (value != 7)
	{
		if (value >> 3)
			os << '!';
		if (value == 15)
			os << "PT";
		else
			os << 'P' << (value & 7);
	}
	else
		os << "PT";
}


void Instruction::DumpPred(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	if (value != 7)
	{
		os << '@';
		if (value >> 3)
			os << '!';
		if (value == 15)
			os << "PT ";
		else
			os << 'P' << (value & 7) << ' ';
	}
}


void Instruction::DumpReg(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	if (value == 255)
		os << "RZ";
	else
		os << 'R' << value;
}


void Instruction::DumpSpecReg(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	if (value == 61)
		os << "SR_RegAlloc";
	else if (value == 62)
		os << "SR_CtxAddr";
	else
		os << "SR" << value;
}


void Instruction::DumpS(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_s_map.MapValue(value);
}


void Instruction::DumpF(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_f_map.MapValue(value);
}


void Instruction::DumpAnd(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_and_map.MapValue(value);
}

void Instruction::DumpU8(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_u8_map.MapValue(value);
}

void Instruction::DumpX(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_x_map.MapValue(value);
}


void Instruction::DumpU32(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_u32_map.MapValue(value);
}


void Instruction::DumpHi(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_hi_map.MapValue(value);
}


void Instruction::DumpSat(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_sat_map.MapValue(value);
}


void Instruction::DumpPo(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_po_map.MapValue(value);
}


void Instruction::DumpUs(std::ostream &os, int high0, int low0, int high1,
		int low1) const
{
	int value0 = misc::getBits64(bytes.as_dword, high0, low0);
	int value1 = misc::getBits64(bytes.as_dword, high1, low1);

	if (value0 == 1 && value1 == 1)
	{
	}
	else
	{
		os << inst_us_map.MapValue(value1);
		os << inst_us_map.MapValue(value0);
	}
}


void Instruction::DumpCc(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_cc_map.MapValue(value);
}


void Instruction::DumpE(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_e_map.MapValue(value);
}


void Instruction::DumpCv(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_cv_map.MapValue(value);
}


void Instruction::DumpLmt(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_lmt_map.MapValue(value);
}


void Instruction::DumpU(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_u_map.MapValue(value);
}


void Instruction::DumpRm(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_rm_map.MapValue(value);
}


void Instruction::DumpKeepRefCount(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_keeprefcount_map.MapValue(value);
}


void Instruction::DumpCc2(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << inst_cc2_map.MapValue(value);
}

void Instruction::DumpSRCB(std::ostream &os, int high0, int low0, int high1, int low1,
		int high2, int low2, int high3, int low3) const
{
	int value0 = misc::getBits64(bytes.as_dword, high0, low0);
	int value1 = misc::getBits64(bytes.as_dword, high1, low1);
	int value2 = misc::getBits64(bytes.as_dword, high2, low2);
	int value3 = misc::getBits64(bytes.as_dword, high3, low3);
	int value4 = misc::getBits64(bytes.as_dword, high3 - 1, low3);
	long long valueConst = 4 * (value2 * 1000000000ll + value3);

	if (value0 == 0)
		os << misc::fmt("c [0x%x] [0x%llx]", value1,
				valueConst);
	else if (value0 == 1)
	{
		if (value4 == 255)
			os << "RZ";
		else
			os << 'R' << value4;
	}
}

void Instruction::DumpEndConst(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	os << misc::fmt("0x%x", value);
}

void Instruction::DumpOffset(std::ostream &os, int high, int low) const
{
	int value = misc::getBits64(bytes.as_dword, high, low);
	if (value)
		os << misc::fmt(" 0x%x", value);
}

void Instruction::DumpTarget(std::ostream &os, int high0, int low0, int high1,
		int low1) const
{
	int value0 = misc::getBits64(bytes.as_dword, high0, low0);
	int value1 = misc::getBits64(bytes.as_dword, high1, low1);
	int value2 = 0xfff00000 | value0;

	if (value1 == 0)
		os << misc::fmt(" 0x%x", value0 + address + 8);
	else if (value1 == 1)
		os << misc::fmt(" 0x%x",  address + value2 + 8);
}

void Instruction::Dump(std::ostream &os) const
{
	// Invalid instruction
	if (!info || !info->fmt_str)
	{
		os << "<unknown>";
		return;
	}

	// Print entire format string temporarily
	const char *fmt_str = info->fmt_str;
	while (*fmt_str)
	{
		// Literal value
		if (*fmt_str != '%')
		{
			os << *fmt_str;
			fmt_str++;
			continue;
		}

		// Tokens
		int length = 0;
		fmt_str++;
		if (comm::Disassembler::isToken(fmt_str, "tgt", length))
		{
			DumpTarget(os, 46, 23, 46, 46);
		}
		else if (comm::Disassembler::isToken(fmt_str, "offset", length))
		{
			DumpOffset(os, 53, 23);
		}
		else if (comm::Disassembler::isToken(fmt_str, "offset2", length))
		{
			DumpOffset(os, 34, 23);
		}
		else if (comm::Disassembler::isToken(fmt_str, "const", length))
		{
			DumpEndConst(os, 45, 42);
		}
		else if (comm::Disassembler::isToken(fmt_str, "srcC", length))
		{
			DumpReg(os, 49, 42);
		}
		else if (comm::Disassembler::isToken(fmt_str, "srcB2", length))
		{
			DumpReg(os, 31, 23);
		}
		else if (comm::Disassembler::isToken(fmt_str, "imm32", length))
		{
			DumpEndConst(os, 54, 23);
		}
		else if (comm::Disassembler::isToken(fmt_str, "srcB", length))
		{
			DumpSRCB(os, 63, 63, 41, 37, 36, 32, 31, 23);
		}
		else if (comm::Disassembler::isToken(fmt_str, "src_spec", length))
		{
			DumpSpecReg(os, 30, 23);
		}
		else if (comm::Disassembler::isToken(fmt_str, "cc_dst", length))
		{
			DumpCc2(os, 6, 2);
		}
		else if (comm::Disassembler::isToken(fmt_str, "pred1", length))
		{
			DumpPredShort(os, 7, 5);
		}
		else if (comm::Disassembler::isToken(fmt_str, "pred2", length))
		{
			DumpPredShort(os, 4, 2);
		}
		else if (comm::Disassembler::isToken(fmt_str, "pred3", length))
		{
			DumpPredNoat(os, 45, 42);
		}
		else if (comm::Disassembler::isToken(fmt_str, "pred4", length))
		{
			DumpPredNoat(os, 17, 14);
		}
		else if (comm::Disassembler::isToken(fmt_str, "pred5", length))
		{
			DumpPredNoat(os, 35, 32);
		}
		else if (comm::Disassembler::isToken(fmt_str, "keeprefcount", length))
		{
			DumpKeepRefCount(os, 7, 7);
		}
		else if (comm::Disassembler::isToken(fmt_str, "rm", length))
		{
			DumpRm(os, 43, 42);
		}
		else if (comm::Disassembler::isToken(fmt_str, "LMT", length))
		{
			DumpLmt(os, 8, 8);
		}
		else if (comm::Disassembler::isToken(fmt_str, "cg", length))
		{
			DumpCv(os, 60, 59);
		}
		else if (comm::Disassembler::isToken(fmt_str, "e", length))
		{
			DumpE(os, 55, 55);
		}
		else if (comm::Disassembler::isToken(fmt_str, "cc", length))
		{
			DumpCc(os, 50, 50);
		}
		else if (comm::Disassembler::isToken(fmt_str, "us", length))
		{
			DumpUs(os, 56, 56, 51, 51);
		}
		else if (comm::Disassembler::isToken(fmt_str, "po", length))
		{
			DumpPo(os, 55, 55);
		}
		else if (comm::Disassembler::isToken(fmt_str, "sat", length))
		{
			DumpSat(os, 53, 53);
		}
		else if (comm::Disassembler::isToken(fmt_str, "hi", length))
		{
			DumpHi(os, 57, 57);
		}
		else if (comm::Disassembler::isToken(fmt_str, "u8", length))
		{
			DumpU8(os, 58, 56);
		}
		else if (comm::Disassembler::isToken(fmt_str, "U32", length))
		{
			DumpU32(os, 51, 51);
		}
		else if (comm::Disassembler::isToken(fmt_str, "x", length))
		{
			DumpX(os, 46, 46);
		}
		else if (comm::Disassembler::isToken(fmt_str, "and", length))
		{
			DumpAnd(os, 49, 48);
		}
		else if (comm::Disassembler::isToken(fmt_str, "f", length))
		{
			DumpF(os, 54, 52);
		}
		else if (comm::Disassembler::isToken(fmt_str, "s", length))
		{
			DumpS(os, 22, 22);
		}
		else if (comm::Disassembler::isToken(fmt_str, "u", length))
		{
			DumpU(os, 9, 9);
		}
		else if (comm::Disassembler::isToken(fmt_str, "srcA", length))
		{
			DumpReg(os, 17, 10);
		}
		else if (comm::Disassembler::isToken(fmt_str, "dst", length))
		{
			DumpReg(os, 9, 2);
		}
		else if (comm::Disassembler::isToken(fmt_str, "pred0", length))
		{
			DumpPred(os, 21, 18);
			assert(fmt_str[length] == ' ');
			length++;
		}
		else if (comm::Disassembler::isToken(fmt_str, "pred", length))
		{
			DumpPred(os, 21, 18);
			assert(fmt_str[length] == ' ');
			length++;
		}
		else
		{
			throw misc::Panic(misc::fmt("%s: Unrecognized token",
				fmt_str));
		}

		// Advance format string
		fmt_str += length;
	}
}


}  // namespace Kepler

