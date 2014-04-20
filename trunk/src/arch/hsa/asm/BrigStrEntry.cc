#include "BrigStrEntry.h"

namespace HSA{

BrigStrEntry::BrigStrEntry(char *buf, BrigFile *file)
	: BrigEntry(buf, file)
{
}

unsigned int BrigStrEntry::getByteCount() const
{
	struct BrigString *str = (struct BrigString *)base;
	return str->byteCount;
}

unsigned int BrigStrEntry::getSize() const
{
	unsigned int byteCount = this->getByteCount();
	return (byteCount + 7) * 4 / 4;
}

void BrigStrEntry::Dump(std::ostream &os = std::cout) const
{
	char *temp = (char *)base;
	temp += 4;
	for(unsigned int i=0; i<this->getByteCount(); i++)
	{
		os << *temp;
		temp++;
	}
}

void BrigStrEntry::DumpString(
		BrigFile *file, 
		uint32_t offset,
		std::ostream& os = std::cout)
{
	BrigSection *stringSection = file->getBrigSection(BrigSectionString);
	char *temp = (char *)stringSection->getBuffer();
	temp += offset;
	BrigStrEntry str(temp, file);
	str.Dump(os);
}

std::string BrigStrEntry::getString() const
{
	std::stringstream ss;
	unsigned char *temp = (unsigned char *)base;
	temp += 4;
	for(unsigned int i=0; i<this->getByteCount(); i++)
	{
		ss << *temp;
		temp++;
	}
	return ss.str();
}

std::string BrigStrEntry::GetStringByOffset(
		BrigFile *file,
		uint32_t offset
	)
{
	BrigSection *stringSection = file->getBrigSection(BrigSectionString);
	char *temp = (char *)stringSection->getBuffer();
	temp += offset;
	BrigStrEntry str(temp, file);
	return str.getString();
}
}