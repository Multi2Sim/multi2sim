#include "Asm.h"
#include "BrigFile.h"
#include "BrigSection.h"
#include "BrigDirEntry.h"

namespace HSA
{

// File to disassembler, set by user
std::string Asm::path;

// Singleton instance
std::unique_ptr<Asm> Asm::instance;


void Asm::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("HSA");

	// Option --hsa-disasm <file>
	command_line->RegisterString("--hsa-disasm <file>", path,
			"Disassemble the HSA BRIG ELF file provided in <arg>. "	
			"This option is incompatible with any other option.");

	// Option incompatibility
	command_line->setIncompatible("--hsa-disasm");
}


void Asm::ProcessOptions()
{
	// Run hsa disassembler
	if (!path.empty())
	{
		Asm *as = Asm::getInstance();
		as->DisassembleBinary(path);
		exit(0);
	}
}


Asm *Asm::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new Asm());
	return instance.get();
}


Asm::Asm()
{
#define DEFINST(_name, _opstr)	\
	InitTable(Inst_##_name, #_name, _opstr);
#include "asm.dat"
#undef DEFINST
	indent = 0;
}

void Asm::InitTable(InstOpcode opcode, const char *name, const char *fmt_str)
{
	inst_info[opcode].opcode = opcode;
	inst_info[opcode].name = name;
	inst_info[opcode].fmt_str = fmt_str;	
}

void Asm::DisassembleBinary(const std::string &path) const
{
	BrigFile bf(path.c_str());
	BrigSection *bs = bf.getBrigSection(BrigSectionDirective);
	const char *buf = bs->getBuffer();

	char *bufPtr = (char *)buf;
	// Increament by 4 to skip the section size field
	bufPtr += 4;
	while(bufPtr && bufPtr < buf + bs->getSize())
	{
		BrigDirEntry dir(bufPtr, &bf);
		dir.Dump();
		bufPtr = dir.nextTop();
	}
}

} // namespace HSA
