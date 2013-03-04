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

#include <assert.h>

#include <arch/southern-islands/emu/emu.h>
#include <arch/x86/emu/context.h>
#include <arch/x86/emu/regs.h>
#include <lib/util/debug.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "opencl.h"


static char *opencl_err_abi_call =
	"\tAn invalid function code was generated by the your application as an\n"
	"\targument of a system call reserved for the Multi2Sim OpenCL Runtime\n"
	"\tlibrary. Please recompile your application and try again.\n";


/* List of OpenCL Runtime calls */
enum opencl_abi_call_t
{
	opencl_abi_invalid = 0,
#define OPENCL_ABI_CALL(name, code) opencl_abi_##name = code,
#include "opencl.dat"
#undef OPENCL_ABI_CALL
	opencl_abi_call_count
};


/* List of OpenCL ABI call names */
char *opencl_abi_call_name[opencl_abi_call_count + 1] =
{
	NULL,
#define OPENCL_ABI_CALL(name, code) #name,
#include "opencl.dat"
#undef OPENCL_ABI_CALL
	NULL
};

/* Forward declarations of OpenCL Runtime functions */
#define OPENCL_ABI_CALL(name, code) \
	static int opencl_abi_##name##_impl(struct x86_ctx_t *ctx);
#include "opencl.dat"
#undef OPENCL_ABI_CALL


/* List of OpenCL Runtime functions */
typedef int (*opencl_abi_call_t)(struct x86_ctx_t *ctx);
static opencl_abi_call_t opencl_abi_call_table[opencl_abi_call_count + 1] =
{
	NULL,
#define OPENCL_ABI_CALL(name, code) opencl_abi_##name##_impl,
#include "opencl.dat"
#undef OPENCL_ABI_CALL
	NULL
};


/* Debug */
int opencl_debug_category;



int opencl_abi_call(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	/* Variables */
	int code;
	int ret;

	/* Function code */
	code = regs->ebx;
	if (code <= opencl_abi_invalid || code >= opencl_abi_call_count)
		fatal("%s: invalid OpenCL ABI call (code %d).\n%s",
			__FUNCTION__, code, opencl_err_abi_call);

	/* Debug */
	opencl_debug("OpenCL ABI call '%s' (code %d)\n",
		opencl_abi_call_name[code], code);

	/* Call OpenCL Runtime function */
	assert(opencl_abi_call_table[code]);
	ret = opencl_abi_call_table[code](ctx);

	/* Return value */
	return ret;
}




/*
 * OpenCL ABI call #1 - init
 *
 * Returns the version of the OpenCL driver.
 *
 * @param struct opencl_vection_t *version
 *
 * 	The driver returns its version information in this argument, with a
 * 	data structure formed of two integer fields:
 *
 * 	struct opencl_version_t
 * 	{
 * 		int major;
 * 		int minor;
 * 	};
 *
 * @return
 *	The function always returns 0.
 */

/* NOTE: when modifying the values of these two macros, the same values should
 * be reflected in 'runtime/opencl/platform.c'. */
#define OPENCL_VERSION_MAJOR  1
#define OPENCL_VERSION_MINOR  1403

struct opencl_version_t
{
	int major;
	int minor;
};

static int opencl_abi_init_impl(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int version_ptr;
	struct opencl_version_t version;

	/* Arguments */
	version_ptr = regs->ecx;
	opencl_debug("\tversion_ptr=0x%x\n", version_ptr);

	/* Return version */
	assert(sizeof(struct opencl_version_t) == 8);
	version.major = OPENCL_VERSION_MAJOR;
	version.minor = OPENCL_VERSION_MINOR;
	mem_write(mem, version_ptr, sizeof version, &version);
	opencl_debug("\tMulti2Sim OpenCL implementation in host: v. %d.%d.\n",
		OPENCL_VERSION_MAJOR, OPENCL_VERSION_MINOR);
	opencl_debug("\tMulti2Sim OpenCL Runtime in guest: v. %d.%d.\n",
		version.major, version.minor);

	/* Return success */
	return 0;
}




/*
 * OpenCL ABI call #2 - si_mem_alloc
 *
 * Allocates memory in the Southern Islands device.
 *
 * @param int device_id
 *
 *	Device ID return by ABI call 'get_device_id'.
 *
 * @param unsigned int size
 *
 * 	Number of bytes to allocate.
 *
 * @return void *
 *
 *	The function returns a pointer in the device memory space. This pointer
 *	should not be dereferenced in the runtime, but instead passed to other
 *	ABI calls taking device pointers as input arguments.
 */

static int opencl_abi_si_mem_alloc_impl(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int size;
	unsigned int device_ptr;

	/* Arguments */
	size = regs->ecx;
	opencl_debug("\tsize = %u\n", size);

	/* For now, memory allocation in device memory is done by just incrementing
	 * a pointer to the top of the global memory space. Since memory deallocation
	 * is not implemented, "holes" in the memory space are not considered. */
	device_ptr = si_emu->global_mem_top;
	si_emu->global_mem_top += size;
	opencl_debug("\t%d bytes of device memory allocated at 0x%x\n",
			size, device_ptr);

	/* Return device pointer */
	return device_ptr;
}




/*
 * OpenCL ABI call #3 - si_mem_read
 *
 * Read memory from Southern Islands device into host memory space.
 *
 * @param void *host_ptr
 *
 * 	Destination pointer in host memory space.
 *
 * @param void *device_ptr
 *
 * 	Source pointer in device memory space.
 *
 * @param unsigned int size
 *
 * 	Number of bytes to read.
 *
 * @return void
 *
 *	The function does not have any return value.
 */

static int opencl_abi_si_mem_read_impl(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int host_ptr;
	unsigned int device_ptr;
	unsigned int size;

	void *buf;

	/* Arguments */
	host_ptr = regs->ecx;
	device_ptr = regs->edx;
	size = regs->esi;
	opencl_debug("\thost_ptr = 0x%x, device_ptr = 0x%x, size = %d bytes\n",
			host_ptr, device_ptr, size);

	/* Check memory range */
	if (device_ptr + size > si_emu->global_mem_top)
		fatal("%s: accessing device memory not allocated",
				__FUNCTION__);

	/* Read memory from device to host */
	buf = xmalloc(size);
	mem_read(si_emu->global_mem, device_ptr, size, buf);
	mem_write(mem, host_ptr, size, buf);
	free(buf);

	/* Return */
	return 0;
}




/*
 * OpenCL ABI call #4 - si_mem_write
 *
 * Write memory from host into Southern Islands device.
 *
 * @param void *device_ptr
 *
 * 	Destination pointer in device memory.
 *
 * @param void *host_ptr
 *
 * 	Source pointer in host memory.
 *
 * @param unsigned int size
 *
 * 	Number of bytes to read.
 *
 * @return void
 *
 *	The function does not have any return value.
 */

static int opencl_abi_si_mem_write_impl(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int device_ptr;
	unsigned int host_ptr;
	unsigned int size;

	void *buf;

	/* Arguments */
	device_ptr = regs->ecx;
	host_ptr = regs->edx;
	size = regs->esi;
	opencl_debug("\tdevice_ptr = 0x%x, host_ptr = 0x%x, size = %d bytes\n",
			device_ptr, host_ptr, size);

	/* Check memory range */
	if (device_ptr + size > si_emu->global_mem_top)
		fatal("%s: accessing device memory not allocated",
				__FUNCTION__);

	/* Write memory from host to device */
	buf = xmalloc(size);
	mem_read(mem, host_ptr, size, buf);
	mem_write(si_emu->global_mem, device_ptr, size, buf);
	free(buf);

	/* Return */
	return 0;
}




/*
 * OpenCL ABI call #5 - si_mem_copy
 *
 * Copy memory across two different regions of the Southern Islands device
 * memory space.
 *
 * @param void *dest_ptr
 *
 * 	Destination pointer in device memory.
 *
 * @param void *src_ptr
 *
 * 	Source pointer in device memory.
 *
 * @param unsigned int size
 *
 * 	Number of bytes to read.
 *
 * @return void
 *
 *	The function does not have any return value.
 */

static int opencl_abi_si_mem_copy_impl(struct x86_ctx_t *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int dest_ptr;
	unsigned int src_ptr;
	unsigned int size;

	void *buf;

	/* Arguments */
	dest_ptr = regs->ecx;
	src_ptr = regs->edx;
	size = regs->esi;
	opencl_debug("\tdest_ptr = 0x%x, src_ptr = 0x%x, size = %d bytes\n",
			dest_ptr, src_ptr, size);

	/* Check memory range */
	if (src_ptr + size > si_emu->global_mem_top ||
			dest_ptr + size > si_emu->global_mem_top)
		fatal("%s: accessing device memory not allocated",
				__FUNCTION__);

	/* Write memory from host to device */
	buf = xmalloc(size);
	mem_read(si_emu->global_mem, src_ptr, size, buf);
	mem_write(si_emu->global_mem, dest_ptr, size, buf);
	free(buf);

	/* Return */
	return 0;
}
