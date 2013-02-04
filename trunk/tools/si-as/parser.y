%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "label.h"
#include "inst.h"
#include "inst-arg.h"
#include "string.h"
#include "task.h"
#include "stream.h"

#define YYERROR_VERBOSE

//int yydebug = 1;

extern FILE *yyin;
extern char* yytext;

extern int yylex(void);
extern void yyset_in(FILE * in_str);

int line_num = 1;
struct si_stream_t *stream;
void yyerror(const char *s);

%}

%union {
	int num;
	char *sym;
	struct si_inst_t *inst;
	struct si_label_t *label;
	struct list_t *list;
	struct si_inst_arg_t *arg;
}

 



%token<sym> TOK_GENERAL_REGISTER
%token<sym> TOK_SPECIAL_REGISTER
%token<num> TOK_DECIMAL
%right TOK_COMMA
%right TOK_COL
%token<sym> TOK_HEX
%right<sym> TOK_ID
%left TOK_OBRA
%token TOK_CBRA
%token TOK_OPAR
%token TOK_CPAR
%left TOK_AMP

%type<inst> rule_instr
%type<list> rule_arg_list
%type<arg> rule_arg
%type<label> rule_label
%type<sym> rule_line
%type<arg> rule_waitcnt_elem
%type<arg> rule_waitcnt_arg

%%


rule_input
	: 
	| rule_input rule_line
	{
	}
;

rule_line
	: '\n'
	{
	}
	| rule_instr '\n'
	{
		si_stream_add_inst(stream, $1);
		line_num++;
		//si_inst_free($1);
		printf("---------- Line %d ----------\n", line_num);
	}
	| rule_label '\n'
	{
		$1->offset = si_stream_get_offset(stream);
		line_num++;
		printf("---------- Line %d ----------\n", line_num);
	} 
;

rule_label
	: TOK_ID TOK_COL
	{ 
		struct si_label_t *label;

		printf("BISON LABEL: %s\n", $1);
		label = si_label_create($1, 0);
		if (!si_label_table_insert(label))
			printf("ERROR: si_label_table insert failed to insert key %s!\n", $1);
		
		$$ = label;
	}

rule_instr
	: TOK_ID
	{
		printf("BISON ID: %s\n", $1);
		struct si_inst_t *inst = si_inst_create($1);
		si_inst_dump(inst, stdout);
		$$ = inst;
	}
	| TOK_ID rule_arg_list 
	{
		struct si_inst_t *inst = si_inst_create($1);
		  
		/* Set the inst objects list to arg_list */
		inst -> arg_list = $2;
		  
		si_inst_dump(inst, stdout);
		$$ = inst;
	}
;

rule_arg_list
	: rule_arg
	{
		struct list_t *arg_list;
		
		/* Create */
		arg_list = list_create();
		
		/* Initialize */
		list_add(arg_list, $1);
		
		/* Return the arg list */
		$$ = arg_list;
		
	}
    | rule_arg TOK_COMMA rule_arg_list
	{
		/* Add argument to head of list_t in $3 */
		list_insert($3, 0, $1);
		
		/* Return the arg list */
		$$ = $3;

	}
;

rule_arg
	: TOK_GENERAL_REGISTER 
	{
		struct si_inst_arg_t *arg;
		
		/* Create argument */
		arg = si_inst_arg_create(); 
		
		/* TOK_GENERAL_REGISTER is m0, v14, s9 etc.
		   so we need to figure out 
		   what kind of TOK_GENERAL_REGISTER it is and 
		   split the string to get the 
		   register identifier. */
		
		/* Initialize */
		if ($1[0] == 's')
		{
			arg->type = si_inst_arg_scalar_register;
			arg->value.scalar_register.id = atoi($1 + 1);
		}
		else if ($1[0] == 'm')
		{
			arg->type = si_inst_arg_mtype_register;
			arg->value.scalar_register.id = atoi($1 + 1);
		}
		else if ($1[0] == 'v')
		{
			arg->type = si_inst_arg_vector_register;
			arg->value.scalar_register.id = atoi($1 + 1);
		}
		else
		{
			printf("Unknown TOK_GENERAL_REGISTER type detected!\n");
		}
	
		free($1);
	
		/* Return created argument */
		$$ = arg;
	}
	| TOK_SPECIAL_REGISTER
	{
		struct si_inst_arg_t *arg;
		
		/* Create argument */
		arg = si_inst_arg_create(); 
		
		/* Initialize */
		arg->type = si_inst_arg_special_register;
		
		if (!strcmp($1, "vcc"))
			arg->value.special_register.type = si_inst_arg_special_register_vcc;
		else if (!strcmp($1, "scc"))
			arg->value.special_register.type = si_inst_arg_special_register_scc;
		else
			fatal("%s: invalid special register", $1);
		
		free($1);
		
		/* Return created argument */
		$$ = arg;
	}
	| TOK_ID TOK_OBRA TOK_DECIMAL TOK_COL TOK_DECIMAL TOK_CBRA  
	{
		struct si_inst_arg_t *arg;
		
		/* Create argument */
		arg = si_inst_arg_create(); 
		
		/* Initialize */
		arg->type = si_inst_arg_register_range;
		
		arg->value.register_range.id_low = $3;
		arg->value.register_range.id_high = $5;
		
		free($1);
		
		/* Return created argument */
		$$ = arg;
	}	
	| TOK_DECIMAL
	{
		struct si_inst_arg_t *arg;
		
		/* Create argument */
		arg = si_inst_arg_create(); 
		
		/* Initialize */
		arg->type = si_inst_arg_literal;
		
		arg->value.literal.val = $1;
		
		/* Return created argument */
		$$ = arg;
	}
	| TOK_HEX
	{
		unsigned int hexVal;
		
		struct si_inst_arg_t *arg;
		
		/* Create argument */
		arg = si_inst_arg_create(); 
		
		/* Initialize */
		arg->type = si_inst_arg_literal;
		sscanf($1, "%x", &hexVal);
		arg->value.literal.val = hexVal;
		
		/* Convert HEX value to int and set in val*/
			
		free($1);
		
		/* Return created argument */
		$$ = arg;
	}
	| TOK_DECIMAL TOK_ID TOK_ID TOK_COL TOK_OBRA TOK_ID TOK_COMMA TOK_ID TOK_CBRA
    {
		struct si_inst_arg_t *arg;
		
		/* Create argument */
		arg = si_inst_arg_create(); 	
			
		/* Initialize */		
		arg->type = si_inst_arg_format;
		
		if (!strcmp($2, "offen"))
			arg->value.format.offen = 1;
		
		arg->value.format.data_format = strdup($6);
		arg->value.format.num_format = strdup($8);
		arg->value.format.offset = $1;
		
		free($3);
		
		$$ = arg;
	}
	| TOK_ID
	{
	
		struct si_inst_arg_t *arg;
		struct si_task_t *task;
		
		/* Create argument */
		arg = si_inst_arg_create(); 
		arg->type = si_inst_arg_label;
		
		/* Insert with an offset of 0, the binary generation function
		 * must check for this, and update it with the OUTPUT file's offset */
		task = si_task_create($1, si_stream_get_offset(stream));
		si_task_list_add(task);
		
		/* Return created argument */
		$$ = arg;
	}
	| rule_waitcnt_arg
	{
		/* The return value is given by 'rule_waitcnt_arg's definition */
	}
;

rule_waitcnt_arg
	: rule_waitcnt_elem
	| rule_waitcnt_elem TOK_AMP rule_waitcnt_arg
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
		
		si_inst_arg_free($1);
		
		$$ = $3;
		
	}
;

rule_waitcnt_elem
	: TOK_ID TOK_OPAR TOK_DECIMAL TOK_CPAR
	{
		struct si_inst_arg_t *arg;
		
		/* Create argument */
		arg = si_inst_arg_create(); 
		
		/* Initialize */
		arg->type = si_inst_arg_waitcnt;
		
		if (!strcmp($1, "vmcnt"))
		{
			arg->value.wait_cnt.vmcnt_active = 1;
			arg->value.wait_cnt.vmcnt_value = $3;
		}
		else if (!strcmp($1, "expcnt"))
		{
			arg->value.wait_cnt.expcnt_active = 1;
			arg->value.wait_cnt.expcnt_value = $3;
		}
		else if (!strcmp($1, "lgkmcnt"))
		{
			arg->value.wait_cnt.lgkmcnt_active = 1;
			arg->value.wait_cnt.lgkmcnt_value = $3;
		}
		
		free($1);
		
		/* Return created argument */
		$$ = arg;
		$$ = arg;
	}
;

%%

#define CLEAR_BITS_64(X, HI, LO) \
	((unsigned long long) (X) & (((1ull << (LO)) - 1) \
	| ~((1ull << ((HI) + 1)) - 1)))

#define TRUNCATE_BITS_64(X, NUM) \
	((unsigned long long) (X) & ((1ull << (NUM)) - 1))

#define SET_BITS_64(X, HI, LO, V) \
	(CLEAR_BITS_64((X), (HI), (LO)) | \
	(TRUNCATE_BITS_64((V), (HI) - (LO) + 1) << (LO)))
	
int main(int argc, char **argv) 
{
	FILE *f;
	char *file_name;

	/* Check syntax */
	if (argc != 2)
		fatal("syntax: %s <file>", *argv);

	/* Open input file */
	file_name = argv[1];
	f = fopen(file_name, "r");
	if (!f)
		fatal("%s: cannot open input file", file_name);
	
	/* Open output file */
	stream = si_stream_create("out.bin");
		
	/* Create the pending task list */
	si_task_list_init();

	/* Create the hash-table which will contain label offsets*/
	si_label_table_init();

	/* Parse input */
	yyset_in(f);
	yyparse();
		
	printf("-------------Dumping Label Table-------------\n");
	si_label_table_dump(stdout);
	printf("---------------------------------------------\n");
		
	printf("------------Dumping Pending Tasks------------\n");
	si_task_list_dump(stdout);
	printf("---------------------------------------------\n");
		
	si_task_list_done();
	si_label_table_done();
	si_stream_close(stream);
	
	return 0;
}

void yyerror(const char *s)
{

	printf("ERROR Message: %s\n", s);
	printf("Text input: %s\n", yytext);
	exit(-1);    
}

