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

#include <fcntl.h>
#include <stdarg.h>
#include <zlib.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/bin-config.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "checkpoint.h"
#include "context.h"
#include "emu.h"
#include "file-desc.h"
#include "loader.h"
#include "regs.h"


void X86EmuLoadCheckpoint(X86Emu *self, char *file_name);
void X86EmuSaveCheckpoint(X86Emu *self, char *file_name);

/* High level "load/save part of architectural state" functions */

static void load_processes(X86Emu *emu);
static void save_processes(X86Emu *emu);
static void load_process(X86Emu *emu);
static void save_process(X86Context *ctx);
static void save_process_misc(X86Context *ctx);
static void load_memory(struct mem_t *mem);
static void save_memory(struct mem_t *mem);
static void load_memory_data(struct mem_t *mem);
static void save_memory_data(struct mem_t *mem);
static void load_memory_range(struct mem_t *mem);
static void save_memory_page(struct mem_page_t *page);
static void load_fds(struct x86_file_desc_table_t *fdt);
static void load_fd(struct x86_file_desc_table_t *fdt);
static void save_fds(struct x86_file_desc_table_t *fdt);
static void load_threads(X86Context *ctx);
static void save_threads(X86Context *ctx);
static void save_thread(X86Context *ctx);
static void load_regs(struct x86_regs_t *regs);
static void save_regs(struct x86_regs_t *regs);

/* Configuration element stack */

static void cfg_init(void);
static void cfg_done(void);
static void cfg_push(char *fmt, ...);
static void cfg_push_unique(void);
static void cfg_pop(void);
static void cfg_descend(char *key);
static int cfg_try_descend(char *key);
static int cfg_next_child(void);
static struct cfg_stack_elem_t *cfg_top(void);
static char *cfg_path(void);

/* Low level helper functions */

static int key_exists(char *key);

static void load_value(char *key, void **value, int size);
static int16_t load_int16(char *key);
static int32_t load_int32(char *key);
static char * load_str(char *key);
static char * load_str_or_dflt(char *key, char *dflt);
static struct linked_list_t *load_str_list(char *key);

static void save_value(char *key, void *value, int size);
static void save_value_no_dup(char *key, void *value, int size);
static void save_int16(char *key, int16_t value);
static void save_int32(char *key, int32_t value);
static void save_str(char *key, char *str);
static void save_str_list(char *key, struct linked_list_t *list);

static void save_raw_str(char *var);

static void check(void);
static int index_of(int *array, int value, int n);

/* Pointer to the whole checkpoint */
struct bin_config_t *ckp;

/* Configuration element stack */
struct cfg_stack_elem_t
{
	struct bin_config_elem_t *elem;
	char *key;
	int child_idx;
};

static struct cfg_stack_elem_t *cfg_stack_elem_create(
	struct bin_config_elem_t *elem,
	char *key);
static void cfg_stack_elem_free(struct cfg_stack_elem_t *elem);

struct list_t *cfg_stack;
static int cfg_unique_num;

void X86EmuLoadCheckpoint(X86Emu *self, char *file_name)
{
	ckp = bin_config_create(file_name);
	bin_config_load(ckp);
	check();
	cfg_init();

	load_processes(self);

	cfg_done();
	bin_config_free(ckp);
	ckp = NULL;
}

void X86EmuSaveCheckpoint(X86Emu *self, char *file_name)
{
	ckp = bin_config_create(file_name);
	cfg_init();

	save_processes(self);

	bin_config_save(ckp);
	check();
	cfg_done();
	bin_config_free(ckp);
	ckp = NULL;
}

static void load_processes(X86Emu *emu)
{
	cfg_descend("processes");

	while (cfg_next_child())
	{
		load_process(emu);
		cfg_pop();
	}

	cfg_pop();
}

static void save_processes(X86Emu *emu)
{
	const int MAX_NUM_PIDS = 1024;
	int pids[MAX_NUM_PIDS];
	X86Context *ctx;
	int num_pids;

	cfg_push("processes");

	num_pids = 0;

	/* Since we do not keep an explicit list of processes, iterate over
	 * contexts and keep track which pids have already been saved. */
	for (ctx = emu->context_list_head; ctx; ctx = ctx->context_list_next)
	{
		if (index_of(pids, ctx->pid, num_pids) == -1)
		{
			save_process(ctx);
			pids[num_pids] = ctx->pid;
			num_pids++;
			assert(num_pids < MAX_NUM_PIDS);
		}
	}

	cfg_pop();
}


static void load_process(X86Emu *emu)
{
	X86Context *ctx;
	struct x86_loader_t *ld;

	ctx = new(X86Context, emu);
	ctx->glibc_segment_base = load_int32("glibc_base");
	ctx->glibc_segment_limit = load_int32("glibc_limit");

	ld = ctx->loader;

	ld->interp = load_str_or_dflt("interpreter", 0);
	ld->exe = load_str_or_dflt("executable", 0);
	ld->cwd = load_str("cwd");
	ld->stdin_file = load_str_or_dflt("stdin_file", 0);
	ld->stdout_file = load_str_or_dflt("stdout_file", 0);

	/* Replace initial args and env */
	assert(linked_list_count(ld->args) == 0);
	linked_list_free(ld->args);
	assert(linked_list_count(ld->env) == 0);
	linked_list_free(ld->env);
	ld->args = load_str_list("args");
	ld->env = load_str_list("env");

	load_memory(ctx->mem);
	load_fds(ctx->file_desc_table);
	load_threads(ctx);
}

static void save_process(X86Context *ctx)
{
	cfg_push("%d", ctx->pid);

	save_process_misc(ctx);
	save_memory(ctx->mem);
	save_fds(ctx->file_desc_table);
	save_threads(ctx);

	cfg_pop();
}

static void save_process_misc(X86Context *ctx)
{
	struct x86_loader_t *ld;

	save_int32("pid", ctx->pid);
	save_int32("glibc_base", ctx->glibc_segment_base);
	save_int32("glibc_limit", ctx->glibc_segment_limit);

	ld = ctx->loader;
	save_str("interpreter", ld->interp);
	save_str("executable", ld->exe);
	save_str("cwd", ld->cwd);
	save_str("stdin_file" , ld->stdin_file);
	save_str("stdout_file", ld->stdout_file);
	save_str_list("args", ld->args);
	save_str_list("env", ld->env);
}

static void load_memory(struct mem_t *mem)
{
	cfg_descend("memory");

	mem->heap_break = load_int32("brk");
	load_memory_data(mem);

	cfg_pop();
}

static void save_memory(struct mem_t *mem)
{
	cfg_push("memory");

	save_int32("brk", mem->heap_break);
	save_memory_data(mem);

	cfg_pop();
}

static void load_memory_data(struct mem_t *mem)
{
	int old_mem_safe;

	cfg_descend("ranges");

	/* Inhibit access permission errors */
	old_mem_safe = mem->safe;
	mem->safe = 0;

	while(cfg_next_child()) {
		load_memory_range(mem);
		cfg_pop();
	}

	mem->safe = old_mem_safe;

	cfg_pop();
}

static void save_memory_data(struct mem_t *mem)
{
	int old_mem_safe;
	int i;

	cfg_push("ranges");

	/* Inhibit access permission errors */
	old_mem_safe = mem->safe;
	mem->safe = 0;

	/* Iterate over memory pages */
	for (i = 0; i < MEM_PAGE_COUNT; i++)
	{
		struct mem_page_t *page;

		for (page = mem->pages[i]; page; page = page->next)
			save_memory_page(page);
	}

	mem->safe = old_mem_safe;

	cfg_pop();
}

static void load_memory_range(struct mem_t *mem)
{
	char *data;
	int have_data;
	int addr, size, perm, offset;

	addr = load_int32("addr");
	size = load_int32("size");
	perm = load_int32("perm");
	have_data = key_exists("data");
	if (have_data)
		load_value("data", (void**)&data, size);

	if (addr % MEM_PAGE_SIZE || size % MEM_PAGE_SIZE)
		fatal("Memory range %s (addr: 0x%x, size: %d)"
			"not aligned to page size (%d)",
			cfg_path(), addr, size, MEM_PAGE_SIZE);
	
	/* Check that no pages within the range already exist */
	for (offset = 0; offset < size; offset += MEM_PAGE_SIZE) {
		if (mem_page_get(mem, addr + offset))
			fatal("Checkpoint element %s duplicates "
				"memory data for addr 0x%x",
				cfg_path(), addr + offset);
	}

	mem_map(mem, addr, size, perm);

	if (have_data) {
		mem_write(mem, addr, size, data);
		/* remove the memory data from the bin_config data structure
		   to avoid potential out-of-memory errors due to memory image
		   duplication */
		data = NULL;
		bin_config_remove(ckp, cfg_top()->elem, "data");
		check();
	}
}

static void save_memory_page(struct mem_page_t *page)
{
	cfg_push_unique();

	save_int32("addr", page->tag);
	save_int32("size", MEM_PAGE_SIZE);
	save_int32("perm", page->perm);
	if (page->data)
		save_value_no_dup("data", page->data, MEM_PAGE_SIZE);

	cfg_pop();
}

static void load_fds(struct x86_file_desc_table_t *fdt)
{
	cfg_descend("file_descriptors");

	while(cfg_next_child()) {
		load_fd(fdt);
		cfg_pop();
	}

	cfg_pop();
}

static void load_fd(struct x86_file_desc_table_t *fdt)
{
	int kind, offset;
	int flags, new_flags;
	int guest_fd, host_fd;
	char *path;
	
	kind = load_int32("kind");
	guest_fd = load_int32("index");
	if (kind != file_desc_regular)
	{
		warning("Ignoring %s (non-regular file)", cfg_path());
		return;
	}
	flags  = load_int32("flags");
	path   = load_str  ("path");
	offset = load_int32("offset");

	new_flags =
		flags & ~O_CREAT & ~O_EXCL & ~O_NOCTTY & ~O_TRUNC;

	host_fd = open(path, new_flags);
	if (host_fd < 0)
	{
		warning("Ignoring %s: could not open %s",
			cfg_path(), path);
		return;
	}

	if (new_flags != flags)
	{
		warning("Flags for %s changed from %x to %x",
			cfg_path(), flags, new_flags);
	}

	if (offset > 0)
	{
		int ret_offset = lseek(host_fd, offset, SEEK_SET);
		if(ret_offset != offset)
			fatal("While loading %s, "
				"could not set offset %d for %s",
				cfg_path(), offset, path);
	}

	x86_file_desc_table_entry_new_guest_fd(fdt, file_desc_regular,
		guest_fd, host_fd, (char *)path, flags);

	free(path);
}

static void save_fds(struct x86_file_desc_table_t *fdt)
{
	int i;

	cfg_push("file_descriptors");

	for (i = 0; i < list_count(fdt->file_desc_list); i++)
	{
		struct x86_file_desc_t *fd;

		fd = list_get(fdt->file_desc_list, i);
		if (!fd) continue; /* unused file descriptor */

		cfg_push("%d", fd->guest_fd);

		save_int32("index", fd->guest_fd);
		save_str("path", fd->path);
		save_int32("flags", fd->flags);
		save_int32("kind", fd->kind);
		if (fd->kind == file_desc_regular)
		{
			save_int32("offset",
				lseek(fd->host_fd, 0, SEEK_CUR));
		}

		cfg_pop();
	}

	cfg_pop();
}

static void load_threads(X86Context *process_ctx)
{
	int first;

	cfg_descend("threads");

	first = 1;

	while (cfg_next_child())
	{
		X86Context *thread_ctx;
		if (first)
		{
			thread_ctx = process_ctx;
			first = 0;
		}
		else
		{
			thread_ctx = new_ctor(X86Context, CreateAndClone,
					process_ctx);
		}

		load_regs(thread_ctx->regs);

		cfg_pop();
	}

	cfg_pop();
}

static void save_threads(X86Context *process_ctx)
{
	X86Emu *emu = process_ctx->emu;
	X86Context *thread_ctx;

	cfg_push("threads");

	/* Iterate over threads belonging to the process represented by ctx */
	for (thread_ctx = emu->context_list_head;
	     thread_ctx;
	     thread_ctx = thread_ctx->context_list_next) 
	{
		if (thread_ctx->pid == process_ctx->pid)
		{
			save_thread(thread_ctx);
		}
	}

	cfg_pop();
}

static void save_thread(X86Context *ctx)
{
	cfg_push_unique();

	if (X86ContextGetState(ctx, X86ContextSpecMode))
	{
		save_regs(ctx->backup_regs);
	}
	else
	{
		save_regs(ctx->regs);
	}

	cfg_pop();
}

static void load_regs(struct x86_regs_t *regs)
{
	cfg_descend("registers");

	/* General purpose */
	{
		regs->eax = load_int32("eax");
		regs->ecx = load_int32("ecx");
		regs->edx = load_int32("edx");
		regs->ebx = load_int32("ebx");
		regs->esp = load_int32("esp");
		regs->ebp = load_int32("ebp");
		regs->esi = load_int32("esi");
		regs->edi = load_int32("edi");
		regs->eip = load_int32("eip");
		regs->eflags = load_int32("eflags");
	}

	/* Segment */
	{
		regs->es = load_int16("es");
		regs->cs = load_int16("cs");
		regs->ss = load_int16("ss");
		regs->ds = load_int16("ds");
		regs->fs = load_int16("fs");
		regs->gs = load_int16("gs");
	}

	/* x87 */
	{
		int fpsw;
		int ftw;
		char buf[10];
		int rel_idx;

		fpsw = load_int32("fpsw");
		regs->fpu_top = BITS32(fpsw, 13, 11);
		regs->fpu_code = SETBITVALUE32(regs->fpu_code, 3, GETBIT32(fpsw, 14));
		regs->fpu_code = SETBITVALUE32(regs->fpu_code, 2, GETBIT32(fpsw, 10));
		regs->fpu_code = SETBITVALUE32(regs->fpu_code, 1, GETBIT32(fpsw,  9));
		regs->fpu_code = SETBITVALUE32(regs->fpu_code, 0, GETBIT32(fpsw,  8));
		regs->fpu_ctrl = load_int16("fpcw");

		ftw = load_int16("ftw");
		for (rel_idx = 0; rel_idx < 8; rel_idx++) {
			int abs_idx;

			sprintf(buf, "st%d", rel_idx);
			abs_idx = (regs->fpu_top + rel_idx) % 8;
			load_value(buf, (void *)regs->fpu_stack[abs_idx].value, 10);
			regs->fpu_stack[abs_idx].valid = GETBIT32(ftw, abs_idx);
		}
	}

	/* XMM */
	{
		int xmm_idx;
		char buf[10];

		for (xmm_idx = 0; xmm_idx < 8; xmm_idx++) {
			sprintf(buf, "xmm%d", xmm_idx);
			load_value(buf, (void *)&regs->xmm[xmm_idx], 16);
		}
	}

	cfg_pop();
}

static void save_regs(struct x86_regs_t *regs)
{
	cfg_push("registers");

	/* General purpose */
	{
		save_int32("eax", regs->eax);
		save_int32("ecx", regs->ecx);
		save_int32("edx", regs->edx);
		save_int32("ebx", regs->ebx);
		save_int32("esp", regs->esp);
		save_int32("ebp", regs->ebp);
		save_int32("esi", regs->esi);
		save_int32("edi", regs->edi);
		save_int32("eip", regs->eip);
		save_int32("eflags", regs->eflags);
	}

	/* Segment */
	{
		save_int16("es", regs->es);
		save_int16("cs", regs->cs);
		save_int16("ss", regs->ss);
		save_int16("ds", regs->ds);
		save_int16("fs", regs->fs);
		save_int16("gs", regs->gs);
	}

	/* x87 */
	{
		int fpsw;
		int ftw;
		int rel_idx;
		char buf[10];

		fpsw = (BITS32(regs->fpu_top, 2, 0) << 11) |
			(GETBIT32(regs->fpu_code, 3) << 14) |
			(GETBIT32(regs->fpu_code, 2) << 10) |
			(GETBIT32(regs->fpu_code, 1) <<  9) |
			(GETBIT32(regs->fpu_code, 0) <<  8);
	
		save_int32("fpsw", fpsw);
		save_int16("fpcw", regs->fpu_ctrl);

		for (rel_idx = 0; rel_idx < 8; rel_idx++) {
			int abs_idx;

			sprintf(buf, "st%d", rel_idx);
			abs_idx = (regs->fpu_top + rel_idx) % 8;
			save_value(buf, regs->fpu_stack[abs_idx].value, 10);
			ftw = SETBITVALUE32(ftw, abs_idx,
				regs->fpu_stack[abs_idx].valid);
		}
		save_int16("ftw", ftw);
	}

	/* XMM */
	{
		int xmm_idx;
		char buf[10];

		for (xmm_idx = 0; xmm_idx < 8; xmm_idx++) {
			sprintf(buf, "xmm%d", xmm_idx);
			save_value(buf, &regs->xmm[xmm_idx], 16);
		}
	}

	cfg_pop();
}

static void check(void)
{
	switch(ckp->error_code) {
	case BIN_CONFIG_ERR_OK:
		return;
		break;
	case BIN_CONFIG_ERR_PARENT:
		fatal("Checkpointing %s: error in checkpoint code", cfg_path());
		break;
	case BIN_CONFIG_ERR_IO:
		fatal("Checkpointing to %s: I/O error", ckp->file_name);
		break;
	default:
		fatal("Checkpointing %s: format error", cfg_path());
		break;
	}
}

static int index_of(int *array, int value, int n)
{
	int i;

	for (i = 0; i < n; i++)
	{
		if (array[i] == value)
			return i;
	}
	return -1;
}

static void cfg_init(void)
{
	cfg_stack = list_create();
	/* dummy root cfg node */
	list_push(cfg_stack, cfg_stack_elem_create(0, ""));
	cfg_unique_num = 0;
}

static void cfg_done(void)
{
	cfg_pop(); /* pop dummy node */
	list_free(cfg_stack);
}

static void cfg_push(char *fmt, ...)
{
	struct bin_config_elem_t *cfg;
	char buf[MAX_STRING_SIZE];
	va_list vl;

	va_start(vl, fmt);
	vsprintf(buf, fmt, vl);
	va_end(vl);

	cfg = bin_config_add(ckp, cfg_top()->elem, buf, NULL, 0);
	list_push(cfg_stack, cfg_stack_elem_create(cfg, buf));
}

static void cfg_push_unique(void)
{
	struct bin_config_elem_t *cfg;
	char buf[MAX_STRING_SIZE];

	sprintf(buf, "%d", cfg_unique_num);
	cfg = bin_config_add(ckp, cfg_top()->elem, buf, NULL, 0);
	cfg_unique_num++;
	list_push(cfg_stack, cfg_stack_elem_create(cfg, buf));
}

static void save_raw_str(char *str)
{
	char buf[MAX_STRING_SIZE];

	sprintf(buf, "%d", cfg_unique_num);
	bin_config_add(ckp, cfg_top()->elem, buf, str, strlen(str) + 1);
	cfg_unique_num++;
}

static void cfg_pop(void)
{
	cfg_stack_elem_free(cfg_top());
	list_pop(cfg_stack);
}

static int cfg_try_descend(char *key)
{
	struct bin_config_elem_t *cfg;

	cfg = bin_config_get(ckp, cfg_top()->elem, key, 0, 0);
	if (ckp->error_code == BIN_CONFIG_ERR_NOT_FOUND)
		return 0;
	check();
	list_push(cfg_stack, cfg_stack_elem_create(cfg, key));

	return 1;
}

static void cfg_descend(char *key)
{
	if (!cfg_try_descend(key))
		fatal("Loading checkpoint from %s: could not find %s/%s",
			ckp->file_name, cfg_path(), key);
}

static int cfg_next_child(void)
{
	struct bin_config_elem_t *child;
	char *key;

	if (cfg_top()->child_idx < 0) {
		/* Children traversal not started */
		child = bin_config_find_first(ckp, cfg_top()->elem, &key, 0, 0);
	} else {
		child = bin_config_find_next(ckp, cfg_top()->elem, &key, 0, 0);
	}
	cfg_top()->child_idx++;

	if (!child && ckp->error_code == BIN_CONFIG_ERR_NOT_FOUND)
		return 0;
	check();

	list_push(cfg_stack, cfg_stack_elem_create(child, key));

	return 1;
}

static struct cfg_stack_elem_t *cfg_top(void)
{
	return (struct cfg_stack_elem_t *)list_top(cfg_stack);
}

static char *cfg_path(void)
{
	static char buf[MAX_STRING_SIZE];
	int i;
	int len = 0;

	/* start with 1 to ignore dummy root node */
	for (i = 1; i < cfg_stack->count; i++) {
		len += snprintf(buf + len, MAX_STRING_SIZE - len, "/%s",
			((struct cfg_stack_elem_t *)
				list_get(cfg_stack, i))->key);
	}

	return buf;
}

static int key_exists(char *key)
{
	if (!bin_config_get(ckp, cfg_top()->elem, key, 0, 0) &&
		ckp->error_code == BIN_CONFIG_ERR_NOT_FOUND) {
		return 0;
	}

	check();

	return 1;
}

static void load_value(char *key, void **value, int size)
{
	int ckp_size;

	bin_config_get(ckp, cfg_top()->elem, key, value, &ckp_size);
	check();
	if (size != ckp_size)
		fatal("Checkpoint element data size mismatch (element: %s/%s, "
			"expected data size: %d, loaded data size: %d)",
			cfg_path(), key, size, ckp_size);
}

static void save_value(char *key, void *value, int size)
{
	bin_config_add(ckp, cfg_top()->elem, key, value, size);
	check();
}

static void save_value_no_dup(char *key, void *value, int size)
{
	bin_config_add_no_dup(ckp, cfg_top()->elem, key, value, size);
	check();
}

#define DEF_LOAD_TYPE(type) \
static type##_t load_##type(char *key) \
{ \
	type##_t *value; \
	load_value(key, (void **)&value, sizeof(type##_t));	\
	return *value; \
} \

DEF_LOAD_TYPE(int16)
DEF_LOAD_TYPE(int32)

#undef DEF_LOAD_TYPE

#define DEF_SAVE_TYPE(type) \
static void save_##type(char *key, type##_t value) \
{ \
	bin_config_add(ckp, cfg_top()->elem, key, &value, sizeof(value)); \
	check(); \
} \

DEF_SAVE_TYPE(int16)
DEF_SAVE_TYPE(int32)

#undef DEF_SAVE_TYPE

static char *load_str(char *key)
{
	char *value;
	char *dup;
	int size;

	bin_config_get(ckp, cfg_top()->elem, key, (void **)&value, &size);

	if (!memchr(value, 0, size))
		fatal("Null terminator not found in string (element: %s/%s)",
			cfg_path(), key);

	dup = xstrdup(value);

	return dup;
}

static char *load_str_or_dflt(char *key, char *dflt)
{
	if(key_exists(key))
		return load_str(key);
	else
		return dflt;
}

static void save_str(char *key, char *str)
{
	if (str) {
		/* Include null terminator for safety */
		save_value(key, str, strlen(str) + 1);
	} else {
		warning("%s/%s not available", cfg_path(), key);
	}
}

static struct linked_list_t *load_str_list(char *key)
{
	struct linked_list_t *ll = linked_list_create();
	cfg_descend(key);
	while(cfg_next_child()) {
		char *dup;

		dup = xstrdup(cfg_top()->key);

		linked_list_add(ll, dup);
		cfg_pop();
	}
	cfg_pop();
	return ll;
}

static void save_str_list(char *key, struct linked_list_t *list)
{
	cfg_push(key);

	linked_list_head(list);
	while(!linked_list_is_end(list)) {
		save_raw_str(linked_list_get(list));
		linked_list_next(list);
	}

	cfg_pop();
}

static struct cfg_stack_elem_t *cfg_stack_elem_create(
	struct bin_config_elem_t *elem,
	char *key)
{
	struct cfg_stack_elem_t *ret;

	ret = xmalloc(sizeof(struct cfg_stack_elem_t));
	ret->elem = elem;
	ret->key = xstrdup(key);
	ret->child_idx = -1;

	return ret;
}

static void cfg_stack_elem_free(struct cfg_stack_elem_t *elem)
{
	free(elem->key);
	free(elem);
}
