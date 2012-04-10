/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <assert.h>
#include <debug.h>

#include <evergreen-emu.h>


/* Required 'libm2s-opencl.so' version.
 * Increase this number when the user is required to download an updated release
 * of the Multi2Sim OpenCL implementation. */
#define SYS_OPENCL_IMPL_VERSION_MAJOR		1
#define SYS_OPENCL_IMPL_VERSION_MINOR		0
#define SYS_OPENCL_IMPL_VERSION_BUILD		0
#define SYS_OPENCL_IMPL_VERSION			((SYS_OPENCL_IMPL_VERSION_MAJOR << 16) | \
						(SYS_OPENCL_IMPL_VERSION_MINOR << 8) | \
						SYS_OPENCL_IMPL_VERSION_BUILD)


/* OpenCL API Implementation */

int opencl_syscall(int code, unsigned int *args)
{
	char err_prefix[MAX_STRING_SIZE];
	char *func_name;
	int func_code, func_argc;
	int retval = 0;
	
	/* Decode OpenCL function */
	assert(code >= OPENCL_FUNC_FIRST && code <= OPENCL_FUNC_LAST);
	func_code = code - OPENCL_FUNC_FIRST;
	func_name = opencl_func_names[func_code];
	func_argc = opencl_func_argc[func_code];
	assert(func_argc <= OPENCL_MAX_ARGS);
	snprintf(err_prefix, MAX_STRING_SIZE, "OpenCL call '%s'", func_name);
	
	/* Execute */
	opencl_debug("%s\n", func_name);
	switch (func_code) {

	/* 1000 */
	case OPENCL_FUNC_clGetPlatformIDs:
	{
		break;
	}

	default:
		
		fatal("opencl_syscall: function '%s' (code=%d) not implemented.\n%s",
			func_name, code, err_opencl_note);
	}

	return retval;
}

