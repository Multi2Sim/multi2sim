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

#include "UInst.h"


namespace x86
{

misc::StringMap uinst_dep_map
{
	{ "-", UInstDepNone },

	{ "eax", UInstDepEax },
	{ "ecx", UInstDepEcx },
	{ "edx", UInstDepEdx },
	{ "ebx", UInstDepEbx },
	{ "esp", UInstDepEsp },
	{ "ebp", UInstDepEbp },
	{ "esi", UInstDepEsi },
	{ "edi", UInstDepEdi },

	{ "es", UInstDepEs },
	{ "cs", UInstDepCs },
	{ "ss", UInstDepSs },
	{ "ds", UInstDepDs },
	{ "fs", UInstDepFs },
	{ "gs", UInstDepGs },

	{ "zps", UInstDepZps },
	{ "of", UInstDepOf },
	{ "cf", UInstDepCf },
	{ "df", UInstDepDf },

	{ "aux", UInstDepAux },
	{ "aux2", UInstDepAux2 },
	{ "ea", UInstDepEa },
	{ "data", UInstDepData },

	{ "st0", UInstDepSt0 },
	{ "st1", UInstDepSt1 },
	{ "st2", UInstDepSt2 },
	{ "st3", UInstDepSt3 },
	{ "st4", UInstDepSt4 },
	{ "st5", UInstDepSt5 },
	{ "st6", UInstDepSt6 },
	{ "st7", UInstDepSt7 },
	{ "fpst", UInstDepFpst },
	{ "fpcw", UInstDepFpcw },
	{ "fpaux", UInstDepFpaux },

	{ "xmm0", UInstDepXmm0 },
	{ "xmm1", UInstDepXmm1 },
	{ "xmm2", UInstDepXmm2 },
	{ "xmm3", UInstDepXmm3 },
	{ "xmm4", UInstDepXmm4 },
	{ "xmm5", UInstDepXmm5 },
	{ "xmm6", UInstDepXmm6 },
	{ "xmm7", UInstDepXmm7 },
	{ "xmm_data", UInstDepXmmData },

	{ "rm8*", UInstDepRm8 },
	{ "rm16*", UInstDepRm16 },
	{ "rm32*", UInstDepRm32 },

	{ "ir8*", UInstDepIr8 },
	{ "ir16*", UInstDepIr16 },
	{ "ir32*", UInstDepIr32 },

	{ "r8*", UInstDepR8 },
	{ "r16*", UInstDepR16 },
	{ "r32*", UInstDepR32 },

	{ "sreg*", UInstDepSreg },

	{ "mem8*", UInstDepMem8 },
	{ "mem16*", UInstDepMem16 },
	{ "mem32*", UInstDepMem32 },
	{ "mem64*", UInstDepMem64 },
	{ "mem80*", UInstDepMem80 },
	{ "mem128*", UInstDepMem128 },

	{ "easeg*", UInstDepEaseg },
	{ "eabas*", UInstDepEabas },
	{ "eaidx*", UInstDepEaidx },

	{ "sti*", UInstDepSti },

	{ "xmmm32*", UInstDepXmmm32 },
	{ "xmmm64*", UInstDepXmmm64 },
	{ "xmmm128*", UInstDepXmmm128 },

	{ "xmm*", UInstDepXmm }
};


// Information related with a micro-instruction opcode
UInstInfo UInst::info[UInstOpcodeCount] =
{
	{ "nop", 0 },

	{ "move", UInstFlagInt },
	{ "add", UInstFlagInt },
	{ "sub", UInstFlagInt },
	{ "mult", UInstFlagInt },
	{ "div", UInstFlagInt },
	{ "effaddr", UInstFlagInt },

	{ "and", UInstFlagLogic },
	{ "or", UInstFlagLogic },
	{ "xor", UInstFlagLogic },
	{ "not", UInstFlagLogic },
	{ "shift", UInstFlagLogic },
	{ "sign", UInstFlagLogic },

	{ "fmove", UInstFlagFp },
	{ "fsign", UInstFlagFp },
	{ "fround", UInstFlagFp },

	{ "fadd", UInstFlagFp },
	{ "fsub", UInstFlagFp },
	{ "fcomp", UInstFlagFp },
	{ "fmult", UInstFlagFp },
	{ "fdiv", UInstFlagFp },

	{ "fexp", UInstFlagFp },
	{ "flog", UInstFlagFp },
	{ "fsin", UInstFlagFp },
	{ "fcos", UInstFlagFp },
	{ "fsincos", UInstFlagFp },
	{ "ftan", UInstFlagFp },
	{ "fatan", UInstFlagFp },
	{ "fsqrt", UInstFlagFp },

	{ "fpush", UInstFlagFp },
	{ "fpop", UInstFlagFp },

	{ "x-and", UInstFlagXmm },
	{ "x-or", UInstFlagXmm },
	{ "x-xor", UInstFlagXmm },
	{ "x-not", UInstFlagXmm },
	{ "x-nand", UInstFlagXmm },
	{ "x-shift", UInstFlagXmm },
	{ "x-sign", UInstFlagXmm },

	{ "x-add", UInstFlagXmm },
	{ "x-sub", UInstFlagXmm },
	{ "x-comp", UInstFlagXmm },
	{ "x-mult", UInstFlagXmm },
	{ "x-div", UInstFlagXmm },

	{ "x-fadd", UInstFlagXmm },
	{ "x-fsub", UInstFlagXmm },
	{ "x-fcomp", UInstFlagXmm },
	{ "x-fmult", UInstFlagXmm },
	{ "x-fdiv", UInstFlagXmm },

	{ "x-fsqrt", UInstFlagXmm },

	{ "x-move", UInstFlagXmm },
	{ "x-shuf", UInstFlagXmm },
	{ "x-conv", UInstFlagXmm },

	{ "load", UInstFlagMem },
	{ "store", UInstFlagMem },
	{ "prefetch", UInstFlagMem },

	{ "call", UInstFlagCtrl | UInstFlagUncond },
	{ "ret", UInstFlagCtrl | UInstFlagUncond },
	{ "jump", UInstFlagCtrl | UInstFlagUncond },
	{ "branch", UInstFlagCtrl | UInstFlagCond },
	{ "ibranch", UInstFlagCtrl | UInstFlagCond },

	{ "syscall", 0 }
};


UInst::UInst(UInstOpcode opcode)
{
	// Pointers
	idep = dep;
	odep = dep + UInstMaxIDeps;

	// Initialize
	this->opcode = opcode;
	size = 0;
	address = 0;

	// Dependences
	for (int i = 0; i < UInstMaxDeps; i++)
		dep[i] = UInstDepNone;
}


bool UInst::addIDep(UInstDep dep)
{
	// Find free index
	int index;
	for (index = 0; index < UInstMaxIDeps; index++)
		if (!idep[index])
			break;
	
	// Return false if no room for new dependency
	if (index == UInstMaxIDeps)
		return false;
	
	// Set new dependence
	idep[index] = dep;
	return true;
}

bool UInst::addODep(UInstDep dep)
{
	// Find free index
	int index;
	for (index = 0; index < UInstMaxODeps; index++)
		if (!odep[index])
			break;
	
	// Return false if no room for new dependency
	if (index == UInstMaxODeps)
		return false;
	
	// Set new dependence
	odep[index] = dep;
	return true;
}


void UInst::Dump(std::ostream &os) const
{
	// Name
	os << info[opcode].name << ' ';

	// Outputs
	std::string comma = "";
	int dep_count = 0;
	for (int i = 0; i < UInstMaxODeps; i++)
	{
		UInstDep dep = odep[i];
		if (!dep)
			continue;
		dep_count++;
		os << comma << getDepName(dep);
		comma = ",";
	}
	if (!dep_count)
		os << '-';

	// Separator
	os << '/';

	/* Input operands */
	comma = "";
	dep_count = 0;
	for (int i = 0; i < UInstMaxODeps; i++)
	{
		UInstDep dep = idep[i];
		if (!dep)
			continue;
		dep_count++;
		os << comma << getDepName(dep);
		comma = ",";
	}
	if (!dep_count)
		os << '-';

	// Memory address
	if (size)
		os << misc::fmt(" [0x%x,%d]", address, size);
}


}  // namespace x86

