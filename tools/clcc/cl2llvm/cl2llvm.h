/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef TOOLS_CLCC_CL2LLVM_CL2LLVM_H
#define TOOLS_CLCC_CL2LLVM_CL2LLVM_H

#include <stdio.h>

#include <lib/util/debug.h>

/* Forward declarations */

struct list_t;
struct hash_table_t;

int cl2llvm_get_col_num(void);
void cl2llvm_set_col_num(int);
int cl2llvm_yyget_lineno(void);
void cl2llvm_yyset_lineno(int);
int cl2llvm_yylex(void);
int cl2llvm_yyparse(void);
int temp_var_count;
char temp_var_name[50];


void cl2llvm_yyerror(char *s);
void cl2llvm_yyerror_fmt(char *fmt, ...);

extern FILE *cl2llvm_yyin;
extern char *cl2llvm_file_name;

extern struct hash_table_t *cl2llvm_symbol_table;




/*
 * Public Functions
 */

void cl2llvm_init(void);
void cl2llvm_done(void);

void cl2llvm_compile(struct list_t *source_file_list,
		struct list_t *llvm_file_list);

#endif

