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

#include <lib/cpp/Debug.h>
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

	/// Minimum size of integer register file
	static const int MinIntegerSize;

	/// Minimum size of floating-point register file
	static const int MinFloatingPointSize;

	/// Minimum size of XMM register file
	static const int MinXmmSize;

	/// Privacy kinds for register file
	enum Kind
	{
		KindInvalid = 0,
		KindShared,
		KindPrivate
	};

	/// String map for values of type Kind
	static misc::StringMap KindMap;

private:

	//
	// Class members
	//

	// Core that the register file belongs to, initialized in constructor
	Core *core;

	// Thread that the register file belongs to, initialized in constructor
	Thread *thread;

	// Structure of physical register
	struct PhysicalRegister
	{
		// Flag indicating whether the result of this physical register
		// is still being computed.
		bool pending = false;

		// Number of logical registers mapped to this physical register
		int busy = 0;
	};




	//
	// Integer registers
	//

	// Integer register aliasing table
	int integer_rat[Uinst::DepIntCount];

	// Integer physical registers
	std::unique_ptr<PhysicalRegister[]> integer_registers;

	// List of free integer physical registers
	std::unique_ptr<int[]> free_integer_registers;

	// Number of free integer physical registers
	int num_free_integer_registers = 0;

	// Number of reads to the integer RAT
	long long num_integer_rat_reads = 0;

	// Number of writes to the integer RAT
	long long num_integer_rat_writes = 0;

	// Number of occupied integer registers
	int num_occupied_integer_registers = 0;

	// Request an integer physical register, and return its identifier
	int RequestIntegerRegister();




	//
	// Floating-point registers
	//

	// Floating-point register aliasing table
	int floating_point_rat[Uinst::DepFpCount];

	// Floating-point physical registers
	std::unique_ptr<PhysicalRegister[]> floating_point_registers;

	// List of free floating-point physical registers
	std::unique_ptr<int[]> free_floating_point_registers;

	// Number of free floating-point physical registers
	int num_free_floating_point_registers = 0;

	// Value between 0 and 7 indicating the top of the stack in the
	// floating-point register stack
	int floating_point_top = 0;

	// Number of reads to the floating-point RAT
	long long num_floating_point_rat_reads = 0;

	// Number of writes to the floating-point RAT
	long long num_floating_point_rat_writes = 0;

	// Number of occupied float point registers
	int num_occupied_floating_point_registers= 0;

	// Request a floating-point physical register, and return its
	// identifier
	int RequestFloatingPointRegister();




	//
	// XMM registers
	//
	
	// XMM register aliasing table
	int xmm_rat[Uinst::DepXmmCount];

	// XMM physical registers
	std::unique_ptr<PhysicalRegister[]> xmm_registers;

	// List of free XMM physical registers
	std::unique_ptr<int[]> free_xmm_registers;

	// Number of free XMM physical registers
	int num_free_xmm_registers = 0;

	// Number of reads to the XMM RAT
	long long num_xmm_rat_reads = 0;

	// Number of writes to the XMM RAT
	long long num_xmm_rat_writes = 0;

	// Number of occupied XMM registers
	int num_occupied_xmm_registers= 0;

	// Request an XMM physical register and return its identifier
	int RequestXmmRegister();




	//
	// Configuration
	//

	// Private/shared register file
	static Kind kind;

	// Total size of integer register file
	static int integer_size;

	// Total size of floating-point register file
	static int floating_point_size;

	// Total size of XMM register file
	static int xmm_size;

	// Per-thread size of integer register file
	static int integer_local_size;

	// Per-thread size of floating-point register file
	static int floating_point_local_size;

	// Per-thread size of XMM register file
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

	// File to dump debug information
	static std::string debug_file;

	// Debug information
	static misc::Debug debug;
	
	/// Read register file configuration from configuration file
	static void ParseConfiguration(misc::IniFile *ini_file);

	/// Return the register file kind, as configured by the user
	static Kind getKind() { return kind; }

	/// Return the integer register file size, as configured by the user.
	static int getIntegerSize() { return integer_size; }

	/// Return the floating-point register file size, as configured.
	static int getFloatingPointSize() { return floating_point_size; }

	/// Return the XMM register file size, as configured by the user.
	static int getXmmSize() { return xmm_size; }




	//
	// Class members
	//

	/// Constructor
	RegisterFile(Thread *thread);

	/// Dump a plain-text representation of the object into the given output
	/// stream, or into the standard output if argument \a os is committed.
	void Dump(std::ostream &os = std::cout) const;
	
	/// Same as Dump()
	friend std::ostream &operator<<(std::ostream &os,
			const RegisterFile &register_file)
	{
		register_file.Dump(os);
		return os;
	}

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

	/// Check if integer register at certain index is free.
	/// Used only in testing.
	bool isIntegerRegisterFree(int index)
	{
		if (integer_registers[index].busy > 0)
			return false;
		return true;
	}

	/// Check if floating point register at certain index is free
	/// Used only in testing.
	bool isFloatingPointRegisterFree(int index)
	{
		if (floating_point_registers[index].busy > 0)
			return false;
		return true;
	}

	/// Check if xmm register at certain index is free
	/// Used only in testing
	bool isXmmRegisterFree(int index)
	{
		if (xmm_registers[index].busy > 0)
			return false;
		return true;
	}




	//
	// Statistics
	//

	/// Return the number of reads to the integer RAT
	long long getNumIntegerRatReads() const { return num_integer_rat_reads; }

	/// Return the number of writes to the integer RAT
	long long getNumIntegerRatWrites() const { return num_integer_rat_writes; }

	/// Return the number of reads to the floating-point RAT
	long long getNumFloatingPointRatReads() const { return num_floating_point_rat_reads; }

	/// Return the number of writes to the floating-point RAT
	long long getNumFloatingPointRatWrites() const { return num_floating_point_rat_writes; }

	/// Return the number of reads to the XMM RAT
	long long getNumXmmRatReads() const { return num_xmm_rat_reads; }

	/// Return the number of writes to the XMM RAT
	long long getNumXmmRatWrites() const { return num_xmm_rat_writes; }
};

}

#endif

