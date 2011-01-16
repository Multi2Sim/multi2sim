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

#include <gpuguest.h>
#include <stdlib.h>
#include <misc.h>
#include <string.h>
#include <unistd.h>
#include <debug.h>


extern const char _binary_guest_code_libguest_opencl_so_start;
extern const void _binary_guest_code_libguest_opencl_so_size;

const void *libguest_opencl_so_buf = &_binary_guest_code_libguest_opencl_so_start;
const size_t libguest_opencl_so_size = (size_t) &_binary_guest_code_libguest_opencl_so_size;


/* Create a temporary file where the guest OpenCL library is extracted.
 * The file path is return in 'ret_path', which has a size of 'ret_path_size'
 * bytes. */
void gpu_guest_extract_opencl(char *ret_path, int ret_path_size)
{
	char path[MAX_STRING_SIZE];
	int fd;
	size_t count;

	/* Create temporary file */
	strcpy(path, "/tmp/m2s.XXXXXX");
	if ((fd = mkstemp(path)) == -1)
		fatal("%s: couldn't create temporary file", __FUNCTION__);
	
	/* Return file name */
	if (ret_path)
		strncpy(ret_path, path, ret_path_size);
	
	/* Write guest OpenCL library */
	count = write(fd, libguest_opencl_so_buf, libguest_opencl_so_size);
	if (count != libguest_opencl_so_size)
		fatal("%s: couldn't dump guest OpenCL library", __FUNCTION__);
	close(fd);
}


