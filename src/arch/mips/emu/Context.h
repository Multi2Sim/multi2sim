/*
 *  Multi2Sim
 *  Copyright (C) 2014  Sida Gu (gu.sid@husky.neu.edu)
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

#include <iostream>

namespace MIPS {

	/// Context States
	enum ContextState{
		ContextRunning	 = 0x00001,  // it is able to run instructions
		ContextSpecMode	 = 0x00002,  // executing in speculative mode
		ContextFinished  = 0x00003,  // executing finished
		ContextInvalid	 = 0x00000
	};

	/// mips Context
	class Context{
		// Emulator that it belongs to
		MIPSEmu *emu;

		// Context properties
		int Pid;
		int status;
		int address_space_index;

		// Parent Context
		MIPSContext *parent;

		// Instruction pointers
		unsigned last_eip;  // Address of last emulated instruction
		unsigned current_eip;  // Address of currently emulated instruction
		unsigned target_eip;  // Target address for branch, even if not taken

		// Context group initiator. There is only one group parent (if not null)
		// with many group children, no tree organization.
		Context *group_parent;

		// Program Data
		struct elf_file_t *elf_file;
		struct linked_list_t *args;
		struct linked_list_t *env;
		char *exe;  // Executable file name
		char *cwd;  // Current working directory
		char *stdin_file;  // File name for stdin
		char *stdout_file;  // File name for stdout

		// Stack
		unsigned int stack_base;
		unsigned int stack_top;
		unsigned int stack_size;
		unsigned int environ_base;


		// Program entries
	    unsigned int prog_entry;
		unsigned int interp_prog_entry;

		// Program headers
		unsigned int phdt_base;
		unsigned int phdr_count;


public:
		MIPSContext();
		~MIPSContext();

		/// Return the context pid
		int getPid()

		/// Get Context State and Set Context State
		int MIPSContextGetState(MIPSContextState status);
		void MIPSContextSetState(MIPSContextState status);
		void MIPSContextClearState(MIPSContextState status);




	}






}
