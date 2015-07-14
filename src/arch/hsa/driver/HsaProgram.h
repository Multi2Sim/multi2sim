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

#ifndef ARCH_HSA_EMU_HSAPROGRAM_H
#define ARCH_HSA_EMU_HSAPROGRAM_H

#include <vector>

namespace HSA
{

class BrigFile;

/**
 * An HSA program is a group of HSA modules
 */
class HsaProgram
{
private:

	// Modules in the HSA program
	std::vector<std::unique_ptr<BrigFile>> modules;

public:

	/**
	 * Constructor
	 */
	HsaProgram();

	/**
	 * Copy constructor
	 */
	HsaProgram(const HsaProgram &program);

	/**
	 * Destructor
	 */
	~HsaProgram();

	/**
	 * Add and HSA module to program
	 */
	void AddModule(const char *module);

	/**
	 * Get the modules
	 */
	const std::vector<std::unique_ptr<BrigFile>> *getModules() const 
	{
		return &modules;
	}
	
};

/**
 * The HsaCodeObject is meant to be an finalized code. However, in our case,
 * we do not need finish the code. Therefore, the HsaCodeObject is identical to
 * an HsaProgram
 */
typedef HsaProgram HsaCodeObject;

}  // namespace HSA

#endif  // ARCH_HSA_EMU_HSAPROGRAM_H
