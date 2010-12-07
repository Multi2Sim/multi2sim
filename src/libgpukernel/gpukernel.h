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

#ifndef GPUKERNEL_H
#define GPUKERNEL_H

/* Debugging */
#define opencl_debug(...) debug(opencl_debug_category, __VA_ARGS__)
extern int opencl_debug_category;


/* Some constants */
#define OPENCL_FUNC_FIRST  1000
#define OPENCL_FUNC_LAST  1073
#define OPENCL_MAX_ARGS  14


/* An enumeration of the OpenCL functions */
enum opencl_func_enum {
#define DEF_OPENCL_FUNC(_name, _argc) OPENCL_FUNC_##_name,
#include "opencl.dat"
#undef DEF_OPENCL_FUNC
	OPENCL_FUNC_COUNT
};


/* List of OpenCL functions and number of arguments */
extern char *opencl_func_names[];
extern int opencl_func_argc[];


/* Execute OpenCL call */
int opencl_func_run(int code, unsigned int *args);


#endif

