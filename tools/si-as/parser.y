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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "arg.h"
#include "dis-inst.h"
#include "id.h"
#include "label.h"
#include "main.h"
#include "stream.h"
#include "string.h"
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
	struct si_id_t *id;
	struct si_dis_inst_t *inst;
	struct si_label_t *label;
	struct list_t *list;
	struct si_arg_t *arg;
}

 



%token<id> TOK_GENERAL_REGISTER
%token<id> TOK_SPECIAL_REGISTER
%token<num> TOK_DECIMAL
%right TOK_COMMA
%right TOK_COL
%token<id> TOK_HEX
%right<id> TOK_ID
%left TOK_OBRA
%token TOK_CBRA
%token TOK_OPAR
%token TOK_CPAR
%token TOK_NEW_LINE
%left TOK_AMP

%type<inst> rl_instr
%type<list> rl_arg_list
%type<arg> rl_arg
%type<label> rl_label
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
	: TOK_ID TOK_COL
	{ 
		struct si_label_t *label;
		struct si_id_t *id;

		/* Get arguments */
		id = $1;

		/* Create label */
		label = si_label_create(id->name, 0);
		if (!si_label_table_insert(label))
			fatal("failed to insert label '%s'", id->name);
		
		/* Return label */
		si_id_free(id);
		$$ = label;
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

rl_arg
	: TOK_GENERAL_REGISTER 
	{
		struct si_arg_t *arg;
		struct si_id_t *id;

		/* Get arguments */
		id = $1;
		
		/* Create argument */
		arg = si_arg_create(); 
		
		/* TOK_GENERAL_REGISTER is m0, v14, s9 etc.
		   so we need to figure out 
		   what kind of TOK_GENERAL_REGISTER it is and 
		   split the string to get the 
		   register identifier. */
		
		/* Initialize */
		if (id->name[0] == 's')
		{
			arg->type = si_arg_scalar_register;
			arg->value.scalar_register.id = atoi(id->name + 1);
		}
		else if (id->name[0] == 'm')
		{
			arg->type = si_arg_mtype_register;
			arg->value.scalar_register.id = atoi(id->name + 1);
		}
		else if (id->name[0] == 'v')
		{
			arg->type = si_arg_vector_register;
			arg->value.scalar_register.id = atoi(id->name + 1);
		}
		else
		{
			yyerror_fmt("invalid register: %s", id->name);
		}
	
		/* Return */
		si_id_free(id);
		$$ = arg;
	}

	| TOK_SPECIAL_REGISTER
	{
		struct si_arg_t *arg;
		struct si_id_t *id;

		/* Read arguments */
		id = $1;
		
		/* Create argument */
		arg = si_arg_create(); 
		
		/* Initialize */
		arg->type = si_arg_special_register;
		
		if (!strcmp(id->name, "vcc"))
			arg->value.special_register.type = si_arg_special_register_vcc;
		else if (!strcmp(id->name, "scc"))
			arg->value.special_register.type = si_arg_special_register_scc;
		else
			fatal("%s: invalid special register", id->name);
		
		/* Return */
		si_id_free(id);
		$$ = arg;
	}

	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_COL TOK_DECIMAL TOK_CBRA  
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

	| TOK_DECIMAL
	{
		struct si_arg_t *arg;
		
		/* Create argument */
		arg = si_arg_create(); 
		
		/* Initialize */
		arg->type = si_arg_literal;
		
		arg->value.literal.val = $1;
		
		/* Return created argument */
		$$ = arg;
	}

	| TOK_HEX
	{
		struct si_arg_t *arg;
		struct si_id_t *id;
		
		unsigned int value;

		/* Read arguments */
		id = $1;
		
		/* Create argument */
		arg = si_arg_create(); 
		
		/* Initialize */
		arg->type = si_arg_literal;
		sscanf(id->name, "%x", &value);
		arg->value.literal.val = value;
		
		/* Return */
		si_id_free(id);
		$$ = arg;
	}

	| TOK_DECIMAL TOK_ID TOK_ID TOK_COL TOK_OBRA TOK_ID TOK_COMMA TOK_ID TOK_CBRA
	{
		struct si_arg_t *arg;
		struct si_id_t *id_offen;
		struct si_id_t *id_format;
		struct si_id_t *id_data_format;
		struct si_id_t *id_num_format;

		/* Read arguments */
		id_offen = $2;
		id_format = $3;
		id_data_format = $6;
		id_num_format = $8;
		
		/* Create argument */
		arg = si_arg_create(); 	
			
		/* Initialize */		
		arg->type = si_arg_format;
		if (!strcmp(id_offen->name, "offen"))
			arg->value.format.offen = 1;
		
		/* Return */
		si_id_free(id_offen);
		si_id_free(id_format);
		si_id_free(id_data_format);
		si_id_free(id_num_format);
		$$ = arg;
	}

	| TOK_ID
	{
		struct si_arg_t *arg;
		struct si_task_t *task;
		struct si_id_t *id;

		/* Read arguments */
		id = $1;
		
		/* Create argument */
		arg = si_arg_create(); 
		arg->type = si_arg_label;
		
		/* Insert with an offset of 0, the binary generation function
		 * must check for this, and update it with the OUTPUT file's offset */
		task = si_task_create(id->name, si_stream_get_offset(stream));
		si_task_list_add(task);

		/* Return */
		si_id_free(id);
		$$ = arg;
	}

	| rl_waitcnt_arg
	{
		/* The return value is given by 'rl_waitcnt_arg's definition */
	}
;

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
