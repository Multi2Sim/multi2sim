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
#include <lib/cpp/Misc.h>
#include "Arg.h"


using namespace misc;

namespace si2bin
{

StringMap arg_type_map =
{
	{ "invalid", ArgTypeInvalid },
	{ "sreg", ArgTypeScalarRegister },
	{ "vreg", ArgTypeVectorRegister },
	{ "sreg_series", ArgTypeScalarRegisterSeries },
	{ "vreg_series", ArgTypeVectorRegisterSeries },
	{ "mreg", ArgTypeMemRegister },
	{ "special_reg", ArgTypeSpecialRegister },
	{ "literal", ArgTypeLiteral },
	{ "literal_reduced", ArgTypeLiteralReduced },
	{ "literal_float", ArgTypeLiteralFloat },
	{ "literal_float_reduced", ArgTypeLiteralFloatReduced },
	{ "waitcnt", ArgTypeWaitCnt },
	{ "label", ArgTypeLabel },
	{ "maddr", ArgTypeMaddr },
	{ "maddr_qual", ArgTypeMaddrQual }
};


/*
 * Class 'Arg'
 */

/* Virtual Destructor */
Arg::~Arg() {}

int Arg::Encode()
{
	panic("%s: cannot encode argument of type %s",
			__FUNCTION__, arg_type_map.MapValue(type));
	return 0;
}


void Arg::ValidTypes(bool types[])
{
	/* If argument type is in array of allowed types, ok. */
	if (types[type])
		return;

	/* Construct error message */
	std::stringstream msg;
	msg << "argument of type " <<
			arg_type_map.MapValue(type)
			<< " found, {";

	/* List allowed types */
	std::string sep = "";
	for (int i = 0; i < ArgTypeCount; i++)
	{
		if (types[i])
		{
			msg << sep << arg_type_map.MapValue(
					(ArgType) i);
			sep = "|";
		}
	}

	/* Message tail */
	msg << "} expected";
	panic("%s", msg.str().c_str());
}




/*
 * Class 'ArgScalarRegister'
 */

int ArgScalarRegister::Encode()
{
	if (inRange(id, 0, 103))
		return id;

	panic("%s: invalid scalar register (s%d)",
			__FUNCTION__, id);
	return 0;
}



/*
 * Class 'ScalarRegisterSeries'
 */

ArgScalarRegisterSeries::ArgScalarRegisterSeries(int low, int high) :
		Arg(ArgTypeScalarRegisterSeries), low(low), high(high)
{
	if (low > high)
		panic("%s: low > high", __FUNCTION__);
}


int ArgScalarRegisterSeries::Encode()
{
	if (inRange(low, 0, 103))
		return low;

	panic("%s: invalid scalar register series (s[%d:%d])",
			__FUNCTION__, low, high);
	return 0;
}




/*
 * Class 'ArgVectorRegister'
 */

int ArgVectorRegister::Encode()
{
	if (inRange(id, 0, 255))
		return id + 256;

	panic("%s: invalid vector register (v%d)",
			__FUNCTION__, id);
	return 0;
}



/*
 * Class 'ArgVectorRegisterSeries'
 */

ArgVectorRegisterSeries::ArgVectorRegisterSeries(int low, int high) :
		Arg(ArgTypeVectorRegisterSeries), low(low), high(high)
{
	if (low > high)
		panic("%s: low > high", __FUNCTION__);
}


int ArgVectorRegisterSeries::Encode()
{
	if (inRange(low, 0, 255))
		return low + 256;

	panic("%s: invalid vector register series (v[%d:%d:])",
			__FUNCTION__, low, high);
	return 0;
}




/*
 * Class 'ArgLiteral'
 */

ArgLiteral::ArgLiteral(int value) :
		Arg(ArgTypeLiteral), value(value)
{
	constant = true;

	/* Detect the special case where the literal constant is in range
	 * [-16..64]. Some instructions can encode these values more
	 * efficiently. Some others even only allow for these values. */
	if (inRange(value, -16, 64))
		type = ArgTypeLiteralReduced;
}


int ArgLiteral::Encode()
{
	if (type != ArgTypeLiteralReduced)
		panic("%s: only valid for ArgTypeLiteralReduced",
				__FUNCTION__);
	
	if (inRange(value, 0, 64))
		return value + 128;
	if (inRange(value, -16, -1))
		return 192 - value;
	
	panic("%s: cannot encode integer constant (%d)",
			__FUNCTION__, value);
	return 0;
}




/*
 * Class 'ArgLiteralFloat'
 */

ArgLiteralFloat::ArgLiteralFloat(float value) :
		Arg(ArgTypeLiteralFloat), value(value)
{
	/* Initialize */
	constant = true;

	/* Detect the special case where the literal float constant can
	 * be encoded in a specific register based on value */
	if (value == 0.5 || value == -0.5 || value == 1.0 || value == -1.0
		|| value == 2.0 || value == -2.0 || value == 4.0
		|| value == -4.0)
		type = ArgTypeLiteralFloatReduced;
}


int ArgLiteralFloat::Encode()
{
	if (type != ArgTypeLiteralFloatReduced)
		panic("%s: only valid for ArgTypeLiteralFloatReduced",
				__FUNCTION__);

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

	panic("%s: cannot encode float constant (%g)",
			__FUNCTION__, value);
	return 0.0;
}



/*
 * Class 'ArgWaitCnt'
 */

ArgWaitCnt::ArgWaitCnt() :
		Arg(ArgTypeWaitCnt)
{
	/* Initialize */
	vmcnt_active = false;
	vmcnt_value = 0;
	lgkmcnt_active = false;
	lgkmcnt_value = 0;
	expcnt_active = false;
	expcnt_value = 0;
}

ArgWaitCnt::ArgWaitCnt(WaitCntType type) :
		Arg(ArgTypeWaitCnt)
{
	/* Initialize */
	vmcnt_value = 0;
	lgkmcnt_value = 0;
	expcnt_value = 0;

	switch (type)
	{

	case WaitCntTypeVmCnt:
		vmcnt_active = true;
		lgkmcnt_active = false;
		expcnt_active = false;
		break;

	case WaitCntTypeLgkmCnt:
		vmcnt_active = false;
		lgkmcnt_active = true;
		expcnt_active = false;
		break;

	case WaitCntTypeExpCnt:
		vmcnt_active = false;
		lgkmcnt_active = false;
		expcnt_active = true;
		break;
	
	default:
		panic("%s: invalid s_waitcnt token (%d)",
			__FUNCTION__, type);
		break;
	}
}

void ArgWaitCnt::Dump(std::ostream &os)
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



/*
 * Class 'ArgMemRegister'
 */

int ArgMemRegister::Encode()
{
	if (!id)
		return 124;

	panic("invalid memory register (m%d)", id);
	return 0;
}



/*
 * Class 'ArgMaddrQual'
 */

void ArgMaddrQual::Dump(std::ostream &os)
{
	if (idxen)
		os << " idxen";
	if (offen)
		os << " offen";
	if (offset)
		os << " offset:" << offset;
}
	


/*
 * Class 'ArgMaddr'
 */

ArgMaddr::ArgMaddr(Arg *soffset, ArgMaddrQual *qual,
		SI::InstBufDataFormat data_format,
		SI::InstBufNumFormat num_format) :
		Arg(ArgTypeMaddr)
{
	/* Initialize */
	this->soffset.reset(soffset);
	this->qual.reset(qual);
	this->data_format = data_format;
	this->num_format = num_format;
}


void ArgMaddr::Dump(std::ostream &os)
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

void ArgSpecialRegister::Dump(std::ostream &os)
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
		panic("%s: unsupported special register (code=%d)",
			__FUNCTION__, reg);
		return 0;
	}
}
	

}  /* namespace si2bin */

