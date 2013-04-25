%{

#include <llvm-c/Core.h>

#include "cl2llvm.h"
#include "parser.h"

int col_num = 0;
void set_col_num(int leng)
{
	col_num = leng;
}
int get_col_num()
{
	return col_num;
}
%}

%option noyywrap
%option nounput
%option noinput



UL  ((Ul)|(lU)|(uL)|(Lu)|(ul)|(UL)|(ll)|(LL)|U|L|l|u|(llu)|(LLu)|(llU)|(LLU)|(ull)|(uLL)|(Ull)|(ULL))

SH			(short)
SP			[ \t\n]+
I			(int)
L			(long)
U			(unsigned)
S			(signed)


TOK_ID			[a-zA-Z_][a-zA-Z0-9_]*
CONST_0			0{UL}?
CONST_HEX		0[xX][a-fA-F0-9]+{UL}?
CONST_OCT		0[0-7]+{UL}?
CONST_INT		[1-9][0-9]*{UL}?
CONST_CHAR		[a-zA-Z_]?'(\\.|[^\\'])+'
CONST_INT_EXPON		[0-9]+[Ee][+-]?[0-9]+(f|F|l|L)?
CONST_DEC		(([0-9]*"."[0-9]+)|([0-9]+"."[0-9]*))([Ee][+-]?[0-9]+)?(f|F|l|L)?
TOK_STRING		\"[^\"\`]*\"
TOK_COMMA		\,
TOK_SEMICOLON		\;
TOK_COLON		\:
TOK_PAR_OPEN		\(
TOK_PAR_CLOSE		\)
TOK_BRACKET_OPEN	\[
TOK_BRACKET_CLOSE	\]
TOK_CURLY_BRACE_OPEN	\{
TOK_CURLY_BRACE_CLOSE	\}
TOK_PLUS		\+
TOK_MINUS		\-
TOK_MULT		\*
TOK_DIV			\/
TOK_MOD			\%
TOK_EQUAL		\=
TOK_ELLIPSIS		(\.\.\.)
TOK_ADD_EQUAL		(\+\=)
TOK_MINUS_EQUAL		(\-\=)
TOK_MULT_EQUAL		(\*\=)
TOK_DIV_EQUAL		(\/\=)
TOK_MOD_EQUAL		(\%\=)
TOK_AND_EQUAL		(\&\=)
TOK_OR_EQUAL		(\|\=)
TOK_EXCLUSIVE_EQUAL	(\^\=)
TOK_SHIFT_RIGHT_EQUAL	(\>\>\=)
TOK_SHIFT_LEFT_EQUAL	(\<\<\=)
TOK_GREATER		\>
TOK_LESS		\<
TOK_INCREMENT		(\+\+)
TOK_DECREMENT		(\-\-)
TOK_SHIFT_LEFT		(\<\<)
TOK_SHIFT_RIGHT		(\>\>)
TOK_BITWISE_NOT		\~
TOK_BITWISE_EXCLUSIVE	\^
TOK_BITWISE_OR		\|
TOK_BITWISE_AND		\&
TOK_CONDITIONAL		\?
TOK_LOGICAL_AND		(\&\&)
TOK_LOGICAL_OR		(\|\|)
TOK_LOGICAL_NEGATE	\!
TOK_GREATER_EQUAL	(\>\=)
TOK_LESS_EQUAL		(\<\=)
TOK_EQUALITY		(\=\=)
TOK_INEQUALITY		(\!\=)
TOK_STRUCT_REF		\.
TOK_STRUCT_DEREF	(\-\>)
TOK_AUTO		(auto)
TOK_BOOL		(bool)
TOK_BREAK		(break)
TOK_CASE		(case)
TOK_CHAR		(char)|(signed[ \t\n]+char)|(char[ \t\n]+signed)
TOK_CHARN		(char)(2|3|4|8|16)
TOK_CONSTANT		(__constant)|(constant)
TOK_CONST		(const)
TOK_CONTINUE		(continue)
TOK_DEFAULT		(default)
TOK_DO			(do)
TOK_DOUBLE		(double)
TOK_DOUBLE_LONG		(double{SP}long)|(long{SP}double)
TOK_DOUBLEN		(double)(2|3|4|8|16)
TOK_ELSE		(else)
TOK_ENUM		(enum)
TOK_EVENT_T		(event_t)
TOK_EXTERN		(extern)
TOK_FLOAT		(float)
TOK_FLOATN		(float)(2|3|4|8|16)
TOK_FOR			(for)
TOK_GLOBAL		(__global)|(global)
TOK_GOTO		(goto)
TOK_HALF		(half)
TOK_IF			(if)
TOK_IMAGE2D_T		(mage2d_t)
TOK_IMAGE3D_T		(image3d_t)
TOK_IMAGE2D_ARRAY_T	(image2d_array_t)
TOK_IMAGE1D_T		(image1d_t)
TOK_IMAGE1D_BUFFER_T	(image1d_buffer_t)
TOK_IMAGE1D_ARRAY_T	(image1d_array_t)
TOK_INLINE		(inline)
TOK_INT			(int)|(int[ \t\n]+signed)|(signed[ \t\n]+int)|(signed)
TOK_INTN		(int)(2|3|4|8|16)
TOK_INTPTR_T		(intptr_t)
TOK_KERNEL		(__kernel)|(kernel)
TOK_LOCAL		(__local)|(local)
TOK_INT_LONG_LONG	({L}{SP}{L})|({L}{SP}{L}{SP}{I})|({L}{SP}{I}{SP}{L})|({I}{SP}{L}{SP}{L})
TOK_INT_LONG		({L}{SP}{I})|({L})|({I}{SP}{L})
TOK_LONGN		(long)(2|3|4|8|16)
TOK_PRIVATE		(__private)|(private)
TOK_PTRDIFF_T		(ptrdiff_t)
TOK_REGISTER		(register)
TOK_RETURN		(return)
TOK_SAMPLER_T		(sampler_t)
TOK_SHORT		(short)|(short{SP}int)|(int{SP}short)
TOK_SHORTN		(short)(2|3|4|8|16)
TOK_SIZEOF		(sizeof)
TOK_SIZE_T		(size_t)
TOK_STATIC		(static)
TOK_STRUCT		(struct)
TOK_SWITCH		(switch)
TOK_TYPEDEF		(typedef)
TOK_UCHAR		(uchar)|(unsigned[ \t\n]+char)|(unsigned{SP}char)
TOK_UCHARN		(uchar)(2|3|4|8|16)
TOK_UINT		(uint)|({I}{SP}{U})|({U}{SP}{I})
TOK_UINT_LONG		({U}{SP]{L})|({L}{SP}{U})|{U}{SP}{I}{SP}{L})|({I}{SP}{U}{SP}{L})|({I}{SP}{L}{SP}{U})|({U}{SP}{L}{SP}{I})|({L}{SP}{I}{SP}{U})|({L}{SP}{U}{SP}{I})
TOK_UINT_LONG_LONG	({U}{SP]{L}{SP}{L})|({L}{SP}{L}{SP}{U})|({L}{SP}{U}{SP}{L})|{L}{SP}{U}{SP}{I}{SP}{L})|({I}{SP}{U}{SP}{L}{SP}{L})|({L}{SP}{I}{SP}{L}{SP}{U})|({L}{SP}{U}{SP}{L}{SP}{I})|({L}{SP}{L}{SP}{I}{SP}{U})|({L}{SP}{L}{SP}{U}{SP}{I})|({L}{SP}{U}{SP}{I}{L})|({I}{SP}{L}{SP}{U}{L})|({I}{SP}{L}{SP}{L}{SP}{U})|({U]{SP}{L}{SP}{I}{SP}{L})|({U}{SP}{L]{SP}{L}{SP}{I})|({U}{SP}{I}{SP}{L}{SP}{L})
TOK_UINTN		(uint)(2|3|4|8|16)
TOK_UINTPTR_T		(uintptr_t)
TOK_ULONG		(ulong)
TOK_ULONGN		(ulong)(2|3|4|8|16)
TOK_UNION		(union)
TOK_USHORT		(ushort)|({SH}{SP}{U})|({U}{SP}{SH})|({I}{SP}{U}{SP}{SH})|({I}{SP}{SH}{U})|({SH}{SP}{U}{SP}{I})|({SH}{SP}{I}{SP}{U})|({U}{SP}{I}{SP}{SH})|({U}{SP}{SH}{SP}{I})
TOK_USHORTN		(ushort)(2|3|4|8|16)
TOK_VOID		(void)
TOK_READ_ONLY		(__read_only)|(read_only)
TOK_WRITE_ONLY		(__write_only)|(write_only)
TOK_READ_WRITE		(__read_write)|(read_write)
TOK_VOLATILE		(volatile)
TOK_WHILE		(while)

TOK_TYPENAME		({TOK_TYPEDEF}[ ]+{TOK_INT})

%%

[\n] { 
	yyset_lineno(yyget_lineno() + 1);
	set_col_num(0);
}


"/*"([^\*]|\*[^/])*"*/" {
}

"//"[^\n]*"\n" {
}

{TOK_STRING} {
	printf("STRING - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return TOK_STRING;
}

{CONST_0} {
	printf("TOK_CONST_VAL - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return 
	TOK_CONST_VAL;
}
{CONST_HEX} {
	set_col_num(get_col_num() + yyget_leng());
	printf("TOK_CONST_VAL - %s\n", yytext);
	return TOK_CONST_VAL;
}
{CONST_OCT} {
	set_col_num(get_col_num() + yyget_leng());
	printf("TOK_CONST_VAL - %s\n", yytext);
	return TOK_CONST_VAL;
}
{CONST_INT} {
	set_col_num(get_col_num() + yyget_leng());
	printf("TOK_CONST_VAL - %s\n", yytext);
	yylval.const_int_val = atoi(yytext);
	return TOK_CONST_INT;
}
{CONST_CHAR} {
	set_col_num(get_col_num() + yyget_leng());
	printf("TOK_CONST_VAL - %s\n", yytext);
	return TOK_CONST_VAL;
}
{CONST_INT_EXPON} {
	printf("TOK_CONST_VAL - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return TOK_CONST_VAL;
}
{CONST_DEC} {
	printf("TOK_CONST_VAL - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	yylval.const_float_val = atof(yytext);
	return TOK_CONST_DEC;
}
{TOK_COMMA} {
	printf("COMMA - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_COMMA;
}
{TOK_SEMICOLON} {
	printf("TOK_SEMICOLON - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_SEMICOLON;
}
{TOK_COLON} {
	printf("TOK_COLON - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_COLON;
}
{TOK_PAR_OPEN} {
	printf("TOK_PAR_OPEN - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_PAR_OPEN;
}
{TOK_PAR_CLOSE} {
	printf("TOK_PAR_CLOSE - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_PAR_CLOSE;
}
{TOK_BRACKET_OPEN} {
	printf("BRACKET_OPEN %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_BRACKET_OPEN;
}
{TOK_BRACKET_CLOSE} {
	printf("TOK_BRACKET_CLOSE %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_BRACKET_CLOSE;
}
{TOK_CURLY_BRACE_OPEN} {
	printf("TOK_CURLY_BRACE_OPEN - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_CURLY_BRACE_OPEN;
}
{TOK_CURLY_BRACE_CLOSE} {
	printf("TOK_CURLY_BRACE_CLOSE - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_CURLY_BRACE_CLOSE;
}

{TOK_PLUS} {
	printf("TOK_PLUS - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_PLUS;
}

{TOK_MINUS} {
	printf("TOK_MINUS - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_MINUS;
}
{TOK_MULT} {
	printf("TOK_MULT - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_MULT;
}
{TOK_DIV} {
	printf("TOK_DIV - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_DIV;
}
{TOK_MOD} {
	printf("TOK_MOD - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_MOD;
}
{TOK_EQUAL} {
	printf("TOK_EQUAL - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_EQUAL;
}
{TOK_ELLIPSIS} {
	printf("TOK_ELLIPSIS - %s\n", yytext);
	set_col_num(get_col_num() + 3);
	return TOK_ELLIPSIS;
}
{TOK_ADD_EQUAL} {
	printf("TOK_ADD_EQUAL - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_ADD_EQUAL;
}
{TOK_MINUS_EQUAL} {
	printf("TOK_MINUS_EQUAL - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_MINUS_EQUAL;
}
{TOK_MULT_EQUAL} {
	printf("TOK_MULT_EQUAL - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_MULT_EQUAL;
}
{TOK_DIV_EQUAL} {
	printf("TOK_DIV_EQUAL - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_DIV_EQUAL;
}
{TOK_MOD_EQUAL} {
	printf("TOK_MOD_EQUAL - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_MOD_EQUAL;
}
{TOK_AND_EQUAL} {
	printf("TOK_AND_EQUAL - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_AND_EQUAL;
}
{TOK_OR_EQUAL} {
	printf("TOK_OR_EQUAL - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_OR_EQUAL;
}
{TOK_EXCLUSIVE_EQUAL} {
	printf("TOK_EXCLUSIVE_EQUAL - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_EXCLUSIVE_EQUAL;
}
{TOK_SHIFT_RIGHT_EQUAL} {
	printf("TOK_SHIFT_LEFT_EQUAL - %s\n", yytext);
	set_col_num(get_col_num() + 3);
	return TOK_SHIFT_RIGHT_EQUAL;
}
{TOK_SHIFT_LEFT_EQUAL} {
	printf("TOK_SHIFT_LEFT_EQUAL - %s\n", yytext);
	set_col_num(get_col_num() + 3);
	return TOK_SHIFT_LEFT_EQUAL;
}
{TOK_GREATER} {
	printf("TOK_GREATER - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_GREATER;
}
{TOK_LESS} {
	printf("TOK_LESS - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_LESS;
}
{TOK_INCREMENT} {
	printf("TOK_INCREMENT - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_INCREMENT;
}
{TOK_DECREMENT} {
	printf("TOK_DECREMENT - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_DECREMENT;
}
{TOK_SHIFT_LEFT} {
	printf("TOK_SHIFT_LEFT - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_SHIFT_LEFT;
}
{TOK_SHIFT_RIGHT} {
	printf("TOK_SHIFT_RIGHT - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_SHIFT_RIGHT;
}
{TOK_CONDITIONAL} {
	printf("TOK_CONDTIONAL - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_CONDITIONAL;
}
{TOK_BITWISE_NOT} {
	printf("TOK_BITWISE_NOT - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_BITWISE_NOT;
}
{TOK_BITWISE_EXCLUSIVE} {
	printf("TOK_BITWISE_EXCLUSIVE - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_BITWISE_EXCLUSIVE;
}
{TOK_BITWISE_OR} {
	printf("TOK_BITWISE_OR - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_BITWISE_OR;
}
{TOK_BITWISE_AND} {
	printf("TOK_BITWISE_AND - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_BITWISE_AND;
}
{TOK_LOGICAL_AND} {
	printf("TOK_LOGICAL_AND -  %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_LOGICAL_AND;
}
{TOK_LOGICAL_OR} {
	printf("TOK_LOGICAL_OR - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_LOGICAL_OR;
}
{TOK_LOGICAL_NEGATE} {
	printf("TOK_LOGICAL_NEGATE - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_LOGICAL_NEGATE;
}

{TOK_GREATER_EQUAL} {
	printf("TOK_GREATER_EQUAL - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_GREATER_EQUAL;
}
{TOK_LESS_EQUAL} {
	printf("TOK_LESS_EQUAL - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_LESS_EQUAL;
}
{TOK_EQUALITY} {
	printf("TOK_EQUALITY - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_EQUALITY;
}
{TOK_INEQUALITY} {
	printf("TOK_INEQUALITY - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_INEQUALITY;
}
{TOK_STRUCT_REF} {
	printf("TOK_STRUCT_REF - %s\n", yytext);
	set_col_num(get_col_num() + 1);
	return TOK_STRUCT_REF;
}
{TOK_STRUCT_DEREF} {
	printf("TOK_STRUCT_DEREF - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_STRUCT_DEREF;
}
{TOK_AUTO} {
	printf("TOK_AUTO - %s\n", yytext);
	set_col_num(get_col_num() + 4);
	return TOK_AUTO;
}
{TOK_BOOL} {
	printf("TOK_BOOL - %s\n", yytext);
	set_col_num(get_col_num() + 4);
	return TOK_BOOL;
}
{TOK_BREAK} {
	printf("TOK_BREAK - %s\n", yytext);
	set_col_num(get_col_num() + 5);
	return TOK_BREAK;
}
{TOK_CASE} {
	printf("TOK_CASE - %s\n", yytext);
	set_col_num(get_col_num() + 4);
	return TOK_CASE;
}
{TOK_CHAR} {
	printf("TOK_CHAR - %s\n", yytext);
	set_col_num(get_col_num() + 4);
	return TOK_CHAR;
}
{TOK_CHARN} {
	printf("TOK_CHARN - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return TOK_CHARN;
}
{TOK_CONSTANT} {
	printf("TOK_CONSTANT - %s\n", yytext);
	set_col_num(get_col_num() + 8);
	return TOK_CONSTANT;
}
{TOK_CONST} {
	printf("TOK_CONST - %s\n", yytext);
	set_col_num(get_col_num() + 5);
	return TOK_CONST;
}
{TOK_CONTINUE} {
	printf("TOK_CONTINUE - %s\n",yytext);
	set_col_num(get_col_num() + 8);
	return TOK_CONTINUE;
}
{TOK_DEFAULT} {
	printf("TOK_DEFAULT - %s\n", yytext);
	set_col_num(get_col_num() + 7);
	return TOK_DEFAULT;
}
{TOK_DO} {
	printf("TOK_DO - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_DO;
}
{TOK_DOUBLE} {
	printf("TOK_DOUBLE - %s\n", yytext);
	set_col_num(get_col_num() + 6);
	return TOK_DOUBLE;
}
{TOK_DOUBLE_LONG} {
	printf("TOK_DOUBLE - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return TOK_DOUBLE_LONG;
}
{TOK_DOUBLEN} {
	printf("TOK_CHARN - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return TOK_DOUBLEN;
}
{TOK_ELSE} {
	printf("TOK_ESLE - %s\n", yytext);
	set_col_num(get_col_num() + 4);
	return TOK_ELSE;
}
{TOK_ENUM} {
	printf("TOK_ENUM - %s\n", yytext);
	set_col_num(get_col_num() + 4);
	return TOK_ENUM;
}
{TOK_EVENT_T} {
	printf("TOK_EVENT_T - %s\n", yytext);
	set_col_num(get_col_num() + 7);
	return TOK_EVENT_T;
}
{TOK_EXTERN} {
	printf("TOK_EXTERN - %s\n", yytext);
	set_col_num(get_col_num() + 6);
	return TOK_EXTERN;
}
{TOK_FLOAT} {
	printf("TOK_FLOAT - %s\n", yytext);
	set_col_num(get_col_num() + 5);
	return TOK_FLOAT;
}
{TOK_FLOATN} {
	printf("TOK_FLOATN - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return TOK_FLOATN;
}
{TOK_FOR} {
	printf("TOK_FOR - %s\n", yytext);
	set_col_num(get_col_num() + 3);
	return TOK_FOR;
}
{TOK_GLOBAL} {
	printf("TOK_GLOBAL - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return TOK_GLOBAL;
}
{TOK_GOTO} {
	printf("TOK_GOTO - %s\n", yytext);
	set_col_num(get_col_num() + 4);
	return TOK_GOTO;
}
{TOK_HALF} {
	printf("TOK_HALF - %s\n", yytext);
	set_col_num(get_col_num() + 4);
	return TOK_HALF;
}
{TOK_IF} {
	printf("TOK_IF - %s\n", yytext);
	set_col_num(get_col_num() + 2);
	return TOK_IF;
}
{TOK_IMAGE2D_T} {
	printf("TOK_IMAGE2D_T - %s\n", yytext);
	set_col_num(get_col_num() + 9);
	return TOK_IMAGE2D_T;
}
{TOK_IMAGE3D_T} {
	printf("TOK_IMAGE3D_T - %s\n", yytext);
	set_col_num(get_col_num() + 9);
	return TOK_IMAGE3D_T;
}
{TOK_IMAGE2D_ARRAY_T} {
	printf("TOK_IMAGE2D_ARRAY_T - %s\n", yytext);
	set_col_num(get_col_num() + 15);
	return TOK_IMAGE2D_ARRAY_T;
}
{TOK_IMAGE1D_T} {
	printf("TOK_IMAGE1D_T - %s\n", yytext);
	set_col_num(get_col_num() + 9);
	return TOK_IMAGE1D_T;
}
{TOK_IMAGE1D_BUFFER_T} {
	printf("TOK_IMAGE1D_BUFFER_T - %s\n", yytext);
	set_col_num(get_col_num() + 16);
	return TOK_IMAGE1D_BUFFER_T;
}
{TOK_IMAGE1D_ARRAY_T} {
	printf("TOK_IMAGE1D_ARRAY_T - %s\n", yytext);
	set_col_num(get_col_num() + 15);
	return TOK_IMAGE1D_ARRAY_T;
}
{TOK_INLINE} {
	printf("TOK_INLINE - %s\n", yytext);
	set_col_num(get_col_num() + 6);
	return TOK_INLINE;
}
{TOK_INT} {
	printf("TOK_INT - %s\n", yytext);
	set_col_num(get_col_num() + 3);
	return TOK_INT;
}
{TOK_INTN} {
	printf("TOK_INTN - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return TOK_INTN;
}
{TOK_INTPTR_T} {
	printf("TOK_INTPTR_T - %s\n", yytext);
	set_col_num(get_col_num() + 8);
	return TOK_INTPTR_T;
}
{TOK_KERNEL} {
	printf("TOK_KERNEL - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return TOK_KERNEL;
}
{TOK_LOCAL} {
	printf("TOK_LOCAL - %s\n", yytext);
	set_col_num(get_col_num() + 5);
	return TOK_LOCAL;
}
{TOK_INT_LONG} {
	printf("TOK_LONG - %s\n", yytext);
	set_col_num(get_col_num() + 4);
	return TOK_LONG;
}
{TOK_LONGN} {
	printf("TOK_LONGN - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return TOK_LONGN;
}
{TOK_PRIVATE} {
	printf("TOK_PRIVATE - %s\n", yytext);
	set_col_num(get_col_num() + 7);
	return TOK_PRIVATE;
}
{TOK_PTRDIFF_T} {
	printf("TOK_PTRDIFF_T - %s\n", yytext);
	set_col_num(get_col_num() + 9);
	return TOK_PTRDIFF_T;
}
{TOK_REGISTER} {
	printf("TOK_REGISTER - %s\n", yytext);
	set_col_num(get_col_num() + 8);
	return TOK_REGISTER;
}
{TOK_RETURN} {
	printf("TOK_RETURN - %s\n", yytext);
	set_col_num(get_col_num() + 6);
	return TOK_RETURN;
}
{TOK_SAMPLER_T} {
	printf("TOK_SAMPLER_T - %s\n", yytext);
	set_col_num(get_col_num() + 9);
	return TOK_SAMPLER_T;
}
{TOK_SHORT} {
	printf("TOK_SHORT - %s\n", yytext);
	set_col_num(get_col_num() + 5);
	return TOK_SHORT;
}
{TOK_SHORTN} {
	printf("TOK_SHORTN - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return TOK_SHORTN;
}
{TOK_SIZEOF} {
	printf("TOK_SIZEOF - %s\n", yytext);
	set_col_num(get_col_num() + 6);
	return TOK_SIZEOF;
}
{TOK_SIZE_T} {
	printf("TOK_SIZE_T - %s\n", yytext);
	set_col_num(get_col_num() + 6);
	return TOK_SIZE_T;
}
{TOK_STATIC} {
	printf("TOK_STATIC - %s\n", yytext);
	set_col_num(get_col_num() + 6);
	return TOK_STATIC;
}
{TOK_STRUCT} {
	printf("TOK_STRUCT - %s\n", yytext);
	set_col_num(get_col_num() + 6);
	return TOK_STRUCT;
}
{TOK_SWITCH} {
	printf("TOK_SWITCH - %s\n", yytext);
	set_col_num(get_col_num() + 6);
	return TOK_SWITCH;
}
{TOK_TYPEDEF} {
	printf("TOK_TYPEDEF - %s\n", yytext);
	set_col_num(get_col_num() + 7);
	return TOK_TYPEDEF;
}
{TOK_TYPENAME} {
	printf("TOK_TYPENAME - %s\n", yytext);
	set_col_num(get_col_num() + 4);
	return TOK_TYPENAME;
}
{TOK_UCHAR} {
	printf("TOK_UNSIGNED TOK_CHAR - %s\n", yytext);
	set_col_num(get_col_num() + 5);
	return TOK_UCHAR;
}
{TOK_UCHARN} {
	printf("TOK_UCHARN - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return TOK_UCHARN;
}
{TOK_UINT} {
	printf("TOK_UNSIGNED TOK_INT - %s\n", yytext);
	set_col_num(get_col_num() + 4);
	return TOK_UINT;
}
{TOK_UINTN} {
	printf("TOK_UINTN - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return TOK_UINTN;
}
{TOK_UINTPTR_T} {
	printf("TOK_UINTPTR_T - %s\n", yytext);
	set_col_num(get_col_num() + 9);
	return TOK_UINTPTR_T;
}
{TOK_ULONG} {
	printf("TOK_UNSIGNED TOK_LONG - %s\n", yytext);
	set_col_num(get_col_num() + 5);
	return TOK_ULONG;
}
{TOK_ULONGN} {
	printf("TOK_ULONGN - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return TOK_ULONGN;
}
{TOK_UNION} {
	printf("TOK_UNION - %s\n", yytext);
	set_col_num(get_col_num() + 5);
	return TOK_UNION;
}
{TOK_USHORT} {
	printf("TOK_UNSIGNED TOK_SHORT - %s\n", yytext);
	set_col_num(get_col_num() + 6);
	return TOK_USHORT;
}
{TOK_USHORTN} {
	printf("TOK_USHORTN - %s\n", yytext);
	set_col_num(get_col_num() + yyget_leng());
	return TOK_USHORTN;
}
{TOK_VOID} {
	printf("TOK_VOID - %s\n", yytext);
	set_col_num(get_col_num() + 4);
	return TOK_VOID;
}
{TOK_READ_ONLY} {
	printf("TOK_READ_ONLY - %s\n", yytext);
	set_col_num(get_col_num() + 9);
	return TOK_READ_ONLY;
}
{TOK_WRITE_ONLY} {
	printf("TOK_WRITE_ONLY - %s\n", yytext);
	set_col_num(get_col_num() + 10);
	return TOK_WRITE_ONLY;
}
{TOK_READ_WRITE} {
	printf("TOK_READ_WRITE - %s\n", yytext);
	set_col_num(get_col_num() + 10);
	return TOK_READ_WRITE;
}
{TOK_VOLATILE} {
	printf("TOK_VOLATILE - %s\n", yytext);
	set_col_num(get_col_num() + 8);
	return TOK_VOLATILE;
}
{TOK_WHILE} {
	printf("TOK_WHILE - %s\n", yytext);
	set_col_num(get_col_num() + 5);
	return TOK_WHILE;
}


{TOK_ID} {
	set_col_num(get_col_num() + yyget_leng());
	printf("TOK_ID - %s\n", yytext);
	yylval.identifier = strdup(yytext);
	return TOK_ID;
}
[ ] {
	set_col_num(get_col_num() + 1);
}
[\t] {
	set_col_num(get_col_num() + 8);
}


. {
	set_col_num(get_col_num() + 1);
	fprintf(stderr, "error: unexpected symbol: %s\n", yytext);
	exit(1);
}

%%

