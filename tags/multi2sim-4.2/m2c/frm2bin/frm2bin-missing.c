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

#include "frm2bin.h"




/*
 * Private Functions
 */

static char *frm2bin_err_note =
	"\tThe Fermi assembler could not be compiled due to missing\n"
	"\tlibraries on your system, as reported by the ./configure script.\n"
	"\tPlease install these libraries and recompile.\n";

static void frm2bin_missing(void)
{
	fatal("Fermi assembler not available.\n%s",
		frm2bin_err_note);
}




/*
 * Public Functions
 */

void frm2bin_init(void)
{
}


void frm2bin_done(void)
{
}


void frm2bin_compile(struct list_t *source_file_list,
		struct list_t *bin_file_list)
{
	frm2bin_missing();
}

