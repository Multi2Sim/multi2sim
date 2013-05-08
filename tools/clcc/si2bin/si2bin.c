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

#include <stdarg.h>
#include <stdio.h>

#include <arch/southern-islands/asm/asm.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-encode.h>
#include <lib/util/list.h>

#include "bin.h"
#include "inst.h"
#include "inst-info.h"
#include "si2bin.h"
#include "parser.h"
#include "symbol.h"
#include "task.h"


int si2bin_assemble;  /* Command-line option set */
char *si2bin_file_name;  /* Current file */

void si2bin_yyerror(const char *s)
{
	fprintf(stderr, "%s:%d: error: %s\n",
			si2bin_file_name, si2bin_yylineno, s);
	exit(1);
}


void si2bin_yyerror_fmt(char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "%s:%d: error: ",
			si2bin_file_name, si2bin_yylineno);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	fflush(NULL);
	exit(1);
}



/*
 * Public Functions
 */

void si2bin_init(void)
{
	/* Initialize */
	si_disasm_init();
	si2bin_inst_info_init();
	si2bin_task_list_init();
	si2bin_symbol_table_init();
	//si2bin_stream_init();
	si2bin_bin_init();

}


void si2bin_done(void)
{
	/* Finalize */
	si2bin_task_list_done();
	si2bin_symbol_table_done();
	//si2bin_stream_done();
	si2bin_bin_done();
	si2bin_inst_info_done();
	si_disasm_done();
}


void si2bin_compile(struct list_t *source_file_list,
		struct list_t *bin_file_list)
{
	int index;

	LIST_FOR_EACH(source_file_list, index)
	{
		/* Open file */
		si2bin_file_name = list_get(source_file_list, index);
		si2bin_yyin = fopen(si2bin_file_name, "r");
		if (!si2bin_yyin)
			fatal("%s: cannot open input file",
					si2bin_file_name);
	
		/* Parse input */
		si2bin_yyparse();

		/* Process pending tasks */
		si2bin_task_list_process();

		/* Close */
		fclose(si2bin_yyin);
	}
}

