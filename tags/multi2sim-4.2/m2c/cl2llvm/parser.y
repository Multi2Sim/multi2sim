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

#include "vector-ops.h"
#include "for-blocks.h"
#include "while-blocks.h"
#include "arg.h"
#include "declarator-list.h"
#include "function.h"
#include "val.h"
#include "type.h"
#include "init.h"
#include "symbol.h"
#include "built-in-funcs.h"
#include "parser.h"
#include "cl2llvm.h"
#include "array.h"

#define CL2LLVM_MAX_FUNC_ARGS 64
#define CL2LLVM_MAX_ARG_NAME_LEN 200
#define CL2LLVM_MAX_NUM_ARRAY_INDEX_DIM 100

extern LLVMBuilderRef cl2llvm_builder;
extern LLVMModuleRef cl2llvm_module;
extern LLVMValueRef cl2llvm_function;
extern LLVMBasicBlockRef cl2llvm_basic_block;

extern int temp_var_count;
extern char temp_var_name[50];

int block_count;
char block_name[50];

int  func_count;
char func_name[50];

extern struct hash_table_t *cl2llvm_built_in_func_table;
struct hash_table_t *cl2llvm_symbol_table;
extern struct hash_table_t *cl2llvm_built_in_const_table;

struct cl2llvm_function_t *cl2llvm_current_function;
LLVMBasicBlockRef current_basic_block;

%}

%union {
	long int const_int_val;
	unsigned long long const_int_val_ull;
	double  const_float_val;
	char * identifier;
	struct cl2llvm_type_t *llvm_type_ref;
	struct cl2llvm_val_t *llvm_value_ref;
	struct list_t * init_list;
	struct list_t * list_val_t;
	struct cl2llvm_arg_t *arg_t;
	struct list_t *arg_list;
	LLVMBasicBlockRef basic_block_ref;
	struct cl2llvm_while_blocks_t *llvm_while_blocks;
	struct cl2llvm_for_blocks_t *llvm_for_blocks;
	struct cl2llvm_decl_list_t *decl_list;
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
%token<const_int_val> TOK_CHARN
%token TOK_CONSTANT
%token TOK_CONST
%token TOK_CONTINUE
%token TOK_DEFAULT
%token TOK_DO
%token TOK_DOUBLE
%token TOK_DOUBLE_LONG
%token<const_int_val> TOK_DOUBLEN
%token TOK_ENUM
%token TOK_EVENT_T
%token TOK_EXTERN
%token TOK_FLOAT
%token<const_int_val> TOK_FLOATN
%token TOK_FOR
%token TOK_GLOBAL
%token TOK_GOTO
%token TOK_HALF
%token TOK_IF
%nonassoc TOK_ELSE
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
%token<const_int_val> TOK_INTN
%token TOK_INTPTR_T
%token TOK_KERNEL
%token TOK_LOCAL
%token TOK_LONG
%token<const_int_val> TOK_LONGN
%token TOK_PRIVATE
%token TOK_PTRDIFF_T
%token TOK_READ_ONLY
%token TOK_READ_WRITE
%token TOK_REGISTER
%token TOK_RETURN
%token TOK_SAMPLER_T
%token TOK_SHORT
%token<const_int_val> TOK_SHORTN
%token TOK_SIGNED
%token TOK_SIZEOF
%token TOK_SIZE_T
%token TOK_STATIC
%token TOK_STRUCT
%token TOK_SWITCH
%token TOK_TYPEDEF
%token TOK_TYPENAME
%token<const_int_val> TOK_UCHARN
%token TOK_UCHAR
%token TOK_ULONG
%token TOK_USHORT
%token TOK_UINT
%token TOK_UINT_LONG
%token TOK_UINT_LONG_LONG
%token<const_int_val> TOK_UINTN
%token<const_int_val> TOK_ULONGN
%token TOK_UINTPTR_T
%token TOK_UNION
%token TOK_UNSIGNED
%token<const_int_val> TOK_USHORTN
%token TOK_VOID
%token TOK_VOLATILE
%token TOK_WHILE
%token TOK_WRITE_ONLY

%type<llvm_value_ref> primary
%type<llvm_value_ref> lvalue
%type<list_val_t> array_deref_list
%type<llvm_value_ref> maybe_expr
%type<llvm_value_ref> expr
%type<list_val_t> vec_literal_param_two_elem
%type<list_val_t> vec_literal_param_list
%type<llvm_value_ref> vec_literal
%type<llvm_value_ref> unary_expr
%type<llvm_value_ref> init
%type<init_list> init_list
%type<list_val_t> param_list
%type<llvm_value_ref> func_call
%type<llvm_type_ref> type_name
%type<const_int_val> addr_qual
%type<llvm_type_ref> type_spec
%type<decl_list> declarator
%type<decl_list> declarator_list
%type<arg_t> arg
%type<arg_list> arg_list
%type<basic_block_ref> if
%type<llvm_while_blocks> while_block_init
%type<llvm_for_blocks> for_loop_header


%start program

%%
program
	: external_def 
	| program external_def 
	;

external_def
	: func_def
	| func_decl
	;

func_decl
	: declarator_list TOK_ID TOK_PAR_OPEN arg_list TOK_PAR_CLOSE TOK_SEMICOLON
	{
		
		cl2llvm_yyerror("function declarations not supported");
	}
	;

func_def
	: declarator_list TOK_ID TOK_PAR_OPEN arg_list TOK_PAR_CLOSE TOK_CURLY_BRACE_OPEN
	{
		int err;
		struct cl2llvm_function_t *new_function;	
		char arg_name[CL2LLVM_MAX_ARG_NAME_LEN];
		LLVMTypeRef func_args[CL2LLVM_MAX_FUNC_ARGS];
		LLVMTypeRef function_type;
		LLVMValueRef function;
		int arg_count;
		int i;

		/* Create name for function entry block */
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		
		arg_count = list_count($4);

		/* Check that first argument is not empty.  This step is 
		necessary because of the grammar definition which allows 
		for the first argument of a multi-argument function to be empty. */
		if (list_get($4, $4->head) == NULL && arg_count > 1)
				yyerror("expected declaration specifiers or" 
					"'...' before ',' token");
		
		/* Read function arguments and create an array of argument types */
		else
		{
			for (i = 0; i < arg_count; i++)
			{
				
				struct cl2llvm_arg_t *current_arg = list_get(
					$4, i);
				if (!current_arg->name)
					yyerror("parameter name omitted");
				func_args[i] = current_arg->type_spec->llvm_type;
			}
		}

		/* Define function */
		function_type = LLVMFunctionType(
			$1->type_spec->llvm_type, func_args, arg_count, 0);
		function = LLVMAddFunction(cl2llvm_module, $2,
			function_type);
		LLVMSetFunctionCallConv(function, LLVMCCallConv);

		/* Create function's variable declaration block */
		LLVMBasicBlockRef basic_block = LLVMAppendBasicBlock( 
			function, block_name);
		
		/* Position builder at start of function */
		LLVMPositionBuilderAtEnd(cl2llvm_builder, basic_block);
			current_basic_block = basic_block;

		/* Create and initialize function object */
		new_function = cl2llvm_function_create($2, $4);

		new_function->func = function;
		new_function->func_type = function_type;
		new_function->sign = $1->type_spec->sign;
		new_function->entry_block = basic_block;

		/* Set current function to newly created function */
		cl2llvm_current_function = new_function;

		/* Insert function into global symbol table */
		err = hash_table_insert(cl2llvm_symbol_table, 
			$2, new_function);
		if (!err)
			yyerror("function already defined");
	
		/* Declare parameters if the argument list is not empty*/
		if (list_get($4, $4->head))
		{
			for (i = 0; i < arg_count; i++)
			{
				struct cl2llvm_arg_t *current_arg = list_get(
					$4, i);

				snprintf(arg_name, sizeof arg_name,
					"%s_addr", current_arg->name);

				struct cl2llvm_val_t *arg_pointer = 
					cl2llvm_val_create_w_init( 
					LLVMBuildAlloca(cl2llvm_builder, 
					current_arg->type_spec->llvm_type, 
					arg_name),
					current_arg->type_spec->sign);

				struct cl2llvm_symbol_t *symbol = 
					cl2llvm_symbol_create_w_init(
					arg_pointer->val, 
					current_arg->type_spec->sign, 
					current_arg->name);

				symbol->cl2llvm_val->type->llvm_type = 
					current_arg->type_spec->llvm_type;

				/* Name and store arguments */
				LLVMValueRef arg_val = LLVMGetParam(
					new_function->func, i);
				LLVMSetValueName(arg_val, current_arg->name);

				LLVMBuildStore(cl2llvm_builder, arg_val,
					arg_pointer->val);
				hash_table_insert(new_function->symbol_table,
					current_arg->name, symbol);

				/* Free argument */
				cl2llvm_val_free(arg_pointer);
			}
		}
		/* End declaration block and create first instruction block. */
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);

		LLVMBasicBlockRef first_block = LLVMAppendBasicBlock( 
			new_function->func, block_name);
		LLVMValueRef branch = LLVMBuildBr(cl2llvm_builder, 
			first_block);
		LLVMPositionBuilderAtEnd(cl2llvm_builder, first_block);

		/* Set current basic block */
		current_basic_block = first_block;

		/* initialize function branch instruction for building 
		declarations off of */
		new_function->branch_instr = branch;

		/* Free declarator list */
		cl2llvm_decl_list_free($1);	
	}
	stmt_list TOK_CURLY_BRACE_CLOSE
	{
		/* If function is of type void, create return instruction 
		by default*/
		if (LLVMGetReturnType(cl2llvm_current_function->func_type)
			== LLVMVoidType())
			LLVMBuildRetVoid(cl2llvm_builder);
	}
	; 



arg_list
	: /*empty*/
	{
		struct cl2llvm_arg_t *empty_arg = NULL;
		struct list_t *arg_list = list_create();
		list_add(arg_list, empty_arg);
		$$ = arg_list;
	}
	| arg
	{
		struct list_t *arg_list = list_create();
		list_add(arg_list, $1);
		$$ = arg_list;
	}
	| arg_list TOK_COMMA arg
	{
		list_add($1, $3);
		$$ = $1;
	}
	;

arg
	: declarator_list TOK_ID
	{
		struct cl2llvm_arg_t *arg = cl2llvm_arg_create($1, $2);
		cl2llvm_decl_list_struct_free($1);
		$$ = arg;
	}
	| declarator_list
	{
		struct cl2llvm_arg_t *arg = cl2llvm_arg_create($1, NULL);
		cl2llvm_decl_list_struct_free($1);
		$$ = arg;
	}
	;



declarator_list
	: declarator
	{
		$$ = $1;
	}
	| declarator_list declarator
	{
		cl2llvm_attach_decl_to_list($2, $1);
		if ($1->type_spec != NULL && $2->addr_qual != 0)
		{
			switch ($2->addr_qual)
			{
				case 1:
					$1->type_spec->llvm_type = LLVMPointerType( 
						LLVMGetElementType($1->type_spec->llvm_type), 1);
					break;
					
				case 2:
					$1->type_spec->llvm_type = LLVMPointerType(  
						LLVMGetElementType($1->type_spec->llvm_type), 2);
						break;

				default:
					break;
			}
		}
		if ($1->addr_qual != 0 && $2->type_spec != NULL)
		{
			switch ($1->addr_qual)
			{
				case 1:
					$1->type_spec->llvm_type = LLVMPointerType( 
						LLVMGetElementType($2->type_spec->llvm_type), 1);
						break;
					
				case 2:
					$1->type_spec->llvm_type = LLVMPointerType( 	
						LLVMGetElementType($2->type_spec->llvm_type), 2);
						break;

				default:
					break;
			}
		}
		cl2llvm_decl_list_struct_free($2);
		$$ = $1;
	}
	;

addr_qual
	: TOK_GLOBAL
	{
		$$ = 1;
	}
	| TOK_LOCAL
	{
		$$ = 2;
	}
	| TOK_PRIVATE
	{
		cl2llvm_yyerror("'private' not supported");
		$$ = 0;
	}
	| TOK_CONSTANT
	{
		cl2llvm_yyerror("'constant' not supported");
		$$ = 0;
	}
	;

declarator
	: type_spec 
	{
		struct cl2llvm_decl_list_t *decl_list = cl2llvm_decl_list_create();
		decl_list->type_spec = $1;
		$$ = decl_list;
	}
	| access_qual
	{
		struct cl2llvm_decl_list_t *decl_list = cl2llvm_decl_list_create();
		decl_list->access_qual = NULL;
		$$ = decl_list;
	}
	| TOK_KERNEL
	{
		struct cl2llvm_decl_list_t *decl_list = cl2llvm_decl_list_create();
		decl_list->kernel_t = 	"kernel";
		$$ = decl_list;
	}
	| TOK_INLINE
	{
		struct cl2llvm_decl_list_t *decl_list = cl2llvm_decl_list_create();
		decl_list->inline_t = NULL;
		$$ = decl_list;
	}
	| sc_spec
	{
		struct cl2llvm_decl_list_t *decl_list = cl2llvm_decl_list_create();
		decl_list->sc_spec = NULL;
		$$ = decl_list;

	}
	| addr_qual
	{
		struct cl2llvm_decl_list_t *decl_list = cl2llvm_decl_list_create();
		decl_list->addr_qual = $1;
		$$ = decl_list;
	}
	| type_qual
	{
		struct cl2llvm_decl_list_t *decl_list = cl2llvm_decl_list_create();
		decl_list->type_qual = NULL;
		$$ = decl_list;
	}
	;

type_qual
	: TOK_CONST
	| TOK_VOLATILE
	;

access_qual
	: TOK_READ_ONLY
	| TOK_WRITE_ONLY
	| TOK_READ_WRITE
	;


sc_spec
	: TOK_EXTERN
	{
		cl2llvm_yyerror("'extern' not supported");
	}
	| TOK_STATIC
	{
		cl2llvm_yyerror("'static' not supported");
	}
	;

stmt_list
	: /*empty*/
	| stmt_or_stmt_list stmt_list
	;

lvalue
	: TOK_PAR_OPEN lvalue TOK_PAR_CLOSE %prec TOK_MULT
	{
		$$ = $2;
	}
	| lvalue array_deref_list %prec TOK_MULT
	{
		struct cl2llvm_val_t *ptr;
		int i;
		struct cl2llvm_val_t *current_index;
		LLVMValueRef deref_ptr;
		LLVMValueRef indices[CL2LLVM_MAX_NUM_ARRAY_INDEX_DIM];

		/*Retrieve symbol from table*/
	 	/*symbol = hash_table_get(cl2llvm_current_function->symbol_table, $1);
		if (symbol == NULL)	
			yyerror("symbol undeclared first use in this program");*/
		
		deref_ptr = $1->val;

		/* Loop through all dereference operators */	
		for (i = 0; i < list_count($2); i++)
		{
			if  (LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(
				deref_ptr))) == LLVMPointerTypeKind)
			{
				snprintf(temp_var_name, sizeof temp_var_name,
					"tmp_%d", temp_var_count++);
			
				deref_ptr = LLVMBuildLoad(cl2llvm_builder, 
					deref_ptr, temp_var_name);

				current_index = list_get($2, i);
				indices[0] = current_index->val;

				snprintf(temp_var_name, sizeof temp_var_name,
					"tmp_%d", temp_var_count++);
				deref_ptr = LLVMBuildGEP(cl2llvm_builder, 
					deref_ptr, indices, 1, 
					temp_var_name);
				
			}
			else if (LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(
				deref_ptr))) == LLVMArrayTypeKind)

			{
				indices[0] = LLVMConstInt(LLVMInt32Type(), 0, 0);
				indices[1] = LLVMConstInt(LLVMInt32Type(), 0, 0);
				
				snprintf(temp_var_name, sizeof temp_var_name,
					"tmp_%d", temp_var_count++);
				
				deref_ptr = LLVMBuildGEP(cl2llvm_builder, 
					deref_ptr, indices, 2, temp_var_name); 

				current_index = list_get($2, i);
				indices[0] = current_index->val;
				

				/* Get element pointer */
				snprintf(temp_var_name, sizeof temp_var_name,
					"tmp_%d", temp_var_count++);

				deref_ptr = LLVMBuildGEP(cl2llvm_builder, 
					deref_ptr, indices, 1, temp_var_name);
			}
			else
				cl2llvm_yyerror("Subscripted value is neither array nor pointer");
		}

		ptr = cl2llvm_val_create_w_init(deref_ptr, 
			$1->type->sign);

		/*Free pointers*/
		LIST_FOR_EACH($2, i)
		{
			cl2llvm_val_free(list_get($2, i));
		}
		list_free($2);
		cl2llvm_val_free($1);

		$$ = ptr;
	}
	| TOK_MULT lvalue %prec TOK_MINUS
	{
		struct cl2llvm_val_t *ptr;
		LLVMValueRef deref_ptr;
		LLVMValueRef indices[CL2LLVM_MAX_NUM_ARRAY_INDEX_DIM];

		deref_ptr = $2->val;

		if  (LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(
			deref_ptr))) == LLVMPointerTypeKind)
		{
			snprintf(temp_var_name, sizeof temp_var_name,
				"tmp_%d", temp_var_count++);
			
			deref_ptr = LLVMBuildLoad(cl2llvm_builder, 
				deref_ptr, temp_var_name);

		}
		else if (LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(
			deref_ptr))) == LLVMArrayTypeKind)

		{
			indices[0] = LLVMConstInt(LLVMInt32Type(), 0, 0);
			indices[1] = LLVMConstInt(LLVMInt32Type(), 0, 0);
				
			snprintf(temp_var_name, sizeof temp_var_name,
				"tmp_%d", temp_var_count++);
				
			deref_ptr = LLVMBuildGEP(cl2llvm_builder, 
				deref_ptr, indices, 2, temp_var_name); 

			indices[0] = LLVMConstInt(LLVMInt32Type(), 0, 0);
				
			/* Get element pointer */
			snprintf(temp_var_name, sizeof temp_var_name,
				"tmp_%d", temp_var_count++);

			deref_ptr = LLVMBuildGEP(cl2llvm_builder, 
				deref_ptr, indices, 1, temp_var_name);
		}
		else
			cl2llvm_yyerror("Invalid type argument of unary '*'.");
		

		ptr = cl2llvm_val_create_w_init(deref_ptr, 
			$2->type->sign);

		$$ = ptr;
	}
	| TOK_ID %prec TOK_MINUS
	{
		struct cl2llvm_symbol_t *symbol;
		struct cl2llvm_val_t *symbol_val_dup;

		/* Access symbol from symbol table */
		symbol = hash_table_get(cl2llvm_current_function->symbol_table, $1);
		if (!symbol)
			yyerror("undefined identifier");

		/*Duplicate symbol*/
		symbol_val_dup = cl2llvm_val_create_w_init(symbol->cl2llvm_val->val, 
			symbol->cl2llvm_val->type->sign);


		$$ = symbol_val_dup;
	}
	| lvalue TOK_STRUCT_REF TOK_ID
	{

		
		/* Get symbol from hash table */
	/*	symbol = hash_table_get(cl2llvm_current_function->symbol_table, $1);
		if (!symbol)	
			yyerror("symbol undeclared first use in this program");
*/

		/* If symbol is a vector retrieve the specified indices */
		if (LLVMGetTypeKind(LLVMGetElementType($1->type->llvm_type))
			== LLVMVectorTypeKind)
		{	
			cl2llvm_get_vector_indices($1, $3);
		}
		$$ = $1;
	}
	;

array_deref_list
	: TOK_BRACKET_OPEN expr TOK_BRACKET_CLOSE
	{
		struct list_t *array_deref_list = list_create();
		
		if (LLVMGetTypeKind($2->type->llvm_type) != LLVMIntegerTypeKind)
			yyerror("array index is not an integer");
		list_add(array_deref_list, $2);
		$$ = array_deref_list;
	}
	| array_deref_list TOK_BRACKET_OPEN expr TOK_BRACKET_CLOSE
	{
		if (LLVMGetTypeKind($3->type->llvm_type) != LLVMIntegerTypeKind)
			yyerror("array index is not an integer");
		list_add($1, $3);
		$$ = $1;
	}
	;

stmt
	: maybe_expr TOK_SEMICOLON
	{
		if ($1 != NULL)
			cl2llvm_val_free($1);
	}
	| declaration
	| func_def
	| func_decl
	| for_loop
	| while_loop
	| do_while_loop
	| if_stmt
	| TOK_RETURN expr TOK_SEMICOLON
	{
		struct cl2llvm_val_t *ret_val;
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init( LLVMGetReturnType(cl2llvm_current_function->func_type), cl2llvm_current_function->sign);
		if (type->llvm_type != $2->type->llvm_type || type->sign != $2->type->sign)
			ret_val = llvm_type_cast($2, type);
		else
			ret_val = $2;
		LLVMBuildRet(cl2llvm_builder, ret_val->val);
		cl2llvm_type_free(type);
		if (ret_val != $2)
			cl2llvm_val_free(ret_val);
		cl2llvm_val_free($2);
	}
	| TOK_CONTINUE TOK_SEMICOLON
	{
		cl2llvm_yyerror("'continue' not supported");
	}
	| TOK_BREAK TOK_SEMICOLON
	{
		cl2llvm_yyerror("'break' not supported");
	}
	| switch_stmt
	{
		cl2llvm_yyerror("'switch' not supported");
	}
	| label_stmt
	{
		cl2llvm_yyerror("'label statements not supported");
	}
	| goto_stmt
	{
		cl2llvm_yyerror("'goto' not supported");
	}
	;

func_call
	: TOK_ID TOK_PAR_OPEN param_list TOK_PAR_CLOSE
	{
		int *func_id;
		int i;
		LLVMValueRef cast_param_array[100];
		struct cl2llvm_arg_t *current_func_arg;
		struct cl2llvm_val_t *current_param;
		struct cl2llvm_function_t *function;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *cast_param;
		LLVMValueRef llvm_val_func_ret;
		struct cl2llvm_val_t *ret_val;
		LLVMTypeRef llvm_param_types[100];
		int offset = 0;

		/* If function is found in the built-in function table but not the
		   global symbol table, declare it and insert it into the  global
		   symbol table. */
		func_id = hash_table_get(cl2llvm_built_in_func_table, $1);
		if (func_id && !hash_table_get(cl2llvm_symbol_table, $1))
			func_declare(func_id);
	
		/* Retrieve function specs from sybmol table */
		function = hash_table_get(cl2llvm_symbol_table, $1);
		if (!function)
			yyerror("implicit declaration of function");
		
		/* If specified function arg count differs from the arg count of
		   the generated function, get offset */
		if (function->arg_count != LLVMCountParamTypes(function->func_type))
		{
			LLVMGetParamTypes(function->func_type, llvm_param_types);

			offset = LLVMCountParamTypes(function->func_type) -
				function->arg_count;
			for (i = 0; i < offset; i++)
			{
				cast_param_array[i] = LLVMConstInt(llvm_param_types[i], 0, 0);
			}		
		}

		/* check that parameter types match */
		for (i = 0; i < function->arg_count; i++)
		{
			current_func_arg = list_get(function->arg_list, i);
			current_param = list_get($3, i);
			if (current_func_arg->type_spec->llvm_type != current_param->type->llvm_type)
			{
				type = cl2llvm_type_create_w_init( 
					current_func_arg->type_spec->llvm_type,
					current_func_arg->type_spec->sign);
				cast_param = llvm_type_cast(current_param, type);
				cl2llvm_type_free(type);
				cast_param_array[i + offset] = cast_param->val;
				cl2llvm_val_free(current_param);
				cl2llvm_val_free(cast_param);
			}
			else
			{
				cast_param_array[i + offset] = current_param->val;
				cl2llvm_val_free(current_param);
			}
		}
		list_free($3);


		/* Build function call */
		if (LLVMGetReturnType(function->func_type) == LLVMVoidType())
		{
			llvm_val_func_ret = LLVMBuildCall(cl2llvm_builder, function->func,
				cast_param_array, function->arg_count + offset, "");
		}
		else
		{
			snprintf(temp_var_name, sizeof temp_var_name,
				"tmp_%d", temp_var_count++);

			llvm_val_func_ret = LLVMBuildCall(cl2llvm_builder, function->func,
				cast_param_array, function->arg_count + offset, temp_var_name);
		}

		/* Create return value */
		ret_val = cl2llvm_val_create_w_init(llvm_val_func_ret, 	
			function->sign);
		
		$$ = ret_val;
	}
	| TOK_ID TOK_PAR_OPEN TOK_PAR_CLOSE
	{
		cl2llvm_yyerror("function calls with no parameters not supported");
		$$ = NULL;
	}
	;

param_list
	: expr
	{
		struct list_t *param_list = list_create();
		list_add(param_list, $1);
		$$ = param_list;
	}
	| array_deref_list TOK_EQUAL expr
	{
		cl2llvm_yyerror("array initializers not supported");
		$$ = NULL;
	}
	| array_init
	{
		cl2llvm_yyerror("array initializers not supported");
		$$ = NULL;
	}
	| param_list TOK_COMMA expr
	{
		list_add($1, $3);
		$$ = $1;
	}
	| param_list TOK_COMMA array_init
	{
		cl2llvm_yyerror("array initializers not supported");
		$$ = NULL;
	}
	| param_list TOK_COMMA array_deref_list TOK_EQUAL expr
	{
		cl2llvm_yyerror("array initializers not supported");
		$$ = NULL;
	}
	;

array_init
	: TOK_CURLY_BRACE_OPEN param_list TOK_CURLY_BRACE_CLOSE
	;

init
	: /*empty*/
	{
		$$ = NULL;
	}
	| TOK_EQUAL expr
	{
		$$ = $2;
	}
	| TOK_EQUAL array_init
	{
		cl2llvm_yyerror("array initializers not supported");
		$$ = NULL;
	}
	;

init_list
	: TOK_ID init %prec TOK_MULT
	{
		struct list_t *init_list = list_create();
		struct cl2llvm_init_t *init = cl2llvm_init_create($1);

		cl2llvm_val_free(init->cl2llvm_val);
		init->cl2llvm_val = $2;

		list_add(init_list, init);
		$$ = init_list;
	}
	| init_list TOK_COMMA TOK_ID init %prec TOK_MULT
	{
		struct cl2llvm_init_t *init = cl2llvm_init_create($3);

		cl2llvm_val_free(init->cl2llvm_val);
		init->cl2llvm_val = $4;

		list_add($1, init);
		$$ = $1;
	}
	| TOK_ID array_deref_list init  %prec TOK_MULT
	{
		struct list_t *init_list = list_create();
		struct cl2llvm_init_t *init = cl2llvm_init_create($1);

		cl2llvm_val_free(init->cl2llvm_val);
		init->cl2llvm_val = $3;
		init->array_deref_list = $2;

		list_add(init_list, init);
		$$ = init_list;

	}
	| init_list TOK_COMMA TOK_ID array_deref_list init %prec TOK_MULT
	{
		struct cl2llvm_init_t *init = cl2llvm_init_create($3);

		cl2llvm_val_free(init->cl2llvm_val);
		init->cl2llvm_val = $5;
		init->array_deref_list = $4;

		list_add($1, init);
		$$ = $1;
	}
	;


declaration
	: declarator_list init_list TOK_SEMICOLON
	{
		struct cl2llvm_val_t *cl2llvm_index;
		LLVMTypeRef type;
		struct cl2llvm_val_t *ptr;
		struct cl2llvm_val_t *value;
		int i;
		struct cl2llvm_symbol_t *symbol;
		struct cl2llvm_val_t *cast_to_val;
		int init_count = list_count($2);
		struct cl2llvm_init_t *current_list_elem;
		struct cl2llvm_symbol_t *err_symbol;
		char error_message[50];
		LLVMValueRef var_addr;

		/*Create each sybmol in the init_list*/
		for(i = 0; i < init_count; i++)
		{	
			int err;			
			current_list_elem = list_get($2, i);
			
			/*if variable type is a vector*/
			if (LLVMGetTypeKind($1->type_spec->llvm_type) == LLVMVectorTypeKind)
			{	
				/*Go to entry block and declare variable*/
				LLVMPositionBuilder(cl2llvm_builder, cl2llvm_current_function->entry_block, cl2llvm_current_function->branch_instr);
				var_addr = LLVMBuildAlloca(cl2llvm_builder, 
					$1->type_spec->llvm_type, current_list_elem->name);
				LLVMPositionBuilderAtEnd(cl2llvm_builder, current_basic_block);

				/*Create symbol*/
				symbol = cl2llvm_symbol_create_w_init( var_addr, 
					$1->type_spec->sign, current_list_elem->name);
				
				/* If initializer is present, store it. */
				if (current_list_elem->cl2llvm_val != NULL)
				{

					if (LLVMTypeOf(current_list_elem->cl2llvm_val->val) == $1->type_spec->llvm_type 
						&& current_list_elem->cl2llvm_val->type->sign == $1->type_spec->sign)
					{
						LLVMBuildStore(cl2llvm_builder, 
							current_list_elem->cl2llvm_val->val, var_addr);
					}
					else 
					{
						yyerror("type of vector initializer does not match type of declarator");
					}
				}
			}
			/*If data type is not an array*/
			else if (current_list_elem->array_deref_list == NULL)
			{
				/*Go to entry block and declare variable*/
				LLVMPositionBuilder(cl2llvm_builder, cl2llvm_current_function->entry_block, cl2llvm_current_function->branch_instr);
				var_addr = LLVMBuildAlloca(cl2llvm_builder, 
					$1->type_spec->llvm_type, current_list_elem->name);
				LLVMPositionBuilderAtEnd(cl2llvm_builder, current_basic_block);

				/*Create symbol*/
				symbol = cl2llvm_symbol_create_w_init( var_addr, 
					$1->type_spec->sign, current_list_elem->name);
				symbol->cl2llvm_val->type->llvm_type = $1->type_spec->llvm_type;

				
				/*If initializer is present, cast initializer to declarator 
				  type and store*/
				if (current_list_elem->cl2llvm_val != NULL)
				{
					cast_to_val = llvm_type_cast( 
						current_list_elem->cl2llvm_val, 
						$1->type_spec);
					LLVMBuildStore(cl2llvm_builder, cast_to_val->val, 
						symbol->cl2llvm_val->val);
					cl2llvm_val_free(cast_to_val);
				}
			}
			/*If init is an array*/
			else
			{		
				int i;
				int j;
				
				/* Check that array sizes are constants */
				for (i = 0; i < list_count(
					current_list_elem->array_deref_list); i++)
				{	
					value = list_get(current_list_elem->array_deref_list, i);
					if (!LLVMIsConstant(value->val))
						cl2llvm_yyerror("Array size must be a constant.");
				}

				/* Create type for pointer to array. */
				type = $1->type_spec->llvm_type;

				for (i = 0; i < list_count(
					current_list_elem->array_deref_list); i++)
				{
					cl2llvm_index = list_get(current_list_elem->array_deref_list, i);
					j = 0;
					while(LLVMConstInt(LLVMInt1Type() ,1 ,0) 
						== LLVMConstICmp(LLVMIntSLT, 
						LLVMConstInt(LLVMInt32Type(), j,
						0), cl2llvm_index->val))
					{
						j++;
					}

					type = LLVMArrayType(type, j);
				}

				/* Go to entry block and allocate array pointer */
				LLVMPositionBuilder(cl2llvm_builder,
					cl2llvm_current_function->entry_block,
					cl2llvm_current_function->branch_instr);

				ptr = cl2llvm_val_create_w_init( LLVMBuildAlloca( 
					cl2llvm_builder, type,
					current_list_elem->name), 
					$1->type_spec->sign);
	
				LLVMPositionBuilderAtEnd(cl2llvm_builder,
					current_basic_block);


				
				/* Create symbol */
				symbol = cl2llvm_symbol_create_w_init( 
					ptr->val , $1->type_spec->sign, 
					current_list_elem->name);

				
				cl2llvm_val_free(ptr);

			}
			/* Insert symbol into symbol table */
			err = hash_table_insert(cl2llvm_current_function->symbol_table, 
				current_list_elem->name, symbol);
			if (!err)
			{
				snprintf(error_message, sizeof error_message,
					"Symbol '%s' previously declared.", current_list_elem->name);

				cl2llvm_yyerror(error_message);
			}
			/* Check that symbol is not a reserved constant */
			err_symbol = hash_table_get(cl2llvm_built_in_const_table, 
				current_list_elem->name);
			if (err_symbol)
			{
				snprintf(error_message, sizeof error_message,
					"'%s' expected an identifier.", current_list_elem->name);
				cl2llvm_yyerror(error_message);
			}

		}

		/* Free pointers */
		cl2llvm_decl_list_free($1);
		LIST_FOR_EACH($2, i)
		{
			cl2llvm_init_free(list_get($2, i));
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
	: if %prec TOK_PLUS
	{
		/* goto endif block*/
		LLVMBuildBr(cl2llvm_builder, $1);
		LLVMPositionBuilderAtEnd(cl2llvm_builder, $1);
		current_basic_block = $1;
	}
	| if TOK_ELSE
	{ 
		/*create endif block. $1 now becomes the if false block*/
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef endif = LLVMAppendBasicBlock(cl2llvm_current_function->func, block_name);
		
		/*Branch to endif*/
		LLVMBuildBr(cl2llvm_builder, endif);
		/*position builder at if false block*/
		LLVMPositionBuilderAtEnd(cl2llvm_builder, $1);
		current_basic_block = $1;

		$<basic_block_ref>$ = endif;
	}
	stmt_or_stmt_list %prec TOK_MULT
	{
		/*branch to endif block and prepare to write code for endif block*/
		LLVMBuildBr(cl2llvm_builder, $<basic_block_ref>3);
		LLVMPositionBuilderAtEnd(cl2llvm_builder, $<basic_block_ref>3);
		current_basic_block = $<basic_block_ref>3;

	}
	;

if
	:  TOK_IF TOK_PAR_OPEN expr TOK_PAR_CLOSE
	{
		struct cl2llvm_type_t *i1 = cl2llvm_type_create_w_init(LLVMInt1Type(), 1);
		
		/*Create endif block*/
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef endif = LLVMAppendBasicBlock(cl2llvm_current_function->func, block_name);
		
		/*Create if true block*/
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef if_true = LLVMAppendBasicBlock(cl2llvm_current_function->func, block_name);
		
		/*evaluate expression*/
		struct cl2llvm_val_t *bool_val =  cl2llvm_val_bool($3);
		LLVMBuildCondBr(cl2llvm_builder, bool_val->val, if_true, endif);
		$<basic_block_ref>$ = endif;
		
		/*prepare to write if_true block*/
		LLVMPositionBuilderAtEnd(cl2llvm_builder, if_true);
		current_basic_block = if_true;
	
		cl2llvm_val_free(bool_val);
		cl2llvm_val_free($3);
		cl2llvm_type_free(i1);
	}
	stmt_or_stmt_list
	{
		$$ = $<basic_block_ref>5;
	}
	;

for_loop
	: for_loop_header stmt_or_stmt_list
	{
		/*Finish for loop statement*/
		LLVMBuildBr(cl2llvm_builder, $1->for_incr);

		/*Prepare to build for end*/
		LLVMPositionBuilderAtEnd(cl2llvm_builder, $1->for_end);
		current_basic_block = $1->for_end;

		/*Free for_blocks*/
		cl2llvm_for_blocks_free($1);

	}
	;

for_loop_header
	: TOK_FOR TOK_PAR_OPEN maybe_expr TOK_SEMICOLON
	{
		struct cl2llvm_for_blocks_t *for_blocks;
		
		/*Initialize "for" blocks*/
		for_blocks = cl2llvm_for_blocks_create();
		
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef for_cond = LLVMAppendBasicBlock(cl2llvm_current_function->func,
			block_name);
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef for_stmt = LLVMAppendBasicBlock(cl2llvm_current_function->func,
			block_name);
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef for_incr = LLVMAppendBasicBlock(cl2llvm_current_function->func,
			block_name);
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef for_end = LLVMAppendBasicBlock(cl2llvm_current_function->func, 
			block_name);

		for_blocks->for_cond = for_cond;
		for_blocks->for_stmt = for_stmt;
		for_blocks->for_incr = for_incr;
		for_blocks->for_end = for_end;

		/*Prepare to build for loop conditional*/
		LLVMBuildBr(cl2llvm_builder, for_blocks->for_cond);
		LLVMPositionBuilderAtEnd(cl2llvm_builder, for_blocks->for_cond);

		$<llvm_for_blocks>$ = for_blocks;
	}
	maybe_expr TOK_SEMICOLON
	{
		struct cl2llvm_val_t *bool_val = NULL;

		/*Build for loop conditional*/
		if ($6 != NULL)
		{
			bool_val = cl2llvm_val_bool($6);
			LLVMBuildCondBr(cl2llvm_builder, bool_val->val, 
				$<llvm_for_blocks>5->for_stmt, $<llvm_for_blocks>5->for_end);	
		}
		else
		{
			LLVMBuildBr(cl2llvm_builder, $<llvm_for_blocks>5->for_stmt);
		}
		
		/*Prepare to build for loop increment*/
		LLVMPositionBuilderAtEnd(cl2llvm_builder, $<llvm_for_blocks>5->for_incr);

			
		/*Free pointers*/
		if (bool_val != NULL)
			cl2llvm_val_free(bool_val);
		if ($3 != NULL)
			cl2llvm_val_free($3);
		if ($6 != NULL)
			cl2llvm_val_free($6);
	}
	maybe_expr TOK_PAR_CLOSE
	{
		/*Finish for loop increment*/
		LLVMBuildBr(cl2llvm_builder, $<llvm_for_blocks>5->for_cond);
			
		/*Prepare to build for loop statements*/
		LLVMPositionBuilderAtEnd(cl2llvm_builder, $<llvm_for_blocks>5->for_stmt);
		current_basic_block = $<llvm_for_blocks>5->for_stmt;

		/*Free pointers*/
		if ($9 != NULL)
			cl2llvm_val_free($9);

		$$ = $<llvm_for_blocks>5;
	}
	| TOK_FOR TOK_PAR_OPEN declaration
	{
		struct cl2llvm_for_blocks_t *for_blocks;
		
		/*Initialize "for" blocks*/
		for_blocks = cl2llvm_for_blocks_create();
		
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef for_cond = LLVMAppendBasicBlock(cl2llvm_current_function->func,
			block_name);
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef for_stmt = LLVMAppendBasicBlock(cl2llvm_current_function->func,
			block_name);
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef for_incr = LLVMAppendBasicBlock(cl2llvm_current_function->func,
			block_name);
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef for_end = LLVMAppendBasicBlock(cl2llvm_current_function->func, 
			block_name);

		for_blocks->for_cond = for_cond;
		for_blocks->for_stmt = for_stmt;
		for_blocks->for_incr = for_incr;
		for_blocks->for_end = for_end;

		/*Prepare to build for loop conditional*/
		LLVMBuildBr(cl2llvm_builder, for_blocks->for_cond);
		LLVMPositionBuilderAtEnd(cl2llvm_builder, for_blocks->for_cond);

		$<llvm_for_blocks>$ = for_blocks;
		
	}
	maybe_expr TOK_SEMICOLON
	{
		struct cl2llvm_val_t *bool_val = NULL;

		/*Build for loop conditional*/
		if ($5 != NULL)
		{
			bool_val = cl2llvm_val_bool($5);
			LLVMBuildCondBr(cl2llvm_builder, bool_val->val, 
				$<llvm_for_blocks>4->for_stmt, $<llvm_for_blocks>4->for_end);
		}
		else
		{
			LLVMBuildBr(cl2llvm_builder, $<llvm_for_blocks>4->for_stmt);
		}

		/*Prepare to build for loop increment*/
		LLVMPositionBuilderAtEnd(cl2llvm_builder, $<llvm_for_blocks>4->for_incr);
		
		/*Free pointers*/
		if (bool_val != NULL)
			cl2llvm_val_free(bool_val);
		if ($5 != NULL)
			cl2llvm_val_free($5);
		
	}
	maybe_expr TOK_PAR_CLOSE
	{
		LLVMBuildBr(cl2llvm_builder, $<llvm_for_blocks>4->for_cond);

		/*Prepare to build for loop statements*/
		LLVMPositionBuilderAtEnd(cl2llvm_builder, $<llvm_for_blocks>4->for_stmt);
		current_basic_block = $<llvm_for_blocks>4->for_stmt;
		/*Free pointers*/
		if ($8 != NULL)
			cl2llvm_val_free($8);

		$$ = $<llvm_for_blocks>4;

	}
	;

do_while_loop
	: TOK_DO
	{
		struct cl2llvm_while_blocks_t *while_blocks = cl2llvm_while_blocks_create();

		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef while_stmt = LLVMAppendBasicBlock(cl2llvm_current_function->func, block_name);

		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef while_cond = LLVMAppendBasicBlock(cl2llvm_current_function->func, block_name);
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef while_end = LLVMAppendBasicBlock(cl2llvm_current_function->func, block_name);
		
		LLVMBuildBr(cl2llvm_builder, while_stmt);
		LLVMPositionBuilderAtEnd(cl2llvm_builder, while_stmt);
		current_basic_block = while_stmt;

		while_blocks->while_stmt = while_stmt;
		while_blocks->while_cond = while_cond;
		while_blocks->while_end = while_end;

		$<llvm_while_blocks>$ = while_blocks;
	}
	stmt_or_stmt_list TOK_WHILE TOK_PAR_OPEN
	{
		LLVMBuildBr(cl2llvm_builder, $<llvm_while_blocks>2->while_cond);
		LLVMPositionBuilderAtEnd(cl2llvm_builder, $<llvm_while_blocks>2->while_cond);
		current_basic_block = $<llvm_while_blocks>2->while_cond;
	}
	expr TOK_PAR_CLOSE TOK_SEMICOLON 
	{
		struct cl2llvm_val_t *bool_val;
		
		bool_val = cl2llvm_val_bool($7);
		LLVMBuildCondBr(cl2llvm_builder, bool_val->val, $<llvm_while_blocks>2->while_stmt,
			$<llvm_while_blocks>2->while_end);
		LLVMPositionBuilderAtEnd(cl2llvm_builder, $<llvm_while_blocks>2->while_end);
		current_basic_block = $<llvm_while_blocks>2->while_end;
	}
	;

while_loop
	: TOK_WHILE TOK_PAR_OPEN while_block_init expr TOK_PAR_CLOSE
	{
		struct cl2llvm_val_t *bool_val;
		bool_val = cl2llvm_val_bool($4);
		LLVMBuildCondBr(cl2llvm_builder, bool_val->val, $3->while_stmt, $3->while_end);

		LLVMPositionBuilderAtEnd(cl2llvm_builder, $3->while_stmt);
	
		current_basic_block = $3->while_stmt;
	
		cl2llvm_val_free(bool_val);
		cl2llvm_val_free($4);
	}
	stmt_or_stmt_list
	{
		LLVMBuildBr(cl2llvm_builder, $3->while_cond);
		LLVMPositionBuilderAtEnd(cl2llvm_builder, $3->while_end);
		current_basic_block = $3->while_end;
		cl2llvm_while_blocks_free($3);
	}
	;

while_block_init
	: /*empty*/
	{
		struct cl2llvm_while_blocks_t *while_blocks = cl2llvm_while_blocks_create();

		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef while_stmt = LLVMAppendBasicBlock(cl2llvm_current_function->func, block_name);

		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef while_cond = LLVMAppendBasicBlock(cl2llvm_current_function->func, block_name);
		snprintf(block_name, sizeof block_name,
			"block_%d", block_count++);
		LLVMBasicBlockRef while_end = LLVMAppendBasicBlock(cl2llvm_current_function->func, block_name);
		
		LLVMBuildBr(cl2llvm_builder, while_cond);
		LLVMPositionBuilderAtEnd(cl2llvm_builder, while_cond);

		while_blocks->while_stmt = while_stmt;
		while_blocks->while_cond = while_cond;
		while_blocks->while_end = while_end;

		$$ = while_blocks;
	}
	;

maybe_expr
	: /*empty*/
	{
		$$ = NULL;
	}
	| expr
	{
		$$ = $1;
	}
	;
expr
	: primary

	| TOK_PAR_OPEN expr TOK_PAR_CLOSE
	{
		$$ = $2;
	}
	| expr TOK_PLUS expr
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_val_t *value;
		struct cl2llvm_type_t *type = cl2llvm_type_create();
		struct cl2llvm_val_t *op1, *op2;
		
		/* Find out which value differs from the original and set the
		   dominant type equal to the type of that value. */
		type_unify($1, $3, &op1, &op2);
		if(op1 == $1)
		{
			type->llvm_type = op1->type->llvm_type;
			type->sign = op1->type->sign;
		}
		else
		{
			type->llvm_type = op2->type->llvm_type;
			type->sign = op2->type->sign;
		}

		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		snprintf(temp_var_name, sizeof temp_var_name,
				"tmp_%d", temp_var_count++);
		
		value = cl2llvm_val_create();

		/* Determine which type of addition to use based on type of
		   operators. */
		switch (LLVMGetTypeKind(switch_type->llvm_type))
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
		
		/* Free pointers */
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free( op2);
		cl2llvm_val_free($3);
		cl2llvm_val_free($1);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;
	}
	| expr TOK_MINUS expr
	{
		struct cl2llvm_val_t *value;
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type = cl2llvm_type_create();
		struct cl2llvm_val_t *op1, *op2;

		/* Find out which value differs from the original and set the
		   dominant type equal to the type of that value. */	
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
		
		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		snprintf(temp_var_name, sizeof temp_var_name,
				"tmp_%d", temp_var_count++);

		value = cl2llvm_val_create();
		switch (LLVMGetTypeKind(switch_type->llvm_type))
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

			yyerror("invalid type of operands for subtraction");
		}
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);

		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;
	}
	| expr TOK_MULT expr
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value;

		type = cl2llvm_type_create();
		value = cl2llvm_val_create();

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);

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
		
		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		switch (LLVMGetTypeKind(switch_type->llvm_type))
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

		/* Free pointers */
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;

	}
	| expr TOK_DIV expr
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value;
		
		type = cl2llvm_type_create();
		value = cl2llvm_val_create();

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);

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

		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		switch (LLVMGetTypeKind(switch_type->llvm_type))
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

		/* Free pointers */
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;
	}
	| expr TOK_MOD expr
	{
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value;
		
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
	
		type = cl2llvm_type_create();
		value = cl2llvm_val_create();

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
		
		/* Free pointers */
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
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value;
		
		type = cl2llvm_type_create();
		value = cl2llvm_val_create();

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);

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

		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		switch (LLVMGetTypeKind(switch_type->llvm_type))
		{
		case LLVMIntegerTypeKind:
			value->val = LLVMBuildShl(cl2llvm_builder, 
				op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;


		default:
			
			yyerror("Invalid type of operands for '<<'.");
		}

		/* Free pointers */
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;

	}
	| expr TOK_SHIFT_RIGHT expr
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value;
		
		type = cl2llvm_type_create();
		value = cl2llvm_val_create();

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);

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

		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		switch (LLVMGetTypeKind(switch_type->llvm_type))
		{
		case LLVMIntegerTypeKind:
			value->val = LLVMBuildAShr(cl2llvm_builder, 
				op1->val, op2->val, temp_var_name);
			value->type->sign = type->sign;
			value->type->llvm_type = type->llvm_type;
			break;


		default:
			
			yyerror("Invalid type of operands for '>>'.");
		}

		/* Free pointers */
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;
	}
	| expr TOK_EQUALITY expr
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value;
		
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);

		type = cl2llvm_type_create();
		value = cl2llvm_val_create();

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

		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(switch_type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}
		
		switch (LLVMGetTypeKind(switch_type->llvm_type))
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

		/* Free pointers */
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;
	}
	| expr TOK_INEQUALITY expr
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value;
		
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);

		type = cl2llvm_type_create();
		value = cl2llvm_val_create();

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

		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(switch_type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		switch (LLVMGetTypeKind(switch_type->llvm_type))
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

		/* Free pointers */
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;

	}
	| expr TOK_LESS expr
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value;
		
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);

		type = cl2llvm_type_create();
		value = cl2llvm_val_create();

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

		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(switch_type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		switch (LLVMGetTypeKind(switch_type->llvm_type))
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

			yyerror("invalid type of operands for comparison");
		}

		/* Free pointers */
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;

	}
	| expr TOK_GREATER expr
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value;
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);

		type = cl2llvm_type_create();
		value = cl2llvm_val_create();

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

		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(switch_type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		switch (LLVMGetTypeKind(switch_type->llvm_type))
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

			yyerror("invalid type of operands for comparison");
		}

		/* Free pointers */
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;
	}
	| expr TOK_LESS_EQUAL expr
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value;
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);

		type = cl2llvm_type_create();
		value = cl2llvm_val_create();

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

		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(switch_type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		switch (LLVMGetTypeKind(switch_type->llvm_type))
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

			yyerror("invalid type of operands for comparison");
		}

		/* Free pointers */
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;
	}
	| expr TOK_GREATER_EQUAL expr
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value;
		
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);

		type = cl2llvm_type_create();
		value = cl2llvm_val_create();

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

		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(switch_type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		switch (LLVMGetTypeKind(switch_type->llvm_type))
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

			yyerror("invalid type of operands for comparison");
		}

		/* Free pointers */
		if ($1 != op1)
			cl2llvm_val_free(op1);
		else if ($3 != op2)
			cl2llvm_val_free(op2);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;
	}
	| expr TOK_LOGICAL_AND expr
	{
		struct cl2llvm_val_t *bool1;
		struct cl2llvm_val_t *bool2;
		struct cl2llvm_val_t *value;
		LLVMBasicBlockRef land_rhs;
		LLVMBasicBlockRef land_end;
		LLVMValueRef land_cond;

		/* Create basic blocks */
		land_rhs = LLVMAppendBasicBlock(cl2llvm_current_function->func, "land_rhs");
		land_end = LLVMAppendBasicBlock(cl2llvm_current_function->func, "land_end");

		/* Convert LHS to boolean value */
		bool1 = cl2llvm_val_bool($1);

		/* Create conditional branch. Branch will skip to end block if LHS
		   is false */
		LLVMBuildCondBr(cl2llvm_builder, bool1->val, land_rhs, land_end);
		
		/* Move builder to RHS block */
		LLVMPositionBuilderAtEnd(cl2llvm_builder, land_rhs);

		/* Convert RHS to Bool */
		bool2 = cl2llvm_val_bool($3);

		/* Build branch to end block */
		LLVMBuildBr(cl2llvm_builder, land_end);
		LLVMPositionBuilderAtEnd(cl2llvm_builder, land_end);

		/* Position builder at end block */
		
		/* Build Phi node */
		LLVMValueRef phi_vals[] = {LLVMConstInt(LLVMInt1Type(), 0, 0), bool2->val};
		LLVMBasicBlockRef phi_blocks[] = {current_basic_block, land_rhs};
		land_cond = LLVMBuildPhi(cl2llvm_builder, LLVMInt1Type(), "land_cond");
		LLVMAddIncoming(land_cond, phi_vals, phi_blocks, 2);

		/* Set current current basic block */
		current_basic_block = land_end;

		value = cl2llvm_val_create_w_init(land_cond, 1);

		/* Free pointers */
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_val_free(bool1);
		cl2llvm_val_free(bool2);
	
		$$ = value;
	}
	| expr TOK_LOGICAL_OR expr
	{
		struct cl2llvm_val_t *bool1;
		struct cl2llvm_val_t *bool2;
		struct cl2llvm_val_t *value;
		LLVMBasicBlockRef land_rhs;
		LLVMBasicBlockRef land_end;
		LLVMValueRef land_cond;

		/* Create basic blocks */
		land_rhs = LLVMAppendBasicBlock(cl2llvm_current_function->func, "land_rhs");
		land_end = LLVMAppendBasicBlock(cl2llvm_current_function->func, "land_end");

		/* Convert LHS to boolean value */
		bool1 = cl2llvm_val_bool($1);

		/* Create conditional branch. Branch will skip to end block if LHS
		   is true */
		LLVMBuildCondBr(cl2llvm_builder, bool1->val, land_end, land_rhs);
		
		/* Move builder to RHS block */
		LLVMPositionBuilderAtEnd(cl2llvm_builder, land_rhs);

		/* Convert RHS to Bool */
		bool2 = cl2llvm_val_bool($3);

		/* Build branch to end block */
		LLVMBuildBr(cl2llvm_builder, land_end);

		/* Position builder at end block */
		LLVMPositionBuilderAtEnd(cl2llvm_builder, land_end);
		
		/* Build Phi node */
		LLVMValueRef phi_vals[] = {LLVMConstInt(LLVMInt1Type(), 1, 0), bool2->val};
		LLVMBasicBlockRef phi_blocks[] = {current_basic_block, land_rhs};
		land_cond = LLVMBuildPhi(cl2llvm_builder, LLVMInt1Type(), "land_cond");
		LLVMAddIncoming(land_cond, phi_vals, phi_blocks, 2);

		/* Set current current basic block */
		current_basic_block = land_end;

		value = cl2llvm_val_create_w_init(land_cond, 1);

		/* Free pointers */
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_val_free(bool1);
		cl2llvm_val_free(bool2);
	
		$$ = value;
	}
	| lvalue TOK_EQUAL expr
	{
		struct cl2llvm_val_t *value;
		struct cl2llvm_type_t *type; 
		
		type = cl2llvm_type_create_w_init(LLVMGetElementType($1->type->llvm_type) , $1->type->sign);

		/* If lvalue is a component referenced vector. */
		if ($1->vector_indices[0])
		{
			value = cl2llvm_build_component_wise_assignment($1, $3);
		}
		else
		{
			/*Cast rvalue to type of lvalue and store*/
			value = llvm_type_cast($3, type);
			LLVMBuildStore(cl2llvm_builder, value->val, $1->val);
		}

		/*Free pointers*/
		cl2llvm_type_free(type);
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);

		$$ = value;
	}
	| lvalue TOK_ADD_EQUAL expr
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;		struct cl2llvm_val_t *value;

		type = cl2llvm_type_create_w_init(LLVMGetElementType(
			$1->type->llvm_type), $1->type->sign);

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
		struct cl2llvm_val_t *lval = cl2llvm_val_create_w_init(
			LLVMBuildLoad(cl2llvm_builder, $1->val, temp_var_name),
			$1->type->sign);

		struct cl2llvm_val_t *rval = llvm_type_cast($3, type);
	
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
		
		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(switch_type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		switch (LLVMGetTypeKind(switch_type->llvm_type))
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
		cl2llvm_type_free(switch_type);

		$$ = value;
	}
	| lvalue TOK_MINUS_EQUAL expr
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value;
		
		type = cl2llvm_type_create_w_init(LLVMGetElementType(
			$1->type->llvm_type), $1->type->sign);

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
		struct cl2llvm_val_t *lval = cl2llvm_val_create_w_init(
			LLVMBuildLoad(cl2llvm_builder, $1->val, temp_var_name),
			$1->type->sign);

		struct cl2llvm_val_t *rval = llvm_type_cast($3, type);

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
		
		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(switch_type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		switch (LLVMGetTypeKind(switch_type->llvm_type))
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
	
		/* Free pointers */
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_val_free(rval);
		cl2llvm_val_free(lval);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;
	}
	| lvalue TOK_DIV_EQUAL expr
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value;

		type = cl2llvm_type_create_w_init( 
			LLVMGetElementType($1->type->llvm_type) , $1->type->sign);
		value = cl2llvm_val_create();

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
		struct cl2llvm_val_t *lval = cl2llvm_val_create_w_init(
			LLVMBuildLoad(cl2llvm_builder, $1->val, temp_var_name),
			$1->type->sign);
	
		struct cl2llvm_val_t *rval = llvm_type_cast($3, type);

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
		
		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(switch_type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		switch (LLVMGetTypeKind(switch_type->llvm_type))
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
		
		/* Free pointers */
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_val_free(rval);
		cl2llvm_val_free(lval);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;
	}
	| lvalue TOK_MULT_EQUAL expr
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value;

		type = cl2llvm_type_create_w_init(LLVMGetElementType(
			$1->type->llvm_type), $1->type->sign);
		value = cl2llvm_val_create();

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
		struct cl2llvm_val_t *lval = cl2llvm_val_create_w_init(
			LLVMBuildLoad(cl2llvm_builder, $1->val, temp_var_name),
			$1->type->sign);
	
		struct cl2llvm_val_t *rval = llvm_type_cast($3, type);

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
		
		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(switch_type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		switch (LLVMGetTypeKind(switch_type->llvm_type))
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
	
		/* Free pointers */
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_val_free(lval);
		cl2llvm_val_free(rval);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;
	}
	| lvalue TOK_MOD_EQUAL expr
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value;
	
		type = cl2llvm_type_create_w_init(LLVMGetElementType(
			$1->type->llvm_type), $1->type->sign);
		value = cl2llvm_val_create();

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
		struct cl2llvm_val_t *lval = cl2llvm_val_create_w_init(
			LLVMBuildLoad(cl2llvm_builder, $1->val, temp_var_name),
			$1->type->sign);
	
		struct cl2llvm_val_t *rval = llvm_type_cast($3, type);

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
		
		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(switch_type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		switch (LLVMGetTypeKind(switch_type->llvm_type))
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
		
		/* Free pointers */
		cl2llvm_val_free($1);
		cl2llvm_val_free($3);
		cl2llvm_val_free(rval);
		cl2llvm_val_free(lval);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = value;
	}
	| lvalue TOK_AND_EQUAL expr
	{
		cl2llvm_yyerror("'&=' not supported");
	}
	| lvalue TOK_OR_EQUAL expr
	{
		cl2llvm_yyerror("'!=' not supported");
	}
	| lvalue TOK_EXCLUSIVE_EQUAL expr
	{
		cl2llvm_yyerror("'^=' not supported");
	}
	| lvalue TOK_SHIFT_RIGHT_EQUAL expr
	{

		cl2llvm_yyerror("'>>=' not supported");
	}
	| lvalue TOK_SHIFT_LEFT_EQUAL expr
	{
		cl2llvm_yyerror("'<<=' not supported");
	}
	| expr TOK_CONDITIONAL expr TOK_COLON expr
	{
		cl2llvm_yyerror("a'?'b':'c not supported");
	}
	| unary_expr
	{
		$$ = $1;
	}

	| func_call
	{
		$$ = $1;
	}

	| TOK_LOGICAL_NEGATE expr
	{
		cl2llvm_yyerror("'!' not supported");
		$$ = NULL;
	}

	| expr TOK_BITWISE_AND expr
	{
		
		cl2llvm_yyerror("bitwise and '&' not supported");
	}
	| expr TOK_BITWISE_OR expr
	{
		
		cl2llvm_yyerror("bitwise or '|' not supported");
	}
	| expr TOK_BITWISE_EXCLUSIVE expr
	{
		
		cl2llvm_yyerror("bitwise exclusive or '^' not supported");
	}
	;


unary_expr
	: lvalue TOK_INCREMENT %prec TOK_POSTFIX
	{
		struct cl2llvm_type_t *switch_type;
		struct cl2llvm_type_t *type;
		struct cl2llvm_val_t *value_plus_one;
		struct cl2llvm_val_t *one;

		/* Create constant one to add to variable */
		one = cl2llvm_val_create_w_init(LLVMConstInt(LLVMInt32Type(), 1, 0), 1);

		type = cl2llvm_type_create_w_init(LLVMGetElementType(
			$1->type->llvm_type), $1->type->sign);

		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
		struct cl2llvm_val_t *lval = cl2llvm_val_create_w_init(
			LLVMBuildLoad(cl2llvm_builder, $1->val, temp_var_name),
			$1->type->sign);

		/* Cast constant one to type of variable */
		struct cl2llvm_val_t *cast_one = llvm_type_cast(one, type);
	
		snprintf(temp_var_name, sizeof temp_var_name,
			"tmp_%d", temp_var_count++);
		
		/* Create an object that will hold the type of the operands.
		   This extra object is necessary since in the case of a vector 
		   type, we are concerned with the type of its components, but the
		   resultant type of the operation is a vector. */
		switch_type = cl2llvm_type_create_w_init(type->llvm_type, type->sign);
		if (LLVMGetTypeKind(switch_type->llvm_type) == LLVMVectorTypeKind)
		{
			switch_type->llvm_type = LLVMGetElementType(type->llvm_type);
		}

		switch (LLVMGetTypeKind(switch_type->llvm_type))
		{
		case LLVMIntegerTypeKind:

			value_plus_one = cl2llvm_val_create_w_init(
				LLVMBuildAdd(cl2llvm_builder, lval->val, 
				cast_one->val, temp_var_name), type->sign);
			break;

		case LLVMHalfTypeKind:
		case LLVMFloatTypeKind:
		case LLVMDoubleTypeKind:

			value_plus_one = cl2llvm_val_create_w_init(
				LLVMBuildFAdd(cl2llvm_builder, lval->val, 
				cast_one->val, temp_var_name), type->sign);
			break;

		default:

			yyerror("invalid type of operand for post '++'");
			value_plus_one = cl2llvm_val_create();
		}
		
		LLVMBuildStore(cl2llvm_builder, value_plus_one->val, $1->val);
		
		cl2llvm_val_free(one);
		cl2llvm_val_free(value_plus_one);
		cl2llvm_val_free(cast_one);
		cl2llvm_val_free(lval);
		cl2llvm_type_free(type);
		cl2llvm_type_free(switch_type);

		$$ = $1;

	}
	| TOK_INCREMENT lvalue %prec TOK_PREFIX
	{	
		cl2llvm_yyerror("pre-increment not supported");
		$$ = NULL;
	}
	| TOK_DECREMENT lvalue %prec TOK_PREFIX
	{
		cl2llvm_yyerror("pre decrement not supported");
		$$ = NULL;
	}
	| lvalue TOK_DECREMENT %prec TOK_POSTFIX
	{
		cl2llvm_yyerror("post decrement not supported");
		$$ = NULL;
	}
	| TOK_MINUS primary %prec TOK_PREFIX
	{
		cl2llvm_yyerror("unary minus not supported");
		$$ = NULL;
	}
	| TOK_PLUS primary %prec TOK_PREFIX
	{
		cl2llvm_yyerror("unary plus not supported");
		$$ = NULL;
	}
	| TOK_PAR_OPEN type_spec TOK_PAR_CLOSE expr %prec TOK_PREFIX
	{
		struct cl2llvm_val_t *value;

		value = llvm_type_cast($4, $2);
	
		/* Free pointers */
		cl2llvm_val_free($4);
		cl2llvm_type_free($2);
		$$ = value;
	}
	| TOK_SIZEOF TOK_PAR_OPEN type_spec TOK_PAR_CLOSE %prec TOK_PREFIX
	{
		cl2llvm_yyerror("'sizeof' not supported");
		$$ = NULL;
	}
	| TOK_BITWISE_NOT expr
	{
		cl2llvm_yyerror("'~'not supported");
		$$ = NULL;
	}
	| TOK_BITWISE_AND lvalue %prec TOK_PREFIX
	{
		cl2llvm_yyerror("'&' not supported");
		$$ = NULL;
	}
	;


/* The vector_literal_param_list is technically the same as the param_list, with
 * the difference that here we need at least two elements in the list. Different
 * syntax rules are created to avoid shift/reduce conflicts. */
vec_literal
	: TOK_PAR_OPEN type_spec TOK_PAR_CLOSE TOK_PAR_OPEN vec_literal_param_list TOK_PAR_CLOSE
	{
		int index;
		struct cl2llvm_val_t *current_vec_elem;
		struct cl2llvm_type_t *elem_type;
		struct cl2llvm_val_t *cast_val;
		struct cl2llvm_val_t *value;
		struct cl2llvm_val_t *blank_elem = cl2llvm_val_create_w_init( LLVMConstInt(LLVMInt32Type(), 0, 0), 1);
		struct cl2llvm_val_t *cast_index;
		struct cl2llvm_val_t *cl2llvm_index;
		LLVMValueRef vec_const_elems[16];
		int vec_nonconst_elems[16];
		int elem_count = 0;
		int non_const_elem_count = 0;

		cast_index = NULL;
		cast_val = NULL;
		current_vec_elem = NULL;

		snprintf(temp_var_name, sizeof(temp_var_name),
			"tmp_%d", temp_var_count++);
		/* Create type object to represent element type */
		elem_type = cl2llvm_type_create_w_init(LLVMGetElementType($2->llvm_type), $2->sign);
		/*Go to entry block and declare vector*/
		LLVMPositionBuilder(cl2llvm_builder, cl2llvm_current_function->entry_block, cl2llvm_current_function->branch_instr);
		LLVMValueRef vec_addr = LLVMBuildAlloca(cl2llvm_builder, 
			$2->llvm_type, temp_var_name);
		LLVMPositionBuilderAtEnd(cl2llvm_builder, current_basic_block);

		/*Expand any vectors present in list*/
		expand_vectors($5);

		/*Iterate over list and build a vector of all constant entries*/
		for (index = 0; index < list_count($5); ++index)
		{
			current_vec_elem = list_get($5, index);
			if(LLVMIsConstant(current_vec_elem->val) == 1)
			{
				
				cast_val = llvm_type_cast(current_vec_elem, elem_type);
				vec_const_elems[index] = cast_val->val;
				vec_nonconst_elems[index] = 0;
				elem_count++;
			}
			else
			{
				cast_val = llvm_type_cast(blank_elem, elem_type);
				vec_const_elems[index] = cast_val->val;
				vec_nonconst_elems[index] = 1;
				elem_count++;
				non_const_elem_count++;
			}
			if (elem_count > LLVMGetVectorSize($2->llvm_type))
				yyerror("Too many elements in vector literal");
			cl2llvm_val_free(cast_val);
		}
		if (elem_count < LLVMGetVectorSize($2->llvm_type))
			yyerror("Too few elements in vector literal");
		
		LLVMValueRef new_vector = LLVMConstVector(vec_const_elems, elem_count);

		/*Store constant vector*/
		if(non_const_elem_count < elem_count)
			LLVMBuildStore(cl2llvm_builder, new_vector, vec_addr);

		
		/*If there are non-constant entries in vector, insert them*/
		if (non_const_elem_count)
		{
			for (index = 0; index < elem_count; index++)
			{	
				cl2llvm_index = cl2llvm_val_create_w_init( LLVMConstInt(LLVMInt32Type(), index, 0), 1);
				cast_val = NULL;
				cast_index = NULL;

				current_vec_elem = list_get($5, index);
				if (vec_nonconst_elems[index])
				{
					cast_val = llvm_type_cast(current_vec_elem, elem_type);
					
					snprintf(temp_var_name, sizeof(temp_var_name),
						"tmp_%d", temp_var_count++);

					LLVMValueRef vector_load = LLVMBuildLoad( cl2llvm_builder, vec_addr, temp_var_name);

					snprintf(temp_var_name, sizeof(temp_var_name),
						"tmp_%d", temp_var_count++);
					cast_index = llvm_type_cast(cl2llvm_index, elem_type);

					new_vector = LLVMBuildInsertElement( cl2llvm_builder, vector_load, cast_val->val, cl2llvm_index->val, temp_var_name);

					LLVMBuildStore(cl2llvm_builder, new_vector, vec_addr);

				}
				if (cast_val != NULL)
					cl2llvm_val_free(cast_val);
				if (cast_index != NULL)
					cl2llvm_val_free(cast_index);
				cl2llvm_val_free(cl2llvm_index);
			}

		}
		cl2llvm_val_free(blank_elem);
		cl2llvm_type_free(elem_type);

		LIST_FOR_EACH($5, index)
		{
			cl2llvm_val_free(list_get($5, index));
		}
		list_free($5);

		value = cl2llvm_val_create_w_init(new_vector, $2->sign);
		
		$$ = value;

		cl2llvm_type_free($2);
	}
	;

vec_literal_param_two_elem
	: expr TOK_COMMA expr
	{
		struct list_t *vec_elem_list;

		vec_elem_list = list_create();
		list_add(vec_elem_list, $1);
		list_add(vec_elem_list, $3);

		$$ = vec_elem_list;

	}
	;

vec_literal_param_list
	: vec_literal_param_two_elem
	{
		$$ = $1;
	}
	| vec_literal_param_list TOK_COMMA expr
	{
		list_add($1, $3);

		$$ = $1;
	}
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
			LLVMConstReal(LLVMDoubleType(), $1), 1);
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
	| lvalue %prec TOK_MINUS
	{
		int i;
		int component_count = 0;
		LLVMValueRef new_vector_addr;
		LLVMValueRef new_vector;
		LLVMValueRef component;
		
		snprintf(temp_var_name, sizeof(temp_var_name),
			"tmp_%d", temp_var_count++);
		struct cl2llvm_val_t *value = cl2llvm_val_create_w_init(
			LLVMBuildLoad(cl2llvm_builder, $1->val, temp_var_name),
			$1->type->sign);

		/* If value is an array element, retrieve element type */
		if (LLVMGetTypeKind(value->type->llvm_type) == LLVMArrayTypeKind
			|| LLVMGetTypeKind(value->type->llvm_type) == LLVMPointerTypeKind
			|| LLVMGetTypeKind(value->type->llvm_type) == LLVMStructTypeKind)
		{
			value->type->llvm_type = LLVMGetElementType(value->type->llvm_type);
		}

		/* If element is of vector type, check for component indices. */
		if (LLVMGetTypeKind(LLVMGetElementType($1->type->llvm_type))
			== LLVMVectorTypeKind && $1->vector_indices[i])
		{
			/* Get vector component count */
			while($1->vector_indices[component_count])
				component_count++;
			/* If there are multiple components */
			if (component_count == 2 || component_count == 3
				|| component_count == 4 || component_count == 8
				|| component_count == 16)
			{

				snprintf(temp_var_name, sizeof temp_var_name,
					"tmp_%d", temp_var_count++);

				/* Go to entry block and allocate new vector */
				LLVMPositionBuilder(cl2llvm_builder, 
					cl2llvm_current_function->entry_block,
					cl2llvm_current_function->branch_instr);
			
				new_vector_addr = LLVMBuildAlloca(cl2llvm_builder,
					LLVMVectorType(LLVMGetElementType(value->type->llvm_type),
					component_count), temp_var_name);

				LLVMPositionBuilderAtEnd(cl2llvm_builder, current_basic_block);


				/* Load new vector */
				snprintf(temp_var_name, sizeof temp_var_name,
					"tmp_%d", temp_var_count++);
				new_vector = LLVMBuildLoad(cl2llvm_builder, new_vector_addr,
					temp_var_name);

				while ($1->vector_indices[i])
				{
					snprintf(temp_var_name, sizeof temp_var_name,
						"tmp_%d", temp_var_count++);
					component = LLVMBuildExtractElement(cl2llvm_builder, 
						value->val, $1->vector_indices[i]->val,
						temp_var_name);

					snprintf(temp_var_name, sizeof temp_var_name,
						"tmp_%d", temp_var_count++);
					new_vector = LLVMBuildInsertElement(cl2llvm_builder, 
						new_vector, component, $1->vector_indices[i]->val,
						temp_var_name);
					i++;
				}
				value->val = new_vector;
				value->type->llvm_type = LLVMTypeOf(new_vector);
			}
			else if (component_count == 1)
			{
				snprintf(temp_var_name, sizeof temp_var_name,
					"tmp_%d", temp_var_count++);
				
				component = LLVMBuildExtractElement(cl2llvm_builder, 
					value->val, $1->vector_indices[0]->val,
					temp_var_name);
				value->val = component;
				value->type->llvm_type = LLVMTypeOf(component);
			}
			else
				cl2llvm_yyerror("Invalid vector type.");
		}
			

		cl2llvm_val_free($1);

		$$ = value;
	}

	| vec_literal
	{
		$$ = $1;
	}
	;


type_spec
	: type_name
	{
		$$ = $1;
	}
	| type_spec TOK_MULT
	{
		LLVMTypeRef ptr_type;
		struct cl2llvm_type_t *type;
		
		/* Create pointer type */
		ptr_type = LLVMPointerType($1->llvm_type, 0);
		
		type = cl2llvm_type_create_w_init(ptr_type, $1->sign);
	
		/* Free pointers */
		cl2llvm_type_free($1);
		
		$$ = type;
	}
	;

type_name
	: TOK_INTPTR_T
	{
		cl2llvm_yyerror("'intptr_t' not supported");
		$$ = NULL;
	}
	| TOK_PTRDIFF_T
	{
		cl2llvm_yyerror("'ptrdiff_t' not supported");
		$$ = NULL;
	}
	| TOK_UINTPTR_T
	{
		cl2llvm_yyerror("'uintptr_t' not supported");
		$$ = NULL;
	}
	| TOK_SAMPLER_T
	{
		cl2llvm_yyerror("'sampler_t' not supported");
		$$ = NULL;
	}
	| TOK_EVENT_T
	{
		cl2llvm_yyerror("'event_t' not supported");
		$$ = NULL;
	}
	| TOK_IMAGE2D_T
	{
		cl2llvm_yyerror("'image2d_t' not supported");
		$$ = NULL;
	}
	| TOK_IMAGE3D_T
	{
		cl2llvm_yyerror("'image3d_t' not supported");
		$$ = NULL;
	}
	| TOK_IMAGE2D_ARRAY_T
	{
		cl2llvm_yyerror("'image2d_array_t' not supported");
		$$ = NULL;
	}
	| TOK_IMAGE1D_T
	{
		cl2llvm_yyerror("'image1d_t' not supported");
		$$ = NULL;
	}
	| TOK_IMAGE1D_BUFFER_T
	{
		cl2llvm_yyerror("'image1d_buffer_t' not supported");
		$$ = NULL;
	}
	| TOK_IMAGE1D_ARRAY_T
	{
		cl2llvm_yyerror("'image1d_array_t' not supported");
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
		struct cl2llvm_type_t *type;
		type = cl2llvm_type_create_w_init(LLVMVectorType(LLVMInt32Type(), $1), 0);
		$$ = type;

	}
	| TOK_UCHARN
	{
		struct cl2llvm_type_t *type;
		type = cl2llvm_type_create_w_init(LLVMVectorType(LLVMInt8Type(), $1), 0);
		$$ = type;

	}
	| TOK_ULONGN
	{
		struct cl2llvm_type_t *type;
		type = cl2llvm_type_create_w_init(LLVMVectorType(LLVMInt32Type(), $1), 0);
		$$ = type;

	}
	| TOK_USHORTN
	{
		struct cl2llvm_type_t *type;
		type = cl2llvm_type_create_w_init(LLVMVectorType(LLVMInt16Type(), $1), 0);
		$$ = type;

	}
	| TOK_SHORTN
	{
		struct cl2llvm_type_t *type;
		type = cl2llvm_type_create_w_init(LLVMVectorType(LLVMInt16Type(), $1), 1);
		$$ = type;

	}
	| TOK_INTN
	{
		struct cl2llvm_type_t *type;
		type = cl2llvm_type_create_w_init(LLVMVectorType(LLVMInt32Type(), $1), 1);
		$$ = type;
	}
	| TOK_LONGN
	{
		struct cl2llvm_type_t *type;
		type = cl2llvm_type_create_w_init(LLVMVectorType(LLVMInt32Type(), $1), 1);
		$$ = type;
	}
	| TOK_CHARN
	{
		struct cl2llvm_type_t *type;
		type = cl2llvm_type_create_w_init(LLVMVectorType(LLVMInt8Type(), $1), 1);
		$$ = type;
	}
	| TOK_FLOATN
	{
		struct cl2llvm_type_t *type;
		type = cl2llvm_type_create_w_init(LLVMVectorType(LLVMFloatType(), $1), 0);
		$$ = type;
	}
	| TOK_DOUBLEN
	{
		struct cl2llvm_type_t *type;
		type = cl2llvm_type_create_w_init(LLVMVectorType(LLVMDoubleType(), $1), 0);
		$$ = type;
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
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMVoidType(), 1);
		$$ = type;

	}
	| TOK_HALF
	{
		struct cl2llvm_type_t *type = cl2llvm_type_create_w_init(
			LLVMHalfType(), 1);
		$$ = type;
	}
	;




%%

