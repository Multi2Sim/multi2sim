#ifndef HSA_ASM_BRIGINSTENTRY_H
#define HSA_ASM_BRIGINSTENTRY_H

#include "BrigEntry.h"
#include "lib/cpp/Misc.h"
#include "lib/cpp/String.h"

namespace HSA
{

/// InstOpcode: I do not use the enumeration fom the BrigDef.h for the 
///  following reasons:
///		1. I need the InstOpcodeCount for the total number of opcodes.
///		2. I forget the second reason.
enum InstOpcode
{
#define DEFINST(_name, _opstr) \
	Inst_##_name,
#include "asm.dat"
#undef DEFINST
	InstOpcodeCount
};

struct InstInfo
{
	InstOpcode opcode;
	const char *name;
	const char *fmt_str;
};

/// Represents a instruction entry in the .code section
class BrigInstEntry : public BrigEntry
{
protected:
	// convert opcode to string
	const char *opcode2str(InstOpcode opcode) const;

	// Convert rouding to string considering the default rouding should be 
	// ignored.
	template <typename BrigInst>
	const char *rounding2str(BrigInst* base) const;

	static misc::StringMap rounding_to_str_map;

	// Convert ALU modifier to string
	const char *modifier2str(unsigned short modifier) const;

	// Pack to string map
	static misc::StringMap pack_to_str_map;

	// Convert pack to string
	const char *pack2str(unsigned char pack) const;

	// Returns true is the instruction has type
	bool hasType() const;

	// Dumps inst's operands
	void dumpOperands(std::ostream &os) const;
public:
	/// Constructor
	BrigInstEntry(char *buf, BrigFile *file);

	/// Returns the type field
	int getKind() const;

	/// Returns the opcode
	int getOpcode() const;

	/// Decode the bytes and prepares the inst_info struct
	/// Not really used
	void Decode();

	/// Prototype of functions that dump the inst
	typedef void (BrigInstEntry::*DumpInstFn)(std::ostream &os) const;

	/// Dumps the assembly of the instruction
	void Dump(std::ostream &os = std::cout) const
	{
		DumpInstFn fn = BrigInstEntry::dump_inst_fn[this->getKind()];
		(this->*fn)(os);
	}

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(
			std::ostream &os, 
			const BrigInstEntry &inst
		) 
	{
		inst.Dump(os);
		return os;
	}
	
	/// Dump functions for different kinds
	void DumpInstNone(std::ostream &os) const;
	void DumpInstBasic(std::ostream &os) const;
	void DumpInstAtomic(std::ostream &os) const;
	void DumpInstAtomicImage(std::ostream &os) const;
	void DumpInstCvt(std::ostream &os) const;
	void DumpInstBar(std::ostream &os) const;
	void DumpInstBr(std::ostream &os) const;
	void DumpInstCmp(std::ostream &os) const;
	void DumpInstFbar(std::ostream &os) const;
	void DumpInstImage(std::ostream &os) const;
	void DumpInstMem(std::ostream &os) const;
	void DumpInstAddr(std::ostream &os) const;
	void DumpInstMod(std::ostream &os) const;
	void DumpInstSeg(std::ostream &os) const;
	void DumpInstSourceType(std::ostream &os) const;
	
	/// Temporarily used, dump unimplemented insts
	void DumpInstUnsupported(const char *kind, std::ostream &os) const;

	//Function list for dump inst. One function for a kind
	static DumpInstFn dump_inst_fn[15];
};

}//end namespace

#endif
