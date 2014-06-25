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

#ifndef HSA_ASM_BRIGOPERANDENTRY_H
#define HSA_ASM_BRIGOPERANDENTRY_H

#include "BrigEntry.h"

namespace HSA
{

class BrigInstEntry;

class BrigOperandEntry : public BrigEntry 
{

protected:

	// The Instruction object that the operand belongs to
	const BrigInstEntry *inst;

	// Index of the operand in the inst
	unsigned char index;

public:

	/// Constructor
	BrigOperandEntry(
		char *buf, BrigFile *file, 
		const BrigInstEntry* inst, unsigned char index
	);

	/// Returns the type of the operand for dumping
	int getOperandType() const;
	
	/// Returns the kind field of an operand
	unsigned short getKind() const;

	/// Definition of functions to dump operands
	typedef void (BrigOperandEntry::*DumpOperandFn)(std::ostream &os) const;

	/// Dump operand
	void Dump(std::ostream &os = std::cout) const
	{
		DumpOperandFn fn = 
			BrigOperandEntry::dump_operand_fn[this->getKind()];
		(this->*fn)(os);
	}

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(std::ostream &os,
			const BrigOperandEntry &operand)
	{
		operand.Dump(os);
		return os;
	}

	/// Functions that dump operands
	void dumpOperandImmed(std::ostream &os) const;
	void dumpOperandWavesize(std::ostream &os) const;
	void dumpOperandReg(std::ostream &os) const;
	void dumpOperandRegVector(std::ostream &os) const;
	void dumpOperandAddress(std::ostream &os) const;
	void dumpOperandLabelRef(std::ostream &os) const;
	void dumpOperandArgumentRef(std::ostream &os) const;
	void dumpOperandArgumentList(std::ostream &os) const;
	void dumpOperandFunctionRef(std::ostream &os) const;
	void dumpOperandFunctionList(std::ostream &os) const;
	void dumpOperandSignatureRef(std::ostream &os) const;
	void dumpOperandFbarrierRef(std::ostream &os) const;
	
	/// Array of functions for dumping operands
	static DumpOperandFn dump_operand_fn[];

	/// Returns the pointer to the operand 
	static char *GetOperandBufferByOffset(BrigFile *file, unsigned int offset);
	
};

}  // namespace HSA

#endif
