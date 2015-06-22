/*
 *  Multi2Sim
 *  Copyright (C) 2013  Sida Gu (gu.sid@husky.neu.edu)
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
#include <unistd.h>

#include <lib/cpp/String.h>

#include "Context.h"
#include "Emulator.h"


namespace MIPS
{

static const unsigned LoaderStackBase = 0xc0000000;
static const unsigned LoaderMaxEnviron = 0x10000;  // 16KB for environment
static const unsigned LoaderStackSize = 0x800000;  // 8MB stack size

static misc::StringMap section_flags_map =
{
	{ "SHF_WRITE", 1 },
	{ "SHF_ALLOC", 2 },
	{ "SHF_EXECINSTR", 4 }
};


void Context::LoadInterp()
{
	// Debug
	emulator->loader_debug << misc::fmt("\nLoading program interpreter '%s'\n",
			loader->interp.c_str());

	// Load section from program interpreter
	ELFReader::File binary(loader->interp);
	LoadELFSections(&binary);

	// Change program entry to the one specified by the interpreter
	loader->interp_prog_entry = binary.getEntry();
	emulator->loader_debug << misc::fmt("  program interpreter entry: 0x%x\n\n",
			loader->interp_prog_entry);
}


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


void Context::LoadProgramHeaders()
{
	// Debug
	emulator->loader_debug << "\nLoading program headers\n";
	ELFReader::File *binary = loader->binary.get();

	// Load program header table from ELF
	int phdr_count = binary->getPhnum();
	int phdr_size = binary->getPhentsize();
	int phdt_size = phdr_count * phdr_size;
	assert(phdr_count == binary->getNumProgramHeaders());

	// Program header PT_PHDR, specifying location and size of the program
	// header table itself. Search for program header PT_PHDR, specifying
	// location and size of the program header table. If none found, choose
	// loader->bottom - phdt_size.
	unsigned phdt_base = loader->bottom - phdt_size;
	for (auto &program_header : binary->getProgramHeaders())
		if (program_header->getType() == PT_PHDR)
			phdt_base = program_header->getVaddr();
	emulator->loader_debug << misc::fmt("  virtual address for program header "
			"table: 0x%x\n", phdt_base);

	// Allocate memory for program headers
	memory->Map(phdt_base, phdt_size, mem::Memory::AccessInit
			| mem::Memory::AccessRead);

	// Load program headers
	int index = 0;
	for (auto &program_header : binary->getProgramHeaders())
	{
		// Load program header
		unsigned address = phdt_base + index * phdr_size;
		memory->Init(address, phdr_size, (char *)
				program_header->getRawInfo());

		// Debug
		emulator->loader_debug << misc::fmt("  header loaded at 0x%x\n", address)
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

		// Program interpreter
		if (program_header->getType() == 3)
			loader->interp = memory->ReadString(program_header->getVaddr());

		// Next
		index++;
	}

	// Free buffer and save pointers
	loader->phdt_base = phdt_base;
	loader->phdr_count = phdr_count;
}


void Context::LoadELFSections(ELFReader::File *binary)
{
	Emulator::loader_debug << "\nLoading ELF sections\n";
	loader->bottom = 0xffffffff;
	for (auto &section : binary->getSections())
	{
		// Debug
		unsigned perm = mem::Memory::AccessInit | mem::Memory::AccessRead;
		std::string flags_str = section_flags_map.MapFlags(section->getFlags());
		Emulator::loader_debug << misc::fmt("  section '%s': offset=0x%x, "
				"addr=0x%x, size=%u, flags=%s\n",
				section->getName().c_str(), section->getOffset(),
				section->getAddr(), section->getSize(),
				flags_str.c_str());

		// Process section
		if (section->getFlags() & SHF_ALLOC)
		{
			// Permissions
			if (section->getFlags() & SHF_WRITE)
				perm |= mem::Memory::AccessWrite;

			if (section->getFlags() & SHF_EXECINSTR)
			{
				perm |= mem::Memory::AccessExec;

				// Add region to call stack
				if (call_stack != nullptr)
				{
					call_stack->Map(binary->getPath(),
							section->getOffset(),
							section->getAddr(),
							section->getSize(),
							false);
				}
			}

			// Load section
			memory->Map(section->getAddr(), section->getSize(), perm);
			memory->growHeapBreak(section->getAddr() + section->getSize());
			loader->bottom = std::min(loader->bottom, section->getAddr());

			// If section type is SHT_NOBITS (sh_type=8), initialize to 0.
			// Otherwise, copy section contents from ELF file.
			if (section->getType() == 8)
			{
				char *zero_buffer = new char[section->getSize()]();
				memory->Init(section->getAddr(), section->getSize(),
						zero_buffer);
				delete zero_buffer;
			}
			else
			{
				memory->Init(section->getAddr(), section->getSize(),
						section->getBuffer());
			}
		}
	}
}


void Context::LoadAVEntry(unsigned &sp, unsigned type, unsigned value)
{
	// Write values
	memory->Write(sp, 4, (char *) &type);
	memory->Write(sp + 4, 4, (char *) &value);

	// Increase stack pointer
	sp += 8;
}


unsigned Context::LoadAV(unsigned where)
{
	// Debug
	unsigned sp = where;
	emulator->loader_debug << misc::fmt("Loading auxiliary vector at 0x%x\n", where);

	// Program headers
	LoadAVEntry(sp, 3, loader->phdt_base);  // AT_PHDR
	LoadAVEntry(sp, 4, 32);  // AT_PHENT -> program header size of 32 bytes
	LoadAVEntry(sp, 5, loader->phdr_count);  // AT_PHNUM

	// Other values
	LoadAVEntry(sp, 6, mem::Memory::PageSize);  // AT_PAGESZ
	LoadAVEntry(sp, 7, 0);  // AT_BASE
	LoadAVEntry(sp, 8, 0);  // AT_FLAGS
	LoadAVEntry(sp, 9, loader->prog_entry);  // AT_ENTRY
	LoadAVEntry(sp, 11, getuid());  // AT_UID
	LoadAVEntry(sp, 12, geteuid());  // AT_EUID
	LoadAVEntry(sp, 13, getgid());  // AT_GID
	LoadAVEntry(sp, 14, getegid());  // AT_EGID
	LoadAVEntry(sp, 17, 0x64);  // AT_CLKTCK
	LoadAVEntry(sp, 23, 0);  // AT_SECURE

	// Random bytes
	loader->at_random_addr_holder = sp + 4;
	LoadAVEntry(sp, 25, 0);  // AT_RANDOM

	/*LoadAVEntry(sp, 32, 0xffffe400);
	LoadAVEntry(sp, 33, 0xffffe000);
	LoadAVEntry(sp, 16, 0xbfebfbff);*/

	/* ??? AT_HWCAP, AT_PLATFORM, 32 and 33 ???*/

	// Finally, AT_NULL, and return size
	LoadAVEntry(sp, 0, 0);
	return sp - where;
}


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
	sp += LoadAV(sp);

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

	// Check that we didn't overflow the stack
	if (sp > LoaderStackBase)
		throw misc::Panic("Stack overflow in loader");
}


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
			throw Emulator::Error(misc::fmt("[%s] Cannot open standard "
					"output", loader->stdout_file_name.c_str()));

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
	loader->binary.reset(new ELFReader::File(loader->exe));

	// Read sections and program entry
	LoadELFSections(loader->binary.get());
	loader->prog_entry = loader->binary->getEntry();

	// Set heap break to the highest written address rounded up to
	// the memory page boundary.
	memory->setHeapBreak(misc::RoundUp(memory->getHeapBreak(),
			mem::Memory::PageSize));

	// Load program header table. If we found a PT_INTERP program header,
	// we have to load the program interpreter. This means we are dealing with
	// a dynamically linked application.
	LoadProgramHeaders();
	if (!loader->interp.empty())
			LoadInterp();

	// Stack
	LoadStack();

	// Register initialization
	regs.setSP(loader->environ_base);
	regs.setPC(loader->interp.empty() ? loader->prog_entry
			: loader->interp_prog_entry);
	previous_ip = 0;
	next_ip = regs.getPC();
	n_next_ip = next_ip + 4;

	//Debug
	emulator->loader_debug << misc::fmt("Program entry is 0x%x\n", regs.getPC())
			<< misc::fmt("Initial stack pointer is 0x%x\n", regs.getSP())
			<< misc::fmt("Heap start set to 0x%x\n", memory->getHeapBreak());
}


} // namespace MIPS

