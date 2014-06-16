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
	friend std::ostream &operator<<(
			std::ostream &os, 
			const BrigOperandEntry &operand
		) 
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

} // end namespace HSA

#endif
