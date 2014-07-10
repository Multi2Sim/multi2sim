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

#include <arch/southern-islands/asm/Inst.h>
#include <lib/cpp/Misc.h>

#include <cassert>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>


namespace si2bin
{

// Forward declarations
class Token;



/// Possible argument types for the s_waitcnt instruction
enum WaitCntType
{
	WaitCntTypeInvalid = 0,

	WaitCntTypeVmCnt,
	WaitCntTypeLgkmCnt,
	WaitCntTypeExpCnt,

	WaitCntTypeCount
};

/// Base class representing the argument of an instruction. More specific
/// instruction classes can be derived from this class
class Argument
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
		TypeWaitCnt,
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

	// FIXME
	friend class Instruction;

	// Argument type, determining the actual subclass.
	Type type;

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

	/// Pure virtual function to dump the argument. Each child class will
	/// provide its own implementation for this function.
	virtual void Dump(std::ostream &os = std::cout) = 0;

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

	/// Check that the argument is of any of the types listed in the
	/// arguments. If not, abort the program with a fatal message.
	/// For example, this function could be used like this:
	///
	/// \code
	///     argument->ValidTypes(Argument::TypeLiteral,
	///                     Argument::TypeScalarRegister,
	///                     Argument::TypeVectorRegister);
	/// \endcode
	void ValidTypes(bool types[]);
	template<typename... T> void ValidTypes(bool types[], Type type,
			T... args)
	{
		assert(misc::inRange(type, 0, TypeCount - 1));
		types[type] = true;
		ValidTypes(types, args...);
	}
	template<typename... T> void ValidTypes(T... args)
	{
		bool types[TypeCount] = { false };
		ValidTypes(types, args...);
	}

	/// Specify argument direction
	void setDirection(Direction direction) { this->direction = direction; }

	/// Return argument direction, assert it has been properly set
	Direction getDirection()
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

	/// see Argument::Encode()
	int Encode();

	/// Dump the scalar register
	void Dump(std::ostream &os) { os << 's' << id; }

	/// Return register number
	int getId() { return id; }

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
	void Dump(std::ostream &os)
	{
		os << "s[" << low << ':' << high << ']';
	}

	int Encode();

	/// Return the position of the first register
	int getLow() { return low; }

	/// Return the position of the last register
	int getHigh() { return high; }

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
	void Dump(std::ostream &os) { os << 'v' << id; }

	/// See Argument::Encode()
	int Encode();

	/// Return register number
	int getId() { return id; }

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
	void Dump(std::ostream &os)
	{
		os << "v[" << low << ':' << high << ']';
	}

	/// See Argument::Encode()
	int Encode();

	/// Return the position of the first register
	int getLow() { return low; }

	/// Return the position of the last register
	int getHigh() { return high; }

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
	void Dump(std::ostream &os)
	{
		os << "0x" << std::hex << value << std::dec;
	}

	/// See Argument::Encode()
	int Encode();

	/// Return literal integer value
	int getValue() { return value; }

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
	void Dump(std::ostream &os) { os << value; }

	/// See Argument::Encode()
	int Encode();

	/// Return literal float value
	float getValue() { return value; }
};


class ArgWaitCnt : public Argument
{
	bool vmcnt_active = false;
	int vmcnt_value = 0;

	bool lgkmcnt_active = false;
	int lgkmcnt_value = 0;

	bool expcnt_active = false;
	int expcnt_value = 0;

public:

	ArgWaitCnt(WaitCntType type = WaitCntTypeInvalid);

	void Dump(std::ostream &os);

	void setVmcntActive(bool active) { vmcnt_active = active; }

	void setVmcntValue(int value) { vmcnt_value = value; }

	void setLgkmcntActive(bool active) { lgkmcnt_active = active; }

	void setLgkmcntValue(int value) { lgkmcnt_value = value; }

	void setExpcntActive(bool active) { expcnt_active = active; }

	void setExpcntValue(int value) { expcnt_value = value; }

	bool getVmcntActive() { return vmcnt_active; }

	int getVmcntValue() { return vmcnt_value; }

	bool getLgkmcntActive() { return lgkmcnt_active; }

	int getLgkmcntValue() { return lgkmcnt_value; }

	bool getExpcntActive() { return expcnt_active; }

	int getExpcntValue() { return expcnt_value; }
};


class ArgMemRegister : public Argument
{
	int id;

public:

	ArgMemRegister(int id) :
			Argument(TypeMemRegister),
			id(id)
	{
	}

	void Dump(std::ostream &os) { os << 'm' << id; }

	int Encode();

	int getId() { return id; }
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

	bool getOffen() { return offen; }

	bool getIdxen() { return idxen; }

	int getOffset() { return offset; }

	bool setOffen(bool offen) { return this->offen = offen; }

	bool setIdxen(bool idxen) { return this->idxen = idxen; }

	int setOffset(int offset) { return this->offset = offset; }

	void Dump(std::ostream &os);
};


class ArgMaddr : public Argument
{
	// Sub-argument of type ArgVector, ArgScalar, ArgLiteral,
	// ArgLiteralReduced, ArgLiteralFloat, ArgLiteralFloatReduced.
	std::unique_ptr<Argument> soffset;

	// Sub-argument of type ArgMaddrQual (memory address qualifier)
	std::unique_ptr<ArgMaddrQual> qual;

	SI::InstBufDataFormat data_format;

	SI::InstBufNumFormat num_format;

public:

	ArgMaddr(Argument *soffset, ArgMaddrQual *qual,
			SI::InstBufDataFormat data_format,
			SI::InstBufNumFormat num_format);

	void Dump(std::ostream &os);

	Argument *getSoffset() { return soffset.get(); }

	ArgMaddrQual *getQual() { return qual.get(); }

	SI::InstBufDataFormat getDataFormat() { return data_format; }

	SI::InstBufNumFormat getNumFormat() { return num_format; }
};


class ArgSpecialRegister : public Argument
{
	SI::InstSpecialReg reg;

public:

	ArgSpecialRegister(SI::InstSpecialReg reg) :
		Argument(TypeSpecialRegister),
		reg(reg)
	{
	}

	void Dump(std::ostream &os);

	int Encode();

	SI::InstSpecialReg getReg() { return reg; }
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

	void Dump(std::ostream &os)
	{
		os << ' ' << name;
	}

	const std::string &getName() { return name; }
};


class ArgPhi : public Argument
{
	// Label associated with the Phi argument
	ArgLabel label;

	// Value associated with the Phi argument. The possible base classes
	// for the argument are:
	// - ArgVectorRegister
	// - ArgScalarRegister
	// - ArgLiteral
	// - ArgLiteralFloat
	std::unique_ptr<Argument> value;

	// Register number
	int id = 0;

public:

	/// Constructor: create a Phi argument whose label is of ArgLabel class
	ArgPhi(const std::string &label_name) :
			Argument(TypePhi),
			label(label_name)
	{
	}

	/// Set value to be an ArgScalarRegister pointer
	void setScalarRegister(int id)
	{
		this->id = id;
		value.reset(new ArgVectorRegister(id));
	}

	/// Set value to be an ArgVectorRegister pointer
	void setVectorRegister(int id)
	{
		this->id = id;
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
	void Dump(std::ostream &os);

	/// Return label name
	const std::string &getName() { return label.getName(); }

	/// Return register number if value is ArgVectorRegister or
	/// ArgScalarRegister
	int getId() { return id; }

	/// See Argument::getRegisters()
	void getRegisters(std::vector<Argument *> &registers)
	{
		registers.push_back(value.get());
	}
};

}  // namespace si2bin

#endif
