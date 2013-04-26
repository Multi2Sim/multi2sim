%{

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include "cl2llvm.h"

extern LLVMBuilderRef cl2llvm_builder;
extern LLVMModuleRef cl2llvm_module;
extern LLVMValueRef cl2llvm_function;
extern LLVMBasicBlockRef cl2llvm_basic_block;


struct symbol_t
{
	char *name;
	LLVMTypeRef type;
	LLVMValueRef value;
};

struct symbol_t *symbol_create(char *name)
{
	struct symbol_t *symbol;

	/* Initialize */
	symbol = calloc(1, sizeof(struct symbol_t));
	symbol->name = strdup(name);

	/* Return */
	return symbol;
}

void symbol_free(struct symbol_t *symbol)
{
	free(symbol->name);
	free(symbol);
}



/*
 * Initializer list elements
 */

struct init_list_elem_t
{
};

/* TODO - write constructor */

/* TODO - write destructor */




int temp_var_count;
char temp_var_name[50];

struct hash_table_t *cl2llvm_symbol_table;

#define type_cmp_num_types  17
LLVMTypeRef type_cmp(LLVMTypeRef type1, LLVMTypeRef type2)
{
	struct llvm_type_table 
	{
		LLVMTypeRef type1;
		LLVMTypeRef type2;
	};
	struct llvm_type_table table[type_cmp_num_types] = 
	{
		{ LLVMInt64Type(), LLVMInt32Type() },
		{ LLVMInt64Type(), LLVMInt16Type() },
		{ LLVMInt64Type(), LLVMInt8Type() },
		{ LLVMInt64Type(), LLVMInt1Type() },
		{ LLVMInt32Type(), LLVMInt8Type() },
		{ LLVMInt32Type(), LLVMInt16Type() },
		{ LLVMInt32Type(), LLVMInt1Type() },
		{ LLVMDoubleType(), LLVMInt64Type() },
		{ LLVMDoubleType(), LLVMInt32Type() },
		{ LLVMDoubleType(), LLVMInt16Type() },
		{ LLVMDoubleType(), LLVMInt8Type() },
		{ LLVMDoubleType(), LLVMInt1Type() },
		{ LLVMFloatType(), LLVMInt64Type() },
		{ LLVMFloatType(), LLVMInt32Type() },
		{ LLVMFloatType(), LLVMInt16Type() },
		{ LLVMFloatType(), LLVMInt8Type() },
		{ LLVMFloatType(), LLVMInt1Type() }
	};
	int i;

	printf("Comparing type %d and %d\n",
		LLVMGetTypeKind(type1),
		LLVMGetTypeKind(type2));

	for (i = 0; i < type_cmp_num_types; i++)
	{
		if ((type1 == table[i].type1 && type2 == table[i].type2)
			|| (type2 == table[i].type1 && type1 == table[i].type2))
		{
			printf("dominant type identified: %d\n", i);
			return table[i].type1;
		}
	}
	/* type must be smaller than 4 bytes promote to int*/
	return LLVMInt32Type();

	/* Code should be unreachable */
	panic("%s: unreachable code reached", __FUNCTION__);
	return NULL;
}

LLVMValueRef llvm_type_cast(LLVMValueRef val, LLVMTypeRef totype)
{
	LLVMTypeRef fromtype = LLVMTypeOf(val);
	if (fromtype == LLVMInt32Type())
	{
		if (totype == LLVMFloatType())
		{
			snprintf(temp_var_name, sizeof temp_var_name,
				"tmp%d", temp_var_count++);
			return LLVMBuildSIToFP(cl2llvm_builder, val,
					LLVMFloatType(), temp_var_name);
		}
	}
	if (fromtype == LLVMInt1Type())
	{
		if (totype == LLVMFloatType())
		{
			snprintf(temp_var_name, sizeof temp_var_name,
				"tmp%d", temp_var_count++);
			return LLVMBuildSIToFP(cl2llvm_builder, val,
					LLVMFloatType(), temp_var_name);
		}
		/*if (totype == LLVMInt32Type())
		{
			snprintf(temp_var_name, sizeof temp_var_name,
				"tmp%d", temp_var_count++);
			return LLVMBuildSIToFP(cl2llvm_builder, val,
					LLVMFloatType(), temp_var_name);
		}*/
	}
	return val;
}


LLVMTypeRef type_unify(LLVMValueRef val1, LLVMValueRef val2,
		LLVMValueRef *new_val1, LLVMValueRef *new_val2)
{
	LLVMTypeRef type1 = LLVMTypeOf(val1);
	LLVMTypeRef type2 = LLVMTypeOf(val2);
	LLVMTypeRef type;

	/* By default, new values returned are the same as the original
	 * values. */
	*new_val1 = val1;
	*new_val2 = val2;

	/* If types match, no type cast needed */
	if (type1 == type2)
		return type1;

	/* Obtain dominant type */
	type = type_cmp(type1, type2);
	assert(type != type1 || type != type2);

	/* Whatever operand differs from the dominant type will be typecast
	 * to it. */
	if (type != type1)
		*new_val1 = llvm_type_cast(val1, type);
	else
		*new_val2 = llvm_type_cast(val2, type);
	
	/* Return dominant type */
	return type;
}




%}

%union {
	int const_int_val;
	float const_float_val;
	char * identifier;
	LLVMValueRef llvm_value_ref;
	LLVMTypeRef llvm_type_ref;
}

%token<identifier>  TOK_ID
%token<const_int_val> TOK_CONST_INT
%token<const_float_val> TOK_CONST_DEC
%token TOK_CONST_VAL
%token TOK_STRING
%token TOK_COMMA
%token TOK_SEMICOLON
%token TOK_ELLIPSIS
%left 	TOK_PAR_OPEN TOK_PAR_CLOSE TOK_BRACKET_OPEN
	TOK_BRACKET_CLOSE TOK_POSTFIX TOK_STRUCT_REF TOK_STRUCT_DEREF
%token 	TOK_CURLY_BRACE_OPEN
%token 	TOK_CURLY_BRACE_CLOSE
%right	TOK_PREFIX TOK_LOGICAL_NEGATE TOK_BITWISE_NOT
%left 	TOK_PLUS TOK_MINUS
%left 	TOK_MULT TOK_DIV TOK_MOD
%left	TOK_SHIFT_RIGHT TOK_SHIFT_LEFT
%left	TOK_GREATER TOK_LESS TOK_GREATER_EQUAL TOK_LESS_EQUAL
%left	TOK_EQUALITY TOK_INEQUALITY
%left	TOK_BITWISE_AND
%left	TOK_BITWISE_EXCLUSIVE
%left	TOK_BITWISE_OR
%left	TOK_LOGICAL_AND
%left	TOK_LOGICAL_OR
%right	TOK_CONDITIONAL TOK_COLON
%right 	TOK_EQUAL TOK_ADD_EQUAL TOK_MINUS_EQUAL
	TOK_MULT_EQUAL TOK_DIV_EQUAL TOK_MOD_EQUAL TOK_AND_EQUAL
	TOK_OR_EQUAL TOK_EXCLUSIVE_EQUAL TOK_SHIFT_RIGHT_EQUAL 
	TOK_SHIFT_LEFT_EQUAL
%token TOK_INCREMENT
%token TOK_DECREMENT
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
%type<llvm_value_ref> expr
%type<llvm_value_ref> init
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
		LLVMValueRef x = LLVMBuildAdd(cl2llvm_builder, LLVMConstInt(LLVMInt32Type(), 1, 0), LLVMConstInt(LLVMInt32Type(), 2, 0), "temp");
		LLVMBuildRet(cl2llvm_builder, x);
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
		printf("lvalue\n");
		$$ = NULL;
	}

	| type_ptr_list
	{
		$$ = NULL;
	}

	| TOK_ID %prec TOK_MINUS
	{
		struct symbol_t *symbol;

		symbol = hash_table_get(cl2llvm_symbol_table, $1);
		if (!symbol)
			yyerror("undefined identifier");

		$$ = symbol->value;
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
		struct symbol_t *symbol;
		int err;

		symbol = symbol_create($1);
		symbol->type = LLVMTypeOf($2);
		symbol->value = LLVMBuildAlloca(cl2llvm_builder, symbol->type, $1);
		err = hash_table_insert(cl2llvm_symbol_table, $1, symbol);
		if (!err)
			printf("duplicated symbol");
		printf("symbol '%s' added to symbol table\n", $1);

		LLVMBuildStore(cl2llvm_builder, $2, symbol->value);
	}
	| init_list TOK_COMMA TOK_ID init %prec TOK_MULT
	| TOK_ID array_deref_list init  %prec TOK_MULT {printf("init_list\n");}
	| init_list TOK_COMMA TOK_ID array_deref_list init %prec TOK_MULT
	;


declaration
	: declarator_list init_list TOK_SEMICOLON
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
	: TOK_IF TOK_PAR_OPEN expr TOK_PAR_CLOSE stmt_or_stmt_list
	| TOK_IF TOK_PAR_OPEN expr TOK_PAR_CLOSE stmt_or_stmt_list TOK_ELSE stmt_or_stmt_list
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
		LLVMValueRef op1;
		LLVMValueRef op2;
		LLVMTypeRef type;
		
		type = type_unify($1, $3, &op1, &op2);
		snprintf(temp_var_name, sizeof temp_var_name,
				"tmp%d", temp_var_count++);

		switch (LLVMGetTypeKind(type))
		{
		case LLVMIntegerTypeKind:

			$$ = LLVMBuildAdd(cl2llvm_builder, op1, op2, temp_var_name);
			break;

		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:

			$$ = LLVMBuildFAdd(cl2llvm_builder, op1, op2, temp_var_name);
			break;

		default:

			yyerror("invalid type of operands for addition");
		}
	}
	| expr TOK_MINUS expr
	{
		LLVMValueRef op1;
		LLVMValueRef op2;
		LLVMTypeRef type;
		
		type = type_unify($1, $3, &op1, &op2);
		snprintf(temp_var_name, sizeof temp_var_name,
				"tmp%d", temp_var_count++);

		switch (LLVMGetTypeKind(type))
		{
		case LLVMIntegerTypeKind:

			$$ = LLVMBuildSub(cl2llvm_builder, op1, op2, temp_var_name);
			break;

		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:

			$$ = LLVMBuildFSub(cl2llvm_builder, op1, op2, temp_var_name);
			break;

		default:

			yyerror("invalid type of operands for addition");
		}

	}
	| expr TOK_MULT expr
	{
		snprintf(temp_var_name, sizeof temp_var_name,
				"tmp%d", temp_var_count++);
		$$ = LLVMBuildMul(cl2llvm_builder, $1, $3, temp_var_name);
	}
	| expr TOK_DIV expr
	{
		snprintf(temp_var_name, sizeof temp_var_name,
				"tmp%d", temp_var_count++);
		$$ = LLVMBuildSDiv(cl2llvm_builder, $1, $3, temp_var_name);
	}
	| expr TOK_MOD expr
	{
		snprintf(temp_var_name, sizeof temp_var_name,
				"tmp%d", temp_var_count++);
		$$ = LLVMBuildSRem(cl2llvm_builder, $1, $3, temp_var_name);
	}
	| expr TOK_SHIFT_LEFT expr
	| expr TOK_SHIFT_RIGHT expr
	| expr TOK_EQUALITY expr
	{
		LLVMValueRef op1;
		LLVMValueRef op2;
		LLVMTypeRef type;
		
		type = type_unify($1, $3, &op1, &op2);
		snprintf(temp_var_name, sizeof temp_var_name,
				"tmp%d", temp_var_count++);

		switch (LLVMGetTypeKind(type))
		{
		case LLVMIntegerTypeKind:

			$$ = LLVMBuildICmp(cl2llvm_builder, LLVMIntEQ, op1, op2, temp_var_name);
			break;

		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:

			$$ = LLVMBuildFCmp(cl2llvm_builder, LLVMRealOEQ, op1, op2, temp_var_name);
			break;

		default:

			yyerror("invalid type of operands for addition");
		}

	}
	| expr TOK_INEQUALITY expr
	{
		LLVMValueRef op1;
		LLVMValueRef op2;
		LLVMTypeRef type;
		
		type = type_unify($1, $3, &op1, &op2);
		snprintf(temp_var_name, sizeof temp_var_name,
				"tmp%d", temp_var_count++);

		switch (LLVMGetTypeKind(type))
		{
		case LLVMIntegerTypeKind:

			$$ = LLVMBuildICmp(cl2llvm_builder, LLVMIntNE, op1, op2, temp_var_name);
			break;

		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:

			$$ = LLVMBuildFCmp(cl2llvm_builder, LLVMRealONE, op1, op2, temp_var_name);
			break;

		default:

			yyerror("invalid type of operands for addition");
		}
	}
	| expr TOK_LESS expr
	| expr TOK_GREATER expr
	| expr TOK_LESS_EQUAL expr
	| expr TOK_GREATER_EQUAL expr
	| expr TOK_LOGICAL_AND expr
	| expr TOK_LOGICAL_OR expr
	| expr TOK_EQUAL expr
	{
	}
	| expr TOK_ADD_EQUAL expr
	| expr TOK_MINUS_EQUAL expr
	| expr TOK_DIV_EQUAL expr
	| expr TOK_MULT_EQUAL expr
	| expr TOK_MOD_EQUAL expr
	| expr TOK_AND_EQUAL expr
	| expr TOK_OR_EQUAL expr
	| expr TOK_EXCLUSIVE_EQUAL expr
	| expr TOK_SHIFT_RIGHT_EQUAL expr
	| expr TOK_SHIFT_LEFT_EQUAL expr
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
	| TOK_INCREMENT lvalue %prec TOK_PREFIX
	| TOK_DECREMENT lvalue %prec TOK_PREFIX
	| lvalue TOK_DECREMENT %prec TOK_POSTFIX
	| TOK_MINUS primary %prec TOK_PREFIX
	| TOK_PLUS primary %prec TOK_PREFIX
	| TOK_PAR_OPEN type_spec TOK_PAR_CLOSE expr %prec TOK_PREFIX
	| TOK_SIZEOF TOK_PAR_OPEN type_spec TOK_PAR_CLOSE %prec TOK_PREFIX
	| TOK_BITWISE_NOT expr
	| TOK_BITWISE_AND lvalue %prec TOK_PREFIX
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
		printf("int added\n");
		$$ = LLVMConstInt(LLVMInt32Type(), $1, 0);
	}
	| TOK_CONST_DEC
	{
		printf("dec1 added\n");
		$$ = LLVMConstReal(LLVMFloatType(), $1);
	}
	| lvalue
	{
		snprintf(temp_var_name, sizeof temp_var_name,
				"tmp%d", temp_var_count++);
		$$ = LLVMBuildLoad(cl2llvm_builder, $1, temp_var_name);
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
		$$ = NULL;
	}
	| TOK_UCHAR
	{
		$$ = NULL;
	}
	| TOK_ULONG
	{
		$$ = NULL;
	}
	| TOK_USHORT
	{
		$$ = NULL;
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
		$$ = LLVMInt32Type();
	}
	| TOK_INT_LONG
	{
		$$ = NULL;
	}
	| TOK_CHAR
	{
		$$ = NULL;
	}
	| TOK_FLOAT
	{
		$$ = NULL;
	}
	| TOK_BOOL
	{
		$$ = NULL;
	}
	| TOK_DOUBLE
	{
		$$ = NULL;
	}
	|TOK_DOUBLE_LONG
	{
		$$ = NULL;
	}
	| TOK_VOID
	{
		$$ = NULL;
	}
	| TOK_HALF
	{
		$$ = NULL;
	}
	;




%%

