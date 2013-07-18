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

#ifndef DRIVER_GLEW_GLEW_H
#define DRIVER_GLEW_GLEW_H

#include <lib/util/class.h>

/*
 * GLEW system call interface
 *
 * NOTE: for every new function or external variable added here, its
 * implementation should be added in the regular 'glew-xx.c' files and also in
 * 'glew-missing.c' to allow for correct compilation when the GLEW library is
 * missing in the user's system.
 */

#define glew_debug(...) debug(glew_debug_category, __VA_ARGS__)
extern int glew_debug_category;

void glew_init(void);
void glew_done(void);

int glew_abi_call(X86Context *ctx);


#endif

