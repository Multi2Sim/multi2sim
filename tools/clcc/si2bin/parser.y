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


/* 'si2bin_inner_bin' and 'si2bin_inner_bin_entry' pointing to the current internal
 * binary that is set every time a new .global section is found. */

struct si2bin_inner_bin_t *si2bin_inner_bin;
struct si2bin_metadata_t *si2bin_metadata;
int si2bin_uniqueid = 1024;


%}


%union {
	int num;
	float num_float;
	struct si2bin_id_t *id;
	struct si2bin_inst_t *inst;
	struct si_label_t *label;
	struct list_t *list;
	struct si2bin_arg_t *arg;
}

 


%token<id> TOK_SCALAR_REGISTER
%token<id> TOK_VECTOR_REGISTER
%token<id> TOK_SPECIAL_REGISTER
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
	: TOK_ID TOK_ID TOK_OBRA TOK_DECIMAL TOK_COLON TOK_DECIMAL TOK_CBRA TOK_NEW_LINE 
	{
		struct si2bin_id_t *id = $1;
		struct si2bin_inner_bin_constant_buffer_t *cb;

		
		if (!strcmp(id->name, "uav"))
		{
			si2bin_inner_bin->uav_ptr->start_reg = $4;
			si2bin_inner_bin->uav_ptr->end_reg = $6;
		}
		else if (!strcmp(id->name, "cb0"))
		{
			cb = list_get(si2bin_inner_bin->cb_list, 0);
			assert(cb);
			cb->start_reg = $4;
			cb->end_reg = $6;
		}
		else if (!strcmp(id->name, "cb1"))
		{
			cb = list_get(si2bin_inner_bin->cb_list, 1);
			assert(cb);
			cb->start_reg = $4;
			cb->end_reg = $6;
		}
		else if (!strcmp(id->name, "cb2"))
		{
			cb = list_get(si2bin_inner_bin->cb_list, 2);
			assert(cb);
			cb->start_reg = $4;
			cb->end_reg = $6;
		}

		si2bin_id_free(id);
		si2bin_id_free($2);
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
	: TOK_NEW_LINE


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
	: TOK_ID TOK_DECIMAL TOK_NEW_LINE
	{
		struct si2bin_id_t *id;
		struct si_arg_t *arg;
		int data_type;

		id = $1;

		data_type = str_map_string(&si_arg_data_type_map, id->name);
		arg = si_arg_create(si_arg_value, "val");
		arg->value.data_type = data_type;
		arg->value.num_elems = 1;
		arg->value.constant_buffer_num = 1;
		arg->value.constant_offset = $2;
		
		si2bin_metadata_add_arg(si2bin_metadata, arg);

		si2bin_id_free(id);
	}
	| TOK_ID TOK_STAR TOK_DECIMAL TOK_NEW_LINE
	{
		struct si2bin_id_t *id;
		struct si_arg_t *arg;
		int data_type;

		id = $1;

		data_type = str_map_string(&si_arg_data_type_map, id->name);
		arg = si_arg_create(si_arg_pointer, "ptr");
		arg->pointer.data_type = data_type;
		arg->pointer.constant_buffer_num = 1;
		arg->pointer.constant_offset = $3;
		arg->pointer.scope = si_arg_uav;
		arg->pointer.buffer_num = 12;
		arg->pointer.access_type = si_arg_read_write;
		
		si2bin_metadata_add_arg(si2bin_metadata, arg);

		si2bin_id_free(id);
		id = $1;
	}
	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_CBRA TOK_DECIMAL TOK_NEW_LINE
	{
		struct si2bin_id_t *id;
		struct si_arg_t *arg;
		int data_type;

		id = $1;

		data_type = str_map_string(&si_arg_data_type_map, id->name);
		arg = si_arg_create(si_arg_value, "val");
		arg->value.data_type = data_type;
		arg->value.num_elems = $3;
		arg->value.constant_buffer_num = 1;
		arg->value.constant_offset = $5;
		
		si2bin_metadata_add_arg(si2bin_metadata, arg);

		si2bin_id_free(id);
	}

/* FIXME - use a check for 'si2bin_inner_bin' == NULL to make sure that we are inside of
 * a valid internal binary, i.e., the order of sections is right (no .text before .global).
 * Same for .data, etc. */
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
		si2bin_inst_dump(inst, stdout);
		si2bin_inst_free(inst);
	}
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
		inst = si2bin_inst_create(id->name, arg_list);

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
		$$ = si2bin_arg_create_scalar_register($1->name);
		si2bin_id_free($1);
	}
	
	| TOK_VECTOR_REGISTER
	{
		$$ = si2bin_arg_create_vector_register($1->name); 
		si2bin_id_free($1);
	}
	
	| TOK_SPECIAL_REGISTER
	{
		$$ = si2bin_arg_create_special_register($1->name); 
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
		}
		else if (!strcmp(id->name, "v"))
		{
			arg->type = si2bin_arg_vector_register_series;
			arg->value.vector_register_series.low = low;
			arg->value.vector_register_series.high = high;
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

		/* Read arguments */
		soffset = $1;
		qual = $2;
		id_data_format = $6;
		id_num_format = $8;

		/* Create argument */
		arg = si2bin_arg_create_maddr(soffset, qual,
			id_data_format->name, id_num_format->name);	
			
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
