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

#ifndef M2S_OPENCL_H
#define M2S_OPENCL_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <evergreen-emu.h>
#include <CL/cl.h>

#define EVG_SYS_OPENCL_IMPL_VERSION_MAJOR		1
#define EVG_SYS_OPENCL_IMPL_VERSION_MINOR		0
#define EVG_SYS_OPENCL_IMPL_VERSION_BUILD		0
#define EVG_SYS_OPENCL_IMPL_VERSION			((EVG_SYS_OPENCL_IMPL_VERSION_MAJOR << 16) | \
						(EVG_SYS_OPENCL_IMPL_VERSION_MINOR << 8) | \
						EVG_SYS_OPENCL_IMPL_VERSION_BUILD)

#define SYS_CODE_OPENCL  325
#define SYS_OPENCL_FUNC_FIRST  1000
#define SYS_OPENCL_FUNC_LAST  1073
#define SYS_OPENCL_FUNC_COUNT  74
#define SYS_OPENCL_MAX_ARGS  14


#define OPENCL_FUNC_clGetPlatformIDs                  1000
#define OPENCL_FUNC_clGetPlatformInfo                 1001

#define OPENCL_FUNC_clGetDeviceIDs                    1002
#define OPENCL_FUNC_clGetDeviceInfo                   1003

#define OPENCL_FUNC_clCreateContext                   1004
#define OPENCL_FUNC_clCreateContextFromType           1005
#define OPENCL_FUNC_clRetainContext                   1006
#define OPENCL_FUNC_clReleaseContext                  1007
#define OPENCL_FUNC_clGetContextInfo                  1008

#define OPENCL_FUNC_clCreateCommandQueue              1009
#define OPENCL_FUNC_clRetainCommandQueue              1010
#define OPENCL_FUNC_clReleaseCommandQueue             1011
#define OPENCL_FUNC_clGetCommandQueueInfo             1012
#define OPENCL_FUNC_clSetCommandQueueProperty         1013

#define OPENCL_FUNC_clCreateBuffer                    1014
#define OPENCL_FUNC_clCreateSubBuffer                 1015
#define OPENCL_FUNC_clCreateImage2D                   1016
#define OPENCL_FUNC_clCreateImage3D                   1017
#define OPENCL_FUNC_clRetainMemObject                 1018
#define OPENCL_FUNC_clReleaseMemObject                1019
#define OPENCL_FUNC_clGetSupportedImageFormats        1020
#define OPENCL_FUNC_clGetMemObjectInfo                1021
#define OPENCL_FUNC_clGetImageInfo                    1022
#define OPENCL_FUNC_clSetMemObjectDestructorCallback  1023

#define OPENCL_FUNC_clCreateSampler                   1024
#define OPENCL_FUNC_clRetainSampler                   1025
#define OPENCL_FUNC_clReleaseSampler                  1026
#define OPENCL_FUNC_clGetSamplerInfo                  1027

#define OPENCL_FUNC_clCreateProgramWithSource         1028
#define OPENCL_FUNC_clCreateProgramWithBinary         1029
#define OPENCL_FUNC_clRetainProgram                   1030
#define OPENCL_FUNC_clReleaseProgram                  1031
#define OPENCL_FUNC_clBuildProgram                    1032
#define OPENCL_FUNC_clUnloadCompiler                  1033
#define OPENCL_FUNC_clGetProgramInfo                  1034
#define OPENCL_FUNC_clGetProgramBuildInfo             1035

#define OPENCL_FUNC_clCreateKernel                    1036
#define OPENCL_FUNC_clCreateKernelsInProgram          1037
#define OPENCL_FUNC_clRetainKernel                    1038
#define OPENCL_FUNC_clReleaseKernel                   1039
#define OPENCL_FUNC_clSetKernelArg                    1040
#define OPENCL_FUNC_clGetKernelInfo                   1041
#define OPENCL_FUNC_clGetKernelWorkGroupInfo          1042

#define OPENCL_FUNC_clWaitForEvents                   1043
#define OPENCL_FUNC_clGetEventInfo                    1044
#define OPENCL_FUNC_clCreateUserEvent                 1045
#define OPENCL_FUNC_clRetainEvent                     1046
#define OPENCL_FUNC_clReleaseEvent                    1047
#define OPENCL_FUNC_clSetUserEventStatus              1048
#define OPENCL_FUNC_clSetEventCallback                1049
#define OPENCL_FUNC_clGetEventProfilingInfo           1050
#define OPENCL_FUNC_clFlush                           1051
#define OPENCL_FUNC_clFinish                          1052

#define OPENCL_FUNC_clEnqueueReadBuffer               1053
#define OPENCL_FUNC_clEnqueueReadBufferRect           1054
#define OPENCL_FUNC_clEnqueueWriteBuffer              1055
#define OPENCL_FUNC_clEnqueueWriteBufferRect          1056
#define OPENCL_FUNC_clEnqueueCopyBuffer               1057
#define OPENCL_FUNC_clEnqueueCopyBufferRect           1058
#define OPENCL_FUNC_clEnqueueReadImage                1059
#define OPENCL_FUNC_clEnqueueWriteImage               1060
#define OPENCL_FUNC_clEnqueueCopyImage                1061
#define OPENCL_FUNC_clEnqueueCopyImageToBuffer        1062
#define OPENCL_FUNC_clEnqueueCopyBufferToImage        1063
#define OPENCL_FUNC_clEnqueueMapBuffer                1064
#define OPENCL_FUNC_clEnqueueMapImage                 1065
#define OPENCL_FUNC_clEnqueueUnmapMemObject           1066
#define OPENCL_FUNC_clEnqueueNDRangeKernel            1067
#define OPENCL_FUNC_clEnqueueTask                     1068
#define OPENCL_FUNC_clEnqueueNativeKernel             1069
#define OPENCL_FUNC_clEnqueueMarker                   1070
#define OPENCL_FUNC_clEnqueueWaitForEvents            1071
#define OPENCL_FUNC_clEnqueueBarrier                  1072

#define OPENCL_FUNC_clGetExtensionFunctionAddress     1073



/* Error macros */

extern char *err_evg_opencl_note;
extern char *err_evg_opencl_param_note;

#define EVG_OPENCL_ARG_NOT_SUPPORTED(p) \
	fatal("%s: not supported for '" #p "' = 0x%x\n%s", __FUNCTION__, p, err_evg_opencl_note);
#define EVG_OPENCL_ARG_NOT_SUPPORTED_EQ(p, v) \
	{ if ((p) == (v)) fatal("%s: not supported for '" #p "' = 0x%x\n%s", __FUNCTION__, (v), err_evg_opencl_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_NEQ(p, v) \
	{ if ((p) != (v)) fatal("%s: not supported for '" #p "' != 0x%x\n%s", __FUNCTION__, (v), err_evg_opencl_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_LT(p, v) \
	{ if ((p) < (v)) fatal("%s: not supported for '" #p "' < %d\n%s", __FUNCTION__, (v), err_evg_opencl_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_RANGE(p, min, max) \
	{ if ((p) < (min) || (p) > (max)) fatal("%s: not supported for '" #p "' out of range [%d:%d]\n%s", \
	__FUNCTION__, (min), (max), err_evg_opencl_param_note); }
#define EVG_OPENCL_ARG_NOT_SUPPORTED_FLAG(p, flag, name) \
	{ if ((p) & (flag)) fatal("%s: flag '" name "' not supported\n%s", __FUNCTION__, err_evg_opencl_param_note); }





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
	struct evg_bin_file_t *amd_bin;
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

