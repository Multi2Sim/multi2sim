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

#include "opengl.h"


static char *opengl_err_missing =
	"\tMulti2Sim has been compiled without support for OpenGL applicationn.\n"
	"\tThe OpenGL headers and OpenGL Utility Toolkit (GLUT) is required in\n"
	"\tyour system to allow for simulation of guest OpenGL programs.\n"
	"\t  1) Install the development packages for GLUT. Under Debian-based\n"
	"\t     Linux distributions, this package is listed as 'freeglut3-dev'.\n"
	"\t  2) Re-run the './configure' script\n"
	"\t  3) Recompile the simulator: make clean && make\n";


#define __OPENGL_MISSING__  opengl_missing();
static void opengl_missing(void)
{
	fatal("support for OpenGL not available.\n%s",
		opengl_err_missing);
}



/* Satisfy external reference to debug category */
int opengl_debug_category;


void opengl_init(void)
{
	/* Silent missing feature */
}


void opengl_done(void)
{
	/* Silent missing feature */
}


int opengl_abi_call(X86Context *context)
{
	__OPENGL_MISSING__
	return 0;
}

