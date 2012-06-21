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

#include <fermi-emu.h>
#include <mem-system.h>
#include <x86-emu.h>

#include <time.h>



/*
 * Macros
 */


#define cuda_debug(stream, ...) ((!strcmp(getenv("LIBM2S_CUDA_DUMP"), "1")) ? \
	fprintf((stream), __VA_ARGS__) : (void) 0)



/*
 * Error Messages
 */


static char *err_frm_cuda_code =
	"\tAn invalid function code was generated by your application in a CUDA system\n"
	"\tcall. Probably, this means that your application is using an incompatible\n"
	"\tversion of the Multi2Sim CUDA driver library ('libm2s-cuda'). Please\n"
	"\trecompile your application and try again.\n";



/*
 * CUDA Driver API Implementation
 */


/* List of CUDA driver calls */

enum frm_cuda_call_t
{
	frm_cuda_call_invalid = 0,
#define FRM_CUDA_DEFINE_CALL(name, code) frm_cuda_call_##name = code,
#include "cuda.dat"
#undef FRM_CUDA_DEFINE_CALL
	frm_cuda_call_count
};

/* List of CUDA driver call names */

char *frm_cuda_call_name[frm_cuda_call_count + 1] =
{
	NULL,
#define FRM_CUDA_DEFINE_CALL(name, code) #name,
#include "cuda.dat"
#undef FRM_CUDA_DEFINE_CALL
	NULL
};

/* Forward declarations of CUDA driver functions */

#define FRM_CUDA_DEFINE_CALL(name, code) static int frm_cuda_func_##name(void);
#include "cuda.dat"
#undef FRM_CUDA_DEFINE_CALL

/* List of CUDA driver functions */

typedef int (*frm_cuda_func_t)(void);
static frm_cuda_func_t frm_cuda_func_table[frm_cuda_call_count + 1] =
{
	NULL,
#define FRM_CUDA_DEFINE_CALL(name, code) frm_cuda_func_##name,
#include "cuda.dat"
#undef FRM_CUDA_DEFINE_CALL
	NULL
};



/*
 * CUDA global functions
 */


int frm_cuda_call(void)
{
	int code;
	int ret;

	/* Function code */
	code = x86_isa_regs->ebx;
	if (code <= frm_cuda_call_invalid || code >= frm_cuda_call_count)
		fatal("%s: invalid CUDA function (code %d).\n%s",
			__FUNCTION__, code, err_frm_cuda_code);

	/* Debug */
	cuda_debug(stdout, "CUDA call '%s' (code %d)\n",
		frm_cuda_call_name[code], code);

	/* Call CUDA function */
	assert(frm_cuda_func_table[code]);
	ret = frm_cuda_func_table[code]();

	/* Return value */
	return ret;
}



/*
 * CUDA call #1 - version
 *
 * @param struct frm_cuda_version_t *version;
 *	Structure where the version of the CUDA runtime implementation will be
 *	dumped. To succeed, the major version should match in the runtime
 *	library (guest) and runtime implementation (host), whereas the minor
 *	version should be equal or higher in the implementation (host).
 *
 *	Features should be added to the CUDA runtime (guest and host) using the
 *	following rules:
 *	1)  If the guest library requires a new feature from the host
 *	    implementation, the feature is added to the host, and the minor
 *	    version is updated to the current Multi2Sim SVN revision both in
 *	    host and guest.
 *          All previous services provided by the host should remain available
 *          and backward-compatible. Executing a newer library on the older
 *          simulator will fail, but an older library on the newer simulator
 *          will succeed.
 *      2)  If a new feature is added that affects older services of the host
 *          implementation breaking backward compatibility, the major version is
 *          increased by 1 in the host and guest code.
 *          Executing a library with a different (lower or higher) major version
 *          than the host implementation will fail.
 *
 * @return
 *	The runtime implementation version is return in argument 'version'.
 *	The return value is always 0.
 */


#define FRM_CUDA_VERSION_MAJOR	1
#define FRM_CUDA_VERSION_MINOR	700

struct frm_cuda_version_t
{
	int major;
	int minor;
};

static int frm_cuda_func_version(void)
{
	struct frm_cuda_version_t version;

	version.major = FRM_CUDA_VERSION_MAJOR;
	version.minor = FRM_CUDA_VERSION_MINOR;

	cuda_debug(stdout, "\tversion.major=%d\n", version.major);
	cuda_debug(stdout, "\tversion.minor=%d\n", version.minor);

	mem_write(x86_isa_mem, x86_isa_regs->ecx, sizeof version, &version);

	return 0;
}



/*
 * CUDA call #2 - cuCtxCreate
 *
 * @param CUcontext *pctx;
 *      Returned context handle of the new context.
 *
 * @param CUdevice dev;
 *      Device to create context on.
 *
 * @return
 *	The return value is always 0 on success.
 */


static int frm_cuda_func_cuCtxCreate(void)
{
	unsigned int args[2];
	unsigned int pctx;
	unsigned int dev;
	struct frm_cuda_context_t *context;

        mem_read(x86_isa_mem, x86_isa_regs->ecx, 2*sizeof(unsigned int), args);
        mem_read(x86_isa_mem, args[0], sizeof(unsigned int), &pctx);
	dev = args[1];

	/* Create context */
	context = frm_cuda_context_create();
        context->device_id = dev;
        mem_write(x86_isa_mem, pctx, sizeof(unsigned int), &context->id);

	return 0;
}



/*
 * CUDA call #3 - cuModuleLoad
 *
 * @param CUmodule *pmod;
 *      Returned module.
 *
 * @param const char *fname;
 *      Filename of module to load.
 *
 * @return
 *	The return value is always 0 on success.
 */


static int frm_cuda_func_cuModuleLoad(void)
{
	unsigned int args[2];
	unsigned int pmod;
	char fname[MAX_STRING_SIZE];
	struct frm_cuda_module_t *module;

        mem_read(x86_isa_mem, x86_isa_regs->ecx, 2*sizeof(unsigned int), args);
        mem_read(x86_isa_mem, args[0], sizeof(unsigned int), &pmod);
	cuda_debug(stdout, "\tpmod=%#8x\n", pmod);
        mem_read(x86_isa_mem, args[1], sizeof(fname), fname);
	cuda_debug(stdout, "\tfname=%s\n", fname);

        module = frm_cuda_module_create();
	module->elf_file = elf_file_create_from_path(fname);
	cuda_debug(stdout, "\tmodule_id=%#x\n", module->id);
        mem_write(x86_isa_mem, pmod, sizeof(unsigned int), &module->id);

	return 0;
}



/*
 * CUDA call #4 - cuModuleGetFunction
 *
 * @param CUfunction *pfunc;
 *      Returned function handle.
 *
 * @param CUmodule mod;
 *      Module to retrieve function from.
 *
 * @param const char *name;
 *      Name of function to retrieve.
 *
 * @return
 *	The return value is always 0 on success.
 */


static int frm_cuda_func_cuModuleGetFunction(void)
{
	unsigned int args[3];
	unsigned int pfunc;
	unsigned int mod_id;
	char function_name[MAX_STRING_SIZE];
	struct frm_cuda_module_t *module;
	struct frm_cuda_function_t *function;
	int i;
	struct elf_section_t *section;

        mem_read(x86_isa_mem, x86_isa_regs->ecx, 3*sizeof(unsigned int), args);
        mem_read(x86_isa_mem, args[0], sizeof(unsigned int), &pfunc);
        mem_read(x86_isa_mem, args[1], sizeof(unsigned int), &mod_id);
        mem_read(x86_isa_mem, args[2], sizeof(function_name), function_name);

	cuda_debug(stdout, "\tfunction_name=%s\n", function_name);

        /* Get module */
        module = frm_cuda_object_get(FRM_CUDA_OBJ_MODULE, mod_id);

        /* Create function */
        function = frm_cuda_function_create();
	function->module_id = module->id;
	strncpy(function->name, function_name, MAX_STRING_SIZE);

        /* Load function */
	for (i = 0; i < list_count(module->elf_file->section_list); ++i)
	{
		section = (struct elf_section_t *)list_get(module->elf_file->section_list, i);
		if (!strncmp(section->name, ".text", 5))
			break;
	}
	if (i == list_count(module->elf_file->section_list))
		fatal(".text section not found!\n");

        mem_write(x86_isa_mem, pfunc, sizeof(unsigned int), &function->id);
	function->function_buffer.ptr = section->buffer.ptr;
	function->function_buffer.size = section->buffer.size;
	function->function_buffer.pos = 0;

	return 0;
}



/*
 * CUDA call #5 - cuLaunchKernel
 *
 * @param CUfunction f;
 *      Kernel to launch.
 *
 * @param unsigned int gridDimX;
 *      Width of grid in blocks.
 *
 * @param unsigned int gridDimY;
 *      Height of grid in blocks.
 *
 * @param unsigned int gridDimZ;
 *      Depth of grid in blocks.
 *
 * @param unsigned int blockDimX;
 *      X dimension of each thread block.
 *
 * @param unsigned int blockDimY;
 *      Y dimension of each thread block.
 *
 * @param unsigned int blockDimZ;
 *      Z dimension of each thread block.
 *
 * @param unsigned int sharedMemBytes;
 *      Dynamic shared-memory size per thread block in bytes.
 *
 * @param CUstream hStream;
 *      Stream identifier.
 *
 * @param void **kernelParams;
 *      Array of pointers to kernel parameters.
 *
 * @param void **extra;
 *      Extra options.
 *
 * @return
 *	The return value is always 0 on success.
 */


static int frm_cuda_func_cuLaunchKernel(void)
{
	unsigned int args[11];
        unsigned int f;
        unsigned int gridDimX;
        unsigned int gridDimY;
        unsigned int gridDimZ;
        unsigned int blockDimX;
        unsigned int blockDimY;
        unsigned int blockDimZ;
        unsigned int sharedMemBytes;
        unsigned int hStream;
        unsigned int kernelParams;
        unsigned int extra;
	struct frm_cuda_function_t *function;

        mem_read(x86_isa_mem, x86_isa_regs->ecx, 11*sizeof(unsigned int), args);
        mem_read(x86_isa_mem, args[0], sizeof(unsigned int), &f);
        gridDimX = args[1];
        gridDimY = args[2];
        gridDimZ = args[3];
        blockDimX = args[4];
        blockDimY = args[5];
        blockDimZ = args[6];
        sharedMemBytes = args[7];
        hStream = args[8];
        kernelParams = args[9];
        extra = args[10];

	cuda_debug(stdout, "\tf=%#x\n", f);
	cuda_debug(stdout, "\tgridDimX=%u\n", gridDimX);
	cuda_debug(stdout, "\tgridDimY=%u\n", gridDimY);
	cuda_debug(stdout, "\tgridDimZ=%u\n", gridDimZ);
	cuda_debug(stdout, "\tblockDimX=%u\n", blockDimX);
	cuda_debug(stdout, "\tblockDimY=%u\n", blockDimY);
	cuda_debug(stdout, "\tblockDimZ=%u\n", blockDimZ);
	cuda_debug(stdout, "\tsharedMemBytes=%u\n", sharedMemBytes);
	cuda_debug(stdout, "\thStream=%u\n", hStream);
	cuda_debug(stdout, "\tkernelParams=%u\n", kernelParams);
	cuda_debug(stdout, "\textra=%u\n", extra);

	function = frm_cuda_object_get(FRM_CUDA_OBJ_FUNCTION, f);
	cuda_debug(stdout, "\tfunction_id=%#x\n", function->id);
	cuda_debug(stdout, "\tfunction_name=%s\n", function->name);

	function->grid = frm_grid_create(function);
	function->global_size3[0] = gridDimX*blockDimX;
	function->global_size3[1] = gridDimY*blockDimY;
	function->global_size3[2] = gridDimZ*blockDimZ;
	function->global_size = function->global_size3[0]*function->global_size3[1]*function->global_size3[2];
	function->local_size3[0] = blockDimX;
	function->local_size3[1] = blockDimY;
	function->local_size3[2] = blockDimZ;
	function->local_size = function->local_size3[0]*function->local_size3[1]*function->local_size3[2];
	function->group_count3[0] = gridDimX;
	function->group_count3[1] = gridDimY;
	function->group_count3[2] = gridDimZ;
	function->group_count = function->group_count3[0]*function->group_count3[1]*function->group_count3[2];

        frm_grid_setup_threads(function->grid);
        frm_grid_setup_args(function->grid);

        frm_grid_set_status(function->grid, frm_grid_pending);

	frm_grid_run(function->grid);
	frm_grid_free(function->grid);

	return 0;
}



/*
 * CUDA call #6 - cuMemAlloc
 *
 * @param CUdeviceptr *dptr;
 *      Returned device pointer.
 *
 * @param size_t bytesize;
 *      Requested allocation size in bytes.
 *
 * @return
 *	The return value is always 0 on success.
 */


static int frm_cuda_func_cuMemAlloc(void)
{
	unsigned int args[2];
	unsigned int pdptr;
	unsigned int bytesize;
	struct frm_cuda_memory_t *mem;

	mem_read(x86_isa_mem, x86_isa_regs->ecx, 2*sizeof(unsigned int), args);
	pdptr = args[0];
        bytesize = args[1];

	cuda_debug(stdout, "\tbytesize= %u\n", bytesize);

        /* Create memory object */
        mem = frm_cuda_memory_create();
        mem->type = 0;  /* FIXME */
        mem->size = bytesize;

        /* Assign position in device global memory */
        mem->device_ptr = frm_emu->global_mem_top;
        frm_emu->global_mem_top += bytesize;

        void *buf = malloc(bytesize);
        mem_write(frm_emu->global_mem, mem->device_ptr, bytesize, buf);
        mem_write(x86_isa_mem, pdptr, 4, &(mem->device_ptr));
        free(buf);

	cuda_debug(stdout, "\tmem->device_ptr = %0#10x\n", mem->device_ptr);

	return 0;
}



/*
 * CUDA call #7 - cuMemcpyHtoD
 *
 * @param CUdeviceptr dstDevice;
 *      Destination device pointer.
 *
 * @param const void *srcHost;
 *      Source host pointer.
 *
 * @param size_t ByteCount;
 *      Size of memory copy in bytes.
 *
 * @return
 *	The return value is always 0 on success.
 */


static int frm_cuda_func_cuMemcpyHtoD(void)
{
	unsigned int args[3];
	unsigned int dstDevice;
	unsigned int srcHost;
	unsigned int ByteCount;

	mem_read(x86_isa_mem, x86_isa_regs->ecx, 3*sizeof(unsigned int), args);
	dstDevice = args[0];
        srcHost = args[1];
        ByteCount = args[2];

	cuda_debug(stdout, "\tdstDevice=%0#10x\n", dstDevice);
	cuda_debug(stdout, "\tsrcHost=%0#10x\n", srcHost);
	cuda_debug(stdout, "\tByteCount=%u\n", ByteCount);

        void *buf = malloc(ByteCount);
        mem_read(x86_isa_mem, srcHost, ByteCount, buf);
        mem_write(frm_emu->global_mem, dstDevice, ByteCount, buf);

	for (int i = 0; i < 4; ++i)
		printf("%f\t", ((float*)buf)[i]);
	printf("\n");

        free(buf);

	return 0;
}



/*
 * CUDA call #8 - cuMemcpyDtoH
 *
 * @param void *dstHost;
 *      Destination host pointer.
 *
 * @param CUdeviceptr srcDevice;
 *      Source device pointer.
 *
 * @param size_t ByteCount;
 *      Size of memory copy in bytes.
 *
 * @return
 *	The return value is always 0 on success.
 */


static int frm_cuda_func_cuMemcpyDtoH(void)
{
	unsigned int args[3];
	unsigned int dstHost;
	unsigned int srcDevice;
	unsigned int ByteCount;

	mem_read(x86_isa_mem, x86_isa_regs->ecx, 3*sizeof(unsigned int), args);
	dstHost = args[0];
        srcDevice = args[1];
        ByteCount = args[2];

	cuda_debug(stdout, "\tdstHost=%0#10x\n", dstHost);
	cuda_debug(stdout, "\tsrcDevice=%0#10x\n", srcDevice);
	cuda_debug(stdout, "\tByteCount=%u\n", ByteCount);

        void *buf = malloc(ByteCount);
        mem_read(frm_emu->global_mem, srcDevice, ByteCount, buf);
        mem_write(x86_isa_mem, dstHost, ByteCount, buf);

	for (int i = 0; i < 4; ++i)
		printf("%f\t", ((float*)buf)[i]);
	printf("\n");

        free(buf);

	return 0;
}


static int frm_cuda_func_cuMemFree(void)
{

	return 0;
}


static int frm_cuda_func_cuCtxDetach(void)
{

	return 0;
}

