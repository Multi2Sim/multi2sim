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

#ifndef M2C_SI2BIN_ARG_H
#define M2C_SI2BIN_ARG_H

#include <arch/southern-islands/asm/Inst.h>
#include <lib/cpp/Misc.h>

#include <cassert>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>


namespace si2bin
{

/* Forward declarations */
class Token;


extern misc::StringMap arg_type_map;

/// Possible argument types
enum ArgType
{
	ArgTypeInvalid = 0,

	ArgTypeScalarRegister,
	ArgTypeVectorRegister,
	ArgTypeScalarRegisterSeries,
	ArgTypeVectorRegisterSeries,
	ArgTypeMemRegister,
	ArgTypeSpecialRegister,
	ArgTypeLiteral,
	ArgTypeLiteralReduced,
	ArgTypeLiteralFloat,
	ArgTypeLiteralFloatReduced,
	ArgTypeWaitCnt,
	ArgTypeLabel,
	ArgTypeMaddr,
	ArgTypeMaddrQual,

	ArgTypeCount
};

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
class Arg
{
	friend class Inst;

protected:

	/* Argument type. It determines the sub-class of the actual instance of
	 * type 'Arg'. */
	ArgType type;

	/* Argument index, populated when inserted into an instruction */
	int index;

	/* Token associated with argument. This field is populated when an
	 * instruction is created that contains the argument as part of its
	 * argument list. */
	Token *token;

	/* Absolute value */
	bool abs;

	/* Negation */
	bool neg;

	/* True if the argument is of a constant type */
	bool constant;

public:

	/// Constructor
	Arg(ArgType type) : type(type), index(-1), token(nullptr),
			abs(false), neg(false), constant(false) { }
	
	/// Virtual Destructor
	virtual ~Arg();

	/* Getters */
	ArgType getType() { return type; }
	Token *getToken() { return token; }
	bool getAbs() { return abs; }
	bool getNeg() { return neg; }
	bool getConstant() { return constant; }
	int getIndex() { return index; }

	bool isConstant() { return type == ArgTypeLiteral ||
			type == ArgTypeLiteralReduced ||
			type == ArgTypeLiteralFloat ||
			type == ArgTypeLiteralFloatReduced; }

	/* Setters */
	bool setAbs(bool abs) { return this->abs = abs; }
	bool setNeg(bool neg) { return this->neg = neg; }
	
	/* Dump operand (pure virtual function) */
	virtual void Dump(std::ostream &os) = 0;
	friend std::ostream &operator<<(std::ostream &os, Arg &arg) {
		arg.Dump(os);
		return os;
	}

	/* Return encoded value representing operands. This operation is valid
	 * only for a certain type of arguments. */
	virtual int Encode();

	/* Check that the argument is of any of the types listed in the
	 * arguments. If not, abort the program with a fatal message.
	 * For example, this function could be used like this:
	 *   arg->ValidTypes(ArgTypeLiteral, ArgTypeScalarRegister); */
	void ValidTypes(bool types[]);
	template<typename... T> void ValidTypes(bool types[], ArgType type,
			T... args) {
		assert(misc::inRange(type, 0, ArgTypeCount - 1));
		types[type] = true;
		ValidTypes(types, args...);
	}
	template<typename... T> void ValidTypes(T... args) {
		bool types[ArgTypeCount];
		memset(types, 0, sizeof types);
		ValidTypes(types, args...);
	}
};


class ArgScalarRegister : public Arg
{
	/* Register number */
	int id;
public:
	ArgScalarRegister(int id) : Arg(ArgTypeScalarRegister), id(id) { }

	int Encode();
	void Dump(std::ostream &os) { os << 's' << id; }

	int getId() { return id; }
	void setId(int id) { this->id = id; }
};


class ArgScalarRegisterSeries : public Arg
{
	int low;
	int high;
public:
	ArgScalarRegisterSeries(int low, int high);

	void Dump(std::ostream &os) { os << "s[" << low << ':'
			<< high << ']'; }
	int Encode();

	int getLow() { return low; }
	int getHigh() { return high; }
};


class ArgVectorRegister : public Arg
{
	int id;
public:
	ArgVectorRegister(int id) : Arg(ArgTypeVectorRegister), id(id) { }

	void Dump(std::ostream &os) { os << 'v' << id; }
	int Encode();

	int getId() { return id; }
	void setId(int id) { this->id = id; }
};
	

class ArgVectorRegisterSeries : public Arg
{
	int low;
	int high;
public:
	ArgVectorRegisterSeries(int low, int high);

	void Dump(std::ostream &os) { os << "v[" << low << ':'
			<< high << ']'; }

	int Encode();

	int getLow() { return low; }
	int getHigh() { return high; }
};


/* Class used both when the argument is of type ArgTypeLiteral and
 * ArgTypeLiteralReduced. */
class ArgLiteral : public Arg
{
	int value;
public:
	ArgLiteral(int value);

	void Dump(std::ostream &os) { os << "0x" << std::hex <<
			value << std::dec; }
	int Encode();

	int getValue() { return value; }
	void setValue(int value) { this->value = value; }
};


/* Class used when the argument is of type ArgTypeLiteralFloat or
 * ArgTypeLiteralFloatReduced */
class ArgLiteralFloat : public Arg
{
	float value;
public:
	ArgLiteralFloat(float value);

	void Dump(std::ostream &os) { os << value; }
	int Encode();

	float getValue() { return value; }
};


class ArgWaitCnt : public Arg
{
	bool vmcnt_active;
	int vmcnt_value;

	bool lgkmcnt_active;
	int lgkmcnt_value;

	bool expcnt_active;
	int expcnt_value;
public:
	ArgWaitCnt();
	ArgWaitCnt(WaitCntType type);
	
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


class ArgMemRegister : public Arg
{
	int id;
public:
	ArgMemRegister(int id) : Arg(ArgTypeMemRegister), id(id) { }

	void Dump(std::ostream &os) { os << 'm' << id; }
	int Encode();

	int getId() { return id; }
};


class ArgMaddrQual : public Arg
{
	bool offen;
	bool idxen;
	int offset;
public:
	ArgMaddrQual(bool offen, bool idxen, int offset) :
		Arg(ArgTypeMaddrQual),
		offen(offen),
		idxen(idxen),
		offset(offset) { }

	/* Getters */
	bool getOffen() { return offen; }
	bool getIdxen() { return idxen; }
	int getOffset() { return offset; }
	
	/* Setters */
	bool setOffen(bool offen) { return this->offen = offen; }
	bool setIdxen(bool idxen) { return this->idxen = idxen; }
	int setOffset(int offset) { return this->offset = offset; }

	void Dump(std::ostream &os);
};


class ArgMaddr : public Arg
{
	/* Sub-argument of type ArgVector, ArgScalar, ArgLiteral,
	 * ArgLiteralReduced, ArgLiteralFloat, ArgLiteralFloatReduced. */
	std::unique_ptr<Arg> soffset;

	/* Sub-argument of type ArgMaddrQual (memory address qualifier) */
	std::unique_ptr<ArgMaddrQual> qual;

	SI::InstBufDataFormat data_format;
	SI::InstBufNumFormat num_format;
public:
	ArgMaddr(Arg *soffset, ArgMaddrQual *qual,
			SI::InstBufDataFormat data_format,
			SI::InstBufNumFormat num_format);
	
	void Dump(std::ostream &os);
	
	Arg *getSoffset() { return soffset.get(); }
	ArgMaddrQual *getQual() { return qual.get(); }
	SI::InstBufDataFormat getDataFormat() { return data_format; }
	SI::InstBufNumFormat getNumFormat() { return num_format; }
};


class ArgSpecialRegister : public Arg
{
	SI::InstSpecialReg reg;
public:
	ArgSpecialRegister(SI::InstSpecialReg reg) :
		Arg(ArgTypeSpecialRegister),
		reg(reg) { }

	void Dump(std::ostream &os);
	int Encode();

	SI::InstSpecialReg getReg() { return reg; }
};


class ArgLabel : public Arg
{
	std::string name;
public:
	ArgLabel(const std::string &name) :
		Arg(ArgTypeLabel),
		name(name) { }

	void Dump(std::ostream &os) { os << ' ' << name; }

	const std::string &getName() { return name; }
};


}  /* namespace si2bin */

#endif
