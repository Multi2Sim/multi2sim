/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include <assert.h>
#include <stdarg.h>

#include <lib/cpp/Wrapper.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/string.h>

#include "arg.h"
#include "PVars.h"
#include "si2bin.h"


StringMap si2bin_arg_type_map =
{
	{ "invalid", Si2binArgInvalid },
	{ "sreg", Si2binArgScalarRegister },
	{ "vreg", Si2binArgVectorRegister },
	{ "sreg_series", Si2binArgScalarRegisterSeries },
	{ "vreg_series", Si2binArgVectorRegisterSeries },
	{ "mreg", Si2binArgMemRegister },
	{ "special_reg", Si2binArgSpecialRegister },
	{ "literal", Si2binArgLiteral },
	{ "literal_reduced", Si2binArgLiteralReduced },
	{ "literal_float", Si2binArgLiteralFloat },
	{ "literal_float_reduced", Si2binArgLiteralFloatReduced },
	{ "waitcnt", Si2binArgWaitcnt },
	{ "label", Si2binArgLabel },
	{ "maddr", Si2binArgMaddr },
	{ "maddr_qual", Si2binArgMaddrQual },
	{ 0, 0 }
};


void Si2binArgCreate(Si2binArg *self)
{
}


void Si2binArgCreateLiteral(Si2binArg *self, int value)
{
	/* Initialize */
	self->value.literal.val = value;

	/* Detect the special case where the literal constant is in range
	 * [-16..64]. Some instructions can encode these values more
	 * efficiently. Some others even only allow for these values. */
	if (IN_RANGE(value, -16, 64))
		self->type = Si2binArgLiteralReduced;
	else
		self->type = Si2binArgLiteral;
}

void Si2binArgCreateLiteralFloat(Si2binArg *self, float value)
{
	/* Initialize */
	self->value.literal_float.val = value;

	/* Detect the special case where the literal float constant can
	 * be encoded in a specific register based on value */
	if (value == 0.5 || value == -0.5 || value == 1.0 || value == -1.0
		|| value == 2.0 || value == -2.0 || value == 4.0
		|| value == -4.0)
		self->type = Si2binArgLiteralFloatReduced;
	else
		self->type = Si2binArgLiteralFloat;
}


void Si2binArgCreateScalarRegister(Si2binArg *self, int id)
{
	/* Initialize */
	self->type = Si2binArgScalarRegister;
	self->value.scalar_register.id = id;

	if (!IN_RANGE(self->value.scalar_register.id, 0, 255))
		si2bin_yyerror_fmt("scalar register out of range: s%d", id);
}


void Si2binArgCreateScalarRegisterSeries(Si2binArg *self, int low, int high)
{
	/* Initialize */
	self->type = Si2binArgScalarRegisterSeries;
	self->value.scalar_register_series.low = low;
	self->value.scalar_register_series.high = high;
	assert(high >= low);
}


void Si2binArgCreateVectorRegister(Si2binArg *self, int id)
{
	/* Initialize */
	self->type = Si2binArgVectorRegister;
	self->value.vector_register.id = id;

	if (!IN_RANGE(self->value.vector_register.id, 0, 255))
		si2bin_yyerror_fmt("vector register out of range: v%d", id);
}


void Si2binArgCreateVectorRegisterSeries(Si2binArg *self, int low, int high)
{
	/* Initialize */
	self->type = Si2binArgVectorRegisterSeries;
	self->value.vector_register_series.low = low;
	self->value.vector_register_series.high = high;
	assert(high >= low);
}


void Si2binArgCreateSpecialRegister(Si2binArg *self, SIInstSpecialReg reg)
{
	/* Initialize */
	self->type = Si2binArgSpecialRegister;
	self->value.special_register.reg = reg;
}

void Si2binArgCreateMemRegister(Si2binArg *self, int id)
{
	/* Initialize */
	self->type = Si2binArgMemRegister;
	self->value.mem_register.id = id;

	if (self->value.mem_register.id)
		si2bin_yyerror_fmt("memory register can only be m0");
}


void Si2binArgCreateMaddr(Si2binArg *self, Si2binArg *soffset,
		Si2binArg *qual,
		SIInstBufDataFormat data_format,
		SIInstBufNumFormat num_format)
{
	/* Initialize */
	self->type = Si2binArgMaddr;
	self->value.maddr.soffset = soffset;
	self->value.maddr.qual = qual;
	self->value.maddr.data_format = data_format;
	self->value.maddr.num_format = num_format;
}


void Si2binArgCreateMaddrQual(Si2binArg *self)
{
	/* Initialize */
	self->type = Si2binArgMaddrQual;
}


void Si2binArgCreateLabel(Si2binArg *self, char *name)
{
	/* Initialize */
	self->type = Si2binArgLabel;
	self->value.label.name = xstrdup(name);
}


void Si2binArgDestroy(Si2binArg *self)
{
	switch (self->type)
	{

	case Si2binArgMaddr:

		delete(self->value.maddr.soffset);
		delete(self->value.maddr.qual);
		break;

	case Si2binArgLabel:

		free(self->value.label.name);
		break;

	default:
		break;
	}
}


int Si2binArgEncodeOperand(Si2binArg *arg)
{
	switch (arg->type)
	{

	case Si2binArgLiteralReduced:
	{
		int value;

		value = arg->value.literal.val;
		if (IN_RANGE(value, 0, 64))
			return value + 128;
		if (IN_RANGE(value, -16, -1))
			return 192 - value;
		
		si2bin_yyerror_fmt("invalid integer constant: %d", value);
		break;
	}

	case Si2binArgLiteralFloatReduced:
	{
		float value;

		value = arg->value.literal_float.val;
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

		si2bin_yyerror_fmt("invalid float constant: %g", value);
		break;
	}

	case Si2binArgScalarRegister:
	{
		int id;

		id = arg->value.scalar_register.id;
		if (IN_RANGE(id, 0, 103))
			return id;
		
		si2bin_yyerror_fmt("invalid scalar register: s%d", id);
		break;
	}

	/* Encode the low register */
	case Si2binArgScalarRegisterSeries:
	{
		int id;

		id = arg->value.scalar_register_series.low;
		if (IN_RANGE(id, 0, 103))
			return id;
		
		si2bin_yyerror_fmt("invalid scalar register: s%d", id);
		break;
	}

	case Si2binArgVectorRegister:
	{
		int id;

		id = arg->value.vector_register.id;
		if (IN_RANGE(id, 0, 255))
			return id + 256;

		si2bin_yyerror_fmt("invalid vector register: v%d", id);
		break;
	}

	/* Encode the low register */
	case Si2binArgVectorRegisterSeries:
	{
		int id;

		id = arg->value.vector_register_series.low;
		if (IN_RANGE(id, 0, 255))
			return id + 256;

		si2bin_yyerror_fmt("invalid vector register: v%d", id);
		break;
	}

	/* Special register */
	case Si2binArgSpecialRegister:
	{
		switch (arg->value.special_register.reg)
		{
		case SIInstSpecialRegVcc:
			return 106;

		case SIInstSpecialRegExec:
			return 126;

		case SIInstSpecialRegScc:
			return 253;

		default:
			si2bin_yyerror_fmt("%s: unsupported special register (code=%d)",
				__FUNCTION__, arg->value.special_register.reg);
		}
		break;
	}
	
	/* Memory Register */
	case Si2binArgMemRegister:
	{	
		int id;

		id = arg->value.mem_register.id;
		
		if (!id)
			return 124;

		si2bin_yyerror_fmt("invalid memory register: m%d", id);
		break;
	}
	
	default:
		si2bin_yyerror_fmt("invalid operand (code %d)", arg->type);
		break;
	}

	/* Unreachable */
	return 0;
}


void Si2binArgDump(Si2binArg *arg, FILE *f)
{
	switch (arg->type)
	{
	
	case Si2binArgInvalid:

		fprintf(f, "<invalid>");
		break;
		
	case Si2binArgScalarRegister:

		fprintf(f, "<sreg> s%d", arg->value.scalar_register.id);
		break;
		
	case Si2binArgVectorRegister:

		fprintf(f, "<vreg> v%d", arg->value.vector_register.id);
		break;
		
	case Si2binArgScalarRegisterSeries:

		fprintf(f, "<sreg_series> s[%d:%d]",
			arg->value.scalar_register_series.low,
			arg->value.scalar_register_series.high);
		break;
			
	case Si2binArgVectorRegisterSeries:

		fprintf(f, "<vreg_series> v[%d:%d]",
			arg->value.vector_register_series.low,
			arg->value.vector_register_series.high);
		break;
			
	case Si2binArgLiteral:
	{
		int value;

		value = arg->value.literal.val;
		fprintf(f, "<const> %d", value);
		if (value)
			fprintf(f, " (0x%x)", value);
		break;
	}

	case Si2binArgLiteralReduced:
	{
		int value;

		value = arg->value.literal.val;
		fprintf(f, "<const_reduced> %d", value);
		if (value)
			fprintf(f, " (0x%x)", value);
		break;
	}
		
	case Si2binArgLiteralFloat:

		fprintf(f, "<const_float> %g", arg->value.literal_float.val);
		break;
	
	case Si2binArgLiteralFloatReduced:

		fprintf(f, "<const_float_reduced> %g", arg->value.literal_float.val);
		break;

	case Si2binArgWaitcnt:
	{
		char buf[MAX_STRING_SIZE];

		fprintf(f, "<waitcnt>");

		snprintf(buf, sizeof buf, "%d", arg->value.wait_cnt.vmcnt_value);
		fprintf(f, " vmcnt=%s", arg->value.wait_cnt.vmcnt_active ? buf : "x");

		snprintf(buf, sizeof buf, "%d", arg->value.wait_cnt.expcnt_value);
		fprintf(f, " expcnt=%s", arg->value.wait_cnt.expcnt_active ? buf : "x");

		snprintf(buf, sizeof buf, "%d", arg->value.wait_cnt.lgkmcnt_value);
		fprintf(f, " lgkmcnt=%s", arg->value.wait_cnt.lgkmcnt_active ? buf : "x");

		break;
	}

	case Si2binArgSpecialRegister:

		fprintf(f, "<special_reg> %s", StringMapValueWrap(
				si_inst_special_reg_map,
				arg->value.special_register.reg));
		break;
	
	case Si2binArgMemRegister:

		fprintf(f, "<mreg> m%d", arg->value.mem_register.id);
		break;
	
	case Si2binArgMaddr:

		fprintf(f, "<maddr>");

		fprintf(f, " soffs={");
		Si2binArgDump(arg->value.maddr.soffset, f);
		fprintf(f, "}");

		fprintf(f, " qual={");
		Si2binArgDump(arg->value.maddr.qual, f);
		fprintf(f, "}");

		fprintf(f, " dfmt=%s", StringMapValueWrap(
				si_inst_buf_data_format_map,
				arg->value.maddr.data_format));
		fprintf(f, " nfmt=%s", StringMapValueWrap(
				si_inst_buf_num_format_map,
				arg->value.maddr.num_format));

		break;

	case Si2binArgMaddrQual:

		fprintf(f, "offen=%c", arg->value.maddr_qual.offen ? 't' : 'f');
		fprintf(f, " idxen=%c", arg->value.maddr_qual.idxen ? 't' : 'f');
		fprintf(f, " offset=%d", arg->value.maddr_qual.offset);
		break;

	case Si2binArgLabel:
		fprintf(f, "<label>");
		break;
		
	default:
		panic("%s: invalid argument type", __FUNCTION__);
		break;
	}
}


void __Si2binArgValidTypes(Si2binArg *arg, const char *user_message,
		int num_types, ...)
{
	va_list ap;

	char msg[MAX_STRING_SIZE];
	char *msg_ptr;
	char *sep;

	Si2binArgType types[64];

	int msg_size;
	int i;

	/* Maximum number of types */
	if (!IN_RANGE(num_types, 1, 64))
		panic("%s: bad number of types", __FUNCTION__);

	/* Check if argument type if valid */
	va_start(ap, num_types);
	for (i = 0; i < num_types; i++)
	{
		types[i] = va_arg(ap, Si2binArgType);
		if (arg->type == types[i])
			return;
	}

	/* Construct error message */
	msg[0] = '\0';
	msg_ptr = msg;
	msg_size = sizeof msg;
	str_printf(&msg_ptr, &msg_size, "argument of type %s found, {",
			StringMapValue(si2bin_arg_type_map, arg->type));

	/* List allowed types */
	sep = "";
	for (i = 0; i < num_types; i++)
	{
		str_printf(&msg_ptr, &msg_size, "%s%s", sep,
				StringMapValue(si2bin_arg_type_map, types[i]));
		sep = "|";
	}

	/* Message tail */
	str_printf(&msg_ptr, &msg_size, "} expected");
	fatal("%s: %s", user_message, msg);
}


void Si2binArgSwap(Si2binArg **arg1_ptr,
		Si2binArg **arg2_ptr)
{
	Si2binArg *arg3;

	arg3 = *arg1_ptr;
	*arg1_ptr = *arg2_ptr;
	*arg2_ptr = arg3;
}


void Si2binArgDumpAssembly(Si2binArg *arg, FILE *f)
{
	switch (arg->type)
	{
	
	case Si2binArgInvalid:
		break;

	case Si2binArgScalarRegister:
		fprintf(f, "s%d", arg->value.scalar_register.id);
		break;
	
	case Si2binArgVectorRegister:
		fprintf(f, "v%d", arg->value.vector_register.id);
		break;

	case Si2binArgScalarRegisterSeries:
		fprintf(f, "s[%d:%d]", 
			arg->value.scalar_register_series.low,
			arg->value.scalar_register_series.high);
		break;
	
	case Si2binArgVectorRegisterSeries:
		fprintf(f, "v[%d:%d]", 
			arg->value.vector_register_series.low,
			arg->value.vector_register_series.high);
		break;
	
	case Si2binArgLiteral:
	case Si2binArgLiteralReduced:
	{
		int value;
		value = arg->value.literal.val;
			fprintf(f, "0x%x", value);
		break;
	}

	case Si2binArgLiteralFloat:
	case Si2binArgLiteralFloatReduced:
		fprintf(f, "%g", arg->value.literal_float.val);
		break;

	case Si2binArgWaitcnt:
	{
		if(arg->value.wait_cnt.vmcnt_active)
			fprintf(f, "vmcnt(%d)", arg->value.wait_cnt.vmcnt_value);
		else if (arg->value.wait_cnt.lgkmcnt_active)
			fprintf(f, "lgkmcnt(%d)", arg->value.wait_cnt.lgkmcnt_value);
		else if (arg->value.wait_cnt.expcnt_active)
			fprintf(f, "expcnt(%d)", arg->value.wait_cnt.expcnt_value);

		break;
	}

	case Si2binArgSpecialRegister:
		fprintf(f, "%s", StringMapValueWrap(si_inst_special_reg_map,
			arg->value.special_register.reg));
		break;

	case Si2binArgMemRegister:
		fprintf(f, "m%d", arg->value.mem_register.id);
		break;

	case Si2binArgMaddr:

		Si2binArgDumpAssembly(arg->value.maddr.soffset, f);
		fprintf(f, " ");
		Si2binArgDumpAssembly(arg->value.maddr.qual, f);
		fprintf(f, " format:[%s,%s]", 
			StringMapValueWrap(si_inst_buf_data_format_map, arg->value.maddr.data_format),
			StringMapValueWrap(si_inst_buf_num_format_map, arg->value.maddr.num_format));
		
		break;
	
	case Si2binArgMaddrQual:
	{
		if (arg->value.maddr_qual.idxen)
			fprintf(f, "idxen");
		else if(arg->value.maddr_qual.offset)
			fprintf(f, "offset");
		else
			fprintf(f, "offen");
		break;
	}

	case Si2binArgLabel:
	{
		fprintf(f, " %s", arg->value.label.name);
		break;
	}

	default:
		fatal("%s: error - not a valid argument type", __FUNCTION__);
	}
}

