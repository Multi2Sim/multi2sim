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
	int getKind();

	/// Prototype of functions that dumps the directive
	typedef void (BrigDirEntry::*DumpDirectiveFn)(std::ostream &os);

	/// Dumps the assembly of the directive
	void Dump(std::ostream &os = std::cout){
		DumpDirectiveFn fn = BrigDirEntry::dump_dir_fn[this->getKind()];
		(this->*fn)(os);
	}

	/// Dumps the directive according to its kind value
	void DumpDirectiveArgScopeEnd(std::ostream &os);	// 0 
	void DumpDirectiveArgScopeStart(std::ostream &os);	// 1
	void DumpDirectiveBlockEnd(std::ostream &os);		// 2
	void DumpDirectiveNumeric(std::ostream &os);		// 3
	void DumpDirectiveBlockStart(std::ostream &os);		// 4
	void DumpDirectiveBlockString(std::ostream &os);	// 5
	void DumpDirectiveComment(std::ostream &os);		// 6
	void DumpDirectiveControl(std::ostream &os);		// 7
	void DumpDirectiveExtension(std::ostream &os);		// 8
	void DumpDirectiveFBarrier(std::ostream &os);		// 9 
	void DumpDirectiveFile(std::ostream &os);			// 10 
	void DumpDirectiveFunction(std::ostream &os);		// 11 
	void DumpDirectiveImage(std::ostream &os);			// 12 
	void DumpDirectiveImageInit(std::ostream &os);		// 13 
	void DumpDirectiveKernel(std::ostream &os);			// 14 
	void DumpDirectiveLabel(std::ostream &os);			// 15 
	void DumpDirectiveLabelInit(std::ostream &os);		// 16 
	void DumpDirectiveLabelTargets(std::ostream &os);	// 17 
	void DumpDirectiveLoc(std::ostream &os);			// 18 
	void DumpDirectivePragma(std::ostream &os);			// 19 
	void DumpDirectiveSampler(std::ostream &os);		// 20
	void DumpDirectiveSamplerInit(std::ostream &os);	// 21 
	void DumpDirectiveScope(std::ostream &os);			// 22 
	void DumpDirectiveSignature(std::ostream &os);		// 23 
	void DumpDirectiveVariable(std::ostream &os);		// 24 
	void DumpDirectiveVariableInit(std::ostream &os);	// 25 
	void DumpDirectiveVersion(std::ostream &os);		// 26 

	/// call back function look up table
	static DumpDirectiveFn dump_dir_fn[27];


};
}


#endif