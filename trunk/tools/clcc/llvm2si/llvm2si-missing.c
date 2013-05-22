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

#include <stdio.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "llvm2si.h"


/*
 * Private Functions
 */

static char *llvm2si_err_note =
	"\tThe LLVM-to-SI back-end could not be compiled due to an unsatisfied\n"
	"\tdependence with the LLVM libraries. Please install this library and\n"
	"\trecompile.\n";

static void llvm2si_missing(void)
{
	fatal("LLVM to Southern Islands back-end not available.\n%s",
			llvm2si_err_note);
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
	llvm2si_missing();
}

