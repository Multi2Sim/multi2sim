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

#ifndef ARCH_HSA_DISASSEMBLER_BRIGCODEENTRY_H
#define ARCH_HSA_DISASSEMBLER_BRIGCODEENTRY_H

#include <iostream>
#include <memory>
#include <map>

#include "Brig.h"
#include "BrigEntry.h"

namespace HSA
{

class BrigEntry;
class BrigSection;
class BrigOperandEntry;
class BrigDataEntry;

/// A BrigCodeEntry is an entry in the hsa_code section. It can 
/// represent a directive, an instruction or an operand
class BrigCodeEntry : public BrigEntry
{
	/// Output a certain number of tabs for indentation
	void DumpIndent(std::ostream &os) const;

	// Throw kind error
	void KindError(const std::string &str) const;
	
public:
	
	/// Constructor
	BrigCodeEntry(const char *buffer) : 
			BrigEntry(buffer)
	{
	}

	/// Destructor
	virtual ~BrigCodeEntry()
	{
	}

	/// Return the kind field of the entry
	BrigKind getKind() const;

	/// Return the size in bytes of the entry
	unsigned getSize() const;

	/// Dump the entry
	void Dump(std::ostream &os = std::cout) const
	{
		DumpEntryFn fn = BrigCodeEntry::dump_entry_fn[getKind()];
		(this->*fn)(os);
	}

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(std::ostream &os,
			const BrigCodeEntry &entry)
	{
		entry.Dump(os);
		return os;
	}

	/// Prototype of functions that dump the entry
	typedef void (BrigCodeEntry::*DumpEntryFn)(std::ostream &os) const;

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
	void DumpDirModule(std::ostream &os) const;
	void DumpDirPragma(std::ostream &os) const;
	void DumpDirSignature(std::ostream &os) const;
	void DumpDirVariable(std::ostream &os) const;

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

	/// A binary search tree that maps the kind of the entry to the 
	/// dump function
	static std::map<unsigned, DumpEntryFn> dump_entry_fn;

	/// Dump symbol declaration
	void DumpSymbolDeclaration(std::ostream &os) const;

	/// Dump argument list
	void DumpArguments(std::unique_ptr<BrigCodeEntry> firstArg,
			unsigned int count, std::ostream &os) const;

	/// Dump function or kernel body
	void DumpBody(std::unique_ptr<BrigCodeEntry> start,
			std::unique_ptr<BrigCodeEntry> end,
			std::ostream &os) const;

	/// Dump operands of an instruction
	void DumpOperands(std::ostream &os) const;

	/// Dump operands for call instructions
	void DumpCallArguments(std::ostream &os) const;

	/// Dump operands for sbr instrucitons
	void DumpSbrArguments(std::ostream &os) const;

	/// Return an unique pointer to the next entry
	std::unique_ptr<BrigCodeEntry> Next() const;

	/// Return an unique pointer to the next top level entry
	std::unique_ptr<BrigCodeEntry> NextTopLevelEntry() const;

	


	//
	// BrigEntry class provide a full banch of getter functions as APIs
	// for other parts of the simulator. It would check is the get is 
	// valid for the entry at first and then returns the value
	//

	/// Return true, if current entry is an instruction
	bool isInstruction() const;

	/// Return true if the instruction use type
	bool useType() const;

	/// Return version major
	unsigned int getHsailMajor() const;

	/// Return version minor
	unsigned int getHsailMinor() const;

	/// Return profile
	BrigProfile getProfile() const;

	/// Return machine model
	BrigMachineModel getMachineModel() const;

	/// Return default rounding
	BrigRound getDefaultFloatRound() const;

	/// Return default width
	BrigWidth getDefaultWidth() const;

	/// Return the width field of an instruction
	BrigWidth getWidth() const;

	/// Return true if instruction is call
	bool isCallInst() const;

	/// Return the string represented by the name field
	std::string getName() const;

	/// Return definition field of symbol modifier
	bool isDefinition() const;

	/// Return true if the variable is const
	bool isConst() const;

	/// Return linkage field
	unsigned char getLinkage() const;

	/// Return allocation field
	BrigAllocation getAllocation() const;

	/// Return segment field
	BrigSegment getSegment() const;

	/// Return true is variable is array
	bool isArray() const;

	/// Return the dim field
	unsigned long long getDim() const;

	/// Return number of output arguments
	unsigned short getOutArgCount() const;

	/// Return number of input arguments
	unsigned short getInArgCount() const;

	/// Return number of entries in a code block
	// unsigned int getCodeBlockEntryCount() const;

	/// Return unique pointer to the BrigEntry of the first input argument
	std::unique_ptr<BrigCodeEntry> getFirstInArg() const;

	/// Return the first entry in the code section of an executable
	std::unique_ptr<BrigCodeEntry> getFirstCodeBlockEntry() const;

	/// Return the pass the last entry of an executable
	std::unique_ptr<BrigCodeEntry> getNextModuleEntry() const;

	/// Return the opcode of an instruction
	BrigOpcode getOpcode() const;

	/// Return the type field of an instruction
	virtual BrigType getType() const;

	/// Return the align field of an entry
	BrigAlignment getAlign() const;

	/// Return the operands list of an instruction
	std::unique_ptr<BrigDataEntry> getOperands() const;

	/// Return the number of operands that an instruction has
	unsigned int getOperandCount() const;

	/// Return unique pointer to the operand
	std::unique_ptr<BrigOperandEntry> getOperand(unsigned char index) const;

	/// Get operand type of current instruction according to the operand 
	/// index
	BrigType getOperandType(unsigned char index) const;

	/// Get compare operation field
	BrigCompareOperation getCompareOperation() const;

	/// Get alu modifier field
	BrigAluModifier getAluModifier() const;

	/// Return source type field
	virtual BrigType getSourceType() const;

	/// Return the round field
	BrigRound getRound() const;

	/// Return the default rounding of an instruction
	BrigRound getDefaultRounding() const;

	/// Return the pack field
	BrigPack getPack() const;

	/// Return the vector modifier
	unsigned getVectorModifier() const;

	/// Return the equivalance class
	unsigned char getEquivClass() const;

	/// Return the atomic operation
	BrigAtomicOperation getAtomicOperation() const;

	/// Return the memory order
	BrigMemoryOrder getMemoryOrder() const;

	/// Return the memory scope
	BrigMemoryScope getMemoryScope() const;

	/// Return the signal operation
	BrigAtomicOperation getSignalOperation() const;

	/// Return the signal type
	BrigType getSignalType() const;

	/// Return the geometry
	BrigImageGeometry getGeometry() const;

	/// Return the image type
	BrigType getImageType() const;

	/// Return the coord type
	BrigType getCoordType() const;

	/// Return the global segment memory scope
	BrigMemoryScope getGlobalSegmentScope() const;

	/// Return the group segment memory scope
	BrigMemoryScope getGroupSegmentScope() const;

	/// Return the image segment memory scope
	BrigMemoryScope getImageSegmentScope() const;

	/// Return the query field for image query instruction
	BrigImageQuery getImageQuery() const;

	/// Return the sampler query
	BrigSamplerQuery getSamplerQuery() const;

	/// Determine is no null 
	bool isNoNull() const;
};

}  // namespace HSA

#endif

