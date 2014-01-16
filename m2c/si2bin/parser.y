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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arch/southern-islands/asm/arg.h>
#include <arch/southern-islands/asm/Wrapper.h>
#include <lib/class/class.h>
#include <lib/class/elf-writer.h>
#include <lib/class/hash-table.h>
#include <lib/class/list.h>
#include <lib/class/string.h>
#include <lib/cpp/Wrapper.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "arg.h"
#include "data.h"
#include "inner-bin.h"
#include "inst.h"
#include "metadata.h"
#include "outer-bin.h"
#include "PVars.h"
#include "si2bin.h"
#include "string.h"
#include "symbol.h"
#include "task.h"


#define YYERROR_VERBOSE


%}


%union {
	int num;
	float num_float;
	String *id;
	Si2binInst *inst;
	struct si_label_t *label;
	List *list;
	Si2binArg *arg;
	SIArg *si_arg;
}

 


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
		Si2binTask *task;
		
		/* Process tasks and reset list */
		ListForEach(si2bin->task_list, task, Si2binTask)
			Si2binTaskProcess(task);
		ListDeleteObjects(si2bin->task_list);

		/* Reset symbol table */
		HashTableDeleteObjects(si2bin->symbol_table);
	}
	;

global_section
	: global_header
	| global_header TOK_NEW_LINE
	;

global_header
	: TOK_GLOBAL TOK_ID
	{
		String *id = $2;
		
		/* Create new objects for each kernel */
		si2bin_inner_bin = si2bin_inner_bin_create($2->text);
		si2bin_metadata = si2bin_metadata_create();
		si2bin_entry = si2bin_inner_bin_entry_create();
		
		/* Add objects and values to over all elf (outer_bin) */
		si2bin_metadata->uniqueid = si2bin_uniqueid;
		si2bin_inner_bin_add_entry(si2bin_inner_bin, si2bin_entry);
		si2bin_outer_bin_add(si2bin_outer_bin, si2bin_inner_bin, si2bin_metadata);

		/* Increment id */
		si2bin_uniqueid ++;

		/* Free id */
		delete(id);
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
		String *id = $1;
		
		/* Find memory information and store it in metadata */
		if (!strcmp("uavprivate", id->text))
		{
			si2bin_metadata->uavprivate = $3;
		}
		else if (!strcmp("hwregion", id->text))
		{	
			si2bin_metadata->hwregion = $3;
		}
		else if (!strcmp("hwlocal", id->text))
		{
			si2bin_metadata->hwlocal = $3;
		}
		else if (!strcmp("userElementCount", id->text))
		{
			warning("User has provided 'userElementCount' but this number is automatically calculated");
		}
		else if (!strcmp("NumVgprs", id->text))
		{	
			warning("User has provided 'NumVgprs' but this number is automatically calculated");
		}
		else if (!strcmp("NumSgprs", id->text))
		{
			warning("User has provided 'NumSgprs' but this number is automatically calculated");
		}
		else if (!strcmp("FloatMode", id->text))
		{
			si2bin_inner_bin->FloatMode = $3;
		}
		else if (!strcmp("IeeeMode", id->text))
		{	
			si2bin_inner_bin->IeeeMode = $3;
		}
		else if (!strcmp("COMPUTE_PGM_RSRC2", id->text))
		{
			warning("User has provided 'COMPUTE_PGM_RSRC2' but this number is automatically calculated from provided PGM_RSRC2 fields");
		}
		else if(!strcmp("rat_op", id->text))
		{	
			si2bin_inner_bin->rat_op = $3;
		}
		else if (!strcmp("COMPUTE_PGM_RSRC2", id->text))
		{
			warning("User has provided 'COMPUTE_PGM_RSRC2' but this number is automatically calculated from provided PGM_RSRC2 fields");
		}
		else
		{
			si2bin_yyerror_fmt("Unrecognized assignment: %s", id->text);
		}

		/* Free id */
		delete(id);
		
	} TOK_NEW_LINE
	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_CBRA TOK_EQ TOK_ID TOK_COMMA TOK_DECIMAL TOK_COMMA TOK_ID TOK_OBRA TOK_DECIMAL TOK_COLON TOK_DECIMAL TOK_CBRA
	{
		struct SIBinaryUserElement *user_elem;
		int err;

		/* check for correct syntax */
		if (strcmp("userElements", $1->text))
			si2bin_yyerror_fmt("User Elements not correctly specified: %s", $1->text);
		
		/* Make sure userElement index is in correct range */
		if ($3 > 15 || $3 < 0)
			si2bin_yyerror_fmt("User Elements index is out of allowed range (0 to 15)");

		/* Create userElement object */
		user_elem = xcalloc(1, sizeof(struct SIBinaryUserElement));
		user_elem->dataClass = StringMapStringErrWrap(si_binary_user_data_map, $6->text, &err);
		if (err)
			si2bin_yyerror_fmt("Unrecognized data class: %s", $6->text);

		user_elem->apiSlot = $8;
		user_elem->startUserReg = $12;
		user_elem->userRegCount = $14 - $12 + 1;
		
		/* Add userElement to userElement list */
		si2bin_inner_bin_add_user_element(si2bin_inner_bin, user_elem, $3);

		/* Free identifiers */
		delete($1);
		delete($6);
		delete($10);

	} TOK_NEW_LINE
	| TOK_ID TOK_COLON TOK_ID TOK_EQ hex_or_dec_value
	{
		/* Find pgm_rsrc2 information */
		if (strcmp("COMPUTE_PGM_RSRC2", $1->text))
			si2bin_yyerror_fmt("Unrecognized assignment: %s", $1->text);
		
		if (!strcmp("SCRATCH_EN", $3->text))
		{
			si2bin_inner_bin->pgm_rsrc2->scrach_en = $5;
		}
		else if (!strcmp("USER_SGPR", $3->text))
		{
			si2bin_inner_bin->pgm_rsrc2->user_sgpr = $5;
		}
		else if (!strcmp("TRAP_PRESENT", $3->text))
		{
			si2bin_inner_bin->pgm_rsrc2->trap_present = $5;
		}
		else if (!strcmp("TGID_X_EN", $3->text))
		{
			si2bin_inner_bin->pgm_rsrc2->tgid_x_en = $5;
		}
		else if (!strcmp("TGID_Y_EN", $3->text))
		{
			si2bin_inner_bin->pgm_rsrc2->tgid_y_en = $5;
		}
		else if (!strcmp("TGID_Z_EN", $3->text))
		{
			si2bin_inner_bin->pgm_rsrc2->tgid_z_en = $5;
		}
		else if (!strcmp("TG_SIZE_EN", $3->text))
		{
			si2bin_inner_bin->pgm_rsrc2->tg_size_en = $5;
		}
		else if (!strcmp("TIDIG_COMP_CNT", $3->text))
		{
			si2bin_inner_bin->pgm_rsrc2->tidig_comp_cnt = $5;
		}
		else if (!strcmp("EXCP_EN_MSB", $3->text))
		{
			si2bin_inner_bin->pgm_rsrc2->excp_en_msb = $5;
		}
		else if (!strcmp("LDS_SIZE", $3->text))
		{
			si2bin_inner_bin->pgm_rsrc2->lds_size = $5;
		}
		else if (!strcmp("EXCP_EN", $3->text))
		{
			si2bin_inner_bin->pgm_rsrc2->excp_en = $5;
		}
		else
		{
			si2bin_yyerror_fmt("Unrecognized field of COMPUTE_PGM_RSRC2: %s", $3->text);
		}

		/* Free id's */
		delete($1);
		delete($3);
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
		
		sscanf($1->text, "%x", &value);

		delete($1);
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
	{	
		delete($1);
	}
	| TOK_ID TOK_COLON TOK_NEW_LINE
	{
		delete($1);
	}
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
		Si2binData *data;
		
		data = new(Si2binData, Si2binDataFloat);
		data->float_value = $2;
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}	
	| float_vals TOK_FLOAT
	{
		Si2binData *data;
		
		data = new(Si2binData, Si2binDataFloat);
		data->float_value = $2;
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	} TOK_NEW_LINE
	;

word_vals
	: 
	| word_vals TOK_NEW_LINE
	| word_vals hex_or_dec_value TOK_COMMA
	{
		Si2binData *data;
		
		data = new(Si2binData, Si2binDataWord);
		data->word_value = (unsigned int) $2;
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
	| word_vals hex_or_dec_value
	{
		Si2binData *data;
		
		data = new(Si2binData, Si2binDataWord);
		data->word_value = (unsigned int) $2;
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	} TOK_NEW_LINE
	;

half_vals
	: 
	| half_vals TOK_NEW_LINE
	| half_vals hex_or_dec_value TOK_COMMA
	{
		Si2binData *data;
		
		data = new(Si2binData, Si2binDataHalf);
		data->half_value = (unsigned short) $2;
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
	| half_vals hex_or_dec_value
	{ 
		Si2binData *data;
		
		data = new(Si2binData, Si2binDataHalf);
		data->half_value = (unsigned short) $2;
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	} TOK_NEW_LINE
	;

byte_vals
	: 
	| byte_vals TOK_NEW_LINE
	| byte_vals hex_or_dec_value TOK_COMMA
	{
		Si2binData *data;
		
		data = new(Si2binData, Si2binDataByte);
		data->byte_value = (unsigned char) $2;
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
	| byte_vals hex_or_dec_value
	{
		Si2binData *data;
		
		data = new(Si2binData, Si2binDataByte);
		data->byte_value = (unsigned char) $2;
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
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
		SIArg *arg = $4;
		int err;

		
		/* Set argument name */
		SIArgSetName(arg, $2->text);
		
		/* Set arg fields */
		arg->value.data_type = str_map_string_err(&si_arg_data_type_map, $1->text, &err);
		if(err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", $1->text);

		arg->value.num_elems = 1;
		arg->value.constant_buffer_num = 1;
		arg->value.constant_offset = $3;
		
		/* Insert argument and free identifiers */
		si2bin_metadata_add_arg(si2bin_metadata, arg);
		delete($1);
		delete($2);
	} TOK_NEW_LINE
	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_CBRA TOK_ID TOK_DECIMAL val_stmt_list
	{
		SIArg *arg = $7;
		int err;

		/* Set argument name */
		SIArgSetName(arg, $5->text);
		
		/* Set argument fields */
		arg->value.data_type = str_map_string_err(&si_arg_data_type_map, $1->text, &err);
		if (err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", $1->text);
		
		arg->value.num_elems = $3;
		arg->value.constant_buffer_num = 1;
		arg->value.constant_offset = $6;
		
		/* Insert argument and free identifiers */
		si2bin_metadata_add_arg(si2bin_metadata, arg);
		delete($1);
		delete($5);
	} TOK_NEW_LINE
	| TOK_ID TOK_STAR TOK_ID TOK_DECIMAL ptr_stmt_list
	{
		SIArg *arg = $5;
		int err;

		/* Set new argument name */
		SIArgSetName(arg, $3->text);
		
		/* Initialize argument */
		arg->pointer.num_elems = 1;
		arg->pointer.data_type = str_map_string_err(&si_arg_data_type_map, $1->text, &err);
		if (err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", $1->text);

		arg->pointer.constant_buffer_num = 1;
		arg->pointer.constant_offset = $4;

		/* Insert argument and free identifiers */
		si2bin_metadata_add_arg(si2bin_metadata, arg);
		delete($1);
		delete($3);
	} TOK_NEW_LINE
	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_CBRA TOK_STAR TOK_ID TOK_DECIMAL ptr_stmt_list
	{
		SIArg *arg = $8;
		int err;

		/* Set new argument name */
		SIArgSetName(arg, $6->text);
		
		/* Initialize argument */
		arg->pointer.num_elems = $3;
		arg->pointer.data_type = str_map_string_err(&si_arg_data_type_map, $1->text, &err);
		if (err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", $1->text);
		
		arg->pointer.constant_buffer_num = 1;
		arg->pointer.constant_offset = $7;


		/* Insert argument and free identifiers */
		si2bin_metadata_add_arg(si2bin_metadata, arg);
		delete($1);
		delete($6);
	} TOK_NEW_LINE
	| TOK_NEW_LINE
	;
	
val_stmt_list
	:
	{
		SIArg *arg;

		/* Create an argument with defaults*/
		arg = new(SIArg, SIArgTypeValue, "arg");

		$$ = arg;
	}
	| val_stmt_list TOK_CONST
	{
		SIArg *arg = $1;

		/* set constarg field to true */
		arg->constarg = 1;
		
		/* Return argument */
		$$ = arg;
	}
	;

ptr_stmt_list
	:
	{
		SIArg *arg;

		/* Create an argument with defaults*/
		arg = new(SIArg, SIArgTypePointer, "arg");
		arg->pointer.scope = SIArgUAV;
		arg->pointer.buffer_num = 12;
		arg->pointer.access_type = SIArgReadWrite;
		
		$$ = arg;
	}
	| ptr_stmt_list TOK_ID
	{
		SIArg *arg = $1;
		String *id = $2;
		int err;

		/* Translate access type */
		arg->pointer.access_type = str_map_string_err(&si_arg_access_type_map, id->text, &err);
		if (err)
			si2bin_yyerror_fmt("Unrecognized access type: %s", id->text);
		
		/* Free ID and return argument */
		delete(id);
		$$ = arg;
	}
	| ptr_stmt_list TOK_UAV
	{
		SIArg *arg = $1;
		String *id = $2;
	
		/* Obtain UAV index */
		arg->pointer.scope = SIArgUAV;
		arg->pointer.buffer_num = atoi(id->text + 3);

		/* Free ID and return argument */
		delete(id);
		$$ = arg;
	}
	| ptr_stmt_list TOK_HL
	{
		SIArg *arg = $1;
	
		/* Set scope to hl */
		arg->pointer.scope = SIArgHwLocal;
		arg->pointer.buffer_num = 1;

		/* Return argument */
		$$ = arg;
	}
	| ptr_stmt_list TOK_CONST
	{
		SIArg *arg = $1;
	
		/* set constarg field to true */
		arg->constarg = 1;

		/* Return argument */
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
		Si2binInst *inst = $1;

		/* Generate code */
		Si2binInstGenerate(inst);
		ELFWriterBufferWrite(si2bin_entry->text_section_buffer,
				inst->bytes.byte, inst->size);
		
		/* Dump Instruction Info */
		//si2bin_inst_dump(inst, stdout);
		delete(inst);
	} TOK_NEW_LINE
	| TOK_NEW_LINE
;

label
	: TOK_ID TOK_COLON
	{
		String *id = $1;
		Si2binSymbol *symbol;

		
		/* Check if symbol exists */
		symbol = asSi2binSymbol(HashTableGet(si2bin->symbol_table, asObject(id)));
		if (symbol && symbol->defined)
			si2bin_yyerror_fmt("multiply defined label: %s", id->text);

		/* Create if it does not exists */
		if (!symbol)
		{
			symbol = new(Si2binSymbol, id->text);
			HashTableInsert(si2bin->symbol_table,
					asObject(id), asObject(symbol));
		}

		/* Define symbol */
		symbol->defined = 1;
		symbol->value = si2bin_entry->text_section_buffer->offset;		

		/* End */
		delete(id);
	}

instr
	: TOK_ID arg_list 
	{
		Si2binInst *inst;
		String *id;
		List *arg_list;

		/* Get arguments */
		id = $1;
		arg_list = $2;
		
		/* Create instruction */
		inst = new_ctor(Si2binInst, CreateWithName,
				id->text, arg_list);

		/* Return instructions */
		delete(id);
		$$ = inst;
	}
;

arg_list
	:  /* Empty argument list */
	{
		$$ = NULL;
	}

	| arg
	{
		List *arg_list;
		
		arg_list = new(List);
		ListAdd(arg_list, asObject($1));
		$$ = arg_list;
	}

	| arg TOK_COMMA arg_list
	{
		List *arg_list = $3;

		/* Add argument to head of argument list */
		ListHead(arg_list);
		ListInsert(arg_list, asObject($1));
		
		/* Return the argument list */
		$$ = arg_list;

	}
;


operand
	: TOK_SCALAR_REGISTER
	{	
		int value;

		value = atoi($1->text + 1);
		$$ = new_ctor(Si2binArg, CreateScalarRegister, value);

		if (value >= si2bin_inner_bin->num_sgprs)
			si2bin_inner_bin->num_sgprs = value + 1;
		
		delete($1);
	}
	
	| TOK_VECTOR_REGISTER
	{
		int value;

		value = atoi($1->text + 1);
		$$ = new_ctor(Si2binArg, CreateVectorRegister, value); 
		
		if (value >= si2bin_inner_bin->num_vgprs)
			si2bin_inner_bin->num_vgprs = value + 1;

		delete($1);
	}
	
	| TOK_SPECIAL_REGISTER
	{
		SIInstSpecialReg reg;
		reg = StringMapStringWrap(si_inst_special_reg_map, $1->text);
		$$ = new_ctor(Si2binArg, CreateSpecialRegister, reg); 
		delete($1);
	}

	| TOK_MEMORY_REGISTER
	{
		$$ = new_ctor(Si2binArg, CreateMemRegister, atoi($1->text + 1));
		delete($1);
	}
	
	| TOK_DECIMAL
	{
		$$ = new_ctor(Si2binArg, CreateLiteral, $1);
	}

	| TOK_HEX
	{
		int value;

		sscanf($1->text, "%x", &value);
		$$ = new_ctor(Si2binArg, CreateLiteral, value); 
		delete($1);
	}

	| TOK_FLOAT
	{
		$$ = new_ctor(Si2binArg, CreateLiteralFloat, $1);
	}	


arg
	: operand
	{
		$$ = $1;
	}

	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_COLON TOK_DECIMAL TOK_CBRA  
	{
		Si2binArg *arg = NULL;
		String *id;

		int low;
		int high;

		/* Read arguments */
		id = $1;
		low = $3;
		high = $5;
		
		/* Initialize */
		if (!strcmp(id->text, "s"))
		{
			arg = new_ctor(Si2binArg, CreateScalarRegisterSeries, low, high);
			if (high >= si2bin_inner_bin->num_sgprs)
				si2bin_inner_bin->num_sgprs = high + 1;
		}
		else if (!strcmp(id->text, "v"))
		{
			arg = new_ctor(Si2binArg, CreateVectorRegisterSeries, low, high);
			if (high >= si2bin_inner_bin->num_vgprs)
				si2bin_inner_bin->num_vgprs = high + 1;
		}
		else
		{
			si2bin_yyerror_fmt("invalid register series: %s", id->text);
		}
		
		/* Return created argument */
		delete(id);
		$$ = arg;
	}

	| TOK_ABS TOK_OPAR arg TOK_CPAR
	{
		Si2binArg *arg = $3;

		/* Activate absolute value flag */
		arg->abs = 1;

		/* Check valid application of 'abs' */
		switch (arg->type)
		{
		case Si2binArgScalarRegister:
		case Si2binArgScalarRegisterSeries:
		case Si2binArgVectorRegister:
		case Si2binArgVectorRegisterSeries:
			break;

		default:
			si2bin_yyerror("abs() function not allowed for argument");
		}

		/* Return */
		$$ = arg;
	}

	| TOK_NEG arg
	{
		Si2binArg *arg = $2;

		/* Activate absolute value flag */
		arg->neg = 1;

		/* Check valid application of 'abs' */
		switch (arg->type)
		{
		case Si2binArgScalarRegister:
		case Si2binArgScalarRegisterSeries:
		case Si2binArgVectorRegister:
		case Si2binArgVectorRegisterSeries:
			break;

		default:
			si2bin_yyerror("abs() function not allowed for argument");
		}

		/* Return */
		$$ = arg;
	}

	| operand maddr_qual TOK_FORMAT TOK_COLON TOK_OBRA TOK_ID TOK_COMMA TOK_ID TOK_CBRA
	{
		Si2binArg *arg;
		Si2binArg *soffset;
		Si2binArg *qual;

		String *id_data_format;
		String *id_num_format;
		
		SIInstBufDataFormat data_format;
		SIInstBufNumFormat num_format;
		
		int err;

		/* Read arguments */
		soffset = $1;
		qual = $2;
		id_data_format = $6;
		id_num_format = $8;
		
		/* Data format */
		data_format = StringMapStringErrWrap(si_inst_buf_data_format_map,
				id_data_format->text, &err);
		if (err)
			si2bin_yyerror_fmt("%s: invalid data format", id_data_format->text);
			
		/* Number format */
		num_format = StringMapStringErrWrap(si_inst_buf_num_format_map,
				id_num_format->text, &err);
		if (err)
			si2bin_yyerror_fmt("%s: invalid number format", id_num_format->text); 

		/* Create argument */
		arg = new_ctor(Si2binArg, CreateMaddr, soffset, qual, data_format, num_format);	
			
		/* Return */
		delete(id_data_format);
		delete(id_num_format);
		$$ = arg;
	}

	| TOK_ID
	{
		Si2binArg *arg;
		String *id;

		/* Get ID */
		id = $1;
		
		/* Create argument */
		arg = new_ctor(Si2binArg, CreateLabel, id->text);
		delete(id);

		/* Return argument */
		$$ = arg;
	}

	| waitcnt_arg
	{
		/* The return value is given by 'waitcnt_arg's definition */
	}
;

maddr_qual

	: 
	{
		$$ = new_ctor(Si2binArg, CreateMaddrQual);
	}

	| maddr_qual TOK_OFFEN
	{
		Si2binArg *qual = $1;

		assert(qual->type == Si2binArgMaddrQual);
		if (qual->value.maddr_qual.offen)
			si2bin_yyerror("redundant qualifier 'offen'");
		qual->value.maddr_qual.offen = 1;
		$$ = qual;
	}

	| maddr_qual TOK_IDXEN
	{
		Si2binArg *qual = $1;

		assert(qual->type == Si2binArgMaddrQual);
		if (qual->value.maddr_qual.idxen)
			si2bin_yyerror("redundant qualifier 'idxen'");
		qual->value.maddr_qual.idxen = 1;
		$$ = qual;
	}

	| maddr_qual TOK_OFFSET TOK_COLON TOK_DECIMAL
	{
		Si2binArg *qual = $1;
		int offset = $4;

		assert(qual->type == Si2binArgMaddrQual);
		qual->value.maddr_qual.offset = offset;
		/* FIXME - check range of 'offset' */
		$$ = qual;
	}

waitcnt_arg

	: waitcnt_elem

	| waitcnt_elem TOK_AMP waitcnt_arg
	{
		if ($3->value.wait_cnt.vmcnt_active && $1->value.wait_cnt.vmcnt_active)
			si2bin_yyerror("duplicate 'vmcnt' token");
		$3->value.wait_cnt.vmcnt_active += $1->value.wait_cnt.vmcnt_active;
		$3->value.wait_cnt.vmcnt_value += $1->value.wait_cnt.vmcnt_value;		
		
		if ($3->value.wait_cnt.expcnt_active && $1->value.wait_cnt.expcnt_active)
			si2bin_yyerror("duplicate 'expcnt' token");
		$3->value.wait_cnt.expcnt_active += $1->value.wait_cnt.expcnt_active;
		$3->value.wait_cnt.expcnt_value += $1->value.wait_cnt.expcnt_value;	
		
		if ($3->value.wait_cnt.lgkmcnt_active && $1->value.wait_cnt.lgkmcnt_active)
			si2bin_yyerror("duplicate 'lgkmcnt' token");
		$3->value.wait_cnt.lgkmcnt_active += $1->value.wait_cnt.lgkmcnt_active;
		$3->value.wait_cnt.lgkmcnt_value += $1->value.wait_cnt.lgkmcnt_value;	
		
		delete($1);
		$$ = $3;
	}
;

waitcnt_elem

	: TOK_ID TOK_OPAR TOK_DECIMAL TOK_CPAR
	{
		Si2binArg *arg;
		String *id;

		/* Read arguments */
		id = $1;
		
		/* Create argument */
		arg = new(Si2binArg);
		arg->type = Si2binArgWaitcnt;
		
		if (!strcmp(id->text, "vmcnt"))
		{
			arg->value.wait_cnt.vmcnt_active = 1;
			arg->value.wait_cnt.vmcnt_value = $3;
		}
		else if (!strcmp(id->text, "expcnt"))
		{
			arg->value.wait_cnt.expcnt_active = 1;
			arg->value.wait_cnt.expcnt_value = $3;
		}
		else if (!strcmp(id->text, "lgkmcnt"))
		{
			arg->value.wait_cnt.lgkmcnt_active = 1;
			arg->value.wait_cnt.lgkmcnt_value = $3;
		}
		
		/* Return */
		delete(id);
		$$ = arg;
	}
;

%%
