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
#include <lib/util/debug.h>
#include "gl.h"


/*
 * Public Variables
 */

int gl_dump_all;
char *gl_control_shader;
char *gl_eval_shader;
char *gl_geo_shader;




/*
 * Private Functions
 */

static char *gl_err_note =
	"\tThe OpenGL command-line compiler wrapper is not available due to\n"
	"\tunsatisfied library dependences during compilation. Please install\n"
	"\tthe OpenGL libraries and recompile.\n";

static void gl_missing(void)
{
	fatal("OpenGL compiler not available.\n%s",
			gl_err_note);
}




/*
 * Public Functions
 */

void gl_init(void)
{
}


void gl_done(void)
{
}


void gl_compile(struct list_t *source_file_list,
	struct list_t *output_file_list)
{
	gl_missing();
}

