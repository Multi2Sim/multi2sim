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

%{

//#include <stdio.h>
//#include <stdlib.h>

#include <cassert>
#include <cstring>

//#include <arch/southern-islands/asm/Arg.h>
#include <arch/southern-islands/asm/Binary.h>
#include <lib/cpp/ELFWriter.h>
#include <lib/cpp/Misc.h>
#include <lib/cpp/Error.h>

//#include "Arg.h"
#include "InternalBinary.h"
#include "Metadata.h"
#include "Binary.h"
#include "Context.h"
#include "Symbol.h"
#include "Task.h"


#define YYERROR_VERBOSE


%}

%code requires {
	#include <string>
	#include <vector>
	#include <arch/southern-islands/asm/Arg.h>
	#include "Argument.h"
	#include "Instruction.h"
	#include "Context.h"
}


%union {
	int num;
	float num_float;
	char *id;
	si2bin::Instruction *inst;
	struct si_label_t *label;
	si2bin::Argument *arg;
	SI::Arg *si_arg;
	std::vector<si2bin::Argument *> *list;
}

 
%destructor { free $$; } <id>
%destructor { delete $$; } <list>


%token<id> TOK_SCALAR_REGISTER
%token<id> TOK_VECTOR_REGISTER
%token<id> TOK_SPECIAL_REGISTER
%token<id> TOK_MEMORY_REGISTER
%token<num> TOK_DECIMAL
%token<id> TOK_HEX
%token<num_float> TOK_FLOAT
%right<id> TOK_ID
%right TOK_COMMA
%right TOK_COLON
%token TOK_FORMAT
%token TOK_OFFEN
%token TOK_IDXEN
%token TOK_OFFSET
%left TOK_OBRA
%token TOK_CBRA
%token TOK_OPAR
%token TOK_CPAR
%token TOK_NEW_LINE
%left TOK_AMP
%token TOK_ABS
%token TOK_NEG
%token TOK_STAR
%token TOK_EQ
%token<id>  TOK_UAV
%token TOK_HL
%token TOK_GLOBAL
%token TOK_METADATA
%token TOK_ARGS
%token TOK_DATA
%token TOK_TEXT
%token TOK_CONST
%token TOK_INT_DECL
%token TOK_SHORT_DECL
%token TOK_FLOAT_DECL
%token TOK_HALF_DECL
%token TOK_WORD_DECL
%token TOK_BYTE_DECL

%type<inst> instr
%type<list> arg_list
%type<arg> operand
%type<arg> arg
%type<arg> maddr_qual
%type<arg> waitcnt_elem
%type<arg> waitcnt_arg
%type<si_arg> val_stmt_list
%type<si_arg> ptr_stmt_list
%type<num> hex_or_dec_value

%%


program
	: kernel_list
	| TOK_NEW_LINE kernel_list
	;

kernel_list
	: kernel
	| kernel kernel_list
	;

kernel
	: global_section section_list
	;

section_list
	: section
	| section section_list
	;

section
	: metadata_section
	| data_section
	| args_section
	| text_section
	{
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();

		// Process tasks and reset list
		context->TaskProcess();

		// Reset symbol table
		context->SymbolTableClear();
	}
	;

global_section
	: global_header
	| global_header TOK_NEW_LINE
	;

global_header
	: TOK_GLOBAL TOK_ID
	{
		int unique_id;

		std::string kernel_name($2);
		
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();

		//Get outer binary from context
		si2bin::OuterBin *outer_bin = context->getOuterBin();
		
		// Create new objects for each kernel
		context->setInnerBin(outer_bin->newInnerBin(kernel_name));
		context->setMetadata(outer_bin->newMetadata());
		context->setEntry(context->getInnerBin()->newEntry());
		context->setTextBuffer(context->getEntry()->getTextSectionBuffer());

		// Add objects and values to over all elf (outer_bin)
		context->getMetadata()->setUniqueId(context->getUniqueId());

		// Increment id
		unique_id = context->getUniqueId();
		unique_id++;
		context->setUniqueId(unique_id);


	} TOK_NEW_LINE
	;



metadata_section
	: metadata_header
	| metadata_header metadata_stmt_list
	;

metadata_header
	: TOK_METADATA TOK_NEW_LINE
	;

metadata_stmt_list
	: metadata_stmt
	| metadata_stmt metadata_stmt_list
	;

metadata_stmt
	: TOK_ID TOK_EQ hex_or_dec_value
	{
		
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();
		
		// Find memory information and store it in metadata
		if (!strcmp($1, "uavprivate"))
		{
			context->getMetadata()->setUAVPrivate($3);
		}
		else if (!strcmp($1, "hwregion"))
		{	
			context->getMetadata()->setHWRegion($3);
		}
		else if (!strcmp($1, "hwlocal"))
		{
			context->getMetadata()->setHWLocal($3);
		}
		else if (!strcmp($1, "userElementCount"))
		{
			throw misc::Error("User has provided 'userElementCount' but this number is automatically calculated");
		}
		else if (!strcmp($1, "NumVgprs"))
		{	
			throw misc::Error("User has provided 'NumVgprs' but this number is automatically calculated");
		}
		else if (!strcmp($1, "NumSgprs"))
		{
			throw misc::Error("User has provided 'NumSgprs' but this number is automatically calculated");
		}
		else if (!strcmp($1, "FloatMode"))
		{
			context->getInnerBin()->setFloatMode($3);
		}
		else if (!strcmp($1, "IeeeMode"))
		{	
			context->getInnerBin()->setIeeeMode($3);
		}
		else if (!strcmp($1, "COMPUTE_PGM_RSRC2"))
		{
			throw misc::Error("User has provided 'COMPUTE_PGM_RSRC2' but this number is automatically calculated from provided PGM_RSRC2 fields");
		}
		else if (!strcmp($1, "COMPUTE_PGM_RSRC2"))
		{
			throw misc::Error("User has provided 'COMPUTE_PGM_RSRC2' but this number is automatically calculated from provided PGM_RSRC2 fields");
		}
		else
		{
			si2bin_yyerror_fmt("Unrecognized assignment: %s", $1);
		}
		
	} TOK_NEW_LINE
	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_CBRA TOK_EQ TOK_ID TOK_COMMA TOK_DECIMAL TOK_COMMA TOK_ID TOK_OBRA TOK_DECIMAL TOK_COLON TOK_DECIMAL TOK_CBRA
	{
		bool err;
		
		// Get context instance
		si2bin::Context *context = si2bin::Context::getInstance();

		//User Elements should be in scalar registers
		if (strcmp($10, "s"))
			si2bin_yyerror_fmt("User Elements should be stored in scalar registers");

		// check for correct syntax
		if (strcmp($1, "userElements"))
			si2bin_yyerror_fmt("User Elements not correctly specified: %s", $1);
		
		// Make sure userElement index is in correct range
		if ($3 > 15 || $3 < 0)
			si2bin_yyerror_fmt("User Elements index is out of allowed range (0 to 15)");

		// Create userElement object
		context->getInnerBin()->newUserElement($3, 
			SI::binary_user_data_map.MapString($6, err),
			$8,
			$12,
			$14 - $12 + 1);

		if (err)
			si2bin_yyerror_fmt("Unrecognized data class: %s", $6);


	} TOK_NEW_LINE
	| TOK_ID TOK_COLON TOK_ID TOK_EQ hex_or_dec_value
	{
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();

		SI::BinaryComputePgmRsrc2 *pgm_rsrc2 = 
			context->getInnerBin()->getPgmRsrc2();
		
		// Find pgm_rsrc2 information
		if (strcmp($1, "COMPUTE_PGM_RSRC2"))
			si2bin_yyerror_fmt("Unrecognized assignment: %s", $1);
		
		if (!strcmp($3, "SCRATCH_EN"))
		{
			pgm_rsrc2->scrach_en = $5;
		}
		else if (!strcmp($3, "USER_SGPR"))
		{
			pgm_rsrc2->user_sgpr = $5;
		}
		else if (!strcmp($3, "TRAP_PRESENT"))
		{
			pgm_rsrc2->trap_present = $5;
		}
		else if (!strcmp($3, "TGID_X_EN"))
		{
			pgm_rsrc2->tgid_x_en = $5;
		}
		else if (!strcmp($3, "TGID_Y_EN"))
		{
			pgm_rsrc2->tgid_y_en = $5;
		}
		else if (!strcmp($3, "TGID_Z_EN"))
		{
			pgm_rsrc2->tgid_z_en = $5;
		}
		else if (!strcmp($3, "TG_SIZE_EN"))
		{
			pgm_rsrc2->tg_size_en = $5;
		}
		else if (!strcmp($3, "TIDIG_COMP_CNT"))
		{
			pgm_rsrc2->tidig_comp_cnt = $5;
		}
		else if (!strcmp($3, "EXCP_EN_MSB"))
		{
			pgm_rsrc2->excp_en_msb = $5;
		}
		else if (!strcmp($3, "LDS_SIZE"))
		{
			pgm_rsrc2->lds_size = $5;
		}
		else if (!strcmp($3, "EXCP_EN"))
		{
			pgm_rsrc2->excp_en = $5;
		}
		else
		{
			si2bin_yyerror_fmt("Unrecognized field of COMPUTE_PGM_RSRC2: %s", $3);
		}


	} TOK_NEW_LINE
	| TOK_NEW_LINE
	;

hex_or_dec_value
	: TOK_DECIMAL
	{
		$$ = $1;
	}
	| TOK_HEX
	{
		int value;
		
		sscanf($1, "%x", &value);

		$$ = value;
	}
	;


data_section
	: data_header
	| data_header data_stmt_list
	;

data_header
	: TOK_DATA TOK_NEW_LINE
	;

data_stmt_list
	: data_stmt
	| data_stmt data_stmt_list
	;

data_stmt
	: TOK_ID TOK_COLON
	| TOK_ID TOK_COLON TOK_NEW_LINE
	| TOK_FLOAT_DECL float_vals
	| TOK_WORD_DECL word_vals
	| TOK_HALF_DECL half_vals
	| TOK_BYTE_DECL byte_vals
	;

float_vals
	: 
	| float_vals TOK_NEW_LINE
	| float_vals TOK_FLOAT TOK_COMMA
	{
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();
		
		context->getOuterBin()->newDataFloat($2);
	}	
	| float_vals TOK_FLOAT
	{
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();
		
		context->getOuterBin()->newDataFloat($2);
	} TOK_NEW_LINE
	;

word_vals
	: 
	| word_vals TOK_NEW_LINE
	| word_vals hex_or_dec_value TOK_COMMA
	{
		unsigned int value = $2;
		
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();
		
		context->getOuterBin()->newDataWord(value);
	}
	| word_vals hex_or_dec_value
	{
		unsigned int value = $2;
		
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();
		
		context->getOuterBin()->newDataWord(value);
	} TOK_NEW_LINE
	;

half_vals
	: 
	| half_vals TOK_NEW_LINE
	| half_vals hex_or_dec_value TOK_COMMA
	{
		unsigned short value = $2;
		
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();
		
		context->getOuterBin()->newDataHalf(value);
	}
	| half_vals hex_or_dec_value
	{ 
		unsigned short value = $2;
		
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();
		
		context->getOuterBin()->newDataHalf(value);
	} TOK_NEW_LINE
	;

byte_vals
	: 
	| byte_vals TOK_NEW_LINE
	| byte_vals hex_or_dec_value TOK_COMMA
	{
		unsigned char value = $2;
		
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();
		
		context->getOuterBin()->newDataByte(value);
	}
	| byte_vals hex_or_dec_value
	{
		unsigned char value = $2;
		
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();
		
		context->getOuterBin()->newDataByte(value);
	} TOK_NEW_LINE
	;

args_section
	: args_header
	| args_header args_stmt_list
	;

args_header
	: TOK_ARGS TOK_NEW_LINE
	;

args_stmt_list
	: args_stmt
	| args_stmt args_stmt_list
	;


args_stmt
	: TOK_ID TOK_ID TOK_DECIMAL val_stmt_list
	{
		SI::Arg *arg = $4;
		SI::ArgValue *arg_val = dynamic_cast<SI::ArgValue *>(arg);
		bool err;

		
		// Set argument name
		arg_val->setName($2);
		
		// Set arg fields
		arg_val->setDataType(
			static_cast<SI::ArgDataType>(
			SI::arg_data_type_map.MapString($1, err)));
		if(err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", $1);

		arg_val->setNumElems(1);
		arg_val->setConstantBufferNum(1);
		arg_val->setConstantOffset($3);
		
	} TOK_NEW_LINE
	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_CBRA TOK_ID TOK_DECIMAL val_stmt_list
	{
		SI::Arg *arg = $7;
		SI::ArgValue *arg_val = dynamic_cast<SI::ArgValue *>(arg);
		bool err;

		// Set argument name
		arg_val->setName($5);
		
		// Set argument fields
		arg_val->setDataType(
			static_cast<SI::ArgDataType>(
			SI::arg_data_type_map.MapString($1, err)));
		if (err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", $1);
		
		arg_val->setNumElems($3);
		arg_val->setConstantBufferNum(1);
		arg_val->setConstantOffset($6);
		
	} TOK_NEW_LINE
	| TOK_ID TOK_STAR TOK_ID TOK_DECIMAL ptr_stmt_list
	{
		SI::Arg *arg = $5;
		SI::ArgPointer *arg_ptr = dynamic_cast<SI::ArgPointer *>(arg);
		bool err;

		// Set new argument name
		arg_ptr->setName($3);
		
		// Initialize argument
		arg_ptr->setNumElems(1);
		arg_ptr->setDataType(
			static_cast<SI::ArgDataType>(
			SI::arg_data_type_map.MapString($1, err)));
		if (err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", $1);

		arg_ptr->setConstantBufferNum(1);
		arg_ptr->setConstantOffset($4);

	} TOK_NEW_LINE
	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_CBRA TOK_STAR TOK_ID TOK_DECIMAL ptr_stmt_list
	{
		SI::Arg *arg = $8;
		SI::ArgPointer *arg_ptr = dynamic_cast<SI::ArgPointer *>(arg);
		bool err;

		// Set new argument name
		arg_ptr->setName($6);
		
		// Initialize argument
		arg_ptr->setNumElems($3);
		arg_ptr->setDataType(
			static_cast<SI::ArgDataType>(
			SI::arg_data_type_map.MapString($1, err)));
		if (err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", $1);
		
		arg_ptr->setConstantBufferNum(1);
		arg_ptr->setConstantOffset($7);

	} TOK_NEW_LINE
	| TOK_NEW_LINE
	;
	
val_stmt_list
	:
	{
		SI::Arg *arg;
		
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();

		/* Create an argument with defaults*/
		arg = context->getMetadata()->newArgValue("arg",
			static_cast<SI::ArgDataType>(0), 0, 0, 0);

		$$ = arg;
	}
	| val_stmt_list TOK_CONST
	{
		SI::Arg *arg = $1;

		// set constarg field to true
		arg->setConstArg(true);
		
		// Return argument
		$$ = arg;
	}
	;

ptr_stmt_list
	:
	{
		SI::Arg *arg;
		
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();

		/* Create an argument with defaults*/
		arg = context->getMetadata()->newArgPointer("arg", 
			static_cast<SI::ArgDataType>(0), 0, 0, 0, 
			SI::ArgScopeUAV, 12, 0, SI::ArgAccessTypeReadWrite);
		$$ = arg;
	}
	| ptr_stmt_list TOK_ID
	{
		SI::Arg *arg = $1;
		SI::ArgPointer *arg_ptr = dynamic_cast<SI::ArgPointer *>(arg);
		bool err;

		// Translate access type
		arg_ptr->setAccessType(
			static_cast<SI::ArgAccessType>(
			SI::arg_access_type_map.MapString($2, err)));
		if (err)
			si2bin_yyerror_fmt("Unrecognized access type: %s", $2);
		
		$$ = arg;
	}
	| ptr_stmt_list TOK_UAV
	{
		SI::Arg *arg = $1;
		SI::ArgPointer *arg_ptr = dynamic_cast<SI::ArgPointer *>(arg);
		
		// Obtain UAV index
		arg_ptr->setScope(SI::ArgScopeUAV);
		arg_ptr->setBufferNum(atoi($2 + 3));

		$$ = arg;
	}
	| ptr_stmt_list TOK_HL
	{
		SI::Arg *arg = $1;
		SI::ArgPointer *arg_ptr = dynamic_cast<SI::ArgPointer *>(arg);
	
		// Set scope to hl
		arg_ptr->setScope(SI::ArgScopeHwLocal);
		arg_ptr->setBufferNum(1);

		// Return argument
		$$ = arg;
	}
	| ptr_stmt_list TOK_CONST
	{
		SI::Arg *arg = $1;
	
		// set constarg field to true
		arg->setConstArg(true);

		// Return argument
		$$ = arg;
	}
	;

text_section
	: text_header
	| text_header text_stmt_list
	;

text_header
	: TOK_TEXT TOK_NEW_LINE
	;

text_stmt_list
	: text_stmt
	| text_stmt text_stmt_list
	;

text_stmt
	: label TOK_NEW_LINE
	| instr
	{
		si2bin::Instruction *inst = $1;
		
		ELFWriter::Buffer *buffer;
		
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();
	
		buffer = context->getEntry()->getTextSectionBuffer();
		std::stringstream &stream =  buffer->getStream();

		// Generate code
		inst->Encode();
		inst->Write(stream);
		//ELFWriterBufferWrite(si2bin_entry->text_section_buffer,
		//		inst->bytes.byte, inst->size);
		
		// Dump Instruction Info
		//si2bin::inst->Dump(os);
	} TOK_NEW_LINE
	| TOK_NEW_LINE
;

label
	: TOK_ID TOK_COLON
	{
		si2bin::Symbol *symbol;
		
		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();

		
		// Check if symbol exists
		symbol = context->getSymbol($1);

		// Create if it does not exists
		if (!symbol)
		{
			symbol = context->newSymbol($1);
		}
		else
		{
			if (symbol->getDefined())
				si2bin_yyerror_fmt("multiply defined label: %s", $1);
		}

		// Define symbol	
		symbol->setDefined(true);
		symbol->setValue(context->getTextBuffer()->getWritePosition());

	}

instr
	: TOK_ID arg_list 
	{
		si2bin::Instruction *inst;
		std::vector<si2bin::Argument *> *arg_list;

		// Get arguments
		arg_list = $2;

		//no arguments - s_endpgm
		if (!arg_list)
			arg_list = new std::vector<si2bin::Argument *>() ;
		
		// Create instruction
		inst = new si2bin::Instruction($1, *arg_list);

		// Return instructions
		$$ = inst;
	}
;

arg_list
	:  // Empty argument list
	{
		$$ = NULL;
	}
	| arg
	{
		std::vector<si2bin::Argument *> *arg_list = new std::vector<si2bin::Argument *>() ;
		
		arg_list->push_back($1);
		$$ = arg_list;
	}
	| arg_list TOK_COMMA arg
	{
		std::vector<si2bin::Argument *> *arg_list = $1;

		// Add argument to head of argument list
		//ListHead(arg_list);
		//ListInsert(arg_list, asObject($1));
		arg_list->push_back($3);
		
		// Return the argument list
		$$ = arg_list;

	}
;


operand
	: TOK_SCALAR_REGISTER
	{	
		int value = atoi($1 + 1);

		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();
		
		$$ = new si2bin::ArgScalarRegister(value);

		if (value >= context->getInnerBin()->getNumSgpr())
			context->getInnerBin()->setNumSgpr(value + 1);
		
	}
	
	| TOK_VECTOR_REGISTER
	{
		int value = atoi($1 + 1);

		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();

		$$ = new si2bin::ArgVectorRegister(value);

		if (value >= context->getInnerBin()->getNumVgpr())
			context->getInnerBin()->setNumVgpr(value + 1);
		
	}
	
	| TOK_SPECIAL_REGISTER
	{
		SI::InstSpecialReg reg;
		reg = static_cast<SI::InstSpecialReg>
			(SI::inst_special_reg_map.MapString($1));
		$$ = new si2bin::ArgSpecialRegister(reg);
	}

	| TOK_MEMORY_REGISTER
	{
		$$ = new si2bin::ArgMemRegister(atoi($1 + 1));
	}
	
	| TOK_DECIMAL
	{
		$$ = new si2bin::ArgLiteral($1);
	}

	| TOK_HEX
	{
		int value;

		sscanf($1, "%x", &value);
		$$ = new si2bin::ArgLiteral(value);
	}

	| TOK_FLOAT
	{
		$$ = new si2bin::ArgLiteralFloat($1);
	}	


arg
	: operand
	{
		$$ = $1;
	}

	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_COLON TOK_DECIMAL TOK_CBRA  
	{
		si2bin::Argument *arg = nullptr;

		int low;
		int high;

		// Get context instance
		si2bin::Context *context;
		context = si2bin::Context::getInstance();
		
		// Read arguments
		low = $3;
		high = $5;
		
		// Initialize
		if (!strcmp($1, "s"))
		{
			arg = new si2bin::ArgScalarRegisterSeries(low, high);
			if (high >= context->getInnerBin()->getNumSgpr())
				context->getInnerBin()->setNumSgpr(high + 1);
		}
		else if (!strcmp($1, "v"))
		{
			arg = new si2bin::ArgVectorRegisterSeries(low, high);
			if (high >= context->getInnerBin()->getNumVgpr())
				context->getInnerBin()->setNumVgpr(high + 1);
		}
		else
		{
			si2bin_yyerror_fmt("invalid register series: %s", $1);
		}
		
		// Return created argument
		$$ = arg;
	}

	| TOK_ABS TOK_OPAR arg TOK_CPAR
	{
		si2bin::Argument *argument = $3;

		// Activate absolute value flag
		argument->setAbs(true);

		// Check valid application of 'abs'
		switch (argument->getType())
		{
		case si2bin::Argument::TypeScalarRegister:
		case si2bin::Argument::TypeScalarRegisterSeries:
		case si2bin::Argument::TypeVectorRegister:
		case si2bin::Argument::TypeVectorRegisterSeries:
			break;

		default:
			si2bin_yyerror_fmt("abs() function not allowed for argument");
		}

		// Return
		$$ = argument;
	}

	| TOK_NEG arg
	{
		si2bin::Argument *argument = $2;

		// Activate absolute value flag
		argument->setNeg(true);

		// Check valid application of 'abs'
		switch (argument->getType())
		{
		case si2bin::Argument::TypeScalarRegister:
		case si2bin::Argument::TypeScalarRegisterSeries:
		case si2bin::Argument::TypeVectorRegister:
		case si2bin::Argument::TypeVectorRegisterSeries:
			break;

		default:
			si2bin_yyerror_fmt("abs() function not allowed for argument");
		}

		// Return
		$$ = argument;
	}

	| operand maddr_qual TOK_FORMAT TOK_COLON TOK_OBRA TOK_ID TOK_COMMA TOK_ID TOK_CBRA
	{
		si2bin::Argument *arg;
		si2bin::Argument *soffset;
		si2bin::ArgMaddrQual *qual;

		char *id_data_format;
		char *id_num_format;
		
		SI::InstBufDataFormat data_format;
		SI::InstBufNumFormat num_format;
		
		bool err;

		// Read arguments
		soffset = $1;
		qual = dynamic_cast<si2bin::ArgMaddrQual *>($2);
		id_data_format = $6;
		id_num_format = $8;
		
		// Data format
		data_format = 
			static_cast<SI::InstBufDataFormat>(
			SI::inst_buf_data_format_map.MapString(id_data_format, err));
		if (err)
			si2bin_yyerror_fmt("%s: invalid data format", id_data_format);
			
		// Number format
		num_format = 
			static_cast<SI::InstBufNumFormat>(
			SI::inst_buf_num_format_map.MapString(id_num_format, err));
		if (err)
			si2bin_yyerror_fmt("%s: invalid number format", id_num_format); 

		// Create argument
		arg = new si2bin::ArgMaddr(soffset, qual, data_format, num_format);	
			
		// Return
		$$ = arg;
	}

	| TOK_ID
	{
		si2bin::Argument *arg;
		
		// Create argument
		arg = new si2bin::ArgLabel($1);

		// Return argument
		$$ = arg;
	}

	| waitcnt_arg
	{
		// The return value is given by 'waitcnt_arg's definition
	}
;

maddr_qual

	: 
	{
		$$ = new si2bin::ArgMaddrQual(false, false, 0);
	}

	| maddr_qual TOK_OFFEN
	{
		si2bin::ArgMaddrQual *qual = 
			static_cast<si2bin::ArgMaddrQual *>($1);

		assert(qual->getType() == si2bin::Argument::TypeMaddrQual);
		if (qual->getOffen())
			si2bin_yyerror_fmt("redundant qualifier 'offen'");
		qual->setOffen(true);
		$$ = qual;
	}

	| maddr_qual TOK_IDXEN
	{
		si2bin::ArgMaddrQual *qual = 
			static_cast<si2bin::ArgMaddrQual *>($1);

		assert(qual->getType() == si2bin::Argument::TypeMaddrQual);
		if (qual->getIdxen())
			si2bin_yyerror_fmt("redundant qualifier 'idxen'");
		qual->setIdxen(true);
		$$ = qual;
	}

	| maddr_qual TOK_OFFSET TOK_COLON TOK_DECIMAL
	{
		si2bin::ArgMaddrQual *qual =
			static_cast<si2bin::ArgMaddrQual *>($1);
		int offset = $4;

		assert(qual->getType() == si2bin::Argument::TypeMaddrQual);
		qual->setOffset(offset);
		// FIXME - check range of 'offset'
		$$ = qual;
	}

waitcnt_arg

	: waitcnt_elem

	| waitcnt_elem TOK_AMP waitcnt_arg
	{
		si2bin::ArgWaitCounter *arg1 =
				dynamic_cast<si2bin::ArgWaitCounter *>($1);
		si2bin::ArgWaitCounter *arg2 =
				dynamic_cast<si2bin::ArgWaitCounter *>($3);
		
		if (arg2->getVmcntActive() && arg1->getVmcntActive())
			si2bin_yyerror_fmt("duplicate 'vmcnt' token");
		arg2->setVmcntActive(arg2->getVmcntActive() + arg1->getVmcntActive());
		arg2->setVmcntValue(arg2->getVmcntValue() + arg1->getVmcntValue());		
		
		if (arg2->getExpcntActive() && arg1->getExpcntActive())
			si2bin_yyerror_fmt("duplicate 'expcnt' token");
		arg2->setExpcntActive(arg2->getExpcntActive() + arg1->getExpcntActive());
		arg2->setExpcntValue(arg2->getExpcntValue() + arg1->getExpcntValue());		
		
		if (arg2->getLgkmcntActive() && arg1->getLgkmcntActive())
			si2bin_yyerror_fmt("duplicate 'lgkmcnt' token");
		arg2->setLgkmcntActive(arg2->getLgkmcntActive() + arg1->getLgkmcntActive());
		arg2->setLgkmcntValue(arg2->getLgkmcntValue() + arg1->getLgkmcntValue());		
		
		delete arg1;
		$$ = arg2;
	}
;

waitcnt_elem

	: TOK_ID TOK_OPAR TOK_DECIMAL TOK_CPAR
	{
		si2bin::ArgWaitCounter *arg;

		// Create argument
		arg = new si2bin::ArgWaitCounter();
		
		if (!strcmp($1, "vmcnt"))
		{
			arg->setVmcntActive(true);
			arg->setVmcntValue($3);
		}
		else if (!strcmp($1, "expcnt"))
		{
			arg->setExpcntActive(true);
			arg->setExpcntValue($3);
		}
		else if (!strcmp($1, "lgkmcnt"))
		{
			arg->setLgkmcntActive(true);
			arg->setLgkmcntValue($3);
		}
		
		// Return
		$$ = arg;
	}
;

%%
