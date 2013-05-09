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

#ifndef RUNTIME_OPENGL_OPENGL_H
#define RUNTIME_OPENGL_OPENGL_H

#include "../include/GL/gl.h"


/* Generic call-back function */
typedef void *(*opengl_callback_t)(void *);

/* Function code to pass as a first argument of a system call */
enum opengl_abi_call_t
{
	opengl_abi_invalid,
#define OPENGL_ABI_CALL(name, code) opengl_abi_##name = code,
#include "../../src/driver/opengl/opengl.dat"
#undef OPENGL_ABI_CALL
	opengl_abi_call_count
};

/* System call for OpenGL runtime */
#define OPENGL_SYSCALL_CODE  327

/*
 * Public Functions 
 */

void opengl_debug(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

extern char *opengl_err_not_impl;
extern char *opengl_err_note;
extern char *opengl_err_param_note;

#define __OPENGL_NOT_IMPL__  \
	fatal("%s: OpenGL call not implemented.\n%s", __FUNCTION__, opengl_err_not_impl);


#endif
