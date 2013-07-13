/**
 * It's a Nvidia Fermi ISA assembler for multi2sim
 */

%{

#include "parser.h"
#include "frm2bin.h"
#include "inst.h"
#include "id.h"
void yyerror_lex(const char *msg);

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

"+" {
	return TOK_ADD;
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

"]" {
  	return TOK_CBRA;
}

":" {
  	return TOK_COLON;
}

";" {
  	return TOK_SEMICOLON;
}

"&" {
  	return TOK_AMP;
}

".global" {
	return TOK_GLOBAL;
}

".args" {
	return TOK_ARGS;
}

".text" {
	return TOK_TEXT;
}

"\n" {
  	return TOK_NEW_LINE;
}

.[bfsu][0-9]+ {
	frm2bin_yylval.id = frm_id_create(yytext);
	return TOK_ARG_TYPE;
}

"pt" {
	/* do not understand it yet, in isetp instruction */
  	frm2bin_yylval.id = frm_id_create(yytext);
  	return TOK_PT;
}

P[0-9]+ {
  	frm2bin_yylval.id = frm_id_create(yytext);
  	return TOK_PRED_REGISTER;
}

@P[0-9]+ {
	/* atoi, start from the 3rd char, get the number 
  	 * there maybe some !P4 predicate */
  	frm2bin_yylval.pred = frm2bin_pred_create(atoi(&yytext[2]));
  	return TOK_PRED;
}

@!P[0-9]+ {
	/* atoi, start from the 3rd char, get the number */
  	frm2bin_yylval.pred = frm2bin_pred_create(atoi(&yytext[2]) + 8);
  	return TOK_PRED;
}


RZ {
  	frm2bin_yylval.id = frm_id_create(yytext);
  	return TOK_ZERO_REGISTER;
}

R[0-9]+ {
  	frm2bin_yylval.id = frm_id_create(yytext);
  	return TOK_SCALAR_REGISTER;
}

SR_[a-zA-Z0-9_]* {
  	frm2bin_yylval.id = frm_id_create(yytext);
  	return TOK_SPECIAL_REGISTER;
}

CC.[A-Z_]* {
	frm2bin_yylval.id = frm_id_create(yytext);
	return TOK_CCOP;
}

[-]?[a-zA-Z][a-zA-Z0-9_\.]* {
  	frm2bin_yylval.id = frm_id_create(yytext);
  	return TOK_ID;
}

[-]?0x[0-9a-f]+ {
  	frm2bin_yylval.id = frm_id_create(yytext);
  	return TOK_HEX;
}

[-]?[0-9]+ {
  	frm2bin_yylval.num = atoi(yytext);
  	return TOK_DECIMAL;
}

"'//'[^\n]*" {
	/* Ignore comments */
}

\/\*[0-9a-fx]+\*\/ {
	/* Ignore comments */
}

"[\t ]" {
	/* Ignore tabs and spaces */
}

"." {
	/* Lexical error */
  	frm2bin_yyerror_fmt("unexpected character: %s\n", yytext);
}

%%
