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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "glew.h"


int glew_debug_category;

static char *glew_err_missing =
	"\tMulti2Sim has been compiled without support for OpenGL graphics.\n"
	"\tPlease install the OpenGL/GLUT/GLU/GLEW development libraries on\n"
	"\tyour system and retry compilation.\n";


/*
 * Private Functions
 */

#define __GLEW_MISSING__  glew_missing();
static void glew_missing(void)
{
	fatal("support for GLEW not available.\n%s",
		glew_err_missing);
}




/*
 * Public Functions
 */

void glew_init(void)
{
	/* Silent missing feature */
}


void glew_done(void)
{
	/* Silent missing feature */
}


int glew_abi_call(X86Context *context)
{
	__GLEW_MISSING__
	return 0;
}


