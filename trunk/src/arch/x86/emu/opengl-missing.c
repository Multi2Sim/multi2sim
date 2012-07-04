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

#include <x86-emu.h>

static char *x86_opengl_err_missing =
	"\tMulti2Sim has been compiled without support for OpenGL applicationn.\n"
	"\tThe OpenGL headers and OpenGL Utility Toolkit (GLUT) is required in\n"
	"\tyour system to allow for simulation of guest OpenGL programs.\n"
	"\t  1) Install the development packages for GLUT. Under Debian-based\n"
	"\t     Linux distributions, this package is listed as 'freeglut3-dev'.\n"
	"\t  2) Re-run the './configure' script\n"
	"\t  3) Recompile the simulator: make clean && make\n";


#define __X86_OPENGL_MISSING__  x86_opengl_missing();
static void x86_opengl_missing(void)
{
	fatal("support for OpenGL not available.\n%s",
		x86_opengl_err_missing);
}



/* Satisfy external reference to debug category */
int x86_opengl_debug_category;


void x86_opengl_init(void)
{
	/* Silent missing feature */
}


void x86_opengl_done(void)
{
	/* Silent missing feature */
}


int x86_opengl_call(struct x86_ctx_t *ctx)
{
	__X86_OPENGL_MISSING__
	return 0;
}

