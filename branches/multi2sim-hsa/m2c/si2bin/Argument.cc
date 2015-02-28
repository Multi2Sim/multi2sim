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

#include <sstream>

#include <lib/cpp/Error.h>
#include <lib/cpp/Misc.h>

#include "Argument.h"
#include "Context.h"


namespace si2bin
{

const misc::StringMap Argument::TypeMap =
{
	{ "invalid",			TypeInvalid },
	{ "sreg",			TypeScalarRegister },
	{ "vreg",			TypeVectorRegister },
	{ "sreg_series",		TypeScalarRegisterSeries },
	{ "vreg_series",		TypeVectorRegisterSeries },
	{ "mreg",			TypeMemRegister },
	{ "special_reg",		TypeSpecialRegister },
	{ "literal",			TypeLiteral },
	{ "literal_reduced",		TypeLiteralReduced },
	{ "literal_float",		TypeLiteralFloat },
	{ "literal_float_reduced",	TypeLiteralFloatReduced },
	{ "waitcnt",			TypeWaitCounter },
	{ "label",			TypeLabel },
	{ "maddr",			TypeMaddr },
	{ "maddr_qual",			TypeMaddrQual }
};

const misc::StringMap Argument::DirectionMap =
{
	{ "invalid",			DirectionInvalid },
	{ "source",			DirectionSource },
	{ "dest",			DirectionDest }
};


int Argument::Encode()
{
	throw misc::Panic(misc::fmt("Cannot encode argument of type '%s'",
			TypeMap[type]));
}



//
// Class 'ArgScalarRegister'
//

int ArgScalarRegister::Encode()
{
	if (misc::inRange(id, 0, 103))
		return id;

	// Invalid
	throw misc::Panic(misc::fmt("Invalid scalar register (s%d)", id));
}



//
// Class 'ScalarRegisterSeries'
//

ArgScalarRegisterSeries::ArgScalarRegisterSeries(int low, int high) :
		Argument(TypeScalarRegisterSeries),
		low(low),
		high(high)
{
	// Verify range
	if (low > high)
		throw misc::Panic("low > high");
}


int ArgScalarRegisterSeries::Encode()
{
	// Valid values
	if (misc::inRange(low, 0, 103))
		return low;

	// Invalid
	throw misc::Panic(misc::fmt("Invalid scalar register series (s[%d:%d])",
			low, high));
}


void ArgScalarRegisterSeries::getRegisters(std::vector<Argument *> &registers)
{
	// Create the register components if they don't exist yet
	if (this->registers.empty())
	{
		for (int i = low; i < high; i++)
			this->registers.emplace_back(new ArgScalarRegister(i));
	}

	// Append registers to the output list
	for (auto &single_register : this->registers)
		registers.push_back(single_register.get());
}



//
// Class 'ArgVectorRegister'
//

int ArgVectorRegister::Encode()
{
	// Valid values
	if (misc::inRange(id, 0, 255))
		return id + 256;

	// Invalid
	throw misc::Panic(misc::fmt("Invalid vector register (v%d)", id));
}



//
// Class 'ArgVectorRegisterSeries'
//

ArgVectorRegisterSeries::ArgVectorRegisterSeries(int low, int high) :
		Argument(TypeVectorRegisterSeries),
		low(low),
		high(high)
{
	// Verify range
	if (low > high)
		throw misc::Panic("low > high");
}


int ArgVectorRegisterSeries::Encode()
{
	// Valid values
	if (misc::inRange(low, 0, 255))
		return low + 256;

	// Invalid
	throw misc::Panic(misc::fmt("Invalid vector register series "
			"(v[%d:%d:])", low, high));
}



void ArgVectorRegisterSeries::getRegisters(std::vector<Argument *> &registers)
{
	// Create the register components if they don't exist yet
	if (this->registers.empty())
	{
		for (int i = low; i < high; i++)
			this->registers.emplace_back(new ArgVectorRegister(i));
	}

	// Append registers to the output list
	for (auto &single_register : this->registers)
		registers.push_back(single_register.get());
}



//
// Class 'ArgLiteral'
//

ArgLiteral::ArgLiteral(int value) :
		Argument(TypeLiteral),
		value(value)
{
	// This is a constant argument
	constant = true;

	// Detect the special case where the literal constant is in range
	// [-16..64]. Some instructions can encode these values more
	// efficiently. Some others even only allow for these values.
	if (misc::inRange(value, -16, 64))
		type = TypeLiteralReduced;
}


int ArgLiteral::Encode()
{
	// Only reduced literals can be encoded
	if (type != TypeLiteralReduced)
		throw misc::Panic("Only reduced literals can be encoded");
	
	// Positive constants
	if (misc::inRange(value, 0, 64))
		return value + 128;

	// Negative constants
	if (misc::inRange(value, -16, -1))
		return 192 - value;
	
	// Invalid
	throw misc::Panic(misc::fmt("Invalid integer constant (%d)", value));
}




//
// Class 'ArgLiteralFloat'
//

ArgLiteralFloat::ArgLiteralFloat(float value) :
		Argument(TypeLiteralFloat),
		value(value)
{
	// This is a constant argument
	constant = true;

	// Detect the special case where the literal float constant can
	// be encoded in a specific register based on value
	if (value == 0.5 ||
			value == -0.5 ||
			value == 1.0 ||
			value == -1.0 ||
			value == 2.0 ||
			value == -2.0 ||
			value == 4.0 ||
			value == -4.0)
		type = TypeLiteralFloatReduced;
}


int ArgLiteralFloat::Encode()
{
	// Only reduced literals
	if (type != TypeLiteralFloatReduced)
		throw misc::Panic("Only reduced literals can be encoded");

	if (value == 0.5)
		return 240;

	if (value == -0.5)
		return 241;

	if (value == 1.0)
		return 242;

	if (value == -1.0)
		return 243;

	if (value == 2.0)
		return 244;

	if (value == -2.0)
		return 245;

	if (value == 4.0)
		return 246;

	if (value == -4.0)
		return 247;

	// Invalid
	throw misc::Panic(misc::fmt("Cannot encode float constant (%g)",
			value));
}



//
// Class 'ArgWaitCounter'
//

const misc::StringMap ArgWaitCounter::CounterTypeMap =
{
	{ "invalid", CounterTypeInvalid },
	{ "vector memory count", CounterTypeVmCnt },
	{ "LDS, GDS, Kconstant, Message", CounterTypeLgkmCnt },
	{ "VGPR-export count", CounterTypeExpCnt }
};

ArgWaitCounter::ArgWaitCounter(CounterType counter_type) :
		Argument(TypeWaitCounter)
{
	switch (counter_type)
	{

	case CounterTypeVmCnt:

		vmcnt_active = true;
		break;

	case CounterTypeLgkmCnt:

		lgkmcnt_active = true;
		break;

	case CounterTypeExpCnt:

		expcnt_active = true;
		break;
	
	default:

		break;
	}
}


void ArgWaitCounter::Dump(std::ostream &os) const
{
	std::string comma;

	if (vmcnt_active)
	{
		os << comma << "vmcnt(" << vmcnt_value << ')';
		comma = " ";
	}
	if (lgkmcnt_active)
	{
		os << comma << "lgkmcnt(" << lgkmcnt_value << ')';
		comma = " ";
	}
	if (expcnt_active)
	{
		os << comma << "expcnt(" << expcnt_value <<')';
		comma = " ";
	}
}



//
// Class 'ArgMemRegister'
//

int ArgMemRegister::Encode()
{
	// Valid value
	if (id == 0)
		return 124;

	// Invalid
	throw misc::Panic(misc::fmt("Invalid memory register (m%d)", id));
}



//
// Class 'ArgMaddrQual'
//

void ArgMaddrQual::Dump(std::ostream &os) const
{
	if (idxen)
		os << " idxen";
	if (offen)
		os << " offen";
	if (offset)
		os << " offset:" << offset;
}
	


//
// Class 'ArgMaddr'
//

ArgMaddr::ArgMaddr(Argument *soffset, ArgMaddrQual *qual,
		SI::InstBufDataFormat data_format,
		SI::InstBufNumFormat num_format) :
		Argument(TypeMaddr)
{
	// Initialize
	this->soffset.reset(soffset);
	this->qual.reset(qual);
	this->data_format = data_format;
	this->num_format = num_format;
}


void ArgMaddr::Dump(std::ostream &os) const
{
	soffset->Dump(os);
	qual->Dump(os);
	os << " format:[" << SI::inst_buf_data_format_map.MapValue(
			data_format) << ',' <<
			SI::inst_buf_num_format_map.MapValue(num_format)
			<< ']';
}



/*
 * Class 'ArgSpecialRegister'
 */

void ArgSpecialRegister::Dump(std::ostream &os) const
{
	os << SI::inst_special_reg_map.MapValue(reg);
}
	

int ArgSpecialRegister::Encode()
{
	switch (reg)
	{
	case SI::InstSpecialRegVcc:

		return 106;

	case SI::InstSpecialRegExec:

		return 126;

	case SI::InstSpecialRegScc:

		return 253;

	default:

		throw misc::Panic(misc::fmt("Unsupported special register "
				"(code %d)", reg));
	}
}



//
// Class 'ArgPhi'
//

void ArgPhi::Dump(std::ostream &os) const
{
	os << misc::fmt("[ v%d, %s ]", getId(), label.getName().c_str());
}
	
int ArgPhi::getId() const
{
	switch (getValueType())
	{
	case TypeScalarRegister:

		// Return scalar register id 
		ArgScalarRegister * s;
		s = getScalarRegister();
		return s->getId();

	case TypeVectorRegister:

		// Return vector register id 
		return getVectorRegister()->getId();

	default:

		return 0;
	}
}


}  /* namespace si2bin */

