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
#include "parser.h"

extern long offset;
%}


%option noyywrap
%option nounput
%option noinput

TOK_DECIMAL [+-]?[0-9]+
TOK_OBRA "["
TOK_CBRA "]"
TOK_COL ":"
TOK_COMMA ","
TOK_OPAR "("
TOK_CPAR ")"
TOK_AMP "&"

TOK_GENERAL_REGISTER [msv][0-9]+
TOK_SPECIAL_REGISTER vcc|scc
TOK_ID [a-zA-Z_][a-zA-Z0-9_]*
TOK_COMMENT "// "[0-9A-Z]+": "[0-9A-Z]+" "*[0-9A-Z]* 
TOK_HEX 0x[0-9a-z]+
TOK_TAG label_[0-9]+

%%

{TOK_COMMA} { 
	return TOK_COMMA;
}

{TOK_DECIMAL} {
	yylval.num = atoi(yytext);
	return TOK_DECIMAL;
}

{TOK_OPAR} { 
	return TOK_OPAR;
}

{TOK_CPAR} { 
	return TOK_CPAR;
}

{TOK_OBRA} { 
	return TOK_OBRA;
}

{TOK_COL} { 
	return TOK_COL;
}

{TOK_CBRA} {   
	return TOK_CBRA;
}
 
{TOK_GENERAL_REGISTER} { 
	yylval.id = si_id_create(yytext);
	return TOK_GENERAL_REGISTER;
}

{TOK_SPECIAL_REGISTER} {
	yylval.id = si_id_create(yytext);
	return TOK_SPECIAL_REGISTER;
}

{TOK_ID} {   
	yylval.id = si_id_create(yytext);
	return TOK_ID; 
}

{TOK_HEX} { 
	yylval.id = si_id_create(yytext);
	return TOK_HEX;	
}

{TOK_AMP} {
	return TOK_AMP;
}

"\n" {
	return TOK_NEW_LINE;
}

{TOK_COMMENT} {
}

[\t ] {
}

. {
	fprintf(stderr, "error: unexpected character: %s\n", yytext);
	exit(1);
}

%%

