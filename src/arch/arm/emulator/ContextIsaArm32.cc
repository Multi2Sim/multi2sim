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

Context::ExecuteInstFn Context::execute_inst_fn[Instruction::OpcodeCount] =
{
	nullptr  // For Instruction::OpcodeNone
#define DEFINST(name, _fmt_str, _category, _arg1, _arg2) \
		 , &Context::ExecuteInst_##name
#include <arch/arm/disassembler/Instruction.def>
#undef DEFINST
};

void Context::ExecuteInst_AND_reg()
{
	int operand2;
	int rn_val;
	if (IsaCheckCond())
	{
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaRegStore(inst.getBytes()->dpr.dst_reg, (operand2 & rn_val));
	}
}

void Context::ExecuteInst_ANDS_reg()
{
	int operand2;
	int rn_val;
	regsPsr prev_regs_casr = regs.getCPSR();

	if(IsaCheckCond())
	{
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaRegStore(inst.getBytes()->dpr.dst_reg, (operand2 & rn_val));

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;

		if ((operand2 & rn_val) == 0)
		{
			regs.getCPSR().z = 1;
			regs.getCPSR().n = 0;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if((operand2 & rn_val) < 0)
		{
			regs.getCPSR().n = 1;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if(IsaOp2Carry(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg))
		{
			regs.getCPSR().C = 1;
			regs.getCPSR().v = 0;
		}
		if(IsaOp2Carry(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg) == -1)
		{
			regs.getCPSR().C = prev_regs_casr.C;
		}

		IsaCpsrPrint();
	}
}

void Context::ExecuteInst_EOR_reg()
{
	int operand2;
	int rn_val;

	if(IsaCheckCond())
	{
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaRegStore(inst.getBytes()->dpr.dst_reg, (operand2 ^ rn_val));
	}
}

void Context::ExecuteInst_EORS_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_SUB_reg()
{
	int operand2;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaSubtract(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
			operand2, 0);
	}
}

void Context::ExecuteInst_SUBS_reg()
{

	int operand2;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaSubtract(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
			operand2, 0);
	}
}

void Context::ExecuteInst_RSB_reg()
{
	int operand2;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaSubtractRev(inst.getBytes()->dpr.dst_reg,
			inst.getBytes()->dpr.op1_reg, operand2, 0);
	}
}

void Context::ExecuteInst_RSBS_reg()
{
	int operand2;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaSubtractRev(inst.getBytes()->dpr.dst_reg,
				inst.getBytes()->dpr.op1_reg, operand2, 0);
	}
}

void Context::ExecuteInst_ADD_reg()
{
	int operand2;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaAdd(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
			operand2, 0);
	}
}

void Context::ExecuteInst_ADDS_reg()
{
	int operand2;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaAdd(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
			operand2, 0);
	}
}

void Context::ExecuteInst_ADC_reg()
{
	int operand2;
	unsigned int carry = regs.getCPSR().C;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaAdd(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
			operand2, carry);
	}
}

void Context::ExecuteInst_ADCS_reg()
{
	int operand2;
	unsigned int carry = regs.getCPSR().C;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaAdd(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
				operand2, carry);
	}
}

void Context::ExecuteInst_SBC_reg()
{
	int operand2;
	unsigned int carry = regs.getCPSR().C;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaSubtract(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
			operand2, ~(carry));
	}
}

void Context::ExecuteInst_SBCS_reg()
{
	int operand2;
	unsigned int carry = regs.getCPSR().C;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaSubtract(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
			operand2, ~(carry));
	}
}

void Context::ExecuteInst_RSC_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_RSCS_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_TSTS_reg()
{
	int operand2;
	int rn_val;
	if(IsaCheckCond())
	{
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;

		if ((operand2 & rn_val) == 0)
		{
			regs.getCPSR().z = 1;
			regs.getCPSR().n = 0;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if((operand2 & rn_val) < 0)
		{
			regs.getCPSR().n = 1;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if(IsaOp2Carry(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg))
		{
			regs.getCPSR().C = 1;
			regs.getCPSR().v = 0;
		}
		IsaCpsrPrint();
	}
}

void Context::ExecuteInst_TEQS_reg()
{
	int operand2;
	int rn_val;
	if(IsaCheckCond())
	{
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;

		if ((operand2 ^ rn_val) == 0)
		{
			regs.getCPSR().z = 1;
			regs.getCPSR().n = 0;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if((operand2 ^ rn_val) < 0)
		{
			regs.getCPSR().n = 1;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if(IsaOp2Carry(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg))
		{
			regs.getCPSR().C = 1;
			regs.getCPSR().v = 0;
		}
		IsaCpsrPrint();
	}
}

void Context::ExecuteInst_CMPS_reg()
{
	int rn_val;
	int operand2;
	int result;
	int op2;
	int rd_val;
	unsigned long flags = 0;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);
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
}

void Context::ExecuteInst_CMNS_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_ORR_reg()
{
	int operand2;
	int rn_val;

	if(IsaCheckCond())
	{
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaRegStore(inst.getBytes()->dpr.dst_reg, (operand2 | rn_val));
	}
}

void Context::ExecuteInst_ORRS_reg()
{
	int operand2;
	int rn_val;
	regsPsr prev_regs_cpsr = regs.getCPSR();
	if(IsaCheckCond())
	{
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaRegStore(inst.getBytes()->dpr.dst_reg, (operand2 | rn_val));

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;

		if ((operand2 | rn_val) == 0)
		{
			regs.getCPSR().z = 1;
			regs.getCPSR().n = 0;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if((operand2 | rn_val) < 0)
		{
			regs.getCPSR().n = 1;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if(IsaOp2Carry(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg))
		{
			regs.getCPSR().C = 1;
			regs.getCPSR().v = 0;
		}
		if(IsaOp2Carry(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg) == -1)
		{
			regs.getCPSR().C = prev_regs_cpsr.C;
		}

		IsaCpsrPrint();
	}
}

void Context::ExecuteInst_MOV_reg()
{
	int operand2;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaRegStore(inst.getBytes()->dpr.dst_reg, operand2);
	}
}

void Context::ExecuteInst_MOVS_reg()
{
	int operand2;
	regsPsr prev_regs_cpsr = regs.getCPSR();

	prev_regs_cpsr = regs.getCPSR();
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		IsaRegStore(inst.getBytes()->dpr.dst_reg, operand2);

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;

		if (operand2 == 0)
		{
			regs.getCPSR().z = 1;
			regs.getCPSR().n = 0;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if(operand2 < 0)
		{
			regs.getCPSR().n = 1;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if(IsaOp2Carry(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg))
		{
			regs.getCPSR().C = 1;
			regs.getCPSR().v = 0;
		}
		if(IsaOp2Carry(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg) == -1)
		{
			regs.getCPSR().C = prev_regs_cpsr.C;
		}
	}
	IsaCpsrPrint();
}

void Context::ExecuteInst_BIC_reg()
{
	int operand2;
	int rd_val;
	int rn_val;


	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);

		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);

		if(!(inst.getBytes()->dpr.s_cond))
		{
			rd_val = (rn_val) & (~(operand2)) ;
			emulator->isa_debug << misc::fmt("  r%d = r%d & (~%d)\n", inst.getBytes()->dpr.dst_reg,
				inst.getBytes()->dpr.op1_reg, operand2);
			IsaRegStore(inst.getBytes()->dpr.dst_reg, rd_val);
		}
		else
		{

		}
	}
}

void Context::ExecuteInst_BICS_reg()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_MVN_reg()
{
	unsigned int operand2;
	unsigned int result;

	if (IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		result = (0xffffffff) ^ (operand2);
		IsaRegStoreSafe(inst.getBytes()->dpr.dst_reg, result);
	}
}

void Context::ExecuteInst_MVNS_reg()
{
	unsigned int operand2;
	unsigned int result;
	regsPsr prev_regs_cpsr = regs.getCPSR();

	prev_regs_cpsr = regs.getCPSR();
	if (IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg);
		result = (0xffffffff) ^ (operand2);
		IsaRegStoreSafe(inst.getBytes()->dpr.dst_reg, result);

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;

		if (result == 0)
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
		if(IsaOp2Carry(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg))
		{
			regs.getCPSR().C = 1;
			regs.getCPSR().v = 0;
		}
		if(IsaOp2Carry(inst.getBytes()->dpr.op2, ContextOp2CatecoryReg) == -1)
		{
			regs.getCPSR().C = prev_regs_cpsr.C;
		}
	}
	IsaCpsrPrint();
}


void Context::ExecuteInst_AND_imm()
{
	int operand2;
	int rn_val;

	if(IsaCheckCond())
	{
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaRegStore(inst.getBytes()->dpr.dst_reg, (operand2 & rn_val));
	}
}

void Context::ExecuteInst_ANDS_imm()
{
	int operand2;
	int rn_val;
	regsPsr prev_regs_cpsr = regs.getCPSR();
	if(IsaCheckCond())
	{
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaRegStore(inst.getBytes()->dpr.dst_reg, (operand2 & rn_val));

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;

		if ((operand2 & rn_val) == 0)
		{
			regs.getCPSR().z = 1;
			regs.getCPSR().n = 0;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if((operand2 & rn_val) < 0)
		{
			regs.getCPSR().n = 1;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if(IsaOp2Carry(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd))
		{
			regs.getCPSR().C = 1;
			regs.getCPSR().v = 0;
		}
		if(IsaOp2Carry(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd) == -1)
		{
			regs.getCPSR().C = prev_regs_cpsr.C;
		}

		IsaCpsrPrint();
	}
}

void Context::ExecuteInst_EOR_imm()
{
	int operand2;
	int rn_val;

	if(IsaCheckCond())
	{
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaRegStore(inst.getBytes()->dpr.dst_reg, (operand2 ^ rn_val));
	}
}

void Context::ExecuteInst_EORS_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_SUB_imm()
{
	int operand2;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaSubtract(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
			operand2, 0);
	}
}

void Context::ExecuteInst_SUBS_imm()
{
	int operand2;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaSubtract(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
			operand2, 0);
	}
}

void Context::ExecuteInst_RSB_imm()
{
	int operand2;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaSubtractRev(inst.getBytes()->dpr.dst_reg,
			inst.getBytes()->dpr.op1_reg, operand2, 0);
	}
}

void Context::ExecuteInst_RSBS_imm()
{
	int operand2;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaSubtractRev(inst.getBytes()->dpr.dst_reg,
			inst.getBytes()->dpr.op1_reg, operand2, 0);
	}
}

void Context::ExecuteInst_ADD_imm()
{
	int operand2;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaAdd(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
			operand2, 0);
	}
}

void Context::ExecuteInst_ADDS_imm()
{
	int operand2;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaAdd(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
			operand2, 0);
	}
}

void Context::ExecuteInst_ADC_imm()
{
	int operand2;
	unsigned int carry;

	carry = regs.getCPSR().C;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaAdd(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
			operand2, carry);
	}
}

void Context::ExecuteInst_ADCS_imm()
{
	int operand2;
	unsigned int carry;

	carry = regs.getCPSR().C;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaAdd(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
			operand2, carry);
	}
}

void Context::ExecuteInst_SBC_imm()
{
	int operand2;
	unsigned int carry;

	carry = regs.getCPSR().C;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaSubtract(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
				operand2, ~(carry));
	}
}

void Context::ExecuteInst_SBCS_imm()
{
	int operand2;
	unsigned int carry;

	carry = regs.getCPSR().C;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaSubtract(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
			operand2, ~(carry));
	}
}

void Context::ExecuteInst_RSC_imm()
{
	int operand2;
	unsigned int carry;

	carry = regs.getCPSR().C;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaSubtractRev(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
				operand2, ~(carry));
	}
}

void Context::ExecuteInst_RSCS_imm()
{
	int operand2;
	unsigned int carry;

	carry = regs.getCPSR().C;
	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaSubtractRev(inst.getBytes()->dpr.dst_reg, inst.getBytes()->dpr.op1_reg,
				operand2, ~(carry));
	}
}

void Context::ExecuteInst_TSTS_imm()
{
	regsPsr prev_regs_cpsr = regs.getCPSR();
	int operand2;
	int rn_val;

	if(IsaCheckCond())
	{
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;

		if ((operand2 & rn_val) == 0)
		{
			regs.getCPSR().z = 1;
			regs.getCPSR().n = 0;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if((operand2 & rn_val) < 0)
		{
			regs.getCPSR().n = 1;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if(IsaOp2Carry(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd))
		{
			regs.getCPSR().C = 1;
			regs.getCPSR().v = 0;
		}
		if(IsaOp2Carry(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd) == -1)
		{
			regs.getCPSR().C = prev_regs_cpsr.C;
		}

	}
	IsaCpsrPrint();
}

void Context::ExecuteInst_TEQS_imm()
{
	int operand2;
	int rn_val;
	if(IsaCheckCond())
	{
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;

		if ((operand2 ^ rn_val) == 0)
		{
			regs.getCPSR().z = 1;
			regs.getCPSR().n = 0;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if((operand2 ^ rn_val) < 0)
		{
			regs.getCPSR().n = 1;
			regs.getCPSR().C = 0;
			regs.getCPSR().v = 0;
		}
		if(IsaOp2Carry(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd))
		{
			regs.getCPSR().C = 1;
			regs.getCPSR().v = 0;
		}
		IsaCpsrPrint();
	}
}

void Context::ExecuteInst_CMPS_imm()
{
	int rn_val;
	int operand2;
	int result;
	int op2;
	int rd_val;
	unsigned long flags = 0;

	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);
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

	}
	IsaCpsrPrint();
}

void Context::ExecuteInst_CMNS_imm()
{
	int rn_val;
	int operand2;
	int result;
	int op2;
	int rd_val;
	unsigned long flags = 0;

	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);

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
}

void Context::ExecuteInst_ORR_imm()
{
	int operand2;
	int rn_val;

	if(IsaCheckCond())
	{
		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaRegStore(inst.getBytes()->dpr.dst_reg, (operand2 | rn_val));
	}
}

void Context::ExecuteInst_ORRS_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_MOV_imm()
{
	unsigned int operand2;

	if (IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		IsaRegStore(inst.getBytes()->dpr.dst_reg, operand2);
	}

}

void Context::ExecuteInst_MOVS_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_BIC_imm()
{
	int operand2;
	int rd_val;
	int rn_val;


	if(IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);

		IsaRegLoad(inst.getBytes()->dpr.op1_reg, rn_val);

		if(!(inst.getBytes()->dpr.s_cond))
		{
			rd_val = (rn_val) & (~(operand2)) ;
			emulator->isa_debug << misc::fmt("  r%d = r%d & (~%d)\n", inst.getBytes()->dpr.dst_reg,
				inst.getBytes()->dpr.op1_reg, operand2);
			IsaRegStore(inst.getBytes()->dpr.dst_reg, rd_val);
		}
		else
		{

		}
	}
}

void Context::ExecuteInst_BICS_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_MVN_imm()
{
	unsigned int operand2;
	unsigned int result;

	if (IsaCheckCond())
	{
		operand2 = IsaGetOp2(inst.getBytes()->dpr.op2, ContextOp2CatecoryImmd);
		result = (0xffffffff) ^ (operand2);
		IsaRegStoreSafe(inst.getBytes()->dpr.dst_reg, result);
	}
}

void Context::ExecuteInst_MVNS_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_MOVT_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_MOVW_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_QADD()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_QSUB()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_QDADD()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_QDSUB()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_MRS_reg()
{
	unsigned int rd_val;
	if (IsaCheckCond())
	{
		if(!(inst.getBytes()->psr.psr_loc))
		{
			rd_val = IsaRetCpsrVal();
		}
	}

	IsaRegStore(inst.getBytes()->psr.dst_reg, rd_val);
}

void Context::ExecuteInst_MSR_reg()
{
	if (IsaCheckCond())
	{
		if(!(inst.getBytes()->psr.psr_loc))
		{
			IsaSetCpsrVal(inst.getBytes()->psr.op2);
		}
	}

}

void Context::ExecuteInst_MSR_imm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_MUL()
{
	if(IsaCheckCond())
	{
		IsaMultiply();
	}
}

void Context::ExecuteInst_MULS()
{
	if(IsaCheckCond())
	{
		IsaMultiplySettingCF();
	}
}

void Context::ExecuteInst_MLA()
{
	if(IsaCheckCond())
	{
		IsaMultiply();
	}
}

void Context::ExecuteInst_MLAS()
{
	if(IsaCheckCond())
	{
		IsaMultiplySettingCF();
	}
}

void Context::ExecuteInst_SMLA()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_SMLAW()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_SMULW()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_SMUL()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_UMULL()
{
	unsigned long long result;

	int rm_val;
	int rs_val;
	unsigned int rdhi;
	unsigned int rdlo;
	unsigned int rs;
	unsigned int rm;

	IsaRegLoad(inst.getBytes()->mult_ln.op1_rs, rs_val);
	IsaRegLoad(inst.getBytes()->mult.op0_rm, rm_val);

	emulator->isa_debug << misc::fmt("  rm_val: 0x%x, rs_val: 0x%x\n", rm_val,rs_val);

	rs =(unsigned int)rs_val;
	rm =(unsigned int)rm_val;

	result = (unsigned long long)rm*rs;
	emulator->isa_debug << misc::fmt("  result = 0x%llx\n", result);

	rdhi = (0xffffffff00000000 & result) >> 32;
	rdlo = 0x00000000ffffffff & result;

	IsaRegStore(inst.getBytes()->mult_ln.dst_hi, rdhi);
	IsaRegStore(inst.getBytes()->mult_ln.dst_lo, rdlo);


}

void Context::ExecuteInst_UMULLS()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_UMLAL()
{
	// Decalre local variable
	unsigned long long result;
	int rm_val;
	int rs_val;

	// Sanity check
	if (inst.getBytes()->mult.m_acc == 0)
		throw misc::Panic("Instruction UMLAL decode error\n");

	// Get Rn and Rm
	IsaRegLoad(inst.getBytes()->mult_ln.op1_rs, rs_val);
	IsaRegLoad(inst.getBytes()->mult.op0_rm, rm_val);
	emulator->isa_debug << misc::fmt("  rm_val: 0x%x, rs_val: 0x%x\n", rm_val,rs_val);

	// Perform operation of instruction
	unsigned int rs =(unsigned int)rs_val;
	unsigned int rm =(unsigned int)rm_val;
	unsigned long long result_hi = inst.getBytes()->mult_ln.dst_hi;
	unsigned long long result_lo = inst.getBytes()->mult_ln.dst_lo;
	result = result_hi << 32 | result_lo;
	result += (unsigned long long)rm*rs;
	emulator->isa_debug << misc::fmt("  result = 0x%llx\n", result);

	// Store result to Rd
	unsigned int rdhi = (0xffffffff00000000 & result) >> 32;
	unsigned int rdlo = 0x00000000ffffffff & result;
	IsaRegStore(inst.getBytes()->mult_ln.dst_hi, rdhi);
	IsaRegStore(inst.getBytes()->mult_ln.dst_lo, rdlo);
}

void Context::ExecuteInst_UMLALS()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_SMULL()
{
	if(IsaCheckCond())
	{
		IsaSignedMultiplyLong();
	}
}

void Context::ExecuteInst_SMULLS()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_SMLAL()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_SMLALS()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_SMLALXX()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_SWP()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_SWPB()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_BX()
{
	if(IsaCheckCond())
		IsaBranch();
}

void Context::ExecuteInst_BLX()
{
	if(IsaCheckCond())
		IsaBranch();
}

void Context::ExecuteInst_LDRH_ptrm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRH_ptrm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRH_ptrp1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRH_ptrp2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRH_ofrm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRH_prrm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRH_ofrp()
{
	int addr;
	int value;
	void *buf;

	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode3Imm();
		memory->Read(addr, 4, (char *)buf);
		value = value & (0x0000ffff);
		IsaRegStore(inst.getBytes()->hfwrd_imm.dst_rd, value);
	}
}

void Context::ExecuteInst_LDRH_prrp()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRH_ptrm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRH_ptrm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRH_ptrp1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRH_ptrp2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRH_ofrm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRH_prrm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRH_ofrp()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRH_prrp()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSB_ptrm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSB_ptrm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSB_ptrp1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSB_ptrp2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSB_ofrm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSB_prrm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSB_ofrp()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSH_ptrm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSH_ptrm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSH_ptrp1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSH_ptrp2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSH_ofrm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSH_prrm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSH_ofrp()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSH_prrp()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRD_ptrm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRD_ptrm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRD_ptrp1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRD_ptrp2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRD_ofrm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRD_prrm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRD_ofrp()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRD_prrp()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRD_ptrm1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRD_ptrm2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRD_ptrp1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRD_ptrp2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRD_ofrm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRD_prrm()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRD_ofrp()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRD_prrp()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_CLZ_reg()
{
	int rm_val;
	unsigned int i;
	unsigned int zero_ct;

	zero_ct = 0;
	IsaRegLoad(inst.getBytes()->hfwrd_reg.off_reg, rm_val);

	for (i = 2147483648; i >= 1; i /= 2)
	{
		if((rm_val & (i)) == 0)
		{
			zero_ct += 1;
		}
		else
		{
			break;
		}
	}
	emulator->isa_debug << misc::fmt("  leading zero count = %d\n", zero_ct);
	IsaRegStore(inst.getBytes()->hfwrd_reg.dst_rd, zero_ct);
}

void Context::ExecuteInst_LDRH_ptim1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRH_ptim2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRH_ptip1()
{
	int addr;
	int value;
	void *buf;

	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode3Imm();
		memory->Read(addr, 4, (char *)buf);
		value = value & (0x0000ffff);
		IsaRegStore(inst.getBytes()->hfwrd_imm.dst_rd, value);
	}
}

void Context::ExecuteInst_LDRH_ptip2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRH_ofim()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRH_prim()
{
	int addr;
	int value;
	void *buf;

	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode3Imm();
		memory->Read(addr, 4, (char *)buf);
		value = value & (0x0000ffff);
		IsaRegStore(inst.getBytes()->hfwrd_imm.dst_rd, value);
	}
}

void Context::ExecuteInst_LDRH_ofip()
{

	int addr;
	int value;
	void *buf;

	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode3Imm();
		memory->Read(addr, 4, (char *)buf);
		value = value & (0x0000ffff);
		IsaRegStore(inst.getBytes()->hfwrd_imm.dst_rd, value);
	}
}

void Context::ExecuteInst_LDRH_prip()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRH_ptim1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRH_ptim2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRH_ptip1()
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd, value);
		value = value & (0x0000ffff);
		if(!(IsaInvalidAddrStr(addr, value)))
			memory->Write(addr, 2, (char *)buf);
	}
}

void Context::ExecuteInst_STRH_ptip2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRH_ofim()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRH_prim()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRH_ofip()
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd, value);
		value = value & (0x0000ffff);
		if(!(IsaInvalidAddrStr(addr, value)))
			memory->Write(addr, 2, (char *)buf);
	}
}

void Context::ExecuteInst_STRH_prip()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSB_ptim1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSB_ptim2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSB_ptip1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSB_ptip2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSB_ofim()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSB_prim()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSB_ofip()
{

	int addr;
	int value;
	void *buf;

	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode3Imm();
		memory->Read(addr, 1, (char *)buf);
		value = value & (0xffffffff);
		IsaRegStore(inst.getBytes()->hfwrd_imm.dst_rd, value);
	}
}

void Context::ExecuteInst_LDRSB_prip()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSH_ptim1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSH_ptim2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSH_ptip1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSH_ptip2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSH_ofim()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSH_prim()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRSH_ofip()
{
	int addr;
	int value;
	void *buf;

	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode3Imm();
		memory->Read(addr, 2, (char *)buf);
		value = value & (0xffffffff);
		IsaRegStore(inst.getBytes()->hfwrd_imm.dst_rd, value);
	}
}

void Context::ExecuteInst_LDRSH_prip()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRD_ptim1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRD_ptim2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRD_ptip1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRD_ptip2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRD_ofim()
{

	int addr;
	int value;
	void *buf;

	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode3Imm();
		memory->Read(addr, 4, (char *)buf);
		IsaRegStore(inst.getBytes()->hfwrd_imm.dst_rd, value);

		memory->Read(addr + 4, 4, (char *)buf);
		IsaRegStore(inst.getBytes()->hfwrd_imm.dst_rd + 1, value);
	}
}

void Context::ExecuteInst_LDRD_prim()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRD_ofip()
{
	int addr;
	int value;
	void *buf;

	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode3Imm();
		memory->Read(addr, 4, (char *)buf);
		IsaRegStore(inst.getBytes()->hfwrd_imm.dst_rd, value);

		memory->Read(addr + 4, 4, (char *)buf);
		IsaRegStore(inst.getBytes()->hfwrd_imm.dst_rd + 1, value);
	}
}

void Context::ExecuteInst_LDRD_prip()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRD_ptim1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRD_ptim2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRD_ptip1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRD_ptip2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRD_ofim()
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode3Imm();
		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd, value);

		if(!(IsaInvalidAddrStr(addr, value)))
			memory->Write(addr, 4, (char *)buf);

		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd + 1, value);

		if(!(IsaInvalidAddrStr(addr + 4, value)))
			memory->Write(addr + 4, 4, (char *)buf);
	}
}

void Context::ExecuteInst_STRD_prim()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRD_ofip()
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode3Imm();
		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd, value);

		if(!(IsaInvalidAddrStr(addr, value)))
			memory->Write(addr, 4, (char *)buf);

		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd + 1, value);

		if(!(IsaInvalidAddrStr(addr + 4, value)))
			memory->Write(addr + 4, 4, (char *)buf);
	}
}

void Context::ExecuteInst_STRD_prip()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_ptim()
{
	unsigned int addr;
	unsigned int value;
	void *buf;

	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();

		if(!(IsaInvalidAddrLdr(addr, (unsigned int *)buf)))
		{
			memory->Read(addr, 4, (char *)(&value));
		}

		IsaRegStore(inst.getBytes()->sdtr.src_dst_rd, value);
	}
}

void Context::ExecuteInst_LDR_ptip()
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();

		if(!(IsaInvalidAddrLdr(addr, (unsigned int *)buf)))
		{
			memory->Read(addr, 4, (char *)(&value));
		}

		IsaRegStore(inst.getBytes()->sdtr.src_dst_rd, value);
	}
}

void Context::ExecuteInst_LDR_ofim()
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();

		if(!(IsaInvalidAddrLdr(addr, (unsigned int *)buf)))
		{
			memory->Read(addr, 4, (char *)(&value));
		}

		IsaRegStore(inst.getBytes()->sdtr.src_dst_rd, value);
	}
}

void Context::ExecuteInst_LDR_prim()
{

	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();

		if(!(IsaInvalidAddrLdr(addr, (unsigned int *)buf)))
		{
			memory->Read(addr, 4, (char *)(&value));
		}

		IsaRegStore(inst.getBytes()->sdtr.src_dst_rd, value);
	}
}

void Context::ExecuteInst_LDR_ofip()
{

	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();

		if(!(IsaInvalidAddrLdr(addr, (unsigned int *)buf)))
		{
			memory->Read(addr, 4, (char *)(&value));
		}

		IsaRegStore(inst.getBytes()->sdtr.src_dst_rd, value);
	}
}

void Context::ExecuteInst_LDR_prip()
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();

		if(!(IsaInvalidAddrLdr(addr, (unsigned int *)buf)))
		{
			memory->Read(addr, 4, (char *)(&value));
		}

		IsaRegStore(inst.getBytes()->sdtr.src_dst_rd, value);
	}
}

void Context::ExecuteInst_LDR_ptrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_ptrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_ptrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_ptrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_ptrpll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_ptrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_ptrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_ptrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_ofrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_ofrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_ofrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_ofrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_prrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_prrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_prrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_prrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_ofrpll()
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();

		if(!(IsaInvalidAddrLdr(addr, (unsigned int *)buf)))
		{
			memory->Read(addr, 4, (char *)(&value));
		}
		IsaRegStore(inst.getBytes()->sdtr.src_dst_rd, value);
	}
}

void Context::ExecuteInst_LDR_ofrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_ofrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_ofrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_prrpll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_prrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_prrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDR_prrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ptim()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ptip()
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd, value);

		if(!(IsaInvalidAddrStr(addr, value)))
			memory->Write(addr, 4, (char *)buf);
	}
}

void Context::ExecuteInst_STR_ofim()
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd, value);

		if(!(IsaInvalidAddrStr(addr, value)))
			memory->Write(addr, 4, (char *)buf);
	}
}

void Context::ExecuteInst_STR_prim()
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd, value);
		if(!(IsaInvalidAddrStr(addr, value)))
			memory->Write(addr, 4, (char *)buf);
	}
}

void Context::ExecuteInst_STR_ofip()
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd, value);

		if(!(IsaInvalidAddrStr(addr, value)))
			memory->Write(addr, 4, (char *)buf);
	}
}

void Context::ExecuteInst_STR_prip()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ptrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ptrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ptrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ptrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ptrpll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ptrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ptrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ptrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ofrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ofrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ofrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ofrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_prrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_prrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_prrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_prrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ofrpll()
{
	unsigned int addr;
	void *buf = 0;
	int value;

	if(IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd, value);

		if(!(IsaInvalidAddrStr(addr, value)))
			memory->Write(addr, 4, (char *)buf);
	}
}

void Context::ExecuteInst_STR_ofrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ofrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_ofrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_prrpll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_prrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_prrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STR_prrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ptim()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ptip()
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	value = 0;
	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		memory->Read(addr, 1, (char *)buf);
		value = value & (0x000000ff);
		IsaRegStore(inst.getBytes()->sdtr.src_dst_rd, (value));

	}
}

void Context::ExecuteInst_LDRB_ofim()
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	value = 0;
	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		memory->Read(addr, 1, (char *)buf);
		value = value & (0x000000ff);
		IsaRegStore(inst.getBytes()->sdtr.src_dst_rd, (value));

	}
}

void Context::ExecuteInst_LDRB_prim()
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	value = 0;
	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		memory->Read(addr, 1, (char *)buf);
		value = value & (0x000000ff);
		IsaRegStore(inst.getBytes()->sdtr.src_dst_rd, (value));

	}
}

void Context::ExecuteInst_LDRB_ofip()
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	value = 0;
	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		memory->Read(addr, 1, (char *)buf);
		value = value & (0x000000ff);
		IsaRegStore(inst.getBytes()->sdtr.src_dst_rd, (value));

	}
}

void Context::ExecuteInst_LDRB_prip()
{

	unsigned int addr;
	void *buf = 0;
	unsigned int value;

	value = 0;
	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		memory->Read(addr, 1, (char *)buf);
		value = value & (0x000000ff);
		IsaRegStore(inst.getBytes()->sdtr.src_dst_rd, (value));

	}
}

void Context::ExecuteInst_LDRB_ptrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ptrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ptrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ptrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ptrpll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ptrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ptrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ptrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ofrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ofrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ofrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ofrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_prrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_prrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_prrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_prrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ofrpll()
{
	unsigned int addr;
	void *buf = 0;
	unsigned int value;


	value = 0;
	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		memory->Read(addr, 1, (char *)buf);
		value = value & (0x000000ff);
		IsaRegStore(inst.getBytes()->sdtr.src_dst_rd, (value));
	}
}

void Context::ExecuteInst_LDRB_ofrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ofrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_ofrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_prrpll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_prrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_prrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRB_prrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ptim()
{
	unsigned int addr;
	void *buf = 0;
	int value;

	value = 0;
	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd, value);
		value = value & (0x000000ff);
		memory->Write(addr, 1, (char *)buf);
	}
}

void Context::ExecuteInst_STRB_ptip()
{
	unsigned int addr;
	void *buf = 0;
	int value;

	value = 0;
	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd, value);
		value = value & (0x000000ff);
		memory->Write(addr, 1, (char *)buf);
	}
}

void Context::ExecuteInst_STRB_ofim()
{
	unsigned int addr;
	void *buf = 0;
	int value;

	value = 0;
	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd, value);
		value = value & (0x000000ff);
		memory->Write(addr, 1, (char *)buf);
	}
}

//STRB pre-index immediate minus
void Context::ExecuteInst_STRB_prim()
{
	unsigned int addr;
	void *buf = 0;
	int value = 0;
	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd, value);
		value = value & (0x000000ff);
		memory->Write(addr, 1, (char *)buf);
	}
}

void Context::ExecuteInst_STRB_ofip()
{
	unsigned int addr;
	void *buf = 0;
	int value;

	value = 0;
	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd, value);
		value = value & (0x000000ff);
		memory->Write(addr, 1, (char *)buf);
	}

}

void Context::ExecuteInst_STRB_prip()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ptrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ptrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ptrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ptrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ptrpll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ptrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ptrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ptrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ofrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ofrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ofrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ofrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_prrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_prrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_prrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_prrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ofrpll()
{
	unsigned int addr;
	void *buf = 0;
	int value;

	value = 0;
	if (IsaCheckCond())
	{
		buf = &value;
		addr = IsaGetAddrAmode2();
		IsaRegLoad(inst.getBytes()->sdtr.src_dst_rd, value);
		value = value & (0x000000ff);
		memory->Write(addr, 1, (char *)buf);
	}
}

void Context::ExecuteInst_STRB_ofrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ofrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_ofrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_prrpll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_prrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_prrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRB_prrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRT_ptim()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRT_ptip()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRT_ptrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRT_ptrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRT_ptrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRT_ptrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRT_ptrpll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRT_ptrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRT_ptrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRT_ptrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRT_ptim()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRT_ptip()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRT_ptrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRT_ptrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRT_ptrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRT_ptrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRT_ptrpll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRT_ptrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRT_ptrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRT_ptrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRBT_ptim()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRBT_ptip()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRBT_ptrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRBT_ptrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRBT_ptrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRBT_ptrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRBT_ptrpll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRBT_ptrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRBT_ptrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDRBT_ptrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRBT_ptim()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRBT_ptip()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRBT_ptrmll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRBT_ptrmlr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRBT_ptrmar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRBT_ptrmrr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRBT_ptrpll()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRBT_ptrplr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRBT_ptrpar()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STRBT_ptrprr()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDMDA()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDMDA_w()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDMDA_u()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDMDA_uw()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDMIA()
{

	if(IsaCheckCond())
		IsaAmode4sLd();
}

void Context::ExecuteInst_LDMIA_w()
{
	if(IsaCheckCond())
		IsaAmode4sLd();
}

void Context::ExecuteInst_LDMIA_u()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDMIA_uw()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDMDB()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDMDB_w()
{
	if(IsaCheckCond())
		IsaAmode4sLd();
}

void Context::ExecuteInst_LDMDB_u()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDMDB_uw()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDMIB()
{

	if(IsaCheckCond())
		IsaAmode4sLd();
}

void Context::ExecuteInst_LDMIB_w()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDMIB_u()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDMIB_uw()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STMDA()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STMDA_w()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STMDA_u()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STMDA_uw()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STMIA()
{
	if(IsaCheckCond())
		IsaAmode4sStr();
}

void Context::ExecuteInst_STMIA_w()
{
	if(IsaCheckCond())
		IsaAmode4sStr();
}

void Context::ExecuteInst_STMIA_u()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STMIA_uw()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STMDB()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STMDB_w()
{
	if(IsaCheckCond())
		IsaAmode4sStr();
}

void Context::ExecuteInst_STMDB_u()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STMDB_uw()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STMIB()
{
	if(IsaCheckCond())
		IsaAmode4sStr();
}

void Context::ExecuteInst_STMIB_w()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STMIB_u()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STMIB_uw()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_BRNCH()
{
	if(IsaCheckCond())
		IsaBranch();
}

void Context::ExecuteInst_BRNCH_LINK()
{
	if(IsaCheckCond())
		IsaBranch();
}

void Context::ExecuteInst_SWI_SVC()
{
	IsaSyscall();
}

void Context::ExecuteInst_LDC2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STC2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_LDF()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_STF()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_VFP_LDMIA1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_VFP_LDMIA2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_VFP_LDMIA3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_VFP_STMIA1()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_VFP_STMIA2()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_VFP_STMIA3()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_VFP_MSR()
{
	throw misc::Panic("Unimplemented instruction");
}

void Context::ExecuteInst_VFP_MRS()
{
	throw misc::Panic("Unimplemented instruction");
}

} // namespace ARM

