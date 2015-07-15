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

#ifndef ARCH_X86_TIMING_REGISTER_FILE_H
#define ARCH_X86_TIMING_REGISTER_FILE_H

#include <lib/cpp/IniFile.h>
#include <arch/x86/emulator/Uinst.h>

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

	// Core that the register file belongs to, initialized in constructor
	Core *core;

	// Thread that the register file belongs to, initialized in constructor
	Thread *thread;

	// Structure of physical register
	struct PhysicalRegister
	{
		int pending;  // not completed (bit) 
		int busy;  // number of mapped logical registers 
	};

	// Integer registers 
	int int_rat[Uinst::DepIntCount]; // Int register aliasing table
	std::unique_ptr<PhysicalRegister[]> int_phy_reg;
	int int_phy_reg_count = 0;
	std::unique_ptr<int[]> int_free_phy_reg;
	int int_free_phy_reg_count = 0;

	// FP registers 
	int fp_top_of_stack = 0;  // Value between 0 and 7
	int fp_rat[Uinst::DepFpCount]; // Fp register aliasing table
	std::unique_ptr<PhysicalRegister[]> fp_phy_reg;
	int fp_phy_reg_count = 0;
	std::unique_ptr<int[]> fp_free_phy_reg;
	int fp_free_phy_reg_count = 0;

	// XMM registers 
	int xmm_rat[Uinst::DepXmmCount]; // Xmm register aliasing table
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

	//
	// Class Error
	//

	/// Exception for X86 register file
	class Error : public misc::Error
	{
	public:

		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("X86 register file");
		}
	};



	//
	// Static members
	//

	/// Read register file configuration from configuration file
	static void ParseConfiguration(misc::IniFile *ini_file);




	//
	// Class members
	//

	/// Constructor
	RegisterFile(Thread *thread);

	/// Dump register file
	void DumpRegisterFile();

	/// Request an integer/FP/XMM physical register, and return its identifier.
	int RequestIntRegister();
	int RequestFPRegister();
	int RequestXMMRegister();

	/// Return true if there are enough available physical registers to
	/// rename the given uop.
	bool canRename(Uop *uop);

	/// Perform register renaming on the given uop. This operation renames
	/// source and destination registers, requesting as many physical
	/// registers as needed for the uop.
	void Rename(Uop *uop);

	/// Check if input dependencies are resolved
	bool isUopReady(Uop *uop);

	/// Update the state of the register file when an uop completes, that
	/// is, when its results are written back.
	void WriteUop(Uop *uop);

	/// Update the state of the register file when an uop is recovered from
	/// speculative execution
	void UndoUop(Uop *uop);

	/// Update the state of the register file when an uop commits
	void CommitUop(Uop *uop);

	/// Check integrity of register file
	void CheckRegisterFile();
};

}

#endif // ARCH_X86_TIMING_REGISTER_FILE_H
