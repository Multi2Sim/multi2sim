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
#include "data.h"
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
		/* Process any tasks still left */
		si2bin_task_list_process();
		
		/* Clean up tasks and symbol table when finished parsing kernel */
		si2bin_task_list_done();
		si2bin_symbol_table_done();

		/* Set up new tasks and symbol table for next kernel */
		si2bin_task_list_init();
		si2bin_symbol_table_init();
	}
	;

global_section
	: global_header
	| global_header TOK_NEW_LINE
	;

global_header
	: TOK_GLOBAL TOK_ID
	{
		/* Create new objects for each kernel */
		si2bin_inner_bin = si2bin_inner_bin_create($2->name);
		si2bin_metadata = si2bin_metadata_create();
		si2bin_entry = si2bin_inner_bin_entry_create();
		
		/* Add objects and values to over all elf (outer_bin) */
		si2bin_metadata->uniqueid = si2bin_uniqueid;
		si2bin_inner_bin_add_entry(si2bin_inner_bin, si2bin_entry);
		si2bin_outer_bin_add(si2bin_outer_bin, si2bin_inner_bin, si2bin_metadata);

		/* Increment id */
		si2bin_uniqueid ++;

		/* Free id */
		si2bin_id_free($2);
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
		/* Find memory information and store it in metadata */
		if (!strcmp("uavprivate", $1->name))
		{
			si2bin_metadata->uavprivate = $3;
		}
		else if (!strcmp("hwregion", $1->name))
		{	
			si2bin_metadata->hwregion = $3;
		}
		else if (!strcmp("hwlocal", $1->name))
		{
			si2bin_metadata->hwlocal = $3;
		}
		else if (!strcmp("userElementCount", $1->name))
		{
			warning("User has provided 'userElementCount' but this number is automatically calculated");
		}
		else if (!strcmp("NumVgprs", $1->name))
		{	
			warning("User has provided 'NumVgprs' but this number is automatically calculated");
		}
		else if (!strcmp("NumSgprs", $1->name))
		{
			warning("User has provided 'NumSgprs' but this number is automatically calculated");
		}
		else if (!strcmp("FloatMode", $1->name))
		{
			si2bin_inner_bin->FloatMode = $3;
		}
		else if (!strcmp("IeeeMode", $1->name))
		{	
			si2bin_inner_bin->IeeeMode = $3;
		}
		else if (!strcmp("COMPUTE_PGM_RSRC2", $1->name))
		{
			warning("User has provided 'COMPUTE_PGM_RSRC2' but this number is automatically calculated from provided PGM_RSRC2 fields");
		}
		else if(!strcmp("rat_op", $1->name))
		{	
			si2bin_inner_bin->rat_op = $3;
		}
		else if (!strcmp("COMPUTE_PGM_RSRC2", $1->name))
		{
			warning("User has provided 'COMPUTE_PGM_RSRC2' but this number is automatically calculated from provided PGM_RSRC2 fields");
		}
		else
		{
			si2bin_yyerror_fmt("Unrecognized assignment: %s", $1->name);
		}

		/* Free id */
		si2bin_id_free($1);
	} TOK_NEW_LINE
	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_CBRA TOK_EQ TOK_ID TOK_COMMA TOK_DECIMAL TOK_COMMA TOK_ID TOK_OBRA TOK_DECIMAL TOK_COLON TOK_DECIMAL TOK_CBRA
	{
		struct si_bin_enc_user_element_t *user_elem;
		int err;

		/* check for correct syntax */
		if (strcmp("userElements", $1->name))
			si2bin_yyerror_fmt("User Elements not correctly specified: %s", $1->name);
		
		/* Make sure userElement index is in correct range */
		if ($3 > 15 || $3 < 0)
			si2bin_yyerror_fmt("User Elements index is out of allowed range (0 to 15)");

		/* Create userElement object */
		user_elem = si_bin_enc_user_element_create();
		user_elem->dataClass = str_map_string_err(&si_bin_user_data_class, $6->name, &err);
		if (err)
			si2bin_yyerror_fmt("Unrecognized data class: %s", $6->name);

		user_elem->apiSlot = $8;
		user_elem->startUserReg = $12;
		user_elem->userRegCount = $14 - $12 + 1;
		
		/* Add userElement to userElement list */
		si2bin_inner_bin_add_user_element(si2bin_inner_bin, user_elem, $3);

		/* Free id */
		si2bin_id_free($1);
		si2bin_id_free($6);
		si2bin_id_free($10);

	} TOK_NEW_LINE
	| TOK_ID TOK_COLON TOK_ID TOK_EQ hex_or_dec_value
	{
		/* Find pgm_rsrc2 information */
		if (strcmp("COMPUTE_PGM_RSRC2", $1->name))
			si2bin_yyerror_fmt("Unrecognized assignment: %s", $1->name);
		
		if (!strcmp("SCRATCH_EN", $3->name))
		{
			si2bin_inner_bin->pgm_rsrc2->scrach_en = $5;
		}
		else if (!strcmp("USER_SGPR", $3->name))
		{
			si2bin_inner_bin->pgm_rsrc2->user_sgpr = $5;
		}
		else if (!strcmp("TRAP_PRESENT", $3->name))
		{
			si2bin_inner_bin->pgm_rsrc2->trap_present = $5;
		}
		else if (!strcmp("TGID_X_EN", $3->name))
		{
			si2bin_inner_bin->pgm_rsrc2->tgid_x_en = $5;
		}
		else if (!strcmp("TGID_Y_EN", $3->name))
		{
			si2bin_inner_bin->pgm_rsrc2->tgid_y_en = $5;
		}
		else if (!strcmp("TGID_Z_EN", $3->name))
		{
			si2bin_inner_bin->pgm_rsrc2->tgid_z_en = $5;
		}
		else if (!strcmp("TG_SIZE_EN", $3->name))
		{
			si2bin_inner_bin->pgm_rsrc2->tg_size_en = $5;
		}
		else if (!strcmp("TIDIG_COMP_CNT", $3->name))
		{
			si2bin_inner_bin->pgm_rsrc2->tidig_comp_cnt = $5;
		}
		else if (!strcmp("EXCP_EN_MSB", $3->name))
		{
			si2bin_inner_bin->pgm_rsrc2->excp_en_msb = $5;
		}
		else if (!strcmp("LDS_SIZE", $3->name))
		{
			si2bin_inner_bin->pgm_rsrc2->lds_size = $5;
		}
		else if (!strcmp("EXCP_EN", $3->name))
		{
			si2bin_inner_bin->pgm_rsrc2->excp_en = $5;
		}
		else
		{
			si2bin_yyerror_fmt("Unrecognized field of COMPUTE_PGM_RSRC2: %s", $3->name);
		}

		/* Free id's */
		si2bin_id_free($1);
		si2bin_id_free($3);
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
		
		sscanf($1->name, "%x", &value);

		si2bin_id_free($1);
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
		si2bin_id_free($1);
	}
	| TOK_ID TOK_COLON TOK_NEW_LINE
	{
		si2bin_id_free($1);
	}
	| TOK_INT_DECL int_vals
	| TOK_SHORT_DECL short_vals
	| TOK_FLOAT_DECL float_vals
	| TOK_WORD_DECL word_vals
	| TOK_HALF_DECL half_vals
	| TOK_BYTE_DECL byte_vals
	;

int_vals
	: 
	| int_vals TOK_NEW_LINE
	| int_vals TOK_DECIMAL TOK_COMMA
	{
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_int;
		data->int_value = $2;
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
	| int_vals TOK_DECIMAL
	{
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_int;
		data->int_value = $2;
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	} TOK_NEW_LINE
	;

short_vals
	: 
	| short_vals TOK_NEW_LINE
	| short_vals TOK_DECIMAL TOK_COMMA
	{
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_short;
		data->short_value = (short)$2;
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
	| short_vals TOK_DECIMAL
	{
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_short;
		data->short_value = (short)$2;
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	} TOK_NEW_LINE
	;

float_vals
	: 
	| float_vals TOK_NEW_LINE
	| float_vals TOK_FLOAT TOK_COMMA
	{
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_float;
		data->float_value = $2;
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}	
	| float_vals TOK_FLOAT
	{
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_float;
		data->float_value = $2;

		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	} TOK_NEW_LINE
	;

word_vals
	: 
	| word_vals TOK_NEW_LINE
	| word_vals hex_or_dec_value TOK_COMMA
	{
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_word;
		data->word_value = (unsigned int)$2;
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
	| word_vals hex_or_dec_value
	{
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_word;
		data->word_value = (unsigned int)$2;
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	} TOK_NEW_LINE
	;

half_vals
	: 
	| half_vals TOK_NEW_LINE
	| half_vals hex_or_dec_value TOK_COMMA
	{
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_half;
		data->half_value = (unsigned short)$2;
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
	| half_vals hex_or_dec_value
	{ 
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_half;
		data->half_value = (unsigned short)$2;
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	} TOK_NEW_LINE
	;

byte_vals
	: 
	| byte_vals TOK_NEW_LINE
	| byte_vals hex_or_dec_value TOK_COMMA
	{
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_byte;
		data->byte_value = (unsigned char)$2;
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
	| byte_vals hex_or_dec_value
	{
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_byte;
		data->byte_value = (unsigned char)$2;
		
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
		struct si_arg_t *arg = $4;
		int err;

		
		/* Set argument name */
		si_arg_name_set(arg, $2->name);
		
		/* Set arg fields */
		arg->value.data_type = str_map_string_err(&si_arg_data_type_map, $1->name, &err);
		if(err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", $1->name);

		arg->value.num_elems = 1;
		arg->value.constant_buffer_num = 1;
		arg->value.constant_offset = $3;
		
		/* Insert argument and free identifiers */
		si2bin_metadata_add_arg(si2bin_metadata, arg);
		si2bin_id_free($1);
		si2bin_id_free($2);
	} TOK_NEW_LINE
	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_CBRA TOK_ID TOK_DECIMAL val_stmt_list
	{
		struct si_arg_t *arg = $7;
		int err;

		/* Set argument name */
		si_arg_name_set(arg, $5->name);
		
		/* Set argument fields */
		arg->value.data_type = str_map_string_err(&si_arg_data_type_map, $1->name, &err);
		if (err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", $1->name);
		
		arg->value.num_elems = $3;
		arg->value.constant_buffer_num = 1;
		arg->value.constant_offset = $6;
		
		/* Insert argument and free identifiers */
		si2bin_metadata_add_arg(si2bin_metadata, arg);
		si2bin_id_free($1);
		si2bin_id_free($5);
	} TOK_NEW_LINE
	| TOK_ID TOK_STAR TOK_ID TOK_DECIMAL ptr_stmt_list
	{
		struct si_arg_t *arg = $5;
		int err;

		/* Set new argument name */
		si_arg_name_set(arg, $3->name);
		
		/* Initialize argument */
		arg->pointer.num_elems = 1;
		arg->pointer.data_type = str_map_string_err(&si_arg_data_type_map, $1->name, &err);
		if (err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", $1->name);

		arg->pointer.constant_buffer_num = 1;
		arg->pointer.constant_offset = $4;

		/* Insert argument and free identifiers */
		si2bin_metadata_add_arg(si2bin_metadata, arg);
		si2bin_id_free($1);
		si2bin_id_free($3);
	} TOK_NEW_LINE
	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_CBRA TOK_STAR TOK_ID TOK_DECIMAL ptr_stmt_list
	{
		struct si_arg_t *arg = $8;
		int err;

		/* Set new argument name */
		si_arg_name_set(arg, $6->name);
		
		/* Initialize argument */
		arg->pointer.num_elems = $3;
		arg->pointer.data_type = str_map_string_err(&si_arg_data_type_map, $1->name, &err);
		if (err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", $1->name);
		
		arg->pointer.constant_buffer_num = 1;
		arg->pointer.constant_offset = $7;


		/* Insert argument and free identifiers */
		si2bin_metadata_add_arg(si2bin_metadata, arg);
		si2bin_id_free($1);
		si2bin_id_free($6);
	} TOK_NEW_LINE
	| TOK_NEW_LINE
	;
	
val_stmt_list
	:
	{
		struct si_arg_t *arg;

		/* Create an argument with defaults*/
		arg = si_arg_create(si_arg_value, "arg");

		$$ = arg;
	}
	| val_stmt_list TOK_CONST
	{
		struct si_arg_t *arg = $1;

		/* set constarg field to true */
		arg->constarg = 1;
		
		/* Return argument */
		$$ = arg;
	}
	;

ptr_stmt_list
	:
	{
		struct si_arg_t *arg;

		/* Create an argument with defaults*/
		arg = si_arg_create(si_arg_pointer, "arg");
		arg->pointer.scope = si_arg_uav;
		arg->pointer.buffer_num = 12;
		arg->pointer.access_type = si_arg_read_write;
		
		$$ = arg;
	}
	| ptr_stmt_list TOK_ID
	{
		struct si_arg_t *arg = $1;
		struct si2bin_id_t *id = $2;
		int err;

		/* Translate access type */
		arg->pointer.access_type = str_map_string_err(&si_arg_access_type_map, id->name, &err);
		if (err)
			si2bin_yyerror_fmt("Unrecognized access type: %s", id->name);
		
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
	| ptr_stmt_list TOK_HL
	{
		struct si_arg_t *arg = $1;
	
		/* Set scope to hl */
		arg->pointer.scope = si_arg_hw_local;
		arg->pointer.buffer_num = 1;

		/* Return argument */
		$$ = arg;
	}
	| ptr_stmt_list TOK_CONST
	{
		struct si_arg_t *arg = $1;
	
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
		struct si2bin_inst_t *inst = $1;

		/* Generate code */
		si2bin_inst_gen(inst);
		elf_enc_buffer_write(si2bin_entry->text_section_buffer, inst->inst_bytes.bytes, inst->size);
		
		/* Dump Instruction Info */
		//si2bin_inst_dump(inst, stdout);
		si2bin_inst_free(inst);
	} TOK_NEW_LINE
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

		if (value >= si2bin_inner_bin->num_sgprs)
			si2bin_inner_bin->num_sgprs = value + 1;
		
		si2bin_id_free($1);
	}
	
	| TOK_VECTOR_REGISTER
	{
		int value;

		value = atoi($1->name + 1);
		$$ = si2bin_arg_create_vector_register(value); 
		
		if (value >= si2bin_inner_bin->num_vgprs)
			si2bin_inner_bin->num_vgprs = value + 1;

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

			if (high >= si2bin_inner_bin->num_sgprs)
				si2bin_inner_bin->num_sgprs = high + 1;
		}
		else if (!strcmp(id->name, "v"))
		{
			arg->type = si2bin_arg_vector_register_series;
			arg->value.vector_register_series.low = low;
			arg->value.vector_register_series.high = high;
			
			if (high >= si2bin_inner_bin->num_vgprs)
				si2bin_inner_bin->num_vgprs = high + 1;
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

		/* Get ID */
		id = $1;
		
		/* Create argument */
		arg = si2bin_arg_create_label(id->name);
		si2bin_id_free(id);

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
