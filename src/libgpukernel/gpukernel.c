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

#include <gpukernel.h>
#include <gpudisasm.h>
#include <cpukernel.h>
#include <options.h>
#include <hash.h>


/* Global variables */

struct gk_t *gk;
char *gk_opencl_binary_name = "";
char *gk_report_file_name = "";
FILE *gk_report_file = NULL;
int gk_kernel_execution_count = 0;


/* Architectural parameters introduced in GPU emulator */
int gpu_wavefront_size = 64;
int gpu_max_work_group_size = 256;



/* Initialize GPU kernel */
void gk_init()
{
	struct opencl_device_t *device;

	/* Debug categories */
	opencl_debug_category = debug_new_category();
	gpu_isa_debug_category = debug_new_category();

	/* Open report file */
	if (gk_report_file_name[0]) {
		gk_report_file = open_write(gk_report_file_name);
		if (!gk_report_file)
			fatal("%s: cannot open GPU report file ", gk_report_file_name);
	}

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
	/* GPU report */
	if (gk_report_file)
		fclose(gk_report_file);

	/* Free OpenCL objects */
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
}


/* Register options */
void gk_reg_options()
{
	opt_reg_string("-opencl:binary", "Pre-compiled binary for OpenCL applications",
		&gk_opencl_binary_name);
	opt_reg_string("-report:gpu", "Report for GPU statistics",
		&gk_report_file_name);
}


/* If 'fullpath' points to the original OpenCL library, redirect it to 'm2s-opencl.so'
 * in the same path. */
void gk_libopencl_redirect(char *fullpath, int size)
{
	char fullpath_original[MAX_PATH_SIZE];
	char buf[MAX_PATH_SIZE];
	char *relpath, *filename;
	int length;
	FILE *f;

	/* Get path length */
	strncpy(fullpath_original, fullpath, MAX_PATH_SIZE);
	length = strlen(fullpath);
	relpath = rindex(fullpath, '/');
	assert(relpath && *relpath == '/');
	filename = relpath + 1;

	/* Detect an attempt to open 'libm2s-opencl' and record it */
	if (!strcmp(filename, "libm2s-opencl.so")) {
		f = fopen(fullpath, "r");
		if (f) {
			fclose(f);
			isa_ctx->libopencl_open_attempt = 0;
		} else
			isa_ctx->libopencl_open_attempt = 1;
	}

	/* Translate libOpenCL -> libm2s-opencl */
	if (!strcmp(filename, "libOpenCL.so") || !strncmp(filename, "libOpenCL.so.", 13)) {
		
		/* Translate name in same path */
		fullpath[length - 13] = '\0';
		snprintf(buf, MAX_STRING_SIZE, "%s/libm2s-opencl.so", fullpath);
		strncpy(fullpath, buf, size);
		f = fopen(fullpath, "r");

		/* If attempt failed, translate name into current working directory */
		if (!f) {
			if (!getcwd(buf, MAX_PATH_SIZE))
				fatal("%s: cannot get current directory", __FUNCTION__);
			sprintf(fullpath, "%s/libm2s-opencl.so", buf);
			f = fopen(fullpath, "r");
		}

		/* End of attempts */
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
 * GPU Work-Group
 */

static uint64_t work_group_id;


struct gpu_work_group_t *gpu_work_group_create()
{
	struct gpu_work_group_t *work_group;

	work_group = calloc(1, sizeof(struct gpu_work_group_t));
	work_group->work_group_id = work_group_id++;
	return work_group;
}


void gpu_work_group_free(struct gpu_work_group_t *work_group)
{
	free(work_group);
}




/*
 * GPU wavefront
 */

static uint64_t wavefront_id;


struct gpu_wavefront_t *gpu_wavefront_create()
{
	struct gpu_wavefront_t *wavefront;

	wavefront = calloc(1, sizeof(struct gpu_wavefront_t));
	wavefront->wavefront_id = wavefront_id++;

	wavefront->active_stack = bit_map_create(GPU_MAX_STACK_SIZE * gpu_wavefront_size);
	wavefront->pred = bit_map_create(gpu_wavefront_size);
	return wavefront;
}


void gpu_wavefront_free(struct gpu_wavefront_t *wavefront)
{
	/* Free wavefront */
	bit_map_free(wavefront->active_stack);
	bit_map_free(wavefront->pred);
	free(wavefront);
}


/* Comparison function to sort list */
static int gpu_wavefront_divergence_compare(const void *elem1, const void *elem2)
{
	int count1 = * (int *) elem1;
	int count2 = * (int *) elem2;
	
	if (count1 < count2)
		return 1;
	else if (count1 > count2)
		return -1;
	return 0;
}


void gpu_wavefront_divergence_dump(struct gpu_wavefront_t *wavefront, FILE *f)
{
	struct gpu_work_item_t *work_item;
	struct elem_t {
		int count;  /* 1st field hardcoded for comparison */
		int list_index;
		uint32_t branch_digest;
	};
	struct elem_t *elem;
	struct list_t *list;
	struct hashtable_t *ht;
	char str[10];
	int i, j;

	/* Create list and hash table */
	list = list_create(20);
	ht = hashtable_create(20, 1);

	/* Create one 'elem' for each work_item with a different branch digest, and
	 * store it into the hash table and list. */
	for (i = 0; i < wavefront->work_item_count; i++) {
		work_item = wavefront->work_items[i];
		sprintf(str, "%08x", work_item->branch_digest);
		elem = hashtable_get(ht, str);
		if (!elem) {
			elem = calloc(1, sizeof(struct elem_t));
			hashtable_insert(ht, str, elem);
			elem->list_index = list_count(list);
			elem->branch_digest = work_item->branch_digest;
			list_add(list, elem);
		}
		elem->count++;
	}

	/* Sort divergence groups as per size */
	list_sort(list, gpu_wavefront_divergence_compare);
	fprintf(f, "work_itemDivergenceGroups = %d\n", list_count(list));
	
	/* Dump size of groups with */
	fprintf(f, "work_itemDivergenceGroupsSize =");
	for (i = 0; i < list_count(list); i++) {
		elem = list_get(list, i);
		fprintf(f, " %d", elem->count);
	}
	fprintf(f, "\n\n");

	/* Dump work_item ids contained in each work_item divergence group */
	for (i = 0; i < list_count(list); i++) {
		elem = list_get(list, i);
		fprintf(f, "work_itemDivergenceGroup[%d] =", i);

		for (j = 0; j < wavefront->work_item_count; j++) {
			int first, last;
			first = wavefront->work_items[j]->branch_digest == elem->branch_digest &&
				(j == 0 || wavefront->work_items[j - 1]->branch_digest != elem->branch_digest);
			last = wavefront->work_items[j]->branch_digest == elem->branch_digest &&
				(j == wavefront->work_item_count - 1 || wavefront->work_items[j + 1]->branch_digest != elem->branch_digest);
			if (first)
				fprintf(f, " %d", j);
			else if (last)
				fprintf(f, "-%d", j);
		}
		fprintf(f, "\n");
	}
	fprintf(f, "\n");

	/* Free 'elem's and structures */
	for (i = 0; i < list_count(list); i++)
		free(list_get(list, i));
	list_free(list);
	hashtable_free(ht);
}


void gpu_wavefront_dump(struct gpu_wavefront_t *wavefront, FILE *f)
{
	int i;
	double emu_time;

	if (!f)
		return;
	
	/* Dump wavefront statistics in GPU report */
	fprintf(f, "[ wavefront %lld ]\n\n", (long long) wavefront->wavefront_id);

	fprintf(f, "KernelExecution = %d\n", gk_kernel_execution_count - 1);
	fprintf(f, "Name = %s\n", wavefront->name);
	fprintf(f, "Global_Id = %d\n", wavefront->global_id);
	fprintf(f, "work_item_Count = %d\n", wavefront->work_item_count);
	fprintf(f, "\n");

	emu_time = (double) (wavefront->emu_time_end = wavefront->emu_time_start) / 1e6;
	fprintf(f, "Emu_Inst_Count = %lld\n", (long long) wavefront->emu_inst_count);
	fprintf(f, "Emu_Time = %.2f\n", emu_time);
	fprintf(f, "Emu_Inst_Per_Sec = %.2f\n", (double) wavefront->emu_inst_count / emu_time);
	fprintf(f, "\n");

	fprintf(f, "Inst_Count = %lld\n", (long long) wavefront->inst_count);
	fprintf(f, "Global_Mem_Inst_Count = %lld\n", (long long) wavefront->global_mem_inst_count);
	fprintf(f, "Local_Mem_Inst_Count = %lld\n", (long long) wavefront->local_mem_inst_count);
	fprintf(f, "\n");

	fprintf(f, "CF_Inst_Count = %lld\n", (long long) wavefront->cf_inst_count);
	fprintf(f, "CF_Inst_Global_Mem_Write_Count = %lld\n", (long long) wavefront->cf_inst_global_mem_write_count);
	fprintf(f, "\n");

	fprintf(f, "ALU_Clause_Count = %lld\n", (long long) wavefront->alu_clause_count);
	fprintf(f, "ALU_Group_Count = %lld\n", (long long) wavefront->alu_group_count);
	fprintf(f, "ALU_Group_Size =");
	for (i = 0; i < 5; i++)
		fprintf(f, " %lld", (long long) wavefront->alu_group_size[i]);
	fprintf(f, "\n");
	fprintf(f, "ALU_Inst_Count = %lld\n", (long long) wavefront->alu_inst_count);
	fprintf(f, "ALU_Inst_Local_Mem_Count = %lld\n", (long long) wavefront->alu_inst_local_mem_count);
	fprintf(f, "\n");

	fprintf(f, "TC_Clause_Count = %lld\n", (long long) wavefront->tc_clause_count);
	fprintf(f, "TC_Inst_Count = %lld\n", (long long) wavefront->tc_inst_count);
	fprintf(f, "TC_Inst_Global_Mem_Read_Count = %lld\n", (long long) wavefront->tc_inst_global_mem_read_count);
	fprintf(f, "\n");

	gpu_wavefront_divergence_dump(wavefront, f);

	fprintf(f, "\n");
}


void gpu_wavefront_stack_push(struct gpu_wavefront_t *wavefront)
{
	if (wavefront->stack_top == GPU_MAX_STACK_SIZE - 1)
		fatal("%s: stack overflow", gpu_isa_inst->info->name);
	wavefront->stack_top++;
	bit_map_copy(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count,
		wavefront->active_stack, (wavefront->stack_top - 1) * wavefront->work_item_count,
		wavefront->work_item_count);
	gpu_isa_debug("  %s:push", wavefront->name);
}


void gpu_wavefront_stack_pop(struct gpu_wavefront_t *wavefront, int count)
{
	if (!count)
		return;
	if (wavefront->stack_top < count)
		fatal("%s: stack underflow", gpu_isa_inst->info->name);
	wavefront->stack_top -= count;
	if (debug_status(gpu_isa_debug_category)) {
		gpu_isa_debug("  %s:pop(%d),act=", wavefront->name, count);
		bit_map_dump(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count,
			wavefront->work_item_count, debug_file(gpu_isa_debug_category));
	}
}




/*
 * GPU work_item
 */

struct gpu_work_item_t *gpu_work_item_create()
{
	struct gpu_work_item_t *work_item;
	work_item = calloc(1, sizeof(struct gpu_work_item_t));
	work_item->write_task_list = lnlist_create();
	work_item->lds_oqa = list_create(5);
	work_item->lds_oqb = list_create(5);
	return work_item;
}


void gpu_work_item_free(struct gpu_work_item_t *work_item)
{
	/* Empty LDS output queues */
	while (list_count(work_item->lds_oqa))
		free(list_dequeue(work_item->lds_oqa));
	while (list_count(work_item->lds_oqb))
		free(list_dequeue(work_item->lds_oqb));
	list_free(work_item->lds_oqa);
	list_free(work_item->lds_oqb);
	lnlist_free(work_item->write_task_list);

	/* Free work_item */
	free(work_item);
}


void gpu_work_item_set_active(struct gpu_work_item_t *work_item, int active)
{
	struct gpu_wavefront_t *wavefront = work_item->wavefront;
	assert(work_item->global_id >= wavefront->global_id &&
		work_item->global_id < wavefront->global_id + wavefront->work_item_count);
	assert(work_item->wavefront_id < wavefront->work_item_count);
	bit_map_set(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count
		+ work_item->wavefront_id, 1, !!active);
}


int gpu_work_item_get_active(struct gpu_work_item_t *work_item)
{
	struct gpu_wavefront_t *wavefront = work_item->wavefront;
	assert(work_item->global_id >= wavefront->global_id &&
		work_item->global_id < wavefront->global_id + wavefront->work_item_count);
	assert(work_item->wavefront_id < wavefront->work_item_count);
	return bit_map_get(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count
		+ work_item->wavefront_id, 1);
}


void gpu_work_item_set_pred(struct gpu_work_item_t *work_item, int pred)
{
	struct gpu_wavefront_t *wavefront = work_item->wavefront;
	assert(work_item->global_id >= wavefront->global_id &&
		work_item->global_id < wavefront->global_id + wavefront->work_item_count);
	assert(work_item->wavefront_id < wavefront->work_item_count);
	bit_map_set(wavefront->pred, work_item->wavefront_id, 1, !!pred);
}


int gpu_work_item_get_pred(struct gpu_work_item_t *work_item)
{
	struct gpu_wavefront_t *wavefront = work_item->wavefront;
	assert(work_item->global_id >= wavefront->global_id &&
		work_item->global_id < wavefront->global_id + wavefront->work_item_count);
	assert(work_item->wavefront_id < wavefront->work_item_count);
	return bit_map_get(wavefront->pred, work_item->wavefront_id, 1);
}


/* Based on an instruction counter, instruction address, and work_item mask,
 * update (xor) branch_digest with a random number */
void gpu_work_item_update_branch_digest(struct gpu_work_item_t *work_item, uint64_t inst_count, uint32_t inst_addr)
{
	struct gpu_wavefront_t *wavefront = work_item->wavefront;
	uint32_t mask = 0;

	/* Update branch digest only if work_item is active */
	if (!bit_map_get(wavefront->active_stack, wavefront->stack_top * wavefront->work_item_count + work_item->wavefront_id, 1))
		return;

	/* Update mask with inst_count */
	mask = (uint32_t) inst_count * 0x4919f71f;  /* Multiply by prime number to generate sparse mask */

	/* Update mask with inst_addr */
	mask += inst_addr * 0x31f2e73b;

	/* Update branch digest */
	work_item->branch_digest ^= mask;
}



