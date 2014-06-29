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

#ifndef ARCH_HSA_EMU_WORKITEM_H
#define ARCH_HSA_EMU_WORKITEM_H

#include <memory>
//#include <map>

#include <arch/common/FileTable.h>
#include <arch/hsa/asm/BrigInstEntry.h>
#include <arch/hsa/asm/BrigDirEntry.h>
#include <arch/hsa/asm/BrigStrEntry.h>

#include "Registers.h"
#include "Function.h"
#include "Emu.h"

namespace HSA
{

class Emu;

/// HSA WorkItem
class WorkItem{

 	// Emulator that is belongs to 
 	Emu *emu;

 	// Process ID
 	int pid;

	// File descriptor table, shared by workitems
	std::shared_ptr<comm::FileTable> file_table;

	// Private register file of this work item
	Registers registers;




 	///
 	/// Functions and data stuctures related to the program loader
 	/// Functions implemented in CopntextLoader.cc
 	///

 	/// Structure containing information initialized by the program loader,
 	/// associated with a workitem. When a workitem is created from a 
 	/// program executable, a Loader object is associtaed to it. 
 	struct Loader
 	{
 		// Binary file in brig format
 		std::unique_ptr<BrigFile> binary;	

 		// Command-line arguments
 		std::vector<std::string> args;

 		// Executable file name
 		std::string exe;

 		// Current working directory
 		std::string cwd;

 		// Entry point of current program 
 		char *entry_point;

 		// File name for standard input and output
 		std::string stdin_file_name;
 		std::string stdout_file_name;

 		// Function table for the functions in the brig file
 		std::map<std::string, std::unique_ptr<Function>> 
 				function_table;
 	};

 	// Loader information. This information can be shared among multiple 
 	// workitems. For this reason, it is declared as a shared pointer. The
 	// last destructed workitem sharing this variable will automatically 
 	// free it.
 	std::shared_ptr<Loader> loader;

 	// Load functions in the brig table. Prepare the function table in 
 	// loader.
 	// \return 
 	// 	Number of functions loaded
 	unsigned int loadFunctions();

 	// Parse and create a function object
 	void parseFunction(BrigDirEntry *dir);

 	// Load output arguments for a function
 	//
 	// \param num_out_arg
 	// 	Number of output arguments
 	//
 	// \param next_dir
 	// 	Pointer to the argument to start with
 	//
 	// \param isInput
 	//	if true, add input arguments. Otherwise, add output arguments
 	//
 	// \param function
 	// 	Pointer to the function to load arguments
 	//
 	// \return
 	// 	Pointer to next directive to parse
 	char *loadArguments(unsigned short num_arg, char *next_dir,
 			bool isInput, Function* function);

 	// Find the main function of the brig elf.
 	// \return
 	// 	pointer to the first inst of the main function
 	//	or nullptr if main function is not found
 	char* findMainFunction();

 	// Load Brig ELF binary, as alread decoded in 'loader.binary'
 	void LoadBinary();




 	//
 	// Functions related with the insts of HSA assembly, implemented in
 	// hsaIsa.cc
 	//

 	// The program counter, pointing to the inst in .code section
 	char* pc;

 	// Prototype of member function of class WorkItem devoted to the 
 	// execution of HSA virtual ISA instructions.
 	typedef void (WorkItem::*ExecuteInstFn)();

 	// Instruction emulation functions. Each entry of of Inst.def will be 
 	// expanded into a funtion prototype.
#define DEFINST(name, opstr) \
 		void ExecuteInst_##name();
#include <arch/hsa/asm/Inst.def>
#undef DEFINST

 	// unsupported inst opcode
 	void ExecuteInst_unsupported();

 	// Get the value of the index-th operand
 	template <typename Type>
 	Type getOperandValue(unsigned int index);

 	// Store the value into registers marked by the operand
 	template <typename Type>
 	void storeOperandValue(unsigned int index, Type value);

 	// Table of functions that implement instructions
 	static ExecuteInstFn execute_inst_fn[InstOpcodeCount + 1];

 public:

 	/// Create a work item from a command line. To safely create a  
 	/// function Emu::NewContext should be used instead. After the creation 
 	/// of a work item, its basic data structures are initialized with 
 	/// Load(), Clone(), or Fork()
 	WorkItem();

 	/// Destructor
 	~WorkItem();

 	/// Returns the pid of this work item
 	int getPid() const{return pid;}

 	/// Load a program on the workitem. The meaning of each argument is 
 	/// identical to the prototype of comm::Emy::Load()
 	void Load(const std::vector<std::string> &args,
 			const std::vector<std::string> &env = { },
 			const std::string &cwd = "",
 			const std::string &stdin_file_name = "",
 			const std::string &stdout_file_name = "");

 	/// Run one instruction for the workitem at the position pointed 
 	void Execute();

};

}  // namespace HSA

 #endif

