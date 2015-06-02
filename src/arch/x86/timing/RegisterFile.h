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

#ifndef X86_ARCH_TIMING_REGISTER_FILE_H
#define X86_ARCH_TIMING_REGISTER_FILE_H

#include <lib/cpp/IniFile.h>
#include <arch/x86/emu/UInst.h>

#include "Uop.h"

namespace x86
{

// Forward declaration
class Thread;
class Core;

// class register file
class RegisterFile
{
public:

	/// Minimum size of INT FP and XMM register
	static const int MinINTSize;
	static const int MinFPSize;
	static const int MinXMMSize;

	/// enumeration of register file kind
	enum Kind
	{
		KindInvalid = 0,
		KindShared,
		KindPrivate
	};
	static misc::StringMap KindMap;

private:

	// Core and thread it belongs to
	Core *core;
	Thread *thread;

	// Structure of physical register
	struct PhysicalRegister
	{
		int pending;  // not completed (bit) 
		int busy;  // number of mapped logical registers 
	};

	// Integer registers 
	int int_rat[UInstDepIntCount]; // Int register aliasing table
	std::unique_ptr<PhysicalRegister[]> int_phy_reg;
	int int_phy_reg_count = 0;
	std::unique_ptr<int[]> int_free_phy_reg;
	int int_free_phy_reg_count = 0;

	// FP registers 
	int fp_top_of_stack = 0;  // Value between 0 and 7
	int fp_rat[UInstDepFpCount]; // Fp register aliasing table
	std::unique_ptr<PhysicalRegister[]> fp_phy_reg;
	int fp_phy_reg_count = 0;
	std::unique_ptr<int[]> fp_free_phy_reg;
	int fp_free_phy_reg_count = 0;

	// XMM registers 
	int xmm_rat[UInstDepXmmCount]; // Xmm register aliasing table
	std::unique_ptr<PhysicalRegister[]> xmm_phy_reg;
	int xmm_phy_reg_count = 0;
	std::unique_ptr<int[]> xmm_free_phy_reg;
	int xmm_free_phy_reg_count = 0;

	// Register file parameters
	static Kind kind;
	static int int_size;
	static int fp_size;
	static int xmm_size;
	static int int_local_size;
	static int fp_local_size;
	static int xmm_local_size;

public:

	/// Constructor
	RegisterFile(Core *core, Thread *thread);

	/// Read register file configuration from configuration file
	static void ParseConfiguration(misc::IniFile &ini_file);

	/// Initialize the register file
	void InitRegisterFile();

	/// Dump register file
	void DumpRegisterFile();

	/// Request an integer/FP/XMM physical register, and return its identifier.
	int RequestIntRegister();
	int RequestFPRegister();
	int RequestXMMRegister();

	/// Rename functions
	bool CanRename(Uop &uop);
	void Rename(Uop &uop);

	/// Check if input dependencies are resolved
	bool IsUopReady(Uop &uop);

	/// Operation on Uop
	void WriteUop(Uop &uop);
	void UndoUop(Uop &uop);
	void CommitUop(Uop &uop);

	/// Check register file
	void CheckRegisterFile();
};

}

#endif // X86_ARCH_TIMING_REGISTER_FILE_H
