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

#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>

#include "arg.h"
#include "dis-inst.h"
#include "id.h"
#include "main.h"
#include "stream.h"
#include "string.h"
#include "symbol.h"
#include "task.h"


#define YYERROR_VERBOSE

extern FILE* yyin;
extern char* yytext;

extern int yylex(void);
void yyerror(const char *s);

int yyget_lineno(void);
void yyset_lineno(int line_number);

struct si_stream_t *stream;

%}

%union {
	int num;
	float num_float;
	struct si_id_t *id;
	struct si_dis_inst_t *inst;
	struct si_label_t *label;
	struct list_t *list;
	struct si_arg_t *arg;
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

%type<inst> rl_instr
%type<list> rl_arg_list
%type<arg> rl_operand
%type<arg> rl_arg
%type<arg> rl_maddr_qual
%type<arg> rl_waitcnt_elem
%type<arg> rl_waitcnt_arg

%%


rl_input
	: 
	| rl_line rl_input
;

rl_line
	: TOK_NEW_LINE
	{
		yyset_lineno(yyget_lineno() + 1); 
	}

	| rl_instr TOK_NEW_LINE
	{
		struct si_dis_inst_t *inst;

		/* Get instruction */
		inst = $1;

		/* Print instruction */
		si_dis_inst_gen(inst);
		si_dis_inst_dump(inst, stdout);
		//si_stream_add_inst(stream, $1);
		
		/* Free instruction */
		si_dis_inst_free(inst);
		
		/* Next line */
		yyset_lineno(yyget_lineno() + 1); 
	}

	| rl_label TOK_NEW_LINE
	{
		/* Next line */
		yyset_lineno(yyget_lineno() + 1); 
	} 
;

rl_label
	: TOK_ID TOK_COLON
	{
		struct si_id_t *id = $1;
		struct si_symbol_t *symbol;

		/* Check if symbol exists */
		symbol = hash_table_get(si_symbol_table, id->name);
		if (symbol && symbol->defined)
			yyerror_fmt("multiply defined label: %s", id->name);

		/* Create if it does not exists */
		if (!symbol)
		{
			symbol = si_symbol_create(id->name);
			hash_table_insert(si_symbol_table, id->name, symbol);
		}

		/* Define symbol */
		symbol->defined = 1;
		symbol->value = si_out_stream->offset;

		/* End */
		si_id_free(id);
	}

rl_instr
	: TOK_ID rl_arg_list 
	{
		struct si_dis_inst_t *inst;
		struct si_id_t *id;
		struct list_t *arg_list;

		/* Get arguments */
		id = $1;
		arg_list = $2;
		
		/* Create instruction */
		inst = si_dis_inst_create(id->name, arg_list);

		/* Return instructions */
		si_id_free(id);
		$$ = inst;
	}
;

rl_arg_list
	:  /* Empty argument list */
	{
		$$ = NULL;
	}

	| rl_arg
	{
		struct list_t *arg_list;
		
		/* Create */
		arg_list = list_create();
		
		/* Initialize */
		list_add(arg_list, $1);
		
		/* Return the arg list */
		$$ = arg_list;
	}

	| rl_arg TOK_COMMA rl_arg_list
	{
		/* Add argument to head of list_t in $3 */
		list_insert($3, 0, $1);
		
		/* Return the arg list */
		$$ = $3;

	}
;


rl_operand
	: TOK_SCALAR_REGISTER
	{
		$$ = si_arg_create_scalar_register($1->name);
		si_id_free($1);
	}
	
	| TOK_VECTOR_REGISTER
	{
		$$ = si_arg_create_vector_register($1->name); 
		si_id_free($1);
	}
	
	| TOK_SPECIAL_REGISTER
	{
		$$ = si_arg_create_special_register($1->name); 
		si_id_free($1);
	}
	
	| TOK_DECIMAL
	{
		$$ = si_arg_create_literal($1);
	}

	| TOK_HEX
	{
		int value;

		sscanf($1->name, "%x", &value);
		$$ = si_arg_create_literal(value); 
		si_id_free($1);
	}

	| TOK_FLOAT
	{
		$$ = si_arg_create_literal_float($1);
	}	


rl_arg
	: rl_operand
	{
		$$ = $1;
	}

	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_COLON TOK_DECIMAL TOK_CBRA  
	{
		struct si_arg_t *arg;
		struct si_id_t *id;

		int low;
		int high;

		/* Read arguments */
		id = $1;
		low = $3;
		high = $5;
		
		/* Create argument */
		arg = si_arg_create(); 
		
		/* Initialize */
		if (!strcmp(id->name, "s"))
		{
			arg->type = si_arg_scalar_register_series;
			arg->value.scalar_register_series.low = low;
			arg->value.scalar_register_series.high = high;
		}
		else if (!strcmp(id->name, "v"))
		{
			arg->type = si_arg_vector_register_series;
			arg->value.vector_register_series.low = low;
			arg->value.vector_register_series.high = high;
		}
		else
		{
			yyerror_fmt("invalid register series: %s", id->name);
		}
		
		/* Return created argument */
		si_id_free(id);
		$$ = arg;
	}

	| rl_operand rl_maddr_qual TOK_FORMAT TOK_COLON TOK_OBRA TOK_ID TOK_COMMA TOK_ID TOK_CBRA
	{
		struct si_arg_t *arg;
		struct si_arg_t *soffset;
		struct si_arg_t *qual;

		struct si_id_t *id_data_format;
		struct si_id_t *id_num_format;

		/* Read arguments */
		soffset = $1;
		qual = $2;
		id_data_format = $6;
		id_num_format = $8;

		/* Create argument */
		arg = si_arg_create_maddr(soffset, qual,
			id_data_format->name, id_num_format->name);	
			
		/* Return */
		si_id_free(id_data_format);
		si_id_free(id_num_format);
		$$ = arg;
	}

	| TOK_ID
	{
		struct si_arg_t *arg;
		struct si_id_t *id;
		struct si_symbol_t *symbol;

		/* Get symbol or create it */
		id = $1;
		symbol = hash_table_get(si_symbol_table, id->name);
		if (!symbol)
		{
			symbol = si_symbol_create(id->name);
			hash_table_insert(si_symbol_table, id->name, symbol);
		}
		
		/* Create argument */
		arg = si_arg_create_label(symbol);

		/* Return */
		si_id_free(id);
		$$ = arg;
	}

	| rl_waitcnt_arg
	{
		/* The return value is given by 'rl_waitcnt_arg's definition */
	}
;

rl_maddr_qual

	: 
	{
		$$ = si_arg_create_maddr_qual();
	}

	| rl_maddr_qual TOK_OFFEN
	{
		struct si_arg_t *qual = $1;

		assert(qual->type == si_arg_maddr_qual);
		if (qual->value.maddr_qual.offen)
			yyerror("redundant qualifier 'offen'");
		qual->value.maddr_qual.offen = 1;
		$$ = qual;
	}

	| rl_maddr_qual TOK_IDXEN
	{
		struct si_arg_t *qual = $1;

		assert(qual->type == si_arg_maddr_qual);
		if (qual->value.maddr_qual.idxen)
			yyerror("redundant qualifier 'idxen'");
		qual->value.maddr_qual.idxen = 1;
		$$ = qual;
	}

	| rl_maddr_qual TOK_OFFSET TOK_COLON TOK_DECIMAL
	{
		struct si_arg_t *qual = $1;
		int offset = $4;

		assert(qual->type == si_arg_maddr_qual);
		qual->value.maddr_qual.offset = offset;
		/* FIXME - check range of 'offset' */
		$$ = qual;
	}

rl_waitcnt_arg

	: rl_waitcnt_elem

	| rl_waitcnt_elem TOK_AMP rl_waitcnt_arg
	{
		/* $1 returns a new waitcnt arg object, and
   		   $2 returns a new object as well, pick one
		   to merge the two waitcnt args and free the
		   other.								   */
		   
		$3->value.wait_cnt.vmcnt_active = $1->value.wait_cnt.vmcnt_active;
		$3->value.wait_cnt.vmcnt_value = $1->value.wait_cnt.vmcnt_value;		
		
		$3->value.wait_cnt.expcnt_active = $1->value.wait_cnt.expcnt_active;
		$3->value.wait_cnt.expcnt_value = $1->value.wait_cnt.expcnt_value;	
		
		$3->value.wait_cnt.lgkmcnt_active = $1->value.wait_cnt.lgkmcnt_active;
		$3->value.wait_cnt.lgkmcnt_value = $1->value.wait_cnt.lgkmcnt_value;	
		
		si_arg_free($1);
		
		$$ = $3;
		
	}
;

rl_waitcnt_elem

	: TOK_ID TOK_OPAR TOK_DECIMAL TOK_CPAR
	{
		struct si_arg_t *arg;
		struct si_id_t *id;

		/* Read arguments */
		id = $1;
		
		/* Create argument */
		arg = si_arg_create(); 
		
		/* Initialize */
		arg->type = si_arg_waitcnt;
		
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
		si_id_free(id);
		$$ = arg;
	}
;

%%
