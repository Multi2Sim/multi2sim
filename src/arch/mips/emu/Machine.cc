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
#include <lib/util/misc.h>
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
}

void Context::ExecuteInst_JAL()
{
}

void Context::ExecuteInst_BEQ()
{
}

void Context::ExecuteInst_BNE()
{
}

void Context::ExecuteInst_BLEZ()
{
}

void Context::ExecuteInst_BGTZ()
{
}

void Context::ExecuteInst_ADDI()
{
}

void Context::ExecuteInst_ADDIU()
{
	unsigned int IMM = inst.GetBytes()->offset_imm.offset;
	unsigned int RS = inst.GetBytes()->standard.rs;
	unsigned int RT = inst.GetBytes()->standard.rt;
	regs.setGPR(RT,regs.getGPR(RS)+SEXT32(IMM,16));
}

void Context::ExecuteInst_SLTI()
{
}

void Context::ExecuteInst_SLTIU()
{
}

void Context::ExecuteInst_ANDI()
{
}

void Context::ExecuteInst_ORI()
{
}

void Context::ExecuteInst_XORI()
{
}

void Context::ExecuteInst_LUI()
{
	unsigned int IMM = inst.GetBytes()->offset_imm.offset;
	unsigned int rt =inst.GetBytes()->standard.rt;
	regs.setGPR(rt,(unsigned int)(IMM << 16));
}

void Context::ExecuteInst_BEQL()
{
}

void Context::ExecuteInst_BNEL()
{
}

void Context::ExecuteInst_BLEZL()
{
}

void Context::ExecuteInst_BGTZL()
{
}

void Context::ExecuteInst_LB()
{
}

void Context::ExecuteInst_LH()
{
}

void Context::ExecuteInst_LWL()
{
}

void Context::ExecuteInst_LW()
{
	char temp;
	unsigned int rs = inst.GetBytes()->standard.rs;
	unsigned int rt = inst.GetBytes()->standard.rt;
	unsigned int IMM = inst.GetBytes()->offset_imm.offset;
	unsigned int addr = regs.getGPR(rs) + SEXT32((signed)IMM,16);
	if ((BITS32(addr, 1, 0) | 0) == 1 )
		std::cout<<"LW: address error, effective address must be naturallty-aligned\n";
	memory->Read(addr, 4, &temp);
	regs.setGPR(rt, (int)temp);
}

void Context::ExecuteInst_LBU()
{
}

void Context::ExecuteInst_LHU()
{
}

void Context::ExecuteInst_LWR()
{
}

void Context::ExecuteInst_SB()
{
}

void Context::ExecuteInst_SH()
{
}

void Context::ExecuteInst_SWL()
{
}

void Context::ExecuteInst_SW()
{
}

void Context::ExecuteInst_SWR()
{
}

void Context::ExecuteInst_CACHE()
{
}

void Context::ExecuteInst_LL()
{
}

void Context::ExecuteInst_LWC1()
{
}

void Context::ExecuteInst_LWC2()
{
}

void Context::ExecuteInst_PREF()
{
}

void Context::ExecuteInst_LDC1()
{
}

void Context::ExecuteInst_LDC2()
{
}

void Context::ExecuteInst_SC()
{
}

void Context::ExecuteInst_SWC1()
{
}

void Context::ExecuteInst_SWC2()
{
}

void Context::ExecuteInst_SDC1()
{
}

void Context::ExecuteInst_SDC2()
{
}

void Context::ExecuteInst_SLL()
{

}

void Context::ExecuteInst_MOVF()
{
}

void Context::ExecuteInst_MOVT()
{
}

void Context::ExecuteInst_SRL()
{
}

void Context::ExecuteInst_ROR()
{
}

void Context::ExecuteInst_SRA()
{
}

void Context::ExecuteInst_SLLV()
{
}

void Context::ExecuteInst_SRLV()
{
}

void Context::ExecuteInst_ROTRV()
{
}

void Context::ExecuteInst_SRAV()
{
}

void Context::ExecuteInst_JR()
{
}

void Context::ExecuteInst_JALR()
{
}

void Context::ExecuteInst_MOVZ()
{
}

void Context::ExecuteInst_MOVN()
{
}

void Context::ExecuteInst_SYSCALL()
{
	ExecuteSyscall();
}

void Context::ExecuteInst_BREAK()
{
}

void Context::ExecuteInst_SYNC()
{
}

void Context::ExecuteInst_MFHI()
{
}

void Context::ExecuteInst_MTHI()
{
}

void Context::ExecuteInst_MFLO()
{
}

void Context::ExecuteInst_MTLO()
{
}

void Context::ExecuteInst_MULT()
{
}

void Context::ExecuteInst_MULTU()
{
}

void Context::ExecuteInst_DIV()
{
}

void Context::ExecuteInst_DIVU()
{
}

void Context::ExecuteInst_ADD()
{
}

void Context::ExecuteInst_ADDU()
{
	unsigned int rt = inst.GetBytes()->standard.rt;
	unsigned int rs = inst.GetBytes()->standard.rs;
	unsigned int rd = inst.GetBytes()->standard.rd;
	unsigned int value = regs.getGPR(rt) + regs.getGPR(rs);
	regs.setGPR(rd, value);
}

void Context::ExecuteInst_SUB()
{
}

void Context::ExecuteInst_SUBU()
{
}

void Context::ExecuteInst_AND()
{
}

void Context::ExecuteInst_OR()
{
}

void Context::ExecuteInst_XOR()
{
}

void Context::ExecuteInst_NOR()
{
}

void Context::ExecuteInst_SLT()
{
}

void Context::ExecuteInst_SLTU()
{
}

void Context::ExecuteInst_TGE()
{
}

void Context::ExecuteInst_TGEU()
{
}

void Context::ExecuteInst_TLT()
{
}

void Context::ExecuteInst_TLTU()
{
}

void Context::ExecuteInst_TEQ()
{
}

void Context::ExecuteInst_TNE()
{
}

void Context::ExecuteInst_BLTZ()
{
}

void Context::ExecuteInst_BGEZ()
{
}

void Context::ExecuteInst_BLTZL()
{
}

void Context::ExecuteInst_BGEZL()
{
}

void Context::ExecuteInst_TGEI()
{
}

void Context::ExecuteInst_TGEIU()
{
}

void Context::ExecuteInst_TLTI()
{
}

void Context::ExecuteInst_TLTIU()
{
}

void Context::ExecuteInst_TEQI()
{
}

void Context::ExecuteInst_TNEI()
{
}

void Context::ExecuteInst_BLTZAL()
{
}

void Context::ExecuteInst_BGEZAL()
{
}

void Context::ExecuteInst_BLTZALL()
{
}

void Context::ExecuteInst_BGEZALL()
{
}

void Context::ExecuteInst_SYNCI()
{
}

void Context::ExecuteInst_MADD()
{
}

void Context::ExecuteInst_MADDU()
{
}

void Context::ExecuteInst_MUL()
{
}

void Context::ExecuteInst_MSUB()
{
}

void Context::ExecuteInst_MSUBU()
{
}

void Context::ExecuteInst_CLZ()
{
}

void Context::ExecuteInst_CLO()
{
}








void Context::ExecuteInst_EXT()
{
}

void Context::ExecuteInst_INS()
{
}

void Context::ExecuteInst_WSBH()
{
}

void Context::ExecuteInst_SEB()
{
}

void Context::ExecuteInst_SEH()
{
}

void Context::ExecuteInst_RDHWR()
{
}

void Context::ExecuteInst_MFCO()
{
}

void Context::ExecuteInst_MTCO()
{
}

void Context::ExecuteInst_RDPGPR()
{
}

void Context::ExecuteInst_DI()
{
}

void Context::ExecuteInst_EI()
{
}

void Context::ExecuteInst_WRPGPR()
{
}

void Context::ExecuteInst_TLBR()
{
}

void Context::ExecuteInst_TLBWI()
{
}

void Context::ExecuteInst_TLBWR()
{
}

void Context::ExecuteInst_TLBP()
{
}

void Context::ExecuteInst_ERET()
{
}

void Context::ExecuteInst_DERET()
{
}

void Context::ExecuteInst_WAIT()
{
}





void Context::ExecuteInst_MFC1()
{
}

void Context::ExecuteInst_CFC1()
{
}

void Context::ExecuteInst_MFHC1()
{
}

void Context::ExecuteInst_MTC1()
{
}

void Context::ExecuteInst_CTC1()
{
}

void Context::ExecuteInst_MTHC1()
{
}

void Context::ExecuteInst_BC1F()
{
}

void Context::ExecuteInst_BC1FL()
{
}

void Context::ExecuteInst_BC1T()
{
}

void Context::ExecuteInst_BC1TL()
{
}

void Context::ExecuteInst_ADD_S()
{
}

void Context::ExecuteInst_SUB_S()
{
}

void Context::ExecuteInst_MUL_S()
{
}

void Context::ExecuteInst_DIV_S()
{
}

void Context::ExecuteInst_SQRT_S()
{
}

void Context::ExecuteInst_ABS_S()
{
}

void Context::ExecuteInst_MOV_S()
{
}

void Context::ExecuteInst_NEG_S()
{
}

void Context::ExecuteInst_ROUND_L_S()
{
}

void Context::ExecuteInst_TRUNC_L_S()
{
}

void Context::ExecuteInst_CEIL_L_S()
{
}

void Context::ExecuteInst_FLOOR_L_S()
{
}

void Context::ExecuteInst_ROUND_W_S()
{
}

void Context::ExecuteInst_TRUNC_W_S()
{
}

void Context::ExecuteInst_CEIL_W_S()
{
}

void Context::ExecuteInst_FLOOR_W_S()
{
}

void Context::ExecuteInst_MOVF_S()
{
}

void Context::ExecuteInst_MOVT_S()
{
}

void Context::ExecuteInst_MOVZ_S()
{
}

void Context::ExecuteInst_MOVN_S()
{
}

void Context::ExecuteInst_RECIP_S()
{
}

void Context::ExecuteInst_RSQRT_S()
{
}

void Context::ExecuteInst_CVT_D_S()
{
}

void Context::ExecuteInst_CVT_W_S()
{
}

void Context::ExecuteInst_CVT_L_S()
{
}

void Context::ExecuteInst_CVT_PS_S()
{
}

void Context::ExecuteInst_ADD_D()
{
}

void Context::ExecuteInst_SUB_D()
{
}

void Context::ExecuteInst_MUL_D()
{
}

void Context::ExecuteInst_DIV_D()
{
}

void Context::ExecuteInst_SQRT_D()
{
}

void Context::ExecuteInst_ABS_D()
{
}

void Context::ExecuteInst_MOV_D()
{
}

void Context::ExecuteInst_NEG_D()
{
}

void Context::ExecuteInst_ROUND_L_D()
{
}

void Context::ExecuteInst_TRUNC_L_D()
{
}

void Context::ExecuteInst_CEIL_L_D()
{
}

void Context::ExecuteInst_FLOOR_L_D()
{
}

void Context::ExecuteInst_ROUND_W_D()
{
}

void Context::ExecuteInst_TRUNC_W_D()
{
}

void Context::ExecuteInst_CEIL_W_D()
{
}

void Context::ExecuteInst_FLOOR_W_D()
{
}

void Context::ExecuteInst_MOVF_D()
{
}

void Context::ExecuteInst_MOVT_D()
{
}

void Context::ExecuteInst_MOVZ_D()
{
}

void Context::ExecuteInst_MOVN_D()
{
}

void Context::ExecuteInst_RECIP_D()
{
}

void Context::ExecuteInst_RSQRT_D()
{
}

void Context::ExecuteInst_CVT_S_D()
{
}

void Context::ExecuteInst_CVT_W_D()
{
}

void Context::ExecuteInst_CVT_L_D()
{
}

void Context::ExecuteInst_C_F_D()
{
}

void Context::ExecuteInst_C_UN_D()
{
}

void Context::ExecuteInst_C_EQ_D()
{
}

void Context::ExecuteInst_C_UEQ_D()
{
}

void Context::ExecuteInst_C_OLT_D()
{
}

void Context::ExecuteInst_C_ULT_D()
{
}

void Context::ExecuteInst_C_OLE_D()
{
}

void Context::ExecuteInst_C_ULE_D()
{
}

void Context::ExecuteInst_C_SF_D()
{
}

void Context::ExecuteInst_C_NGLE_D()
{
}

void Context::ExecuteInst_C_SEQ_D()
{
}

void Context::ExecuteInst_C_NGL_D()
{
}

void Context::ExecuteInst_C_LT_D()
{
}

void Context::ExecuteInst_C_NGE_D()
{
}

void Context::ExecuteInst_C_LE_D()
{
}

void Context::ExecuteInst_C_NGT_D()
{
}

void Context::ExecuteInst_CVT_S_W()
{
}

void Context::ExecuteInst_CVT_D_W()
{
}

void Context::ExecuteInst_ADD_PS()
{
}

void Context::ExecuteInst_SUB_PS()
{
}

void Context::ExecuteInst_MUL_PS()
{
}

void Context::ExecuteInst_ABS_PS()
{
}

void Context::ExecuteInst_MOV_PS()
{
}

void Context::ExecuteInst_NEG_PS()
{
}

void Context::ExecuteInst_MOVF_PS()
{
}

void Context::ExecuteInst_MOVT_PS()
{
}

void Context::ExecuteInst_MOVZ_PS()
{
}

void Context::ExecuteInst_MOVN_PS()
{
}

void Context::ExecuteInst_CVT_S_PU()
{
}

void Context::ExecuteInst_CVT_S_PL()
{
}

void Context::ExecuteInst_PLL_PS()
{
}

void Context::ExecuteInst_PLU_PS()
{
}

void Context::ExecuteInst_PUL_PS()
{
}

void Context::ExecuteInst_PUU_PS()
{
}

void Context::ExecuteInst_MFC2()
{
}

void Context::ExecuteInst_CFC2()
{
}

void Context::ExecuteInst_MFHC2()
{
}

void Context::ExecuteInst_MTC2()
{
}

void Context::ExecuteInst_CTC2()
{
}

void Context::ExecuteInst_MTHC2()
{
}

void Context::ExecuteInst_BC2F()
{
}

void Context::ExecuteInst_BC2FL()
{
}

void Context::ExecuteInst_BC2T()
{
}

void Context::ExecuteInst_BC2TL()
{
}

void Context::ExecuteInst_COP2()
{
}
}
