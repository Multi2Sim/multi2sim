/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef M2C_SI2BIN_ARGUMENT_H
#define M2C_SI2BIN_ARGUMENT_H

#include <arch/southern-islands/disassembler/Instruction.h>
#include <lib/cpp/Error.h>
#include <lib/cpp/Misc.h>
#include <m2c/common/Argument.h>

#include <cassert>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>


namespace si2bin
{

// Forward declarations
class Instruction;
class Token;



/// Base class representing the argument of an instruction. More specific
/// instruction classes can be derived from this class
class Argument : public comm::Argument
{
public:

	/// Possible argument types
	enum Type
	{
		TypeInvalid = 0,

		TypeScalarRegister,
		TypeVectorRegister,
		TypeScalarRegisterSeries,
		TypeVectorRegisterSeries,
		TypeMemRegister,
		TypeSpecialRegister,
		TypeLiteral,
		TypeLiteralReduced,
		TypeLiteralFloat,
		TypeLiteralFloatReduced,
		TypeWaitCounter,
		TypeLabel,
		TypeMaddr,
		TypeMaddrQual,
		TypePhi,

		TypeCount
	};

	/// String map for Type
	static const misc::StringMap TypeMap;

	/// Possible argument direction
	enum Direction
	{
		DirectionInvalid = 0,

		DirectionSource,
		DirectionDest,
		DirectionOther,

		DirectionCount
	};

	/// String map for Direction
	static const misc::StringMap DirectionMap;

protected:

	// Argument type, determining the actual subclass.
	Type type;

	// Instruction that the argument belongs to, or null if the argument
	// has not been inserted to an instruction yet.
	Instruction *instruction = nullptr;

	// Argument index, populated when inserted into an instruction
	int index = -1;

	// Token associated with argument. This field is populated when an
	// instruction is created that contains the argument as part of its
	// argument list.
	Token *token = nullptr;

	// Absolute value operation applied on the argument
	bool abs = false;

	// Negation operation applied on the argument
	bool neg = false;

	// True if the argument is of a constant type
	bool constant = false;

	// Direction
	Direction direction = DirectionInvalid;

public:

	/// Constructor
	Argument(Type type) : type(type)
	{
	}

	/// Virtual destructor
	virtual ~Argument()
	{
	}

	/// Return the argument type
	Type getType() const { return type; }

	/// Return the token associated with the argument
	Token *getToken() const { return token; }

	/// Return whether an absolute value operation is applied on the
	/// argument.
	bool getAbs() const { return abs; }

	/// Return whether a negation operation is applied on the argument.
	bool getNeg() const { return neg; }

	/// Return whether the argument is of type constant
	bool getConstant() const { return constant; }

	/// Return the index of the argument in an instruction's argument list,
	/// or -1 if the argument has not been inserted yet.
	int getIndex() const { return index; }

	/// Return whether the argument is of a constant type
	bool isConstant() const
	{
		return type == TypeLiteral ||
				type == TypeLiteralReduced ||
				type == TypeLiteralFloat ||
				type == TypeLiteralFloatReduced;
	}

	/// Specify whether an absolute value operation should be applied on the
	/// argument.
	bool setAbs(bool abs) { return this->abs = abs; }

	/// Specify whether a negation operation should be applied on the
	/// argument.
	bool setNeg(bool neg) { return this->neg = neg; }

	/// Set the index that the argument occupies within the instruction that
	/// it belongs to. This function should only be used when the argument
	/// is inserted to an instruction's argument list.
	void setIndex(int index) { this->index = index; }

	/// Set the instruction that the argument belongs to. This function
	/// should only be used when the argument is inserted to an
	/// instruction's argument list.
	void setInstruction(Instruction *instruction)
	{
		this->instruction = instruction;
	}

	/// Specify argument token
	void setToken(Token *token) { this->token = token; }

	/// Pure virtual function to dump the argument. Each child class will
	/// provide its own implementation for this function.
	virtual void Dump(std::ostream &os = std::cout) const = 0;

	/// Alternative syntax for Dump()
	friend std::ostream &operator<<(std::ostream &os,
			Argument &argument)
	{
		argument.Dump(os);
		return os;
	}

	/// Return encoded value representing operands. This operation is valid
	/// only for a certain type of arguments.
	virtual int Encode();

	/// Return whether the argument object is of any of the types passed to
	/// the function. For example, this function could be used like this:
	///
	/// \code
	///     assert(argument->hasValidType(Argument::TypeLiteral,
	///                     Argument::TypeScalarRegister,
	///                     Argument::TypeVectorRegister));
	/// \endcode
	bool hasValidType(bool types[]) { return types[type]; }
	template<typename... T> bool hasValidType(bool types[], Type type,
			T... args)
	{
		assert(misc::inRange(type, 0, TypeCount - 1));
		types[type] = true;
		return hasValidType(types, args...);
	}
	template<typename... T> bool hasValidType(T... args)
	{
		bool types[TypeCount] = { false };
		return hasValidType(types, args...);
	}

	/// Check that the argument object is of any of the types passed to
	/// the function, and throw an exception otherwise.
	/// For example, this function could be used like this:
	///
	/// \code
	///     argument->ValidTypes(Argument::TypeLiteral,
	///                     Argument::TypeScalarRegister,
	///                     Argument::TypeVectorRegister);
	/// \endcode
	template<typename... T> void ValidTypes(T... args)
	{
		if (!hasValidType(std::forward<T>(args)...))
			throw misc::Panic(misc::fmt("Invalid argument type "
					"(%s)", TypeMap[type]));
	}

	/// Specify argument direction
	void setDirection(Direction direction) { this->direction = direction; }

	/// Return argument direction, assert it has been properly set
	Direction getDirection() const
	{
		assert(direction != DirectionInvalid);
		return direction;
	}

	/// Return a list of registers involved in this arguments. By default, no
	/// register is returned. Subclasses can override this function as needed.
	///
	/// \param registers
	///	Existing list where the registers will be appended. The previous content
	///	of the list is not modified. Ownership of the provided registers is kept
	///	internally, and the caller is not responsible for freeing them.
	virtual void getRegisters(std::vector<Argument *> &registers)
	{
	}
};


class ArgScalarRegister : public Argument
{
	// Register number
	int id;

public:

	/// Constructor
	ArgScalarRegister(int id) :
			Argument(TypeScalarRegister),
			id(id)
	{
	}

	/// See Argument::Encode()
	int Encode();

	/// Dump the scalar register
	void Dump(std::ostream &os) const { os << 's' << id; }

	/// Return register number
	int getId() const { return id; }

	/// Set register number
	void setId(int id) { this->id = id; }

	/// See Argument::getRegisters()
	void getRegisters(std::vector<Argument *> &registers)
	{
		registers.push_back(this);
	}
};


class ArgScalarRegisterSeries : public Argument
{
	// First register in the series
	int low;

	// Last register in the series
	int high;

	// Expanded list of scalar registers forming the series
	std::vector<std::unique_ptr<ArgScalarRegister>> registers;

public:

	/// Constructor
	ArgScalarRegisterSeries(int low, int high);

	/// Dump the scalar register series
	void Dump(std::ostream &os) const
	{
		os << "s[" << low << ':' << high << ']';
	}

	/// See Argument::Encode()
	int Encode();

	/// Return the position of the first register
	int getLow() const { return low; }

	/// Return the position of the last register
	int getHigh() const { return high; }

	/// See Argument::getRegisters()
	void getRegisters(std::vector<Argument *> &registers);
};


class ArgVectorRegister : public Argument
{
	// Register number
	int id;

public:

	/// Constructor
	ArgVectorRegister(int id) :
			Argument(TypeVectorRegister),
			id(id)
	{
	}

	/// Dump the vector register
	void Dump(std::ostream &os) const { os << 'v' << id; }

	/// See Argument::Encode()
	int Encode();

	/// Return register number
	int getId() const { return id; }

	/// Set register number
	void setId(int id) { this->id = id; }

	/// See Argument::getRegisters()
	void getRegisters(std::vector<Argument *> &registers)
	{
		registers.push_back(this);
	}
};


class ArgVectorRegisterSeries : public Argument
{
	// First register in the series
	int low;

	// Last register in the series
	int high;

	// Expanded list of scalar registers forming the series
	std::vector<std::unique_ptr<ArgVectorRegister>> registers;

public:

	/// Constructor
	ArgVectorRegisterSeries(int low, int high);

	/// Dump the vector register series
	void Dump(std::ostream &os) const
	{
		os << "v[" << low << ':' << high << ']';
	}

	/// See Argument::Encode()
	int Encode();

	/// Return the position of the first register
	int getLow() const { return low; }

	/// Return the position of the last register
	int getHigh() const { return high; }

	/// See Argument::getRegisters()
	void getRegisters(std::vector<Argument *> &registers);
};


/// Class used both when the argument is of type TypeLiteral and
/// TypeLiteralReduced.
class ArgLiteral : public Argument
{
	// Literal integer value
	int value;

public:

	/// Constructor
	ArgLiteral(int value);

	/// Dump the literal integer value
	void Dump(std::ostream &os) const
	{
		os << "0x" << std::hex << value << std::dec;
	}

	/// See Argument::Encode()
	int Encode();

	/// Return literal integer value
	int getValue() const { return value; }

	/// Set literal integer value
	void setValue(int value) { this->value = value; }
};


/// Class used when the argument is of type TypeLiteralFloat or
/// TypeLiteralFloatReduced
class ArgLiteralFloat : public Argument
{
	// Literal float value
	float value;

public:

	/// Constructor
	ArgLiteralFloat(float value);

	/// Dump literal float value
	void Dump(std::ostream &os) const { os << value; }

	/// See Argument::Encode()
	int Encode();

	/// Return literal float value
	float getValue() const { return value; }
};


class ArgWaitCounter : public Argument
{
	bool vmcnt_active = false;
	int vmcnt_value = 0;

	bool lgkmcnt_active = false;
	int lgkmcnt_value = 0;

	bool expcnt_active = false;
	int expcnt_value = 0;

public:

	/// Possible argument types for the s_waitcnt instruction
	enum CounterType
	{
		CounterTypeInvalid = 0,

		CounterTypeVmCnt,
		CounterTypeLgkmCnt,
		CounterTypeExpCnt,

		CounterTypeCount
	};

	/// String map for CounterType
	static const misc::StringMap CounterTypeMap;
	
	/// 
	ArgWaitCounter(CounterType type = CounterTypeInvalid);

	void Dump(std::ostream &os) const;

	void setVmcntActive(bool active) { vmcnt_active = active; }

	void setVmcntValue(int value) { vmcnt_value = value; }

	void setLgkmcntActive(bool active) { lgkmcnt_active = active; }

	void setLgkmcntValue(int value) { lgkmcnt_value = value; }

	void setExpcntActive(bool active) { expcnt_active = active; }

	void setExpcntValue(int value) { expcnt_value = value; }

	bool getVmcntActive() const { return vmcnt_active; }

	int getVmcntValue() const { return vmcnt_value; }

	bool getLgkmcntActive() const { return lgkmcnt_active; }

	int getLgkmcntValue() const { return lgkmcnt_value; }

	bool getExpcntActive() const { return expcnt_active; }

	int getExpcntValue() const { return expcnt_value; }
};


class ArgMemRegister : public Argument
{
	// Register number
	int id;

public:

	/// Constructor
	ArgMemRegister(int id) :
			Argument(TypeMemRegister),
			id(id)
	{
	}

	/// Dump the memory register
	void Dump(std::ostream &os) const { os << 'm' << id; }

	/// See Argument::Encode()
	int Encode();

	/// Return register number
	int getId() const { return id; }
};


class ArgMaddrQual : public Argument
{
	bool offen;
	bool idxen;
	int offset;

public:

	/// Constructor
	ArgMaddrQual(bool offen, bool idxen, int offset) :
		Argument(TypeMaddrQual),
		offen(offen),
		idxen(idxen),
		offset(offset)
	{
	}

	bool getOffen() const { return offen; }

	bool getIdxen() const { return idxen; }

	int getOffset() const { return offset; }

	bool setOffen(bool offen) { return this->offen = offen; }

	bool setIdxen(bool idxen) { return this->idxen = idxen; }

	int setOffset(int offset) { return this->offset = offset; }

	void Dump(std::ostream &os) const;
};


class ArgMaddr : public Argument
{
	// Sub-argument of type ArgVector, ArgScalar, ArgLiteral,
	// ArgLiteralReduced, ArgLiteralFloat, ArgLiteralFloatReduced.
	std::unique_ptr<Argument> soffset;

	// Sub-argument of type ArgMaddrQual (memory address qualifier)
	std::unique_ptr<ArgMaddrQual> qual;

	SI::Instruction::BufDataFormat data_format;

	SI::Instruction::BufNumFormat num_format;

public:

	ArgMaddr(Argument *soffset, ArgMaddrQual *qual,
			SI::Instruction::BufDataFormat data_format,
			SI::Instruction::BufNumFormat num_format);

	void Dump(std::ostream &os) const;

	Argument *getSoffset() const { return soffset.get(); }

	ArgMaddrQual *getQual() const { return qual.get(); }

	SI::Instruction::BufDataFormat getDataFormat() const { return data_format; }

	SI::Instruction::BufNumFormat getNumFormat() const { return num_format; }
};


class ArgSpecialRegister : public Argument
{
	SI::Instruction::SpecialReg reg;

public:

	ArgSpecialRegister(SI::Instruction::SpecialReg reg) :
			Argument(TypeSpecialRegister),
			reg(reg)
	{
	}

	void Dump(std::ostream &os) const;

	int Encode();

	SI::Instruction::SpecialReg getReg() const { return reg; }
};


class ArgLabel : public Argument
{
	std::string name;

public:

	ArgLabel(const std::string &name) :
			Argument(TypeLabel),
			name(name)
	{
	}

	void Dump(std::ostream &os) const
	{
		os << ' ' << name;
	}

	const std::string &getName() const { return name; }
};


class ArgPhi : public Argument
{
	// Label associated with the Phi argument
	ArgLabel label;

	// Symbol associated with the Phi argument
	std::string symbol;

	// Value associated with the Phi argument. The possible child classes
	// for the argument are:
	// - ArgVectorRegister
	// - ArgScalarRegister
	// - ArgLiteral
	// - ArgLiteralFloat
	std::unique_ptr<Argument> value;

public:

	/// Constructor: create a Phi argument whose label is of ArgLabel class
	ArgPhi(const std::string &label_name):
			Argument(TypePhi),
			label(label_name)
	{
	}

	ArgPhi(const std::string &label_name, const std::string &symbol_name):
			Argument(TypePhi),
			label(label_name),
			symbol(symbol_name)
	{
	}

	/// Return value type. If the value of the Phi argument has not been
	/// set yet, TypeInvalid is returned.
	Type getValueType() const
	{
		return value == nullptr ? TypeInvalid : value->getType();
	}

	/// Return the Phi value interpreted as a scalar register, or `nullptr`
	/// if the type of the Phi value is not scalar register. 
	ArgScalarRegister *getScalarRegister() const
	{ 
	       return dynamic_cast<ArgScalarRegister *>(value.get()); 
	} 

	/// Return the Phi value interpreted as a vector register, or `nullptr`
	/// if the type of the Phi value is not vector register. 
	ArgVectorRegister *getVectorRegister() const
	{ 
	       return dynamic_cast<ArgVectorRegister *>(value.get()); 
	} 

	/// Return the Phi value interpreted as a literal, or `nullptr`
	/// if the type of the Phi value is not literal. 
	ArgLiteral *getLiteral() const
	{ 
	       return dynamic_cast<ArgLiteral *>(value.get()); 
	} 

	/// Return the Phi value interpreted as a literalFloat, or `nullptr`
	/// if the type of the Phi value is not literalFloat. 
	ArgLiteralFloat *getLiteralFloat() const
	{ 
	       return dynamic_cast<ArgLiteralFloat *>(value.get()); 
	} 

	/// Set value to be an ArgScalarRegister pointer
	void setScalarRegister(int id)
	{
		value.reset(new ArgScalarRegister(id));
	}

	/// Set value to be an ArgVectorRegister pointer
	void setVectorRegister(int id)
	{
		value.reset(new ArgVectorRegister(id));
	}

	/// Set value to be a literal (interger)
	void setLiteral(int value)
	{
		this->value.reset(new ArgLiteral(value));
	}

	/// Set value to be a literal (float)
	void setLiteralFloat(float value)
	{
		this->value.reset(new ArgLiteralFloat(value));
	}

	/// Dump ArgPhi
	void Dump(std::ostream &os) const;

	/// Return label
	ArgLabel *getLabel() { return &label; }

	/// Return name of symbol
	const std::string &getSymbol() const { return symbol; }

	/// Return register number if value is ArgVectorRegister or
	/// ArgScalarRegister
	int getId() const;

	/// See Argument::getRegisters()
	void getRegisters(std::vector<Argument *> &registers)
	{
		registers.push_back(value.get());
	}
};

}  // namespace si2bin

#endif
