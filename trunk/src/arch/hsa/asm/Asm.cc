#include "Asm.h"
#include "BrigFile.h"
#include "BrigSection.h"
#include "BrigDirEntry.h"

namespace HSA
{


void AsmConfig::Register(misc::CommandLine &command_line)
{
	// Option --hsa-disasm <file>
	command_line.RegisterString("--hsa-disasm", path, 
			"Disassemble the HSA BRIG ELF file provided in <arg>. "	
			"This options is incompatible with any other option."
		);
	command_line.setIncompatible("--hsa-disasm");
}

void AsmConfig::Process()
{
	// Run hsa disassembler
	if(!path.empty())
	{
		Asm *as = Asm::getInstance();
		as->DisassembleBinary(path);
		exit(0);
	}
}

std::unique_ptr<Asm> Asm::as;
	
Asm *Asm::getInstance()
{
	if(Asm::as)
		return Asm::as.get();
	Asm::as.reset(new Asm());
	return Asm::as.get();
}

// hsa disassembler configuration
AsmConfig Asm::config;

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
