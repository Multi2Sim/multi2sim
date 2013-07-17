%{

#include <math.h>

#include <lib/util/list.h>
#include <llvm-c/Core.h>

#include "format.h"
#include "val.h"
#include "type.h"
#include <llvm-c/Core.h>
#include "parser.h"
#include "cl2llvm.h"

extern char *cl2llvm_file_name;
extern struct list_t *cl2llvm_preprcr_file_list;

int cl2llvm_col_num = 0;

void cl2llvm_set_lineno(int lineno)
{
	yyset_lineno(lineno);
}

void cl2llvm_set_col_num(int leng)
{
	cl2llvm_col_num = leng;
}
int cl2llvm_get_col_num()
{
	return cl2llvm_col_num;
}

int long long unsigned htod(char *hex)
{
	unsigned long long decimal = 0;
	int multval = 0;
	int i = 0;
	int zero_count = 2;
	int length = strlen(hex);
	while(hex[i+2] == '0')
	{
		i++;
		zero_count++;
	}
	for(i = 0; i + zero_count < length; i++)
	{
		switch(hex[i+zero_count])
		{
			case'0':
				multval = 0;
				break;
			case'1':
				multval = 1;
				break;
			case'2':
				multval = 2;
				break;
			case'3':
				multval = 3;
				break;
			case'4':
				multval = 4;
				break;
			case'5':
				multval = 5;
				break;
			case'6':
				multval = 6;
				break;
			case'7':
				multval = 7;
				break;
			case'8':
				multval = 8;
				break;
			case'9':
				multval = 9;
				break;
			case'a':
			case'A':
				multval = 10;
				break;
			case'b':
			case'B':
				multval = 11;
				break;
			case'c':
			case'C':
				multval = 12;
				break;
			case'd':
			case'D':
				multval = 13;
				break;
			case'e':
			case'E':
				multval = 14;
				break;
			case'f':
			case'F':
				multval = 15;
				break;
			case'U':
			case'u':
			case'l':
			case'L':
				multval = 0;
				break;
			}
			int place_val = i;
			decimal = decimal + multval * pow(16, place_val);
		}
		return decimal;
	}

int long long unsigned otod(char *oct)
{
	unsigned long long decimal = 0;
	int multval = 0;
	int i = 0;
	int zero_count = 1;
	while(oct[i+1] == '0')
	{
		i++;
		zero_count++;
	}
	int length = strlen(oct);
	for(i = 0; i + zero_count < length; i++)
	{
		switch(oct[i+zero_count])
	{
		case'0':
			multval = 0;
			break;
		case'1':
			multval = 1;
			break;
		case'2':
			multval = 2;
			break;
		case'3':
			multval = 3;
			break;
		case'4':
			multval = 4;
			break;
		case'5':
			multval = 5;
			break;
		case'6':
			multval = 6;
			break;
		case'7':
			multval = 7;
			break;			case'U':
		case'u':
		case'l':
		case'L':
			multval = 0;
				break;
		}
		int place_val = i;
		decimal = decimal + multval * pow(8, place_val);			
	}
	return decimal;
}

double atod(char *ascii)
{
	char base[100];
	char expon[100];
	int expon_is_neg = 0;

/*	if (strlen(ascii) > 99)
	{
		yyerror("Constant is too large for floating point precision data type");
	}
*/
	int i = 0;
	while (ascii[i] != '\00' && ascii[i] != 'e' && ascii[i] != 'E' &&
		ascii[i] != 'f' && ascii[i] != 'F' && ascii[i] != 'l' &&
		ascii[i] != 'L' && ascii[i] != 'h' && ascii[i] != 'H')
	{
		base[i] = ascii[i];
		i++;
	}
	base[i] = '\00';
	i++;

	int j = 0;

	while (ascii[i] != '\00' && ascii[i] != 'f' && ascii[i] != 'F' && ascii[i] != 'l' && ascii[i] != 'L' && ascii[i] != 'h' && ascii[i] != 'H')
	{
		if(ascii[i] == '+')
			i++;
		if(ascii[i] == '-')
		{
			expon_is_neg = 1;
			i++;
		}
		expon[j] = ascii[i];
		i++;
		j++;
	}
	expon[i] = '\00';

	double base_d = atof(base);
	double expon_d = atof(expon);
	
	if (expon_is_neg)
		expon_d = expon_d * -1;
	
	double decimal = base_d * pow(10, expon_d);
	return decimal;
}

int extract_lineno(char *text)
{
	char buffer[100];
	int lineno;
	int i = 1;
	int j = 0;

	/*Copy Line number to buffer*/
	while(text[i] == 32)
	{
		i++;
	}
	while(text[i] != 32)
	{
		buffer[j] = text[i];
		j++;
		i++;
	}
	buffer[j] = '\00';
	
	/*Convert buffer to integer and return*/
	lineno = atoi(buffer);
	return lineno;
}



int extract_vector_size(char *text)
{
	int i = 2;
	char buffer[10];

	while  (text[i] != 49
		&& text[i] != 50
		&& text[i] != 51
		&& text[i] != 52
		&& text[i] != 56)
	{
		i++;
	}
	buffer[0] = text[i];
	i++;
	if (text[i] == 54)
	{
		buffer[1] = text[i];
		i++;
	}
	buffer[i] = '\00';
	return atoi(buffer);
}
	
%}

%option noyywrap
%option nounput
%option noinput



UL  (Ul)|(lU)|(uL)|(Lu)|(ul)|(UL)
LL  (ll)|(LL)|(lL)|(Ll)
CU  (U)|(u)
CL  (L)|(l)
ULL (llu)|(LLu)|(llU)|(LLU)|(ull)|(uLL)|(Ull)|(ULL)|(Llu)|(lLu)|(LlU)|(lLU)|(uLl)|(ulL)|(ULl)|(UlL)

SH			(short)
SP			[ \t\n]+
I			(int)
L			(long)
U			(unsigned)
S			(signed)

PREPROCESSOR		((\#)[ \t]+[0-9]+[ \t]+\"[^\"\`]*\"[ \t]*(\n))
EXTENSION		((\#pragma)[ \t]+(OPENCL)[ \t]+(EXTENSION)[ \t]+[^\n]*)
TOK_ID			[a-zA-Z_][a-zA-Z0-9_]*
CONST_HEX		0[xX][a-fA-F0-9]+
CONST_HEX_U		0[xX][a-fA-F0-9]+{CU}
CONST_HEX_L		0[xX][a-fA-F0-9]+{CL}
CONST_HEX_UL		0[xX][a-fA-F0-9]+{UL}
CONST_HEX_LL		0[xX][a-fA-F0-9]+{LL}
CONST_HEX_ULL		0[xX][a-fA-F0-9]+{ULL}
CONST_OCT		0[0-7]+
CONST_OCT_L		0[0-7]+{CL}
CONST_OCT_U		0[0-7]+{CU}
CONST_OCT_UL		0[0-7]+{UL}
CONST_OCT_LL		0[0-7]+{LL}
CONST_OCT_ULL		0[0-7]+{ULL}
CONST_INT		([1-9][0-9]*)|(0)
CONST_INT_U		([1-9][0-9]*{U}+)|(0{U})
CONST_INT_L		([1-9][0-9]*{L}+)|(0{L})
CONST_INT_UL		([1-9][0-9]*{UL}+)|(0{UL})
CONST_INT_LL		([1-9][0-9]*{UL}+)|(0{LL})
CONST_INT_ULL		([1-9][0-9]*{ULL}+)|(0{ULL})
CONST_CHAR		[a-zA-Z_]?'(\\.|[^\\'])+'
CONST_DEC_NO_POINT	[0-9]+[Ee][+-]?[0-9]+
CONST_DEC		(([0-9]*"."[0-9]+)|([0-9]+"."[0-9]*)|([0-9]+))([Ee][+-]?[0-9]+)?
CONST_DEC_H		(([0-9]*"."[0-9]+)|([0-9]+"."[0-9]*)|([0-9]+))([Ee][+-]?[0-9]+)?(h|H)
CONST_DEC_F		(([0-9]*"."[0-9]+)|([0-9]+"."[0-9]*)|([0-9]+))([Ee][+-]?[0-9]+)?(f|F)
CONST_DEC_L		(([0-9]*"."[0-9]+)|([0-9]+"."[0-9]*)|([0-9]+))([Ee][+-]?[0-9]+)?(l|L)
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
TOK_CLK_LOCAL_MEM_FENCE	(CLK_LOCAL_MEM_FENCE)
TOK_CLK_GLOBAL_MEM_FENCE (CLK_GLOBAL_MEM_FENCE)
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
TOK_SHORT		({SH})|({SH}{SP}{I})|({I}{SP}{SH})|({SH}{SP}{S})|({S}{SP}{SH})
TOK_SHORTN		(short)(2|3|4|8|16)
TOK_SIZEOF		(sizeof)
TOK_SIZE_T		(size_t)
TOK_STATIC		(static)
TOK_STRUCT		(struct)
TOK_SWITCH		(switch)
TOK_TYPEDEF		(typedef)
TOK_UCHAR		(uchar)|(unsigned[ \t\n]+char)|(unsigned{SP}char)
TOK_UCHARN		(uchar)(2|3|4|8|16)
TOK_UINT		(uint)|({I}{SP}{U})|({U}{SP}{I})|({U})
TOK_UINT_LONG		(ulong)|({U}{SP}{L})|({L}{SP}{U})|({U}{SP}{I}{SP}{L})|({I}{SP}{U}{SP}{L})|({I}{SP}{L}{SP}{U})|({U}{SP}{L}{SP}{I})|({L}{SP}{I}{SP}{U})|({L}{SP}{U}{SP}{I})
TOK_UINT_LONG_LONG ({U}{SP}{L}{SP}{L})|({L}{SP}{L}{SP}{U})|({L}{SP}{U}{SP}{L})|({L}{SP}{U}{SP}{I}{SP}{L})|({I}{SP}{U}{SP}{L}{SP}{L})|({L}{SP}{I}{SP}{L}{SP}{U})|({L}{SP}{U}{SP}{L}{SP}{I})|({L}{SP}{L}{SP}{I}{SP}{U})|({L}{SP}{L}{SP}{U}{SP}{I})|({L}{SP}{U}{SP}{I}{L})|({I}{SP}{L}{SP}{U}{L})|({I}{SP}{L}{SP}{L}{SP}{U})|({U}{SP}{L}{SP}{I}{SP}{L})|({U}{SP}{L}{SP}{L}{SP}{I})|({U}{SP}{I}{SP}{L}{SP}{L})
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

{PREPROCESSOR} {
	yyset_lineno(extract_lineno(yytext));
	cl2llvm_set_col_num(0);
	cl2llvm_file_name = extract_file_name(yytext);
	list_add(cl2llvm_preprcr_file_list, cl2llvm_file_name);
}
{EXTENSION} {
	yyset_lineno(yyget_lineno() + 1);
	cl2llvm_set_col_num(0);
	cl2llvm_warning("Extensions are not supported by m2c");
}
[\n] { 
	yyset_lineno(yyget_lineno() + 1);
	cl2llvm_set_col_num(0);
}


"/*"([^\*]|\*[^/])*"*/" {
}

"//"[^\n]*"\n" {
}

{TOK_STRING} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	return TOK_STRING;
}
{CONST_HEX} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = htod(yytext);
	return TOK_CONST_INT;
}
{CONST_HEX_U} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = htod(yytext);
	return TOK_CONST_INT_U;
}
{CONST_HEX_L} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = htod(yytext);
	return TOK_CONST_INT_L;
}
{CONST_HEX_LL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	return TOK_CONST_INT_LL;
}
{CONST_HEX_UL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = htod(yytext);
	return TOK_CONST_INT_UL;
}
{CONST_HEX_ULL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val_ull = htod(yytext);
	return TOK_CONST_INT_ULL;
}
{CONST_OCT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = otod(yytext);
	return TOK_CONST_INT;
}
{CONST_INT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = atoi(yytext);
	return TOK_CONST_INT;
}
{CONST_INT_U} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = atoi(yytext);
	return TOK_CONST_INT_U;
}
{CONST_INT_L} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = atoi(yytext);
	return TOK_CONST_INT_L;
}
{CONST_INT_UL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = atoi(yytext);
	return TOK_CONST_INT_UL;
}
{CONST_INT_LL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val_ull = atoi(yytext);
	return TOK_CONST_INT_LL;
}
{CONST_INT_ULL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val_ull = atoi(yytext);
	return TOK_CONST_INT_ULL;
}
{CONST_DEC_NO_POINT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_float_val = atod(yytext);
	return TOK_CONST_DEC;
}
{CONST_DEC} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_float_val = atod(yytext);
	return TOK_CONST_DEC;
}
{CONST_DEC_H} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_float_val = atod(yytext);
	return TOK_CONST_DEC_H;
}
{CONST_DEC_F} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_float_val = atod(yytext);
	return TOK_CONST_DEC_F;
}
{CONST_DEC_L} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_float_val = atod(yytext);
	return TOK_CONST_DEC_L;
}
{CONST_CHAR} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	return TOK_CONST_VAL;
}
{TOK_COMMA} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_COMMA;
}
{TOK_SEMICOLON} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_SEMICOLON;
}
{TOK_COLON} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_COLON;
}
{TOK_PAR_OPEN} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_PAR_OPEN;
}
{TOK_PAR_CLOSE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_PAR_CLOSE;
}
{TOK_BRACKET_OPEN} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_BRACKET_OPEN;
}
{TOK_BRACKET_CLOSE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_BRACKET_CLOSE;
}
{TOK_CURLY_BRACE_OPEN} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_CURLY_BRACE_OPEN;
}
{TOK_CURLY_BRACE_CLOSE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_CURLY_BRACE_CLOSE;
}

{TOK_PLUS} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_PLUS;
}

{TOK_MINUS} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_MINUS;
}
{TOK_MULT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_MULT;
}
{TOK_DIV} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_DIV;
}
{TOK_MOD} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_MOD;
}
{TOK_EQUAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_EQUAL;
}
{TOK_ELLIPSIS} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 3);
	return TOK_ELLIPSIS;
}
{TOK_ADD_EQUAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_ADD_EQUAL;
}
{TOK_MINUS_EQUAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_MINUS_EQUAL;
}
{TOK_MULT_EQUAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_MULT_EQUAL;
}
{TOK_DIV_EQUAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_DIV_EQUAL;
}
{TOK_MOD_EQUAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_MOD_EQUAL;
}
{TOK_AND_EQUAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_AND_EQUAL;
}
{TOK_OR_EQUAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_OR_EQUAL;
}
{TOK_EXCLUSIVE_EQUAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_EXCLUSIVE_EQUAL;
}
{TOK_SHIFT_RIGHT_EQUAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 3);
	return TOK_SHIFT_RIGHT_EQUAL;
}
{TOK_SHIFT_LEFT_EQUAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 3);
	return TOK_SHIFT_LEFT_EQUAL;
}
{TOK_GREATER} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_GREATER;
}
{TOK_LESS} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_LESS;
}
{TOK_INCREMENT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_INCREMENT;
}
{TOK_DECREMENT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_DECREMENT;
}
{TOK_SHIFT_LEFT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_SHIFT_LEFT;
}
{TOK_SHIFT_RIGHT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_SHIFT_RIGHT;
}
{TOK_CONDITIONAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_CONDITIONAL;
}
{TOK_BITWISE_NOT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_BITWISE_NOT;
}
{TOK_BITWISE_EXCLUSIVE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_BITWISE_EXCLUSIVE;
}
{TOK_BITWISE_OR} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_BITWISE_OR;
}
{TOK_BITWISE_AND} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_BITWISE_AND;
}
{TOK_LOGICAL_AND} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_LOGICAL_AND;
}
{TOK_LOGICAL_OR} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_LOGICAL_OR;
}
{TOK_LOGICAL_NEGATE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_LOGICAL_NEGATE;
}

{TOK_GREATER_EQUAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_GREATER_EQUAL;
}
{TOK_LESS_EQUAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_LESS_EQUAL;
}
{TOK_EQUALITY} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_EQUALITY;
}
{TOK_INEQUALITY} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_INEQUALITY;
}
{TOK_STRUCT_REF} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	return TOK_STRUCT_REF;
}
{TOK_STRUCT_DEREF} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_STRUCT_DEREF;
}
{TOK_AUTO} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 4);
	return TOK_AUTO;
}
{TOK_BOOL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 4);
	return TOK_BOOL;
}
{TOK_BREAK} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 5);
	return TOK_BREAK;
}
{TOK_CASE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 4);
	return TOK_CASE;
}
{TOK_CHAR} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 4);
	return TOK_CHAR;
}
{TOK_CHARN} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = extract_vector_size(yytext);
	return TOK_CHARN;
}
{TOK_CLK_LOCAL_MEM_FENCE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val_ull = 1;
	return TOK_CONST_INT;
}
{TOK_CLK_GLOBAL_MEM_FENCE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val_ull = 2;
	return TOK_CONST_INT;
}
{TOK_CONSTANT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 8);
	return TOK_CONSTANT;
}
{TOK_CONST} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 5);
	return TOK_CONST;
}
{TOK_CONTINUE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 8);
	return TOK_CONTINUE;
}
{TOK_DEFAULT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 7);
	return TOK_DEFAULT;
}
{TOK_DO} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_DO;
}
{TOK_DOUBLE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 6);
	return TOK_DOUBLE;
}
{TOK_DOUBLE_LONG} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	return TOK_DOUBLE_LONG;
}
{TOK_DOUBLEN} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = extract_vector_size(yytext);
	return TOK_DOUBLEN;
}
{TOK_ELSE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 4);
	return TOK_ELSE;
}
{TOK_ENUM} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 4);
	return TOK_ENUM;
}
{TOK_EVENT_T} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 7);
	return TOK_EVENT_T;
}
{TOK_EXTERN} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 6);
	return TOK_EXTERN;
}
{TOK_FLOAT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 5);
	return TOK_FLOAT;
}
{TOK_FLOATN} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = extract_vector_size(yytext);
	return TOK_FLOATN;
}
{TOK_FOR} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 3);
	return TOK_FOR;
}
{TOK_GLOBAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	return TOK_GLOBAL;
}
{TOK_GOTO} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 4);
	return TOK_GOTO;
}
{TOK_HALF} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 4);
	return TOK_HALF;
}
{TOK_IF} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 2);
	return TOK_IF;
}
{TOK_IMAGE2D_T} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 9);
	return TOK_IMAGE2D_T;
}
{TOK_IMAGE3D_T} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 9);
	return TOK_IMAGE3D_T;
}
{TOK_IMAGE2D_ARRAY_T} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 15);
	return TOK_IMAGE2D_ARRAY_T;
}
{TOK_IMAGE1D_T} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 9);
	return TOK_IMAGE1D_T;
}
{TOK_IMAGE1D_BUFFER_T} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 16);
	return TOK_IMAGE1D_BUFFER_T;
}
{TOK_IMAGE1D_ARRAY_T} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 15);
	return TOK_IMAGE1D_ARRAY_T;
}
{TOK_INLINE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 6);
	return TOK_INLINE;
}
{TOK_INT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 3);
	return TOK_INT;
}
{TOK_INTN} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = extract_vector_size(yytext);
	return TOK_INTN;
}
{TOK_INTPTR_T} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 8);
	return TOK_INTPTR_T;
}
{TOK_KERNEL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	return TOK_KERNEL;
}
{TOK_LOCAL} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 5);
	return TOK_LOCAL;
}
{TOK_INT_LONG} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 4);
	return TOK_LONG;
}
{TOK_INT_LONG_LONG} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	return TOK_LONG_LONG;
}
{TOK_LONGN} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = extract_vector_size(yytext);
	return TOK_LONGN;
}
{TOK_PRIVATE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 7);
	return TOK_PRIVATE;
}
{TOK_PTRDIFF_T} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 9);
	return TOK_PTRDIFF_T;
}
{TOK_REGISTER} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 8);
	return TOK_REGISTER;
}
{TOK_RETURN} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 6);
	return TOK_RETURN;
}
{TOK_SAMPLER_T} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 9);
	return TOK_SAMPLER_T;
}
{TOK_SHORT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 5);
	return TOK_SHORT;
}
{TOK_SHORTN} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = extract_vector_size(yytext);
	return TOK_SHORTN;
}
{TOK_SIZEOF} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 6);
	return TOK_SIZEOF;
}
{TOK_SIZE_T} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 6);
	return TOK_SIZE_T;
}
{TOK_STATIC} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 6);
	return TOK_STATIC;
}
{TOK_STRUCT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 6);
	return TOK_STRUCT;
}
{TOK_SWITCH} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 6);
	return TOK_SWITCH;
}
{TOK_TYPEDEF} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 7);
	return TOK_TYPEDEF;
}
{TOK_TYPENAME} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 4);
	return TOK_TYPENAME;
}
{TOK_UCHAR} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 5);
	return TOK_UCHAR;
}
{TOK_UCHARN} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = extract_vector_size(yytext);
	return TOK_UCHARN;
}
{TOK_UINT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 4);
	return TOK_UINT;
}
{TOK_UINT_LONG} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	return TOK_UINT_LONG;
}
{TOK_UINT_LONG_LONG} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	return TOK_UINT_LONG_LONG;
}
{TOK_UINTN} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = extract_vector_size(yytext);
	return TOK_UINTN;
}
{TOK_UINTPTR_T} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 9);
	return TOK_UINTPTR_T;
}
{TOK_ULONGN} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = extract_vector_size(yytext);
	return TOK_ULONGN;
}
{TOK_UNION} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 5);
	return TOK_UNION;
}
{TOK_USHORT} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 6);
	return TOK_USHORT;
}
{TOK_USHORTN} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.const_int_val = extract_vector_size(yytext);
	return TOK_USHORTN;
}
{TOK_VOID} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 4);
	return TOK_VOID;
}
{TOK_READ_ONLY} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 9);
	return TOK_READ_ONLY;
}
{TOK_WRITE_ONLY} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 10);
	return TOK_WRITE_ONLY;
}
{TOK_READ_WRITE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 10);
	return TOK_READ_WRITE;
}
{TOK_VOLATILE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 8);
	return TOK_VOLATILE;
}
{TOK_WHILE} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 5);
	return TOK_WHILE;
}


{TOK_ID} {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + yyget_leng());
	cl2llvm_yylval.identifier = strdup(yytext);
	return TOK_ID;
}
[ ] {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
}
[\t] {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 8);
}


. {
	cl2llvm_set_col_num(cl2llvm_get_col_num() + 1);
	cl2llvm_yyerror_fmt("unexpected symbol '%s'", yytext);
}

%%

