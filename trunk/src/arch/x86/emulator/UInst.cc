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

const int Uinst::MaxIDeps;
const int Uinst::MaxODeps;
const int Uinst::MaxDeps;


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
Uinst::Info Uinst::info[Uinst::OpcodeCount] =
{
	{ "nop", 0 },

	{ "move", FlagInt },
	{ "add", FlagInt },
	{ "sub", FlagInt },
	{ "mult", FlagInt },
	{ "div", FlagInt },
	{ "effaddr", FlagInt },

	{ "and", FlagLogic },
	{ "or", FlagLogic },
	{ "xor", FlagLogic },
	{ "not", FlagLogic },
	{ "shift", FlagLogic },
	{ "sign", FlagLogic },

	{ "fmove", FlagFp },
	{ "fsign", FlagFp },
	{ "fround", FlagFp },

	{ "fadd", FlagFp },
	{ "fsub", FlagFp },
	{ "fcomp", FlagFp },
	{ "fmult", FlagFp },
	{ "fdiv", FlagFp },

	{ "fexp", FlagFp },
	{ "flog", FlagFp },
	{ "fsin", FlagFp },
	{ "fcos", FlagFp },
	{ "fsincos", FlagFp },
	{ "ftan", FlagFp },
	{ "fatan", FlagFp },
	{ "fsqrt", FlagFp },

	{ "fpush", FlagFp },
	{ "fpop", FlagFp },

	{ "x-and", FlagXmm },
	{ "x-or", FlagXmm },
	{ "x-xor", FlagXmm },
	{ "x-not", FlagXmm },
	{ "x-nand", FlagXmm },
	{ "x-shift", FlagXmm },
	{ "x-sign", FlagXmm },

	{ "x-add", FlagXmm },
	{ "x-sub", FlagXmm },
	{ "x-comp", FlagXmm },
	{ "x-mult", FlagXmm },
	{ "x-div", FlagXmm },

	{ "x-fadd", FlagXmm },
	{ "x-fsub", FlagXmm },
	{ "x-fcomp", FlagXmm },
	{ "x-fmult", FlagXmm },
	{ "x-fdiv", FlagXmm },

	{ "x-fsqrt", FlagXmm },

	{ "x-move", FlagXmm },
	{ "x-shuf", FlagXmm },
	{ "x-conv", FlagXmm },

	{ "load", FlagMem },
	{ "store", FlagMem },
	{ "prefetch", FlagMem },

	{ "call", FlagCtrl | FlagUncond },
	{ "ret", FlagCtrl | FlagUncond },
	{ "jump", FlagCtrl | FlagUncond },
	{ "branch", FlagCtrl | FlagCond },
	{ "ibranch", FlagCtrl | FlagCond },

	{ "syscall", 0 }
};


bool Uinst::addIDep(UInstDep dep)
{
	// Find free index
	int index;
	for (index = 0; index < MaxIDeps; index++)
		if (!idep[index])
			break;
	
	// Return false if no room for new dependency
	if (index == MaxIDeps)
		return false;
	
	// Set new dependence
	idep[index] = dep;
	return true;
}


bool Uinst::addODep(UInstDep dep)
{
	// Find free index
	int index;
	for (index = 0; index < MaxODeps; index++)
		if (!odep[index])
			break;
	
	// Return false if no room for new dependency
	if (index == MaxODeps)
		return false;
	
	// Set new dependence
	odep[index] = dep;
	return true;
}


void Uinst::Dump(std::ostream &os) const
{
	// Name
	os << info[opcode].name << ' ';

	// Outputs
	std::string comma = "";
	int dep_count = 0;
	for (int i = 0; i < MaxODeps; i++)
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
	for (int i = 0; i < MaxODeps; i++)
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

