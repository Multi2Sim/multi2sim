/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include <arch/x86/emu/Emu.h>
#include "Driver.h"

namespace Driver
{

x86::Emu *Common::x86_emu;
x86::Cpu *Common::x86_cpu;

Common::Common()
{
	// Get the only instance of X86 functional emulator
	x86_emu = x86::Emu::getInstance();

	// Get the only instance of X86 timing simulator
	// x86_cpu = x86::CPU::getInstance();
}

}  // namespace Driver