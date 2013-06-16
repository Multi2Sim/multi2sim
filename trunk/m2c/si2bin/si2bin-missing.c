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

#include "si2bin.h"




/*
 * Private Functions
 */

static char *si2bin_err_note =
	"\tThe Southern Islands assembler could not be compiled due to missing\n"
	"\tlibraries on your system, as reported by the ./configure script.\n"
	"\tPlease install these libraries and recompile.\n";

static void si2bin_missing(void)
{
	fatal("Southern Islands assembler not available.\n%s",
		si2bin_err_note);
}




/*
 * Public Functions
 */

void si2bin_init(void)
{
}


void si2bin_done(void)
{
}


void si2bin_compile(struct list_t *source_file_list,
		struct list_t *bin_file_list)
{
	si2bin_missing();
}

