%{

#define YYDEBUG 1

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>
#include <llvm-c/BitWriter.h>

#include "val.h"
#include "type.h"
#include "init-list-elem.h"
#include "symbol.h"
#include "parser.h"
#include "cl2llvm.h"


extern LLVMBuilderRef cl2llvm_builder;
extern LLVMModuleRef cl2llvm_module;
extern LLVMValueRef cl2llvm_function;
extern LLVMBasicBlockRef cl2llvm_basic_block;

extern int temp_var_count;
extern char temp_var_name[50];

int block_count;
char block_name[50];

struct hash_table_t *cl2llvm_symbol_table;

%}

%union {
	long int const_int_val;
	unsigned long long const_int_val_ull;
	double  const_float_val;
	char * identifier;
	struct cl2llvm_type_t *llvm_type_ref;
	struct cl2llvm_val_t *llvm_value_ref;
	struct list_t * init_list;
	LLVMBasicBlockRef basic_block_ref;
}

%token<identifier>  TOK_ID
%token<const_int_val> TOK_CONST_INT
%token<const_int_val> TOK_CONST_INT_U
%token<const_int_val> TOK_CONST_INT_L
%token<const_int_val> TOK_CONST_INT_UL
%token<const_int_val_ull> TOK_CONST_INT_LL
%token<const_int_val_ull> TOK_CONST_INT_ULL
%token<const_float_val> TOK_CONST_DEC
%token<const_float_val> TOK_CONST_DEC_H
%token<const_float_val> TOK_CONST_DEC_F
%token<const_float_val> TOK_CONST_DEC_L
%token TOK_CONST_VAL
%token TOK_STRING
%token TOK_COMMA
%token TOK_SEMICOLON
%token TOK_ELLIPSIS
%right 	TOK_EQUAL TOK_ADD_EQUAL TOK_MINUS_EQUAL
	TOK_MULT_EQUAL TOK_DIV_EQUAL TOK_MOD_EQUAL TOK_AND_EQUAL
	TOK_OR_EQUAL TOK_EXCLUSIVE_EQUAL TOK_SHIFT_RIGHT_EQUAL 
	TOK_SHIFT_LEFT_EQUAL
%right	TOK_CONDITIONAL TOK_COLON
%left	TOK_LOGICAL_OR
%left	TOK_LOGICAL_AND
%left	TOK_BITWISE_OR
%left	TOK_BITWISE_EXCLUSIVE
%left	TOK_BITWISE_AND
%left	TOK_EQUALITY TOK_INEQUALITY
%left	TOK_GREATER TOK_LESS TOK_GREATER_EQUAL TOK_LESS_EQUAL
%left	TOK_SHIFT_RIGHT TOK_SHIFT_LEFT
%left 	TOK_PLUS TOK_MINUS
%left 	TOK_MULT TOK_DIV TOK_MOD
%right	TOK_PREFIX TOK_LOGICAL_NEGATE TOK_BITWISE_NOT
%token 	TOK_CURLY_BRACE_CLOSE
%token 	TOK_CURLY_BRACE_OPEN
%left 	TOK_PAR_OPEN TOK_PAR_CLOSE TOK_BRACKET_OPEN
	TOK_BRACKET_CLOSE TOK_POSTFIX TOK_STRUCT_REF TOK_STRUCT_DEREF
%token TOK_DECREMENT
%token TOK_INCREMENT
%token TOK_AUTO
%token TOK_BOOL
%token TOK_BREAK
%token TOK_CASE
%token TOK_CHAR
%token TOK_CHARN
%token TOK_CONSTANT
%token TOK_CONST
%token TOK_CONTINUE
%token TOK_DEFAULT
%token TOK_DO
%token TOK_DOUBLE
%token TOK_DOUBLE_LONG
%token TOK_DOUBLEN
%token TOK_ENUM
%token TOK_EVENT_T
%token TOK_EXTERN
%token TOK_FLOAT
%token TOK_FLOATN
%token TOK_FOR
%token TOK_GLOBAL
%token TOK_GOTO
%token TOK_HALF
%token TOK_IF
%left TOK_ELSE
%token TOK_IMAGE2D_T
%token TOK_IMAGE3D_T
%token TOK_IMAGE2D_ARRAY_T
%token TOK_IMAGE1D_T
%token TOK_IMAGE1D_BUFFER_T
%token TOK_IMAGE1D_ARRAY_T
%token TOK_INLINE
%token TOK_INT
%token TOK_INT_LONG
%token TOK_LONG_LONG
%token TOK_INTN
%token TOK_INTPTR_T
%token TOK_KERNEL
%token TOK_LOCAL
%token TOK_LONG
%token TOK_LONGN
%token TOK_PRIVATE
%token TOK_PTRDIFF_T
%token TOK_READ_ONLY
%token TOK_READ_WRITE
%token TOK_REGISTER
%token TOK_RETURN
%token TOK_SAMPLER_T
%token TOK_SHORT
%token TOK_SHORTN
%token TOK_SIGNED
%token TOK_SIZEOF
%token TOK_SIZE_T
%token TOK_STATIC
%token TOK_STRUCT
%token TOK_SWITCH
%token TOK_TYPEDEF
%token TOK_TYPENAME
%token TOK_UCHARN
%token TOK_UCHAR
%token TOK_ULONG
%token TOK_USHORT
%token TOK_UINT
%token TOK_UINT_LONG
%token TOK_UINT_LONG_LONG
%token TOK_UINTN
%token TOK_ULONGN
%token TOK_UINTPTR_T
%token TOK_UNION
%token TOK_UNSIGNED
%token TOK_USHORTN
%token TOK_VOID
%token TOK_VOLATILE
%token TOK_WHILE
%token TOK_WRITE_ONLY

%type<llvm_value_ref> primary
%type<llvm_value_ref> lvalue
%type<llvm_value_ref> maybe_expr
%type<llvm_value_ref> expr
%type<llvm_value_ref> unary_expr
%type<llvm_value_ref> init
%type<init_list> init_list
%type<llvm_type_ref> type_name
%type<llvm_type_ref> type_spec
%type<llvm_type_ref> declarator
%type<llvm_type_ref> declarator_list

%start program

%%
program
	: external_def {printf("start rule matched\n");}
	| program external_def {printf("start rule matched\n");}
	;

external_def
	: func_def
	| declaration
	;


func_def
	: declarator_list TOK_ID
	/*{
		current_id.id_name = yylval.identifier;
	}*/
	TOK_PAR_OPEN arg_list TOK_PAR_CLOSE
	{
		LLVMTypeRef func_args[0];
		
		cl2llvm_function = LLVMAddFunction(cl2llvm_module, "func_name",
			LLVMFunctionType(LLVMInt32Type(), func_args, 0, 0));
		LLVMSetFunctionCallConv(cl2llvm_function, LLVMCCallConv);
		cl2llvm_basic_block = LLVMAppendBasicBlock(cl2llvm_function, "bb_entry");
		LLVMPositionBuilderAtEnd(cl2llvm_builder, cl2llvm_basic_block);

	}
	TOK_CURLY_BRACE_OPEN stmt_list TOK_CURLY_BRACE_CLOSE
	{
		LLVMBuildRet(cl2llvm_builder, LLVMConstInt(LLVMInt32Type(), 1, 0));
	}
	; 



arg_list
	: /*empty*/
	| arg
	| arg TOK_COMMA arg_list
	;

arg
	: declarator_list TOK_ID
	| declarator_list
	;



declarator_list
	: declarator
	{
		$$ = $1;
	}
	| declarator_list declarator
	;

access_qual
	: TOK_GLOBAL
	| TOK_LOCAL
	| TOK_PRIVATE
	| TOK_CONSTANT
	;

declarator
	: type_spec 
	{
		$$ = $1;
	}
	| addr_qual
	{
		$$ = NULL;
	}
	| TOK_KERNEL
	{
		$$ = NULL;
	}
	| TOK_INLINE
	{
		$$ = NULL;
	}
	| sc_spec
	{
		$$ = NULL;
	}
	| access_qual
	{
		$$ = NULL;
	}
	| type_qual
	{
		$$ = NULL;
	}
	;

type_qual
	: TOK_CONST
	| TOK_VOLATILE
	;

addr_qual
	: TOK_READ_ONLY
	| TOK_WRITE_ONLY
	| TOK_READ_WRITE
	;


sc_spec
	: TOK_EXTERN
	| TOK_STATIC
	;

stmt_list
	: /*empty*/
	| stmt_or_stmt_list stmt_list
	;

lvalue
	: type_ptr_list TOK_ID array_deref_list
	{
		$$ = NULL;
	}

	| TOK_ID array_deref_list %prec TOK_MINUS
	{
		$$ = NULL;
	}

	| type_ptr_list
	{
		$$ = NULL;
	}

	| TOK_ID %prec TOK_MINUS
	{
		struct cl2llvm_symbol_t *symbol;

		symbol = hash_table_get(cl2llvm_symbol_table, $1);
		if (!symbol)
			yyerror("undefined identifier");

		struct cl2llvm_val_t *symbol_val_dup = cl2llvm_val_create_w_init(symbol->cl2llvm_val->val, symbol->cl2llvm_val->type->sign);

		symbol_val_dup->type->llvm_type = symbol->cl2llvm_val->type->llvm_type;

		$$ = symbol_val_dup;
	}
	| struct_deref_list
	{
		$$ = NULL;
	}
	;

struct_deref_list
	: TOK_ID TOK_STRUCT_REF TOK_ID
	| TOK_ID array_deref_list TOK_STRUCT_REF TOK_ID array_deref_list
	| TOK_ID array_deref_list TOK_STRUCT_REF TOK_ID
	| TOK_ID TOK_STRUCT_REF TOK_ID array_deref_list
	| struct_deref_list TOK_STRUCT_REF TOK_ID
	;


array_deref_list
	: TOK_BRACKET_OPEN expr TOK_BRACKET_CLOSE
	| array_deref_list TOK_BRACKET_OPEN expr TOK_BRACKET_CLOSE
	;

stmt
	: maybe_expr TOK_SEMICOLON
	{
		cl2llvm_val_free($1);
	}
	| declaration
	| func_def
	| for_loop
	| while_loop
	| do_while_loop
	| if_stmt
	| TOK_RETURN expr TOK_SEMICOLON
	| TOK_CONTINUE TOK_SEMICOLON
	| TOK_BREAK TOK_SEMICOLON
	| switch_stmt
	| label_stmt
	| goto_stmt
	;

func_call
	: TOK_ID TOK_PAR_OPEN param_list TOK_PAR_CLOSE
	| TOK_ID TOK_PAR_OPEN TOK_PAR_CLOSE
	;

param_list
	: expr
	| array_deref_list TOK_EQUAL expr
	| array_init
	| param_list TOK_COMMA expr
	| param_list TOK_COMMA array_init
	| param_list TOK_COMMA array_deref_list TOK_EQUAL expr
	;

array_init
	: TOK_CURLY_BRACE_OPEN param_list TOK_CURLY_BRACE_CLOSE
	;

init
	: /*empty*/
	{
		$$ = NULL;
	}
	| TOK_PAR_OPEN arg_list TOK_PAR_CLOSE
	{
		$$ = NULL;
	}
	| TOK_EQUAL expr
	{
		$$ = $2;
	}
	| TOK_EQUAL array_init
	{
		$$ = NULL;
	}
	;

init_list
	: TOK_ID init %prec TOK_MULT
	{
		struct list_t *init_list = list_create();
		struct cl2llvm_symbol_t *symbol = cl2llvm_symbol_create($1);

		cl2llvm_val_free(symbol->cl2llvm_val);
		symbol->cl2llvm_val = $2;

		list_add(init_list, symbol);
		$$ = init_list;
	}
	| init_list TOK_COMMA TOK_ID init %prec TOK_MULT
	{
		struct cl2llvm_symbol_t *symbol = cl2llvm_symbol_create($3);

		cl2llvm_val_free(symbol->cl2llvm_val);
		symbol->cl2llvm_val = $4;

		list_add($1, symbol);
		$$ = $1;
	}
	| TOK_ID array_deref_list init  %prec TOK_MULT
	{
		$$ = NULL;
	}
	| init_list TOK_COMMA TOK_ID array_deref_list init %prec TOK_MULT
	;


declaration
	: declarator_list init_list TOK_SEMICOLON
	{
		struct cl2llvm_symbol_t *symbol;
		struct cl2llvm_val_t *cast_to_val;
		int init_count = list_count($2);
		int i;
		for(i = 0; i < init_count; i++)
		{	
			int err;			
			struct cl2llvm_symbol_t *current_list_elem = list_get($2, i);
			symbol = cl2llvm_symbol_create_w_init( LLVMBuildAlloca( 
				cl2llvm_builder, $1->llvm_type, 
				current_list_elem->name), $1->sign, 
				current_list_elem->name);
			symbol->cl2llvm_val->type->llvm_type = $1->llvm_type;
			err = hash_table_insert(cl2llvm_symbol_table, 
				current_list_elem->name, symbol);
			if (!err)
				printf("duplicated symbol");
			if (LLVMTypeOf(current_list_elem->cl2llvm_val->val) == $1->llvm_type 
				&& current_list_elem->cl2llvm_val->type->sign == $1->sign)
			{
				LLVMBuildStore(cl2llvm_builder,
					current_list_elem->cl2llvm_val->val, symbol->cl2llvm_val->val);
			}
			else
			{
				cast_to_val = llvm_type_cast( current_list_elem->cl2llvm_val, $1);
				LLVMBuildStore(cl2llvm_builder,
					cast_to_val->val,
					symbol->cl2llvm_val->val);
				cl2llvm_val_free(cast_to_val);
			}
		}
		cl2llvm_type_free($1);
		LIST_FOR_EACH($2, i)
		{
			cl2llvm_symbol_free(list_get($2, i));
		}
		list_free($2);
	}
	;

stmt_or_stmt_list
	: stmt
	| TOK_CURLY_BRACE_OPEN stmt_list TOK_CURLY_BRACE_CLOSE
	;

label_stmt
	: TOK_ID TOK_COLON
	;

goto_stmt
	: TOK_GOTO TOK_ID TOK_SEMICOLON
	;

switch_stmt
	: TOK_SWITCH expr TOK_CURLY_BRACE_OPEN switch_body TOK_CURLY_BRACE_CLOSE
	;

switch_body
	: default_clause
	| case_clause
	| switch_body default_clause
	| switch_body case_clause
	;

default_clause
	: TOK_DEFAULT TOK_COLON stmt_list
	;

case_clause
	: TOK_CASE expr TOK_COLON stmt_list
	;

if_stmt
	: TOK_IF TOK_PAR_OPEN expr TOK_PAR_CLOSE
	{
		struct cl2llvm_type_t *i1 = cl2llvm_type_create_w_init(LLVMInt1Type(), 1);

		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);

		LLVMBasicBlockRef if_false = LLVMAppendBasicBlock(cl2llvm_function, block_name);
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef if_true = LLVMAppendBasicBlock(cl2llvm_function, block_name);

		LLVMBuildCondBr(cl2llvm_builder, llvm_type_cast($3, i1)->val, if_true, if_false);
		$<basic_block_ref>$ = if_false;
		LLVMPositionBuilderAtEnd(cl2llvm_builder, if_true);
	}
	stmt_or_stmt_list
	{
		LLVMBuildBr(cl2llvm_builder, $<basic_block_ref>5);
		LLVMPositionBuilderAtEnd(cl2llvm_builder, $<basic_block_ref>5);
	}
/*	| TOK_IF TOK_PAR_OPEN expr TOK_PAR_CLOSE stmt_or_stmt_list TOK_ELSE stmt_or_stmt_list*/
	;

for_loop
	: for_loop_header stmt_or_stmt_list
	;

for_loop_header
	: TOK_FOR TOK_PAR_OPEN maybe_expr TOK_SEMICOLON maybe_expr TOK_SEMICOLON maybe_expr TOK_PAR_CLOSE
	| TOK_FOR TOK_PAR_OPEN declaration maybe_expr TOK_SEMICOLON maybe_expr TOK_PAR_CLOSE
	;

do_while_loop
	: TOK_DO stmt_or_stmt_list TOK_WHILE TOK_PAR_OPEN expr TOK_PAR_CLOSE TOK_SEMICOLON 
	;

while_loop
	: TOK_WHILE TOK_PAR_OPEN expr TOK_PAR_CLOSE stmt_or_stmt_list
	;

maybe_expr
	: /*empty*/
	{
		/*create object so that maybe_expr always points to a memory 
		  location regardless of its contents*/
		struct cl2llvm_val_t *value = cl2llvm_val_create();
		$$ = value;
	}
	| expr
	;
expr
	: primary

	| TOK_PAR_OPEN expr TOK_PAR_CLOSE
	{
		$$ = $2;
	}

	| expr TOK_PLUS expr
	{
		printf("addition rule\n");
		struct cl2llvm_type_t *type = cl2llvm_type_create();
		struct cl2llvm_val_t *op1, *op2;
		
		type_unify($1, $3, &op1, &op2);
		if(op1 == $1)
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}
		else
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}
		snprintf(temp_var_name, sizeof temp_var_name,
				"tmp%d", temp_var_count++);
		struct cl2llvm_val_t *value = cl2llvm_val_create();
		switch (LLVMGetTypeKind(type->llvm_type))
		{
		case LLVMIntegerTypeKind:

			value->val = LLVMBuildAdd(cl2llvm_builder, op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		case LLVMHalfTypeKind:
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:

			value->val = LLVMBuildFAdd(cl2llvm_builder, op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		default:

			yyerror("invalid type of operands for addition");
		}
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free( op2);
		cl2llvm_val_free($3);
		cl2llvm_val_free($1);
		cl2llvm_type_free(type);
		$$ = value;
	}
	| expr TOK_MINUS expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create();
		struct cl2llvm_val_t *op1, *op2;
		
		type_unify($1, $3, &op1, & op2);
		if(op1 == $1)
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}
		else
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}
	
		snprintf(temp_var_name, sizeof temp_var_name,
				"tmp%d", temp_var_count++);

		struct cl2llvm_val_t *value = cl2llvm_val_create();
		switch (LLVMGetTypeKind(type->llvm_type))
		{
		case LLVMIntegerTypeKind:

			value->val = LLVMBuildSub(cl2llvm_builder, op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		case LLVMHalfTypeKind:
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:

			value->val = LLVMBuildFSub(cl2llvm_builder, op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		default:

			yyerror("invalid type of operands for addition");
		}
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);

		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		$$ = value;
	}
	| expr TOK_MULT expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create();
		struct cl2llvm_val_t *value = cl2llvm_val_create();
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);

		struct cl2llvm_val_t *op1, *op2;
		
		type_unify($1, $3, &op1, &op2);
		if(op1 == $1)
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}
		else
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}
		
		switch (LLVMGetTypeKind(type->llvm_type))
		{
		case LLVMIntegerTypeKind:
			value->val = LLVMBuildMul(cl2llvm_builder, op1->val,
				op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		case LLVMHalfTypeKind:
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:
			value->val = LLVMBuildFMul(cl2llvm_builder, op1->val,
				op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;
		default:

			yyerror("invalid type of operands for addition");
		}
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);

		$$ = value;

	}
	| expr TOK_DIV expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create();
		struct cl2llvm_val_t *value = cl2llvm_val_create();
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);

		struct cl2llvm_val_t *op1, *op2;
		
		type_unify($1, $3, &op1, & op2);
		if(op1 == $1)
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}
		else
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}


		switch (LLVMGetTypeKind(type->llvm_type))
		{
		case LLVMIntegerTypeKind:
			if (type->sign)
			{
				value->val = LLVMBuildSDiv(cl2llvm_builder, 
					op1->val, op2->val, temp_var_name);
			}
			else
			{
				value->val = LLVMBuildUDiv(cl2llvm_builder, 
					op1->val, op2->val, temp_var_name);
			}
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		case LLVMHalfTypeKind:
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:
			value->val = LLVMBuildFDiv(cl2llvm_builder, 
					op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		default:
			
			yyerror("invalid type of operands for addition");
		}
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		$$ = value;
	}
	| expr TOK_MOD expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create();
		struct cl2llvm_val_t *value = cl2llvm_val_create();
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);

		struct cl2llvm_val_t *op1, *op2;
		
		type_unify($1, $3, &op1, & op2);
		if(op1 == $1)
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}
		else
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}

		switch (type->sign)
		{
		case 1:
			value->val = LLVMBuildSRem(cl2llvm_builder, 
				op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		case 0:
			value->val = LLVMBuildURem(cl2llvm_builder, 
				op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		default:
			
			yyerror("invalid type of operands for addition");
		}
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		$$ = value;

	}
	| expr TOK_SHIFT_LEFT expr
	| expr TOK_SHIFT_RIGHT expr
	| expr TOK_EQUALITY expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create();
		struct cl2llvm_val_t *value = cl2llvm_val_create();
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);

		struct cl2llvm_val_t *op1, *op2;
		
		type_unify($1, $3, &op1, & op2);
		if(op1 == $1)
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}
		else
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}

		switch (LLVMGetTypeKind(type->llvm_type))
		{
		case LLVMIntegerTypeKind:

			value->val = LLVMBuildICmp(cl2llvm_builder, LLVMIntEQ,
				op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		case LLVMHalfTypeKind:
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:

			value->val = LLVMBuildFCmp(cl2llvm_builder,
				LLVMRealOEQ, op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		default:

			yyerror("invalid type of operands for equality");
		}
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		$$ = value;
	}
	| expr TOK_INEQUALITY expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create();
		struct cl2llvm_val_t *value = cl2llvm_val_create();
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);

		struct cl2llvm_val_t *op1, *op2;
		
		type_unify($1, $3, &op1, & op2);
		if(op1 == $1)
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}
		else
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}

		switch (LLVMGetTypeKind(type->llvm_type))
		{
		case LLVMIntegerTypeKind:

			value->val = LLVMBuildICmp(cl2llvm_builder, LLVMIntNE,
				op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		case LLVMHalfTypeKind:
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:

			value->val = LLVMBuildFCmp(cl2llvm_builder, 
				LLVMRealONE, op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		default:

			yyerror("invalid type of operands for addition");
		}
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		$$ = value;

	}
	| expr TOK_LESS expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create();
		struct cl2llvm_val_t *value = cl2llvm_val_create();
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);

		struct cl2llvm_val_t *op1, *op2;
		
		type_unify($1, $3, &op1, & op2);
		if(op1 == $1)
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}
		else
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}

		switch (LLVMGetTypeKind(type->llvm_type))
		{
		case LLVMIntegerTypeKind:

			if (type->sign)
			{
				value->val = LLVMBuildICmp(cl2llvm_builder, 
					LLVMIntSLT, op1->val, op2->val, 
					temp_var_name);
			}
			else
			{
				value->val = LLVMBuildICmp(cl2llvm_builder, 
					LLVMIntULT, op1->val, op2->val, 
					temp_var_name);
			}
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		case LLVMHalfTypeKind:
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:

			value->val = LLVMBuildFCmp(cl2llvm_builder, 
				LLVMRealOLT, op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		default:

			yyerror("invalid type of operands for addition");
		}
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		$$ = value;

	}
	| expr TOK_GREATER expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create();
		struct cl2llvm_val_t *value = cl2llvm_val_create();
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);

		struct cl2llvm_val_t *op1, *op2;
		
		type_unify($1, $3, &op1, & op2);
		if(op1 == $1)
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}
		else
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}

		switch (LLVMGetTypeKind(type->llvm_type))
		{
		case LLVMIntegerTypeKind:

			if (type->sign)
			{
				value->val = LLVMBuildICmp(cl2llvm_builder, 
					LLVMIntSGT, op1->val, op2->val, 
					temp_var_name);
			}
			else
			{
				value->val = LLVMBuildICmp(cl2llvm_builder, 
					LLVMIntUGT, op1->val, op2->val, 
					temp_var_name);
			}
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		case LLVMHalfTypeKind:
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:

			value->val = LLVMBuildFCmp(cl2llvm_builder, 
				LLVMRealOGT, op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		default:

			yyerror("invalid type of operands for addition");
		}
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		$$ = value;
	}
	| expr TOK_LESS_EQUAL expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create();
		struct cl2llvm_val_t *value = cl2llvm_val_create();
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);

		struct cl2llvm_val_t *op1, *op2;
		
		type_unify($1, $3, &op1, & op2);
		if(op1 == $1)
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}
		else
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}

		switch (LLVMGetTypeKind(type->llvm_type))
		{
		case LLVMIntegerTypeKind:

			if (type->sign)
			{
				value->val = LLVMBuildICmp(cl2llvm_builder, 
					LLVMIntSLE, op1->val, op2->val, 
					temp_var_name);
			}
			else
			{
				value->val = LLVMBuildICmp(cl2llvm_builder, 
					LLVMIntULE, op1->val, op2->val, 
					temp_var_name);
			}
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		case LLVMHalfTypeKind:
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:

			value->val = LLVMBuildFCmp(cl2llvm_builder, 
				LLVMRealOLE, op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		default:

			yyerror("invalid type of operands for addition");
		}
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		$$ = value;
	}
	| expr TOK_GREATER_EQUAL expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create();
		struct cl2llvm_val_t *value = cl2llvm_val_create();
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);

		struct cl2llvm_val_t *op1, *op2;
		
		type_unify($1, $3, &op1, & op2);
		if(op1 == $1)
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}
		else
		{
			type->llvm_type = LLVMTypeOf(op1->val);
			type->sign = op1->type->sign;
		}

		switch (LLVMGetTypeKind(type->llvm_type))
		{
		case LLVMIntegerTypeKind:

			if (type->sign)
			{
				value->val = LLVMBuildICmp(cl2llvm_builder, 
					LLVMIntSGE, op1->val, op2->val, 
					temp_var_name);
			}
			else
			{
				value->val = LLVMBuildICmp(cl2llvm_builder, 
					LLVMIntUGE, op1->val, op2->val, 
					temp_var_name);
			}
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		case LLVMHalfTypeKind:
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:

			value->val = LLVMBuildFCmp(cl2llvm_builder, 
				LLVMRealOGE, op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		default:

			yyerror("invalid type of operands for addition");
		}
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		$$ = value;
	}
	| expr TOK_LOGICAL_AND expr
	| expr TOK_LOGICAL_OR expr
	| lvalue TOK_EQUAL expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init( 
			$1->type->llvm_type , $1->type->sign);

		struct cl2llvm_val_t *value = llvm_type_cast($3, type);
		LLVMBuildStore(cl2llvm_builder, $3->val, $1->val);
		cl2llvm_type_free(type);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		$$ = value;
	}
	| lvalue TOK_ADD_EQUAL expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init( 
			$1->type->llvm_type , $1->type->sign);
		struct cl2llvm_val_t *value;
		
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);
		struct cl2llvm_val_t *lval = cl2llvm_val_create_w_init(
			LLVMBuildLoad(cl2llvm_builder, $1->val, temp_var_name),
			$1->type->sign);

		struct cl2llvm_val_t *rval = llvm_type_cast($3, type);
	
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);
		
		switch (LLVMGetTypeKind(type->llvm_type))
		{
		case LLVMIntegerTypeKind:

			value = cl2llvm_val_create_w_init(
				LLVMBuildAdd(cl2llvm_builder, lval->val, 
				rval->val, temp_var_name), type->sign);
			break;

		case LLVMHalfTypeKind:
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:

			value = cl2llvm_val_create_w_init(
				LLVMBuildFAdd(cl2llvm_builder, lval->val, 
				rval->val, temp_var_name), type->sign);
			break;

		default:

			yyerror("invalid type of operands for addition");
			value = cl2llvm_val_create();
		}

		LLVMBuildStore(cl2llvm_builder, value->val, $1->val);
		
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_val_free(rval);
		cl2llvm_val_free(lval);
		cl2llvm_type_free(type);
		$$ = value;
	}
	| lvalue TOK_MINUS_EQUAL expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init( 
			$1->type->llvm_type , $1->type->sign);
		struct cl2llvm_val_t *value;
		
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);
		struct cl2llvm_val_t *lval = cl2llvm_val_create_w_init(
			LLVMBuildLoad(cl2llvm_builder, $1->val, temp_var_name),
			$1->type->sign);

		struct cl2llvm_val_t *rval = llvm_type_cast($3, type);

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);
		
		switch (LLVMGetTypeKind(type->llvm_type))
		{
		case LLVMIntegerTypeKind:

			value = cl2llvm_val_create_w_init(
				LLVMBuildSub(cl2llvm_builder, lval->val, 
				rval->val, temp_var_name), type->sign);
			break;

		case LLVMHalfTypeKind:
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:

			value = cl2llvm_val_create_w_init(
				LLVMBuildFSub(cl2llvm_builder, lval->val, 
				rval->val, temp_var_name), type->sign);
			break;

		default:
			
			yyerror("invalid type of operands for addition");
			value = cl2llvm_val_create();
		}

		LLVMBuildStore(cl2llvm_builder, value->val, $1->val);

		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_val_free(rval);
		cl2llvm_val_free(lval);
		cl2llvm_type_free(type);
		$$ = value;
	}
	| lvalue TOK_DIV_EQUAL expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init( 
			$1->type->llvm_type , $1->type->sign);
		struct cl2llvm_val_t *value = cl2llvm_val_create();
		
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);
		struct cl2llvm_val_t *lval = cl2llvm_val_create_w_init(
			LLVMBuildLoad(cl2llvm_builder, $1->val, temp_var_name),
			$1->type->sign);
	
		struct cl2llvm_val_t *rval = llvm_type_cast($3, type);

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);
		
		switch (LLVMGetTypeKind(type->llvm_type))
		{
		case LLVMIntegerTypeKind:
			if (type->sign)
			{
				value->val = LLVMBuildSDiv(cl2llvm_builder, 
					lval->val, rval->val, temp_var_name);
			}
			else
			{
				value->val = LLVMBuildUDiv(cl2llvm_builder, 
					lval->val, rval->val, temp_var_name);
			}
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		case LLVMHalfTypeKind:
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:
			value->val = LLVMBuildFDiv(cl2llvm_builder, 
					lval->val, rval->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		default:
			
			yyerror("invalid type of operands for addition");
		}
	
		LLVMBuildStore(cl2llvm_builder, value->val, $1->val);
		
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_val_free(rval);
		cl2llvm_val_free(lval);
		cl2llvm_type_free(type);
		$$ = value;
	}
	| lvalue TOK_MULT_EQUAL expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init( 
			$1->type->llvm_type , $1->type->sign);
		struct cl2llvm_val_t *value = cl2llvm_val_create();
		
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);
		struct cl2llvm_val_t *lval = cl2llvm_val_create_w_init(
			LLVMBuildLoad(cl2llvm_builder, $1->val, temp_var_name),
			$1->type->sign);
	
		struct cl2llvm_val_t *rval = llvm_type_cast($3, type);

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);
		
		switch (LLVMGetTypeKind(type->llvm_type))
		{
		case LLVMIntegerTypeKind:

			value->val = LLVMBuildMul(cl2llvm_builder, 
				lval->val, rval->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		case LLVMHalfTypeKind:
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:
			value->val = LLVMBuildFMul(cl2llvm_builder, 
					lval->val, rval->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		default:
			
			yyerror("invalid type of operands for addition");
		}
	
		LLVMBuildStore(cl2llvm_builder, value->val, $1->val);
	
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_val_free(lval);
		cl2llvm_val_free(rval);
		cl2llvm_type_free(type);
		$$ = value;
	}
	| lvalue TOK_MOD_EQUAL expr
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init( 
			$1->type->llvm_type , $1->type->sign);
		struct cl2llvm_val_t *value = cl2llvm_val_create();
		
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);
		struct cl2llvm_val_t *lval = cl2llvm_val_create_w_init(
			LLVMBuildLoad(cl2llvm_builder, $1->val, temp_var_name),
			$1->type->sign);
	
		struct cl2llvm_val_t *rval = llvm_type_cast($3, type);

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp%d", temp_var_count++);
		
		switch (LLVMGetTypeKind(type->llvm_type))
		{
		case LLVMIntegerTypeKind:
			if (type->sign)
			{
				value->val = LLVMBuildSRem(cl2llvm_builder, 
					lval->val, rval->val, temp_var_name);
			}
			else
			{
				value->val = LLVMBuildURem(cl2llvm_builder, 
					lval->val, rval->val, temp_var_name);
			}
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;

		default:
			
			yyerror("invalid type of operands for addition");
		}
	
		LLVMBuildStore(cl2llvm_builder, value->val, $1->val);
		
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_val_free(rval);
		cl2llvm_val_free(lval);
		cl2llvm_type_free(type);
		$$ = value;
	}
	| lvalue TOK_AND_EQUAL expr
	| lvalue TOK_OR_EQUAL expr
	| lvalue TOK_EXCLUSIVE_EQUAL expr
	| lvalue TOK_SHIFT_RIGHT_EQUAL expr
	| lvalue TOK_SHIFT_LEFT_EQUAL expr
	| expr TOK_CONDITIONAL expr TOK_COLON expr

	| unary_expr
	{
		$$ = NULL;
	}

	| func_call
	{
		$$ = NULL;
	}

	| TOK_LOGICAL_NEGATE expr
	{
		$$ = NULL;
	}

	| expr TOK_BITWISE_AND expr
	| expr TOK_BITWISE_OR expr
	| expr TOK_BITWISE_EXCLUSIVE expr
	;


unary_expr
	: lvalue TOK_INCREMENT %prec TOK_POSTFIX
	{
		$$ = NULL;
	}
	| TOK_INCREMENT lvalue %prec TOK_PREFIX
	{
		$$ = NULL;
	}
	| TOK_DECREMENT lvalue %prec TOK_PREFIX
	{
		$$ = NULL;
	}
	| lvalue TOK_DECREMENT %prec TOK_POSTFIX
	{
		$$ = NULL;
	}
	| TOK_MINUS primary %prec TOK_PREFIX
	{
		$$ = NULL;
	}
	| TOK_PLUS primary %prec TOK_PREFIX
	{
		$$ = NULL;
	}
	| TOK_PAR_OPEN type_spec TOK_PAR_CLOSE expr %prec TOK_PREFIX
	{
		printf("cast rule\n");
		llvm_type_cast($4, $2);
		$$ = $4;
	}
	| TOK_SIZEOF TOK_PAR_OPEN type_spec TOK_PAR_CLOSE %prec TOK_PREFIX
	{
		$$ = NULL;
	}
	| TOK_BITWISE_NOT expr
	{
		$$ = NULL;
	}
	| TOK_BITWISE_AND lvalue %prec TOK_PREFIX
	{
		$$ = NULL;
	}
	;


/* The vector_literal_param_list is technically the same as the param_list, with
 * the difference that here we need at least two elements in the list. Different
 * syntax rules are created to avoid shift/reduce conflicts. */
vec_literal
	: TOK_PAR_OPEN type_spec TOK_PAR_CLOSE TOK_PAR_OPEN vec_literal_param_list TOK_PAR_CLOSE
	;

vec_literal_param_elem
	: expr
	| array_deref_list
	| array_init
	;

vec_literal_param_two_elem
	: vec_literal_param_elem TOK_COMMA vec_literal_param_elem
	;

vec_literal_param_list
	: vec_literal_param_two_elem
	| vec_literal_param_elem TOK_COMMA vec_literal_param_list
	;


primary
	: TOK_CONST_INT
	{
		struct cl2llvm_val_t *value = cl2llvm_val_create_w_init(
			LLVMConstInt(LLVMInt32Type(), $1, 0), 1);
		$$ = value;
	}
	| TOK_CONST_INT_L
	{
		struct cl2llvm_val_t *value = cl2llvm_val_create_w_init(
			LLVMConstInt(LLVMInt32Type(), $1, 0), 1);
		$$ = value;
	}
	| TOK_CONST_INT_U
	{
		struct cl2llvm_val_t *value = cl2llvm_val_create_w_init(
			LLVMConstInt(LLVMInt32Type(), $1, 0), 0);
		$$ = value;
	}
	| TOK_CONST_INT_UL
	{
		struct cl2llvm_val_t *value =  cl2llvm_val_create_w_init(
			LLVMConstInt(LLVMInt32Type(), $1, 0), 0);
		$$ = value;
	}
	| TOK_CONST_INT_LL
	{
		struct cl2llvm_val_t *value = cl2llvm_val_create_w_init(
			LLVMConstInt(LLVMInt64Type(), $1, 0), 1);
		$$ = value;
	}
	| TOK_CONST_INT_ULL
	{
		struct cl2llvm_val_t *value = cl2llvm_val_create_w_init(
			LLVMConstInt(LLVMInt64Type(), $1, 0), 0);
		$$ = value;
	}
	| TOK_CONST_DEC
	{
		struct cl2llvm_val_t *value = cl2llvm_val_create_w_init(
			LLVMConstReal(LLVMFloatType(), $1), 1);
		$$ = value;
	}
	| TOK_CONST_DEC_H
	{
		struct cl2llvm_val_t *value = cl2llvm_val_create_w_init(
			LLVMConstReal(LLVMHalfType(), $1), 1);
		$$ = value;
	}
	| TOK_CONST_DEC_F
	{
		struct cl2llvm_val_t *value = cl2llvm_val_create_w_init(
			LLVMConstReal(LLVMFloatType(), $1), 1);
		$$ = value;
	}
	| TOK_CONST_DEC_L
	{
		struct cl2llvm_val_t *value = cl2llvm_val_create_w_init(
			LLVMConstReal(LLVMDoubleType(), $1), 1);
		$$ = value;
	}
	| lvalue
	{
		snprintf(temp_var_name, sizeof(temp_var_name),
				"tmp%d", temp_var_count++);
		struct cl2llvm_val_t *value = cl2llvm_val_create_w_init(
			LLVMBuildLoad(cl2llvm_builder, $1->val, temp_var_name),
			$1->type->sign);
		cl2llvm_val_free($1);

		$$ = value;
	}

	| vec_literal
	{
		$$ = NULL;
	}
	;


type_spec
	: type_name
	{
		$$ = $1;
	}
	| type_name type_ptr_list 
	;

type_ptr_list
	: TOK_MULT 
	| TOK_MULT type_ptr_list 
	;

type_name
	: TOK_INTPTR_T
	{
		$$ = NULL;
	}
	| TOK_PTRDIFF_T
	{
		$$ = NULL;
	}
	| TOK_UINTPTR_T
	{
		$$ = NULL;
	}
	| TOK_SAMPLER_T
	{
		$$ = NULL;
	}
	| TOK_EVENT_T
	{
		$$ = NULL;
	}
	| TOK_IMAGE2D_T
	{
		$$ = NULL;
	}
	| TOK_IMAGE3D_T
	{
		$$ = NULL;
	}
	| TOK_IMAGE2D_ARRAY_T
	{
		$$ = NULL;
	}
	| TOK_IMAGE1D_T
	{
		$$ = NULL;
	}
	| TOK_IMAGE1D_BUFFER_T
	{
		$$ = NULL;
	}
	| TOK_IMAGE1D_ARRAY_T
	{
		$$ = NULL;
	}
	| TOK_UINT
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMInt32Type(), 0);
		$$ = type;
	}
	| TOK_UINT_LONG
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMInt32Type(), 0);
		$$ = type;
	}
	| TOK_UINT_LONG_LONG
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMInt64Type(), 0);
		$$ = type;
	}
	| TOK_UCHAR
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMInt8Type(), 0);
		$$ = type;
	}
	| TOK_SHORT
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMInt16Type(), 1);
		$$ = type;
	}
	| TOK_USHORT
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMInt16Type(), 0);
		$$ = type;
	}
	| TOK_UINTN
	{
		$$ = NULL;
	}
	| TOK_UCHARN
	{
		$$ = NULL;
	}
	| TOK_ULONGN
	{
		$$ = NULL;
	}
	| TOK_USHORTN
	{
		$$ = NULL;
	}
	| TOK_SHORTN
	{
		$$ = NULL;
	}
	| TOK_INTN
	{
		$$ = NULL;
	}
	| TOK_LONGN
	{
		$$ = NULL;
	}
	| TOK_CHARN
	{
		$$ = NULL;
	}
	| TOK_FLOATN
	{
		$$ = NULL;
	}
	| TOK_DOUBLEN
	{
		$$ = NULL;
	}
	| TOK_INT 
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMInt32Type(), 1);
		$$ = type;
	}
	| TOK_INT_LONG
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMInt32Type(), 1);
		$$ = type;
	}
	| TOK_LONG_LONG
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMInt64Type(), 1);
		$$ = type;
	}
	| TOK_CHAR
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMInt8Type(), 1);
		$$ = type;
	}
	| TOK_FLOAT
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMFloatType(), 1);
		$$ = type;
	}
	| TOK_BOOL
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMInt1Type(), 1);
		$$ = type;
	}
	| TOK_DOUBLE
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMDoubleType(), 1);
		$$ = type;
	}
	|TOK_DOUBLE_LONG
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMInt64Type(), 1);
		$$ = type;
	}
	| TOK_VOID
	{
		$$ = NULL;
	}
	| TOK_HALF
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMHalfType(), 1);
		$$ = type;
	}
	;




%%

