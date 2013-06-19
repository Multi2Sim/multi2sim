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

%type<inst> rl_instr
%type<list> rl_arg_list
%type<arg> rl_operand
%type<arg> rl_arg

%%

rl_input
	:  /* empty */
	| rl_line rl_input
;

rl_line
	: TOK_NEW_LINE
	| rl_instr  TOK_SEMICOLON TOK_NEW_LINE
	{
	      struct frm2bin_inst_t *inst = $1;
	      
	      /* Generate code */
	      /*
	      frm_stream_add_inst(frm_out_stream, inst);
	      */
	      frm2bin_inst_gen(inst);
	      
	      /* write instruction binary to the buffer */
	      elf_enc_buffer_write(text_section_buffer, inst->inst_bytes.bytes, inst->size);
	
	
	      /* dump the instruction binary to the console */
	      /*
	      frm2bin_inst_dump(inst, stdout);
	      */
	
	      frm2bin_inst_free(inst);
	}
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
  	
  	|rl_arg TOK_ADD rl_arg_list
  	{
  		/* case: global memory [reg_idx + offset] */
  		list_insert($3, 0, $1);
  		$$ = $3;
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

  	  	if (!strcmp($1->name, "c"))
  	  	{
  	  		/* extract the hex value and assign it to the variable */
  	  	  	sscanf($3->name, "%x", &bank_idx);
  	  	  	sscanf($6->name, "%x", &offset);

  	  	  	$$ = frm_arg_create_const_maddr(bank_idx, offset);
  	  	}
  	  	else 
  	  	{
  	  		frm2bin_yyerror_fmt("invalid constant memory identifer: %s", 
				$1->name);
  	  	}

  	  	frm_id_free($1);
  	  	frm_id_free($3);
  	  	frm_id_free($6);
  	}

  	/* global memory [reg] */
  	| TOK_OBRA TOK_SCALAR_REGISTER TOK_CBRA
  	{
  	  	int reg_idx;
  	  	/* start from the 2nd character, 1st one is "R" */
  	  	reg_idx = atoi(($2->name) + 1);
  	  	$$ = frm_arg_create_glob_maddr_reg(reg_idx);

  	  	frm_id_free($2);
  	}
  	
  	/* global memory [reg + offset], reg part */
  	| TOK_OBRA TOK_SCALAR_REGISTER 
  	{
  	  	int reg_idx;
  	  	/* start from the 2nd character, 1st one is "R" */
  	  	reg_idx = atoi(($2->name) + 1);
  	  	$$ = frm_arg_create_glob_maddr_reg(reg_idx);

  	  	frm_id_free($2);
  	}
  	
  	/* global memory [reg + offset], offset part */
  	| TOK_HEX TOK_CBRA
  	{
  	  	int value;

  	  	sscanf($1->name, "%x", &value);
  		$$ = frm_arg_create_glob_maddr_offset(value);
  		
  		frm_id_free($1);
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
