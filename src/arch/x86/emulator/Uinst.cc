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

#include "Uinst.h"


namespace x86
{

const int Uinst::MaxIDeps;
const int Uinst::MaxODeps;
const int Uinst::MaxDeps;


const misc::StringMap Uinst::dep_map
{
	{ "-", DepNone },

	{ "eax", DepEax },
	{ "ecx", DepEcx },
	{ "edx", DepEdx },
	{ "ebx", DepEbx },
	{ "esp", DepEsp },
	{ "ebp", DepEbp },
	{ "esi", DepEsi },
	{ "edi", DepEdi },

	{ "es", DepEs },
	{ "cs", DepCs },
	{ "ss", DepSs },
	{ "ds", DepDs },
	{ "fs", DepFs },
	{ "gs", DepGs },

	{ "zps", DepZps },
	{ "of", DepOf },
	{ "cf", DepCf },
	{ "df", DepDf },

	{ "aux", DepAux },
	{ "aux2", DepAux2 },
	{ "ea", DepEa },
	{ "data", DepData },

	{ "st0", DepSt0 },
	{ "st1", DepSt1 },
	{ "st2", DepSt2 },
	{ "st3", DepSt3 },
	{ "st4", DepSt4 },
	{ "st5", DepSt5 },
	{ "st6", DepSt6 },
	{ "st7", DepSt7 },
	{ "fpst", DepFpst },
	{ "fpcw", DepFpcw },
	{ "fpaux", DepFpaux },

	{ "xmm0", DepXmm0 },
	{ "xmm1", DepXmm1 },
	{ "xmm2", DepXmm2 },
	{ "xmm3", DepXmm3 },
	{ "xmm4", DepXmm4 },
	{ "xmm5", DepXmm5 },
	{ "xmm6", DepXmm6 },
	{ "xmm7", DepXmm7 },
	{ "xmm_data", DepXmmData },

	{ "rm8*", DepRm8 },
	{ "rm16*", DepRm16 },
	{ "rm32*", DepRm32 },

	{ "ir8*", DepIr8 },
	{ "ir16*", DepIr16 },
	{ "ir32*", DepIr32 },

	{ "r8*", DepR8 },
	{ "r16*", DepR16 },
	{ "r32*", DepR32 },

	{ "sreg*", DepSreg },

	{ "mem8*", DepMem8 },
	{ "mem16*", DepMem16 },
	{ "mem32*", DepMem32 },
	{ "mem64*", DepMem64 },
	{ "mem80*", DepMem80 },
	{ "mem128*", DepMem128 },

	{ "easeg*", DepEaseg },
	{ "eabas*", DepEabas },
	{ "eaidx*", DepEaidx },

	{ "sti*", DepSti },

	{ "xmmm32*", DepXmmm32 },
	{ "xmmm64*", DepXmmm64 },
	{ "xmmm128*", DepXmmm128 },

	{ "xmm*", DepXmm }
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


bool Uinst::addIDep(Dep dep)
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


bool Uinst::addODep(Dep dep)
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
		Dep dep = odep[i];
		if (!dep)
			continue;
		dep_count++;
		os << comma << getDependencyName(dep);
		comma = ",";
	}
	if (!dep_count)
		os << '-';

	// Separator
	os << '/';

	/* Input operands */
	comma = "";
	dep_count = 0;
	for (int i = 0; i < MaxIDeps; i++)
	{
		Dep dep = idep[i];
		if (!dep)
			continue;
		dep_count++;
		os << comma << getDependencyName(dep);
		comma = ",";
	}
	if (!dep_count)
		os << '-';

	// Memory address
	if (size)
		os << misc::fmt(" [0x%x,%d]", address, size);
}


}  // namespace x86

