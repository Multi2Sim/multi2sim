#include "BrigSection.h"
#include "BrigDirEntry.h"
#include "BrigStrEntry.h"
#include "BrigDef.h"
#include "lib/cpp/Misc.h"
#include "lib/cpp/String.h"

namespace HSA{

BrigDirEntry::BrigDirEntry(char *buf, BrigFile *file)
	:BrigEntry(buf, file)
{
}

int BrigDirEntry::getKind() const
{
	struct BrigDirectiveBase *dir 
		= (struct BrigDirectiveBase *)base;
	return dir->kind;
}

char *BrigDirEntry::next() const
{
	switch(this->getKind())
	{
	case BRIG_DIRECTIVE_FUNCTION:
	case BRIG_DIRECTIVE_KERNEL:
	{
		BrigSection *bs = file->getBrigSection(BrigSectionDirective);
		struct BrigDirectiveExecutable * dir
			= (struct BrigDirectiveExecutable *)base;
		char *bufPtr = (char *)bs->getBuffer() + dir->nextTopLevelDirective;
		return bufPtr;
	}
	default:
		return BrigEntry::next();
	}
	
}

BrigDirEntry::DumpDirectiveFn BrigDirEntry::dump_dir_fn[27] = 
{
	&BrigDirEntry::DumpDirectiveArgScopeEnd,	// 0 
	&BrigDirEntry::DumpDirectiveArgScopeStart, // 1
	&BrigDirEntry::DumpDirectiveBlockEnd,		// 2
	&BrigDirEntry::DumpDirectiveNumeric,		// 3
	&BrigDirEntry::DumpDirectiveBlockStart,	// 4
	&BrigDirEntry::DumpDirectiveBlockString,	// 5
	&BrigDirEntry::DumpDirectiveComment,		// 6
	&BrigDirEntry::DumpDirectiveControl,		// 7
	&BrigDirEntry::DumpDirectiveExtension,		// 8
	&BrigDirEntry::DumpDirectiveFBarrier,		// 9 
	&BrigDirEntry::DumpDirectiveFile,			// 10 
	&BrigDirEntry::DumpDirectiveFunction,		// 11 
	&BrigDirEntry::DumpDirectiveImage,			// 12 
	&BrigDirEntry::DumpDirectiveImageInit,		// 13 
	&BrigDirEntry::DumpDirectiveKernel,		// 14 
	&BrigDirEntry::DumpDirectiveLabel,			// 15 
	&BrigDirEntry::DumpDirectiveLabelInit,		// 16 
	&BrigDirEntry::DumpDirectiveLabelTargets,	// 17 
	&BrigDirEntry::DumpDirectiveLoc,			// 18 
	&BrigDirEntry::DumpDirectivePragma,		// 19 
	&BrigDirEntry::DumpDirectiveSampler,		// 20
	&BrigDirEntry::DumpDirectiveSamplerInit,	// 21 
	&BrigDirEntry::DumpDirectiveScope,			// 22 
	&BrigDirEntry::DumpDirectiveSignature,		// 23 
	&BrigDirEntry::DumpDirectiveVariable,		// 24 
	&BrigDirEntry::DumpDirectiveVariableInit,	// 25 
	&BrigDirEntry::DumpDirectiveVersion		// 26 
};

void BrigDirEntry::DumpDirectiveArgScopeEnd(std::ostream &os = std::cout) const
{
	os << "}";
}
void BrigDirEntry::DumpDirectiveArgScopeStart(std::ostream &os = std::cout) const
{
	os << "{";
}
void BrigDirEntry::DumpDirectiveBlockEnd(std::ostream &os = std::cout) const
{
	os << "endblock;";
}
void BrigDirEntry::DumpDirectiveNumeric(std::ostream &os = std::cout) const
{
	os << "blocknumeric";
	struct BrigBlockNumeric *dir = (struct BrigBlockNumeric *)this->base;
	os << BrigEntry::type2str(dir->type);
	this->dumpValueList(dir->data, dir->type, dir->elementCount, os);
}
void BrigDirEntry::DumpDirectiveBlockStart(std::ostream &os = std::cout) const
{
	os << "block ";
	struct BrigBlockStart *dir = (struct BrigBlockStart *)this->base;
	BrigStrEntry::DumpString(this->file, dir->name, os);
}
void BrigDirEntry::DumpDirectiveBlockString(std::ostream &os = std::cout) const
{
	os << "blockstring ";
	struct BrigBlockString *dir = (struct BrigBlockString *)this->base;
	BrigStrEntry::DumpString(this->file, dir->string, os);
	os << ";";
}
void BrigDirEntry::DumpDirectiveComment(std::ostream &os = std::cout) const
{
	misc::warning("Unsupport directive %s", "Comment");
}
void BrigDirEntry::DumpDirectiveControl(std::ostream &os = std::cout) const
{
	misc::warning("Unsupport directive %s", "Control");
}
void BrigDirEntry::DumpDirectiveExtension(std::ostream &os = std::cout) const
{
	misc::warning("Unsupport directive %s", "Extension");
}
void BrigDirEntry::DumpDirectiveFBarrier(std::ostream &os = std::cout) const
{		
	struct BrigDirectiveFbarrier * fbar = 
		(struct BrigDirectiveFbarrier *)this->base;
	os << "fbarrier ";
	os << BrigStrEntry::GetStringByOffset(this->file, fbar->name);
	os << ";\n";
}
void BrigDirEntry::DumpDirectiveFile(std::ostream &os = std::cout) const
{
	misc::warning("Unsupport directive %s", "File");
}
void BrigDirEntry::DumpDirectiveFunction(std::ostream &os = std::cout) const
{
	struct BrigDirectiveFunction *dir
		= (struct BrigDirectiveFunction *)this->base;
	// Pointer to the next directive in sequence
	// Cannot use this->next(), since it is overwritten
	char *next = BrigEntry::next();
	SymbolModifier modifier(dir->modifier.allBits);
	os << modifier.getLinkageStr() << "function ";
	os << BrigStrEntry::GetStringByOffset(this->file, dir->name);
	next = BrigEntry::dumpArgs(next, dir->outArgCount, os);
	next = BrigEntry::dumpArgs(next, dir->inArgCount, os);
	// Dump the function body;
	dumpBody(dir->code, dir->instCount, modifier.isDeclaration(), os);
	
}
void BrigDirEntry::DumpDirectiveImage(std::ostream &os = std::cout) const
{
	struct BrigDirectiveImage *image = 
		(struct BrigDirectiveImage *)this->base;
	BrigEntry::dumpSymDecl(this, os);
	if(image->init)
	{
		misc::warning("Image init is not supported!");
	}
	os << ";\n";
}
void BrigDirEntry::DumpDirectiveImageInit(std::ostream &os = std::cout) const
{
	misc::warning("Unsupport directive %s", "ImageInit");
}
void BrigDirEntry::DumpDirectiveKernel(std::ostream &os = std::cout) const
{
	struct BrigDirectiveKernel *dir
		= (struct BrigDirectiveKernel *)this->base;
	char *next = BrigEntry::next();
	os << "kernel ";
	os << BrigStrEntry::GetStringByOffset(this->file, dir->name);
	next = BrigEntry::dumpArgs(next, dir->inArgCount, os);
	dumpBody(dir->code, dir->instCount, false, os);
}
void BrigDirEntry::DumpDirectiveLabel(std::ostream &os = std::cout) const
{
	struct BrigDirectiveLabel *label
		= (struct BrigDirectiveLabel *)this->base;
	os << BrigStrEntry::GetStringByOffset(this->file, label->name);
	os << ":\n";
}
void BrigDirEntry::DumpDirectiveLabelInit(std::ostream &os = std::cout) const
{
	misc::warning("Unsupport directive %s", "LabelInit");
}
void BrigDirEntry::DumpDirectiveLabelTargets(std::ostream &os = std::cout) const
{
	misc::warning("Unsupport directive %s", "LabelTagets");
}
void BrigDirEntry::DumpDirectiveLoc(std::ostream &os = std::cout) const
{
	misc::warning("Unsupport directive %s", "Loc");
}
void BrigDirEntry::DumpDirectivePragma(std::ostream &os = std::cout) const
{
	misc::warning("Unsupport directive %s", "Pragma");
}
void BrigDirEntry::DumpDirectiveSampler(std::ostream &os = std::cout) const
{
	struct BrigDirectiveSampler *samp
		= (struct BrigDirectiveSampler *)this->base;
	BrigEntry::dumpSymDecl(this, os);
	if(samp->init)
	{
		misc::warning("Sampler init is not supported!");
	}
	os << ";\n";
}
void BrigDirEntry::DumpDirectiveSamplerInit(std::ostream &os = std::cout) const
{
	misc::warning("Unsupport directive %s", "SamplerInit");
}
void BrigDirEntry::DumpDirectiveScope(std::ostream &os = std::cout) const
{
	misc::warning("Unsupport directive %s", "Scope");
}
void BrigDirEntry::DumpDirectiveSignature(std::ostream &os = std::cout) const
{
	misc::warning("Unsupport directive %s", "Signature");
}
void BrigDirEntry::DumpDirectiveVariable(std::ostream &os = std::cout) const
{
	struct BrigDirectiveVariable *var 
		= (struct BrigDirectiveVariable *)this->base;
	BrigEntry::dumpSymDecl(this, os);
	if(var->init)
	{
		misc::warning("Variable init is not supported yet.");
	}
	os << ";\n";
}
void BrigDirEntry::DumpDirectiveVariableInit(std::ostream &os = std::cout) const
{
	misc::warning("Unsupport directive %s", "VariableInit");
}
void BrigDirEntry::DumpDirectiveVersion(std::ostream &os = std::cout) const
{
	struct BrigDirectiveVersion *dir 
		= (struct BrigDirectiveVersion *)this->base;
	os << misc::fmt("version %d:%d:", dir->hsailMajor, dir->hsailMinor);
	os << BrigEntry::profile2str(dir->profile);
	os << ":";
	os << BrigEntry::machineModel2str(dir->machineModel);
	os << misc::fmt(
			" // Brig Object Format Version %d:%d",
			dir->brigMajor, dir->brigMinor);
	os << "\n";
}
}
