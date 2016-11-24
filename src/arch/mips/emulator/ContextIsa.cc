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
#include "Emulator.h"
#include "Regs.h"


namespace MIPS

{
Context::ExecuteInstFn Context::execute_inst_fn[Instruction::OpcodeCount] =
{
	nullptr  // For InstOpcodeNone
#define DEFINST(name, _fmt_str, _op0, _op1, _op2, _op3) \
		 , &Context::ExecuteInst_##name
#include "../disassembler/Instruction.def"
#undef DEFINST
};

void Context::MipsIsaBranch(unsigned int dest)
{
	n_next_ip = dest;
}

void Context::MipsIsaRelBranch(unsigned int dest)
{
	n_next_ip = regs.getPC() + dest + 4;
}

void Context::ExecuteInst_J()
{
	// Read Operands
	unsigned int target = inst.getBytes()->target.target;

	// Perform Operation
	unsigned int dest = ((misc::getBits32(next_ip, 32, 28)) << 28 )
			| (target << 2);
	MipsIsaBranch(dest);
}


void Context::ExecuteInst_JAL()
{
	// Read Operands
	unsigned int target = inst.getBytes()->target.target;

	// Perform Operation
	unsigned int branch_target = ((misc::getBits32(next_ip, 32, 28)) << 28)
			| (target << 2);
	regs.setGPR(31, regs.getPC() + 8);
	MipsIsaBranch(branch_target);
}


void Context::ExecuteInst_BEQ()
{
	// Read Operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform Operation
	if(regs.getGPR(rs) == regs.getGPR(rt))
		MipsIsaRelBranch(misc::SignExtend32((imm << 2), 18));
}


void Context::ExecuteInst_BNE()
{
	// Read Operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform Operation
	if (regs.getGPR(rs) != regs.getGPR(rt))
		MipsIsaRelBranch(misc::SignExtend32((imm << 2), 18));
}


void Context::ExecuteInst_BLEZ()
{
	// Read Operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform Operation
	if ((int)regs.getGPR(rs) <= 0)
		MipsIsaRelBranch(misc::SignExtend32(imm << 2, 18));
}


void Context::ExecuteInst_BGTZ()
{
	// Read Operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform Operation
	if ((int)regs.getGPR(rs) > 0)
		MipsIsaRelBranch(misc::SignExtend32(imm << 2, 18));
}


void Context::ExecuteInst_ADDI()
{
	// Read Operands
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int offset = inst.getBytes()->offset_imm.offset;
	unsigned int rs_reg = regs.getGPR(rs);

	// Perform Operation
	int temp = ((rs_reg & 0x80000000) | rs_reg)
			+ misc::SignExtend32(offset, 16);
	if ((temp & 0x80000000) != (temp & 0x40000000))
			misc::Panic("ADDI: Integer overflow");
	regs.setGPR(rt, temp);
}


void Context::ExecuteInst_ADDIU()
{
	// Read Operands
	unsigned int imm = inst.getBytes()->offset_imm.offset;
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;

	// Perform Operation
	int temp = regs.getGPR(rs) + (signed)misc::SignExtend32(imm, 16);

	regs.setGPR(rt,temp);
	// emu->isa_debug << misc::fmt("  result is %d\n", regs.getGPR(rt));
}


void Context::ExecuteInst_SLTI()
{
	// Read Operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform Operation
	if ((int)(regs.getGPR(rs)) < (int)misc::SignExtend32(imm,16))
		regs.setGPR(rt, 1);
	else
		regs.setGPR(rt, 0);
}


void Context::ExecuteInst_SLTIU()
{
	// Read Operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform Operation
	if ((regs.getGPR(rs)) < ((unsigned)misc::SignExtend32(imm, 16)))
		regs.setGPR(rt, 1);
	else
		regs.setGPR(rt, 0);
}


void Context::ExecuteInst_ANDI()
{
	// Read operands from instruction bytes
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int imm = inst.getBytes()->offset_imm.offset;
	unsigned int rt = inst.getBytes()->standard.rt;

	// Perform AND operation
	regs.setGPR(rt, (regs.getGPR(rs) & imm));
}


void Context::ExecuteInst_ORI()
{
	// Read operands from instruction bytes
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform ORI operation
	regs.setGPR(rt, (regs.getGPR(rs) | imm));
}


void Context::ExecuteInst_XORI()
{
	// Read operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform XOR immediate operation
	unsigned int value = regs.getGPR(rs) ^ imm;
	regs.setGPR(rt, value);
}


void Context::ExecuteInst_LUI()
{
	// Read Operands
	unsigned int imm = inst.getBytes()->offset_imm.offset;
	unsigned int rt =inst.getBytes()->standard.rt;

	// Perform Operation
	regs.setGPR(rt,(imm << 16));
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
	// read operands from instruction
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform operation LB
	unsigned int addr = regs.getGPR(rs) + misc::SignExtend32(imm,16);
	char temp;
	memory->Read(addr, sizeof(char), &temp);
	regs.setGPR(rt, misc::SignExtend32(temp, 8));
}


void Context::ExecuteInst_LH()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LWL()
{
//	throw misc::Panic("Unimplemented instruction");
	// Loop counter
	int i;

	// Read operands
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Get the value of register rt as a char*
	unsigned int rt_value = regs.getGPR(rt);
	char *dst = (char *) &rt_value;

	// Get address of the memory read
	unsigned int addr = regs.getGPR(rs) + misc::SignExtend32(imm, 16);

	int size = 1 + (addr & 3);
	char src[4];
	memory->Read(addr, size, src);
	for(i = 0; i < size; i++)
		dst[3 - i] = src[i];
}


void Context::ExecuteInst_LW()
{
	// read operands from instruction
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform operation LW
	unsigned int addr = regs.getGPR(rs) + misc::SignExtend32(imm,16);
	unsigned int temp;
	if ((misc::getBits32(addr, 1, 0) | 0) == 1 )
		misc::panic("LW: error, effective address is not naturally-aligned\n");
	memory->Read(addr, 4, (char *)&temp);
	regs.setGPR(rt, temp);

//	// Debug
//	if(emu->isa_debug)
//	{
//		emu->isa_debug << misc::fmt("Addr is reg[%d]=%x + 0x%x = 0x%x, ",
//				rs, regs.getGPR(rs), imm, addr);
//		emu->isa_debug << misc::fmt("value loaded: %x\n", temp);
//	}
}


void Context::ExecuteInst_LBU()
{
	// Read Operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform operation LBU
	unsigned int virtual_addr = regs.getGPR(base) + misc::SignExtend32((signed)imm, 16);
	unsigned char temp;
	memory->Read(virtual_addr, sizeof(unsigned char), (char *)&temp);
	regs.setGPR(rt, (unsigned)temp);
}


void Context::ExecuteInst_LHU()
{
	// Read Operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform operation LHU
	unsigned short int temp;
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed)imm, 16);
	memory->Read(addr, sizeof(unsigned short int), (char *)&temp);
	regs.setGPR(rt, misc::SignExtend32(temp, 16));
}


void Context::ExecuteInst_LWR()
{
	// Read Operands from instruction
	char src[4];
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int imm = inst.getBytes()->offset_imm.offset;
	unsigned int rt_value = regs.getGPR(rt);
	unsigned char *dst = (unsigned char *) &rt_value;

	// Perform operation LWR
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed) imm, 16);
	int i, size = 1 + (addr & 3);
	memory->Read(addr-size + 1, size, src);
	for (i = 0; i < size; i++)
		dst[size - i - 1]= src[i];
}


void Context::ExecuteInst_SB()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform operation SB
	char temp = regs.getGPR(rt);
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed) imm, 16);
	memory->Write(addr, sizeof(char), &temp);
}


void Context::ExecuteInst_SH()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform operation SH
	unsigned short int temp = regs.getGPR(rt);
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed) imm, 16);
	memory->Write(addr, sizeof(unsigned short int), (char *)&temp);
}


void Context::ExecuteInst_SWL()
{
	throw misc::Panic("Unimplemented instruction");
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int imm = inst.getBytes()->offset_imm.offset;
	unsigned int rt_value = regs.getGPR(rt);
	unsigned char *src = (unsigned char *) & rt_value;
	char dst[4];

	// Perform operation SWL
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed) imm, 16);
	int i, size = 4 - (addr & 3);
	for (i = 0; i < size; i++)
		dst[i] = src[3 -i];
	memory->Write(addr, size, dst);
}


void Context::ExecuteInst_SW()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int temp = regs.getGPR(rt);
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int imm = inst.getBytes()->offset_imm.offset;
	unsigned int addr = regs.getGPR(rs) + misc::SignExtend32(imm, 16);

	// Perform operation SW
	memory->Write(addr, 4, (char*)&temp);
//	if(emu->isa_debug)
//		emu->isa_debug << misc::fmt("stored value: 0x%x\n", temp);
}


void Context::ExecuteInst_SWR()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int imm = inst.getBytes()->offset_imm.offset;
	unsigned int rt_value = regs.getGPR(rt);
	unsigned char *src = (unsigned char *) & rt_value;
	char dst[4];

	// Perform operation SWR
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed) imm, 16);
	int i, size = 1 + (addr & 3);
	for (i = 0; i < size; i++)
		dst[i] = src[size - i -1];
	memory->Write(addr - size + 1, size, dst);
}


void Context::ExecuteInst_CACHE()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LL()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int imm = inst.getBytes()->offset_imm.offset;
	unsigned int temp;

	// Perform operation LL
	unsigned int addr = regs.getGPR(base) + imm;
	memory->Read(addr, 4, (char *)&temp);
	regs.setGPR(rt, temp);
	// FIXME: set LLbit = 1
}


void Context::ExecuteInst_LWC1()
{
	// Read operands from instruction
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int imm = inst.getBytes()->offset_imm.offset;
	unsigned int ft = inst.getBytes()->offset_imm.rt;
	unsigned int temp;
	float f;

	// Perform operation LWC1
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed)imm, 16);
	memory->Read(addr, 4, (char*)&temp);
	f = (float)temp;
	regs.setSinglePrecisionFPR(ft, f);
}


void Context::ExecuteInst_LWC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_PREF()
{
  //  throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_LDC1()
{
	// Read operands from instruction
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int imm = inst.getBytes()->offset_imm.offset;
	unsigned int ft =inst.getBytes()->offset_imm.rt;
	unsigned long long temp;
	float f;

	// Perform operation LDC1
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed) imm, 16);
	memory->Read(addr, 4, (char *) &temp);
	f = (double) temp;
	regs.setDoublePrecisionFPR(ft, f);
}


void Context::ExecuteInst_LDC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SC()
{
	// Read operands from instruction
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int imm = inst.getBytes()->offset_imm.offset;
	unsigned int rt = inst.getBytes()->offset_imm.rt;
	unsigned int temp = regs.getGPR(rt);

	// Perform operation SC
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32(imm, 16);
	memory->Write(addr, 4, (char *) &temp);
	regs.setGPR(rt, 1);
}


void Context::ExecuteInst_SWC1()
{
	// Read operands from instruction
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int imm = inst.getBytes()->offset_imm.offset;
	unsigned int ft =inst.getBytes()->offset_imm.rt;

	// Perform operation SWC1
	float f = regs.getSinglePrecisionFPR(ft);
	unsigned int temp = (unsigned int) f;
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed) imm, 16);
	memory->Write(addr, 4, (char *) &temp);
}


void Context::ExecuteInst_SWC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SDC1()
{
	// Read operands from instruction
	unsigned int base = inst.getBytes()->offset_imm.base;
	unsigned int imm = inst.getBytes()->offset_imm.offset;
	unsigned int ft = inst.getBytes()->standard.rt;

	// Perform operation SDC1
	double dbl = regs.getDoublePrecisionFPR(ft);
	unsigned int temp = (unsigned int) dbl;
	unsigned int addr = regs.getGPR(base) + misc::SignExtend32((signed) imm, 16);
	memory->Write(addr, sizeof(unsigned int), (char*) &temp);
}


void Context::ExecuteInst_SDC2()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SLL()
{
	// Read operands from instruction
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int sa = inst.getBytes()->standard.sa;

	// Perform operation SLL
	regs.setGPR(rd,((regs.getGPR(rt) << sa)|0));
}


void Context::ExecuteInst_MOVF()
{
	// FIXME: need to implement float point
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MOVT()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SRL()
{
	// Read operands from instruction
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int sa = inst.getBytes()->standard.sa;

	// Perform operation SRL
	unsigned int temp = regs.getGPR(rt) >> sa;
	regs.setGPR(rd, temp);
}


void Context::ExecuteInst_ROR()
{
	unsigned int sa = inst.getBytes()->standard.sa;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rd = inst.getBytes()->standard.rd;

	unsigned int temp = misc::getBits32(regs.getGPR(rt), sa-1, 0) |
			misc::getBits32(regs.getGPR(rt), 31, sa);
	regs.setGPR(rd, temp);
}


void Context::ExecuteInst_SRA()
{
	// Read operands from instruction
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int sa = inst.getBytes()->standard.sa;

	// Perform operation SRA
	unsigned int temp = (int) regs.getGPR(rt) >> sa;
	regs.setGPR(rd, temp);
}


void Context::ExecuteInst_SLLV()
{
	// Read operands from instruction
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rs = inst.getBytes()->standard.rs;

	// Perform operation SLLV
	unsigned int s = misc::getBits32(regs.getGPR(rs), 4, 0);
	unsigned int temp = regs.getGPR(rt) << s;
	regs.setGPR(rd, temp);
}


void Context::ExecuteInst_SRLV()
{
	// Read operands from instruction
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rs = inst.getBytes()->standard.rs;

	// Perform operation SRLV
	unsigned int s = misc::getBits32(regs.getGPR(rs), 4, 0);
	regs.setGPR(rd, regs.getGPR(rt) >> s);
}


void Context::ExecuteInst_ROTRV()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SRAV()
{
	// Read operands from instruction
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rs = inst.getBytes()->standard.rs;

	// Perform operation SRAV
	unsigned int s = misc::getBits32(regs.getGPR(rs), 4, 0);
	regs.setGPR(rd, (int) regs.getGPR(rt) >> s);
}


void Context::ExecuteInst_JR()
{
	// Read operands from instruction
	unsigned int rs = inst.getBytes()->standard.rs;

	// Perform operation JR
	MipsIsaBranch(regs.getGPR(rs));

	// Debug
	emulator->isa_debug << misc::fmt("jump to reg[%d]=0x%x",
			rs, regs.getCoprocessor0GPR(rs));
}


void Context::ExecuteInst_JALR()
{
	// Read operands from instruction
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rd = inst.getBytes()->standard.rd;

	// Perform operation JALR
	MipsIsaBranch(regs.getGPR(rs));
	regs.setGPR(rd, (regs.getPC() + 8));
}


void Context::ExecuteInst_MOVZ()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rs = inst.getBytes()->standard.rs;

	// Perform operation MOVZ
	if (regs.getGPR(rt) == 0)
		regs.setGPR(rd, regs.getGPR(rs));
}


void Context::ExecuteInst_MOVN()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rs = inst.getBytes()->standard.rs;

	// Perform operation MOVN
	if (regs.getGPR(rt) != 0)
		regs.setGPR(rd, regs.getGPR(rs));
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
	//FIXME: implement sync
	//throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MFHI()
{
	// Read operands
	unsigned int reg_hi = regs.getHI();
	unsigned int rd = inst.getBytes()->standard.rd;

	// Perform operation
	regs.setGPR(rd, reg_hi);
}


void Context::ExecuteInst_MTHI()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MFLO()
{
	// Read operands from instruction
	unsigned int rd = inst.getBytes()->standard.rd;

	// Perform operation MFLO
	regs.setGPR(rd, regs.getLO());
}


void Context::ExecuteInst_MTLO()
{
	// Read operands from instruction
	unsigned int rs = inst.getBytes()->standard.rs;

	// Perform operation MTLO
	regs.setLO(regs.getGPR(rs));
}


void Context::ExecuteInst_MULT()
{
	// Read operands from instruction
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;

	// Perform operation MULT
	unsigned long long int temp = (long long int)regs.getGPR(rs) *
			(long long int)regs.getGPR(rt);
	regs.setLO(misc::getBits64(temp, 31, 0));
	regs.setHI(misc::getBits64(temp, 63, 32));
}


void Context::ExecuteInst_MULTU()
{
	// Read operands from instruction
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;

	// Perform operation MULTU
	unsigned long long int temp = (unsigned long long int) regs.getGPR(rs) *
			(unsigned long long int) regs.getGPR(rt);
	regs.setLO(misc::getBits64(temp, 31, 0));
	regs.setHI(misc::getBits64(temp, 63, 32));
}


void Context::ExecuteInst_DIV()
{
	// Read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rs = inst.getBytes()->standard.rs;

	// Perform operation DIV
	if (regs.getGPR(rt))
	{
		regs.setLO((int)regs.getGPR(rs) / (int)regs.getGPR(rt));
		regs.setHI((int)regs.getGPR(rs) % (int)regs.getGPR(rt));
	}
}


void Context::ExecuteInst_DIVU()
{
	// Read operands
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rs = inst.getBytes()->standard.rs;

	// Perform operation
	unsigned int rs_value = regs.getGPR(rs);
	unsigned int rt_value = regs.getGPR(rt);

	if (rt_value != 0)
	{
		regs.setHI(rs_value % rt_value);
		regs.setLO(rs_value / rt_value);
	}
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
	// read operands from instruction
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rd = inst.getBytes()->standard.rd;

	// perform operation SUBU
	unsigned int value = regs.getGPR(rs) - regs.getGPR(rt);
	regs.setGPR(rd, value);
}


void Context::ExecuteInst_AND()
{
	// Read operands from instruction bytes
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rd = inst.getBytes()->standard.rd;

	// Perform AND operation
	regs.setGPR(rd, (regs.getGPR(rs) & regs.getGPR(rt)));
}


void Context::ExecuteInst_OR()
{
	// Read operands from instruction bytes
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rd = inst.getBytes()->standard.rd;

	// Perform OR operation
	regs.setGPR(rd, (regs.getGPR(rs) | regs.getGPR(rt)));
}


void Context::ExecuteInst_XOR()
{
	// Read operands from instruction bytes
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rd = inst.getBytes()->standard.rd;

	// Perform XOR operation
	regs.setGPR(rd, (regs.getGPR(rs) ^ regs.getGPR(rt)));
}


void Context::ExecuteInst_NOR()
{
	// Read operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rd = inst.getBytes()->standard.rd;

	// Perform operation
	unsigned int temp = regs.getGPR(rs) | regs.getGPR(rt);
	regs.setGPR(rd, ~temp);
}


void Context::ExecuteInst_SLT()
{
	// Read operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rd = inst.getBytes()->standard.rd;

	// Perform operation
	if((int)regs.getGPR(rs) < (int)regs.getGPR(rt))
		regs.setGPR(rd, 1);
	else
		regs.setGPR(rd, 0);
}


void Context::ExecuteInst_SLTU()
{
	// Read operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rd = inst.getBytes()->standard.rd;

	// Perform SLTU operation
	if(regs.getGPR(rs) < regs.getGPR(rt))
		regs.setGPR(rd, 1);
	else
		regs.setGPR(rd, 0);
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
	// Read operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;

	// Perform 'Trap if Equal' operation
	if ((int)regs.getGPR(rs) == (int)regs.getGPR(rt))
	{
		misc::Panic(misc::fmt("%s: Trap exception", __FUNCTION__));
	}
}


void Context::ExecuteInst_TNE()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_BLTZ()
{
	// Read operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform operation
	if ((int)regs.getGPR(rs) < 0)
		MipsIsaRelBranch(misc::SignExtend32(imm << 2, 18));
}


void Context::ExecuteInst_BGEZ()
{
	// Read operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int imm = inst.getBytes()->offset_imm.offset;

	// Perform operation
	if ((int)regs.getGPR(rs) >= 0)
		MipsIsaRelBranch(misc::SignExtend32(imm << 2, 18));
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
	// Read operands from instruction
	unsigned int pc = regs.getPC();
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int imm = inst.getBytes()->offset_imm.offset;
	regs.setGPR(31, pc + 8);

	// Perform instruction bgezal
	if ((int)regs.getGPR(rs) >= 0)
		MipsIsaRelBranch(misc::SignExtend32(imm, 16) << 2);
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
	// Read operands
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rd = inst.getBytes()->standard.rd;

	// Perform operation
	long long int temp = (int)regs.getGPR(rs) * (int)regs.getGPR(rt);
	regs.setGPR(rd, temp);
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
	// Get arguments
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rs_reg = regs.getGPR(rs);

	// Perform CLZ operation:
	// Count the number of leading zero's in GPR rs and put in GPR rd
	int i;
	int temp = 32;

	for (i=31; i>=0; i--)
	{
	if (rs_reg & 0x80000000)
	{
		temp = 31 - i;
		break;
	}
	rs_reg = rs_reg << 1;
	}
	regs.setGPR(rd, temp);
}


void Context::ExecuteInst_CLO()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_EXT()
{
  unsigned int rs = inst.getBytes()->standard.rs;
  unsigned int rt = inst.getBytes()->standard.rt;
  unsigned int rd = inst.getBytes()->standard.rd;
  unsigned int sa = inst.getBytes()->standard.sa;

  if (rd + sa > 31)
    misc::Panic("inst EXT, unexpected behavior");

  unsigned temp = 0 | misc::getBits32(regs.getGPR(rs), rd, sa);
  regs.setGPR(rt, temp);
}


void Context::ExecuteInst_INS()
{
	// Get operands
	unsigned int sa = inst.getBytes()->standard.sa;
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rs = inst.getBytes()->standard.rs;
	unsigned int rt = inst.getBytes()->standard.rt;

	// Perform operation
	unsigned int value = misc::getBits32(regs.getGPR(rs), rd - sa, 0);
	unsigned int new_rt = misc::setBits32(regs.getGPR(rt), rd, sa, value);
	regs.setGPR(rt, new_rt);
}


void Context::ExecuteInst_WSBH()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_SEB()
{
	// Get operands
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int rd = inst.getBytes()->standard.rd;

	// Perform Sign Extend Byte operation
	unsigned int rt_value = regs.getGPR(rt);
	unsigned int value = misc::SignExtend32(misc::getBits32(rt_value, 7, 0), 8);
	regs.setGPR(rd, value);
}


void Context::ExecuteInst_SEH()
{

	// Read operands from instruction
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;

	unsigned short int temp =
			(unsigned short int)misc::getBits32(regs.getGPR(rt), 15, 0);
	regs.setGPR(rd, misc::SignExtend32(temp, 16));
}


void Context::ExecuteInst_RDHWR()
{
	// Read operands from instruction
	unsigned int rd = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;

	if(rd == 29)
		regs.setGPR(rt, regs.getCoprocessor0GPR(rd));
	//FIXME: This is implemented only for the case where RD is 29
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
	unsigned int temp = 0;
	unsigned int fs = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;
	switch(fs)
	{
	case 0:
		temp = regs.getFIR();
		break;
	case 25:
		temp = (misc::getBits32(regs.getFCSR(), 31, 25) << 1) |
				misc::getBits32(regs.getFCSR(), 23, 23);
		break;
	case 28:
		temp = (misc::getBits32(regs.getFCSR(), 11, 7) << 7 |
				misc::getBits32(regs.getFCSR(), 1, 0));
		break;
	case 31:
		temp = regs.getFCSR();
		break;
	default:
		misc::Panic(misc::fmt("%s: CFC1 instruction, unknown value for fs", __FILE__));
	}
	regs.setGPR(rt, temp);
}


void Context::ExecuteInst_MFHC1()
{
	throw misc::Panic("Unimplemented instruction");
}


void Context::ExecuteInst_MTC1()
{
	// Read operands from instruction
	unsigned int fs = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;

	// Perform instruction MTC1
	regs.setSinglePrecisionFPR(fs,regs.getGPR(rt));
}


void Context::ExecuteInst_CTC1()
{
	// Read operands from instruction
	unsigned int fs = inst.getBytes()->standard.rd;
	unsigned int rt = inst.getBytes()->standard.rt;
	unsigned int temp = regs.getGPR(rt);

	// Perform instruction ctc1
	switch(fs)
	{
	case 25:
		regs.setFCSR((misc::getBits32(temp, 7, 1) << 25) |
				(misc::getBits32(regs.getFCSR(), 24, 24) << 24) |
				(misc::getBits32(temp, 0, 0) << 23) |
				misc::getBits32(regs.getFCSR(), 1, 0));
		break;
	case 26:
		regs.setFCSR(((misc::getBits32(temp, 31, 18) << 18) |
				(misc::getBits32(regs.getFCSR(), 17, 12) << 24) |
				(misc::getBits32(temp, 0, 0) << 23) |
				misc::getBits32(regs.getFCSR(), 1, 0)));
		break;
	case 28:
		regs.setFCSR(((misc::getBits32(temp, 31, 25) << 25) |
				(misc::getBits32(temp, 2, 2) << 24) |
				(misc::getBits32(regs.getFCSR(), 23, 12) << 12) |
				(misc::getBits32(temp, 11, 7) << 7) |
				misc::getBits32(regs.getFCSR(), 6, 2) << 2) |
				misc::getBits32(temp, 1, 0));
		break;
	case 31:
		regs.setFCSR(temp);
		break;
	default:
		misc::fatal("machine.c: CTC1: unknown value for fs");
	}
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
