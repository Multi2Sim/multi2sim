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

#include <lib/mhandle/mhandle.h>

#include "cl2llvm.h"


/*
 * Private Functions
 */

static char *cl2llvm_err_note =
	"\tThe OpenCL-to-LLVM front-end could not be compiled due to an unsatisfied\n"
	"\tdependence with Flex/Bison or LLVM libraries. Please install these\n"
	"\tlibraries and recompile.\n";

static void cl2llvm_missing(void)
{
	fatal("OpenCL-to-LLVM front-end not available.\n%s",
			cl2llvm_err_note);
}




/*
 * Public Functions
 */

void cl2llvm_init(void)
{
}


void cl2llvm_done(void)
{
}


void cl2llvm_compile(struct list_t *source_file_list,
		struct list_t *llvm_file_list, int opt_level)
{
	cl2llvm_missing();
}

