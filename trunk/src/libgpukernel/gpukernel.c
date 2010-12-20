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

#include <gpukernel-local.h>
#include <assert.h>
#include <debug.h>
#include <lnlist.h>
#include <stdlib.h>
#include <m2skernel.h>
#include <gpudisasm.h>
#include <options.h>


/* Global variables */

struct gk_t *gk;
char *gk_opencl_binary_name = "";



/* Initialize GPU kernel */
void gk_init()
{
	struct opencl_device_t *device;

	/* Debug categories */
	opencl_debug_category = debug_new_category();

	/* Initialize kernel */
	gk = calloc(1, sizeof(struct gk_t));
	gk->const_mem = mem_create();
	gk->const_mem->safe = 0;
	gk->global_mem = mem_create();
	gk->global_mem->safe = 0;

	/* Initialize disassembler (decoding tables...) */
	amd_disasm_init();

	/* Initialize ISA (instruction execution tables...) */
	gpu_isa_init();

	/* Create platform and device */
	opencl_object_list = lnlist_create();
	opencl_platform = opencl_platform_create();
	device = opencl_device_create();
}


/* Finalize GPU kernel */
void gk_done()
{
	opencl_object_free_all();
	lnlist_free(opencl_object_list);

	/* Finalize disassembler */
	amd_disasm_done();

	/* Finalize ISA */
	gpu_isa_done();

	/* Finalize GPU kernel */
	mem_free(gk->const_mem);
	mem_free(gk->global_mem);
	free(gk);

	/* Mhandle */
	mhandle_done();//////// FIXME
}


/* Register options */
void gk_reg_options()
{
	opt_reg_string("-opencl:binary", "Pre-compiled binary for OpenCL applications",
		&gk_opencl_binary_name);
}


/* If 'fullpath' points to the original OpenCL library, redirect it to 'm2s-opencl.so'
 * in the same path. */
void gk_libopencl_redirect(char *fullpath, int size)
{
	char fullpath_original[MAX_PATH_SIZE];
	char buf[MAX_PATH_SIZE];
	int length;
	FILE *f;

	/* Get path length */
	strncpy(fullpath_original, fullpath, MAX_PATH_SIZE);
	length = strlen(fullpath);

	/* Detect an attempt to open 'libm2s-opencl' and record it */
	if (length >= 17 && !strcmp(fullpath + length - 17, "/libm2s-opencl.so")) {
		f = fopen(fullpath, "r");
		if (f) {
			fclose(f);
			isa_ctx->libopencl_open_attempt = 0;
		} else
			isa_ctx->libopencl_open_attempt = 1;
	}

	/* Translate libOpenCL -> libm2s-opencl */
	if (length >= 13 && !strcmp(fullpath + length - 13, "/libOpenCL.so")) {
		
		/* Translate name */
		fullpath[length - 13] = '\0';
		snprintf(buf, MAX_STRING_SIZE, "%s/libm2s-opencl.so", fullpath);
		strncpy(fullpath, buf, size);

		/* Check if this attempt if successful */
		f = fopen(fullpath, "r");
		if (f) {
			fclose(f);
			warning("path '%s' has been redirected to '%s'\n"
				"\tYour application is trying to access the default OpenCL library, which is being\n"
				"\tredirected by Multi2Sim to its own provided library. Though this should work,\n"
				"\tthe safest way to simulate an OpenCL program is by linking it initially with\n"
				"\t'libm2s-opencl.so'. See the Multi2Sim Guide for further details (www.multi2sim.org).\n",
				fullpath_original, fullpath);
			isa_ctx->libopencl_open_attempt = 0;
		} else
			isa_ctx->libopencl_open_attempt = 1;
	}
}


/* Dump a warning about failed attempts of context to access OpenCL library */
void gk_libopencl_failed(int pid)
{
	warning("context %d finished after failing to access OpenCL library.\n"
		"\tMulti2Sim has detected several attempts to access 'libm2s-opencl.so' by your\n"
		"\tapplication's dynamic linker. Please, make sure that this file is available\n"
		"\teither in any shared library path, in the current working directory, or in\n"
		"\tany directory pointed by the environment variable LD_LIBRARY_PATH. See the\n"
		"\tMulti2Sim Guide for further details (www.multi2sim.org).\n",
		pid);
}




/*
 * GPU Thread
 */

struct gpu_thread_t *gpu_thread_create()
{
	struct gpu_thread_t *thread;
	thread = calloc(1, sizeof(struct gpu_thread_t));
	return thread;
}


void gpu_thread_free(struct gpu_thread_t *thread)
{
	free(thread);
}

