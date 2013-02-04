%{
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
						yylval.sym = strdup(yytext);
						return TOK_GENERAL_REGISTER;
					   }

{TOK_SPECIAL_REGISTER} {
						yylval.sym = strdup(yytext);
						return TOK_SPECIAL_REGISTER;
					   }

{TOK_ID} {   
    	  yylval.sym = strdup(yytext);
		  return TOK_ID; 
         }
{TOK_HEX} { 
		   yylval.sym = strdup(yytext);
		   return TOK_HEX;	
		  }
{TOK_AMP} {
		   return TOK_AMP;
		  }
"\n" { 
	  return '\n';
     }
{TOK_COMMENT} {
				/* Used to parse out comments from .isa */
              }
[\t ] {
	  }
. {
	fprintf(stderr, "error: unexpected character: %s\n", yytext);
	exit(1);
  }

%%

