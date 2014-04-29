#ifndef HSA_ASM_BRIGDIRENTRY_H
#define HSA_ASM_BRIGDIRENTRY_H

#include "BrigEntry.h"
#include "BrigDef.h"

namespace HSA{
/// Represents Brig directive entry
class BrigDirEntry : public BrigEntry
{
protected:
public:
	/// Constructor
	BrigDirEntry(char *buf, BrigFile *file);
	
	// Get the kind field from the base
	int getKind() const;

	/// Prototype of functions that dump the directive
	typedef void (BrigDirEntry::*DumpDirectiveFn)(std::ostream &os) const;

	/// Dumps the assembly of the directive
	void Dump(std::ostream &os = std::cout) const
	{
		DumpDirectiveFn fn = BrigDirEntry::dump_dir_fn[this->getKind()];
		(this->*fn)(os);
	}

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(
			std::ostream &os, 
			const BrigDirEntry &dir
		) 
	{
		dir.Dump(os);
		return os;
	}

	/// Returns the pointer to the next same-level directive	
	virtual char *next() const;

	/// Dumps the directive according to its kind value
	void DumpDirectiveArgScopeEnd(std::ostream &os) const;	// 0 
	void DumpDirectiveArgScopeStart(std::ostream &os) const;	// 1
	void DumpDirectiveBlockEnd(std::ostream &os) const;		// 2
	void DumpDirectiveNumeric(std::ostream &os) const;		// 3
	void DumpDirectiveBlockStart(std::ostream &os) const;		// 4
	void DumpDirectiveBlockString(std::ostream &os) const;	// 5
	void DumpDirectiveComment(std::ostream &os) const;		// 6
	void DumpDirectiveControl(std::ostream &os) const;		// 7
	void DumpDirectiveExtension(std::ostream &os) const;		// 8
	void DumpDirectiveFBarrier(std::ostream &os) const;		// 9 
	void DumpDirectiveFile(std::ostream &os) const;			// 10 
	void DumpDirectiveFunction(std::ostream &os) const;		// 11 
	void DumpDirectiveImage(std::ostream &os) const;			// 12 
	void DumpDirectiveImageInit(std::ostream &os) const;		// 13 
	void DumpDirectiveKernel(std::ostream &os) const;			// 14 
	void DumpDirectiveLabel(std::ostream &os) const;			// 15 
	void DumpDirectiveLabelInit(std::ostream &os) const;		// 16 
	void DumpDirectiveLabelTargets(std::ostream &os) const;	// 17 
	void DumpDirectiveLoc(std::ostream &os) const;			// 18 
	void DumpDirectivePragma(std::ostream &os) const;			// 19 
	void DumpDirectiveSampler(std::ostream &os) const;		// 20
	void DumpDirectiveSamplerInit(std::ostream &os) const;	// 21 
	void DumpDirectiveScope(std::ostream &os) const;			// 22 
	void DumpDirectiveSignature(std::ostream &os) const;		// 23 
	void DumpDirectiveVariable(std::ostream &os) const;		// 24 
	void DumpDirectiveVariableInit(std::ostream &os) const;	// 25 
	void DumpDirectiveVersion(std::ostream &os) const;		// 26 

	/// call back function look up table
	static DumpDirectiveFn dump_dir_fn[27];


};
}


#endif
