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


#ifndef RUNTIME_GLEW_API_H
#define RUNTIME_GLEW_API_H


/* Debug */
extern int glew_debug;

#define glew_debug(stream, ...) (glew_debug ? fprintf((stream), __VA_ARGS__) : (void) 0)

/* System call for glew runtime */
#define GLEW_SYSCALL_CODE  330


/* List of glew runtime calls */
enum glew_call_t
{
	glew_call_invalid,

	glew_call_init,

	glew_call_count
};

#endif

