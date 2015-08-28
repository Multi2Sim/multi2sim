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

Context::ExecuteInstThumb16Fn Context::execute_inst_thumb16_fn[Instruction::Thumb16OpcodeCount] =
{
	nullptr  // For Instruction::Thumb16OpcodeNone
#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6) \
		 , &Context::ExecuteInstThumb16_##_name
#include <arch/arm/disassembler/InstructionThumb.def>
#undef DEFINST
};


void Context::ExecuteInstThumb16_LSL_imm()
{
	// Local variable declaration
	int rm_val;

	// Get the shift offset from immediate
	unsigned int immd = inst.getThumb16Bytes()->movshift_reg_ins.offset;

	// Get the current CPSR register status
	regsPsr prev_regs_cpsr = regs.getCPSR();

	// Load register to local variable from register reg_rs
	IsaRegLoad(inst.getThumb16Bytes()->movshift_reg_ins.reg_rs, rm_val);

	// Left shift the value just loaded by immd
	rm_val = rm_val << immd;

	// Store the value to register reg_rd
	IsaRegStore(inst.getThumb16Bytes()->movshift_reg_ins.reg_rd, rm_val);

	// Reset the CPSR flag z/n/C/v
	regs.getCPSR().z = 0;
	regs.getCPSR().n = 0;
	regs.getCPSR().C = 0;
	regs.getCPSR().v = 0;

	// Update the CPSR flag
	if (rm_val == 0)
	{
		regs.getCPSR().z = 1;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;
	}
	if (rm_val < 0)
	{
		regs.getCPSR().n = 1;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;
	}
	regs.getCPSR().C = prev_regs_cpsr.C;
}

void Context::ExecuteInstThumb16_LSR_imm()
{
	// Local variable declaration
	int rm_val;

	// Get the current CPSR register status
	regsPsr prev_regs_cpsr = regs.getCPSR();

	// Get the shift offset from immediate
	unsigned int immd = inst.getThumb16Bytes()->movshift_reg_ins.offset;

	// Load register to local variable from register reg_rs
	IsaRegLoad(inst.getThumb16Bytes()->movshift_reg_ins.reg_rs, rm_val);

	// Right shift the value just loaded by immd
	rm_val = rm_val >> immd;

	// Store the value to register reg_rd
	IsaRegStore(inst.getThumb16Bytes()->movshift_reg_ins.reg_rd, rm_val);

	// Reset the CPSR flag z/n/C/v
	regs.getCPSR().z = 0;
	regs.getCPSR().n = 0;
	regs.getCPSR().C = 0;
	regs.getCPSR().v = 0;

	// Update the CPSR flag
	if (rm_val == 0)
	{
		regs.getCPSR().z = 1;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;
	}
	if (rm_val < 0)
	{
		regs.getCPSR().n = 1;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;
	}
	regs.getCPSR().C = prev_regs_cpsr.C;
}

void Context::ExecuteInstThumb16_ASR_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_MOV_imm()
{
	// Get the current CPSR register status
	regsPsr prev_regs_cpsr = regs.getCPSR();

	// Get the shift offset from immediate
	int operand2 = inst.getThumb16Bytes()->immd_oprs_ins.offset8;

	// Move the value to register reg_rd
	IsaRegStore(inst.getThumb16Bytes()->immd_oprs_ins.reg_rd, operand2);

	// Reset the CPSR flag z/n/C/v
	regs.getCPSR().z = 0;
	regs.getCPSR().n = 0;
	regs.getCPSR().C = 0;
	regs.getCPSR().v = 0;

	// Update the CPSR flag
	if (operand2 == 0)
	{
		regs.getCPSR().z = 1;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;
	}
	if (operand2 < 0)
	{
		regs.getCPSR().n = 1;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;
	}
	regs.getCPSR().C = prev_regs_cpsr.C;

	// Print out the CPSR status
	IsaCpsrPrint();
}

void Context::ExecuteInstThumb16_CMP_imm()
{
	// Local variable declaration
	int rn_val;
	int rd_val;
	unsigned long flags = 0;

	// Get operand 2 from the instruction info
	int operand2 = inst.getThumb16Bytes()->immd_oprs_ins.offset8;

	// Load the value of register reg_rd to local value rn_val
	IsaRegLoad(inst.getThumb16Bytes()->immd_oprs_ins.reg_rd, rn_val);

	// Calculate the result of subtracting rn_val by operand 2
	int result = rn_val - operand2;
	emulator->isa_debug << misc::fmt("  result = %d ; 0x%x\n", result, result);

	// Get the negative value of operand 2
	int op2 = (-1 * operand2);

	// Reset the CPSR flag z/n/C/v
	regs.getCPSR().z = 0;
	regs.getCPSR().n = 0;
	regs.getCPSR().C = 0;
	regs.getCPSR().v = 0;

	// Embedded assembly code
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

	// Update the CPSR based on the flag
	emulator->isa_debug << misc::fmt("  flags = 0x%lx\n", flags);
	if (flags & 0x00000001)
	{
		regs.getCPSR().C = 1;
	}
	if (flags & 0x00008000)
	{
		regs.getCPSR().v = 1;
	}
	if (flags & 0x00000040)
	{
		regs.getCPSR().z = 1;
	}
	if (flags & 0x00000080)
	{
		regs.getCPSR().n = 1;
	}
	if ((operand2 == 0) && (rn_val == 0))
	{
		regs.getCPSR().C = 1;
	}
	if (operand2 == 0)
	{
		regs.getCPSR().C = 1;
	}

	// Print out the CPSR status
	IsaCpsrPrint();

}

void Context::ExecuteInstThumb16_ADD_imm()
{
	// Get the immediate
	unsigned int immd = inst.getThumb16Bytes()->immd_oprs_ins.offset8;

	// Perform add
	IsaThumbAdd(inst.getThumb16Bytes()->immd_oprs_ins.reg_rd,
		inst.getThumb16Bytes()->immd_oprs_ins.reg_rd, immd, 0, 1);

}

void Context::ExecuteInstThumb16_SUB_imm()
{
	// Get the immediate
	unsigned int immd = inst.getThumb16Bytes()->immd_oprs_ins.offset8;

	// Perform subtract
	IsaThumbSubtract(inst.getThumb16Bytes()->immd_oprs_ins.reg_rd,
		inst.getThumb16Bytes()->immd_oprs_ins.reg_rd, immd, 0, 1);
}

void Context::ExecuteInstThumb16_ADD_reg()
{
	// Local variable declaration
	int rn_val;

	// Load the register rn_imm to local varaible rn_val
	IsaRegLoad(inst.getThumb16Bytes()->addsub_ins.rn_imm, rn_val);

	// Perform add
	IsaThumbAdd(inst.getThumb16Bytes()->addsub_ins.reg_rd,
		inst.getThumb16Bytes()->addsub_ins.reg_rs, rn_val, 0, 1);

}

void Context::ExecuteInstThumb16_SUB_reg()
{
	// Local variable declaration
	int rn_val;

	// Load the register rn_imm to local varaible rn_val
	IsaRegLoad(inst.getThumb16Bytes()->addsub_ins.rn_imm, rn_val);

	// Perform add
	IsaThumbSubtract(inst.getThumb16Bytes()->addsub_ins.reg_rd,
		inst.getThumb16Bytes()->addsub_ins.reg_rs, rn_val, 0, 1);
}

void Context::ExecuteInstThumb16_ADD_immd3()
{
	// Get the immediate
	unsigned int immd = inst.getThumb16Bytes()->addsub_ins.rn_imm;

	// Perform add
	IsaThumbAdd(inst.getThumb16Bytes()->addsub_ins.reg_rd,
		inst.getThumb16Bytes()->addsub_ins.reg_rs, immd, 0, 1);
}

void Context::ExecuteInstThumb16_SUB_immd3()
{
	// Get the immediate
	unsigned int immd = inst.getThumb16Bytes()->addsub_ins.rn_imm;

	// Perform subtract
	IsaThumbSubtract(inst.getThumb16Bytes()->addsub_ins.reg_rd,
		inst.getThumb16Bytes()->addsub_ins.reg_rs, immd, 0, 1);
}

void Context::ExecuteInstThumb16_AND_reg()
{
	// Local variable declaration
	int rn_val;
	int rm_val;

	// Get the current CPSR register status
	regsPsr prev_regs_cpsr = regs.getCPSR();

	// Load register rs/rd value to local variable
	IsaRegLoad(inst.getThumb16Bytes()->dpr_ins.reg_rs, rm_val);
	IsaRegLoad(inst.getThumb16Bytes()->dpr_ins.reg_rd, rn_val);

	// Perform AND operation locally and store the result to register rd
	unsigned int result = rm_val & rn_val;
	IsaRegStore(inst.getThumb16Bytes()->dpr_ins.reg_rd, result);

	// Reset the CPSR flag z/n/C/v
	regs.getCPSR().z = 0;
	regs.getCPSR().n = 0;
	regs.getCPSR().C = 0;
	regs.getCPSR().v = 0;

	// Update the CPSR flag
	if (result == 0)
	{
		regs.getCPSR().z = 1;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;
	}
	if (result < 0)
	{
		regs.getCPSR().n = 1;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;
	}
	regs.getCPSR().C = prev_regs_cpsr.C;

	// Print out the CPSR status
	IsaCpsrPrint();
}

void Context::ExecuteInstThumb16_EOR_reg()
{
	// Local variable declaration
	int rn_val;
	int rm_val;

	// Get the current CPSR register status
	regsPsr prev_regs_cpsr = regs.getCPSR();

	// Load register rs/rd value to local variable
	IsaRegLoad(inst.getThumb16Bytes()->dpr_ins.reg_rs, rm_val);
	IsaRegLoad(inst.getThumb16Bytes()->dpr_ins.reg_rd, rn_val);

	// Perform XOR operation locally and store the result to regist rd
	unsigned int result = rm_val ^ rn_val;
	IsaRegStore(inst.getThumb16Bytes()->dpr_ins.reg_rd, result);

	// Reset the CPSR flag z/n/C/v
	regs.getCPSR().z = 0;
	regs.getCPSR().n = 0;
	regs.getCPSR().C = 0;
	regs.getCPSR().v = 0;

	// Update the CPSR flag
	if (result == 0)
	{
		regs.getCPSR().z = 1;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;
	}
	if (result < 0)
	{
		regs.getCPSR().n = 1;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;
	}
	regs.getCPSR().C = prev_regs_cpsr.C;

	// Print out the CPSR status
	IsaCpsrPrint();
}


void Context::ExecuteInstThumb16_LSL_reg()
{
	// Local variable declaration
	int rn_val;
	int rm_val;

	// Get the current CPSR register status
	regsPsr prev_regs_cpsr = regs.getCPSR();

	// Load register rs/rd value to local variable
	IsaRegLoad(inst.getThumb16Bytes()->dpr_ins.reg_rs, rm_val);
	IsaRegLoad(inst.getThumb16Bytes()->dpr_ins.reg_rd, rn_val);

	// Perform left shift operation locally and store the result to regist rd
	int shift = rm_val & 0x000000ff;
	unsigned int result = rn_val << shift;
	IsaRegStore(inst.getThumb16Bytes()->dpr_ins.reg_rd, result);

	// Reset the CPSR flag z/n/C/v
	regs.getCPSR().z = 0;
	regs.getCPSR().n = 0;
	regs.getCPSR().C = 0;
	regs.getCPSR().v = 0;

	// Update the CPSR flag
	if (result == 0)
	{
		regs.getCPSR().z = 1;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;
	}
	if (result < 0)
	{
		regs.getCPSR().n = 1;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;
	}
	regs.getCPSR().C = prev_regs_cpsr.C;

	// Print out the CPSR status
	IsaCpsrPrint();
}

void Context::ExecuteInstThumb16_LSR_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_ASR_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_ADC_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_SBC_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_ROR_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_TST_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_RSB_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CMP_reg1()
{
	// Local variable declaration
	int rn_val;
	int rm_val;
	int rd_val;
	unsigned long flags = 0;

	// Load register rs/rd value to local variable
	IsaRegLoad(inst.getThumb16Bytes()->dpr_ins.reg_rd, rn_val);
	IsaRegLoad(inst.getThumb16Bytes()->dpr_ins.reg_rs, rm_val);

	// Subtrace rn_val by rm_val to get a local result
	int result = rn_val - rm_val;
	emulator->isa_debug << misc::fmt("  result = %d ; 0x%x\n", result, result);

	// Get the negative value of operand 2
	int op2 = (-1 * rm_val);

	// Reset the CPSR flag z/n/C/v
	regs.getCPSR().z = 0;
	regs.getCPSR().n = 0;
	regs.getCPSR().C = 0;
	regs.getCPSR().v = 0;

	// Embedded assembly code
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

	// Update the CPSR flag
	if (flags & 0x00000001)
	{
		regs.getCPSR().C = 1;
	}
	if (flags & 0x00008000)
	{
		regs.getCPSR().v = 1;
	}
	if (flags & 0x00000040)
	{
		regs.getCPSR().z = 1;
	}
	if (flags & 0x00000080)
	{
		regs.getCPSR().n = 1;
	}
	if ((rm_val == 0) && (rn_val == 0))
	{
		regs.getCPSR().C = 1;
	}

	if (rm_val == 0)
	{
		regs.getCPSR().C = 1;
	}

	// Print out the CPSR status
	IsaCpsrPrint();

}

void Context::ExecuteInstThumb16_CMN_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_ORR_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_MUL_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_BIC_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_MVN_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_ADD_reg_lo()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_ADD_reg_hi1()
{
	// Local variable declaration
	int rs_val;
	int rd_val;

	// Get rn with value of hi1 and register rd together
	int rn = (inst.getThumb16Bytes()->high_oprs_ins.h1 << 3) | 
			(inst.getThumb16Bytes()->high_oprs_ins.reg_rd);

	// Load register rs/rn value to local variable
	IsaRegLoad(inst.getThumb16Bytes()->high_oprs_ins.reg_rs, rs_val);
	IsaRegLoad(rn, rd_val);

	// Perform ADD operation and store the result to register rn
	int result = rd_val + rs_val;
	IsaRegStore(rn, result);
}

void Context::ExecuteInstThumb16_ADD_reg_hi2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CMP_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CMP_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_MOV_reg1()
{
	// Local variable declaration
	int rs_val;

	// Get rn with value of hi1 and register rd together
	int rd = (inst.getThumb16Bytes()->high_oprs_ins.h1 << 3) | 
			(inst.getThumb16Bytes()->high_oprs_ins.reg_rd);

	// Perform MOV operation only when rd is smaller than 15
	IsaRegLoad(inst.getThumb16Bytes()->high_oprs_ins.reg_rs, rs_val);
	if (rd < 15)
	{
		IsaRegStore(rd, rs_val);
	}
}

void Context::ExecuteInstThumb16_MOV_reg2()
{
	// Local variable declaration
	int rs_val;

	// Get rn with value of hi1 and register rd together
	int rd = (inst.getThumb16Bytes()->high_oprs_ins.h1 << 3) | 
			(inst.getThumb16Bytes()->high_oprs_ins.reg_rd);

	// Perform MOV operation only when rd is smaller than 15
	IsaRegLoad(inst.getThumb16Bytes()->high_oprs_ins.reg_rs, rs_val);
	if (rd < 15)
	{
		IsaRegStore(rd, rs_val);
	}
}

void Context::ExecuteInstThumb16_BX()
{
	// Local variable declaration
	int rs_val;
	int addr;

	// Load register rs to local variable
	IsaRegLoad(inst.getThumb16Bytes()->high_oprs_ins.reg_rs, rs_val);

	// Align the address by 2 bytes
	if (rs_val % 2)
		addr = rs_val - 1;
	else
		addr = rs_val;

	// Set Program Counter
	regs.setPC(addr + 2);
}

void Context::ExecuteInstThumb16_BLX()
{
	// Local variable declaration
	int rs_val;
	int addr;

	// Load register rs to local variable
	IsaRegLoad(inst.getThumb16Bytes()->high_oprs_ins.reg_rs, rs_val);

	// Align the address by 2 bytes
	if (rs_val % 2)
		addr = rs_val - 1;
	else
		addr = rs_val;

	// Set Link Register and Program Counter
	regs.setLR(regs.getPC());
	regs.setPC(addr + 2);
}

void Context::ExecuteInstThumb16_LDR_lit1()
{
	// Local variable declaration
	unsigned int offset;
	int value;
	void *buf = &value;

	// Get the immediate
	unsigned int immd32 = inst.getThumb16Bytes()->pcldr_ins.immd_8 << 2;

	// Get the offset according to Program Counter
	emulator->isa_debug << misc::fmt("  pc  = 0x%x; \n", regs.getPC());
	if ((regs.getPC() - 2) % 4 == 2)
		offset = (regs.getPC() - 2) + 2;
	else
		offset = regs.getPC() - 2;
	emulator->isa_debug << misc::fmt("  offset  = 0x%x; \n", offset);

	// Calculate the address and load the value from memory
	// And store it to register rd
	unsigned int addr = offset + (immd32);
	memory->Read(addr, 4, (char *)buf);
	IsaRegStore(inst.getThumb16Bytes()->pcldr_ins.reg_rd, value);
}

void Context::ExecuteInstThumb16_LDR_lit2()
{
	// Local variable declaration
	unsigned int offset;
	int value;
	void *buf = &value;

	// Get the immediate
	unsigned int immd32 = inst.getThumb16Bytes()->pcldr_ins.immd_8 << 2;

	// Get the offset according to Program Counter
	emulator->isa_debug << misc::fmt("  pc  = 0x%x; \n", regs.getPC());
	if ((regs.getPC() - 2) % 4 == 2)
		offset = (regs.getPC() - 2) + 2;
	else
		offset = regs.getPC() - 2;
	emulator->isa_debug << misc::fmt("  offset  = 0x%x; \n", offset);

	// Calculate the address and load the value from memory
	// And store it to register rd
	unsigned int addr = offset + (immd32);
	memory->Read(addr, 4, (char *)buf);
	IsaRegStore(inst.getThumb16Bytes()->pcldr_ins.reg_rd, value);
}

void Context::ExecuteInstThumb16_STR_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STRH_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STRB_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRSB_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDR_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRH_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRB_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRSH_reg1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STR_imm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDR_imm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STRB_imm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRB_imm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STRH_imm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRH_imm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STR_imm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDR_imm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STR_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STRH_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STRB_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRSB_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDR_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRH_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRB_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRSH_reg2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STR_immd3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDR_immd3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STRB_imm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRB_imm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STRH_imm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRH_imm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STR_imm4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDR_imm4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STR_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STRH_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STRB_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRSB_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDR_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRH_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRB_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRSH_reg3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STR_immd5()
{
	// Local variable declaration
	int rn_val;
	int rd_val;
	void *buf = &rd_val;

	// Get immediate
	unsigned int immd5 = inst.getThumb16Bytes()->ldstr_immd_ins.offset << 2;

	// Load register rb to local variable
	IsaRegLoad(inst.getThumb16Bytes()->ldstr_immd_ins.reg_rb, rn_val);

	// Calculate the address by adding rn_val and immd5
	int addr = rn_val + immd5;

	// Load register rd to local variable
	IsaRegLoad(inst.getThumb16Bytes()->ldstr_immd_ins.reg_rd,
			rd_val);
	emulator->isa_debug << misc::fmt(" r%d (0x%x) => [0x%x]\n",
		inst.getThumb16Bytes()->ldstr_immd_ins.reg_rd, rd_val, addr);

	// Store the rd_val to the specific address of memory
	memory->Write(addr, 4, (char *)buf);
}

void Context::ExecuteInstThumb16_LDR_immd5()
{
	// Local variable declaration
	int rn_val;
	int rd_val;
	void *buf = &rd_val;

	// Get immediate
	unsigned int immd5 = inst.getThumb16Bytes()->ldstr_immd_ins.offset << 2;

	// Load register rb to local variable
	IsaRegLoad(inst.getThumb16Bytes()->ldstr_immd_ins.reg_rb, rn_val);

	// Calculate the address by adding rn_val and immd5
	int addr = rn_val + immd5;

	// Load from memory according to the specific address
	// And store it to the register rd
	memory->Read(addr, 4, (char *)buf);
	emulator->isa_debug << misc::fmt(" r%d (0x%x) <= [0x%x]\n",
		inst.getThumb16Bytes()->ldstr_immd_ins.reg_rd, rd_val, addr);
	IsaRegStore(inst.getThumb16Bytes()->ldstr_immd_ins.reg_rd, rd_val);
}

void Context::ExecuteInstThumb16_STRB_immd3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRB_immd3()
{
	// Local variable declaration
	int rn_val;
	int rd_val;
	void *buf = &rd_val;

	// Get immediate
	unsigned int immd5 = inst.getThumb16Bytes()->ldstr_immd_ins.offset << 2;

	// Load register rb to local variable
	IsaRegLoad(inst.getThumb16Bytes()->ldstr_immd_ins.reg_rb, rn_val);

	// Calculate the address by adding rn_val and immd5
	int addr = rn_val + immd5;

	// Load from memory according to the specific address
	// And store it to the register rd
	memory->Read(addr, 1, (char *)buf);
	rd_val = rd_val & 0x000000ff;
	emulator->isa_debug << misc::fmt(" r%d (0x%x) <= [0x%x]\n",
		inst.getThumb16Bytes()->ldstr_immd_ins.reg_rd, rd_val, addr);
	IsaRegStore(inst.getThumb16Bytes()->ldstr_immd_ins.reg_rd, rd_val);
}

void Context::ExecuteInstThumb16_STRH_immd3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDRH_immd3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_STR_imm6()
{
	// Local variable declaration
	int sp_val, rn_val;
	void *buf = &rn_val;

	// Get immediate
	unsigned int immd8 = 4 * inst.getThumb16Bytes()->sp_immd_ins.immd_8;

	// Load SP to local variable
	// Load register rd to local variable
	IsaRegLoad(13, sp_val);
	IsaRegLoad(inst.getThumb16Bytes()->sp_immd_ins.reg_rd,
			rn_val);

	// Calculate the address by adding sp_val and immd8
	// Store the value from register rd to the memory according to the 
	// Calculated memory
	unsigned int addr = sp_val + immd8;
	emulator->isa_debug << misc::fmt(" r%d (0x%x) => [0x%x]\n",
		inst.getThumb16Bytes()->sp_immd_ins.reg_rd, rn_val, addr);
	memory->Write(addr, 4, (char *)buf);
}

void Context::ExecuteInstThumb16_LDR_imm6()
{
	// Local variable declaration
	int sp_val, rn_val;
	void *buf = &rn_val;

	// Get immediate
	unsigned int immd8 = 4 * inst.getThumb16Bytes()->sp_immd_ins.immd_8;

	// Load SP to local variable
	IsaRegLoad(13, sp_val);

	// Calculate the address by adding sp_val and immd8	
	unsigned int addr = sp_val + immd8;

	// Load from memory according to the specific address
	// And store it to the register rd
	emulator->isa_debug << misc::fmt(" r%d (0x%x) <= [0x%x]\n",
		inst.getThumb16Bytes()->sp_immd_ins.reg_rd, rn_val, addr);
	memory->Read(addr, 4, (char *)buf);
	IsaRegStore(inst.getThumb16Bytes()->sp_immd_ins.reg_rd,
			rn_val);
}

void Context::ExecuteInstThumb16_ADR_PC()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_ADD_SP()
{
	// Load SP to local variable sp_val
	int sp_val;
	IsaRegLoad(13, sp_val);

	// Get immediate
	int immd = inst.getThumb16Bytes()->addsp_ins.immd_8 * 4;

	// Get the result by adding sp_val and immd
	// Store the aligned result to register rd
	int result = sp_val + immd;
	if (inst.getThumb16Bytes()->addsp_ins.reg_rd < 15)
	{
		IsaRegStore(inst.getThumb16Bytes()->addsp_ins.reg_rd, result);
	}
	else
	{
		if (result % 2)
		{
			IsaRegStore(inst.getThumb16Bytes()->addsp_ins.reg_rd, result - 3);
		}
		else
		{
			IsaRegStore(inst.getThumb16Bytes()->addsp_ins.reg_rd, result - 2);
		}
	}

}

void Context::ExecuteInstThumb16_STM()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_LDM()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_SVC()
{
	IsaSyscall();
}

void Context::ExecuteInstThumb16_B_0()
{
	// Calculate the target address
	int cond = inst.getThumb16Bytes()->cond_br_ins.cond;
	int immd = inst.getThumb16Bytes()->cond_br_ins.s_offset << 1;
	immd = misc::SignExtend32(immd, 9);
	emulator->isa_debug << misc::fmt("  Offset = %x (%d)\n", immd, immd);
	int addr = regs.getPC() + immd;

	// Set the PC accordingly
	if (IsaThumbCheckCond(cond))
	{
		emulator->isa_debug << misc::fmt("  Branch addr = 0x%x, pc <= %x\n", addr, regs.getPC());
		regs.setPC(addr + 2);
	}
}

void Context::ExecuteInstThumb16_B_1()
{
	// Calculate the target address
	int cond = inst.getThumb16Bytes()->cond_br_ins.cond;
	int immd = inst.getThumb16Bytes()->cond_br_ins.s_offset << 1;
	immd = misc::SignExtend32(immd, 9);
	emulator->isa_debug << misc::fmt("  Offset = %x (%d)\n", immd, immd);
	int addr = regs.getPC() + immd;

	// Set the PC accordingly
	if (IsaThumbCheckCond(cond))
	{
		emulator->isa_debug << misc::fmt("  Branch addr = 0x%x, pc <= %x\n", addr, regs.getPC());
		regs.setPC(addr + 2);
	}
}

void Context::ExecuteInstThumb16_B_2()
{
	int immd;
	int addr;
	int cond;

	cond = inst.getThumb16Bytes()->cond_br_ins.cond;
	immd = inst.getThumb16Bytes()->cond_br_ins.s_offset << 1;
	immd = misc::SignExtend32(immd, 9);
	emulator->isa_debug << misc::fmt("  Offset = %x (%d)\n", immd, immd);
	addr = regs.getPC() + immd;

	if (IsaThumbCheckCond(cond))
	{
		emulator->isa_debug << misc::fmt("  Branch addr = 0x%x, pc <= %x\n", addr, regs.getPC());
		regs.setPC(addr + 2);
	}

}

void Context::ExecuteInstThumb16_B_3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_B_4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_B_5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_B_6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_B_7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_B_8()
{
	int immd;
	int addr;
	int cond;

	cond = inst.getThumb16Bytes()->cond_br_ins.cond;
	immd = inst.getThumb16Bytes()->cond_br_ins.s_offset << 1;
	immd = misc::SignExtend32(immd, 9);
	emulator->isa_debug << misc::fmt("  Offset = %x (%d)\n", immd, immd);
	addr = regs.getPC() + immd;

	if (IsaThumbCheckCond(cond))
	{
		emulator->isa_debug << misc::fmt("  Branch addr = 0x%x, pc <= %x\n", addr, regs.getPC());
		regs.setPC(addr + 2);
	}

}

void Context::ExecuteInstThumb16_B_9()
{
	int immd;
	int addr;
	int cond;

	cond = inst.getThumb16Bytes()->cond_br_ins.cond;
	immd = inst.getThumb16Bytes()->cond_br_ins.s_offset << 1;
	immd = misc::SignExtend32(immd, 9);
	emulator->isa_debug << misc::fmt("  Offset = %x (%d)\n", immd, immd);
	addr = regs.getPC() + immd;

	if (IsaThumbCheckCond(cond))
	{
		emulator->isa_debug << misc::fmt("  Branch addr = 0x%x, pc <= %x\n", addr, regs.getPC());
		regs.setPC(addr + 2);
	}

}

void Context::ExecuteInstThumb16_B_10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_B_11()
{
	int immd;
	int addr;
	int cond;

	cond = inst.getThumb16Bytes()->cond_br_ins.cond;
	immd = inst.getThumb16Bytes()->cond_br_ins.s_offset << 1;
	immd = misc::SignExtend32(immd, 9);
	emulator->isa_debug << misc::fmt("  Offset = %x (%d)\n", immd, immd);
	addr = regs.getPC() + immd;

	if (IsaThumbCheckCond(cond))
	{
		emulator->isa_debug << misc::fmt("  Branch addr = 0x%x, pc <= %x\n", addr, regs.getPC());
		regs.setPC(addr + 2);
	}

}

void Context::ExecuteInstThumb16_B_12()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_B_13()
{
	int immd;
	int addr;
	int cond;

	cond = inst.getThumb16Bytes()->cond_br_ins.cond;
	immd = inst.getThumb16Bytes()->cond_br_ins.s_offset << 1;
	immd = misc::SignExtend32(immd, 9);
	emulator->isa_debug << misc::fmt("  Offset = %x (%d)\n", immd, immd);
	addr = regs.getPC() + immd;

	if (IsaThumbCheckCond(cond))
	{
		emulator->isa_debug << misc::fmt("  Branch addr = 0x%x, pc <= %x\n", addr, regs.getPC());
		regs.setPC(addr + 2);
	}

}

void Context::ExecuteInstThumb16_B_14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_B_15()
{
	int immd;
	int addr;


	immd = inst.getThumb16Bytes()->br_ins.immd11 << 1;
	immd = misc::SignExtend32(immd, 12);
	emulator->isa_debug << misc::fmt("  Offset = %x (%d)\n", immd, immd);

	addr = regs.getPC() + immd;

	emulator->isa_debug << misc::fmt("  Branch addr = 0x%x, pc <= %x\n", addr, regs.getPC());
	regs.setPC(addr + 2);

}

void Context::ExecuteInstThumb16_ADD_SP1()
{
	unsigned int immd7;
	int sp_val;

	IsaRegLoad(13, sp_val);
	immd7 = 4 * inst.getThumb16Bytes()->sub_sp_ins.immd_8;

	sp_val = sp_val + immd7;

	IsaRegStore(13, sp_val);
}

void Context::ExecuteInstThumb16_ADD_SP2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_ADD_SP3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_ADD_SP4()
{
	unsigned int immd7;
	int sp_val;

	IsaRegLoad(13, sp_val);
	immd7 = 4 * inst.getThumb16Bytes()->sub_sp_ins.immd_8;

	sp_val = sp_val + immd7;

	IsaRegStore(13, sp_val);
}

void Context::ExecuteInstThumb16_SUB_SP1()
{
	unsigned int immd7;
	int sp_val;

	IsaRegLoad(13, sp_val);
	immd7 = 4 * inst.getThumb16Bytes()->sub_sp_ins.immd_8;

	sp_val = sp_val - immd7;

	IsaRegStore(13, sp_val);
}

void Context::ExecuteInstThumb16_SUB_SP2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_SUB_SP3()
{
	unsigned int immd7;
	int sp_val;

	IsaRegLoad(13, sp_val);
	immd7 = 4 * inst.getThumb16Bytes()->sub_sp_ins.immd_8;

	sp_val = sp_val - immd7;

	IsaRegStore(13, sp_val);
}

void Context::ExecuteInstThumb16_SUB_SP4()
{
	unsigned int immd7;
	int sp_val;

	IsaRegLoad(13, sp_val);
	immd7 = 4 * inst.getThumb16Bytes()->sub_sp_ins.immd_8;

	sp_val = sp_val - immd7;

	IsaRegStore(13, sp_val);
}

void Context::ExecuteInstThumb16_SXTH_SP1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_SXTH_SP2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_SXTB_SP1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_SXTB_SP2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_UXTH_SP1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_UXTH_SP2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_UXTB_SP1()
{
	int rm_val;
	IsaRegLoad(inst.getThumb16Bytes()->ldstr_exts_ins.reg_rb, rm_val);
	rm_val = rm_val & 0x000000ff;

	IsaRegStore(inst.getThumb16Bytes()->ldstr_exts_ins.reg_rd, rm_val);
}

void Context::ExecuteInstThumb16_UXTB_SP2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_PUSH_0()
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = inst.getThumb16Bytes()->push_pop_ins.reg_list;
	regs = (inst.getThumb16Bytes()->push_pop_ins.m_ext << 14) | regs;

	buf = &reg_val;
	IsaRegLoad(13, sp_val);
	wrt_val = sp_val - 4*(IsaBitCount(regs));

	for (i = 1; i < 65536; i *= 2)
	{
		if (regs & (i))
		{
			IsaRegLoad(misc::LogBase2(i), reg_val);
			memory->Write(wrt_val, 4, (char *)buf);
			emulator->isa_debug << misc::fmt("  push r%d => 0x%x\n",misc::LogBase2(i),wrt_val);
			wrt_val += 4;
		}
	}

	sp_val = sp_val - 4*(IsaBitCount(regs));
	IsaRegStore(13, sp_val);
}

void Context::ExecuteInstThumb16_PUSH_1()
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = inst.getThumb16Bytes()->push_pop_ins.reg_list;
	regs = (inst.getThumb16Bytes()->push_pop_ins.m_ext << 14) | regs;

	buf = &reg_val;
	IsaRegLoad(13, sp_val);
	wrt_val = sp_val - 4*(IsaBitCount(regs));

	for (i = 1; i < 65536; i *= 2)
	{
		if (regs & (i))
		{
			IsaRegLoad(misc::LogBase2(i), reg_val);
			memory->Write(wrt_val, 4, (char *)buf);
			emulator->isa_debug << misc::fmt("  push r%d => 0x%x\n",misc::LogBase2(i),wrt_val);
			wrt_val += 4;
		}
	}

	sp_val = sp_val - 4*(IsaBitCount(regs));
	IsaRegStore(13, sp_val);

}

void Context::ExecuteInstThumb16_PUSH_2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_PUSH_3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_PUSH_4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_PUSH_5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_PUSH_6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_PUSH_7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_PUSH_8()
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = inst.getThumb16Bytes()->push_pop_ins.reg_list;
	regs = (inst.getThumb16Bytes()->push_pop_ins.m_ext << 14) | regs;

	buf = &reg_val;
	IsaRegLoad(13, sp_val);
	wrt_val = sp_val - 4*(IsaBitCount(regs));

	for (i = 1; i < 65536; i *= 2)
	{
		if (regs & (i))
		{
			IsaRegLoad(misc::LogBase2(i), reg_val);
			memory->Write(wrt_val, 4, (char *)buf);
			emulator->isa_debug << misc::fmt("  push r%d => 0x%x\n",misc::LogBase2(i),wrt_val);
			wrt_val += 4;
		}
	}

	sp_val = sp_val - 4*(IsaBitCount(regs));
	IsaRegStore(13, sp_val);
}

void Context::ExecuteInstThumb16_PUSH_9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_PUSH_10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_PUSH_11()
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = inst.getThumb16Bytes()->push_pop_ins.reg_list;
	regs = (inst.getThumb16Bytes()->push_pop_ins.m_ext << 14) | regs;

	buf = &reg_val;
	IsaRegLoad(13, sp_val);
	wrt_val = sp_val - 4*(IsaBitCount(regs));

	for (i = 1; i < 65536; i *= 2)
	{
		if (regs & (i))
		{
			IsaRegLoad(misc::LogBase2(i), reg_val);
			memory->Write(wrt_val, 4, (char *)buf);
			emulator->isa_debug << misc::fmt("  push r%d => 0x%x\n",misc::LogBase2(i),wrt_val);
			wrt_val += 4;
		}
	}

	sp_val = sp_val - 4*(IsaBitCount(regs));
	IsaRegStore(13, sp_val);

}

void Context::ExecuteInstThumb16_PUSH_12()
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = inst.getThumb16Bytes()->push_pop_ins.reg_list;
	regs = (inst.getThumb16Bytes()->push_pop_ins.m_ext << 14) | regs;

	buf = &reg_val;
	IsaRegLoad(13, sp_val);
	wrt_val = sp_val - 4*(IsaBitCount(regs));

	for (i = 1; i < 65536; i *= 2)
	{
		if (regs & (i))
		{
			IsaRegLoad(misc::LogBase2(i), reg_val);
			memory->Write(wrt_val, 4, (char *)buf);
			emulator->isa_debug << misc::fmt("  push r%d => 0x%x\n",misc::LogBase2(i),wrt_val);
			wrt_val += 4;
		}
	}

	sp_val = sp_val - 4*(IsaBitCount(regs));
	IsaRegStore(13, sp_val);
}

void Context::ExecuteInstThumb16_PUSH_13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_PUSH_14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_PUSH_15()
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = inst.getThumb16Bytes()->push_pop_ins.reg_list;
	regs = (inst.getThumb16Bytes()->push_pop_ins.m_ext << 14) | regs;

	buf = &reg_val;
	IsaRegLoad(13, sp_val);
	wrt_val = sp_val - 4*(IsaBitCount(regs));

	for (i = 1; i < 65536; i *= 2)
	{
		if (regs & (i))
		{
			IsaRegLoad(misc::LogBase2(i), reg_val);
			memory->Write(wrt_val, 4, (char *)buf);
			emulator->isa_debug << misc::fmt("  push r%d => 0x%x\n",misc::LogBase2(i),wrt_val);
			wrt_val += 4;
		}
	}

	sp_val = sp_val - 4*(IsaBitCount(regs));
	IsaRegStore(13, sp_val);
}

void Context::ExecuteInstThumb16_POP_0()
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = inst.getThumb16Bytes()->push_pop_ins.reg_list;
	regs = (inst.getThumb16Bytes()->push_pop_ins.m_ext << 15) | regs;

	buf = &reg_val;


	IsaRegLoad(13, sp_val);
	wrt_val = sp_val;


	for (i = 1; i < 65536; i *= 2)
	{
		if (regs & (i))
		{
			if (misc::LogBase2(i) < 15)
			{
				memory->Read(wrt_val, 4, (char *)buf);
				IsaRegStore(misc::LogBase2(i), reg_val);
				emulator->isa_debug << misc::fmt("  pop r%d <= 0x%x\n",misc::LogBase2(i),wrt_val);
				wrt_val += 4;
			}
			else
			{
				memory->Read(wrt_val, 4, (char *)buf);
				if (reg_val % 2)
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

void Context::ExecuteInstThumb16_POP_1()
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = inst.getThumb16Bytes()->push_pop_ins.reg_list;
	regs = (inst.getThumb16Bytes()->push_pop_ins.m_ext << 15) | regs;

	buf = &reg_val;


	IsaRegLoad(13, sp_val);
	wrt_val = sp_val;


	for (i = 1; i < 65536; i *= 2)
	{
		if (regs & (i))
		{
			if (misc::LogBase2(i) < 15)
			{
				memory->Read(wrt_val, 4, (char *)buf);
				IsaRegStore(misc::LogBase2(i), reg_val);
				emulator->isa_debug << misc::fmt("  pop r%d <= 0x%x\n",misc::LogBase2(i),wrt_val);
				wrt_val += 4;
			}
			else
			{
				memory->Read(wrt_val, 4, (char *)buf);
				if (reg_val % 2)
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

void Context::ExecuteInstThumb16_POP_2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_POP_3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_POP_4()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_POP_5()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_POP_6()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_POP_7()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_POP_8()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_POP_9()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_POP_10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_POP_11()
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = inst.getThumb16Bytes()->push_pop_ins.reg_list;
	regs = (inst.getThumb16Bytes()->push_pop_ins.m_ext << 15) | regs;

	buf = &reg_val;


	IsaRegLoad(13, sp_val);
	wrt_val = sp_val;


	for (i = 1; i < 65536; i *= 2)
	{
		if (regs & (i))
		{
			if (misc::LogBase2(i) < 15)
			{
				memory->Read(wrt_val, 4, (char *)buf);
				IsaRegStore(misc::LogBase2(i), reg_val);
				emulator->isa_debug << misc::fmt("  pop r%d <= 0x%x\n",misc::LogBase2(i),wrt_val);
				wrt_val += 4;
			}
			else
			{
				memory->Read(wrt_val, 4, (char *)buf);
				if (reg_val % 2)
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

void Context::ExecuteInstThumb16_POP_12()
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = inst.getThumb16Bytes()->push_pop_ins.reg_list;
	regs = (inst.getThumb16Bytes()->push_pop_ins.m_ext << 15) | regs;

	buf = &reg_val;


	IsaRegLoad(13, sp_val);
	wrt_val = sp_val;


	for (i = 1; i < 65536; i *= 2)
	{
		if (regs & (i))
		{
			if (misc::LogBase2(i) < 15)
			{
				memory->Read(wrt_val, 4, (char *)buf);
				IsaRegStore(misc::LogBase2(i), reg_val);
				emulator->isa_debug << misc::fmt("  pop r%d <= 0x%x\n",misc::LogBase2(i),wrt_val);
				wrt_val += 4;
			}
			else
			{
				memory->Read(wrt_val, 4, (char *)buf);
				if (reg_val % 2)
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

void Context::ExecuteInstThumb16_POP_13()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_POP_14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_POP_15()
{
	unsigned int regs;
	int i;
	int sp_val;
	int wrt_val;
	int reg_val;
	void *buf;

	regs = inst.getThumb16Bytes()->push_pop_ins.reg_list;
	regs = (inst.getThumb16Bytes()->push_pop_ins.m_ext << 15) | regs;

	buf = &reg_val;


	IsaRegLoad(13, sp_val);
	wrt_val = sp_val;


	for (i = 1; i < 65536; i *= 2)
	{
		if (regs & (i))
		{
			if (misc::LogBase2(i) < 15)
			{
				memory->Read(wrt_val, 4, (char *)buf);
				IsaRegStore(misc::LogBase2(i), reg_val);
				emulator->isa_debug << misc::fmt("  pop r%d <= 0x%x\n",misc::LogBase2(i),wrt_val);
				wrt_val += 4;
			}
			else
			{
				memory->Read(wrt_val, 4, (char *)buf);
				if (reg_val % 2)
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

void Context::ExecuteInstThumb16_CBZ_8()
{
	unsigned int immd5;
	int rn_val;
	int inst_addr;


	emulator->isa_debug << misc::fmt(" PC : 0x%x\n", regs.getPC());
	immd5 = inst.getThumb16Bytes()->cbnz_ins.immd_5;
	inst_addr = inst.getAddress();
	emulator->isa_debug << misc::fmt("  Inst addr <= 0x%x\n", inst_addr);
	if ((inst_addr + 2) % 4)
		immd5 = (inst_addr + 4) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (inst_addr + 4) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));


/*
	if ((regs.getPC() - 2) % 4)
		immd5 = (regs.getPC()) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (regs.getPC() - 2) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));
*/
	emulator->isa_debug << misc::fmt("  Branch addr = 0x%x, Before Branch pc <= 0x%x\n",
		immd5, regs.getPC());

	IsaRegLoad(inst.getThumb16Bytes()->cbnz_ins.reg_rn, rn_val);
	if (rn_val == 0)
	{
//		if (immd5 % 4)
//			regs.getPC() = immd5 + 4;
//		else
			regs.setPC(immd5 + 2);
		emulator->isa_debug << misc::fmt("  After Branch pc <= 0x%x\n", regs.getPC());
	}
}

void Context::ExecuteInstThumb16_CBZ_9()
{
	unsigned int immd5;
	int rn_val;
	int inst_addr;


	emulator->isa_debug << misc::fmt(" PC : 0x%x\n", regs.getPC());
	immd5 = inst.getThumb16Bytes()->cbnz_ins.immd_5;
	inst_addr = inst.getAddress();
	emulator->isa_debug << misc::fmt("  Inst addr <= 0x%x\n", inst_addr);
	if ((inst_addr + 2) % 4)
		immd5 = (inst_addr + 4) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (inst_addr + 4) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));


/*
	if ((regs.getPC() - 2) % 4)
		immd5 = (regs.getPC()) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (regs.getPC() - 2) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));
*/
	emulator->isa_debug << misc::fmt("  Branch addr = 0x%x, Before Branch pc <= 0x%x\n",
		immd5, regs.getPC());

	IsaRegLoad(inst.getThumb16Bytes()->cbnz_ins.reg_rn, rn_val);
	if (rn_val == 0)
	{
//		if (immd5 % 4)
//			regs.getPC() = immd5 + 4;
//		else
			regs.setPC(immd5 + 2);
		emulator->isa_debug << misc::fmt("  After Branch pc <= 0x%x\n", regs.getPC());
	}

}

void Context::ExecuteInstThumb16_CBZ_10()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBZ_11()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBZ_12()
{
	unsigned int immd5;
	int rn_val;
	int inst_addr;


	emulator->isa_debug << misc::fmt(" PC : 0x%x\n", regs.getPC());
	immd5 = inst.getThumb16Bytes()->cbnz_ins.immd_5;
	inst_addr = inst.getAddress();
	emulator->isa_debug << misc::fmt("  Inst addr <= 0x%x\n", inst_addr);
	if ((inst_addr + 2) % 4)
		immd5 = (inst_addr + 4) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (inst_addr + 4) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));


/*
	if ((regs.getPC() - 2) % 4)
		immd5 = (regs.getPC()) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (regs.getPC() - 2) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));
*/
	emulator->isa_debug << misc::fmt("  Branch addr = 0x%x, Before Branch pc <= 0x%x\n",
		immd5, regs.getPC());

	IsaRegLoad(inst.getThumb16Bytes()->cbnz_ins.reg_rn, rn_val);
	if (rn_val == 0)
	{
//		if (immd5 % 4)
//			regs.getPC() = immd5 + 4;
//		else
			regs.setPC(immd5 + 2);
		emulator->isa_debug << misc::fmt("  After Branch pc <= 0x%x\n", regs.getPC());
	}

}

void Context::ExecuteInstThumb16_CBZ_13()
{
	unsigned int immd5;
	int rn_val;
	int inst_addr;


	emulator->isa_debug << misc::fmt(" PC : 0x%x\n", regs.getPC());
	immd5 = inst.getThumb16Bytes()->cbnz_ins.immd_5;
	inst_addr = inst.getAddress();
	emulator->isa_debug << misc::fmt("  Inst addr <= 0x%x\n", inst_addr);
	if ((inst_addr + 2) % 4)
		immd5 = (inst_addr + 4) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (inst_addr + 4) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));


/*
	if ((regs.getPC() - 2) % 4)
		immd5 = (regs.getPC()) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (regs.getPC() - 2) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));
*/
	emulator->isa_debug << misc::fmt("  Branch addr = 0x%x, Before Branch pc <= 0x%x\n",
		immd5, regs.getPC());

	IsaRegLoad(inst.getThumb16Bytes()->cbnz_ins.reg_rn, rn_val);
	if (rn_val == 0)
	{
//		if (immd5 % 4)
//			regs.getPC() = immd5 + 4;
//		else
			regs.setPC(immd5 + 2);
		emulator->isa_debug << misc::fmt("  After Branch pc <= 0x%x\n", regs.getPC());
	}

}

void Context::ExecuteInstThumb16_CBZ_14()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBZ_15()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBZ_18()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBZ_19()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBZ_110()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBZ_111()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBZ_112()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBZ_113()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBZ_114()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBZ_115()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBNZ_58()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBNZ_59()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBNZ_510()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBNZ_511()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBNZ_512()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBNZ_513()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBNZ_514()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBNZ_515()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBNZ_48()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBNZ_49()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBNZ_410()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBNZ_411()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBNZ_412()
{
	unsigned int immd5;
	int rn_val;


	emulator->isa_debug << misc::fmt(" PC : 0x%x\n", regs.getPC());
	immd5 = inst.getThumb16Bytes()->cbnz_ins.immd_5;
	if ((regs.getPC() - 2) % 4)
		immd5 = (regs.getPC()) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));
	else
		immd5 = (regs.getPC() - 2) + ((inst.getThumb16Bytes()->cbnz_ins.i_ext << 6) | (immd5 << 1));

	emulator->isa_debug << misc::fmt("  Branch addr = 0x%x, Before Branch pc <= 0x%x\n",
		immd5, regs.getPC());
	IsaRegLoad(inst.getThumb16Bytes()->cbnz_ins.reg_rn, rn_val);
	if (rn_val != 0)
	{
		regs.setPC(immd5 + 2);
		emulator->isa_debug << misc::fmt("  After Branch pc <= 0x%x\n", regs.getPC());
	}
}

void Context::ExecuteInstThumb16_CBNZ_413()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBNZ_414()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_CBNZ_415()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_NOP()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_41()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_42()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_43()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_44()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_45()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_46()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_47()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_48()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_49()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_410()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_411()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_412()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_413()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_414()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_415()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_51()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_52()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_53()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_54()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_55()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_56()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_57()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_58()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_59()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_510()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_511()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_512()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_513()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_514()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_515()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_61()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_62()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_63()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_64()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_65()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_66()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_67()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_68()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_69()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_610()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_611()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_612()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_613()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_614()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_615()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_71()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_72()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_73()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_74()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_75()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_76()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_77()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_78()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_79()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_710()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_711()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_712()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_713()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_714()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_715()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_81()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_82()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_83()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_84()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_85()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_86()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_87()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_88()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_89()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_810()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_811()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_812()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_813()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_814()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_815()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_91()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_92()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_93()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_94()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_95()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_96()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_97()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_98()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_99()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_910()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_911()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_912()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_913()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_914()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_915()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_101()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_102()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_103()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_104()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_105()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_106()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_107()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_108()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_109()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_1010()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_1011()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_1012()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_1013()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_1014()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_1015()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_111()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_112()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_113()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_114()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_115()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_116()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_117()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_118()
{
	IsaThumbIteq();
}

void Context::ExecuteInstThumb16_IT_119()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_1110()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_1111()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_1112()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_1113()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_1114()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_IT_1115()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_REV_0()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInstThumb16_REV_1()
{
	throw misc::Panic("Unimplemented instruction");
}

} // namespace ARM
