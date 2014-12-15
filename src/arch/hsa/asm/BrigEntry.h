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

#ifndef ARCH_HSA_ASM_BRIGENTRY_H
#define ARCH_HSA_ASM_BRIGENTRY_H

#include <iostream>
#include <memory>

namespace HSA
{

class BrigFile;
class BrigSection;

/// A BRIG Entry is an entry in the hsa_code or hsa_operand section. It can 
/// represent a directive, an instruction or an operand
class BrigEntry
{
protected:

	// Pointer to the first byte of the entry
	const char *base;

	// A Pointer to the file that this brig entry belongs to 
	const BrigSection *section;

	// Return the pointer in the file to the entry. This function is set 
	// to be private because the BrigEntry should populates all the fields
	// and directly read from the the binary should be forbidden in the 
	// emulator
	const char *getBuffer() const { return base; }

	/// Output a certain number of tabs for indentation
	void DumpIndent(std::ostream &os) const;

public:
	
	/// Constructor
	BrigEntry(const char *buf, const BrigSection *section);

	/// Return the size in bytes of the entry
	unsigned getSize() const;

	/// Return the kind field of the entry
	unsigned getKind() const;

	/// Get the offset of this entry int the section
	unsigned int getOffset() const;

	/// Return the section that has this entry
	const BrigSection *getSection() const { return section; }

	/// Returns the file that has this entry
	BrigFile *getBinary() const { return section->getBinary(); }

	/// Dump the entry
	void Dump(std::ostream &os = std::cout) const
	{
		DumpEntryFn fn = BrigEntry::dump_entry_fn[getKind()];
		(this->*fn)(os);
	}

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(std::ostream &os,
			const BrigEntry &entry)
	{
		entry.Dump(os);
		return os;
	}

	/// Prototype of functions that dump the entry
	typedef void (BrigEntry::*DumpEntryFn)(std::ostream &os) const;

	/// Dump directive functions
	void DumpDirArgBlockEnd(std::ostream &os) const;
	void DumpDirArgBlockStart(std::ostream &os) const;
	void DumpDirComment(std::ostream &os) const;
	void DumpDirControl(std::ostream &os) const;
	void DumpDirExtension(std::ostream &os) const;
	void DumpDirFbarrier(std::ostream &os) const;
	void DumpDirFunction(std::ostream &os) const;
	void DumpDirIndirectFunction(std::ostream &os) const;
	void DumpDirKernel(std::ostream &os) const;
	void DumpDirLabel(std::ostream &os) const;
	void DumpDirLoc(std::ostream &os) const;
	void DumpDirPragma(std::ostream &os) const;
	void DumpDirSignature(std::ostream &os) const;
	void DumpDirVariable(std::ostream &os) const;
	void DumpDirVersion(std::ostream &os) const;

	/// Dump instruction functions
	void DumpInstAddr(std::ostream &os) const;
	void DumpInstAtomic(std::ostream &os) const;
	void DumpInstBasic(std::ostream &os) const;
	void DumpInstBr(std::ostream &os) const;
	void DumpInstCmp(std::ostream &os) const;
	void DumpInstCvt(std::ostream &os) const;
	void DumpInstImage(std::ostream &os) const;
	void DumpInstLane(std::ostream &os) const;
	void DumpInstMem(std::ostream &os) const;
	void DumpInstMemFence(std::ostream &os) const;
	void DumpInstMod(std::ostream &os) const;
	void DumpInstQueryImage(std::ostream &os) const;
	void DumpInstQuerySampler(std::ostream &os) const;
	void DumpInstQueue(std::ostream &os) const;
	void DumpInstSeg(std::ostream &os) const;
	void DumpInstSegCvt(std::ostream &os) const;
	void DumpInstSignal(std::ostream &os) const;
	void DumpInstSourceType(std::ostream &os) const;
	
	/// Dump operand functions
	void DumpOperandAddress(std::ostream &os) const;
	void DumpOperandData(std::ostream &os) const;
	void DumpOperandCodeList(std::ostream &os) const;
	void DumpOperandCodeRef(std::ostream &os) const;
	void DumpOperandImageProperties(std::ostream &os) const;
	void DumpOperandOperandList(std::ostream &os) const;
	void DumpOperandReg(std::ostream &os) const;
	void DumpOperandSamplerProperties(std::ostream &os) const;
	void DumpOperandString(std::ostream &os) const;
	void DumpOperandWavesize(std::ostream &os) const;

	/// A binary search tree that maps the kind of the entry to the 
	/// dump function
	static std::map<unsigned, DumpEntryFn> dump_entry_fn;

	/// Returns an unique pointer to the next entry
	std::unique_ptr<BrigEntry> next() const;

};

}  // namespace HSA

#endif

