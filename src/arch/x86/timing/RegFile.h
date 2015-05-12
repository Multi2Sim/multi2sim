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

#ifndef X86_ARCH_TIMING_REG_FILE_H
#define X86_ARCH_TIMING_REG_FILE_H

#include <lib/cpp/IniFile.h>
#include <arch/x86/emu/UInst.h>

#include "Thread.h"

namespace x86
{

// Minimum size of INT FP and XMM register
const int RegFileMinINTSize = UInstDepIntCount + UInstMaxODeps;
const int RegFileMinFPSize = UInstDepFpCount + UInstMaxODeps;
const int RegFileMinXMMSize = UInstDepXmmCount + UInstMaxODeps;

// class register file
class RegFile
{
public:

	// enumeration of register file
	enum Type
	{
		TypeInvalid = 0,
		TypeShared,
		TypePrivate
	};

private:

	// Structure of physical register
	struct PhysicalRegister
	{
		int pending;  // not completed (bit) 
		int busy;  // number of mapped logical registers 
	};

	// Integer registers 
	int int_rat[UInstDepIntCount];
	std::unique_ptr<PhysicalRegister> int_phy_reg;
	int int_phy_reg_count;
	std::unique_ptr<int> int_free_phy_reg;
	int int_free_phy_reg_count;

	// FP registers 
	int fp_top_of_stack;  // Value between 0 and 7 
	int fp_rat[UInstDepFpCount];
	std::unique_ptr<PhysicalRegister> fp_phy_reg;
	int fp_phy_reg_count;
	std::unique_ptr<int> fp_free_phy_reg;
	int fp_free_phy_reg_count;

	// XMM registers 
	int xmm_rat[UInstDepXmmCount];
	std::unique_ptr<PhysicalRegister> xmm_phy_reg;
	int xmm_phy_reg_count;
	std::unique_ptr<int> xmm_free_phy_regg;
	int xmm_free_phy_reg_count;

	// Register file parameter
	static int int_size;
	static int fp_size;
	static int xmm_size;
	static int int_local_size;
	static int fp_local_size;
	static int xmm_local_size;

public:

	/// Constructor
	RegFile();

	/// Read register file configuration from configuration file
	static void ParseConfiguration(const std::string &section,
				misc::IniFile &config);
};

}

#endif // X86_ARCH_TIMING_REG_FILE_H
