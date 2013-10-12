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


using namespace Misc;

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
	{ "maddr_qual", ArgTypeMaddrQual },
	{ 0, 0 }
};


/*
 * Class 'Arg'
 */

Arg::Arg()
{
	type = ArgTypeInvalid;
	constant = false;
	index = -1;
}


int Arg::Encode()
{
	panic("%s: cannot encode argument of type %s",
			__FUNCTION__, StringMapValue(arg_type_map, type));
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
			StringMapValue(arg_type_map, type)
			<< " found, {";

	/* List allowed types */
	std::string sep = "";
	for (int i = 0; i < ArgTypeCount; i++)
	{
		if (types[i])
		{
			msg << sep << StringMapValue(arg_type_map,
					(ArgType) i);
			sep = "|";
		}
	}

	/* Message tail */
	msg << "} expected";
	fatal("%s", msg.str().c_str());
}




/*
 * Class 'ArgScalarRegister'
 */

ArgScalarRegister::ArgScalarRegister(int value)
{
	type = ArgTypeScalarRegister;
	this->id = id;
}


int ArgScalarRegister::Encode()
{
	if (InRange(id, 0, 103))
		return id;

	panic("%s: invalid scalar register (s%d)",
			__FUNCTION__, id);
	return 0;
}



/*
 * Class 'ScalarRegisterSeries'
 */

ArgScalarRegisterSeries::ArgScalarRegisterSeries(int low, int high)
{
	type = ArgTypeScalarRegisterSeries;
	this->low = low;
	this->high = high;
	if (low > high)
		panic("%s: low > high", __FUNCTION__);
}


int ArgScalarRegisterSeries::Encode()
{
	if (InRange(low, 0, 103))
		return low;

	panic("%s: invalid scalar register series (s[%d:%d])",
			__FUNCTION__, low, high);
	return 0;
}




/*
 * Class 'ArgVectorRegister'
 */

ArgVectorRegister::ArgVectorRegister(int value)
{
	type = ArgTypeVectorRegister;
	this->id = id;
}


int ArgVectorRegister::Encode()
{
	if (InRange(id, 0, 255))
		return id + 256;

	panic("%s: invalid vector register (v%d)",
			__FUNCTION__, id);
	return 0;
}



/*
 * Class 'ArgVectorRegisterSeries'
 */

ArgVectorRegisterSeries::ArgVectorRegisterSeries(int low, int high)
{
	type = ArgTypeVectorRegisterSeries;
	this->low = low;
	this->high = high;
	if (low > high)
		panic("%s: low > high", __FUNCTION__);
}


int ArgVectorRegisterSeries::Encode()
{
	if (InRange(low, 0, 255))
		return low + 256;

	panic("%s: invalid vector register series (v[%d:%d:])",
			__FUNCTION__, low, high);
	return 0;
}




/*
 * Class 'ArgLiteral'
 */

ArgLiteral::ArgLiteral(int value)
{
	/* Initialize */
	this->value = value;
	constant = true;

	/* Detect the special case where the literal constant is in range
	 * [-16..64]. Some instructions can encode these values more
	 * efficiently. Some others even only allow for these values. */
	if (InRange(value, -16, 64))
		type = ArgTypeLiteralReduced;
	else
		type = ArgTypeLiteral;
}


int ArgLiteral::Encode()
{
	if (type != ArgTypeLiteralReduced)
		panic("%s: only valid for ArgTypeLiteralReduced",
				__FUNCTION__);
	
	if (InRange(value, 0, 64))
		return value + 128;
	if (InRange(value, -16, -1))
		return 192 - value;
	
	panic("%s: cannot encode integer constant (%d)",
			__FUNCTION__, value);
	return 0;
}




/*
 * Class 'ArgLiteralFloat'
 */

ArgLiteralFloat::ArgLiteralFloat(float value)
{
	/* Initialize */
	this->value = value;
	constant = true;

	/* Detect the special case where the literal float constant can
	 * be encoded in a specific register based on value */
	if (value == 0.5 || value == -0.5 || value == 1.0 || value == -1.0
		|| value == 2.0 || value == -2.0 || value == 4.0
		|| value == -4.0)
		type = ArgTypeLiteralFloatReduced;
	else
		type = ArgTypeLiteralFloat;
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

ArgWaitCnt::ArgWaitCnt()
{
	/* Initialize */
	type = ArgTypeWaitCnt;
	vmcnt_active = false;
	vmcnt_value = 0;
	lgkmcnt_active = false;
	lgkmcnt_value = 0;
	expcnt_active = false;
	expcnt_value = 0;
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

ArgMemRegister::ArgMemRegister(int id)
{
	/* Initialize */
	type = ArgTypeMemRegister;
	this->id = id;
}


int ArgMemRegister::Encode()
{
	if (!id)
		return 124;

	panic("invalid memory register (m%d)", id);
	return 0;
}
	


/*
 * Class 'ArgMaddr'
 */

ArgMaddr::ArgMaddr(Arg *soffset, Arg *qual,
		SI::InstBufDataFormat data_format,
		SI::InstBufNumFormat num_format)
{
	/* Initialize */
	type = ArgTypeMaddr;
	this->soffset.reset(soffset);
	this->qual.reset(qual);
	this->data_format = data_format;
	this->num_format = num_format;
}


void ArgMaddr::Dump(std::ostream &os)
{
	soffset->Dump(os);
	os << ' ';
	qual->Dump(os);
	os << " format:[" << StringMapValue(SI::inst_buf_data_format_map,
			data_format) << ',' <<
			StringMapValue(SI::inst_buf_num_format_map, num_format)
			<< ']';
}



/*
 * Class 'ArgMaddrQual'
 */

ArgMaddrQual::ArgMaddrQual(bool offen, bool idxen, int offset)
{
	/* Initialize */
	type = ArgTypeMaddrQual;
	this->offen = offen;
	this->idxen = idxen;
	this->offset = offset;
}




/*
 * Class 'ArgSpecialRegister'
 */

ArgSpecialRegister::ArgSpecialRegister(SI::InstSpecialReg reg)
{
	/* Initialize */
	type = ArgTypeSpecialRegister;
	this->reg = reg;
}


void ArgSpecialRegister::Dump(std::ostream &os)
{
	os << StringMapValue(SI::inst_special_reg_map, reg);
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
	




/*
 * Class 'ArgLabel'
 */

ArgLabel::ArgLabel(const std::string &name)
{
	/* Initialize */
	type = ArgTypeLabel;
	this->name = name;
}



}  /* namespace si2bin */

