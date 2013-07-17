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

#include <pthread.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "glut.h"


int glut_debug_category;

pthread_mutex_t glut_mutex;

static char *glut_err_missing =
	"\tMulti2Sim has been compiled without support for OpenGL graphics.\n"
	"\tPlease install the OpenGL/GLUT/GLU/GLEW development libraries on\n"
	"\tyour system and retry compilation.\n";


/*
 * Private Functions
 */

#define __GLUT_MISSING__  glut_missing();
static void glut_missing(void)
{
	fatal("support for GLUT not available.\n%s",
		glut_err_missing);
}




/*
 * Public Functions
 */

void glut_init(void)
{
	/* Silent missing feature */
}


void glut_done(void)
{
	/* Silent missing feature */
}


int glut_abi_call(X86Context *context)
{
	__GLUT_MISSING__
	return 0;
}




/*
 * Frame Buffer
 */

void glut_frame_buffer_init(void)
{
	/* Silent missing initialization */
}


void glut_frame_buffer_done(void)
{
	/* Silent missing finalization */
}


void glut_frame_buffer_clear(void)
{
	__GLUT_MISSING__
}


void glut_frame_buffer_pixel(int x, int y, int color)
{
	__GLUT_MISSING__
}


void glut_frame_buffer_resize(int width, int height)
{
	__GLUT_MISSING__
}


void glut_frame_buffer_get_size(int *width, int *height)
{
	__GLUT_MISSING__
}


void glut_frame_buffer_flush_request(void)
{
	__GLUT_MISSING__
}


void glut_frame_buffer_flush_if_requested(void)
{
	__GLUT_MISSING__
}

