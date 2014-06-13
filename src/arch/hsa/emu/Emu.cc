/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun
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

#include <arch/hsa/driver/Driver.h>

#include "Emu.h"


namespace HSA
{

//
// Class 'EmuConfig'
//

EmuConfig::EmuConfig()
{
	// Initialize
	sim_kind = comm::ArchSimFunctional;
}

void EmuConfig::Register(misc::CommandLine &command_line)
{
	// Option --hsa-debug <file>
	command_line.RegisterString("--hsa-debug <file>", hsa_debug_file, 
		"Dump debug information about hsa");
	
	// Option --hsa-sim <kind>
	command_line.RegisterEnum("--hsa-sim {functional|detailed} "
			"(default = functional)",
			(int &) sim_kind, comm::arch_sim_kind_map,
			"Level of accuracy of hsa simulation");
}

void EmuConfig::Process()
{
	Emu::hsa_debug.setPath(hsa_debug_file);
	// std::cout << "In" << __FUNCTION__ << "\n";	
}




//
// Class 'Emu'
//

misc::Debug Emu::hsa_debug;

std::unique_ptr<Emu> Emu::instance;

EmuConfig Emu::config;


Emu::Emu() : comm::Emu("hsa")
{
}


Emu *Emu::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	instance.reset(new Emu());
	return instance.get();
}


bool Emu::Run()
{
	return false;
}


}  // namespace HSA

