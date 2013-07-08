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

#include <lib/util/debug.h>

#include "amd.h"



/*
 * Global Variables
 */

int amd_opt_level;
int amd_dump_all;
int amd_list_devices;
char *amd_device_name = "";



/*
 * Private Functions
 */

static char *amd_err_note =
	"\tThe AMD Catalyst driver or AMD APP software kit are not present in your\n"
	"\tsystem. The '--amd' option relies on the native drivers to be installed\n"
	"\ton the machine, and acts as a command-line wrapper of AMD's compiler.\n"
	"\tPlease install this software and recompile Multi2Sim.\n";

static void amd_missing(void)
{
	fatal("AMD native drivers not available.\n%s",
			amd_err_note);
}





/*
 * Public Functions
 */

void amd_init(void)
{
	amd_missing();
}


void amd_done(void)
{
	amd_missing();
}


void amd_dump_device_list(FILE *f)
{
	amd_missing();
}


void amd_compile(struct list_t *source_file_list, struct list_t *bin_file_list)
{
	amd_missing();
}

