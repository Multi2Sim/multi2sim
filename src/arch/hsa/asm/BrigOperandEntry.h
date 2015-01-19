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

#ifndef ARCH_HSA_ASM_BRIGOPERANDENTRY_H
#define ARCH_HSA_ASM_BRIGOPERANDENTRY_H

#include <map>

#include "BrigEntry.h"

namespace HSA 
{

class BrigEntry;
class BrigDataEntry;
class BrigCodeEntry;
class BrigSection;

class BrigOperandEntry : public BrigEntry
{

	// Throw kind error
	void KindError(const std::string &str) const;

public:

	/// Constructor
	BrigOperandEntry(const char *buf, const BrigSection *section);

	/// Prototype of functions that dump the entry
	typedef void (BrigOperandEntry::*DumpEntryFn)(BrigTypeX type, 
			std::ostream &os) const;

	/// A binary search tree that maps the kind of the entry to the
	/// dump function
	static std::map<unsigned, DumpEntryFn> dump_entry_fn;

	// Dump the operand
	void Dump(BrigTypeX type = BRIG_TYPE_NONE, 
			std::ostream &os = std::cout) const
	{
		DumpEntryFn fn = BrigOperandEntry::dump_entry_fn[getKind()];
		(this->*fn)(type, os);
	}

	/// Return the kind field of the entry
	BrigKinds getKind() const;

	/// Dump operand functions
	void DumpOperandAddress(BrigTypeX type, std::ostream &os) const;
	void DumpOperandData(BrigTypeX type, std::ostream &os) const;
	void DumpOperandCodeList(BrigTypeX type, std::ostream &os) const;
	void DumpOperandCodeRef(BrigTypeX type, std::ostream &os) const;
	void DumpOperandImageProperties(BrigTypeX type, std::ostream &os) const;
	void DumpOperandOperandList(BrigTypeX type, std::ostream &os) const;
	void DumpOperandReg(BrigTypeX type, std::ostream &os) const;
	void DumpOperandSamplerProperties(BrigTypeX type, std::ostream &os) const;
	void DumpOperandString(BrigTypeX type, std::ostream &os) const;
	void DumpOperandWavesize(BrigTypeX type, std::ostream &os) const;

	/// Dump a list of operand
	void DumpListOfOperand(BrigDataEntry *operands, BrigTypeX type,
			std::ostream& os) const;

	/// Return data bytes of data field
	const unsigned char *getData() const;

	/// Return the symbol field of the operand
	std::unique_ptr<BrigCodeEntry> getSymbol() const;

	/// Return the reg field of the operand
	std::unique_ptr<BrigOperandEntry> getReg() const;

	/// Return register name
	std::string getRegisterName() const;

	/// Return the offset field of the operand
	unsigned long long getOffset() const;

	/// Return the regKind field of the operand
	BrigRegisterKind getRegKind() const;

	/// Return the regNum field of the operand
	unsigned short getRegNumber() const;

	/// Return the string of the operand
	std::string getString() const;

	/// Return the code entry pointed by ref field
	std::unique_ptr<BrigCodeEntry> getRef() const;

	std::unique_ptr<BrigDataEntry> getElements() const;

	/// Return the number of elements 
	unsigned int getElementCount() const;

	/// Return a certain element
	std::unique_ptr<BrigCodeEntry> getElement(unsigned int index) const;

};

}  // namespace HSA

#endif
