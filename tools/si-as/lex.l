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

#include "id.h"
#include "main.h"
#include "parser.h"

extern long offset;
%}


%option noyywrap
%option nounput
%option noinput

%%

"," { 
	return TOK_COMMA;
}

"(" { 
	return TOK_OPAR;
}

")" { 
	return TOK_CPAR;
}

"[" { 
	return TOK_OBRA;
}

":" { 
	return TOK_COLON;
}

"]" {   
	return TOK_CBRA;
}
 
"&" {
	return TOK_AMP;
}

"\n" {
	return TOK_NEW_LINE;
}

s[0-9]+ {
	yylval.id = si_id_create(yytext);
	return TOK_SCALAR_REGISTER;
}

v[0-9]+ {
	yylval.id = si_id_create(yytext);
	return TOK_VECTOR_REGISTER;
}

vcc|scc {
	yylval.id = si_id_create(yytext);
	return TOK_SPECIAL_REGISTER;
}

"format" {
	return TOK_FORMAT;
}

[a-zA-Z_][a-zA-Z0-9_]* {   
	yylval.id = si_id_create(yytext);
	return TOK_ID; 
}

0x[0-9a-z]+ { 
	yylval.id = si_id_create(yytext);
	return TOK_HEX;	
}

[+-]?[0-9]+"."[0-9]+ {
	yylval.num_float = atof(yytext);
	return TOK_FLOAT;
}

[+-]?[0-9]+ {
	yylval.num = atoi(yytext);
	return TOK_DECIMAL;
}

"//"[^\n]* {
	/* Ignore comments */
}

[\t ] {
	/* Ignore tabs and spaces */
}

. {
	/* Lexical error */
	yyerror_fmt("unexpected character: %s\n", yytext);
}

%%

