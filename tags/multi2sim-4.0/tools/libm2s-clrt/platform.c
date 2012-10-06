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

#include <unistd.h>
#include <stdio.h>

#include <m2s-clrt.h>


static char *m2s_clrt_err_version =
	"\tYour OpenCL program is using a version of the Multi2Sim Runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tMulti2Sim OpenCL Runtime library ('libm2s-clrt').\n";


struct m2s_clrt_version_t
{
	int major;
	int minor;
};

struct _cl_platform_id *m2s_platform = NULL;
struct _cl_device_id *m2s_device = NULL;

/*
 * Private Functions
 */

cl_int clGetPlatformIDs(
	cl_uint num_entries,
	cl_platform_id *platforms,
	cl_uint *num_platforms)
{
	struct m2s_clrt_version_t version;
	int ret;

	/* Debug */
	m2s_clrt_debug("call '%s'", __FUNCTION__);
	m2s_clrt_debug("\tnum_entries = %d", num_entries);
	m2s_clrt_debug("\tplatforms = %p", platforms);
	m2s_clrt_debug("\tnum_platforms = %p", num_platforms);

	/* It can be assumed that this is the first OpenCL function called by
	 * the host program. It is checked here whether we're running in native
	 * or simulation mode. If it's simulation mode, Multi2Sim's version is
	 * checked for compatibility with the runtime library version. */
	ret = syscall(M2S_CLRT_SYS_CODE, m2s_clrt_call_init, &version);

	/* If the system call returns error, we are in native mode. */
	if (ret == -1)
		m2s_clrt_native_mode = 1;

	/* On simulation mode, check Multi2sim version and Multi2Sim OpenCL
	 * Runtime version compatibility. */
	if (!m2s_clrt_native_mode)
		if (version.major != M2S_CLRT_VERSION_MAJOR
				|| version.minor < M2S_CLRT_VERSION_MINOR)
			fatal("incompatible Multi2Sim Runtime version.\n"
				"\tRuntime library v. %d.%d / "
				"Host implementation v. %d.%d.\n%s",
				M2S_CLRT_VERSION_MAJOR, M2S_CLRT_VERSION_MINOR,
				version.major, version.minor, m2s_clrt_err_version);

	/* Create the platform object if it has not already been made */
	if (m2s_platform == NULL)
	{
		m2s_platform = (struct _cl_platform_id *) malloc(sizeof (struct _cl_platform_id));
		if (m2s_platform == NULL)
			fatal("%s: out of memory", __FUNCTION__);
		m2s_platform->empty = 0;
	}	

	

	/* Implementation */

	/* If an array is passed in, it must have a corresponding length
	 * and the client must either want a count or a list of platforms */
	if ((num_entries == 0 && platforms != NULL) || (num_entries != 0 && platforms == NULL) || (num_platforms == 0 && platforms == NULL))
		return CL_INVALID_VALUE;

	/* If they just want to know how many platforms there are, tell them */
	else if (num_entries == 0 && num_platforms != NULL)
	{
		*num_platforms = 1;
		return CL_SUCCESS;
	}

	/* The client wants the platform itself.  Also return the number of platforms inserted if they asked for it */
	else
	{
		if (num_platforms != NULL)
			*num_platforms = 1;
		*platforms = m2s_platform;
	
		return CL_SUCCESS;
	}
}

/*
 * Public Functions
 */

cl_int clGetPlatformInfo(
	cl_platform_id platform,
	cl_platform_info param_name,
	size_t param_value_size,
	void *param_value,
	size_t *param_value_size_ret)
{
	__M2S_CLRT_NOT_IMPL__
	return 0;
}

