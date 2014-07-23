/*
 *  Multi2Sim
 *  Copyright (C) 2014  Sida Gu (gu.sid@husky.neu.edu)
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
#include <lib/cpp/Misc.h>
#include <memory/Memory.h>

#include "Context.h"
#include "Emu.h"
#include "Regs.h"

namespace MIPS
{
Context::ExecuteInstFn Context::execute_inst_fn[InstOpcodeCount] =
{
	nullptr  // For InstOpcodeNone
#define DEFINST(name, _fmt_str, _op0, _op1, _op2, _op3) \
		 , &Context::ExecuteInst_##name
#include <arch/mips/asm/Inst.def>
#undef DEFINST
};


void Context::ExecuteInst_J()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_JAL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BEQ()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BNE()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BLEZ()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BGTZ()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ADDI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ADDIU()
{
	// Read Operands
	unsigned int IMM = inst.getBytes()->offset_imm.offset;
	unsigned int RS = inst.getBytes()->standard.rs;
	unsigned int RT = inst.getBytes()->standard.rt;

	// Perform Operation
	regs.setGPR(RT,regs.getGPR(RS) + misc::SignExtend32(IMM,16));
}


void Context::ExecuteInst_SLTI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SLTIU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ANDI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ORI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_XORI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LUI()
{
	// Read Operands
	unsigned int IMM = inst.getBytes()->offset_imm.offset;
	unsigned int rt =inst.getBytes()->standard.rt;

	// Perform Operation
	regs.setGPR(rt,(unsigned int)(IMM << 16));
}


void Context::ExecuteInst_BEQL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BNEL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BLEZL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BGTZL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LB()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LH()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LWL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LW()
{
	// read operands from instruction
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int IMM = inst.getBytes()->offset_imm.offset;

	// Perform operation LW
	unsigned int addr = regs.getGPR(rs) + misc::SignExtend32((signed)IMM,16);
	char temp;
	if ((misc::getBits32(addr, 1, 0) | 0) == 1 )
		misc::panic("LW: address error, effective address must be naturallty-aligned\n");
	memory->Read(addr, 4, &temp);
	regs.setGPR(rt, (int)temp);
}


void Context::ExecuteInst_LBU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LHU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LWR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SB()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SH()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SWL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SW()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SWR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CACHE()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LWC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LWC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_PREF()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LDC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LDC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SC()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SWC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SWC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SDC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SDC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SLL()
{

}


void Context::ExecuteInst_MOVF()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVT()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SRL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ROR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SRA()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SLLV()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SRLV()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ROTRV()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SRAV()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_JR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_JALR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVZ()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVN()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SYSCALL()
{
	ExecuteSyscall();
}


void Context::ExecuteInst_BREAK()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SYNC()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MFHI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MTHI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MFLO()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MTLO()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MULT()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MULTU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_DIV()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_DIVU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ADD()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ADDU()
{
	// read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rd = inst.getBytes()->standard.rd;

	// perform operation ADDU
	unsigned int value = regs.getGPR(rt) + regs.getGPR(rs);
	regs.setGPR(rd, value);
}


void Context::ExecuteInst_SUB()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SUBU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_AND()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_OR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_XOR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_NOR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SLT()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SLTU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TGE()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TGEU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLT()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLTU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TEQ()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TNE()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BLTZ()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BGEZ()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BLTZL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BGEZL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TGEI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TGEIU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLTI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLTIU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TEQI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TNEI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BLTZAL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BGEZAL()
{
	unsigned int pc = regs.getPC();
	std::cout<<"pc = "<<pc;
}


void Context::ExecuteInst_BLTZALL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BGEZALL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SYNCI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MADD()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MADDU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MUL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MSUB()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MSUBU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CLZ()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CLO()
{
	throw misc::Panic("Unimplemented instruction");
}









void Context::ExecuteInst_EXT()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_INS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_WSBH()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SEB()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SEH()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_RDHWR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MFCO()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MTCO()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_RDPGPR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_DI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_EI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_WRPGPR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLBR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLBWI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLBWR()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TLBP()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ERET()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_DERET()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_WAIT()
{
	throw misc::Panic("Unimplemented instruction");
}






void Context::ExecuteInst_MFC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CFC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MFHC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MTC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CTC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MTHC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC1F()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC1FL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC1T()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC1TL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ADD_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SUB_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MUL_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_DIV_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SQRT_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ABS_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOV_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_NEG_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ROUND_L_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TRUNC_L_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CEIL_L_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_FLOOR_L_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ROUND_W_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TRUNC_W_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CEIL_W_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_FLOOR_W_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVF_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVT_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVZ_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVN_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_RECIP_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_RSQRT_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_D_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_W_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_L_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_PS_S()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ADD_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SUB_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MUL_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_DIV_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SQRT_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ABS_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOV_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_NEG_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ROUND_L_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TRUNC_L_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CEIL_L_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_FLOOR_L_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ROUND_W_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_TRUNC_W_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CEIL_W_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_FLOOR_W_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVF_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVT_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVZ_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVN_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_RECIP_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_RSQRT_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_S_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_W_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_L_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_F_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_UN_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_EQ_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_UEQ_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_OLT_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_ULT_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_OLE_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_ULE_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_SF_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_NGLE_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_SEQ_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_NGL_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_LT_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_NGE_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_LE_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_C_NGT_D()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_S_W()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_D_W()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ADD_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SUB_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MUL_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_ABS_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOV_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_NEG_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVF_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVT_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVZ_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVN_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_S_PU()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CVT_S_PL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_PLL_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_PLU_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_PUL_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_PUU_PS()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MFC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CFC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MFHC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MTC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_CTC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MTHC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC2F()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC2FL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC2T()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BC2TL()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_COP2()
{
	throw misc::Panic("Unimplemented instruction");
}
}
