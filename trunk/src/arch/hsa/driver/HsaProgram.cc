/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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
#include <lib/cpp/String.h>
#include <arch/hsa/disassembler/BrigFile.h>

#include "HsaProgram.h"


namespace HSA
{

HsaProgram::HsaProgram() :
		modules()
{
}


HsaProgram::HsaProgram(const HsaProgram &program):
		HsaProgram()
{
	for (auto it = program.modules.begin();
			it != program.modules.end();
			it++)
	{
		AddModule((*it)->getBuffer());
	}
}


HsaProgram::~HsaProgram()
{
}


void HsaProgram::AddModule(const char *module)
{
	auto binary = misc::new_unique<BrigFile>();
	binary->LoadFileFromBuffer(module);
	modules.push_back(std::move(binary));
}

}  // namespace HSA

