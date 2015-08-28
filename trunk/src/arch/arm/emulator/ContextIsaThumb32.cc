/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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


namespace ARM
{

Context::ExecuteInstThumb32Fn Context::execute_inst_thumb32_fn[Instruction::Thumb32OpcodeCount] =
{
	nullptr  // For Instruction::Thumb32OpcodeNone
#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6,_op7,_op8) \
		 , &Context::ExecuteInstThumb32_##_name
#include <arch/arm/disassembler/InstructionThumb32.def>
#undef DEFINST
};


void Context::ExecuteInstThumb32_STREX()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDREX()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRD_imm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRD_imm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRD_imm3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRD_imm4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRD_imm5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRD_imm6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_lit1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_lit2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imma()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_immb()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_immc()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_immd()
{
	int offset;
	int value1;
	int value2;
	void *buf1;
	void *buf2;
	int immd12;
	unsigned int idx;
	unsigned int wback;
	unsigned int add;
	int rn_val;
	unsigned int addr;

	buf1 = &value1;
	buf2 = &value2;
	immd12 = inst.getThumb32Bytes()->ld_st_double.immd8;
	idx = inst.getThumb32Bytes()->ld_st_double.index;
	add = inst.getThumb32Bytes()->ld_st_double.add_sub;
	wback = inst.getThumb32Bytes()->ld_st_double.wback;

	if(inst.getThumb32Bytes()->ld_st_double.rn < 15)
	{
		IsaRegLoad(inst.getThumb32Bytes()->ld_st_double.rn, rn_val);
		if(add)
			offset = rn_val + (immd12 & 0x000000ff);
		else
			offset = rn_val - (immd12 & 0x000000ff);

		if(idx)
			addr = offset;
		else
			addr = rn_val;
		emulator->isa_debug << misc::fmt("imm4  addr  = %d; (0x%x)\n", addr, addr);
		if(wback)
			IsaRegStore( inst.getThumb32Bytes()->ld_st_double.rn, offset);

		memory->Read(addr, 4, (char *)buf1);
		memory->Read(addr + 4, 4, (char *)buf2);
		value1 = value1 & (0xffffffff);
		value2 = value2 & (0xffffffff);

		if(inst.getThumb32Bytes()->ld_st_double.rt < 15)
			IsaRegStore( inst.getThumb32Bytes()->ld_st_double.rt, value1);
		else
		{
			if(value1 % 2)
				IsaRegStore(inst.getThumb32Bytes()->ld_st_double.rt, value1 - 3);
			else
				IsaRegStore(inst.getThumb32Bytes()->ld_st_double.rt, value1 - 2);
		}

		if(inst.getThumb32Bytes()->ld_st_double.rt2 < 15)
			IsaRegStore( inst.getThumb32Bytes()->ld_st_double.rt2, value2);
		else
		{
			if(value1 % 2)
				IsaRegStore(inst.getThumb32Bytes()->ld_st_double.rt2, value2 - 3);
			else
				IsaRegStore(inst.getThumb32Bytes()->ld_st_double.rt2, value2 - 2);
		}

	}

	else if (inst.getThumb32Bytes()->ldstr_imm.rn == 15)
	{
		emulator->isa_debug << misc::fmt("  pc  = 0x%x; \n", regs.getPC());
		if((regs.getPC() - 4) % 4 == 2)
			offset = (regs.getPC() - 4) + 2;
		else
			offset = regs.getPC() - 4;
		emulator->isa_debug << misc::fmt("  offset  = 0x%x; \n", offset);
		if(inst.getThumb32Bytes()->ld_st_double.add_sub)
			addr = offset + immd12;
		else
			addr = offset - immd12;

		memory->Read(addr, 4, (char *)buf1);
		memory->Read(addr + 4, 4, (char *)buf2);
		value1 = value1 & (0xffffffff);
		value2 = value2 & (0xffffffff);

		if(inst.getThumb32Bytes()->ld_st_double.rt < 15)
			IsaRegStore( inst.getThumb32Bytes()->ld_st_double.rt, value1);
		else
		{
			if(value1 % 2)
				IsaRegStore(inst.getThumb32Bytes()->ld_st_double.rt, value1 - 3);
			else
				IsaRegStore(inst.getThumb32Bytes()->ld_st_double.rt, value1 - 2);
		}

		if(inst.getThumb32Bytes()->ld_st_double.rt2 < 15)
			IsaRegStore( inst.getThumb32Bytes()->ld_st_double.rt2, value2);
		else
		{
			if(value1 % 2)
				IsaRegStore(inst.getThumb32Bytes()->ld_st_double.rt2, value2 - 3);
			else
				IsaRegStore(inst.getThumb32Bytes()->ld_st_double.rt2, value2 - 2);
		}
	}
}

void Context::ExecuteInstThumb32_LDRD_imm0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm20()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm21()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm22()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm23()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm24()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm25()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm26()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm27()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm28()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm29()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm210()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm211()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm212()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm213()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRD_imm214()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_TBB()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_TBH()
{
	int rn_val;
	int rm_val;
	int addr;
	unsigned int hfwrd;
	void *buf;

	hfwrd = 0;
	buf = &hfwrd;
	if(inst.getThumb32Bytes()->table_branch.rn == 15)
	{
		emulator->isa_debug << misc::fmt("  PC = 0x%x\n", regs.getPC());
		IsaRegLoad(inst.getThumb32Bytes()->table_branch.rn, rn_val);
	}

	IsaRegLoad(inst.getThumb32Bytes()->table_branch.rm, rm_val);

	rm_val = rm_val << 1;

	addr = rn_val + rm_val;
	emulator->isa_debug << misc::fmt("  Addr = 0x%x\n", addr);

	memory->Read(addr, 2, (char *)buf);
	emulator->isa_debug << misc::fmt("  HFwrd = 0x%x; Changed PC = 0x%x\n", hfwrd, (regs.getPC() - 2 + (2 * hfwrd)));
	regs.incPC(2 * hfwrd);
}

void Context::ExecuteInstThumb32_STM()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDM15()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_POP()
{
	unsigned int regs;
	unsigned int i;
	int wrt_val;
	int sp_val;
	int reg_val;
	void* buf;

	Instruction::Thumb32Category cat = inst.getInstThumb32Info()->cat32;

	buf = &reg_val;

	if (cat == Instruction::Thumb32CategoryLdStMult)
		regs = inst.getThumb32Bytes()->ld_st_mult.reglist;
	else if (cat == Instruction::Thumb32CategoryPushPop)
		regs = inst.getThumb32Bytes()->push_pop.reglist;

	else
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	IsaRegLoad(13, sp_val);
	wrt_val = sp_val;

	for (i = 1; i < 65536; i *= 2)
	{
		if(regs & (i))
		{
			if(misc::LogBase2(i) < 15)
			{
				memory->Read(wrt_val, 4, (char *)buf);
				IsaRegStore(misc::LogBase2(i), reg_val);
				emulator->isa_debug << misc::fmt("  pop r%d <= 0x%x\n",misc::LogBase2(i),wrt_val);
				wrt_val += 4;
			}
			else
			{
				memory->Read(wrt_val, 4, (char *)buf);
				if(reg_val % 2)
					reg_val = reg_val - 1;

				IsaRegStore(misc::LogBase2(i), reg_val - 2);
				emulator->isa_debug << misc::fmt("  pop r%d <= 0x%x\n",misc::LogBase2(i),wrt_val);
				wrt_val += 4;

			}
		}
	}

	sp_val = sp_val + 4*(IsaBitCount(regs));
	IsaRegStore(13, sp_val);
}

void Context::ExecuteInstThumb32_PUSH()
{


	unsigned int regs;
	unsigned int i;
	int wrt_val;
	int sp_val;
	int reg_val;
	void* buf;



	Instruction::Thumb32Category cat = inst.getInstThumb32Info()->cat32;

	buf = &reg_val;

	if (cat == Instruction::Thumb32CategoryLdStMult)
		regs = inst.getThumb32Bytes()->ld_st_mult.reglist;
	else if (cat == Instruction::Thumb32CategoryPushPop)
		regs = inst.getThumb32Bytes()->push_pop.reglist;

	else
		throw misc::Panic(misc::fmt("%d: regs fmt not recognized", cat));
	IsaRegLoad(13, sp_val);
	wrt_val = sp_val - 4*(IsaBitCount(regs));

	for (i = 1; i < 65536; i *= 2)
	{
		if(regs & (i))
		{
			IsaRegLoad(misc::LogBase2(i) , reg_val);
			memory->Write(wrt_val, 4, (char *)buf);
			emulator->isa_debug << misc::fmt("  push r%d => 0x%x\n",misc::LogBase2(i),wrt_val);
			wrt_val += 4;
		}
	}

	sp_val = sp_val - 4*(IsaBitCount(regs));
	IsaRegStore(13, sp_val);
}

void Context::ExecuteInstThumb32_LDMDB()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STMDB15()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_TST_reg0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_reg0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_reg4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_reg5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_reg6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_reg7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_reg8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_reg9()
{
	unsigned int shift;
	unsigned int type;

	//regsPsr prev_regs_cpsr;
	int rm_val;
	int rn_val;
	int result;

	Instruction::Thumb32Category cat = inst.getInstThumb32Info()->cat32;

	//prev_regs_cpsr = regs.getCPSR();
	IsaRegLoad(inst.getThumb32Bytes()->data_proc_shftreg.rm, rm_val);
	IsaRegLoad(inst.getThumb32Bytes()->data_proc_shftreg.rn, rn_val);

	if (cat == Instruction::Thumb32CategoryDprShftreg)
	{
		type = inst.getThumb32Bytes()->data_proc_shftreg.type;
		shift = (inst.getThumb32Bytes()->data_proc_shftreg.imm3 << 2) | (inst.getThumb32Bytes()->data_proc_shftreg.imm2);
	}

	else
		throw misc::Panic(misc::fmt("%d: fmt not recognized", cat));

	if(shift)
	{
		switch(type)
		{
		case (Instruction::ShiftOperatorLsl):
			rm_val = rm_val << shift;
		break;

		case (Instruction::ShiftOperatorLsr):
			rm_val = rm_val >> shift;
		break;

		case (Instruction::ShiftOperatorAsr):
			rm_val = rm_val / (1 << shift);
		break;

		case (Instruction::ShiftOperatorRor):
			rm_val = IsaRotr(rm_val, shift);
		break;
		}
	}

	result = rn_val & rm_val;
	IsaRegStore(inst.getThumb32Bytes()->data_proc_shftreg.rd, result);
	if(inst.getThumb32Bytes()->data_proc_immd.sign)
	{

		//prev_regs_cpsr = regs.getCPSR();

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;

		if(result == 0)
		{
			regs.getCPSR().z = 1;
			regs.getCPSR().n = 0;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if(result < 0)
		{
			regs.getCPSR().n = 1;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		//regs.getCPSR().C = prev_regs_cpsr.C;
		IsaCpsrPrint();
	}


}

void Context::ExecuteInstThumb32_AND_reg10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_reg11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_reg12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_reg13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_reg14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BIC_reg()
{
	unsigned int shift;
	unsigned int type;



	int rm_val;
	int rn_val;
	int result;



	Instruction::Thumb32Category cat = inst.getInstThumb32Info()->cat32;

	//prev_regs_cpsr = regs.getCPSR();
	IsaRegLoad(inst.getThumb32Bytes()->data_proc_shftreg.rm, rm_val);
	IsaRegLoad(inst.getThumb32Bytes()->data_proc_shftreg.rn, rn_val);

	if (cat == Instruction::Thumb32CategoryDprShftreg)
	{
		type = inst.getThumb32Bytes()->data_proc_shftreg.type;
		shift = (inst.getThumb32Bytes()->data_proc_shftreg.imm3 << 2) | (inst.getThumb32Bytes()->data_proc_shftreg.imm2);
	}

	else
		throw misc::Panic(misc::fmt("%d: fmt not recognized", cat));

	if(shift)
	{
		switch(type)
		{
		case (Instruction::ShiftOperatorLsl):
			rm_val = rm_val << shift;
		break;

		case (Instruction::ShiftOperatorLsr):
			rm_val = rm_val >> shift;
		break;

		case (Instruction::ShiftOperatorAsr):
			rm_val = rm_val / (1 << shift);
		break;

		case (Instruction::ShiftOperatorRor):
			rm_val = IsaRotr(rm_val, shift);
		break;
		}
	}

	result = rn_val & (~(rm_val));
	IsaRegStore(inst.getThumb32Bytes()->data_proc_shftreg.rd, result);
	if(inst.getThumb32Bytes()->data_proc_immd.sign)
	{

		//prev_regs_cpsr = regs.getCPSR();

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;

		if(result == 0)
		{
			regs.getCPSR().z = 1;
			regs.getCPSR().n = 0;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if(result < 0)
		{
			regs.getCPSR().n = 1;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		//regs.getCPSR().C = prev_regs_cpsr.C;
		IsaCpsrPrint();
	}

}

void Context::ExecuteInstThumb32_ORR_reg0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_reg2()
{
	unsigned int shift;
	unsigned int type;



	//regsPsr prev_regs_cpsr;

	int rm_val;
	int rn_val;
	int result;



	Instruction::Thumb32Category cat = inst.getInstThumb32Info()->cat32;

	//prev_regs_cpsr = regs.getCPSR();
	IsaRegLoad(inst.getThumb32Bytes()->data_proc_shftreg.rm, rm_val);
	IsaRegLoad(inst.getThumb32Bytes()->data_proc_shftreg.rn, rn_val);

	if (cat == Instruction::Thumb32CategoryDprShftreg)
	{
		type = inst.getThumb32Bytes()->data_proc_shftreg.type;
		shift = (inst.getThumb32Bytes()->data_proc_shftreg.imm3 << 2) | (inst.getThumb32Bytes()->data_proc_shftreg.imm2);
	}

	else
		throw misc::Panic(misc::fmt("%d: fmt not recognized", cat));

	if(shift)
	{
		switch(type)
		{
		case (Instruction::ShiftOperatorLsl):
			rm_val = rm_val << shift;
		break;

		case (Instruction::ShiftOperatorLsr):
			rm_val = rm_val >> shift;
		break;

		case (Instruction::ShiftOperatorAsr):
			rm_val = rm_val / (1 << shift);
		break;

		case (Instruction::ShiftOperatorRor):
			rm_val = IsaRotr(rm_val, shift);
		break;
		}
	}

	result = rn_val | rm_val;
	IsaRegStore(inst.getThumb32Bytes()->data_proc_shftreg.rd, result);
	if(inst.getThumb32Bytes()->data_proc_immd.sign)
	{

		//prev_regs_cpsr = regs.getCPSR();

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;

		if(result == 0)
		{
			regs.getCPSR().z = 1;
			regs.getCPSR().n = 0;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if(result < 0)
		{
			regs.getCPSR().n = 1;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		//regs.getCPSR().C = prev_regs_cpsr.C;
		IsaCpsrPrint();
	}

}

void Context::ExecuteInstThumb32_ORR_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_reg4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_reg5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_reg6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_reg7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_reg8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_reg9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_reg10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_reg11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_reg12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_reg13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_reg14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_reg0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_reg4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_reg5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_reg6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_reg7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_reg8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_reg9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_reg10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_reg11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_reg12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_reg13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_reg14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MVN_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_reg0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_reg4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_reg5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_reg6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_reg7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_reg8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_reg9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_reg10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_reg11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_reg12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_reg13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_reg14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_TST_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_reg0()
{
	unsigned int shift;
	unsigned int type;



	int rm_val;


	Instruction::Thumb32Category cat = inst.getInstThumb32Info()->cat32;

	IsaRegLoad(inst.getThumb32Bytes()->data_proc_shftreg.rm, rm_val);

	if (cat == Instruction::Thumb32CategoryDprShftreg)
	{
		type = inst.getThumb32Bytes()->data_proc_shftreg.type;
		shift = (inst.getThumb32Bytes()->data_proc_shftreg.imm3 << 2) | (inst.getThumb32Bytes()->data_proc_shftreg.imm2);
	}

	else
		throw misc::Panic(misc::fmt("%d: fmt not recognized", cat));

	if(shift)
	{
		switch(type)
		{
		case (Instruction::ShiftOperatorLsl):
			rm_val = rm_val << shift;
		break;

		case (Instruction::ShiftOperatorLsr):
			rm_val = rm_val >> shift;
		break;

		case (Instruction::ShiftOperatorAsr):
			rm_val = rm_val / (1 << shift);
		break;

		case (Instruction::ShiftOperatorRor):
			rm_val = IsaRotr(rm_val, shift);
		break;
		}
	}

	IsaThumbAdd(inst.getThumb32Bytes()->data_proc_shftreg.rd,
		inst.getThumb32Bytes()->data_proc_shftreg.rn, rm_val, 0, 0);

}

void Context::ExecuteInstThumb32_ADD_reg1()
{
	unsigned int shift;
	unsigned int type;



	int rm_val;


	Instruction::Thumb32Category cat = inst.getInstThumb32Info()->cat32;

	IsaRegLoad(inst.getThumb32Bytes()->data_proc_shftreg.rm, rm_val);

	if (cat == Instruction::Thumb32CategoryDprShftreg)
	{
		type = inst.getThumb32Bytes()->data_proc_shftreg.type;
		shift = (inst.getThumb32Bytes()->data_proc_shftreg.imm3 << 2) | (inst.getThumb32Bytes()->data_proc_shftreg.imm2);
	}

	else
		throw misc::Panic(misc::fmt("%d: fmt not recognized", cat));

	if(shift)
	{
		switch(type)
		{
		case (Instruction::ShiftOperatorLsl):
			rm_val = rm_val << shift;
		break;

		case (Instruction::ShiftOperatorLsr):
			rm_val = rm_val >> shift;
		break;

		case (Instruction::ShiftOperatorAsr):
			rm_val = rm_val / (1 << shift);
		break;

		case (Instruction::ShiftOperatorRor):
			rm_val = IsaRotr(rm_val, shift);
		break;
		}
	}

	IsaThumbAdd(inst.getThumb32Bytes()->data_proc_shftreg.rd,
		inst.getThumb32Bytes()->data_proc_shftreg.rn, rm_val, 0, 0);

}

void Context::ExecuteInstThumb32_ADD_reg2()
{
	unsigned int shift;
	unsigned int type;



	int rm_val;


	Instruction::Thumb32Category cat = inst.getInstThumb32Info()->cat32;

	IsaRegLoad(inst.getThumb32Bytes()->data_proc_shftreg.rm, rm_val);

	if (cat == Instruction::Thumb32CategoryDprShftreg)
	{
		type = inst.getThumb32Bytes()->data_proc_shftreg.type;
		shift = (inst.getThumb32Bytes()->data_proc_shftreg.imm3 << 2) | (inst.getThumb32Bytes()->data_proc_shftreg.imm2);
	}

	else
		throw misc::Panic(misc::fmt("%d: fmt not recognized", cat));

	if(shift)
	{
		switch(type)
		{
		case (Instruction::ShiftOperatorLsl):
			rm_val = rm_val << shift;
		break;

		case (Instruction::ShiftOperatorLsr):
			rm_val = rm_val >> shift;
		break;

		case (Instruction::ShiftOperatorAsr):
			rm_val = rm_val / (1 << shift);
		break;

		case (Instruction::ShiftOperatorRor):
			rm_val = IsaRotr(rm_val, shift);
		break;
		}
	}

	IsaThumbAdd(inst.getThumb32Bytes()->data_proc_shftreg.rd,
		inst.getThumb32Bytes()->data_proc_shftreg.rn, rm_val, 0, 0);

}

void Context::ExecuteInstThumb32_ADD_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_reg4()
{
	unsigned int shift;
	unsigned int type;



	int rm_val;


	Instruction::Thumb32Category cat = inst.getInstThumb32Info()->cat32;

	IsaRegLoad(inst.getThumb32Bytes()->data_proc_shftreg.rm, rm_val);

	if (cat == Instruction::Thumb32CategoryDprShftreg)
	{
		type = inst.getThumb32Bytes()->data_proc_shftreg.type;
		shift = (inst.getThumb32Bytes()->data_proc_shftreg.imm3 << 2) | (inst.getThumb32Bytes()->data_proc_shftreg.imm2);
	}

	else
		throw misc::Panic(misc::fmt("%d: fmt not recognized", cat));

	if(shift)
	{
		switch(type)
		{
		case (Instruction::ShiftOperatorLsl):
			rm_val = rm_val << shift;
		break;

		case (Instruction::ShiftOperatorLsr):
			rm_val = rm_val >> shift;
		break;

		case (Instruction::ShiftOperatorAsr):
			rm_val = rm_val / (1 << shift);
		break;

		case (Instruction::ShiftOperatorRor):
			rm_val = IsaRotr(rm_val, shift);
		break;
		}
	}

	IsaThumbAdd(inst.getThumb32Bytes()->data_proc_shftreg.rd,
		inst.getThumb32Bytes()->data_proc_shftreg.rn, rm_val, 0, 0);

}

void Context::ExecuteInstThumb32_ADD_reg5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_reg6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_reg7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_reg8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_reg9()
{
	unsigned int shift;
	unsigned int type;



	int rm_val;


	Instruction::Thumb32Category cat = inst.getInstThumb32Info()->cat32;

	IsaRegLoad(inst.getThumb32Bytes()->data_proc_shftreg.rm, rm_val);

	if (cat == Instruction::Thumb32CategoryDprShftreg)
	{
		type = inst.getThumb32Bytes()->data_proc_shftreg.type;
		shift = (inst.getThumb32Bytes()->data_proc_shftreg.imm3 << 2) | (inst.getThumb32Bytes()->data_proc_shftreg.imm2);
	}

	else
		throw misc::Panic(misc::fmt("%d: fmt not recognized", cat));

	if(shift)
	{
		switch(type)
		{
		case (Instruction::ShiftOperatorLsl):
			rm_val = rm_val << shift;
		break;

		case (Instruction::ShiftOperatorLsr):
			rm_val = rm_val >> shift;
		break;

		case (Instruction::ShiftOperatorAsr):
			rm_val = rm_val / (1 << shift);
		break;

		case (Instruction::ShiftOperatorRor):
			rm_val = IsaRotr(rm_val, shift);
		break;
		}
	}

	IsaThumbAdd(inst.getThumb32Bytes()->data_proc_shftreg.rd,
		inst.getThumb32Bytes()->data_proc_shftreg.rn, rm_val, 0, 0);

}

void Context::ExecuteInstThumb32_ADD_reg10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_reg11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_reg12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_reg13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_reg14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_CMN_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADC_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SBC_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_reg0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_reg4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_reg5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_reg6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_reg7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_reg8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_reg9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_reg10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_reg11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_reg12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_reg13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_reg14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_CMP_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_RSB_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MOVS_LSR_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MOVS_ASR_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MOVS_LSL_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_imm0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_imm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_imm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_imm3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_imm4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_imm5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_imm6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_imm7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_imm8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_imm9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_imm10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_imm11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_imm12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_imm13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_AND_imm14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_TST_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BIC_imm()
{
	unsigned int immd;
	int result;
	int rn_val;




	immd = (inst.getThumb32Bytes()->data_proc_immd.i_flag << 11)
					| (inst.getThumb32Bytes()->data_proc_immd.immd3 << 8)
					| (inst.getThumb32Bytes()->data_proc_immd.immd8);
	immd = IsaThumbImmdExtend(immd);

	IsaRegLoad(inst.getThumb32Bytes()->data_proc_immd.rn, rn_val);

	result = rn_val & (~(immd));
	IsaRegStore(inst.getThumb32Bytes()->data_proc_immd.rd, result);
	if(inst.getThumb32Bytes()->data_proc_immd.sign)
	{

		//prev_regs_cpsr = regs.getCPSR();

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;

		if(result == 0)
		{
			regs.getCPSR().z = 1;
			regs.getCPSR().n = 0;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if(result < 0)
		{
			regs.getCPSR().n = 1;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		//regs.getCPSR().C = prev_regs_cpsr.C;
		IsaCpsrPrint();
	}
}

void Context::ExecuteInstThumb32_ORR_imm0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_imm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_imm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_imm3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_imm4()
{
	unsigned int operand;
	int rn_val;
	int result;


	regsPsr prev_regs_cpsr;


	operand = IsaThumb32Immd12(inst.getInstThumb32Info()->cat32);
	emulator->isa_debug << misc::fmt("  immd32 = 0x%x\n", operand);

	IsaRegLoad(inst.getThumb32Bytes()->data_proc_immd.rn, rn_val);

	result = rn_val | operand;
	emulator->isa_debug << misc::fmt("  result = 0x%x\n", result);

	IsaRegStore(inst.getThumb32Bytes()->data_proc_immd.rd, result);

	if(inst.getThumb32Bytes()->data_proc_immd.sign)
	{

		prev_regs_cpsr = regs.getCPSR();

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;

		if(operand == 0)
		{
			regs.getCPSR().z = 1;
			regs.getCPSR().n = 0;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if(operand < 0)
		{
			regs.getCPSR().n = 1;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		regs.getCPSR().C = prev_regs_cpsr.C;
		IsaCpsrPrint();
	}
}

void Context::ExecuteInstThumb32_ORR_imm5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_imm6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_imm7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_imm8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_imm9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_imm10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_imm11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_imm12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_imm13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORR_imm14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MOV_imm()
{
	unsigned int operand;

	operand = IsaThumb32Immd12(inst.getInstThumb32Info()->cat32);
	emulator->isa_debug << misc::fmt("  immd32 = %d\n", operand);
	IsaRegStore(inst.getThumb32Bytes()->data_proc_immd.rd, operand);
}

void Context::ExecuteInstThumb32_ORN_imm0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_imm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_imm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_imm3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_imm4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_imm5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_imm6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_imm7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_imm8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_imm9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_imm10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_imm11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_imm12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_imm13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ORN_imm14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MVN_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_imm0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_imm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_imm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_imm3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_imm4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_imm5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_imm6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_imm7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_imm8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_imm9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_imm10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_imm11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_imm12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_imm13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_EOR_imm14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_TEQ_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_imm0()
{
	unsigned int flags;
	unsigned int immd;
	flags = inst.getThumb32Bytes()->data_proc_immd.sign;

	immd = (inst.getThumb32Bytes()->data_proc_immd.i_flag << 11)
		| (inst.getThumb32Bytes()->data_proc_immd.immd3 << 8)
		| (inst.getThumb32Bytes()->data_proc_immd.immd8);

	immd = IsaThumbImmdExtend(immd);

	IsaThumbAdd(inst.getThumb32Bytes()->data_proc_immd.rd,
		inst.getThumb32Bytes()->data_proc_immd.rn, immd, 0, flags);

}

void Context::ExecuteInstThumb32_ADD_imm1()
{
	unsigned int flags;
	unsigned int immd;
	flags = inst.getThumb32Bytes()->data_proc_immd.sign;

	immd = (inst.getThumb32Bytes()->data_proc_immd.i_flag << 11)
		| (inst.getThumb32Bytes()->data_proc_immd.immd3 << 8)
		| (inst.getThumb32Bytes()->data_proc_immd.immd8);
	immd = IsaThumbImmdExtend(immd);
	IsaThumbAdd(inst.getThumb32Bytes()->data_proc_immd.rd,
		inst.getThumb32Bytes()->data_proc_immd.rn, immd, 0, flags);
}

void Context::ExecuteInstThumb32_ADD_imm2()
{
	unsigned int flags;
	unsigned int immd;
	flags = inst.getThumb32Bytes()->data_proc_immd.sign;

	immd = (inst.getThumb32Bytes()->data_proc_immd.i_flag << 11)
		| (inst.getThumb32Bytes()->data_proc_immd.immd3 << 8)
		| (inst.getThumb32Bytes()->data_proc_immd.immd8);
	immd = IsaThumbImmdExtend(immd);
	IsaThumbAdd(inst.getThumb32Bytes()->data_proc_immd.rd,
		inst.getThumb32Bytes()->data_proc_immd.rn, immd, 0, flags);

}

void Context::ExecuteInstThumb32_ADD_imm3()
{
	unsigned int flags;
	unsigned int immd;
	flags = inst.getThumb32Bytes()->data_proc_immd.sign;

	immd = (inst.getThumb32Bytes()->data_proc_immd.i_flag << 11)
		| (inst.getThumb32Bytes()->data_proc_immd.immd3 << 8)
		| (inst.getThumb32Bytes()->data_proc_immd.immd8);
	immd = IsaThumbImmdExtend(immd);
	IsaThumbAdd(inst.getThumb32Bytes()->data_proc_immd.rd,
		inst.getThumb32Bytes()->data_proc_immd.rn, immd, 0, flags);


}

void Context::ExecuteInstThumb32_ADD_imm4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_imm5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_imm6()
{
	unsigned int flags;
	unsigned int immd;
	flags = inst.getThumb32Bytes()->data_proc_immd.sign;

	immd = (inst.getThumb32Bytes()->data_proc_immd.i_flag << 11)
		| (inst.getThumb32Bytes()->data_proc_immd.immd3 << 8)
		| (inst.getThumb32Bytes()->data_proc_immd.immd8);
	immd = IsaThumbImmdExtend(immd);
	IsaThumbAdd(inst.getThumb32Bytes()->data_proc_immd.rd,
		inst.getThumb32Bytes()->data_proc_immd.rn, immd, 0, flags);

}

void Context::ExecuteInstThumb32_ADD_imm7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_imm8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_imm9()
{
	unsigned int flags;
	unsigned int immd;
	flags = inst.getThumb32Bytes()->data_proc_immd.sign;

	immd = (inst.getThumb32Bytes()->data_proc_immd.i_flag << 11)
		| (inst.getThumb32Bytes()->data_proc_immd.immd3 << 8)
		| (inst.getThumb32Bytes()->data_proc_immd.immd8);
	immd = IsaThumbImmdExtend(immd);
	IsaThumbAdd(inst.getThumb32Bytes()->data_proc_immd.rd,
		inst.getThumb32Bytes()->data_proc_immd.rn, immd, 0, flags);


}

void Context::ExecuteInstThumb32_ADD_imm10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_imm11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_imm12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_imm13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_imm14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_CMN_imm()
{

	int rn_val;
	int operand2;
	int result;
	int op2;
	int rd_val;
	unsigned long flags;

	flags = 0;


	IsaRegLoad(inst.getThumb32Bytes()->data_proc_immd.rn, rn_val);
	operand2 = (inst.getThumb32Bytes()->data_proc_immd.i_flag << 11)
		| (inst.getThumb32Bytes()->data_proc_immd.immd3 << 8)
		| (inst.getThumb32Bytes()->data_proc_immd.immd8);

	result = rn_val + operand2;
	emulator->isa_debug << misc::fmt("  result = %d ; 0x%x\n", result, result);

	op2 = operand2;
	regs.getCPSR().z = 0;
	regs.getCPSR().n = 0;
	regs.getCPSR().C = 0;
	regs.getCPSR().v = 0;

	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"add %3, %%eax\n\t"
		"mov %%eax, %4\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (rn_val)
		  : "m" (rn_val), "m" (op2), "m" (rd_val), "g" (flags)
		    : "eax"
	);


	if(flags & 0x00000001)
	{
		regs.getCPSR().C = 1;
	}
	if(flags & 0x00008000)
	{
		regs.getCPSR().v = 1;
	}
	if(flags & 0x00000040)
	{
		regs.getCPSR().z = 1;
	}
	if(flags & 0x00000080)
	{
		regs.getCPSR().n = 1;
	}
	IsaCpsrPrint();

}

void Context::ExecuteInstThumb32_SUB_imm0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_imm1()
{
	unsigned int flags;
	unsigned int immd;
	flags = inst.getThumb32Bytes()->data_proc_immd.sign;

	immd = (inst.getThumb32Bytes()->data_proc_immd.i_flag << 11)
		| (inst.getThumb32Bytes()->data_proc_immd.immd3 << 8)
		| (inst.getThumb32Bytes()->data_proc_immd.immd8);
	IsaThumbSubtract(inst.getThumb32Bytes()->data_proc_immd.rd,
		inst.getThumb32Bytes()->data_proc_immd.rn, immd, 0, flags);
}

void Context::ExecuteInstThumb32_SUB_imm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_imm3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_imm4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_imm5()
{
	unsigned int flags;
	unsigned int immd;
	flags = inst.getThumb32Bytes()->data_proc_immd.sign;

	immd = (inst.getThumb32Bytes()->data_proc_immd.i_flag << 11)
		| (inst.getThumb32Bytes()->data_proc_immd.immd3 << 8)
		| (inst.getThumb32Bytes()->data_proc_immd.immd8);
	IsaThumbSubtract(inst.getThumb32Bytes()->data_proc_immd.rd,
		inst.getThumb32Bytes()->data_proc_immd.rn, immd, 0, flags);
}

void Context::ExecuteInstThumb32_SUB_imm6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_imm7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_imm8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_imm9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_imm10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_imm11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_imm12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_imm13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_imm14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_CMP_imm()
{
	unsigned long flags;
	unsigned int immd;
	int rn_val;
	int rd_val;
	int result;
	int operand2;
	int op2;




	flags = inst.getThumb32Bytes()->data_proc_immd.sign;

	immd = (inst.getThumb32Bytes()->data_proc_immd.i_flag << 11)
		| (inst.getThumb32Bytes()->data_proc_immd.immd3 << 8)
		| (inst.getThumb32Bytes()->data_proc_immd.immd8);
	immd = IsaThumbImmdExtend(immd);
	operand2 = immd;
	IsaRegLoad(inst.getThumb32Bytes()->data_proc_immd.rn, rn_val);
	result = rn_val - operand2;
	emulator->isa_debug << misc::fmt("  result = %d ; 0x%x\n", result, result);

	op2 = (-1 * operand2);

	regs.getCPSR().z = 0;
	regs.getCPSR().n = 0;
	regs.getCPSR().C = 0;
	regs.getCPSR().v = 0;

	asm volatile (
		"push %5\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"add %%eax, %3\n\t"
		"mov %3, %%eax\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags), "=m" (op2)
		  : "m" (op2), "m" (rn_val), "m" (rd_val), "g" (flags)
		    : "eax"
	);

	emulator->isa_debug << misc::fmt("  flags = 0x%lx\n", flags);
	if(flags & 0x00000001)
	{
		regs.getCPSR().C = 1;
	}
	if(flags & 0x00008000)
	{
		regs.getCPSR().v = 1;
	}
	if(flags & 0x00000040)
	{
		regs.getCPSR().z = 1;
	}
	if(flags & 0x00000080)
	{
		regs.getCPSR().n = 1;
	}

	if((operand2 == 0) && (rn_val == 0))
	{
		regs.getCPSR().C = 1;
	}

	if(operand2 == 0)
	{
		regs.getCPSR().C = 1;
	}


	IsaCpsrPrint();

}

void Context::ExecuteInstThumb32_RSB_imm()
{
	unsigned int flags;
	unsigned int immd;
	flags = inst.getThumb32Bytes()->data_proc_immd.sign;

	immd = (inst.getThumb32Bytes()->data_proc_immd.i_flag << 11)
		| (inst.getThumb32Bytes()->data_proc_immd.immd3 << 8)
		| (inst.getThumb32Bytes()->data_proc_immd.immd8);
	IsaThumbRevSubtract(inst.getThumb32Bytes()->data_proc_immd.rd,
		inst.getThumb32Bytes()->data_proc_immd.rn, immd, 0, flags);
}

void Context::ExecuteInstThumb32_SBC_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADC_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_binimm0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_binimm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_binimm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_binimm3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_binimm4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_binimm5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_binimm6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_binimm7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_binimm8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_binimm9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_binimm10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_binimm11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_binimm12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_binimm13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ADD_binimm14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_binimm0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_binimm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_binimm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_binimm3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_binimm4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_binimm5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_binimm6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_binimm7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_binimm8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_binimm9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_binimm10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_binimm11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_binimm12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_binimm13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SUB_binimm14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFI_binimm0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFI_binimm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFI_binimm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFI_binimm3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFI_binimm4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFI_binimm5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFI_binimm6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFI_binimm7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFI_binimm8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFI_binimm9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFI_binimm10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFI_binimm11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFI_binimm12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFI_binimm13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFI_binimm14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BFC_binimm()
{
	unsigned int msb;
	unsigned int immd2;
	unsigned int immd3;
	unsigned int lsb;
	int rd_val;

	int i_mask;


	msb = inst.getThumb32Bytes()->bit_field.msb;
	immd2 = inst.getThumb32Bytes()->bit_field.immd2;
	immd3 = inst.getThumb32Bytes()->bit_field.immd3;

	lsb = (immd3 << 2) | immd2;
	emulator->isa_debug << misc::fmt("  msb = 0x%x, lsb = 0x%x\n", msb, lsb);
	IsaRegLoad(inst.getThumb32Bytes()->bit_field.rd, rd_val);
	for (unsigned int i = lsb; i <= msb; i++)
	{
		i_mask = 1 << i;
		rd_val = rd_val & (~(i_mask));
	}
	IsaRegStore(inst.getThumb32Bytes()->bit_field.rd, rd_val);
}

void Context::ExecuteInstThumb32_UBFX_binimm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SBFX_binimm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MOVT_binimm()
{
	unsigned int immd16;
	unsigned int immd8;
	unsigned int immd3;
	unsigned int i;
	unsigned int immd4;
	int rd_val;



	IsaRegLoad(inst.getThumb32Bytes()->bit_field.rd, rd_val);

	Instruction::Thumb32Category cat = inst.getInstThumb32Info()->cat32;

		if (cat == Instruction::Thumb32CategoryDprBinImm)
		{
			immd8 = inst.getThumb32Bytes()->data_proc_immd.immd8;
			immd3 = inst.getThumb32Bytes()->data_proc_immd.immd3;
			i = inst.getThumb32Bytes()->data_proc_immd.i_flag;
			immd4 = inst.getThumb32Bytes()->data_proc_immd.rn;
		}

		else
			throw misc::Panic(misc::fmt("%d: immd16 fmt not recognized", cat));

		immd16 = (immd4 << 12) | (i << 11) | (immd3 << 8) | immd8;

	emulator->isa_debug << misc::fmt("  immd16 = 0x%x\n", immd16);
	IsaRegLoad(inst.getThumb32Bytes()->data_proc_immd.rd, rd_val);
	emulator->isa_debug << misc::fmt("  rd_val = 0x%x\n", rd_val);
	rd_val = (rd_val & 0x0000ffff) | (immd16 << 16);
	IsaRegStore(inst.getThumb32Bytes()->data_proc_immd.rd, rd_val);
}

void Context::ExecuteInstThumb32_MOVW_binimm()
{
	unsigned int immd16;
	unsigned int immd8;
	unsigned int immd3;
	unsigned int i;
	unsigned int immd4;





	Instruction::Thumb32Category cat = inst.getInstThumb32Info()->cat32;

		if (cat == Instruction::Thumb32CategoryDprBinImm)
		{
			immd8 = inst.getThumb32Bytes()->data_proc_immd.immd8;
			immd3 = inst.getThumb32Bytes()->data_proc_immd.immd3;
			i = inst.getThumb32Bytes()->data_proc_immd.i_flag;
			immd4 = inst.getThumb32Bytes()->data_proc_immd.rn;
		}

		else
			throw misc::Panic(misc::fmt("%d: immd16 fmt not recognized", cat));

		immd16 = (immd4 << 12) | (i << 11) | (immd3 << 8) | immd8;

	emulator->isa_debug << misc::fmt("  immd32 = %d\n", immd16);
	IsaRegStore(inst.getThumb32Bytes()->data_proc_immd.rd, immd16);

}

void Context::ExecuteInstThumb32_B_0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_B_01()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_B_02()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_B_03()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_B_04()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_B_05()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_B_06()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_B_1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_B_2()
{
	IsaThumb32Branch();
}

void Context::ExecuteInstThumb32_B_21()
{
	IsaThumb32Branch();
}

void Context::ExecuteInstThumb32_B_22()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_B_23()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_B_24()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_B_25()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_B_26()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_B_3()
{
	IsaThumb32Branch();
}

void Context::ExecuteInstThumb32_NOP_0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_NOP_1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BL_0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BL_1()
{
	IsaThumb32BranchLink();
}

void Context::ExecuteInstThumb32_BLX_0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_BLX_1()
{
	IsaThumb32BranchLink();
}

void Context::ExecuteInstThumb32_STRB_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRB_immd()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRBT_immd()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRH_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRHT_immd0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRH_immd()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STR_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRHT_immd()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STR_immd()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRB_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRB_immd2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRBT_immd2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRH_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRHT_immd02()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRH_immd2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STR_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRHT_immd2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STR_immd2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRB_reg4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRB_immd4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRBT_immd4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRH_reg4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRHT_immd04()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRH_immd4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STR_reg4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRHT_immd4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STR_immd4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRB_reg6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRB_immd66()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRBT_immd6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRH_reg6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRHT_immd06()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRH_immd6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STR_reg6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRHT_immd6()
{
	int offset;
	int value;
	void *buf;
	int immd12;
	unsigned int idx;
	unsigned int wback;
	unsigned int add;
	int rn_val;
	unsigned int addr;

	buf = &value;
	immd12 = inst.getThumb32Bytes()->ldstr_imm.immd12;
	idx = (immd12 & 0x00000400) >> 10;
	add = (immd12 & 0x00000200) >> 9;
	wback = (immd12 & 0x00000100) >> 8;

	IsaRegLoad(inst.getThumb32Bytes()->ldstr_imm.rn, rn_val);
	if(add)
		offset = rn_val + (immd12 & 0x000000ff);
	else
		offset = rn_val - (immd12 & 0x000000ff);

	if(idx)
		addr = offset;
	else
		addr = rn_val;
	emulator->isa_debug << misc::fmt(" immd12 = %x; offset = %x;  addr  = %d; (0x%x)\n", immd12, offset, addr, addr);
	if(wback)
		IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rn, offset);

	IsaRegLoad(inst.getThumb32Bytes()->ldstr_imm.rd, value);
	value = value & (0xffffffff);
	memory->Write(addr, 4, (char *)buf);
}

void Context::ExecuteInstThumb32_STR_immd6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRB_immda()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRB_immdb()
{
	int offset;
	int value;
	void *buf;
	int immd12;
	int rn_val;
	unsigned int addr;

	buf = &value;
	immd12 = inst.getThumb32Bytes()->ldstr_imm.immd12;
	IsaRegLoad(inst.getThumb32Bytes()->ldstr_imm.rn, rn_val);
	offset = rn_val + (immd12);
	addr = offset;
	emulator->isa_debug << misc::fmt(" immd12 = %x; offset = %x;  addr  = %d; (0x%x)\n", immd12, offset, addr, addr);

	IsaRegLoad(inst.getThumb32Bytes()->ldstr_imm.rd, value);
	value = value & (0x000000ff);
	memory->Write(addr, 1, (char *)buf);

}

void Context::ExecuteInstThumb32_STRB_immdc()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRB_immdd()
{
	int offset;
	int value;
	void *buf;
	int immd12;
	int rn_val;
	unsigned int addr;

	buf = &value;
	immd12 = inst.getThumb32Bytes()->ldstr_imm.immd12;
	IsaRegLoad(inst.getThumb32Bytes()->ldstr_imm.rn, rn_val);
	offset = rn_val + (immd12);
	addr = offset;
	emulator->isa_debug << misc::fmt(" immd12 = %x; offset = %x;  addr  = %d; (0x%x)\n", immd12, offset, addr, addr);

	IsaRegLoad(inst.getThumb32Bytes()->ldstr_imm.rd, value);
	value = value & (0x000000ff);
	memory->Write(addr, 1, (char *)buf);
}

void Context::ExecuteInstThumb32_STRH_immda()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRH_immdb()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRH_immdc()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STRH_immdd()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STR_immda()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STR_immdb()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STR_immdc()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STR_immdd()
{
	unsigned int immd12;
	int rn_val;
	int addr;
	int rd_val;
	void *buf;

	buf = &rd_val;
	immd12 = inst.getThumb32Bytes()->ldstr_imm.immd12;

	IsaRegLoad(inst.getThumb32Bytes()->ldstr_imm.rn,
				rn_val);

	addr = rn_val + immd12;

	IsaRegLoad(inst.getThumb32Bytes()->ldstr_imm.rd,
					rd_val);

	emulator->isa_debug << misc::fmt(" r%d (0x%x) => [0x%x]\n",
		inst.getThumb32Bytes()->ldstr_imm.rd, rd_val, addr);

	memory->Write(addr, 4, (char *)buf);
}

void Context::ExecuteInstThumb32_LDRB_lit()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_reg0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm40()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm30()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm50()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm70()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm41()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm31()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm51()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm71()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm42()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm32()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm52()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm72()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm43()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm33()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm53()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm73()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_reg4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm44()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm34()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm54()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm74()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_reg5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm45()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm15()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm35()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm55()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm75()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_reg6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm46()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm16()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm36()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm56()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm76()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_reg7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm47()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm17()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm37()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm57()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm77()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_reg8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm48()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm18()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm38()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm58()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm78()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_reg9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm49()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm19()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm39()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm59()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm79()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_reg10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm410()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm110()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm310()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm510()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm710()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_reg11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm411()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm111()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm311()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm511()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm711()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_reg12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm412()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm112()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm312()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm512()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm712()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_reg13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm413()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm113()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm313()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm513()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm713()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_reg14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm414()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm114()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm314()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm514()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm714()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_imm715()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_lit()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_reg0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm40()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm30()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm50()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm70()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm41()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm31()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm51()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm71()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm42()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm32()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm52()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm72()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm43()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm33()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm53()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm73()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_reg4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm44()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm34()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm54()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm74()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_reg5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm45()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm15()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm35()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm55()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm75()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_reg6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm46()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm16()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm36()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm56()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm76()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_reg7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm47()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm17()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm37()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm57()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm77()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_reg8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm48()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm18()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm38()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm58()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm78()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_reg9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm49()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm19()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm39()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm59()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm79()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_reg10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm410()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm110()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm310()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm510()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm710()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_reg11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm411()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm111()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm311()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm511()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm711()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_reg12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm412()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm112()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm312()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm512()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm712()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_reg13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm413()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm113()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm313()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm513()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm713()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_reg14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm414()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm114()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm314()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm514()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_imm714()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_regA0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA40()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA30()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA50()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA70()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_regA1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA41()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA31()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA51()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA71()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_regA2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA42()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA32()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA52()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA72()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_regA3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA43()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA33()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA53()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA73()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_regA4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA44()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA34()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA54()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA74()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_regA5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA45()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA15()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA35()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA55()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA75()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_regA6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA46()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA16()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA36()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA56()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA76()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_regA7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA47()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA17()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA37()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA57()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA77()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_regA8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA48()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA18()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA38()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA58()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA78()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_regA9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA49()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA19()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA39()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA59()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA79()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_regA10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA410()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA110()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA310()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA510()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA710()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_regA11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA411()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA111()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA311()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA511()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA711()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_regA12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA412()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA112()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA312()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA512()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA712()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_regA13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA413()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA113()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA313()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA513()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA713()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_regA14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA414()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA114()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA314()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA514()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immA714()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_regA0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA40()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA30()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA50()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA70()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_regA1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA41()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA31()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA51()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA71()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_regA2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA42()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA32()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA52()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA72()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_regA3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA43()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA33()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA53()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA73()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_regA4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA44()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA34()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA54()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA74()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_regA5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA45()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA15()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA35()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA55()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA75()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_regA6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA46()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA16()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA36()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA56()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA76()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_regA7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA47()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA17()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA37()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA57()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA77()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_regA8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA48()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA18()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA38()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA58()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA78()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_regA9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA49()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA19()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA39()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA59()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA79()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_regA10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA410()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA110()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA310()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA510()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA710()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_regA11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA411()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA111()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA311()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA511()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA711()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_regA12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA412()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA112()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA312()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA512()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA712()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_regA13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA413()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA113()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA313()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA513()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA713()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_regA14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA414()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA114()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA314()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA514()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA714()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_litA()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRB_immA7140()
{
	int offset;
	int value;
	void *buf;
	int immd12;
	int rn_val;
	unsigned int addr;

	buf = &value;
	immd12 = inst.getThumb32Bytes()->ldstr_imm.immd12;
	IsaRegLoad(inst.getThumb32Bytes()->ldstr_imm.rn, rn_val);
	offset = rn_val + (immd12);
	addr = offset;
	emulator->isa_debug << misc::fmt(" immd12 = 0x%x; offset = 0x%x;  addr  = %d; (0x%x)\n", immd12, offset, addr, addr);
	memory->Read(addr, 1, (char *)buf);
	value = value & (0x000000ff);
	if(inst.getThumb32Bytes()->ldstr_imm.rd < 15)
	{
		IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rd, value);
	}
	else
	{
		if(value % 2)
			IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rd, value - 3);
		else
			IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rd, value - 2);
	}
}

void Context::ExecuteInstThumb32_LDRSB_litA()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immB1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSB_immB2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRH_reg0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRH_imm0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSH_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSH_imm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRH_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRH_imm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRH_imm3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRH_imm4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSH_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSH_imm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSH_imm3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDRSH_imm4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDR_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDR_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDR_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDR_imm2()
{
	int offset;
	int value;
	void *buf;
	int immd12;
	unsigned int idx;
	unsigned int wback;
	unsigned int add;
	int rn_val;
	unsigned int addr;

	buf = &value;
	immd12 = inst.getThumb32Bytes()->ldstr_imm.immd12;
	idx = (immd12 & 0x00000400) >> 10;
	add = (immd12 & 0x00000200) >> 9;
	wback = (immd12 & 0x00000100) >> 8;

	if(inst.getThumb32Bytes()->ldstr_imm.rn < 15)
	{
		IsaRegLoad(inst.getThumb32Bytes()->ldstr_imm.rn, rn_val);
		if(add)
			offset = rn_val + (immd12 & 0x000000ff);
		else
			offset = rn_val - (immd12 & 0x000000ff);

		if(idx)
			addr = offset;
		else
			addr = rn_val;
		emulator->isa_debug << misc::fmt("imm2  addr  = %d; (0x%x)\n", addr, addr);
		if(wback)
			IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rn, offset);

		memory->Read(addr, 4, (char *)buf);
		value = value & (0xffffffff);

		if(inst.getThumb32Bytes()->ldstr_imm.rd < 15)
			IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rd, value);
		else
		{
			if(value % 2)
				IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rd, value - 3);
			else
				IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rd, value - 2);
		}
	}
	else if (inst.getThumb32Bytes()->ldstr_imm.rn == 15)
	{
		emulator->isa_debug << misc::fmt("  pc  = 0x%x; \n", regs.getPC());
		if((regs.getPC() - 4) % 4 == 2)
			offset = (regs.getPC() - 4) + 2;
		else
			offset = regs.getPC() - 4;
		emulator->isa_debug << misc::fmt("  offset  = 0x%x; \n", offset);
		if(inst.getThumb32Bytes()->ldstr_imm.add)
			addr = offset + immd12;
		else
			addr = offset - immd12;

		memory->Read(addr, 4, (char *)buf);
		value = value & (0xffffffff);
		if(inst.getThumb32Bytes()->ldstr_imm.rd < 15)
			IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rd, value);
		else
		{
			if(value % 2)
				IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rd, value - 3);
			else
				IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rd, value - 2);
		}

	}
}

void Context::ExecuteInstThumb32_LDR_imm3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LDR_imm4()
{
	int offset;
	int value;
	void *buf;
	int immd12;
	unsigned int idx;
	unsigned int wback;
	unsigned int add;
	int rn_val;
	unsigned int addr;

	buf = &value;
	immd12 = inst.getThumb32Bytes()->ldstr_imm.immd12;
	idx = (immd12 & 0x00000400) >> 10;
	add = (immd12 & 0x00000200) >> 9;
	wback = (immd12 & 0x00000100) >> 8;

	if(inst.getThumb32Bytes()->ldstr_imm.rn < 15)
	{
		IsaRegLoad(inst.getThumb32Bytes()->ldstr_imm.rn, rn_val);
		if(add)
			offset = rn_val + (immd12 & 0x000000ff);
		else
			offset = rn_val - (immd12 & 0x000000ff);

		if(idx)
			addr = offset;
		else
			addr = rn_val;
		emulator->isa_debug << misc::fmt("imm4  addr  = %d; (0x%x)\n", addr, addr);
		if(wback)
			IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rn, offset);

		memory->Read(addr, 4, (char *)buf);
		value = value & (0xffffffff);

		if(inst.getThumb32Bytes()->ldstr_imm.rd < 15)
			IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rd, value);
		else
		{
			if(value % 2)
				IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rd, value - 3);
			else
				IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rd, value - 2);
		}
	}
	else if (inst.getThumb32Bytes()->ldstr_imm.rn == 15)
	{
		emulator->isa_debug << misc::fmt("  pc  = 0x%x; \n", regs.getPC());
		if((regs.getPC() - 4) % 4 == 2)
			offset = (regs.getPC() - 4) + 2;
		else
			offset = regs.getPC() - 4;
		emulator->isa_debug << misc::fmt("  offset  = 0x%x; \n", offset);
		if(inst.getThumb32Bytes()->ldstr_imm.add)
			addr = offset + immd12;
		else
			addr = offset - immd12;

		memory->Read(addr, 4, (char *)buf);
		value = value & (0xffffffff);
		if(inst.getThumb32Bytes()->ldstr_imm.rd < 15)
			IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rd, value);
		else
		{
			if(value % 2)
				IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rd, value - 3);
			else
				IsaRegStore(inst.getThumb32Bytes()->ldstr_imm.rd, value - 2);
		}

	}

}

void Context::ExecuteInstThumb32_MUL()
{
	int rm_val;
	int rn_val;

	int result;

	IsaRegLoad(inst.getThumb32Bytes()->mult.rn, rn_val);
	IsaRegLoad(inst.getThumb32Bytes()->mult.rm, rm_val);

	result = rn_val * rm_val;

	IsaRegStore(inst.getThumb32Bytes()->mult.rd, result);

}

void Context::ExecuteInstThumb32_MLA0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MLA1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MLA2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MLA3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MLA4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MLA5()
{
	int rm_val;
	int rn_val;
	int ra_val;


	int result;

	IsaRegLoad(inst.getThumb32Bytes()->mult.rn, rn_val);
	IsaRegLoad(inst.getThumb32Bytes()->mult.rm, rm_val);
	IsaRegLoad(inst.getThumb32Bytes()->mult.ra, ra_val);

	result = (rn_val * rm_val) + ra_val;

	IsaRegStore(inst.getThumb32Bytes()->mult.rd, result);

}

void Context::ExecuteInstThumb32_MLA6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MLA7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MLA8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MLA9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MLA10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MLA11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MLA12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MLA13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MLA14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MLS()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SMULL()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UMULL()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LSL_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LSL_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LSR_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_LSR_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ASR_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_ASR_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_SXTH_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTH_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTAH_reg0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTAH_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTAH_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTAH_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTAH_reg4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTAH_reg5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTAH_reg6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTAH_reg7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTAH_reg8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTAH_reg9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTAH_reg10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTAH_reg11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTAH_reg12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTAH_reg13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTAH_reg14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UXTB_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UADD8_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_REV_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_CLZ_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UADD8_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_REV_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_CLZ_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UADD8_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_REV_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_CLZ_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_UADD8_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_REV_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_CLZ_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_MRC()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb32_STLDC()
{
	throw misc::Panic("Unimplemented instruction");
}

} // namespace ARM
