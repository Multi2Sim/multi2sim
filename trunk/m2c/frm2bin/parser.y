%{
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/elf-encode.h>
#include <lib/util/list.h>

#include "id.h"
#include "frm2bin.h"
#include "arg.h"
#include "inst.h"
#include "inst-info.h"
#include "inner-bin.h"
#include "outer-bin.h"

/*
#include "stream.h"
#include "symbol.h"
*/

#define YYERROR_VERBOSE

%}

%union {
	int num;
  	struct frm_id_t *id;
  	struct frm2bin_inst_t *inst;
  	struct frm_label_t *label;
  	struct list_t *list;
  	struct frm_arg_t *arg;
  	struct frm_mod_t *mod;
  	struct frm2bin_pred_t *pred;
}

%token<pred> TOK_PRED
%token<id> TOK_PRED_REGISTER
%token<id> TOK_ZERO_REGISTER
%token<id> TOK_SCALAR_REGISTER
%token<id> TOK_SPECIAL_REGISTER
%token<id> TOK_CCOP
%token<num> TOK_DECIMAL
%token<id> TOK_HEX
%token<id> TOK_PT
%token<id> TOK_ARG_TYPE
%right<id> TOK_ID

%right TOK_COMMA
%right TOK_COLON
%token TOK_ADD
%right TOK_SEMICOLON
%left TOK_OBRA
%token TOK_CBRA
%token TOK_OPAR
%token TOK_CPAR
%token TOK_NEW_LINE
%left TOK_AMP
%token TOK_ABS
%token TOK_NEG
%token TOK_NUM
%token TOK_GLOBAL
%token TOK_ARGS
%token TOK_TEXT

%type<inst> rl_instr
%type<list> rl_arg_list
%type<arg> rl_operand
%type<arg> rl_arg

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
	: args_section
	{
		/* do nothing right now */
	}
	| text_section
	{
		/* do nothing right now */
		/* Process any tasks still left */
		/* si2bin_task_list_process(); */
		
		/* Clean up tasks and symbol table when finished parsing kernel */
		/* si2bin_task_list_done(); */
		/* si2bin_symbol_table_done(); */

		/* Set up new tasks and symbol table for next kernel */
		/* si2bin_task_list_init(); */
		/* si2bin_symbol_table_init(); */
	}
	;

global_section
	: TOK_GLOBAL TOK_ID TOK_NEW_LINE
	{
		struct frm_id_t *id = $2;

		frm2bin_inner_bin = frm2bin_inner_bin_create(id->name);
		//frm2bin_metadata = frm2bin_metadata_create();
		frm2bin_entry = frm2bin_inner_bin_entry_create();

		//frm2bin_metadata->uniqueid = frm2bin_uniqueid;

		frm2bin_inner_bin_add_entry(frm2bin_inner_bin, frm2bin_entry);

		frm2bin_outer_bin_add(frm2bin_outer_bin, frm2bin_inner_bin);

		//frm2bin_uniqueid ++;

		frm_id_free(id);
	}
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
	: TOK_ARG_TYPE TOK_NEW_LINE
	{
		int size;
		
		/* check the size of the current argument */
		/* format for tok_arg_type: .i32, .i64, .f32, .f64 */
		size = (atoi(($1->name) + 2)) / 4;
		
		/* increase the kernel argument total size */
		frm2bin_inner_bin->arg_totalSize += size;
		
		frm_id_free($1);
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
	: rl_instr TOK_SEMICOLON TOK_NEW_LINE
	{
		//struct FRM2bin_inst_t *inst = $1;

		/* Generate code */
		//frm2bin_inst_gen(inst);
		//elf_enc_buffer_write(frm2bin_entry->text_section_buffer, inst->inst_bytes.bytes, inst->size);
		
		/* Dump Instruction Info */
		//frm2bin_inst_dump(inst, stdout);
		//frm2bin_inst_free(inst);
		
		
		struct frm2bin_inst_t *inst = $1;
	      
		/* Generate code */
		/*
		frm_stream_add_inst(frm_out_stream, inst);
		*/
		frm2bin_inst_gen(inst);
		
		/* write instruction binary to the buffer */
		/* This will be deleted later */
		elf_enc_buffer_write(text_section_buffer, inst->inst_bytes.bytes, inst->size);
		
		/* write instruction binary to text section buffer */
		elf_enc_buffer_write(frm2bin_entry->text_section_buffer, inst->inst_bytes.bytes, inst->size);
		
		/* dump the instruction binary to the console */
		/*
		frm2bin_inst_dump(inst, stdout);
		*/
		
		frm2bin_inst_free(inst);
	}
	
	| TOK_NEW_LINE
;	

rl_instr
	: TOK_PRED TOK_ID rl_arg_list
  	{
  	  	struct frm2bin_inst_t *inst;
  	  	struct frm_id_t *id;
  	  	struct list_t *arg_list;
  	  	struct frm2bin_pred_t *pred;

  	  	/* Get arguments */
  	  	id = $2;
  	  	arg_list = $3;

  	  	/* create predicate */
  	  	pred = $1;

  	  	/* create instruction */
  	  	inst = frm2bin_inst_create(pred, id->name, arg_list);

  	  	/* Return instructions */
  	  	frm_id_free(id);
  	  	frm2bin_pred_free(pred);

  	  	$$ = inst;
  	}
  	/* maybe this should be improved later */
  	| TOK_ID rl_arg_list
  	{
  	  	struct frm2bin_inst_t *inst;
  	  	struct frm_id_t *id;
  	  	struct list_t *arg_list;

  	  	/* Get arguments */
  	  	id = $1;
  	  	arg_list = $2;

  	  	/* create instruction */
  	  	inst = frm2bin_inst_create(NULL, id->name, arg_list);

  	  	/* Return instructions */
  	  	frm_id_free(id);
  	  	$$ = inst;
  	}
;


rl_arg_list
	: /* Empty argument list */
  	{
  	  	$$ = NULL;
  	}

  	| rl_arg
  	{
  	  	struct list_t *arg_list;

  	  	/* create */
  	  	arg_list = list_create();

  	  	/* Initialize */
  	  	list_add(arg_list, $1);

  	  	/* Return the arg list */
  	  	$$ = arg_list;
  	}

  	| rl_arg TOK_COMMA rl_arg_list
  	{
  		/* Add argument to head of the list_t in $3 */
  	  	list_insert($3, 0, $1);
  	  	$$ = $3;
  	}
;

rl_operand
	: TOK_SCALAR_REGISTER
  	{
  	  	$$ = frm_arg_create_scalar_register($1->name);
  	  	frm_id_free($1);
  	}

  	| TOK_SPECIAL_REGISTER
  	{
  	  	$$ = frm_arg_create_special_register($1->name);
  	  	frm_id_free($1);
  	}

  	| TOK_ZERO_REGISTER
  	{
		$$ = frm_arg_create_zero_register($1->name);
  	  	frm_id_free($1);
  	}

  	| TOK_PRED_REGISTER
  	{
  	  	$$ = frm_arg_create_predicate_register($1->name);
  	  	frm_id_free($1);
  	}

  	| TOK_DECIMAL
  	{
  	  	$$ = frm_arg_create_literal($1);
  	}

  	| TOK_HEX
  	{
  	  	int value;

  	  	sscanf($1->name, "%x", &value);
  	  	$$ = frm_arg_create_literal(value);
  	  	frm_id_free($1);
  	}
;

rl_arg
	: rl_operand
  	{
  	  	$$ = $1;
  	}

  	/* for constant memory, eg. c[0x1][offset] 
  	* c[0x1][reg + offset] will be supported later */
  	| TOK_ID TOK_OBRA TOK_HEX TOK_CBRA TOK_OBRA TOK_HEX TOK_CBRA
  	{
  	  	int bank_idx;
  	  	/* offset within a bank */
  	  	int offset;
  	  	int negative;

  	  	if (!strcmp($1->name, "c") || !strcmp($1->name, "-c"))
  	  	{
  	  		/* extract the hex value and assign it to the variable */
  	  	  	sscanf($3->name, "%x", &bank_idx);
  	  	  	sscanf($6->name, "%x", &offset);
  	  	  	negative = strcmp($1->name, "c") ? 1 : 0;

  	  	  	$$ = frm_arg_create_const_maddr(bank_idx, offset, negative);
  	  	}
  	  	else 
  	  	{
  	  		frm2bin_yyerror_fmt("invalid constant memory identifier: %s", 
				$1->name);
  	  	}

  	  	frm_id_free($1);
  	  	frm_id_free($3);
  	  	frm_id_free($6);
  	}

  	/* global memory [reg], even shared memory? */
  	| TOK_OBRA TOK_SCALAR_REGISTER TOK_CBRA
  	{
  	  	int reg_idx;
  	  	/* start from the 2nd character, 1st one is "R" */
  	  	reg_idx = atoi(($2->name) + 1);
  	  	$$ = frm_arg_create_glob_maddr(reg_idx, 0);

  	  	frm_id_free($2);
  	}
  	
  	/* global memory [reg + offset], even shared memory? */
  	| TOK_OBRA TOK_SCALAR_REGISTER TOK_ADD TOK_HEX TOK_CBRA
  	{
  	  	int reg_idx;
  	  	int offset;
  	  	/* start from the 2nd character, 1st one is "R" */
  	  	reg_idx = atoi(($2->name) + 1);
  	  	sscanf($4->name, "%x", &offset);
  	  	$$ = frm_arg_create_glob_maddr(reg_idx, offset);

  	  	frm_id_free($2);
  	  	frm_id_free($4);
  	}
  	
  	/* shared memory [offset], even global memory? */
  	| TOK_OBRA TOK_HEX TOK_CBRA
  	{
  		int offset;
  		sscanf($2->name, "%x", &offset);
  		$$ = frm_arg_create_shared_maddr(-1, offset);
  		
  		frm_id_free($2);
  	}

  	/* for ISETP instruction */
  	| TOK_PT
  	{
  	  	$$ = frm_arg_create_pt($1->name);
  	  	
  	  	frm_id_free($1);
  	}
  	
  	/* CCOP, conditional code operation? */
  	| TOK_CCOP
  	{
  		$$ = frm_arg_create_ccop($1->name);
  		
  		frm_id_free($1);
  	}

  	/* identifer  */
  	| TOK_ID
  	{
  		/* not supported yet */
  	  	frm2bin_yyerror_fmt("unsupported TOK_ID: %s", $1->name);
  	  	frm_id_free($1);

		/*
  	  	struct frm_arg_t *arg;
  	  	struct frm_id_t *id;
  	  	struct frm_symbol_t *symbol;

  	  	id = $1;
  	  	symbol = hash_talbe_get(frm_symbol_table, id->name);
  	  	if(!symbol)
  	  	{
  	  	  symbol = frm_symbol_create(id->name);
  	  	  hash_talbe_insert(frm_symbol_talbe, id->name, symbol);
  	  	}

  	  	arg = frm_arg_create_
		*/
  	}
;


%%
