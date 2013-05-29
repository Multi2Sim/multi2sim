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

#include <assert.h>
#include <stdio.h>

#include <clcc/si2bin/arg.h>
#include <clcc/si2bin/inst.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "llvm2si.h"
#include "translate.h"


/*
 * Private Functions
 */

void llvm2si_compile_source(FILE *inf, FILE *outf)
{
	struct list_t *arg_list;

	struct si2bin_inst_t *inst;
	struct si2bin_arg_t *arg;

	/* Create argument list */
	arg_list = list_create();

	/* Argument 1 */
	arg = si2bin_arg_create_vector_register("v1");
	list_add(arg_list, arg);

	/* Argument 2 */
	arg = si2bin_arg_create_scalar_register("s0");
	list_add(arg_list, arg);

	/* Create instruction, dump, and free it */
	inst = si2bin_inst_create("v_mov_b32", arg_list);
	si2bin_inst_gen(inst);
	si2bin_inst_dump(inst, stdout);
	si2bin_inst_free(inst);
}




/*
 * Public Functions
 */

void llvm2si_init(void)
{
}


void llvm2si_done(void)
{
}


void llvm2si_compile(struct list_t *source_file_list,
		struct list_t *output_file_list)
{
	char *source_file;
	char *output_file;

	int index;

	LIST_FOR_EACH(source_file_list, index)
	{
		/* Get file names */
		source_file = list_get(source_file_list, index);
		output_file = list_get(output_file_list, index);
		assert(source_file);
		assert(output_file);

		/* Translate */
		llvm2si_translate(source_file, output_file);

		/* Info */
		printf("\t%s: Southern Islands assembly dumped\n",
				output_file);
	}
}
