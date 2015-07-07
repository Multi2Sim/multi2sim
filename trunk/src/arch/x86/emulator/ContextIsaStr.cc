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

#include "Context.h"


namespace x86
{

// Macros defined to prevent accidental use of functions that cause unsafe
// execution in speculative mode.
#undef assert
#define memory __COMPILATION_ERROR__
#define fatal __COMPILATION_ERROR__
#define panic __COMPILATION_ERROR__
#define warning __COMPILATION_ERROR__
#define assert __COMPILATION_ERROR__


void Context::StartRepInst()
{
	if (last_eip == current_eip)
	{
		str_op_count++;
	}
	else
	{
		str_op_count = 0;
		str_op_esi = regs.getEsi();
		str_op_edi = regs.getEdi();
		str_op_dir = regs.getEflags() & (1 << 10) ? -1 : 1;
	}
}


#define OP_REP_IMPL(X, SIZE) \
	void Context::ExecuteInst_rep_##X() \
	{ \
		StartRepInst(); \
		\
		if (regs.getEcx()) \
		{ \
			ExecuteStringInst_##X(); \
			regs.decEcx(); \
			regs.decEip(inst.getSize()); \
		} \
		\
		newUinst_##X( \
			str_op_esi + str_op_count * (SIZE) * str_op_dir, \
			str_op_edi + str_op_count * (SIZE) * str_op_dir); \
		newUinst(Uinst::OpcodeSub, \
				Uinst::DepEcx, \
				0, \
				0, \
				Uinst::DepEcx, \
				0, \
				0, \
				0); \
		newUinst(Uinst::OpcodeIbranch, \
				Uinst::DepEcx, \
				0, \
				0, \
				0, \
				0, \
				0, \
				0); \
	}


#define OP_REPZ_IMPL(X, SIZE) \
	void Context::ExecuteInst_repz_##X() \
	{ \
		StartRepInst(); \
		\
		if (regs.getEcx()) \
		{ \
			ExecuteStringInst_##X(); \
			regs.decEcx(); \
			if (regs.getFlag(Instruction::FlagZF)) \
				regs.decEip(inst.getSize()); \
		} \
		\
		newUinst_##X( \
			str_op_esi + str_op_count * (SIZE) * str_op_dir, \
			str_op_edi + str_op_count * (SIZE) * str_op_dir); \
		newUinst(Uinst::OpcodeSub, \
				Uinst::DepEcx, \
				0, \
				0, \
				Uinst::DepEcx, \
				0, \
				0, \
				0); \
		newUinst(Uinst::OpcodeIbranch, \
				Uinst::DepEcx, \
				Uinst::DepZps, \
				0, \
				0, \
				0, \
				0, \
				0); \
	}


#define OP_REPNZ_IMPL(X, SIZE) \
	void Context::ExecuteInst_repnz_##X() \
	{ \
		StartRepInst(); \
		\
		if (regs.getEcx()) \
		{ \
			ExecuteStringInst_##X(); \
			regs.decEcx(); \
			if (!regs.getFlag(Instruction::FlagZF)) \
				regs.decEip(inst.getSize()); \
		} \
		\
		newUinst_##X( \
			str_op_esi + str_op_count * (SIZE) * str_op_dir, \
			str_op_edi + str_op_count * (SIZE) * str_op_dir); \
		newUinst(Uinst::OpcodeSub, \
				Uinst::DepEcx, \
				0, \
				0, \
				Uinst::DepEcx, \
				0, \
				0, \
				0); \
		newUinst(Uinst::OpcodeIbranch, \
				Uinst::DepEcx, \
				Uinst::DepZps, \
				0, \
				0, \
				0, \
				0, \
				0); \
	}




//
// CMPSB
//

void Context::newUinst_cmpsb(unsigned int esi, unsigned int edi)
{
	newMemoryUinst(Uinst::OpcodeLoad,
			esi,
			1,
			Uinst::DepEsi,
			0,
			0,
			Uinst::DepAux,
			0,
			0,
			0);
	newMemoryUinst(Uinst::OpcodeLoad,
			edi,
			1,
			Uinst::DepEdi,
			0,
			0,
			Uinst::DepAux2,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeSub,
			Uinst::DepAux,
			Uinst::DepAux2,
			0,
			Uinst::DepZps,
			Uinst::DepOf,
			Uinst::DepCf,
			0);

	newUinst(Uinst::OpcodeAdd,
			Uinst::DepEsi,
			Uinst::DepDf,
			0,
			Uinst::DepEsi,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeAdd,
			Uinst::DepEdi,
			Uinst::DepDf,
			0,
			Uinst::DepEdi,
			0,
			0,
			0);
}


void Context::ExecuteStringInst_cmpsb()
{
	unsigned char op1;
	unsigned char op2;
	unsigned long flags;

	MemoryRead(regs.getEsi(), 1, &op1);
	MemoryRead(regs.getEdi(), 1, &op2);
	flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__
	asm volatile (
		"push %1\n\t"
		"popf\n\t"
		"mov %2, %%al\n\t"
		"cmp %3, %%al\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "g" (flags), "m" (op1), "m" (op2)
		: "al"
	);
	__X86_CONTEXT_RESTORE_FLAGS__

	regs.setEflags(flags);
	regs.incEsi(regs.getFlag(Instruction::FlagDF) ? -1 : 1);
	regs.incEdi(regs.getFlag(Instruction::FlagDF) ? -1 : 1);
}


void Context::ExecuteInst_cmpsb()
{
	newUinst_cmpsb(regs.getEsi(), regs.getEdi());
	ExecuteStringInst_cmpsb();
}




//
// CMPSD
//

void Context::newUinst_cmpsd(unsigned int esi, unsigned int edi)
{
	newMemoryUinst(Uinst::OpcodeLoad,
			esi,
			4,
			Uinst::DepEsi,
			0,
			0,
			Uinst::DepAux,
			0,
			0,
			0);
	newMemoryUinst(Uinst::OpcodeLoad,
			edi,
			4,
			Uinst::DepEdi,
			0,
			0,
			Uinst::DepAux2,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeSub,
			Uinst::DepAux,
			Uinst::DepAux2,
			0,
			Uinst::DepZps,
			Uinst::DepOf,
			Uinst::DepCf,
			0);

	newUinst(Uinst::OpcodeAdd,
			Uinst::DepEsi,
			Uinst::DepDf,
			0,
			Uinst::DepEsi,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeAdd,
			Uinst::DepEdi,
			Uinst::DepDf,
			0,
			Uinst::DepEdi,
			0,
			0,
			0);
}


void Context::ExecuteStringInst_cmpsd()
{
	unsigned int op1;
	unsigned int op2;
	unsigned long flags;

	MemoryRead(regs.getEdi(), 4, &op1);
	MemoryRead(regs.getEsi(), 4, &op2);
	flags = regs.getEflags();

	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile (
		"push %1\n\t"
		"popf\n\t"
		"mov %2, %%eax\n\t"
		"cmp %3, %%eax\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "g" (flags), "m" (op1), "m" (op2)
		: "eax"
	);
	__X86_CONTEXT_RESTORE_FLAGS__ \

	regs.setEflags(flags);
	regs.incEsi(regs.getFlag(Instruction::FlagDF) ? -4 : 4);
	regs.incEdi(regs.getFlag(Instruction::FlagDF) ? -4 : 4);
}


void Context::ExecuteInst_cmpsd()
{
	newUinst_cmpsd(regs.getEsi(), regs.getEdi());
	ExecuteStringInst_cmpsd();
}




//
// INSB
//

void Context::ExecuteStringInst_insb()
{
	// Not implemented
}


void Context::newUinst_insb(unsigned int esi, unsigned int edi)
{
	// Not implemented
}


void Context::ExecuteInst_insb()
{
	newUinst_insb(regs.getEsi(), regs.getEdi());
	ExecuteStringInst_insb();
	throw misc::Panic("Unimplemented instruction");
}




//
// INSD
//

void Context::ExecuteStringInst_insd()
{
	// Not implemented
}


void Context::newUinst_insd(unsigned int esi, unsigned int edi)
{
	// Not implemented
}


void Context::ExecuteInst_insd()
{
	newUinst_insd(regs.getEsi(), regs.getEdi());
	ExecuteStringInst_insd();
	throw misc::Panic("Unimplemented instruction");
}




//
// LODSB
//

void Context::ExecuteStringInst_lodsb()
{
	// Not implemented
}


void Context::newUinst_lodsb(unsigned int esi, unsigned int edi)
{
	// Not implemented
}


void Context::ExecuteInst_lodsb()
{
	newUinst_lodsb(regs.getEsi(), regs.getEdi());
	ExecuteStringInst_lodsb();
	throw misc::Panic("Unimplemented instruction");
}




//
// LODSD
//

void Context::ExecuteStringInst_lodsd()
{
	// Not implemented
}


void Context::newUinst_lodsd(unsigned int esi, unsigned int edi)
{
	// Not implemented
}


void Context::ExecuteInst_lodsd()
{
	newUinst_lodsd(regs.getEsi(), regs.getEdi());
	ExecuteStringInst_lodsd();
	throw misc::Panic("Unimplemented instruction");
}




//
// MOVSB
//

void Context::ExecuteStringInst_movsb()
{
	unsigned char m8;

	MemoryRead(regs.getEsi(), 1, &m8);
	MemoryWrite(regs.getEdi(), 1, &m8);

	regs.incEdi(regs.getFlag(Instruction::FlagDF) ? -1 : 1);
	regs.incEsi(regs.getFlag(Instruction::FlagDF) ? -1 : 1);
}


void Context::newUinst_movsb(unsigned int esi, unsigned int edi)
{
	newMemoryUinst(Uinst::OpcodeLoad,
			esi,
			1,
			Uinst::DepEsi,
			0,
			0,
			Uinst::DepAux,
			0,
			0,
			0);
	newMemoryUinst(Uinst::OpcodeStore,
			edi,
			1,
			Uinst::DepEdi,
			Uinst::DepAux,
			0,
			0,
			0,
			0,
			0);

	newUinst(Uinst::OpcodeAdd,
			Uinst::DepEdi,
			Uinst::DepDf,
			0,
			Uinst::DepEdi,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeAdd,
			Uinst::DepEsi,
			Uinst::DepDf,
			0,
			Uinst::DepEsi,
			0,
			0,
			0);
}


void Context::ExecuteInst_movsb()
{
	newUinst_movsb(regs.getEsi(), regs.getEdi());
	ExecuteStringInst_movsb();
}




//
// MOVSW
//

void Context::ExecuteStringInst_movsw()
{
	unsigned short m16;

	MemoryRead(regs.getEsi(), 2, &m16);
	MemoryWrite(regs.getEdi(), 2, &m16);

	regs.incEdi(regs.getFlag(Instruction::FlagDF) ? -2 : 2);
	regs.incEsi(regs.getFlag(Instruction::FlagDF) ? -2 : 2);

}


void Context::newUinst_movsw(unsigned int esi, unsigned int edi)
{
	newMemoryUinst(Uinst::OpcodeLoad,
			esi,
			1,
			Uinst::DepEsi,
			0,
			0,
			Uinst::DepAux,
			0,
			0,
			0);
	newMemoryUinst(Uinst::OpcodeStore,
			edi,
			1,
			Uinst::DepEdi,
			Uinst::DepAux,
			0,
			0,
			0,
			0,
			0);

	newUinst(Uinst::OpcodeAdd,
			Uinst::DepEdi,
			Uinst::DepDf,
			0,
			Uinst::DepEdi,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeAdd,
			Uinst::DepEsi,
			Uinst::DepDf,
			0,
			Uinst::DepEsi,
			0,
			0,
			0);
}


void Context::ExecuteInst_movsw()
{
	newUinst_movsw(regs.getEsi(), regs.getEdi());
	ExecuteStringInst_movsw();
}



//
// MOVSD
//

void Context::ExecuteStringInst_movsd()
{
	unsigned int m32;

	MemoryRead(regs.getEsi(), 4, &m32);
	MemoryWrite(regs.getEdi(), 4, &m32);

	regs.incEdi(regs.getFlag(Instruction::FlagDF) ? -4 : 4);
	regs.incEsi(regs.getFlag(Instruction::FlagDF) ? -4 : 4);

}


void Context::newUinst_movsd(unsigned int esi, unsigned int edi)
{
	newMemoryUinst(Uinst::OpcodeLoad,
			esi,
			4,
			Uinst::DepEsi,
			0,
			0,
			Uinst::DepAux,
			0,
			0,
			0);
	newMemoryUinst(Uinst::OpcodeStore,
			edi,
			4,
			Uinst::DepEdi,
			Uinst::DepAux,
			0,
			0,
			0,
			0,
			0);

	newUinst(Uinst::OpcodeAdd,
			Uinst::DepEdi,
			Uinst::DepDf,
			0,
			Uinst::DepEdi,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeAdd,
			Uinst::DepEsi,
			Uinst::DepDf,
			0,
			Uinst::DepEsi,
			0,
			0,
			0);
}


void Context::ExecuteInst_movsd()
{
	newUinst_movsd(regs.getEsi(), regs.getEdi());
	ExecuteStringInst_movsd();
}




//
// OUTSB
//

void Context::ExecuteStringInst_outsb()
{
	// Not implemented
}


void Context::newUinst_outsb(unsigned int esi, unsigned int edi)
{
	// Not implemented
}


void Context::ExecuteInst_outsb()
{
	newUinst_outsb(regs.getEsi(), regs.getEdi());
	ExecuteStringInst_outsb();
	throw misc::Panic("Unimplemented instruction");
}




//
// OUTSD
//

void Context::ExecuteStringInst_outsd()
{
	// Not implemented
}


void Context::newUinst_outsd(unsigned int esi, unsigned int edi)
{
	// Not implemented
}


void Context::ExecuteInst_outsd()
{
	newUinst_outsd(regs.getEsi(), regs.getEdi());
	ExecuteStringInst_outsd();
	throw misc::Panic("Unimplemented instruction");
}




//
// SCAS
//

void Context::ExecuteStringInst_scasb()
{
	unsigned char al = regs.Read(Instruction::RegAl);
	unsigned char m8;
	unsigned long flags = regs.getEflags();

	MemoryRead(regs.getEdi(), 1, &m8);

	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %1, %%al\n\t"
		"cmp %2, %%al\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "m" (al), "m" (m8), "g" (flags)
		: "al"
	);
	__X86_CONTEXT_RESTORE_FLAGS__ \

	regs.setEflags(flags);
	regs.incEdi(regs.getFlag(Instruction::FlagDF) ? -1 : 1);

}


void Context::newUinst_scasb(unsigned int esi, unsigned int edi)
{
	newMemoryUinst(Uinst::OpcodeLoad,
			edi,
			1,
			Uinst::DepEdi,
			0,
			0,
			Uinst::DepAux,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeSub,
			Uinst::DepAux,
			Uinst::DepEax,
			0,
			Uinst::DepZps,
			Uinst::DepOf,
			Uinst::DepCf,
			0);
	newUinst(Uinst::OpcodeAdd,
			Uinst::DepEdi,
			Uinst::DepDf,
			0,
			Uinst::DepEdi,
			0,
			0,
			0);
}


void Context::ExecuteInst_scasb()
{
	newUinst_scasb(regs.getEsi(), regs.getEdi());
	ExecuteStringInst_scasb();
}



//
// SCASD
//

void Context::ExecuteStringInst_scasd()
{
	unsigned int eax = regs.Read(Instruction::RegEax);
	unsigned int m32;
	unsigned long flags = regs.getEflags();

	MemoryRead(regs.getEdi(), 4, &m32);

	__X86_CONTEXT_SAVE_FLAGS__ \
	asm volatile (
		"push %3\n\t"
		"popf\n\t"
		"mov %1, %%eax\n\t"
		"cmp %2, %%eax\n\t"
		"pushf\n\t"
		"pop %0\n\t"
		: "=g" (flags)
		: "m" (eax), "m" (m32), "g" (flags)
		: "eax"
	);
	__X86_CONTEXT_RESTORE_FLAGS__ \

	regs.setEflags(flags);
	regs.incEdi(regs.getFlag(Instruction::FlagDF) ? -4 : 4);

}


void Context::newUinst_scasd(unsigned int esi, unsigned int edi)
{
	newMemoryUinst(Uinst::OpcodeLoad,
			edi,
			4,
			Uinst::DepEdi,
			0,
			0,
			Uinst::DepAux,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeSub,
			Uinst::DepAux,
			Uinst::DepEax,
			0,
			Uinst::DepZps,
			Uinst::DepOf,
			Uinst::DepCf,
			0);
	newUinst(Uinst::OpcodeAdd,
			Uinst::DepEdi,
			Uinst::DepDf,
			0,
			Uinst::DepEdi,
			0,
			0,
			0);
}


void Context::ExecuteInst_scasd()
{
	newUinst_scasd(regs.getEsi(), regs.getEdi());
	ExecuteStringInst_scasd();
}



//
// STOSB
//

void Context::ExecuteStringInst_stosb()
{
	unsigned char m8 = regs.Read(Instruction::RegAl);
	unsigned int addr = regs.Read(Instruction::RegEdi);
	
	MemoryWrite(addr, 1, &m8);
	regs.incEdi(regs.getFlag(Instruction::FlagDF) ? -1 : 1);
}


void Context::newUinst_stosb(unsigned int esi, unsigned int edi)
{
	newMemoryUinst(Uinst::OpcodeStore,
			edi,
			1,
			Uinst::DepEdi,
			Uinst::DepEax,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeAdd,
			Uinst::DepEdi,
			Uinst::DepDf,
			0,
			Uinst::DepEdi,
			0,
			0,
			0);
}


void Context::ExecuteInst_stosb()
{
	newUinst_stosb(regs.getEsi(), regs.getEdi());
	ExecuteStringInst_stosb();
}




//
// STOSD
//

void Context::ExecuteStringInst_stosd()
{
	unsigned int m32 = regs.Read(Instruction::RegEax);
	unsigned int addr = regs.Read(Instruction::RegEdi);
	
	MemoryWrite(addr, 4, &m32);
	regs.incEdi(regs.getFlag(Instruction::FlagDF) ? -4 : 4);
}


void Context::newUinst_stosd(unsigned int esi, unsigned int edi)
{
	newMemoryUinst(Uinst::OpcodeStore,
			edi,
			4,
			Uinst::DepEdi,
			Uinst::DepEax,
			0,
			0,
			0,
			0,
			0);
	newUinst(Uinst::OpcodeAdd,
			Uinst::DepEdi,
			Uinst::DepDf,
			0,
			Uinst::DepEdi,
			0,
			0,
			0);
}


void Context::ExecuteInst_stosd()
{
	newUinst_stosd(regs.getEsi(), regs.getEdi());
	ExecuteStringInst_stosd();
}




//
// Repetition prefixes
///

OP_REP_IMPL(insb, 1)
OP_REP_IMPL(insd, 4)

OP_REP_IMPL(movsb, 1)
OP_REP_IMPL(movsd, 4)

OP_REP_IMPL(outsb, 1)
OP_REP_IMPL(outsd, 4)

OP_REP_IMPL(lodsb, 1)
OP_REP_IMPL(lodsd, 4)

OP_REP_IMPL(stosb, 1)
OP_REP_IMPL(stosd, 4)

OP_REPZ_IMPL(cmpsb, 1)
OP_REPZ_IMPL(cmpsd, 4)

OP_REPZ_IMPL(scasb, 1)
OP_REPZ_IMPL(scasd, 4)

OP_REPNZ_IMPL(cmpsb, 1)
OP_REPNZ_IMPL(cmpsd, 4)

OP_REPNZ_IMPL(scasb, 1)
OP_REPNZ_IMPL(scasd, 4)



}  // namespace x86

