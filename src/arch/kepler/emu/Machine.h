/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_KEPLER_EMU_MACHINE_H
#define ARCH_KEPLER_EMU_MACHINE_H

#include "Thread.h"
#include "../asm/Inst.h"

namespace Kepler
{
/* Declarations of function prototypes implementing Kepler ISA */
#define DEFINST(_name, _fmt_str, ...) \
		extern void kpl_isa_##_name##_impl(Thread *thread, \
				Inst *inst);
#include <arch/kepler/asm/Inst.def>
#undef DEFINST

}//namespace
#endif