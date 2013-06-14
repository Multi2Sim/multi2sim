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
#include <arch/southern-islands/asm/bin-file.h>
#include <lib/util/debug.h>
#include <lib/util/elf-encode.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "arg.h"
#include "inner-bin.h"
#include "id.h"
#include "inst.h"
#include "metadata.h"
#include "outer-bin.h"
#include "si2bin.h"
#include "string.h"
#include "symbol.h"
#include "task.h"


#define YYERROR_VERBOSE

%}


%union {
	int num;
	float num_float;
	struct si2bin_id_t *id;
	struct si2bin_inst_t *inst;
	struct si_label_t *label;
	struct list_t *list;
	struct si2bin_arg_t *arg;
	struct si_arg_t *si_arg;
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
%token TOK_GLOBAL
%token TOK_MEM
%token TOK_ARGS
%token TOK_DATA
%token TOK_TEXT

%type<inst> instr
%type<list> arg_list
%type<arg> operand
%type<arg> arg
%type<arg> maddr_qual
%type<arg> waitcnt_elem
%type<arg> waitcnt_arg
%type<si_arg> ptr_stmt_list

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
	: mem_section
	| data_section
	| args_section
	| text_section
	{
		si2bin_metadata->num_sgprs = si2bin_num_sgprs + 1;
		si2bin_metadata->num_vgprs = si2bin_num_vgprs + 1;

	}	
	;

global_section
	: TOK_GLOBAL TOK_ID TOK_NEW_LINE
	{
		struct si2bin_id_t *id = $2;

		si2bin_inner_bin = si2bin_inner_bin_create(id->name);
		si2bin_metadata = si2bin_metadata_create();
		si2bin_entry = si2bin_inner_bin_entry_create();

		si2bin_metadata->uniqueid = si2bin_uniqueid;

		si2bin_inner_bin_add_entry(si2bin_inner_bin, si2bin_entry);

		si2bin_outer_bin_add(si2bin_outer_bin, si2bin_inner_bin, si2bin_metadata);

		si2bin_uniqueid ++;

		si2bin_num_vgprs = 0;
		si2bin_num_sgprs = 0;
		
		si2bin_id_free(id);
	}

mem_section
	: mem_header
	| mem_header mem_stmt_list
	;

mem_header
	: TOK_MEM TOK_NEW_LINE
	;

mem_stmt_list
	: mem_stmt
	| mem_stmt mem_stmt_list
;

mem_stmt
	: TOK_NEW_LINE 
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
	: TOK_ID TOK_EQ TOK_DECIMAL TOK_NEW_LINE
	{
		if (!strcmp("userElementCount", $1->name))
		{
		}
		else if (!strcmp("NumVgprs", $1->name))
		{
		}
		else if (!strcmp("NumSgprs", $1->name))
		{
		}
		else if (!strcmp("FloatMode", $1->name))
		{
			si2bin_inner_bin->FloatMode = $3;
		}
		else if (!strcmp("IeeeMode", $1->name))
		{	
			si2bin_inner_bin->IeeeMode = $3;
		}
		else
		{
			fatal("Unrecognized assignment");
		}

		si2bin_id_free($1);
	}
	| TOK_ID TOK_EQ TOK_HEX TOK_NEW_LINE
	{
		if (!strcmp("COMPUTE_PGM_RSRC2", $1->name))
		{
			int pgm_rsrc2;
			
			sscanf($3->name, "%x", &pgm_rsrc2);
			si2bin_inner_bin->pgm_rsrc2 = pgm_rsrc2;

			si2bin_id_free($1);
			si2bin_id_free($3);

		}
	}
	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_CBRA TOK_EQ TOK_ID TOK_COMMA TOK_DECIMAL TOK_COMMA TOK_ID TOK_OBRA TOK_DECIMAL TOK_COLON TOK_DECIMAL TOK_CBRA TOK_NEW_LINE
	{
		struct si_bin_enc_user_element_t *user_elem;
		int data_class;


		if (strcmp("userElements", $1->name))
			fatal("User Elements not correctly specified: %s", $1->name);
		
		data_class = str_map_string(&si_bin_user_data_class, $6->name);

		user_elem = si_bin_enc_user_element_create();
		user_elem->dataClass = data_class;
		user_elem->apiSlot = $8;
		user_elem->startUserReg = $12;
		user_elem->userRegCount = $14 - $12 + 1;
		
		si2bin_inner_bin_add_user_element(si2bin_inner_bin, user_elem);

		si2bin_id_free($1);
		si2bin_id_free($6);
		si2bin_id_free($10);

	}
	| TOK_ID TOK_COLON TOK_ID TOK_EQ TOK_DECIMAL TOK_NEW_LINE
	{
		/* Added to accept COMPUTE_PGM_RSRC2:USER_SGPR */
		si2bin_id_free($1);
		si2bin_id_free($3);
	}


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
	: TOK_ID TOK_ID TOK_DECIMAL TOK_NEW_LINE
	{
		struct si_arg_t *arg;
		int data_type;

		data_type = str_map_string(&si_arg_data_type_map, $1->name);
		arg = si_arg_create(si_arg_value, $2->name);
		arg->value.data_type = data_type;
		arg->value.num_elems = 1;
		arg->value.constant_buffer_num = 1;
		arg->value.constant_offset = $3;
		
		si2bin_metadata_add_arg(si2bin_metadata, arg);

		si2bin_id_free($1);
		si2bin_id_free($2);
	}
	
	| TOK_ID TOK_STAR TOK_ID TOK_DECIMAL ptr_stmt_list TOK_NEW_LINE
	{
		struct si_arg_t *arg = $5;

		/* Set new argument name */
		si_arg_name_set(arg, $3->name);
		
		/* Initialize argument */
		arg->pointer.data_type = str_map_string(&si_arg_data_type_map, $1->name);
		arg->pointer.constant_buffer_num = 1;
		arg->pointer.constant_offset = $4;

		/* If UAV is not specified, default to 12 */
		if (!arg->pointer.scope)
		{
			arg->pointer.scope = si_arg_uav;
			arg->pointer.buffer_num = 12;
		}

		/* If access type not specified, default to RW */
		if (!arg->pointer.access_type)
			arg->pointer.access_type = str_map_string(&si_arg_access_type_map, "RW");

		/* Insert argument and free identifiers */
		si2bin_metadata_add_arg(si2bin_metadata, arg);
		si2bin_id_free($1);
		si2bin_id_free($3);
	}

	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_CBRA TOK_DECIMAL TOK_NEW_LINE
	{
		struct si_arg_t *arg;
		int data_type;

		data_type = str_map_string(&si_arg_data_type_map, $1->name);
		arg = si_arg_create(si_arg_value, "arg");
		arg->value.data_type = data_type;
		arg->value.num_elems = $3;
		arg->value.constant_buffer_num = 1;
		arg->value.constant_offset = $5;
		
		si2bin_metadata_add_arg(si2bin_metadata, arg);
		si2bin_id_free($1);
	}

ptr_stmt_list
	:
	{
		struct si_arg_t *arg;

		/* Create an argument */
		arg = si_arg_create(si_arg_pointer, "arg");
		$$ = arg;
	}
	| ptr_stmt_list TOK_ID
	{
		struct si_arg_t *arg = $1;
		struct si2bin_id_t *id = $2;

		/* Translate access type */
		arg->pointer.access_type = str_map_string(&si_arg_access_type_map, arg->name);
		if (!arg->pointer.access_type)
			si2bin_yyerror_fmt("%s: invalid access type", id->name);

		/* Free ID and return argument */
		si2bin_id_free(id);
		$$ = arg;
	}
	| ptr_stmt_list TOK_UAV
	{
		struct si_arg_t *arg = $1;
		struct si2bin_id_t *id = $2;
	
		/* Obtain UAV index */
		arg->pointer.scope = si_arg_uav;
		arg->pointer.buffer_num = atoi(id->name + 3);

		/* Free ID and return argument */
		si2bin_id_free(id);
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

	| instr TOK_NEW_LINE
	{
		struct si2bin_inst_t *inst = $1;

		/* Generate code */
		si2bin_inst_gen(inst);
		elf_enc_buffer_write(si2bin_entry->text_section_buffer, inst->inst_bytes.bytes, inst->size);
		
		/* Dump Instruction Info */
		/* si2bin_inst_dump(inst, stdout); */
		si2bin_inst_free(inst);
	}
	
	| TOK_NEW_LINE
;

label
	: TOK_ID TOK_COLON
	{
		struct si2bin_id_t *id = $1;
		struct si2bin_symbol_t *symbol;

		
		/* Check if symbol exists */
		symbol = hash_table_get(si2bin_symbol_table, id->name);
		if (symbol && symbol->defined)
			si2bin_yyerror_fmt("multiply defined label: %s", id->name);

		/* Create if it does not exists */
		if (!symbol)
		{
			symbol = si2bin_symbol_create(id->name);
			hash_table_insert(si2bin_symbol_table, id->name, symbol);
		}

		/* Define symbol */
		symbol->defined = 1;
		symbol->value = si2bin_entry->text_section_buffer->offset;		

		/* End */
		si2bin_id_free(id);
	}

instr
	: TOK_ID arg_list 
	{
		struct si2bin_inst_t *inst;
		struct si2bin_id_t *id;
		struct list_t *arg_list;

		/* Get arguments */
		id = $1;
		arg_list = $2;
		
		/* Create instruction */
		inst = si2bin_inst_create_with_name(id->name, arg_list);

		/* Return instructions */
		si2bin_id_free(id);
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
		struct list_t *arg_list;
		
		/* Create */
		arg_list = list_create();
		
		/* Initialize */
		list_add(arg_list, $1);
		
		/* Return the arg list */
		$$ = arg_list;
	}

	| arg TOK_COMMA arg_list
	{
		/* Add argument to head of list_t in $3 */
		list_insert($3, 0, $1);
		
		/* Return the arg list */
		$$ = $3;

	}
;


operand
	: TOK_SCALAR_REGISTER
	{	
		int value;

		value = atoi($1->name + 1);
		$$ = si2bin_arg_create_scalar_register(value); 

		if (value > si2bin_num_sgprs)
			si2bin_num_sgprs = value;
		
		si2bin_id_free($1);
	}
	
	| TOK_VECTOR_REGISTER
	{
		int value;

		value = atoi($1->name + 1);
		$$ = si2bin_arg_create_vector_register(value); 
		
		if (value > si2bin_num_vgprs)
			si2bin_num_vgprs = value;

		si2bin_id_free($1);
	}
	
	| TOK_SPECIAL_REGISTER
	{
		enum si_inst_special_reg_t reg;
		reg = str_map_string(&si_inst_special_reg_map, $1->name);
		$$ = si2bin_arg_create_special_register(reg); 
		si2bin_id_free($1);
	}

	| TOK_MEMORY_REGISTER
	{
		$$ = si2bin_arg_create_mem_register(atoi($1->name + 1));
		si2bin_id_free($1);
	}
	
	| TOK_DECIMAL
	{
		$$ = si2bin_arg_create_literal($1);
	}

	| TOK_HEX
	{
		int value;

		sscanf($1->name, "%x", &value);
		$$ = si2bin_arg_create_literal(value); 
		si2bin_id_free($1);
	}

	| TOK_FLOAT
	{
		$$ = si2bin_arg_create_literal_float($1);
	}	


arg
	: operand
	{
		$$ = $1;
	}

	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_COLON TOK_DECIMAL TOK_CBRA  
	{
		struct si2bin_arg_t *arg;
		struct si2bin_id_t *id;

		int low;
		int high;

		/* Read arguments */
		id = $1;
		low = $3;
		high = $5;
		
		/* Create argument */
		arg = si2bin_arg_create(); 
		
		/* Initialize */
		if (!strcmp(id->name, "s"))
		{
			arg->type = si2bin_arg_scalar_register_series;
			arg->value.scalar_register_series.low = low;
			arg->value.scalar_register_series.high = high;

			if (high > si2bin_num_sgprs)
				si2bin_num_sgprs = high;
		}
		else if (!strcmp(id->name, "v"))
		{
			arg->type = si2bin_arg_vector_register_series;
			arg->value.vector_register_series.low = low;
			arg->value.vector_register_series.high = high;
			
			if (high > si2bin_num_vgprs)
				si2bin_num_vgprs = high;
		}
		else
		{
			si2bin_yyerror_fmt("invalid register series: %s", id->name);
		}
		
		/* Return created argument */
		si2bin_id_free(id);
		$$ = arg;
	}

	| TOK_ABS TOK_OPAR arg TOK_CPAR
	{
		struct si2bin_arg_t *arg = $3;

		/* Activate absolute value flag */
		arg->abs = 1;

		/* Check valid application of 'abs' */
		switch (arg->type)
		{
		case si2bin_arg_scalar_register:
		case si2bin_arg_scalar_register_series:
		case si2bin_arg_vector_register:
		case si2bin_arg_vector_register_series:
			break;

		default:
			si2bin_yyerror("abs() function not allowed for argument");
		}

		/* Return */
		$$ = arg;
	}

	| TOK_NEG arg
	{
		struct si2bin_arg_t *arg = $2;

		/* Activate absolute value flag */
		arg->neg = 1;

		/* Check valid application of 'abs' */
		switch (arg->type)
		{
		case si2bin_arg_scalar_register:
		case si2bin_arg_scalar_register_series:
		case si2bin_arg_vector_register:
		case si2bin_arg_vector_register_series:
			break;

		default:
			si2bin_yyerror("abs() function not allowed for argument");
		}

		/* Return */
		$$ = arg;
	}

	| operand maddr_qual TOK_FORMAT TOK_COLON TOK_OBRA TOK_ID TOK_COMMA TOK_ID TOK_CBRA
	{
		struct si2bin_arg_t *arg;
		struct si2bin_arg_t *soffset;
		struct si2bin_arg_t *qual;

		struct si2bin_id_t *id_data_format;
		struct si2bin_id_t *id_num_format;
		
		enum si_inst_buf_data_format_t data_format;
		enum si_inst_buf_num_format_t num_format;
		
		int err;

		/* Read arguments */
		soffset = $1;
		qual = $2;
		id_data_format = $6;
		id_num_format = $8;
		
		/* Data format */
		data_format = str_map_string_err(&si_inst_buf_data_format_map,
				id_data_format->name, &err);
		if (err)
			si2bin_yyerror_fmt("%s: invalid data format", id_data_format->name);
			
		/* Number format */
		num_format = str_map_string_err(&si_inst_buf_num_format_map,
				id_num_format->name, &err);
		if (err)
			si2bin_yyerror_fmt("%s: invalid number format", id_num_format->name); 

		/* Create argument */
		arg = si2bin_arg_create_maddr(soffset, qual, data_format, num_format);	
			
		/* Return */
		si2bin_id_free(id_data_format);
		si2bin_id_free(id_num_format);
		$$ = arg;
	}

	| TOK_ID
	{
		struct si2bin_arg_t *arg;
		struct si2bin_id_t *id;
		struct si2bin_symbol_t *symbol;

		/* Get symbol or create it */
		id = $1;
		symbol = hash_table_get(si2bin_symbol_table, id->name);
		if (!symbol)
		{
			symbol = si2bin_symbol_create(id->name);
			hash_table_insert(si2bin_symbol_table, id->name, symbol);
		}
		
		/* Create argument */
		arg = si2bin_arg_create_label(symbol);

		/* Return */
		si2bin_id_free(id);
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
		$$ = si2bin_arg_create_maddr_qual();
	}

	| maddr_qual TOK_OFFEN
	{
		struct si2bin_arg_t *qual = $1;

		assert(qual->type == si2bin_arg_maddr_qual);
		if (qual->value.maddr_qual.offen)
			si2bin_yyerror("redundant qualifier 'offen'");
		qual->value.maddr_qual.offen = 1;
		$$ = qual;
	}

	| maddr_qual TOK_IDXEN
	{
		struct si2bin_arg_t *qual = $1;

		assert(qual->type == si2bin_arg_maddr_qual);
		if (qual->value.maddr_qual.idxen)
			si2bin_yyerror("redundant qualifier 'idxen'");
		qual->value.maddr_qual.idxen = 1;
		$$ = qual;
	}

	| maddr_qual TOK_OFFSET TOK_COLON TOK_DECIMAL
	{
		struct si2bin_arg_t *qual = $1;
		int offset = $4;

		assert(qual->type == si2bin_arg_maddr_qual);
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
		
		si2bin_arg_free($1);
		$$ = $3;
	}
;

waitcnt_elem

	: TOK_ID TOK_OPAR TOK_DECIMAL TOK_CPAR
	{
		struct si2bin_arg_t *arg;
		struct si2bin_id_t *id;

		/* Read arguments */
		id = $1;
		
		/* Create argument */
		arg = si2bin_arg_create(); 
		
		/* Initialize */
		arg->type = si2bin_arg_waitcnt;
		
		if (!strcmp(id->name, "vmcnt"))
		{
			arg->value.wait_cnt.vmcnt_active = 1;
			arg->value.wait_cnt.vmcnt_value = $3;
		}
		else if (!strcmp(id->name, "expcnt"))
		{
			arg->value.wait_cnt.expcnt_active = 1;
			arg->value.wait_cnt.expcnt_value = $3;
		}
		else if (!strcmp(id->name, "lgkmcnt"))
		{
			arg->value.wait_cnt.lgkmcnt_active = 1;
			arg->value.wait_cnt.lgkmcnt_value = $3;
		}
		
		/* Return */
		si2bin_id_free(id);
		$$ = arg;
	}
;

%%
