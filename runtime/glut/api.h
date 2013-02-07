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


#ifndef RUNTIME_GLUT_API_H
#define RUNTIME_GLUT_API_H

/* Debug */
extern int glut_debug;

#define glut_debug(stream, ...) (glut_debug ? fprintf((stream), __VA_ARGS__) : (void) 0)

/* System call for GLUT runtime */
#define GLUT_SYSCALL_CODE  326


/* List of GLUT runtime calls */
enum glut_call_t
{
	glut_call_invalid,
	glut_call_init,
	glut_call_get_event,
	glut_call_new_window,
	glut_call_test_draw,
	glut_call_count
};


#endif

