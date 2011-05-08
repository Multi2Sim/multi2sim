/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (ubal@gap.upv.es)
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#define SYS_OPENCL_IMPL_VERSION_MAJOR		1
#define SYS_OPENCL_IMPL_VERSION_MINOR		0
#define SYS_OPENCL_IMPL_VERSION_BUILD		0
#define SYS_OPENCL_IMPL_VERSION			((SYS_OPENCL_IMPL_VERSION_MAJOR << 16) | \
						(SYS_OPENCL_IMPL_VERSION_MINOR << 8) | \
						SYS_OPENCL_IMPL_VERSION_BUILD)

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


cl_int clGetPlatformIDs(
	cl_uint num_entries,
	cl_platform_id *platforms,
	cl_uint *num_platforms)
{
	unsigned int sys_args[3];
	int ret;

	sys_args[0] = (unsigned int) num_entries;
	sys_args[1] = (unsigned int) platforms;
	sys_args[2] = (unsigned int) num_platforms;
	
	/* An additional argument is added with the version information of this
	 * OpenCL implementation. If Multi2Sim expects a later version, the
	 * system call with fail and cause a 'fatal' message. */
	sys_args[3] = SYS_OPENCL_IMPL_VERSION;

	/* Perform system call */
	ret = syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetPlatformIDs, sys_args);

	/* Detect the case where an OpenCL program linked with 'libm2s-opencl' is
	 * being run natively. */
	if (ret == -1) {
		fprintf(stderr, "error: OpenCL program cannot be run natively.\n"
			"\tThis is an error message provided by the Multi2Sim OpenCL library\n"
			"\t(libm2s-opencl). Apparently, you are attempting to run natively a\n"
			"\tprogram that was linked with this library. You should either run\n"
			"\tit on top of Multi2Sim, or link it with the OpenCL library provided\n"
			"\tin the ATI Stream SDK if you want to use your physical GPU device.\n");
		abort();
	}

	/* Result */
	return (cl_int) ret;
}


cl_int clGetPlatformInfo(
	cl_platform_id platform,
	cl_platform_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) platform;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetPlatformInfo, sys_args);
}


cl_int clGetDeviceIDs(
	cl_platform_id platform,
	cl_device_type device_type,
	cl_uint num_entries,
	cl_device_id *devices,
	cl_uint *num_devices)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) platform;
	sys_args[1] = (unsigned int) device_type;
	sys_args[2] = (unsigned int) num_entries;
	sys_args[3] = (unsigned int) devices;
	sys_args[4] = (unsigned int) num_devices;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetDeviceIDs, sys_args);
}


cl_int clGetDeviceInfo(
	cl_device_id device,
	cl_device_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) device;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetDeviceInfo, sys_args);
}


cl_context clCreateContext(
	const cl_context_properties *properties,
	cl_uint num_devices,
	const cl_device_id *devices,
	void (*pfn_notify)(const char *, const void *, size_t , void *),
	void *user_data,
	cl_int *errcode_ret)
{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) properties;
	sys_args[1] = (unsigned int) num_devices;
	sys_args[2] = (unsigned int) devices;
	sys_args[3] = (unsigned int) pfn_notify;
	sys_args[4] = (unsigned int) user_data;
	sys_args[5] = (unsigned int) errcode_ret;
	return (cl_context) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clCreateContext, sys_args);
}


cl_context clCreateContextFromType(
	const cl_context_properties *properties,
	cl_device_type device_type,
	void (*pfn_notify)(const char *, const void *, size_t , void *),
	void *user_data,
	cl_int *errcode_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) properties;
	sys_args[1] = (unsigned int) device_type;
	sys_args[2] = (unsigned int) pfn_notify;
	sys_args[3] = (unsigned int) user_data;
	sys_args[4] = (unsigned int) errcode_ret;
	return (cl_context) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clCreateContextFromType, sys_args);
}


cl_int clRetainContext(
	cl_context context)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) context;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clRetainContext, sys_args);
}


cl_int clReleaseContext(
	cl_context context)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) context;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clReleaseContext, sys_args);
}


cl_int clGetContextInfo(
	cl_context context,
	cl_context_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) context;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetContextInfo, sys_args);
}


cl_command_queue clCreateCommandQueue(
	cl_context context,
	cl_device_id device,
	cl_command_queue_properties properties,
	cl_int *errcode_ret)
{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) context;
	sys_args[1] = (unsigned int) device;
	sys_args[2] = (unsigned int) properties;
	sys_args[3] = (unsigned int) errcode_ret;
	return (cl_command_queue) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clCreateCommandQueue, sys_args);
}


cl_int clRetainCommandQueue(
	cl_command_queue command_queue)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) command_queue;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clRetainCommandQueue, sys_args);
}


cl_int clReleaseCommandQueue(
	cl_command_queue command_queue)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) command_queue;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clReleaseCommandQueue, sys_args);
}


cl_int clGetCommandQueueInfo(
	cl_command_queue command_queue,
	cl_command_queue_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetCommandQueueInfo, sys_args);
}


cl_int clSetCommandQueueProperty(
	cl_command_queue command_queue,
	cl_command_queue_properties properties,
	cl_bool enable,
	cl_command_queue_properties *old_properties)
{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) properties;
	sys_args[2] = (unsigned int) enable;
	sys_args[3] = (unsigned int) old_properties;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clSetCommandQueueProperty, sys_args);
}


cl_mem clCreateBuffer(
	cl_context context,
	cl_mem_flags flags,
	size_t size,
	void *host_ptr,
	cl_int *errcode_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) context;
	sys_args[1] = (unsigned int) flags;
	sys_args[2] = (unsigned int) size;
	sys_args[3] = (unsigned int) host_ptr;
	sys_args[4] = (unsigned int) errcode_ret;
	return (cl_mem) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clCreateBuffer, sys_args);
}


cl_mem clCreateSubBuffer(
	cl_mem buffer,
	cl_mem_flags flags,
	cl_buffer_create_type buffer_create_type,
	const void *buffer_create_info,
	cl_int *errcode_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) buffer;
	sys_args[1] = (unsigned int) flags;
	sys_args[2] = (unsigned int) buffer_create_type;
	sys_args[3] = (unsigned int) buffer_create_info;
	sys_args[4] = (unsigned int) errcode_ret;
	return (cl_mem) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clCreateSubBuffer, sys_args);
}


cl_mem clCreateImage2D(
	cl_context context,
	cl_mem_flags flags,
	const cl_image_format *image_format,
	size_t image_width,
	size_t image_height,
	size_t image_row_pitch,
	void *host_ptr,
	cl_int *errcode_ret)
{
	unsigned int sys_args[8];
	sys_args[0] = (unsigned int) context;
	sys_args[1] = (unsigned int) flags;
	sys_args[2] = (unsigned int) image_format;
	sys_args[3] = (unsigned int) image_width;
	sys_args[4] = (unsigned int) image_height;
	sys_args[5] = (unsigned int) image_row_pitch;
	sys_args[6] = (unsigned int) host_ptr;
	sys_args[7] = (unsigned int) errcode_ret;
	return (cl_mem) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clCreateImage2D, sys_args);
}


cl_mem clCreateImage3D(
	cl_context context,
	cl_mem_flags flags,
	const cl_image_format *image_format,
	size_t image_width,
	size_t image_height,
	size_t image_depth,
	size_t image_row_pitch,
	size_t image_slice_pitch,
	void *host_ptr,
	cl_int *errcode_ret)
{
	unsigned int sys_args[10];
	sys_args[0] = (unsigned int) context;
	sys_args[1] = (unsigned int) flags;
	sys_args[2] = (unsigned int) image_format;
	sys_args[3] = (unsigned int) image_width;
	sys_args[4] = (unsigned int) image_height;
	sys_args[5] = (unsigned int) image_depth;
	sys_args[6] = (unsigned int) image_row_pitch;
	sys_args[7] = (unsigned int) image_slice_pitch;
	sys_args[8] = (unsigned int) host_ptr;
	sys_args[9] = (unsigned int) errcode_ret;
	return (cl_mem) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clCreateImage3D, sys_args);
}


cl_int clRetainMemObject(
	cl_mem memobj)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) memobj;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clRetainMemObject, sys_args);
}


cl_int clReleaseMemObject(
	cl_mem memobj)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) memobj;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clReleaseMemObject, sys_args);
}


cl_int clGetSupportedImageFormats(
	cl_context context,
	cl_mem_flags flags,
	cl_mem_object_type image_type,
	cl_uint num_entries,
	cl_image_format *image_formats,
	cl_uint *num_image_formats)
{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) context;
	sys_args[1] = (unsigned int) flags;
	sys_args[2] = (unsigned int) image_type;
	sys_args[3] = (unsigned int) num_entries;
	sys_args[4] = (unsigned int) image_formats;
	sys_args[5] = (unsigned int) num_image_formats;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetSupportedImageFormats, sys_args);
}


cl_int clGetMemObjectInfo(
	cl_mem memobj,
	cl_mem_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) memobj;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetMemObjectInfo, sys_args);
}


cl_int clGetImageInfo(
	cl_mem image,
	cl_image_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) image;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetImageInfo, sys_args);
}


cl_int clSetMemObjectDestructorCallback(
	cl_mem memobj,
	void (*pfn_notify)(cl_mem memobj , void *user_data),
	void *user_data)
{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) memobj;
	sys_args[1] = (unsigned int) pfn_notify;
	sys_args[2] = (unsigned int) user_data;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clSetMemObjectDestructorCallback, sys_args);
}


cl_sampler clCreateSampler(
	cl_context context,
	cl_bool normalized_coords,
	cl_addressing_mode addressing_mode,
	cl_filter_mode filter_mode,
	cl_int *errcode_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) context;
	sys_args[1] = (unsigned int) normalized_coords;
	sys_args[2] = (unsigned int) addressing_mode;
	sys_args[3] = (unsigned int) filter_mode;
	sys_args[4] = (unsigned int) errcode_ret;
	return (cl_sampler) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clCreateSampler, sys_args);
}


cl_int clRetainSampler(
	cl_sampler sampler)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) sampler;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clRetainSampler, sys_args);
}


cl_int clReleaseSampler(
	cl_sampler sampler)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) sampler;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clReleaseSampler, sys_args);
}


cl_int clGetSamplerInfo(
	cl_sampler sampler,
	cl_sampler_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) sampler;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetSamplerInfo, sys_args);
}


cl_program clCreateProgramWithSource(
	cl_context context,
	cl_uint count,
	const char **strings,
	const size_t *lengths,
	cl_int *errcode_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) context;
	sys_args[1] = (unsigned int) count;
	sys_args[2] = (unsigned int) strings;
	sys_args[3] = (unsigned int) lengths;
	sys_args[4] = (unsigned int) errcode_ret;
	return (cl_program) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clCreateProgramWithSource, sys_args);
}


cl_program clCreateProgramWithBinary(
	cl_context context,
	cl_uint num_devices,
	const cl_device_id *device_list,
	const size_t *lengths,
	const unsigned char **binaries,
	cl_int *binary_status,
	cl_int *errcode_ret)
{
	unsigned int sys_args[7];
	sys_args[0] = (unsigned int) context;
	sys_args[1] = (unsigned int) num_devices;
	sys_args[2] = (unsigned int) device_list;
	sys_args[3] = (unsigned int) lengths;
	sys_args[4] = (unsigned int) binaries;
	sys_args[5] = (unsigned int) binary_status;
	sys_args[6] = (unsigned int) errcode_ret;
	return (cl_program) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clCreateProgramWithBinary, sys_args);
}


cl_int clRetainProgram(
	cl_program program)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) program;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clRetainProgram, sys_args);
}


cl_int clReleaseProgram(
	cl_program program)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) program;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clReleaseProgram, sys_args);
}


cl_int clBuildProgram(
	cl_program program,
	cl_uint num_devices,
	const cl_device_id *device_list,
	const char *options,
	void (*pfn_notify)(cl_program program , void *user_data),
	void *user_data)
{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) program;
	sys_args[1] = (unsigned int) num_devices;
	sys_args[2] = (unsigned int) device_list;
	sys_args[3] = (unsigned int) options;
	sys_args[4] = (unsigned int) pfn_notify;
	sys_args[5] = (unsigned int) user_data;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clBuildProgram, sys_args);
}


cl_int clUnloadCompiler()
{
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clUnloadCompiler);
}


cl_int clGetProgramInfo(
	cl_program program,
	cl_program_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) program;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetProgramInfo, sys_args);
}


cl_int clGetProgramBuildInfo(
	cl_program program,
	cl_device_id device,
	cl_program_build_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) program;
	sys_args[1] = (unsigned int) device;
	sys_args[2] = (unsigned int) param_name;
	sys_args[3] = (unsigned int) param_value_size;
	sys_args[4] = (unsigned int) param_value;
	sys_args[5] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetProgramBuildInfo, sys_args);
}


cl_kernel clCreateKernel(
	cl_program program,
	const char *kernel_name,
	cl_int *errcode_ret)
{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) program;
	sys_args[1] = (unsigned int) kernel_name;
	sys_args[2] = (unsigned int) errcode_ret;
	return (cl_kernel) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clCreateKernel, sys_args);
}


cl_int clCreateKernelsInProgram(
	cl_program program,
	cl_uint num_kernels,
	cl_kernel *kernels,
	cl_uint *num_kernels_ret)
{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) program;
	sys_args[1] = (unsigned int) num_kernels;
	sys_args[2] = (unsigned int) kernels;
	sys_args[3] = (unsigned int) num_kernels_ret;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clCreateKernelsInProgram, sys_args);
}


cl_int clRetainKernel(
	cl_kernel kernel)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) kernel;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clRetainKernel, sys_args);
}


cl_int clReleaseKernel(
	cl_kernel kernel)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) kernel;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clReleaseKernel, sys_args);
}


cl_int clSetKernelArg(
	cl_kernel kernel,
	cl_uint arg_index,
	size_t arg_size,
	const void *arg_value)
{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) kernel;
	sys_args[1] = (unsigned int) arg_index;
	sys_args[2] = (unsigned int) arg_size;
	sys_args[3] = (unsigned int) arg_value;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clSetKernelArg, sys_args);
}


cl_int clGetKernelInfo(
	cl_kernel kernel,
	cl_kernel_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) kernel;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetKernelInfo, sys_args);
}


cl_int clGetKernelWorkGroupInfo(
	cl_kernel kernel,
	cl_device_id device,
	cl_kernel_work_group_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) kernel;
	sys_args[1] = (unsigned int) device;
	sys_args[2] = (unsigned int) param_name;
	sys_args[3] = (unsigned int) param_value_size;
	sys_args[4] = (unsigned int) param_value;
	sys_args[5] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetKernelWorkGroupInfo, sys_args);
}


cl_int clWaitForEvents(
	cl_uint num_events,
	const cl_event *event_list)
{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) num_events;
	sys_args[1] = (unsigned int) event_list;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clWaitForEvents, sys_args);
}


cl_int clGetEventInfo(
	cl_event event,
	cl_event_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) event;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetEventInfo, sys_args);
}


cl_event clCreateUserEvent(
	cl_context context,
	cl_int *errcode_ret)
{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) context;
	sys_args[1] = (unsigned int) errcode_ret;
	return (cl_event) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clCreateUserEvent, sys_args);
}


cl_int clRetainEvent(
	cl_event event)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clRetainEvent, sys_args);
}


cl_int clReleaseEvent(
	cl_event event)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clReleaseEvent, sys_args);
}


cl_int clSetUserEventStatus(
	cl_event event,
	cl_int execution_status)
{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) event;
	sys_args[1] = (unsigned int) execution_status;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clSetUserEventStatus, sys_args);
}


cl_int clSetEventCallback(
	cl_event event,
	cl_int command_exec_callback_type,
	void (*pfn_notify)(cl_event , cl_int , void *),
	void *user_data)
{
	unsigned int sys_args[4];
	sys_args[0] = (unsigned int) event;
	sys_args[1] = (unsigned int) command_exec_callback_type;
	sys_args[2] = (unsigned int) pfn_notify;
	sys_args[3] = (unsigned int) user_data;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clSetEventCallback, sys_args);
}


cl_int clGetEventProfilingInfo(
	cl_event event,
	cl_profiling_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) event;
	sys_args[1] = (unsigned int) param_name;
	sys_args[2] = (unsigned int) param_value_size;
	sys_args[3] = (unsigned int) param_value;
	sys_args[4] = (unsigned int) param_value_size_ret;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetEventProfilingInfo, sys_args);
}


cl_int clFlush(
	cl_command_queue command_queue)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) command_queue;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clFlush, sys_args);
}


cl_int clFinish(
	cl_command_queue command_queue)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) command_queue;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clFinish, sys_args);
}


cl_int clEnqueueReadBuffer(
	cl_command_queue command_queue,
	cl_mem buffer,
	cl_bool blocking_read,
	size_t offset,
	size_t cb,
	void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) buffer;
	sys_args[2] = (unsigned int) blocking_read;
	sys_args[3] = (unsigned int) offset;
	sys_args[4] = (unsigned int) cb;
	sys_args[5] = (unsigned int) ptr;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueReadBuffer, sys_args);
}


cl_int clEnqueueReadBufferRect(
	cl_command_queue command_queue,
	cl_mem buffer,
	cl_bool blocking_read,
	const size_t *buffer_origin,
	const size_t *host_origin,
	const size_t *region,
	size_t buffer_row_pitch,
	size_t buffer_slice_pitch,
	size_t host_row_pitch,
	size_t host_slice_pitch,
	void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[14];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) buffer;
	sys_args[2] = (unsigned int) blocking_read;
	sys_args[3] = (unsigned int) buffer_origin;
	sys_args[4] = (unsigned int) host_origin;
	sys_args[5] = (unsigned int) region;
	sys_args[6] = (unsigned int) buffer_row_pitch;
	sys_args[7] = (unsigned int) buffer_slice_pitch;
	sys_args[8] = (unsigned int) host_row_pitch;
	sys_args[9] = (unsigned int) host_slice_pitch;
	sys_args[10] = (unsigned int) ptr;
	sys_args[11] = (unsigned int) num_events_in_wait_list;
	sys_args[12] = (unsigned int) event_wait_list;
	sys_args[13] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueReadBufferRect, sys_args);
}


cl_int clEnqueueWriteBuffer(
	cl_command_queue command_queue,
	cl_mem buffer,
	cl_bool blocking_write,
	size_t offset,
	size_t cb,
	const void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) buffer;
	sys_args[2] = (unsigned int) blocking_write;
	sys_args[3] = (unsigned int) offset;
	sys_args[4] = (unsigned int) cb;
	sys_args[5] = (unsigned int) ptr;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueWriteBuffer, sys_args);
}


cl_int clEnqueueWriteBufferRect(
	cl_command_queue command_queue,
	cl_mem buffer,
	cl_bool blocking_write,
	const size_t *buffer_origin,
	const size_t *host_origin,
	const size_t *region,
	size_t buffer_row_pitch,
	size_t buffer_slice_pitch,
	size_t host_row_pitch,
	size_t host_slice_pitch,
	const void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[14];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) buffer;
	sys_args[2] = (unsigned int) blocking_write;
	sys_args[3] = (unsigned int) buffer_origin;
	sys_args[4] = (unsigned int) host_origin;
	sys_args[5] = (unsigned int) region;
	sys_args[6] = (unsigned int) buffer_row_pitch;
	sys_args[7] = (unsigned int) buffer_slice_pitch;
	sys_args[8] = (unsigned int) host_row_pitch;
	sys_args[9] = (unsigned int) host_slice_pitch;
	sys_args[10] = (unsigned int) ptr;
	sys_args[11] = (unsigned int) num_events_in_wait_list;
	sys_args[12] = (unsigned int) event_wait_list;
	sys_args[13] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueWriteBufferRect, sys_args);
}


cl_int clEnqueueCopyBuffer(
	cl_command_queue command_queue,
	cl_mem src_buffer,
	cl_mem dst_buffer,
	size_t src_offset,
	size_t dst_offset,
	size_t cb,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) src_buffer;
	sys_args[2] = (unsigned int) dst_buffer;
	sys_args[3] = (unsigned int) src_offset;
	sys_args[4] = (unsigned int) dst_offset;
	sys_args[5] = (unsigned int) cb;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueCopyBuffer, sys_args);
}


cl_int clEnqueueCopyBufferRect(
	cl_command_queue command_queue,
	cl_mem src_buffer,
	cl_mem dst_buffer,
	const size_t *src_origin,
	const size_t *dst_origin,
	const size_t *region,
	size_t src_row_pitch,
	size_t src_slice_pitch,
	size_t dst_row_pitch,
	size_t dst_slice_pitch,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[13];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) src_buffer;
	sys_args[2] = (unsigned int) dst_buffer;
	sys_args[3] = (unsigned int) src_origin;
	sys_args[4] = (unsigned int) dst_origin;
	sys_args[5] = (unsigned int) region;
	sys_args[6] = (unsigned int) src_row_pitch;
	sys_args[7] = (unsigned int) src_slice_pitch;
	sys_args[8] = (unsigned int) dst_row_pitch;
	sys_args[9] = (unsigned int) dst_slice_pitch;
	sys_args[10] = (unsigned int) num_events_in_wait_list;
	sys_args[11] = (unsigned int) event_wait_list;
	sys_args[12] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueCopyBufferRect, sys_args);
}


cl_int clEnqueueReadImage(
	cl_command_queue command_queue,
	cl_mem image,
	cl_bool blocking_read,
	const size_t *origin3,
	const size_t *region3,
	size_t row_pitch,
	size_t slice_pitch,
	void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[11];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) image;
	sys_args[2] = (unsigned int) blocking_read;
	sys_args[3] = (unsigned int) origin3;
	sys_args[4] = (unsigned int) region3;
	sys_args[5] = (unsigned int) row_pitch;
	sys_args[6] = (unsigned int) slice_pitch;
	sys_args[7] = (unsigned int) ptr;
	sys_args[8] = (unsigned int) num_events_in_wait_list;
	sys_args[9] = (unsigned int) event_wait_list;
	sys_args[10] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueReadImage, sys_args);
}


cl_int clEnqueueWriteImage(
	cl_command_queue command_queue,
	cl_mem image,
	cl_bool blocking_write,
	const size_t *origin3,
	const size_t *region3,
	size_t input_row_pitch,
	size_t input_slice_pitch,
	const void *ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[11];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) image;
	sys_args[2] = (unsigned int) blocking_write;
	sys_args[3] = (unsigned int) origin3;
	sys_args[4] = (unsigned int) region3;
	sys_args[5] = (unsigned int) input_row_pitch;
	sys_args[6] = (unsigned int) input_slice_pitch;
	sys_args[7] = (unsigned int) ptr;
	sys_args[8] = (unsigned int) num_events_in_wait_list;
	sys_args[9] = (unsigned int) event_wait_list;
	sys_args[10] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueWriteImage, sys_args);
}


cl_int clEnqueueCopyImage(
	cl_command_queue command_queue,
	cl_mem src_image,
	cl_mem dst_image,
	const size_t *src_origin3,
	const size_t *dst_origin3,
	const size_t *region3,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) src_image;
	sys_args[2] = (unsigned int) dst_image;
	sys_args[3] = (unsigned int) src_origin3;
	sys_args[4] = (unsigned int) dst_origin3;
	sys_args[5] = (unsigned int) region3;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueCopyImage, sys_args);
}


cl_int clEnqueueCopyImageToBuffer(
	cl_command_queue command_queue,
	cl_mem src_image,
	cl_mem dst_buffer,
	const size_t *src_origin3,
	const size_t *region3,
	size_t dst_offset,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) src_image;
	sys_args[2] = (unsigned int) dst_buffer;
	sys_args[3] = (unsigned int) src_origin3;
	sys_args[4] = (unsigned int) region3;
	sys_args[5] = (unsigned int) dst_offset;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueCopyImageToBuffer, sys_args);
}


cl_int clEnqueueCopyBufferToImage(
	cl_command_queue command_queue,
	cl_mem src_buffer,
	cl_mem dst_image,
	size_t src_offset,
	const size_t *dst_origin3,
	const size_t *region3,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) src_buffer;
	sys_args[2] = (unsigned int) dst_image;
	sys_args[3] = (unsigned int) src_offset;
	sys_args[4] = (unsigned int) dst_origin3;
	sys_args[5] = (unsigned int) region3;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueCopyBufferToImage, sys_args);
}


void * clEnqueueMapBuffer(
	cl_command_queue command_queue,
	cl_mem buffer,
	cl_bool blocking_map,
	cl_map_flags map_flags,
	size_t offset,
	size_t cb,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event,
	cl_int *errcode_ret)
{
	unsigned int sys_args[10];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) buffer;
	sys_args[2] = (unsigned int) blocking_map;
	sys_args[3] = (unsigned int) map_flags;
	sys_args[4] = (unsigned int) offset;
	sys_args[5] = (unsigned int) cb;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	sys_args[9] = (unsigned int) errcode_ret;
	return (void *) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueMapBuffer, sys_args);
}


void * clEnqueueMapImage(
	cl_command_queue command_queue,
	cl_mem image,
	cl_bool blocking_map,
	cl_map_flags map_flags,
	const size_t *origin3,
	const size_t *region3,
	size_t *image_row_pitch,
	size_t *image_slice_pitch,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event,
	cl_int *errcode_ret)
{
	unsigned int sys_args[12];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) image;
	sys_args[2] = (unsigned int) blocking_map;
	sys_args[3] = (unsigned int) map_flags;
	sys_args[4] = (unsigned int) origin3;
	sys_args[5] = (unsigned int) region3;
	sys_args[6] = (unsigned int) image_row_pitch;
	sys_args[7] = (unsigned int) image_slice_pitch;
	sys_args[8] = (unsigned int) num_events_in_wait_list;
	sys_args[9] = (unsigned int) event_wait_list;
	sys_args[10] = (unsigned int) event;
	sys_args[11] = (unsigned int) errcode_ret;
	return (void *) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueMapImage, sys_args);
}


cl_int clEnqueueUnmapMemObject(
	cl_command_queue command_queue,
	cl_mem memobj,
	void *mapped_ptr,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[6];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) memobj;
	sys_args[2] = (unsigned int) mapped_ptr;
	sys_args[3] = (unsigned int) num_events_in_wait_list;
	sys_args[4] = (unsigned int) event_wait_list;
	sys_args[5] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueUnmapMemObject, sys_args);
}


cl_int clEnqueueNDRangeKernel(
	cl_command_queue command_queue,
	cl_kernel kernel,
	cl_uint work_dim,
	const size_t *global_work_offset,
	const size_t *global_work_size,
	const size_t *local_work_size,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[9];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) kernel;
	sys_args[2] = (unsigned int) work_dim;
	sys_args[3] = (unsigned int) global_work_offset;
	sys_args[4] = (unsigned int) global_work_size;
	sys_args[5] = (unsigned int) local_work_size;
	sys_args[6] = (unsigned int) num_events_in_wait_list;
	sys_args[7] = (unsigned int) event_wait_list;
	sys_args[8] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueNDRangeKernel, sys_args);
}


cl_int clEnqueueTask(
	cl_command_queue command_queue,
	cl_kernel kernel,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[5];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) kernel;
	sys_args[2] = (unsigned int) num_events_in_wait_list;
	sys_args[3] = (unsigned int) event_wait_list;
	sys_args[4] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueTask, sys_args);
}


cl_int clEnqueueNativeKernel(
	cl_command_queue command_queue,
	void (*user_func)(void *),
	void *args,
	size_t cb_args,
	cl_uint num_mem_objects,
	const cl_mem *mem_list,
	const void **args_mem_loc,
	cl_uint num_events_in_wait_list,
	const cl_event *event_wait_list,
	cl_event *event)
{
	unsigned int sys_args[10];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) user_func;
	sys_args[2] = (unsigned int) args;
	sys_args[3] = (unsigned int) cb_args;
	sys_args[4] = (unsigned int) num_mem_objects;
	sys_args[5] = (unsigned int) mem_list;
	sys_args[6] = (unsigned int) args_mem_loc;
	sys_args[7] = (unsigned int) num_events_in_wait_list;
	sys_args[8] = (unsigned int) event_wait_list;
	sys_args[9] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueNativeKernel, sys_args);
}


cl_int clEnqueueMarker(
	cl_command_queue command_queue,
	cl_event *event)
{
	unsigned int sys_args[2];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) event;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueMarker, sys_args);
}


cl_int clEnqueueWaitForEvents(
	cl_command_queue command_queue,
	cl_uint num_events,
	const cl_event *event_list)
{
	unsigned int sys_args[3];
	sys_args[0] = (unsigned int) command_queue;
	sys_args[1] = (unsigned int) num_events;
	sys_args[2] = (unsigned int) event_list;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueWaitForEvents, sys_args);
}


cl_int clEnqueueBarrier(
	cl_command_queue command_queue)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) command_queue;
	return (cl_int) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clEnqueueBarrier, sys_args);
}


void * clGetExtensionFunctionAddress(
	const char *func_name)
{
	unsigned int sys_args[1];
	sys_args[0] = (unsigned int) func_name;
	return (void *) syscall(SYS_CODE_OPENCL, OPENCL_FUNC_clGetExtensionFunctionAddress, sys_args);
}


