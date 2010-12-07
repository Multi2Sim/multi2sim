/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#include <gpukernel.h>
#include <assert.h>
#include <debug.h>


/* Debug info */
int opencl_debug_category;


/* List of OpenCL function names */
char *opencl_func_names[] = {
#define DEF_OPENCL_FUNC(_name, _argc) #_name,
#include "opencl.dat"
#undef DEF_OPENCL_FUNC
	""
};


/* Number of arguments for each OpenCL function */
int opencl_func_argc[] = {
#define DEF_OPENCL_FUNC(_name, _argc) _argc,
#include "opencl.dat"
#undef DEF_OPENCL_FUNC
	0
};


int opencl_func_run(int code, unsigned int *args)
{
	char *func_name;
	int func_code, func_argc;
	int retval = 0;
	
	assert(code >= OPENCL_FUNC_FIRST && code <= OPENCL_FUNC_LAST);
	func_code = code - OPENCL_FUNC_FIRST;
	func_name = opencl_func_names[func_code];
	func_argc = opencl_func_argc[func_code];

	switch (func_code) {

	case OPENCL_FUNC_clGetPlatformIDs:
		
		//opencl_debug(

		break;
	
	default:
		
		fatal("opencl_func_run: function '%s' (code=%d) not implemented.\n"
			"\tThe OpenCL interface is implemented in library 'm2s-opencl.so' as a set of\n"
			"\tsystem calls, intercepted by Multi2Sim and emulated in 'opencl.c'.\n"
			"\tHowever, only a subset of this interface is currently implemented in the simulator.\n"
			"\tTo request the implementation of a specific OpenCL call, please email\n"
			"\t'development@multi2sim.org'.",
			func_name, code);
	}

	return retval;
}

