/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#include <fcntl.h>
#include <fstream>
#include <unistd.h>

#include <lib/cpp/String.h>

#include "Context.h"
#include "Emulator.h"


namespace x86
{

#if 0
int x86_loader_debug_category;

char *x86_loader_help =
	"A context configuration file contains a list of executable programs and\n"
	"their parameters that will be simulated by Multi2Sim. The context\n"
	"configuration file is a plain text file in the IniFile format, containing\n"
	"as many sections as x86 programs simulated. Each program is denoted with\n"
	"a section called '[ Context <num> ]', where <num> is an integer number\n"
	"starting from 0.\n"
	"\n"
	"Variables in section '[ Context <num> ]':\n"
	"\n"
	"  Exe = <path> (Required)\n"
	"      Path for the x86 executable file that will be simulated.\n"
	"  Args = <arg_list>\n"
	"      List of command-line arguments for the simulated program.\n"
	"  Env = <env_list>\n"
	"      List of environment variables enumerated using single or double\n"
	"      quotes. These variables will be added to the current set of\n"
	"      active environment variables.\n"
	"      E.g.: Env = 'ENV_VAR1=100' \"ENV_VAR2=200\"\n"
	"  Cwd = <path>\n"
	"      Current working directory for simulated program. If not specified,\n"
	"      the current working directory for the simulator will be also used\n"
	"      for the simulated program.\n"
	"  StdIn = <file>\n"
	"      File to use as standard input for the simulated program. If none\n"
	"      specified, the simulator standard input is selected.\n"
	"  StdOut = <file>\n"
	"      File to use as standard output and standard error output for the\n"
	"      simulated program. If none specified, the standard output for the\n"
	"      simulator is used in both cases.\n"
	"  IPCReport = <file>\n"
	"      File to dump a report of the context performance. At specific\n"
	"      intervals, the context IPC (instructions-per-cycle) value will be\n"
	"      dumped in this file. This option must be specified together with\n"
	"      command-line option '--x86-sim detailed'.\n"
	"  IPCReportInterval = <cycles>\n"
	"      Interval in number of cycles that a new record will be added into\n"
	"      the IPC report file.\n"
	"\n"
	"See the Multi2Sim Guide (www.multi2sim.org) for further details and\n"
	"examples on how to use the context configuration file.\n"
	"\n";
#endif


#if 0
/* Add environment variables from the actual environment plus
 * the list attached in the argument 'env'. */
void X86ContextAddEnv(X86Context *self, char *env)
{
	struct x86_loader_t *loader = self->loader;
	extern char **environ;

	char *next;
	char *str;

	int i;

	// Add variables from actual environment
	for (i = 0; environ[i]; i++)
	{
		str = str_set(NULL, environ[i]);
		linked_list_add(loader->env, str);
	}
	
	// Add the environment vars provided in 'env'
	while (env)
	{
		// Skip spaces
		while (*env == ' ')
			env++;
		if (!*env)
			break;

		// Get new environment variable
		switch (*env)
		{

		case '"':
		case '\'':
			if (!(next = strchr(env + 1, *env)))
				fatal("%s: wrong format", __FUNCTION__);
			*next = 0;
			str = str_set(NULL, env + 1);
			linked_list_add(loader->env, str);
			env = next + 1;
			break;

		default:
			str = str_set(NULL, env);
			linked_list_add(loader->env, str);
			env = NULL;
		}
	}
}

#endif


static const unsigned LoaderStackBase = 0xc0000000;
static const unsigned LoaderMaxEnviron = 0x10000;  // 16KB for environment
static const unsigned LoaderStackSize = 0x800000;  // 8MB stack size


misc::StringMap Context::program_header_type_map =
{
	{ "PT_NULL",        0 },
	{ "PT_LOAD",        1 },
	{ "PT_DYNAMIC",     2 },
	{ "PT_INTERP",      3 },
	{ "PT_NOTE",        4 },
	{ "PT_SHLIB",       5 },
	{ "PT_PHDR",        6 },
	{ "PT_TLS",         7 }
};


unsigned Context::LoadSegments(ELFReader::File *binary)
{
	// Highest written memory address
	unsigned top = 0;

	// Debug
	emulator->loader_debug << "\nLoading segments\n";

	// Traverse segments
	for (auto &program_header : binary->getProgramHeaders())
	{
		// Skip if segment is not loadable
		if (program_header->getType() != 1)
			continue;

		// Debug
		emulator->loader_debug << "  segment loaded\n"
				<< misc::fmt("    type=%s, offset=0x%x, vaddr=0x%x, paddr=0x%x\n",
				program_header_type_map.MapValue(program_header->getType()),
				program_header->getOffset(),
				program_header->getVaddr(),
				program_header->getPaddr())
				<< misc::fmt("    filesz=%d, memsz=%d, flags=%d, align=%d\n",
				program_header->getFilesz(),
				program_header->getMemsz(),
				program_header->getFlags(),
				program_header->getAlign());

		// Compute segment permissions, initially just init and read
		unsigned perm = mem::Memory::AccessInit | mem::Memory::AccessRead;
		
		// Check if segment has write permission
		if (program_header->getFlags() & PF_W)
			perm |= mem::Memory::AccessWrite;

		// Check if segment has execution permission
		if (program_header->getFlags() & PF_X)
			perm |= mem::Memory::AccessExec;

		// Map segment in memory
		memory->Map(program_header->getVaddr(),
				program_header->getMemsz(),
				perm);
		
		// Load segment
		memory->Init(program_header->getVaddr(),
				program_header->getFilesz(),
				program_header->getBuffer());

		// Record highest address
		top = std::max(top, program_header->getVaddr() + program_header->getMemsz());
	}

	// Return highest address
	return top;
}


static const unsigned LoaderInterpBase = 0xc0001000;
static const unsigned LoaderInterpMaxSize = 0x800000;  // 8MB

void Context::LoadInterpreter()
{
	// Traverse program headers in search of a program interpreter
	for (auto &program_header : loader->binary->getProgramHeaders())
	{
		// Skip if not a program interpreter entry
		if (program_header->getType() != 3)
			continue;

		// Found
		loader->interpreter = memory->ReadString(program_header->getVaddr());
		break;
	}

	// No program interpreter
	if (loader->interpreter.empty())
	{
		// Debug
		emulator->loader_debug << "\nNo program interpreter found\n";

		// Done
		return;
	}

	// Debug
	emulator->loader_debug << misc::fmt("\nLoading program interpreter '%s'\n",
			loader->interpreter.c_str());

	// Open interpreter
	std::ifstream f(loader->interpreter);
	if (!f)
		throw Error(loader->interpreter + ": Invalid interpreter");

	// Get file size
	f.seekg(0, std::ios_base::end);
	unsigned size = f.tellg();
	f.seekg(0, std::ios_base::beg);

	// Check maximum size
	if (size > LoaderInterpMaxSize)
		throw Error(loader->interpreter + ": Interpreter too large");
	
	// Read content of interpreter
	auto buffer = misc::new_unique_array<char>(size);
	f.read(buffer.get(), size);
	f.close();

	// Load interpreter in memory
	unsigned address = LoaderInterpBase;
	memory->Map(address, size, mem::Memory::AccessInit);
	memory->Init(address, size, buffer.get());
	
	// Debug
	emulator->loader_debug << misc::fmt(
			"\n\tInterpreter loaded at 0x%x (%u bytes)\n\n",
			address, size);
	
	// Load segments from program interpreter
	ELFReader::File binary(loader->interpreter);
	LoadSegments(&binary);
	
	// Change program entry to the one specified by the interpreter
	loader->interp_prog_entry = binary.getEntry();
	emulator->loader_debug << misc::fmt("  program interpreter entry: 0x%x\n\n",
			loader->interp_prog_entry);
	
}


void Context::LoadAuxiliaryVectorEntry(unsigned &sp, unsigned type, unsigned value)
{
	// Write values
	memory->Write(sp, 4, (char *) &type);
	memory->Write(sp + 4, 4, (char *) &value);

	// Increase stack pointer
	sp += 8;
}


unsigned Context::LoadAuxiliaryVector(unsigned where)
{
	// Debug
	unsigned sp = where;
	emulator->loader_debug << misc::fmt("Loading auxiliary vector at 0x%x\n", where);
	
	// Search for program header PT_PHDR, which specifies the location of
	// the program header table itself.
	unsigned phdt_base = 0;
	for (auto &program_header : loader->binary->getProgramHeaders())
		if (program_header->getType() == PT_PHDR)
			phdt_base = program_header->getVaddr();

	// AT_PHDR
	if (phdt_base)
		LoadAuxiliaryVectorEntry(sp, 3, phdt_base);

	// AT_PHENT
	LoadAuxiliaryVectorEntry(sp, 4, 32);

	// AT_PHNUM
	LoadAuxiliaryVectorEntry(sp, 5, loader->binary->getPhnum());

	// AT_PAGESZ
	LoadAuxiliaryVectorEntry(sp, 6, mem::Memory::PageSize);

	// AT_BASE
	if (!loader->interpreter.empty())
		LoadAuxiliaryVectorEntry(sp, 7, LoaderInterpBase);

	// AT_FLAGS
	LoadAuxiliaryVectorEntry(sp, 8, 0);

	// AT_ENTRY
	LoadAuxiliaryVectorEntry(sp, 9, loader->prog_entry);

	// AT_UID
	LoadAuxiliaryVectorEntry(sp, 11, getuid());

	// AT_EUID
	LoadAuxiliaryVectorEntry(sp, 12, geteuid());

	// AT_GID
	LoadAuxiliaryVectorEntry(sp, 13, getgid());

	// AT_EGID
	LoadAuxiliaryVectorEntry(sp, 14, getegid());

	// AT_PLATFORM
	loader->at_platform_ptr = sp + 4;
	LoadAuxiliaryVectorEntry(sp, 15, 0);

	// AT_HWCAP
	LoadAuxiliaryVectorEntry(sp, 16, 0x78bfbff);

	// AT_CLKTCK
	LoadAuxiliaryVectorEntry(sp, 17, 0x64);

	// AT_SECURE
	LoadAuxiliaryVectorEntry(sp, 23, 0);

	// AT_RANDOM
	loader->at_random_addr_holder = sp + 4;
	LoadAuxiliaryVectorEntry(sp, 25, 0);

	/*LoadAuxiliaryVectorEntry(sp, 32, 0xffffe400);
	LoadAuxiliaryVectorEntry(sp, 33, 0xffffe000);
	LoadAuxiliaryVectorEntry(sp, 16, 0xbfebfbff);*/

	/* ??? 32 and 33 ???*/

	// AT_NULL
	LoadAuxiliaryVectorEntry(sp, 0, 0);

	// Return size of auxiliary vector
	return sp - where;
}


/* Load stack with the following layout.
 *
 * Address		Description			Size	Value
 * ------------------------------------------------------------------------------
 * (0xc0000000)		< bottom of stack >		0	(virtual)
 * (0xbffffffc)		[ end marker ]			4	(= NULL)
 *
 *			[ platform string ]		= "i686"
 *
 * 			[ environment ASCIIZ strings ]	>= 0
 * 			[ argument ASCIIZ strings ]	>= 0
 * 			[ padding ]			0 - 16
 *
 * 			[ auxv[term] (Elf32_auxv_t) ]	8	(= AT_NULL vector)
 * 			[ auxv[...] (Elf32_auxv_t) ]
 * 			[ auxv[1] (Elf32_auxv_t) ]	8
 * 			[ auxv[0] (Elf32_auxv_t) ]	8
 *
 * 			[ envp[term] (pointer) ]	4	(= NULL)
 * 			[ envp[...] (pointer) ]
 * 			[ envp[1] (pointer) ]		4
 * 			[ envp[0] (pointer) ]		4
 *
 * 			[ argv[argc] (pointer) ]	4	(= NULL)
 * 			[ argv[argc - 1] (pointer) ]	4
 * 			[ argv[...] (pointer) ]
 * 			[ argv[1] (pointer) ]		4
 * 			[ argv[0] (pointer) ]		4	(program name)
 * stack pointer ->	[ argc ]			4	(number of arguments)
 */

void Context::LoadStack()
{
	// Allocate stack
	loader->stack_base = LoaderStackBase;
	loader->stack_size = LoaderStackSize;
	loader->stack_top = LoaderStackBase - LoaderStackSize;
	memory->Map(loader->stack_top, loader->stack_size,
			mem::Memory::AccessRead | mem::Memory::AccessWrite);
	emulator->loader_debug << misc::fmt("mapping region for stack from 0x%x to 0x%x\n",
			loader->stack_top, loader->stack_base - 1);
	
	// Load arguments and environment variables
	loader->environ_base = LoaderStackBase - LoaderMaxEnviron;
	unsigned sp = loader->environ_base;
	int argc = loader->args.size();
	emulator->loader_debug << misc::fmt("  saved 'argc=%d' at 0x%x\n", argc, sp);
	memory->Write(sp, 4, (char *) &argc);
	sp += 4;
	unsigned argvp = sp;
	sp += (argc + 1) * 4;

	// Save space for environ and null
	unsigned envp = sp;
	sp += loader->env.size() * 4 + 4;

	// Load here the auxiliary vector
	sp += LoadAuxiliaryVector(sp);

	// Write arguments into stack
	emulator->loader_debug << "\nArguments:\n";
	for (int i = 0; i < argc; i++)
	{
		std::string str = loader->args[i];
		memory->Write(argvp + i * 4, 4, (char *) &sp);
		memory->WriteString(sp, str);
		emulator->loader_debug << misc::fmt("  argument %d at 0x%x: '%s'\n",
				i, sp, str.c_str());
		sp += str.length() + 1;
	}
	unsigned zero = 0;
	memory->Write(argvp + argc * 4, 4, (char *) &zero);

	// Write environment variables
	emulator->loader_debug << "\nEnvironment variables:\n";
	for (unsigned i = 0; i < loader->env.size(); i++)
	{
		std::string str = loader->env[i];
		memory->Write(envp + i * 4, 4, (char *) &sp);
		memory->WriteString(sp, str);
		emulator->loader_debug << misc::fmt("  env var %d at 0x%x: '%s'\n",
				i, sp, str.c_str());
		sp += str.length() + 1;
	}
	memory->Write(envp + loader->env.size() * 4, 4, (char *) &zero);

	// Random bytes
	loader->at_random_addr = sp;
	for (int i = 0; i < 16; i++)
	{
		char c = random();
		memory->Write(sp, 1, &c);
		sp++;
	}
	memory->Write(loader->at_random_addr_holder, 4, (char *)
			&loader->at_random_addr);
	
	// Platform string
	std::string platform = "i686";
	assert(loader->at_platform_ptr);
	memory->WriteString(sp, platform);
	memory->Write(loader->at_platform_ptr, 4, (char *) &sp);
	sp += platform.length() + 1;

	// Check that we didn't overflow the stack
	if (sp > LoaderStackBase)
		misc::fatal("%s: initial stack overflow, increment LoaderMaxEnviron",
			__FUNCTION__);
}


/*
 * The following layout is used for the initial virtual memory image of the
 * new context:
 *
 *
 * Low		High		Size			Description
 * ------------------------------------------------------------------------------
 * 0xc0001000	0xc0801000	0x800000 (8MB) max	Interpreter
 * 0xbf800000	0xc0000000	0x800000 (8MB)		Stack
 * <variable>	0xb7fb0000	<variable>		Dynamic data (mmap)
 *
 */

void Context::LoadBinary()
{
	// Alternative stdin
	if (!loader->stdin_file_name.empty())
	{
		// Open new stdin
		int f = open(loader->stdin_file_name.c_str(), O_RDONLY);
		if (f < 0)
			misc::fatal("%s: cannot open stdin",
					loader->stdin_file_name.c_str());

		// Replace file descriptor 0
		file_table->freeFileDescriptor(0);
		file_table->newFileDescriptor(
				comm::FileDescriptor::TypeStandard,
				0,
				f,
				loader->stdin_file_name,
				O_RDONLY);
	}

	// Alternative stdout/stderr
	if (!loader->stdout_file_name.empty())
	{
		// Open new stdout
		int f = open(loader->stdout_file_name.c_str(),
				O_CREAT | O_APPEND | O_TRUNC | O_WRONLY,
				0660);
		if (f < 0)
			misc::fatal("%s: cannot open stdout",
					loader->stdout_file_name.c_str());

		// Replace file descriptors 1 and 2
		file_table->freeFileDescriptor(1);
		file_table->freeFileDescriptor(2);
		file_table->newFileDescriptor(
				comm::FileDescriptor::TypeStandard,
				1,
				f,
				loader->stdout_file_name,
				O_WRONLY);
		file_table->newFileDescriptor(
				comm::FileDescriptor::TypeStandard,
				2,
				f,
				loader->stdout_file_name,
				O_WRONLY);
	}
	
	// Load ELF binary
	loader->binary = misc::new_unique<ELFReader::File>(loader->exe);

	// Read sections and program entry
	loader->prog_entry = loader->binary->getEntry();

	// Load segments from binary. The returned value is the highest virtual
	// address used by a segment. Round it up to the page boundary.
	unsigned top = LoadSegments(loader->binary.get());
	top = misc::RoundUp(top, mem::Memory::PageSize);

	// Set heap break to the highest written address rounded up to
	// the memory page boundary.
	memory->setHeapBreak(top);

	// Load program header table. If we found a PT_INTERP program header,
	// we have to load the program interpreter. This means we are dealing with
	// a dynamically linked application.
	// Load program interpreter, if any
	LoadInterpreter();

	// Stack
	LoadStack();

	// Register initialization
	regs.setEsp(loader->environ_base);
	regs.setEip(loader->interpreter.empty() ? loader->prog_entry
			: loader->interp_prog_entry);

	// Debug
	emulator->loader_debug << misc::fmt("Program entry is 0x%x\n", regs.getEip())
			<< misc::fmt("Initial stack pointer is 0x%x\n", regs.getEsp())
			<< misc::fmt("Heap start set to 0x%x\n", memory->getHeapBreak());
}


}  // namespace x86

