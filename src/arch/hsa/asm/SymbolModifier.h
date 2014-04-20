#ifndef HSA_ASM_SYMBOLMODIFIER_H
#define HSA_ASM_SYMBOLMODIFIER_H

namespace HSA{
	/// Represents and encapsulates the symbol modifier
class SymbolModifier
{
protected:
	// The symbol modifier byte
	unsigned char byte;
public:
	/// Constructor
	SymbolModifier(unsigned char byte);

	/// Returns the linkage in number
	unsigned char getLinkage() const;

	/// Returns the linkage string
	const char *getLinkageStr() const;

	/// Return this const in number 0 or 1
	unsigned char getConst() const;

	/// Returns the const string 
	const char *getConstStr() const;
	
	/// Returns true if it is flex array
	bool isFlexArray() const;

	/// Returns true if it is array
	bool isArray() const;

	/// Returns true if it is declaration
	bool isDeclaration() const;
};
}

#endif