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


int x86_glut_debug_category;

pthread_mutex_t x86_glut_mutex;



/*
 * Private Functions
 */

#define __X86_GLUT_MISSING__  x86_glut_missing();
static void x86_glut_missing(void)
{
	fatal("support for GLUT not available.\n"
		"\tMulti2Sim requires the OpenGL Utility Toolkit (GLUT) library to be\n"
		"\tinstalled in your system to simulate OpenGL programs.\n"
		"\t  1) Install the development packages for GLUT. Under Debian-based\n"
		"\t     Linux distributions, this package is listed as 'freeglut3-dev'.\n"
		"\t  2) Re-run the './configure' script\n"
		"\t  3) Recompile the simulator: make clean && make\n");
}




/*
 * Public Functions
 */

void x86_glut_init(void)
{
	/* Silent missing feature */
}


void x86_glut_done(void)
{
	/* Silent missing feature */
}


int x86_glut_call(struct x86_ctx_t *ctx)
{
	__X86_GLUT_MISSING__
	return 0;
}




/*
 * Frame Buffer
 */

void x86_glut_frame_buffer_init(void)
{
	/* Silent missing initialization */
}


void x86_glut_frame_buffer_done(void)
{
	/* Silent missing finalization */
}


void x86_glut_frame_buffer_clear(void)
{
	__X86_GLUT_MISSING__
}


void x86_glut_frame_buffer_pixel(int x, int y, int color)
{
	__X86_GLUT_MISSING__
}


void x86_glut_frame_buffer_resize(int width, int height)
{
	__X86_GLUT_MISSING__
}


void x86_glut_frame_buffer_get_size(int *width, int *height)
{
	__X86_GLUT_MISSING__
}


void x86_glut_frame_buffer_flush_request(void)
{
	__X86_GLUT_MISSING__
}


void x86_glut_frame_buffer_flush_if_requested(void)
{
	__X86_GLUT_MISSING__
}

