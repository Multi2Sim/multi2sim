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
#include "si2bin.h"
#include "parser.h"

extern long offset;
%}


%option noyywrap
%option nounput
%option noinput
%option yylineno

%%


"," { 
	return TOK_COMMA;
}

"abs" {
	return TOK_ABS;
}

"-" {
	return TOK_NEG;
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

([ \t]*\n)* {
	return TOK_NEW_LINE;
}

"*" {
	return TOK_STAR;
}

"=" {
	return TOK_EQ;
}

s[0-9]+ {
	si2bin_yylval.id = si2bin_id_create(yytext);
	return TOK_SCALAR_REGISTER;
}

v[0-9]+ {
	si2bin_yylval.id = si2bin_id_create(yytext);
	return TOK_VECTOR_REGISTER;
}

vcc|scc|exec {
	si2bin_yylval.id = si2bin_id_create(yytext);
	return TOK_SPECIAL_REGISTER;
}

m[0-9]+ {
	si2bin_yylval.id = si2bin_id_create(yytext);
	return TOK_MEMORY_REGISTER;
}

uav[0-9]+ {
	si2bin_yylval.id = si2bin_id_create(yytext);
	return TOK_UAV;
}

"hl" {
	return TOK_HL;
}

"format" {
	return TOK_FORMAT;
}

"offen" {
	return TOK_OFFEN;
}

"idxen" {
	return TOK_IDXEN;
}

"offset" {
	return TOK_OFFSET;
}

".global" {
	return TOK_GLOBAL;
}

".globl" {
	return TOK_GLOBAL;
}

".metadata" {
	return TOK_METADATA;
}

".args" {
	return TOK_ARGS;
}

".data" {
	return TOK_DATA;
}

".text" {
	return TOK_TEXT;
}

"const" {
	return TOK_CONST;
}

".int" {
	return TOK_INT_DECL;
}

".short" {
	return TOK_SHORT_DECL;
}

".float" {
	return TOK_FLOAT_DECL;
}

".half" {
	return TOK_HALF_DECL;
}

".word" {
	return TOK_WORD_DECL;
}
	
".byte" {
	return TOK_BYTE_DECL;
}


[a-zA-Z_][a-zA-Z0-9_]* {   
	si2bin_yylval.id = si2bin_id_create(yytext);
	return TOK_ID; 
}

0x[0-9a-z]+ { 
	si2bin_yylval.id = si2bin_id_create(yytext);
	return TOK_HEX;	
}

[+-]?[0-9]+"."[0-9]+ {
	si2bin_yylval.num_float = atof(yytext);
	return TOK_FLOAT;
}

[+-]?[0-9]+ {
	si2bin_yylval.num = atoi(yytext);
	return TOK_DECIMAL;
}

"//"[^\n]* {
	/* Ignore comments */
}

"#"[^\n]* {
	/* Ignore comments */
}

[\t ] {
	/* Ignore tabs and spaces */
}

. {
	/* Lexical error */
	si2bin_yyerror_fmt("unexpected character: %s\n", yytext);
}

%%

