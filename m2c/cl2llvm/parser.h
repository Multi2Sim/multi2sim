/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_CL2LLVM_YY_PARSER_H_INCLUDED
# define YY_CL2LLVM_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int cl2llvm_yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOK_ID = 258,
    TOK_CONST_INT = 259,
    TOK_CONST_INT_U = 260,
    TOK_CONST_INT_L = 261,
    TOK_CONST_INT_UL = 262,
    TOK_CONST_INT_LL = 263,
    TOK_CONST_INT_ULL = 264,
    TOK_CONST_DEC = 265,
    TOK_CONST_DEC_H = 266,
    TOK_CONST_DEC_F = 267,
    TOK_CONST_DEC_L = 268,
    TOK_CONST_VAL = 269,
    TOK_STRING = 270,
    TOK_COMMA = 271,
    TOK_SEMICOLON = 272,
    TOK_ELLIPSIS = 273,
    TOK_EQUAL = 274,
    TOK_ADD_EQUAL = 275,
    TOK_MINUS_EQUAL = 276,
    TOK_MULT_EQUAL = 277,
    TOK_DIV_EQUAL = 278,
    TOK_MOD_EQUAL = 279,
    TOK_AND_EQUAL = 280,
    TOK_OR_EQUAL = 281,
    TOK_EXCLUSIVE_EQUAL = 282,
    TOK_SHIFT_RIGHT_EQUAL = 283,
    TOK_SHIFT_LEFT_EQUAL = 284,
    TOK_CONDITIONAL = 285,
    TOK_COLON = 286,
    TOK_LOGICAL_OR = 287,
    TOK_LOGICAL_AND = 288,
    TOK_BITWISE_OR = 289,
    TOK_BITWISE_EXCLUSIVE = 290,
    TOK_BITWISE_AND = 291,
    TOK_EQUALITY = 292,
    TOK_INEQUALITY = 293,
    TOK_GREATER = 294,
    TOK_LESS = 295,
    TOK_GREATER_EQUAL = 296,
    TOK_LESS_EQUAL = 297,
    TOK_SHIFT_RIGHT = 298,
    TOK_SHIFT_LEFT = 299,
    TOK_PLUS = 300,
    TOK_MINUS = 301,
    TOK_MULT = 302,
    TOK_DIV = 303,
    TOK_MOD = 304,
    TOK_PREFIX = 305,
    TOK_LOGICAL_NEGATE = 306,
    TOK_BITWISE_NOT = 307,
    TOK_CURLY_BRACE_CLOSE = 308,
    TOK_CURLY_BRACE_OPEN = 309,
    TOK_PAR_OPEN = 310,
    TOK_PAR_CLOSE = 311,
    TOK_BRACKET_OPEN = 312,
    TOK_BRACKET_CLOSE = 313,
    TOK_POSTFIX = 314,
    TOK_STRUCT_REF = 315,
    TOK_STRUCT_DEREF = 316,
    TOK_DECREMENT = 317,
    TOK_INCREMENT = 318,
    TOK_AUTO = 319,
    TOK_BOOL = 320,
    TOK_BREAK = 321,
    TOK_CASE = 322,
    TOK_CHAR = 323,
    TOK_CHARN = 324,
    TOK_CONSTANT = 325,
    TOK_CONST = 326,
    TOK_CONTINUE = 327,
    TOK_DEFAULT = 328,
    TOK_DO = 329,
    TOK_DOUBLE = 330,
    TOK_DOUBLE_LONG = 331,
    TOK_DOUBLEN = 332,
    TOK_ENUM = 333,
    TOK_EVENT_T = 334,
    TOK_EXTERN = 335,
    TOK_FLOAT = 336,
    TOK_FLOATN = 337,
    TOK_FOR = 338,
    TOK_GLOBAL = 339,
    TOK_GOTO = 340,
    TOK_HALF = 341,
    TOK_IF = 342,
    TOK_ELSE = 343,
    TOK_IMAGE2D_T = 344,
    TOK_IMAGE3D_T = 345,
    TOK_IMAGE2D_ARRAY_T = 346,
    TOK_IMAGE1D_T = 347,
    TOK_IMAGE1D_BUFFER_T = 348,
    TOK_IMAGE1D_ARRAY_T = 349,
    TOK_INLINE = 350,
    TOK_INT = 351,
    TOK_INT_LONG = 352,
    TOK_LONG_LONG = 353,
    TOK_INTN = 354,
    TOK_INTPTR_T = 355,
    TOK_KERNEL = 356,
    TOK_LOCAL = 357,
    TOK_LONG = 358,
    TOK_LONGN = 359,
    TOK_PRIVATE = 360,
    TOK_PTRDIFF_T = 361,
    TOK_READ_ONLY = 362,
    TOK_READ_WRITE = 363,
    TOK_REGISTER = 364,
    TOK_RETURN = 365,
    TOK_SAMPLER_T = 366,
    TOK_SHORT = 367,
    TOK_SHORTN = 368,
    TOK_SIGNED = 369,
    TOK_SIZEOF = 370,
    TOK_SIZE_T = 371,
    TOK_STATIC = 372,
    TOK_STRUCT = 373,
    TOK_SWITCH = 374,
    TOK_TYPEDEF = 375,
    TOK_TYPENAME = 376,
    TOK_UCHARN = 377,
    TOK_UCHAR = 378,
    TOK_ULONG = 379,
    TOK_USHORT = 380,
    TOK_UINT = 381,
    TOK_UINT_LONG = 382,
    TOK_UINT_LONG_LONG = 383,
    TOK_UINTN = 384,
    TOK_ULONGN = 385,
    TOK_UINTPTR_T = 386,
    TOK_UNION = 387,
    TOK_UNSIGNED = 388,
    TOK_USHORTN = 389,
    TOK_VOID = 390,
    TOK_VOLATILE = 391,
    TOK_WHILE = 392,
    TOK_WRITE_ONLY = 393
  };
#endif
/* Tokens.  */
#define TOK_ID 258
#define TOK_CONST_INT 259
#define TOK_CONST_INT_U 260
#define TOK_CONST_INT_L 261
#define TOK_CONST_INT_UL 262
#define TOK_CONST_INT_LL 263
#define TOK_CONST_INT_ULL 264
#define TOK_CONST_DEC 265
#define TOK_CONST_DEC_H 266
#define TOK_CONST_DEC_F 267
#define TOK_CONST_DEC_L 268
#define TOK_CONST_VAL 269
#define TOK_STRING 270
#define TOK_COMMA 271
#define TOK_SEMICOLON 272
#define TOK_ELLIPSIS 273
#define TOK_EQUAL 274
#define TOK_ADD_EQUAL 275
#define TOK_MINUS_EQUAL 276
#define TOK_MULT_EQUAL 277
#define TOK_DIV_EQUAL 278
#define TOK_MOD_EQUAL 279
#define TOK_AND_EQUAL 280
#define TOK_OR_EQUAL 281
#define TOK_EXCLUSIVE_EQUAL 282
#define TOK_SHIFT_RIGHT_EQUAL 283
#define TOK_SHIFT_LEFT_EQUAL 284
#define TOK_CONDITIONAL 285
#define TOK_COLON 286
#define TOK_LOGICAL_OR 287
#define TOK_LOGICAL_AND 288
#define TOK_BITWISE_OR 289
#define TOK_BITWISE_EXCLUSIVE 290
#define TOK_BITWISE_AND 291
#define TOK_EQUALITY 292
#define TOK_INEQUALITY 293
#define TOK_GREATER 294
#define TOK_LESS 295
#define TOK_GREATER_EQUAL 296
#define TOK_LESS_EQUAL 297
#define TOK_SHIFT_RIGHT 298
#define TOK_SHIFT_LEFT 299
#define TOK_PLUS 300
#define TOK_MINUS 301
#define TOK_MULT 302
#define TOK_DIV 303
#define TOK_MOD 304
#define TOK_PREFIX 305
#define TOK_LOGICAL_NEGATE 306
#define TOK_BITWISE_NOT 307
#define TOK_CURLY_BRACE_CLOSE 308
#define TOK_CURLY_BRACE_OPEN 309
#define TOK_PAR_OPEN 310
#define TOK_PAR_CLOSE 311
#define TOK_BRACKET_OPEN 312
#define TOK_BRACKET_CLOSE 313
#define TOK_POSTFIX 314
#define TOK_STRUCT_REF 315
#define TOK_STRUCT_DEREF 316
#define TOK_DECREMENT 317
#define TOK_INCREMENT 318
#define TOK_AUTO 319
#define TOK_BOOL 320
#define TOK_BREAK 321
#define TOK_CASE 322
#define TOK_CHAR 323
#define TOK_CHARN 324
#define TOK_CONSTANT 325
#define TOK_CONST 326
#define TOK_CONTINUE 327
#define TOK_DEFAULT 328
#define TOK_DO 329
#define TOK_DOUBLE 330
#define TOK_DOUBLE_LONG 331
#define TOK_DOUBLEN 332
#define TOK_ENUM 333
#define TOK_EVENT_T 334
#define TOK_EXTERN 335
#define TOK_FLOAT 336
#define TOK_FLOATN 337
#define TOK_FOR 338
#define TOK_GLOBAL 339
#define TOK_GOTO 340
#define TOK_HALF 341
#define TOK_IF 342
#define TOK_ELSE 343
#define TOK_IMAGE2D_T 344
#define TOK_IMAGE3D_T 345
#define TOK_IMAGE2D_ARRAY_T 346
#define TOK_IMAGE1D_T 347
#define TOK_IMAGE1D_BUFFER_T 348
#define TOK_IMAGE1D_ARRAY_T 349
#define TOK_INLINE 350
#define TOK_INT 351
#define TOK_INT_LONG 352
#define TOK_LONG_LONG 353
#define TOK_INTN 354
#define TOK_INTPTR_T 355
#define TOK_KERNEL 356
#define TOK_LOCAL 357
#define TOK_LONG 358
#define TOK_LONGN 359
#define TOK_PRIVATE 360
#define TOK_PTRDIFF_T 361
#define TOK_READ_ONLY 362
#define TOK_READ_WRITE 363
#define TOK_REGISTER 364
#define TOK_RETURN 365
#define TOK_SAMPLER_T 366
#define TOK_SHORT 367
#define TOK_SHORTN 368
#define TOK_SIGNED 369
#define TOK_SIZEOF 370
#define TOK_SIZE_T 371
#define TOK_STATIC 372
#define TOK_STRUCT 373
#define TOK_SWITCH 374
#define TOK_TYPEDEF 375
#define TOK_TYPENAME 376
#define TOK_UCHARN 377
#define TOK_UCHAR 378
#define TOK_ULONG 379
#define TOK_USHORT 380
#define TOK_UINT 381
#define TOK_UINT_LONG 382
#define TOK_UINT_LONG_LONG 383
#define TOK_UINTN 384
#define TOK_ULONGN 385
#define TOK_UINTPTR_T 386
#define TOK_UNION 387
#define TOK_UNSIGNED 388
#define TOK_USHORTN 389
#define TOK_VOID 390
#define TOK_VOLATILE 391
#define TOK_WHILE 392
#define TOK_WRITE_ONLY 393

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 64 "parser.y" /* yacc.c:1909  */

	long int const_int_val;
	unsigned long long const_int_val_ull;
	double  const_float_val;
	char * identifier;
	struct cl2llvmTypeWrap *llvm_type_ref;
	struct cl2llvm_val_t *llvm_value_ref;
	struct list_t * init_list;
	struct list_t * list_val_t;
	struct cl2llvm_arg_t *arg_t;
	struct list_t *arg_list;
	LLVMBasicBlockRef basic_block_ref;
	struct cl2llvm_while_blocks_t *llvm_while_blocks;
	struct cl2llvm_for_blocks_t *llvm_for_blocks;
	struct cl2llvm_decl_list_t *decl_list;

#line 347 "parser.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE cl2llvm_yylval;

int cl2llvm_yyparse (void);

#endif /* !YY_CL2LLVM_YY_PARSER_H_INCLUDED  */
