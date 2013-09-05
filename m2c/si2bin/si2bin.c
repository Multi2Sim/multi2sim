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

#include <lib/class/elf-writer.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "inst-info.h"
#include "outer-bin.h"
#include "si2bin.h"
#include "symbol.h"
#include "task.h"


/*
 * Global Variables
 */

/* Command-line options */
char *si2bin_machine_name = "tahiti";

/* Current source file name being assembled. Set internally in function
 * 'si2bin_compile()' */
char *si2bin_source_file;

/* Current output binary buffer being assembled. Set internally in function
 * 'si2bin_compile()' */
struct si2bin_outer_bin_t *si2bin_outer_bin;
struct si2bin_inner_bin_entry_t *si2bin_entry;
struct si2bin_inner_bin_t *si2bin_inner_bin;
struct si2bin_metadata_t *si2bin_metadata;

ELFWriterBuffer *bin_buffer;

int si2bin_uniqueid = 1024;




/*
 * Public Functions
 */

void si2bin_yyerror(const char *s)
{
	fprintf(stderr, "%s:%d: error: %s\n",
			si2bin_source_file, si2bin_yylineno, s);
	exit(1);
}


void si2bin_yyerror_fmt(char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "%s:%d: error: ",
			si2bin_source_file, si2bin_yylineno);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	fflush(NULL);
	exit(1);
}



/*
 * Class 'Si2bin'
 */

void Si2binCreate(Si2bin *self)
{
	/* Initialize */
	si2bin_inst_info_init();
	si2bin_task_list_init();
	si2bin_symbol_table_init();
}


void Si2binDestroy(Si2bin *self)
{
	/* Finalize */
	si2bin_task_list_done();
	si2bin_symbol_table_done();
	si2bin_inst_info_done();
}


void Si2binCompile(Si2bin *self, struct list_t *source_file_list,
		struct list_t *bin_file_list)
{
	int index;
	char *output_file;
	FILE *f;
	
	LIST_FOR_EACH(source_file_list, index)
	{
		/* Open source file */
		si2bin_source_file = list_get(source_file_list, index);
		si2bin_yyin = fopen(si2bin_source_file, "r");
		if (!si2bin_yyin)
			fatal("%s: cannot open input file", si2bin_source_file);

		/* Open output file */
		output_file = list_get(bin_file_list, index);
		f = fopen(output_file, "wb");
		if (!f)
			fatal("%s: cannot output output file", output_file);

		/* Create output buffer */
		si2bin_outer_bin = si2bin_outer_bin_create();
		bin_buffer = new(ELFWriterBuffer);
		
		/* Parse input */
		si2bin_yyparse();

		/* Close source file */
		fclose(si2bin_yyin);
		
		/* Dump output buffer and free it */
		si2bin_outer_bin_generate(si2bin_outer_bin, bin_buffer);
		
		/* Write contents of buffer to file */
		ELFWriterBufferWriteToFile(bin_buffer, f);

		/* Free Outer ELF and bin_buffer */
		si2bin_outer_bin_free(si2bin_outer_bin);
		delete(bin_buffer);
	}
}

