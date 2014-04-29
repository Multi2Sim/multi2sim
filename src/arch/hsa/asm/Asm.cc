#include "Asm.h"
#include "BrigFile.h"
#include "BrigSection.h"
#include "BrigDirEntry.h"

namespace HSA
{

std::unique_ptr<Asm> Asm::as;
	
Asm *Asm::getInstance()
{
	if(Asm::as)
		return Asm::as.get();
	Asm::as.reset(new Asm());
	return Asm::as.get();
}

Asm::Asm()
{
#define DEFINST(_name, _opstr)	\
	InitTable(Inst_##_name, #_name, _opstr);
#include "asm.dat"
#undef DEFINST
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
	while(bufPtr < buf + bs->getSize())
	{
		BrigDirEntry dir(bufPtr, &bf);
		dir.Dump();
		bufPtr = dir.next();
	}
}

} // namespace HSA
