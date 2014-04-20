#include "SymbolModifier.h"
#include "lib/cpp/Misc.h"

namespace HSA{

SymbolModifier::SymbolModifier(unsigned char byte)
{
	this->byte = byte;
}

unsigned char SymbolModifier::getLinkage() const
{
	unsigned char mask = 3;
	return byte & mask;
}

const char *SymbolModifier::getLinkageStr() const
{
	switch(this->getLinkage())
	{
		case 0: return "";
		case 1: return "static";
		case 2: return "extern";
		default: misc::warning("Unsupportd linkage type!");
	}
	return "";
}

unsigned char SymbolModifier::getConst() const
{
	unsigned char mask = 8;
	return !(!(this->byte & mask));
}

const char *SymbolModifier::getConstStr() const
{
	if(this->getConst())
	{
		return "const ";
	}
	return "";
}

bool SymbolModifier::isFlexArray() const
{
	unsigned char mask = 32;
	return !(!(this->byte & mask));
}

bool SymbolModifier::isArray() const
{
	unsigned char mask = 16;
	return !(!(this->byte & mask));
}

bool SymbolModifier::isDeclaration() const
{
	unsigned char mask = 4;
	return !(!(this->byte & mask));
}

}// end namespace