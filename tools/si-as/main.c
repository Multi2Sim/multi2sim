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

#include <stdio.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "label.h"
#include "parser.h"
#include "stream.h"
#include "task.h"

extern void yyset_in(FILE *in_str);
int yyparse(void);

int main(int argc, char **argv) 
{
	FILE *f;
	char *file_name;

	/* Check syntax */
	if (argc != 2)
		fatal("syntax: %s <file>", *argv);

	/* Open input file */
	file_name = argv[1];
	f = fopen(file_name, "r");
	if (!f)
		fatal("%s: cannot open input file", file_name);
	
	/* Open output file */
	stream = si_stream_create("out.bin");
		
	/* Create the pending task list */
	si_task_list_init();

	/* Create the hash-table which will contain label offsets*/
	si_label_table_init();

	/* Parse input */
	yyset_in(f);
	yyparse();
		
	printf("-------------Dumping Label Table-------------\n");
	si_label_table_dump(stdout);
	printf("---------------------------------------------\n");
		
	printf("------------Dumping Pending Tasks------------\n");
	si_task_list_dump(stdout);
	printf("---------------------------------------------\n");
		
	si_task_list_done();
	si_label_table_done();
	si_stream_close(stream);

	mhandle_done();
	
	return 0;
}

