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
#include "Emu.h"
#include "Regs.h"

namespace ARM
{

unsigned int Context::IsaRotr(unsigned int value, int shift)
{
	// Rotating 32 bits on a 32-bit integer is the same as rotating 0 bits; 33 bits -> 1 bit; etc.
	if (shift >= 32 || shift <= -32) {
		shift = shift % 32;
	}

	unsigned int temp = value;

	// Rotate input to the right
	value = value >> shift;

	// Build mask for carried over bits
	temp = temp << (32 - shift);

	return value | temp;
}


int Context::IsaGetOp2(unsigned int op2, ContextOp2Catecory cat)
{
	unsigned int imm;
	unsigned int rotate;
	int op_val;
	unsigned int shift;
	unsigned int rm;
	unsigned int rs;
	unsigned int shift_imm;
	int rm_val;
	int rs_val;
	if (cat == ContextOp2CatecoryImmd)
	{
		imm = op2 & (0x000000ff);
		rotate = (op2 >> 8) & 0x0000000f;
		op_val = IsaRotr(imm, rotate * 2);
	}
	else if (cat == ContextOp2CatecoryReg)
	{
		rm = (op2 & (0x0000000f));
		IsaRegLoad(rm, rm_val);
		shift = ((op2 >> 4) & (0x000000ff));
		shift_imm = ((shift >> 3) & 0x0000001f);

		if (shift & 0x00000001)
		{
			rs = (shift >> 4);
			IsaRegLoad(rs, rs_val);

			switch ((shift >> 1) & 0x00000003)
			{
			case AsmShiftOperatorLsl:

				op_val = rm_val << (rs_val & 0x000000ff);
				break;

			case AsmShiftOperatorLsr:

				op_val = ((unsigned int)rm_val) >> (rs_val & 0x000000ff) ;
				break;

			case AsmShiftOperatorAsr:

				op_val = rm_val / (1 << (rs_val & 0x000000ff));
				break;

			case AsmShiftOperatorRor:

				op_val = IsaRotr(rm_val, (rs_val & 0x000000ff));
				break;

			default:

				throw misc::Panic(misc::fmt("shift mode NOT supported"));
			}
		}
		else
		{
			switch ((shift >> 1) & 0x00000003)
			{
			case AsmShiftOperatorLsl:

				op_val = rm_val << shift_imm;
				break;

			case AsmShiftOperatorLsr:

				op_val = ((unsigned int)rm_val) >> shift_imm;
				break;

			case AsmShiftOperatorAsr:

				op_val = rm_val /(1 << shift_imm);
				break;

			case AsmShiftOperatorRor:

				op_val = IsaRotr(rm_val, shift_imm);
				break;

			default:

				throw misc::Panic(misc::fmt("shift mode NOT supported"));
			}
		}
	}
	emu->isa_debug << misc::fmt("  op2 = %d; 0x%x\n", op_val, op_val);
	return op_val;
}


int Context::IsaOp2Carry(unsigned int op2, ContextOp2Catecory cat)
{
	unsigned int imm;
	unsigned int rotate;
	unsigned int shift;
	unsigned int rm;
	unsigned int rs;
	unsigned int shift_imm;
	signed int rm_val;
	unsigned int carry_ret;
	unsigned int cry_bit;
	unsigned int cry_mask;
	unsigned int rot_val;
	unsigned int imm_8r;
	int rs_val;

	if (cat == ContextOp2CatecoryImmd)
	{
		imm = (op2 & (0x000000ff));
		rotate = ((op2 >> 8) & 0x0000000f);
		imm_8r = IsaRotr( imm , rotate);

		if (rotate == 0)
		{
			carry_ret = -1;
		}
		else
		{
			carry_ret = ((0x80000000) & imm_8r);
		}
		cry_bit = rotate;
		cry_mask = (unsigned int)(1 << cry_bit);

		if (cry_mask & imm)
			carry_ret = 1;
		else
			carry_ret = 0;
		emu->isa_debug << misc::fmt("  carry bit = %d, imm = 0x%x, rotate = %d\n",
				carry_ret, imm, rotate);
	}
	else if (cat == ContextOp2CatecoryReg)
	{
		rm = (op2 & (0x0000000f));
		IsaRegLoad( rm, rm_val);
		shift = ((op2 >> 4) & (0x000000ff));
		shift_imm = ((shift >> 3) & 0x0000001f);

		if (shift & 0x00000001)
		{
			rs = (shift >> 4);
			IsaRegLoad( rs, rs_val);
			switch ((shift >> 1) & 0x00000003)
			{
			case AsmShiftOperatorLsl:

				cry_bit = (32 - (rs_val & 0x000000ff));
				cry_mask = (unsigned int)(1 << cry_bit);
				if((rs_val & 0x000000ff) == 0)
				{
					carry_ret = -1;
				}
				else if ((rs_val & 0x000000ff) == 32)
				{
					carry_ret = (rm_val & 0x00000001);
				}
				else if ((rs_val & 0x000000ff) < 32)
				{
					if(cry_mask & rm_val)
						carry_ret = 1;
					else
						carry_ret = 0;
				}
				break;

			case AsmShiftOperatorLsr:

				cry_bit = ((rs_val & 0x000000ff) - 1);
				cry_mask = (unsigned int)(1 << cry_bit);
				if ((rs_val & 0x000000ff) == 0)
				{
					carry_ret = -1;
				}
				else if ((rs_val & 0x000000ff) == 32)
				{
					carry_ret = (rm_val & 0x80000000);
				}
				else if ((rs_val & 0x000000ff) < 32)
				{
					if(cry_mask & rm_val)
						carry_ret = 1;
					else
						carry_ret = 0;
				}
				break;

			case AsmShiftOperatorAsr:

				cry_bit = ((rs_val & 0x000000ff) - 1);
				cry_mask = (unsigned int)(1 << cry_bit);
				if((rs_val & 0x000000ff) == 0)
				{
					carry_ret = -1;
				}
				else if ((rs_val & 0x000000ff) >= 32)
				{
					if(rm_val & 0x80000000)
					{
						carry_ret = 1;
					}
					else
					{
						carry_ret = 0;
					}
				}
				else if ((rs_val & 0x000000ff) < 32)
				{
					if(cry_mask & rm_val)
						carry_ret = 1;
					else
						carry_ret = 0;
				}
				break;

			case AsmShiftOperatorRor:

				cry_bit = ((rs_val & 0x0000000f) - 1);
				cry_mask = (unsigned int)(1 << cry_bit);
				if((rs_val & 0x000000ff) == 0)
				{
					carry_ret = -1;
				}
				else if ((rs_val & 0x0000000f) == 0)
				{
					carry_ret = (rm_val & 0x80000000);
				}
				else
				{
					if(cry_mask & rm_val)
						carry_ret = 1;
					else
						carry_ret = 0;
				}
				break;
			}
			rot_val = rs_val;
		}

		else
		{
			switch ((shift >> 1) & 0x00000003)
			{
			case AsmShiftOperatorLsl:

				cry_bit = (32 - shift_imm);
				cry_mask = (unsigned int)(1 << cry_bit);
				if(shift_imm == 0)
				{
					carry_ret = -1;
				}
				else
				{
					if(cry_mask & rm_val)
						carry_ret = 1;
					else
						carry_ret = 0;
				}
				break;

			case AsmShiftOperatorLsr:

				cry_bit = (shift_imm - 1);
				cry_mask = (unsigned int)(1 << cry_bit);
				if(shift_imm == 0)
				{
					carry_ret = -1;
				}
				else
				{
					if(cry_mask & rm_val)
						carry_ret = 1;
					else
						carry_ret = 0;
				}
				break;

			case AsmShiftOperatorAsr:

				cry_bit = (shift_imm - 1);
				cry_mask = (unsigned int)(1 << cry_bit);
				if(shift_imm == 0)
				{
					if (rm_val & (0x80000000))
					{
						carry_ret = 1;
					}
					else
					{
						carry_ret = 0;
					}
				}
				else
				{
					if(cry_mask & rm_val)
						carry_ret = 1;
					else
						carry_ret = 0;
				}
				break;

			case AsmShiftOperatorRor:

				cry_bit = (shift_imm - 1);
				cry_mask = (unsigned int)(1 << cry_bit);
				if(shift_imm == 0)
				{
					if (rm_val & (0x00000001))
					{
						carry_ret = 1;
					}
					else
					{
						carry_ret = 0;
					}
				}
				else
				{
					if(cry_mask & rm_val)
						carry_ret = 1;
					else
						carry_ret = 0;
				}
				break;
			}
			rot_val = shift_imm;
		}

		if(shift_imm == 0)
		{
			carry_ret = -1;
		}
		else
		{
			if(cry_mask & rm_val)
				carry_ret = 1;
			else
				carry_ret = 0;
		}

		emu->isa_debug << misc::fmt("  carry bit = %d, rm_val = 0x%x, rotate = %d\n",
				carry_ret, rm_val, rot_val);
	}

	return (carry_ret);
}


unsigned int Context::IsaGetAddrAmode2()
{
	unsigned int rn;
	unsigned int rm;
	unsigned int shift;
	unsigned int offset;
	int rn_val;
	unsigned int ret_addr;
	unsigned int shift_val;
	int rm_val;

	offset = inst.getBytes()->sdtr.off;
	rn = inst.getBytes()->sdtr.base_rn;
	IsaRegLoad(rn, rn_val);
	emu->isa_debug << misc::fmt("  rn = 0x%x\n", rn_val);
	if(inst.getBytes()->sdtr.imm == 1)
	{
		rm = (offset & (0x0000000f));
		IsaRegLoad(rm, rm_val);
		shift = ((offset >> 4) & (0x000000ff));
		shift_val = ((shift >> 3) & 0x0000001f);

		if(inst.getBytes()->sdtr.idx_typ)
		{
			// Pre- Indexed
			if (inst.getBytes()->sdtr.up_dn)
			{
				// Increment
				switch ((shift >> 1) & 0x00000003)
				{
				case AsmShiftOperatorLsl:

					ret_addr = rn_val
						+ (rm_val << shift_val);
					break;

				case AsmShiftOperatorLsr:

					ret_addr = rn_val
						+ (rm_val >> shift_val);
					break;

				case AsmShiftOperatorAsr:

					ret_addr = rn_val
						+ (rm_val / (1 << shift_val));
					break;

				case AsmShiftOperatorRor:

					ret_addr = rn_val + IsaRotr(
						rm_val, shift_val);
					break;

				default:

					throw misc::Panic(misc::fmt("shift mode NOT supported"));
				}
			}
			else
			{
				// Decrement
				switch ((shift >> 1) & 0x00000003)
				{
				case AsmShiftOperatorLsl:

					ret_addr = rn_val
						- (rm_val << shift_val);
					break;

				case AsmShiftOperatorLsr:

					ret_addr = rn_val
						- (rm_val >> shift_val);
					break;

				case AsmShiftOperatorAsr:

					ret_addr = rn_val
						- (rm_val / (1 << shift_val));
					break;

				case AsmShiftOperatorRor:

					ret_addr = rn_val - IsaRotr(
						rm_val, shift_val);
					break;
				default:

					throw misc::Panic(misc::fmt("shift mode NOT supported"));
				}
			}

			if (inst.getBytes()->sdtr.wb)
				IsaRegStore(rn, ret_addr);
		}
		else
		{
			// Post Indexed
			if (inst.getBytes()->sdtr.up_dn)
			{
				// Increment
				switch ((shift >> 1) & 0x00000003)
				{
				case AsmShiftOperatorLsl:

					ret_addr = rn_val;
					rn_val = ret_addr
						+ (rm_val << shift_val);
					break;

				case AsmShiftOperatorLsr:

					ret_addr = rn_val;
					rn_val = ret_addr
						+ (rm_val >> shift_val);
					break;

				case AsmShiftOperatorAsr:

					ret_addr = rn_val;
					rn_val = ret_addr
						+ (rm_val / (1 << shift_val));
					break;

				case AsmShiftOperatorRor:

					ret_addr = rn_val;
					rn_val = ret_addr + IsaRotr(
						rm_val, shift_val);
					break;

				default:

					throw misc::Panic(misc::fmt("shift mode NOT supported"));
				}
			}
			else
			{
				// Decrement
				switch ((shift >> 1) & 0x00000003)
				{
				case AsmShiftOperatorLsl:

					ret_addr = rn_val;
					rn_val = ret_addr
						- (rm_val << shift_val);
					break;

				case AsmShiftOperatorLsr:

					ret_addr = rn_val;
					rn_val = ret_addr
						- (rm_val >> shift_val);
					break;

				case AsmShiftOperatorAsr:

					ret_addr = rn_val;
					rn_val = ret_addr
						- (rm_val / (1 << shift_val));
					break;

				case AsmShiftOperatorRor:

					ret_addr = rn_val;
					rn_val = ret_addr - IsaRotr(
						rm_val, shift_val);
					break;

				default:

					throw misc::Panic(misc::fmt("shift mode NOT supported"));
				}
			}
			IsaRegStore(rn, rn_val);
		}
	}
	else
	{
		// Register Addressing
		if (inst.getBytes()->sdtr.idx_typ)
		{
			// Pre-Indexed
			if (!offset)
			{
				ret_addr = rn_val + 0;
			}
			else
			{
				if(inst.getBytes()->sdtr.up_dn)
				{
					// Increment
					ret_addr = rn_val + offset;
				}
				else
				{
					// Decrement
					ret_addr = rn_val - offset;
				}
			}

			if (inst.getBytes()->sdtr.wb)
				IsaRegStore(rn, ret_addr);
		}
		else
		{
			// Post-Index
			if (!offset)
			{
				ret_addr = rn_val + 0;
			}
			else
			{
				if (inst.getBytes()->sdtr.up_dn)
				{
					// Increment
					ret_addr = rn_val;
					rn_val = rn_val + offset;
				}
				else
				{
					// Decrement
					ret_addr = rn_val;
					rn_val = rn_val - offset;
				}
			}
			IsaRegStore(rn, rn_val);
		}
	}


	emu->isa_debug << misc::fmt("  ls/st addr = 0x%x\n", ret_addr);
	return ret_addr;
}


unsigned int Context::IsaGetAddrAmode3Imm()
{
	unsigned int imm4l;
	unsigned int imm4h;
	unsigned int immd8;
	int rn_val;
	unsigned int ret_addr;

	imm4l = inst.getBytes()->hfwrd_imm.imm_off_lo;
	imm4h = inst.getBytes()->hfwrd_imm.imm_off_hi;
	immd8 = (0x000000ff) & ((imm4h << 4) | (imm4l));
	emu->isa_debug << misc::fmt("  imm8 offset = %d,  (0x%x)\n", immd8, immd8);

	if (inst.getBytes()->hfwrd_imm.idx_typ)
	{
		// Pre-Indexed
		IsaRegLoad(inst.getBytes()->hfwrd_imm.base_rn, rn_val);
		if (inst.getBytes()->hfwrd_imm.up_dn)
		{
			ret_addr = rn_val + immd8;
		}

		else
		{
			ret_addr = rn_val - immd8;
		}

		if (inst.getBytes()->hfwrd_imm.wb)
		{
			rn_val = ret_addr;
			IsaRegStore(inst.getBytes()->hfwrd_imm.base_rn, rn_val);

		}
		emu->isa_debug << misc::fmt("  ld/str addr = %d,  (0x%x)\n", ret_addr, ret_addr);
		return ret_addr;
	}
	else
	{
		// Post Indexed
		IsaRegLoad(inst.getBytes()->hfwrd_imm.base_rn, rn_val);
		if (inst.getBytes()->hfwrd_imm.up_dn)
		{
			ret_addr = rn_val;
		}

		else
		{
			ret_addr = rn_val;
		}

		if (inst.getBytes()->hfwrd_imm.wb)
		{
			throw misc::Panic(misc::fmt("%s: Arm instruction not according to v7 ISA specification,"
					" unpredictable behavior possible. Please check your "
					"compiler flags.\n",__FUNCTION__));
		}
		emu->isa_debug << misc::fmt("  addr = %d,  (0x%x)\n", ret_addr, ret_addr);
		return ret_addr;
	}
}


void Context::IsaRegStore(unsigned int reg_no, int value)
{
	emu->isa_debug << misc::fmt("  r%d <= %d; (0x%x)\n", reg_no, value, value);
	switch (reg_no)
	{
	case AsmUserRegistersR0:

		regs.setRegister(0, value);
		break;

	case AsmUserRegistersR1:

		regs.setRegister(1, value);
		break;

	case AsmUserRegistersR2:

		regs.setRegister(2, value);
		break;

	case AsmUserRegistersR3:

		regs.setRegister(3, value);
		break;

	case AsmUserRegistersR4:

		regs.setRegister(4, value);
		break;

	case AsmUserRegistersR5:

		regs.setRegister(5, value);
		break;

	case AsmUserRegistersR6:

		regs.setRegister(6, value);
		break;

	case AsmUserRegistersR7:

		regs.setRegister(7, value);
		break;

	case AsmUserRegistersR8:

		regs.setRegister(8, value);
		break;

	case AsmUserRegistersR9:

		regs.setRegister(9, value);
		break;

	case AsmUserRegistersR10:

		regs.setSL(value);
		break;

	case AsmUserRegistersR11:

		regs.setFP(value);
		break;

	case AsmUserRegistersR12:

		regs.setIP(value);
		break;

	case AsmUserRegistersR13:

		regs.setSP(value);
		break;

	case AsmUserRegistersR14:

		regs.setLR(value);
		break;

	case AsmUserRegistersR15:

		regs.setPC(value + 4);
		break;

	default:

		throw misc::Panic(misc::fmt("The register number is wrong\n"));
	}
}


void Context::IsaRegStoreSafe(unsigned int reg_no, unsigned int value)
{
	emu->isa_debug << misc::fmt("  r%d <= %d; (0x%x); safe_store\n", reg_no, value, value);
	switch (reg_no)
	{
	case AsmUserRegistersR0:

		regs.setRegister(0, value);
		break;

	case AsmUserRegistersR1:

		regs.setRegister(1, value);
		break;

	case AsmUserRegistersR2:

		regs.setRegister(2, value);
		break;

	case AsmUserRegistersR3:

		regs.setRegister(3, value);
		break;

	case AsmUserRegistersR4:

		regs.setRegister(4, value);
		break;

	case AsmUserRegistersR5:

		regs.setRegister(5, value);
		break;

	case AsmUserRegistersR6:

		regs.setRegister(6, value);
		break;

	case AsmUserRegistersR7:

		regs.setRegister(7, value);
		break;

	case AsmUserRegistersR8:

		regs.setRegister(8, value);
		break;

	case AsmUserRegistersR9:

		regs.setRegister(9, value);
		break;

	case AsmUserRegistersR10:

		regs.setSL(value);
		break;

	case AsmUserRegistersR11:

		regs.setFP(value);
		break;

	case AsmUserRegistersR12:

		regs.setIP(value);
		break;

	case AsmUserRegistersR13:

		regs.setSP(value);
		break;

	case AsmUserRegistersR14:

		regs.setLR(value);
		break;

	case AsmUserRegistersR15:

		regs.setPC(value + 4);
		break;

	default:

		throw misc::Panic(misc::fmt("The register number is wrong\n"));
	}
}


void Context::IsaRegLoad(unsigned int reg_no, int &value)
{
	switch (reg_no)
	{
	case AsmUserRegistersR0:

		value = regs.getRegister(0);
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getRegister(0));
		break;

	case AsmUserRegistersR1:

		value = regs.getRegister(1);
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getRegister(1));
		break;

	case AsmUserRegistersR2:

		value = regs.getRegister(2);
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getRegister(2));
		break;

	case AsmUserRegistersR3:

		value = regs.getRegister(3);
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getRegister(3));
		break;

	case AsmUserRegistersR4:

		value = regs.getRegister(4);
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getRegister(4));
		break;

	case AsmUserRegistersR5:

		value = regs.getRegister(5);
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getRegister(5));
		break;

	case AsmUserRegistersR6:

		value = regs.getRegister(6);
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getRegister(6));
		break;

	case AsmUserRegistersR7:

		value = regs.getRegister(7);
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getRegister(7));
		break;

	case AsmUserRegistersR8:

		value = regs.getRegister(8);
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getRegister(8));
		break;

	case AsmUserRegistersR9:

		value = regs.getRegister(9);
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getRegister(9));
		break;

	case AsmUserRegistersR10:

		value = regs.getSL();
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getSL());
		break;

	case AsmUserRegistersR11:

		value = regs.getFP();
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getFP());
		break;

	case AsmUserRegistersR12:

		value = regs.getIP();
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getIP());
		break;

	case AsmUserRegistersR13:

		value = regs.getSP();
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getSP());
		break;

	case AsmUserRegistersR14:

		value = regs.getLR();
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getLR());
		break;

	case AsmUserRegistersR15:

		value = regs.getPC();
		emu->isa_debug << misc::fmt("  r%d = 0x%x\n", reg_no, regs.getPC());
		break;

	default:

		throw misc::Panic(misc::fmt("The register number is wrong\n"));
	}
}


void Context::IsaBranch()
{
	unsigned int offset;
	unsigned int br_add;
	int rm_val;

	if (inst.getInstInfo()->category == InstCategoryBrnch)
	{
		offset = inst.getBytes()->brnch.off << 2;
		br_add = offset + regs.getPC();

		if(inst.getBytes()->brnch.link)
		{
			IsaRegStore(14, regs.getPC() - 4);
		}

		regs.setPC(br_add + 4);
		emu->isa_debug << misc::fmt("  Branch addr = 0x%x, pc <= %d\n",
				regs.getPC() - 4, regs.getPC());
	}
	else if (inst.getInstInfo()->category == InstCategoryBax)
	{
		if((inst.getOpcode() == InstOpcodeBLX))
		{
			IsaRegLoad(inst.getBytes()->bax.op0_rn, rm_val);
			IsaRegStore(14, regs.getPC() - 4);
			regs.setPC((rm_val & 0xfffffffe) + 4);
			emu->isa_debug << misc::fmt("  Branch addr = 0x%x, pc <= %d\n", rm_val, rm_val);
		}
		else
		{
			IsaRegLoad(inst.getBytes()->bax.op0_rn, rm_val);
			regs.setPC((rm_val & 0xfffffffe) + 4);
			emu->isa_debug << misc::fmt("  Branch addr = 0x%x, pc <= %d\n", rm_val, rm_val);
		}
	}
}


bool Context::IsaCheckCond()
{
	unsigned int ret_val;
	unsigned int cond = inst.getBytes()->brnch.cond;
	switch (cond)
	{
	case (AsmConditionCodesEQ):

		ret_val = (regs.getCPSR().z) ? true : false;
		emu->isa_debug << misc::fmt("  Cond = EQ\n");
		break;

	case (AsmConditionCodesNE):

		ret_val = (!(regs.getCPSR().z)) ? true : false;
		emu->isa_debug << misc::fmt("  Cond = NE\n");
		break;

	case (AsmConditionCodesCS):

		ret_val = (regs.getCPSR().C) ? true : false;
		emu->isa_debug << misc::fmt("  Cond = CS\n");
		break;

	case (AsmConditionCodesCC):

		ret_val = (!(regs.getCPSR().C)) ? true : false;
		emu->isa_debug << misc::fmt("  Cond = CC\n");
		break;

	case (AsmConditionCodesMI):
		ret_val = (regs.getCPSR().n) ? true : false;
		emu->isa_debug << misc::fmt("  Cond = MI\n");
		break;

	case (AsmConditionCodesPL):

		ret_val = (!(regs.getCPSR().n)) ? true : false;
		emu->isa_debug << misc::fmt("  Cond = PL\n");
		break;

	case (AsmConditionCodesVS):

		ret_val = (regs.getCPSR().v) ? true : false;
		emu->isa_debug << misc::fmt("  Cond = VS\n");
		break;

	case (AsmConditionCodesVC):

		ret_val = (!(regs.getCPSR().v)) ? true : false;
		emu->isa_debug << misc::fmt("  Cond = VC\n");
		break;

	case (AsmConditionCodesHI):

		ret_val = (!(regs.getCPSR().z) && (regs.getCPSR().C)) ? true : false;
		emu->isa_debug << misc::fmt("  Cond = HI\n");
		break;

	case (AsmConditionCodesLS):

		ret_val = ((regs.getCPSR().z) | !(regs.getCPSR().C)) ? true : false;
		emu->isa_debug << misc::fmt("  Cond = LS\n");
		break;

	case (AsmConditionCodesGE):

		ret_val = (((regs.getCPSR().n) & (regs.getCPSR().v))
			| (!(regs.getCPSR().n) & !(regs.getCPSR().v))) ? true : false;
		emu->isa_debug << misc::fmt("  Cond = GE\n");
		break;

	case (AsmConditionCodesLT):

		ret_val = (((regs.getCPSR().n) & !(regs.getCPSR().v))
			| (!(regs.getCPSR().n) && (regs.getCPSR().v))) ? true : false;
		emu->isa_debug << misc::fmt("  Cond = LT\n");
		break;

	case (AsmConditionCodesGT):

		ret_val = (((regs.getCPSR().n) & (regs.getCPSR().v) & !(regs.getCPSR().z))
			| (!(regs.getCPSR().n) & !(regs.getCPSR().v)
			& !(regs.getCPSR().z))) ? true : false;
		emu->isa_debug << misc::fmt("  Cond = GT\n");
		break;

	case (AsmConditionCodesLE):

		ret_val = (((regs.getCPSR().z) | (!(regs.getCPSR().n) && (regs.getCPSR().v))
			| ((regs.getCPSR().n) && !(regs.getCPSR().v)))) ? true : false;
		emu->isa_debug << misc::fmt("  Cond = LE\n");
		break;

	case (AsmConditionCodesAL):

		ret_val = true;
		break;

	default:

		throw misc::Panic(misc::fmt("Unsupported condition"));
	}

	return (ret_val);
}


void Context::IsaAmode4sStr()
{
	unsigned int wrt_val;
	int rn_val;
	int copy_buf;

	void *buf = &copy_buf;
	unsigned int reg_list = inst.getBytes()->bdtr.reg_lst;
	IsaRegLoad(inst.getBytes()->bdtr.base_rn, rn_val);
	if (inst.getBytes()->bdtr.idx_typ)
	{
		// Pre indexed
		if (inst.getBytes()->bdtr.up_dn)
		{
			wrt_val = rn_val + 4;

			for (int i = 1; i < 65536; i *= 2)
			{
				if(reg_list & (i))
				{

					IsaRegLoad(misc::LogBase2(i), copy_buf);
					memory->Write(wrt_val, 4, (char *)buf);
					emu->isa_debug << misc::fmt("  push r%d => 0x%x\n",
							misc::LogBase2(i),wrt_val);
					wrt_val += 4;
				}
			}
			if(inst.getBytes()->bdtr.wb)
				IsaRegStore(inst.getBytes()->bdtr.base_rn, (wrt_val - 4));
		}

		else
		{
			wrt_val = rn_val - 4;

			for (int i = 32768; i >= 1; i /= 2)
			{
				if(reg_list & (i))
				{
					IsaRegLoad(misc::LogBase2(i), copy_buf);
					memory->Write(wrt_val, 4, (char *)buf);
					emu->isa_debug << misc::fmt("  push r%d => 0x%x\n",
							misc::LogBase2(i),wrt_val);
					wrt_val -= 4;
				}
			}
			if(inst.getBytes()->bdtr.wb)
				IsaRegStore(inst.getBytes()->bdtr.base_rn, (wrt_val + 4));

		}
	}
	else
	{
		// Post-Indexed
		if (inst.getBytes()->bdtr.up_dn)
		{
			wrt_val = rn_val;

			for (int i = 1; i < 65536; i *= 2)
			{
				if(reg_list & (i))
				{

					IsaRegLoad(misc::LogBase2(i), copy_buf);
					memory->Write(wrt_val, 4, (char *)buf);
					emu->isa_debug << misc::fmt("  push r%d => 0x%x\n",
							misc::LogBase2(i),wrt_val);
					wrt_val += 4;
				}
			}
			if(inst.getBytes()->bdtr.wb)
				IsaRegStore(inst.getBytes()->bdtr.base_rn, (wrt_val));
		}

		else
		{
			wrt_val = rn_val;

			for (int i = 32768; i >= 1; i /= 2)
			{
				if(reg_list & (i))
				{
					IsaRegLoad(misc::LogBase2(i), copy_buf);
					memory->Write(wrt_val, 4, (char *)buf);
					emu->isa_debug << misc::fmt("  push r%d => 0x%x\n",
							misc::LogBase2(i),wrt_val);
					wrt_val -= 4;
				}
			}
			if(inst.getBytes()->bdtr.wb)
				IsaRegStore(inst.getBytes()->bdtr.base_rn, (wrt_val));

		}
	}
}


void Context::IsaAmode4sLd()
{
	unsigned int read_val;
	int rn_val;
	int copy_buf;

	void *buf = &copy_buf;
	unsigned int reg_list = inst.getBytes()->bdtr.reg_lst;
	IsaRegLoad(inst.getBytes()->bdtr.base_rn, rn_val);
	if(inst.getBytes()->bdtr.idx_typ)
	{
		// Pre indexed
		if (inst.getBytes()->bdtr.up_dn)
		{
			read_val = rn_val + 4;

			for (int i = 1; i < 65536; i *= 2)
			{
				if(reg_list & (i))
				{
					memory->Read(read_val, 4, (char *)buf);
					if(copy_buf % 2 && misc::LogBase2(i) == 15)
					{
						copy_buf = copy_buf - 1;
					}
					IsaRegStore(misc::LogBase2(i), copy_buf);
					emu->isa_debug << misc::fmt("  pop r%d <= 0x%x\n",
							misc::LogBase2(i),read_val);
					read_val += 4;
				}
			}
			if(inst.getBytes()->bdtr.wb)
				IsaRegStore(inst.getBytes()->bdtr.base_rn, (read_val - 4));
		}

		else
		{
			read_val = rn_val - 4;

			for (int i = 32768; i >= 1; i /= 2)
			{
				if(reg_list & (i))
				{
					memory->Read(read_val, 4, (char *)buf);
					if(copy_buf % 2 && misc::LogBase2(i) == 15)
					{
						copy_buf = copy_buf - 1;
					}
					IsaRegStore(misc::LogBase2(i), copy_buf);
					emu->isa_debug << misc::fmt("  pop r%d <= 0x%x\n",
							misc::LogBase2(i),read_val);
					read_val -= 4;
				}
			}
			if(inst.getBytes()->bdtr.wb)
				IsaRegStore(inst.getBytes()->bdtr.base_rn, (read_val + 4));

		}
	}
	else
	{
		// Post-Indexed
		if (inst.getBytes()->bdtr.up_dn)
		{
			read_val = rn_val;

			for (int i = 1; i < 65536; i *= 2)
			{
				if(reg_list & (i))
				{
					memory->Read(read_val, 4, (char *)buf);
					if(copy_buf % 2 && misc::LogBase2(i) == 15)
					{
						copy_buf = copy_buf - 1;
					}
					IsaRegStore(misc::LogBase2(i), copy_buf);
					emu->isa_debug << misc::fmt("  pop r%d <= 0x%x\n",
							misc::LogBase2(i),read_val);
					read_val += 4;
				}
			}
			if(inst.getBytes()->bdtr.wb)
				IsaRegStore(inst.getBytes()->bdtr.base_rn, (read_val));
		}

		else
		{
			read_val = rn_val;

			for (int i = 32768; i >= 1; i /= 2)
			{
				if(reg_list & (i))
				{
					memory->Read(read_val, 4, (char *)buf);
					if(copy_buf % 2 && misc::LogBase2(i) == 15)
					{
						copy_buf = copy_buf - 1;
					}
					IsaRegStore(misc::LogBase2(i), copy_buf);
					emu->isa_debug << misc::fmt("  pop r%d <= 0x%x\n",
							misc::LogBase2(i),read_val);
					read_val -= 4;
				}
			}
			if(inst.getBytes()->bdtr.wb)
				IsaRegStore(inst.getBytes()->bdtr.base_rn, (read_val));

		}
	}
}


void Context::IsaCpsrPrint()
{
	emu->isa_debug << misc::fmt("  CPSR update\n"
			"  n = %d\n"
			"  z = %d\n"
			"  c = %d\n"
			"  v = %d\n"
			"  q = %d\n"
			"  mode = 0x%x\n",regs.getCPSR().n,regs.getCPSR().z,regs.getCPSR().C,
			regs.getCPSR().v,regs.getCPSR().q,regs.getCPSR().mode);
}


unsigned int Context::IsaRetCpsrVal()
{
	unsigned int cpsr_val = ((regs.getCPSR().n << 31) | (regs.getCPSR().z << 30)
			| (regs.getCPSR().C << 29) | (regs.getCPSR().v) | (regs.getCPSR().mode));

	emu->isa_debug << misc::fmt("  cpsr = 0x%x\n",cpsr_val);

	return (cpsr_val);
}


void Context::IsaSetCpsrVal(unsigned int op2)
{
	int rd_val;
	unsigned int rd = op2 & 0x0000000f;
	IsaRegLoad(rd, rd_val);

	unsigned int rd_str = (unsigned int)(rd_val);
	emu->isa_debug << misc::fmt("  rd_str = 0x%x\n",rd_str);
	regs.getCPSR().n = (rd_str & (0x80000000)) ? 1 : 0;
	regs.getCPSR().z = (rd_str & (0x40000000)) ? 1 : 0;
	regs.getCPSR().C = (rd_str & (0x20000000)) ? 1 : 0;
	regs.getCPSR().v = (rd_str & (0x10000000)) ? 1 : 0;
	regs.getCPSR().mode = (unsigned int)(rd_str & 0x000000ff);
	IsaCpsrPrint();
}


void Context::IsaSubtract(unsigned int rd, unsigned int rn, int op2,
		unsigned int op3)
{
	int rd_val;
	int rn_val;
	int operand2;
	unsigned int rd_val_safe;
	unsigned long flags = 0;
	IsaRegLoad(rn, rn_val);

	if (!(inst.getBytes()->dpr.s_cond))
	{
		if (rd == 15)
		{
			rd_val_safe = rn_val - op2 - op3;
			emu->isa_debug << misc::fmt("  r%d = r%d - %d\n", rd, rn, op2);
			IsaRegStoreSafe(rd, rd_val_safe);
		}
		else
		{
			rd_val = rn_val - op2 - op3;
			emu->isa_debug << misc::fmt("  r%d = r%d - %d\n", rd, rn, op2);
			IsaRegStore(rd, rd_val);
		}
	}
	else
	{
		rd_val = rn_val - op2 - op3;
		emu->isa_debug << misc::fmt("  r%d = r%d - %d\n", rd, rn, op2);

		operand2 = (-1 * (op2  + op3));

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
				"mov %%eax, %4\n\t"
				"pushf\n\t"
				"pop %0\n\t"
				: "=g" (flags), "=m" (operand2)
				  : "m" (operand2), "m" (rn_val), "m" (rd_val), "g" (flags)
				    : "eax"
		);

		emu->isa_debug << misc::fmt("  flags = 0x%lx\n", flags);
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

		IsaRegStore(rd, rd_val);
		IsaCpsrPrint();
	}
}


void Context::IsaSubtractRev(unsigned int rd, unsigned int rn, int op2,
		unsigned int op3)
{
	int rd_val;
	int rn_val;
	int operand2;
	unsigned long flags = 0;

	IsaRegLoad( rn, rn_val);
	if (!(inst.getBytes()->dpr.s_cond))
	{
		rd_val = op2 - rn_val - op3;
		emu->isa_debug << misc::fmt("  r%d = r%d - %d\n", rd, rn, op2);
		IsaRegStore(rd, rd_val);
	}
	else
	{
		rd_val = op2 - rn_val - op3;
		emu->isa_debug << misc::fmt("  r%d = r%d - %d\n", rd, rn, op2);

		operand2 = (-1 * (rn_val + op3));

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
				"mov %%eax, %4\n\t"
				"pushf\n\t"
				"pop %0\n\t"
				: "=g" (flags), "=m" (operand2)
				  : "m" (operand2), "m" (op2), "m" (rd_val), "g" (flags)
				    : "eax"
		);

		emu->isa_debug << misc::fmt("  flags = 0x%lx\n", flags);
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

		IsaRegStore(rd, rd_val);
		IsaCpsrPrint();
	}
}


void Context::IsaAdd(unsigned int rd, unsigned int rn, int op2,
		unsigned int op3)
{
	int rd_val;
	int rn_val;
	unsigned long flags = 0;

	IsaRegLoad( rn, rn_val);
	if (!(inst.getBytes()->dpr.s_cond))
	{
		rd_val = rn_val + op2 + op3;
		emu->isa_debug << misc::fmt("  r%d = r%d + %d\n", rd, rn, op2);
		IsaRegStore(rd, rd_val);
	}
	else
	{

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;
		rn_val = rn_val + op3;
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

		emu->isa_debug << misc::fmt("  r%d = r%d + %d\n", rd, rn, op2);
		IsaRegStore(rd, rd_val);
		IsaCpsrPrint();
	}
}


void Context::IsaMultiply()
{
	int rm_val;
	int rs_val;
	int rd_val;
	int rn_val;

	if (!(inst.getBytes()->mult.m_acc))
	{
		IsaRegLoad( inst.getBytes()->mult.op1_rs, rs_val);
		IsaRegLoad( inst.getBytes()->mult.op0_rm, rm_val);

		rd_val = rm_val * rs_val;

		IsaRegStore(inst.getBytes()->mult.dst_rd, rd_val);
	}
	else
	{

		IsaRegLoad( inst.getBytes()->mult.op1_rs, rs_val);
		IsaRegLoad( inst.getBytes()->mult.op0_rm, rm_val);
		IsaRegLoad( inst.getBytes()->mult.op2_rn, rn_val);

		rd_val = (rm_val * rs_val) + rn_val;

		IsaRegStore(inst.getBytes()->mult.dst_rd, rd_val);
	}
}


unsigned int Context::IsaBitCount(unsigned int ip_num)
{
	unsigned int count = 0;

	while (ip_num)
	{
		ip_num &= ip_num - 1;
		++count;
	}
	return count;
}


void Context::IsaSyscall()
{
	// Set specific value to register 7
	if (regs.getRegister(7) == ARM_SET_TLS)
	{
		regs.setRegister(7, 330);
	}
	else if (regs.getRegister(7) == ARM_EXIT_GROUP)
	{
		regs.setRegister(7, 252);
	}

	// Execute the system call
	ExecuteSyscall();

	// Debug
	if(regs.getRegister(0) == 0)
		emu->isa_debug << misc::fmt("  System call code = %d\n", regs.getRegister(7));
	else
		emu->isa_debug << misc::fmt("  System call code = %d\n", regs.getRegister(7));
}


bool Context::IsaInvalidAddrStr(unsigned int addr, int value)
{
	if (addr == 0x5bd4dc)
	{
		// Fault subroutine return address
		fault_addr = addr;
		fault_value = value;
		return true;
	}
	else if (addr == 0x5bd4c8)
	{
		// Fault subroutine return address
		fault_addr = addr;
		fault_value = value;
		return true;
	}
	else if (addr == 0x5bd080)
	{
		// Fault subroutine return address
		return true;
	}
	else if (addr == 0x5bd4d8)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool Context::IsaInvalidAddrLdr(unsigned int addr, unsigned int *value)
{
	mem::Memory::Page *page;
	if (addr == 0x5bd4dc)
	{
		// Fault subroutine return address
		if(fault_addr == addr)
			*(value) = fault_value;
		return true;
	}
	else if (addr == 0x5bd4c8)
	{
		// Fault subroutine return address
		*(value) = 0x8fd6c;
		return true;
	}
	else if (addr == 0x5bd080)
	{
		return true;
	}
	else if (addr == 0x5bd07c)
	{
		*(value) = 0;
		return true;
	}
	else if (addr == 0x5bd4d8)
	{
		*(value) = 0;
		return true;
	}
	else if (addr == 0x8fd70)
	{

		page = memory->getPage(addr);

		if(!page)
		{
			*(value) = 0x81cc4;
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (addr == 0x8fda0)
	{
		page = memory->getPage(addr);

		if(!page)
		{
			*(value) = 0x76230;
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}



void Context::IsaThumbAdd(unsigned int rd, unsigned int rn, int op2,
		unsigned int op3, unsigned int flag_set)
{
	int rd_val;
	int rn_val;
	unsigned long flags = 0;

	IsaRegLoad(rn, rn_val);
	if (!flag_set)
	{
		rd_val = rn_val + op2 + op3;
		emu->isa_debug << misc::fmt("  r%d = r%d + %d\n", rd, rn, op2);
		IsaRegStore(rd, rd_val);
	}
	else
	{

		regs.getCPSR().z = 0;
		regs.getCPSR().n = 0;
		regs.getCPSR().C = 0;
		regs.getCPSR().v = 0;
		rn_val = rn_val + op3;
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

		emu->isa_debug << misc::fmt("  r%d = r%d + %d\n", rd, rn, op2);
		IsaRegStore(rd, rd_val);
		IsaCpsrPrint();
	}
}


void Context::IsaThumbSubtract(unsigned int rd, unsigned int rn, int op2,
		unsigned int op3, unsigned int flag_set)
{
	int rd_val;
	int rn_val;
	int operand2;
	unsigned int rd_val_safe;
	unsigned long flags = 0;

	IsaRegLoad(rn, rn_val);
	if (!(flag_set))
	{
		if (rd == 15)
		{
			rd_val_safe = rn_val - op2 - op3;
			emu->isa_debug << misc::fmt("  r%d = r%d - %d\n", rd, rn, op2);
			IsaRegStoreSafe(rd, rd_val_safe);
		}
		else
		{
			rd_val = rn_val - op2 - op3;
			emu->isa_debug << misc::fmt("  r%d = r%d - %d\n", rd, rn, op2);
			IsaRegStore(rd, rd_val);
		}
	}
	else
	{
		rd_val = rn_val - op2 - op3;
		emu->isa_debug << misc::fmt("  r%d = r%d - %d\n", rd, rn, op2);

		operand2 = (-1 * (op2  + op3));

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
				"mov %%eax, %4\n\t"
				"pushf\n\t"
				"pop %0\n\t"
				: "=g" (flags), "=m" (operand2)
				  : "m" (operand2), "m" (rn_val), "m" (rd_val), "g" (flags)
				    : "eax"
		);

		emu->isa_debug << misc::fmt("  flags = 0x%lx\n", flags);
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

		IsaRegStore(rd, rd_val);
		IsaCpsrPrint();
	}
}


void Context::IsaThumbRevSubtract(unsigned int rd, unsigned int rn, int op2,
		unsigned int op3, unsigned int flag_set)
{
	int rd_val;
	int rn_val;
	int operand2;
	unsigned long flags = 0;

	IsaRegLoad(rn, rn_val);
	if (!(flag_set))
	{
		rd_val = op2 - rn_val - op3;
		emu->isa_debug << misc::fmt("  r%d = r%d - %d\n", rd, rn, op2);
		IsaRegStore(rd, rd_val);
	}
	else
	{
		rd_val = op2 - rn_val - op3;
		emu->isa_debug << misc::fmt("  r%d = r%d - %d\n", rd, rn, op2);

		operand2 = (-1 * (rn_val + op3));

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
				"mov %%eax, %4\n\t"
				"pushf\n\t"
				"pop %0\n\t"
				: "=g" (flags), "=m" (operand2)
				  : "m" (operand2), "m" (op2), "m" (rd_val), "g" (flags)
				    : "eax"
		);

		emu->isa_debug << misc::fmt("  flags = 0x%lx\n", flags);
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

		IsaRegStore(rd, rd_val);
		IsaCpsrPrint();
	}
}


unsigned int Context::IsaThumb32Immd12(InstThumb32Category cat)
{
	unsigned int immd8;
	unsigned int immd3;
	unsigned int i;
	unsigned int imm4;
	unsigned int imm5;
	unsigned int shft;
	unsigned int const_val;

	if (cat == InstThumb32CategoryDprImm)
	{
		immd8 = inst.getThumb32Bytes()->data_proc_immd.immd8;
		immd3 = inst.getThumb32Bytes()->data_proc_immd.immd3;
		i = inst.getThumb32Bytes()->data_proc_immd.i_flag;
	}
	else if (cat == InstThumb32CategoryDprBinImm)
	{
		immd8 = inst.getThumb32Bytes()->data_proc_immd.immd8;
		immd3 = inst.getThumb32Bytes()->data_proc_immd.immd3;
		i = inst.getThumb32Bytes()->data_proc_immd.i_flag;
	}
	else
		throw misc::Panic(misc::fmt("%d: immd12 fmt not recognized", cat));

	imm4 = (i << 3) | (immd3);
	if (imm4 < 4)
	{
		switch(imm4)
		{
		case(0) :

			const_val =  immd8;
			break;

		case(1) :

			const_val = (immd8 << 16) | immd8;
			break;

		case(2) :

			const_val = (immd8 << 24) | (immd8 << 8);
			break;

		case(3) :

			const_val = (immd8 << 24) | (immd8 << 16) | (immd8 << 8) | immd8;
			break;

		}
	}
	else
	{
		imm5 = (imm4 << 1) | ((0x00000008 & immd8) >> 8);

		const_val = (immd8 << 24) | 0x10000000;
		shft = (imm5 - 8);

		const_val = (const_val >> shft);
	}
	return (const_val);
}


void Context::IsaThumb32BranchLink()
{
	unsigned int addr = 0;

	InstThumb32Category cat = inst.getInstThumb32Info()->cat32;
	if (cat == InstThumb32CategoryBranch)
	{
		addr = (inst.getThumb32Bytes()->branch_link.sign << 24)
				| ((!(inst.getThumb32Bytes()->branch.j1 ^ inst.getThumb32Bytes()->branch_link.sign)) << 23)
				| ((!(inst.getThumb32Bytes()->branch.j2 ^ inst.getThumb32Bytes()->branch_link.sign)) << 22)
				| (inst.getThumb32Bytes()->branch_link.immd10 << 12)
				| (inst.getThumb32Bytes()->branch_link.immd11 << 1);
		addr = misc::SignExtend32(addr,25);
		emu->isa_debug << misc::fmt("  Inst_32 addr = 0x%x, Branch offset = 0x%x\n", inst.getAddress(), addr);
		addr = (inst.getAddress() + 4) + addr;
	}
	else if (cat == InstThumb32CategoryBranchLx)
	{
		addr = (inst.getThumb32Bytes()->branch_link.sign << 24)
				| ((!(inst.getThumb32Bytes()->branch.j1 ^ inst.getThumb32Bytes()->branch_link.sign)) << 23)
				| ((!(inst.getThumb32Bytes()->branch.j2 ^ inst.getThumb32Bytes()->branch_link.sign)) << 22)
				| (inst.getThumb32Bytes()->branch_link.immd10 << 12)
				| ((inst.getThumb32Bytes()->branch_link.immd11 & 0xfffffffe) << 1);
		addr = misc::SignExtend32(addr,25);

		if((inst.getAddress() + 2) % 4)
			addr = (inst.getAddress() + 4) + addr;
		else
			addr = (inst.getAddress() + 2) + addr;
	}
	else if (cat == InstThumb32CategoryBranchCond)
	{
		addr = (inst.getThumb32Bytes()->branch.sign << 20)
				| (((inst.getThumb32Bytes()->branch.j2)) << 19)
				| (((inst.getThumb32Bytes()->branch.j1)) << 18)
				| (inst.getThumb32Bytes()->branch.immd6 << 12)
				| (inst.getThumb32Bytes()->branch.immd11 << 1);
		addr = misc::SignExtend32(addr,21);
		addr = (inst.getAddress() + 2) + addr;
	}
	else
		throw misc::Panic(misc::fmt("%d: addr fmt not recognized", cat));

	// FIXME : Changed from +4 to +2
	emu->isa_debug << misc::fmt("  Branch addr_32 = 0x%x, Current pc <= 0x%x\n", addr, regs.getPC());
	IsaRegStore(14, regs.getPC() - 1);
	regs.setPC(addr + 2);
	emu->isa_debug << misc::fmt("  Branch addr_32 = 0x%x, Written pc <= 0x%x\n", addr, regs.getPC());
}


void Context::IsaThumb32Branch()
{
	unsigned int addr = 0;
	InstThumb32Category cat = inst.getInstThumb32Info()->cat32;
	unsigned int cond;
	if (cat == InstThumb32CategoryBranch)
	{
		addr = (inst.getThumb32Bytes()->branch_link.sign << 24)
				| ((!(inst.getThumb32Bytes()->branch.j1 ^ inst.getThumb32Bytes()->branch_link.sign)) << 23)
				| ((!(inst.getThumb32Bytes()->branch.j2 ^ inst.getThumb32Bytes()->branch_link.sign)) << 22)
				| (inst.getThumb32Bytes()->branch_link.immd10 << 12)
				| (inst.getThumb32Bytes()->branch_link.immd11 << 1);
		addr = misc::SignExtend32(addr,25);

		addr = (regs.getPC() - 4) + (addr);
		regs.setPC(addr + 4);
		emu->isa_debug << misc::fmt("  Branch addr = 0x%x, pc <= 0x%x\n", regs.getPC() - 2, regs.getPC());
	}
	else if (cat == InstThumb32CategoryBranchCond)
	{
		addr = (inst.getThumb32Bytes()->branch.sign << 20)
						| (((inst.getThumb32Bytes()->branch.j2)) << 19)
						| (((inst.getThumb32Bytes()->branch.j1)) << 18)
						| (inst.getThumb32Bytes()->branch.immd6 << 12)
						| (inst.getThumb32Bytes()->branch.immd11 << 1);
		addr = misc::SignExtend32(addr,21);

		cond = inst.getThumb32Bytes()->branch.cond;
		if(IsaThumbCheckCond(cond))
		{
			addr = (regs.getPC() - 4) + (addr);
			regs.setPC(addr + 4);
			emu->isa_debug << misc::fmt("  Branch addr = 0x%x, pc <= 0x%x\n", regs.getPC() - 2, regs.getPC());
		}
	}
	else
	{
		throw misc::Panic(misc::fmt("%d: addr fmt not recognized", cat));
	}

	// FIXME : Changed from +4 to +2
}


unsigned int Context::IsaThumbCheckCond(unsigned int cond)
{
	unsigned int ret_val;
	switch (cond)
	{
	case (AsmConditionCodesEQ):

		ret_val = (regs.getCPSR().z) ? 1 : 0;
		emu->isa_debug << misc::fmt("  Cond = EQ\n");
		break;

	case (AsmConditionCodesNE):

		ret_val = (!(regs.getCPSR().z)) ? 1 : 0;
		emu->isa_debug << misc::fmt("  Cond = NE\n");
		break;

	case (AsmConditionCodesCS):

		ret_val = (regs.getCPSR().C) ? 1 : 0;
		emu->isa_debug << misc::fmt("  Cond = CS\n");
		break;

	case (AsmConditionCodesCC):

		ret_val = (!(regs.getCPSR().C)) ? 1 : 0;
		emu->isa_debug << misc::fmt("  Cond = CC\n");
		break;

	case (AsmConditionCodesMI):

		ret_val = (regs.getCPSR().n) ? 1 : 0;
		emu->isa_debug << misc::fmt("  Cond = MI\n");
		break;

	case (AsmConditionCodesPL):

		ret_val = (!(regs.getCPSR().n)) ? 1 : 0;
		emu->isa_debug << misc::fmt("  Cond = PL\n");
		break;

	case (AsmConditionCodesVS):

		ret_val = (regs.getCPSR().v) ? 1 : 0;
		emu->isa_debug << misc::fmt("  Cond = VS\n");
		break;

	case (AsmConditionCodesVC):

		ret_val = (!(regs.getCPSR().v)) ? 1 : 0;
		emu->isa_debug << misc::fmt("  Cond = VC\n");
		break;

	case (AsmConditionCodesHI):

		ret_val = (!(regs.getCPSR().z) && (regs.getCPSR().C)) ? 1 : 0;
		emu->isa_debug << misc::fmt("  Cond = HI\n");
		break;

	case (AsmConditionCodesLS):

		ret_val = ((regs.getCPSR().z) | !(regs.getCPSR().C)) ? 1 : 0;
		emu->isa_debug << misc::fmt("  Cond = LS\n");
		break;

	case (AsmConditionCodesGE):

		ret_val = (((regs.getCPSR().n) & (regs.getCPSR().v))
			| (!(regs.getCPSR().n) & !(regs.getCPSR().v))) ? 1 : 0;
		emu->isa_debug << misc::fmt("  Cond = GE\n");
		break;

	case (AsmConditionCodesLT):

		ret_val = (((regs.getCPSR().n) & !(regs.getCPSR().v))
			| (!(regs.getCPSR().n) && (regs.getCPSR().v))) ? 1 : 0;
		emu->isa_debug << misc::fmt("  Cond = LT\n");
		break;

	case (AsmConditionCodesGT):

		ret_val = (((regs.getCPSR().n) & (regs.getCPSR().v) & !(regs.getCPSR().z))
			| (!(regs.getCPSR().n) & !(regs.getCPSR().v)
			& !(regs.getCPSR().z))) ? 1 : 0;
		emu->isa_debug << misc::fmt("  Cond = GT\n");
		break;

	case (AsmConditionCodesLE):

		ret_val = (((regs.getCPSR().z) | (!(regs.getCPSR().n) && (regs.getCPSR().v))
			| ((regs.getCPSR().n) && !(regs.getCPSR().v)))) ? 1 : 0;
		emu->isa_debug << misc::fmt("  Cond = LE\n");
		break;

	case (AsmConditionCodesAL):

		ret_val = 1;
		break;
	}

	return (ret_val);
}


void Context::IsaThumbIteq()
{
	unsigned int mask = inst.getThumb16Bytes()->if_eq_ins.mask;
	if(mask == 8)
	{
		iteq_inst_num = 1;
		iteq_block_flag = ContextThumbIteqEnabled;
	}
}


unsigned int Context::IsaThumbImmdExtend(unsigned int immd)
{
	unsigned int shift = (immd & 0x00000f80) >> 7;
	emu->isa_debug << misc::fmt("  Shift = 0x%x\n", shift);
	if (((immd & 0x00000c00) >> 10) == 0)
	{
		switch(((immd & 0x00000300) >> 8))
		{
		case 0:

			immd = immd & 0x000000ff;
			break;

		case 1:

			immd = immd & 0x000000ff;
			immd = (immd << 16) | immd;
			break;

		case 2:

			immd = immd & 0x000000ff;
			immd = (immd << 24) | (immd << 8);
			break;

		case 3:

			immd = immd & 0x000000ff;
			immd = (immd << 24) | (immd << 16) | (immd << 8) | (immd);
			break;
		}
	}
	else
	{
		immd = (1 << 7) | (immd & 0x0000007f);
		immd = IsaRotr(immd, shift);
		emu->isa_debug << misc::fmt("  Rotated immd = 0x%x\n", immd);
	}
	return (immd);
}

} // namespace ARM
