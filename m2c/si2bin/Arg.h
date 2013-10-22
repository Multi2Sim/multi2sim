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


extern Misc::StringMap arg_type_map;
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


class Arg
{
	friend class Inst;

	/* Token associated with argument. This field is populated when an
	 * instruction is created that contains the argument as part of its
	 * argument list. */
	Token *token;

	/* Argument index, populated when inserted into an instruction */
	int index;

protected:
	/* Argument type. It determines the sub-class of the actual instance of
	 * type 'Arg'. */
	ArgType type;

	/* Absolute value */
	bool abs;

	/* Negation */
	bool neg;

	/* True if the argument is of a constant type */
	bool constant;

public:

	/* Constructor */
	Arg();
	
	/* Getters */
	ArgType GetType() { return type; }
	bool GetAbs() { return abs; }
	bool GetNeg() { return neg; }
	bool GetConstant() { return constant; }
	int GetIndex() { return index; }

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
		assert(Misc::InRange(type, 0, ArgTypeCount - 1));
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
	ArgScalarRegister(int id);
	int Encode();
	void Dump(std::ostream &os) { os << 's' << id; }
	int GetId() { return id; }
	void SetId(int id) { this->id = id; }
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

	int GetLow() { return low; }
	int GetHigh() { return high; }
};


class ArgVectorRegister : public Arg
{
	int id;
public:
	ArgVectorRegister(int id);

	void Dump(std::ostream &os) { os << 'v' << id; }
	int Encode();
	int GetId() { return id; }
};
	

class ArgVectorRegisterSeries : public Arg
{
	int low;
	int high;
public:
	ArgVectorRegisterSeries(int low, int high);

	void Dump(std::ostream &os) { os << "v[" << low << ':'
			<< high << ']'; }

	int GetLow() { return low; }
	int Encode();
	int GetHigh() { return high; }
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
	int GetValue() { return value; }
	void SetValue(int value) { this->value = value; }
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
	float GetValue() { return value; }
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
	
	void Dump(std::ostream &os);

	void SetVmcntActive(bool active) { vmcnt_active = active; }
	void SetVmcntValue(int value) { vmcnt_value = value; }
	void SetLgkmcntActive(bool active) { lgkmcnt_active = active; }
	void SetLgkmcntValue(int value) { lgkmcnt_value = value; }
	void SetExpcntActive(bool active) { expcnt_active = active; }
	void SetExpcntValue(int value) { expcnt_value = value; }

	bool GetVmcntActive() { return vmcnt_active; }
	int GetVmcntValue() { return vmcnt_value; }
	bool GetLgkmcntActive() { return lgkmcnt_active; }
	int GetLgkmcntValue() { return lgkmcnt_value; }
	bool GetExpcntActive() { return expcnt_active; }
	int GetExpcntValue() { return expcnt_value; }
};


class ArgMemRegister : public Arg
{
	int id;
public:
	ArgMemRegister(int id);

	void Dump(std::ostream &os) { os << 'm' << id; }
	int Encode();
	int GetId() { return id; }
};


class ArgMaddrQual : public Arg
{
	bool offen;
	bool idxen;
	int offset;
public:
	ArgMaddrQual(bool offen, bool idxen, int offset);

	void Dump(std::ostream &os);

	bool GetOffen() { return offen; }
	bool GetIdxen() { return idxen; }
	int GetOffset() { return offset; }
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
	
	Arg *GetSoffset() { return soffset.get(); }
	ArgMaddrQual *GetQual() { return qual.get(); }
	SI::InstBufDataFormat GetDataFormat() { return data_format; }
	SI::InstBufNumFormat GetNumFormat() { return num_format; }
};


class ArgSpecialRegister : public Arg
{
	SI::InstSpecialReg reg;
public:
	ArgSpecialRegister(SI::InstSpecialReg reg);

	void Dump(std::ostream &os);
	int Encode();
	SI::InstSpecialReg GetReg() { return reg; }
};


class ArgLabel : public Arg
{
	std::string name;
public:
	ArgLabel(const std::string &name);
	void Dump(std::ostream &os) { os << ' ' << name; }
	const std::string &GetName() { return name; }
};


}  /* namespace si2bin */

#endif
