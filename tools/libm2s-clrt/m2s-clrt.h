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

#ifndef M2S_CLRT_H
#define M2S_CLRT_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <CL/cl.h>





/*
 * Error macros
 */

extern char *m2s_clrt_err_not_impl;
extern char *m2s_clrt_err_note;
extern char *m2s_clrt_err_param_note;

#define __M2S_CLRT_NOT_IMPL__  \
	fatal("%s: OpenCL call not implemented.\n%s", __FUNCTION__, m2s_clrt_err_not_impl);
#define EVG_OPENCL_ARG_NOT_SUPPORTED(p) \
	fatal("%s: not supported for '" #p "' = 0x%x\n%s", __FUNCTION__, p, m2s_clrt_err_note);
#define EVG_OPENCL_ARG_NOT_SUPPORTED_EQ(p, v) \
	{ if ((p) == (v)) fatal("%s: not supported for '" #p "' = 0x%x\n%s", __FUNCTION__, (v), m2s_clrt_err_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: not supported for '" #p "' != 0x%x\n%s", __FUNCTION__, (v), m2s_clrt_err_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_LT(p, v) \
	{ if ((p) < (v)) fatal("%s: not supported for '" #p "' < %d\n%s", __FUNCTION__, (v), m2s_clrt_err_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_RANGE(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) fatal("%s: not supported for '" #p "' out of range [%d:%d]\n%s", \
	__FUNCTION__, (min), (max), m2s_clrt_err_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_FLAG(p, flag, name) \
	{ if ((p) & (flag)) fatal("%s: flag '" name "' not supported\n%s", __FUNCTION__, m2s_clrt_err_param_note); }




/*
 * OpenCL Types
 */

struct _cl_platform_id
{
	unsigned int id;
};


struct _cl_device_id
{
	unsigned int id;

	cl_platform_id platform;
};


struct _cl_context
{
	unsigned int id;
};


struct _cl_command_queue
{
	unsigned int id;
};


struct _cl_mem
{
	unsigned int id;
};


struct _cl_program
{
	unsigned int id;

	cl_context context;
};


struct _cl_kernel
{
	unsigned int id;
};


struct _cl_event
{
	unsigned int id;
};


struct _cl_sampler
{
	unsigned int id;
};


#endif

