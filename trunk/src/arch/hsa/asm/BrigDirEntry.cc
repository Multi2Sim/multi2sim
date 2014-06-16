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

#include "BrigSection.h"
#include "BrigDirEntry.h"
#include "BrigStrEntry.h"
#include "BrigDef.h"
#include "SymbolModifier.h"
#include "Asm.h"
#include "lib/cpp/Misc.h"
#include "lib/cpp/String.h"
#include "Asm.h"

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

char *BrigDirEntry::nextTop() const
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
		if(bufPtr >= bs->getBuffer() + bs->getSize())
			return NULL;
		return bufPtr;
	}
	case BRIG_DIRECTIVE_IMAGE:
	case BRIG_DIRECTIVE_VARIABLE:
	case BRIG_DIRECTIVE_SAMPLER:
	{
		struct BrigDirectiveSymbol *dir = 
			(struct BrigDirectiveSymbol *)this->base;
		if(!dir->init) return next();
		else
		{
			BrigDirEntry initDir(
				BrigDirEntry::GetDirByOffset(file, dir->init), file
			);
			return initDir.next();
		}
	}
	default:
		return next();
	}
}


char *BrigDirEntry::next() const
{
	BrigSection *bs = file->getBrigSection(BrigSectionDirective);
	char * ret = this->base + this->getSize();
	if(ret >= bs->getBuffer() + bs->getSize())
		return NULL;
	return ret;
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
	Asm *as = Asm::getInstance();
	as->indent--;
	BrigEntry::dumpIndent(os);
	os << "}\n";
}
void BrigDirEntry::DumpDirectiveArgScopeStart(std::ostream &os = std::cout) const
{
	Asm *as = Asm::getInstance();
	BrigEntry::dumpIndent(os);
	os << "{\n";
	as->indent++;
}
void BrigDirEntry::DumpDirectiveBlockEnd(std::ostream &os = std::cout) const
{
	BrigEntry::dumpIndent(os);
	os << "endblock;";
}
void BrigDirEntry::DumpDirectiveNumeric(std::ostream &os = std::cout) const
{
	BrigEntry::dumpIndent(os);
	os << "blocknumeric";
	struct BrigBlockNumeric *dir = (struct BrigBlockNumeric *)this->base;
	os << BrigEntry::type2str(dir->type);
	this->dumpValueList(dir->data, dir->type, dir->elementCount, this->file, os);
}
void BrigDirEntry::DumpDirectiveBlockStart(std::ostream &os = std::cout) const
{
	BrigEntry::dumpIndent(os);
	os << "block ";
	struct BrigBlockStart *dir = (struct BrigBlockStart *)this->base;
	BrigStrEntry::DumpString(this->file, dir->name, os);
}
void BrigDirEntry::DumpDirectiveBlockString(std::ostream &os = std::cout) const
{
	BrigEntry::dumpIndent(os);
	os << "blockstring ";
	struct BrigBlockString *dir = (struct BrigBlockString *)this->base;
	BrigStrEntry::DumpString(this->file, dir->string, os);
	os << ";";
}
void BrigDirEntry::DumpDirectiveComment(std::ostream &os = std::cout) const
{
	BrigEntry::dumpIndent(os);
	misc::warning("Unsupport directive %s", "Comment");
}
void BrigDirEntry::DumpDirectiveControl(std::ostream &os = std::cout) const
{
	BrigEntry::dumpIndent(os);
	misc::warning("Unsupport directive %s", "Control");
}
void BrigDirEntry::DumpDirectiveExtension(std::ostream &os = std::cout) const
{
	BrigEntry::dumpIndent(os);
	misc::warning("Unsupport directive %s", "Extension");
}
void BrigDirEntry::DumpDirectiveFBarrier(std::ostream &os = std::cout) const
{		
	BrigEntry::dumpIndent(os);
	struct BrigDirectiveFbarrier * fbar = 
		(struct BrigDirectiveFbarrier *)this->base;
	os << "fbarrier ";
	os << BrigStrEntry::GetStringByOffset(this->file, fbar->name);
	os << ";\n";
}
void BrigDirEntry::DumpDirectiveFile(std::ostream &os = std::cout) const
{
	BrigEntry::dumpIndent(os);
	misc::warning("Unsupport directive %s", "File");
}
void BrigDirEntry::DumpDirectiveFunction(std::ostream &os = std::cout) const
{
	BrigEntry::dumpIndent(os);
	struct BrigDirectiveFunction *dir
		= (struct BrigDirectiveFunction *)this->base;
	// Pointer to the next directive in sequence
	char *next = this->next();
	SymbolModifier modifier(dir->modifier.allBits);
	os << "\n";
	os << modifier.getLinkageStr() << "function ";
	os << BrigStrEntry::GetStringByOffset(this->file, dir->name);
	next = BrigEntry::dumpArgs(next, dir->outArgCount,this->file, os);
	next = BrigEntry::dumpArgs(next, dir->inArgCount, this->file, os);
	// Dump the function body;
	dumpBody(dir->code, dir->instCount, next, modifier.isDeclaration(), os);
	
}
void BrigDirEntry::DumpDirectiveImage(std::ostream &os = std::cout) const
{
	BrigEntry::dumpIndent(os);
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
	BrigEntry::dumpIndent(os);
	struct BrigDirectiveKernel *dir
		= (struct BrigDirectiveKernel *)this->base;
	char *next = this->next();
	os << "\nkernel ";
	os << BrigStrEntry::GetStringByOffset(this->file, dir->name);
	next = BrigEntry::dumpArgs(next, dir->inArgCount, this->file, os);
	dumpBody(dir->code, dir->instCount, next, false, os);
}
void BrigDirEntry::DumpDirectiveLabel(std::ostream &os = std::cout) const
{
	os << "\n";
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
	BrigEntry::dumpIndent(os);
	misc::warning("Unsupport directive %s", "LabelTagets");
}
void BrigDirEntry::DumpDirectiveLoc(std::ostream &os = std::cout) const
{
	BrigEntry::dumpIndent(os);
	misc::warning("Unsupport directive %s", "Loc");
}
void BrigDirEntry::DumpDirectivePragma(std::ostream &os = std::cout) const
{
	BrigEntry::dumpIndent(os);
	misc::warning("Unsupport directive %s", "Pragma");
}
void BrigDirEntry::DumpDirectiveSampler(std::ostream &os = std::cout) const
{
	BrigEntry::dumpIndent(os);
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
	BrigEntry::dumpIndent(os);
	misc::warning("Unsupport directive %s", "Scope");
}
void BrigDirEntry::DumpDirectiveSignature(std::ostream &os = std::cout) const
{
	BrigEntry::dumpIndent(os);
	misc::warning("Unsupport directive %s", "Signature");
}
void BrigDirEntry::DumpDirectiveVariable(std::ostream &os = std::cout) const
{
	BrigEntry::dumpIndent(os);
	struct BrigDirectiveVariable *dir
		= (struct BrigDirectiveVariable *)this->base;
	BrigEntry::dumpSymDecl(this, os);
	if(dir->init)
	{
		os << " = ";
		SymbolModifier modifier(dir->modifier.allBits);
		if(modifier.isArray()) os << "{";
		BrigDirEntry initDir(
			BrigDirEntry::GetDirByOffset(file, dir->init), file
		);
		initDir.Dump();	
		if(modifier.isArray()) os << "}";
		/*
		struct BrigDirectiveVariableInit *init = (struct BrigDirectiveVariableInit *)GetDirByOffset(this->file, var->init);
		if(init->elementCount == 0)
			os << ";\n";
		else
		{
			/// assert(var->type == init->type);
			/// The assembler provided by the HSA foundation might be incorrect to cause this unmatched type.
			os << " = ";
			BrigEntry::dumpValueList(init->data, init->type, init->elementCount, this->file, os);
		}
		*/
		os << ";\n";
	}else
	{
		os << ";\n";
	}
}

void BrigDirEntry::DumpDirectiveVariableInit(std::ostream &os = std::cout) const
{
	struct BrigDirectiveVariableInit *dir = 
		(struct BrigDirectiveVariableInit *) this->base;
	dumpValueList(dir->data, dir->type, dir->elementCount, this->file, os);
}

void BrigDirEntry::DumpDirectiveVersion(std::ostream &os = std::cout) const
{
	dumpIndent(os);
	struct BrigDirectiveVersion *dir 
		= (struct BrigDirectiveVersion *)this->base;
	os << misc::fmt("version %d:%d:", dir->hsailMajor, dir->hsailMinor);
	os << BrigEntry::profile2str(dir->profile);
	os << ":";
	os << BrigEntry::machineModel2str(dir->machineModel);
	os << ";";
	os << misc::fmt(
			" // BRIG Object Format Version %d:%d",
			dir->brigMajor, dir->brigMinor);
	os << "\n";
}

char *BrigDirEntry::GetDirByOffset(BrigFile *file, BrigDirectiveOffset32_t offset)
{
	BrigSection *sec = file->getBrigSection(BrigSectionDirective);
	char *buf = (char *)sec->getBuffer();
	buf += offset;
	return buf;
}
}
