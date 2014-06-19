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

#include <cstdarg>
#include <cstring>

#include <lib/cpp/Misc.h>

#include "Context.h"
#include "Emu.h"


namespace MIPS
{

Context::ExecuteInstFn Context::execute_inst_fn[InstOpcodeCount] =
{
		nullptr  // For InstOpcodeNone
#define DEFINST(_name, _fmt_str, _op0, _op1, _op2, _op3) \
			, &Context::ExecuteInst_##_name
#include <arch/mips/asm/asm.dat>
#undef DEFINST
};


}
