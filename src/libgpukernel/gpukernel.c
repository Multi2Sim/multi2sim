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
	gpu_isa_debug_category = debug_new_category();

	/* Initialize kernel */
	gk = calloc(1, sizeof(struct gk_t));
	gk->const_mem = mem_create();
	gk->const_mem->safe = 0;
	gk->global_mem = mem_create();
	gk->global_mem->safe = 0;
	gk->global_mem_top = GK_GLOBAL_MEM_BASE;
	gk->local_mem_top = GK_LOCAL_MEM_BASE;

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
 * GPU Warp
 */

struct gpu_warp_t *gpu_warp_create(int thread_count, int global_id)
{
	struct gpu_warp_t *warp;
	warp = calloc(1, sizeof(struct gpu_warp_t));
	warp->thread_count = thread_count;
	warp->global_id = global_id;
	warp->active_stack = bit_map_create(GPU_MAX_STACK_SIZE * thread_count);
	warp->pred = bit_map_create(thread_count);
	snprintf(warp->name, sizeof(warp->name), "warp[%d-%d]",
		global_id, global_id + thread_count - 1);
	return warp;
}


void gpu_warp_free(struct gpu_warp_t *warp)
{
	bit_map_free(warp->active_stack);
	bit_map_free(warp->pred);
	free(warp);
}


void gpu_warp_stack_push(struct gpu_warp_t *warp)
{
	if (warp->stack_top == GPU_MAX_STACK_SIZE - 1)
		fatal("%s: stack overflow", gpu_isa_inst->info->name);
	warp->stack_top++;
	bit_map_copy(warp->active_stack, warp->stack_top * warp->thread_count,
		warp->active_stack, (warp->stack_top - 1) * warp->thread_count,
		warp->thread_count);
	gpu_isa_debug("  %s:push", warp->name);
}


void gpu_warp_stack_pop(struct gpu_warp_t *warp, int count)
{
	if (!count)
		return;
	if (warp->stack_top < count)
		fatal("%s: stack underflow", gpu_isa_inst->info->name);
	warp->stack_top -= count;
	if (debug_status(gpu_isa_debug_category)) {
		gpu_isa_debug("  %s:pop(%d),act=", warp->name, count);
		bit_map_dump(warp->active_stack, warp->stack_top * warp->thread_count,
			warp->thread_count, debug_file(gpu_isa_debug_category));
	}
}




/*
 * GPU Thread
 */

struct gpu_thread_t *gpu_thread_create()
{
	struct gpu_thread_t *thread;
	thread = calloc(1, sizeof(struct gpu_thread_t));
	thread->write_task_list = lnlist_create();
	thread->lds_oqa = list_create(5);
	thread->lds_oqb = list_create(5);
	return thread;
}


void gpu_thread_free(struct gpu_thread_t *thread)
{
	/* Empty LDS output queues */
	while (list_count(thread->lds_oqa))
		free(list_dequeue(thread->lds_oqa));
	while (list_count(thread->lds_oqb))
		free(list_dequeue(thread->lds_oqb));
	list_free(thread->lds_oqa);
	list_free(thread->lds_oqb);
	lnlist_free(thread->write_task_list);

	/* Free thread */
	free(thread);
}


void gpu_thread_set_active(struct gpu_thread_t *thread, int active)
{
	struct gpu_warp_t *warp = thread->warp;
	assert(thread->global_id >= warp->global_id &&
		thread->global_id < warp->global_id + warp->thread_count);
	assert(thread->warp_id < warp->thread_count);
	bit_map_set(warp->active_stack, warp->stack_top * warp->thread_count
		+ thread->warp_id, 1, !!active);
}


int gpu_thread_get_active(struct gpu_thread_t *thread)
{
	struct gpu_warp_t *warp = thread->warp;
	assert(thread->global_id >= warp->global_id &&
		thread->global_id < warp->global_id + warp->thread_count);
	assert(thread->warp_id < warp->thread_count);
	return bit_map_get(warp->active_stack, warp->stack_top * warp->thread_count
		+ thread->warp_id, 1);
}


void gpu_thread_set_pred(struct gpu_thread_t *thread, int pred)
{
	struct gpu_warp_t *warp = thread->warp;
	assert(thread->global_id >= warp->global_id &&
		thread->global_id < warp->global_id + warp->thread_count);
	assert(thread->warp_id < warp->thread_count);
	bit_map_set(warp->pred, thread->warp_id, 1, !!pred);
}


int gpu_thread_get_pred(struct gpu_thread_t *thread)
{
	struct gpu_warp_t *warp = thread->warp;
	assert(thread->global_id >= warp->global_id &&
		thread->global_id < warp->global_id + warp->thread_count);
	assert(thread->warp_id < warp->thread_count);
	return bit_map_get(warp->pred, thread->warp_id, 1);
}

