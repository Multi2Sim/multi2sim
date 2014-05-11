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
public:
	/// Constructor
	BrigOperandEntry(char *buf, BrigFile *file, const BrigInstEntry* inst);
	
	/// Returns the kind field of an operand
	unsigned short getKind() const;

	/// Definition of functions to dump operands
	typedef void (BrigOperandEntry::*DumpOperandFn)(std::ostream &os) const;

	/// Definition of functions to dump immed operands
	typedef void (BrigOperandEntry::*DumpImmedFn)(unsigned char *ptr, std::ostream &os) const;

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

	/// Map of the functions for dumping immed
	static std::map<int, DumpImmedFn> dump_immed_fn;

	/// Dumps the immed operand according to the inst type
	void dumpImmedNONE(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedU8(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedU16(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedU32(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedU64(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedS8(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedS16(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedS32(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedS64(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedF16(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedF32(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedF64(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedB1(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedB8(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedB16(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedB32(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedB64(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedB128(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedSAMP(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedROIMG(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedRWIMG(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedFBAR(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedU8X4(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedU8X8(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedU8X16(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedU16X2(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedU16X4(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedU16X8(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedU32X2(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedU32X4(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedU64X2(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedS8X4(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedS8X8(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedS8X16(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedS16X2(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedS16X4(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedS16X8(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedS32X2(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedS32X4(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedS64X2(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedF16X2(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedF16X4(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedF16X8(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedF32X2(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedF32X4(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedF64X2(unsigned char *ptr, std::ostream &os) const;
	void dumpImmedUnsupported(std::ostream &os) const;

	/// Returns the pointer to the operand 
	static char *GetOperandBufferByOffset(BrigFile *file, unsigned int offset);
	
};

} // end namespace HSA

#endif
