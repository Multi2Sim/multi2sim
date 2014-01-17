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

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "context.h"
#include "file.h"
#include "isa.h"
#include "regs.h"
#include "syscall.h"


/*
 * Public Variables
 */


int arm_sys_debug_category;




/*
 * Private Variables
 */


static char *err_arm_sys_note =
	"\tThe system calls performed by the executed application are intercepted by\n"
	"\tMulti2Sim and emulated in file 'syscall.c'. The most common system calls are\n"
	"\tcurrently supported, but your application might perform specific unsupported\n"
	"\tsystem calls or combinations of parameters. To request support for a given\n"
	"\tsystem call, please email 'development@multi2sim.org'.\n";


/* System call names */
static char *arm_sys_call_name[] =
{
#define DEFSYSCALL(name, code) #name,
#include "syscall.dat"
#undef DEFSYSCALL
	""
};


/* System call codes */
enum
{
#define DEFSYSCALL(name, code) arm_sys_code_##name = code,
#include "syscall.dat"
#undef DEFSYSCALL
arm_sys_code_count
};


/* Forward declarations of system calls */
#define DEFSYSCALL(name, code) \
	static int arm_sys_##name##_impl(struct arm_ctx_t *ctx);
#include "syscall.dat"
#undef DEFSYSCALL


/* System call functions */
static int (*arm_sys_call_func[arm_sys_code_count + 1])(struct arm_ctx_t *ctx) =
{
#define DEFSYSCALL(name, code) arm_sys_##name##_impl,
#include "syscall.dat"
#undef DEFSYSCALL
	NULL
};


/* Statistics */
static int arm_sys_call_freq[arm_sys_code_count + 1];




/*
 * System call error codes
 */

#define SIM_EPERM		1
#define SIM_ENOENT		2
#define SIM_ESRCH		3
#define SIM_EINTR		4
#define SIM_EIO			5
#define SIM_ENXIO		6
#define SIM_E2BIG		7
#define SIM_ENOEXEC		8
#define SIM_EBADF		9
#define SIM_ECHILD		10
#define SIM_EAGAIN		11
#define SIM_ENOMEM		12
#define SIM_EACCES		13
#define SIM_EFAULT		14
#define SIM_ENOTBLK		15
#define SIM_EBUSY		16
#define SIM_EEXIST		17
#define SIM_EXDEV		18
#define SIM_ENODEV		19
#define SIM_ENOTDIR		20
#define SIM_EISDIR		21
#define SIM_EINVAL		22
#define SIM_ENFILE		23
#define SIM_EMFILE		24
#define SIM_ENOTTY		25
#define SIM_ETXTBSY		26
#define SIM_EFBIG		27
#define SIM_ENOSPC		28
#define SIM_ESPIPE		29
#define SIM_EROFS		30
#define SIM_EMLINK		31
#define SIM_EPIPE		32
#define SIM_EDOM		33
#define SIM_ERANGE		34

#define SIM_ERRNO_MAX		34


static struct str_map_t arm_sys_error_code_map =
{
	34,
	{
		{ "EPERM", 1 },
		{ "ENOENT", 2 },
		{ "ESRCH", 3 },
		{ "EINTR", 4 },
		{ "EIO", 5 },
		{ "ENXIO", 6 },
		{ "E2BIG", 7 },
		{ "ENOEXEC", 8 },
		{ "EBADF", 9 },
		{ "ECHILD", 10 },
		{ "EAGAIN", 11 },
		{ "ENOMEM", 12 },
		{ "EACCES", 13 },
		{ "EFAULT", 14 },
		{ "ENOTBLK", 15 },
		{ "EBUSY", 16 },
		{ "EEXIST", 17 },
		{ "EXDEV", 18 },
		{ "ENODEV", 19 },
		{ "ENOTDIR", 20 },
		{ "EISDIR", 21 },
		{ "EINVAL", 22 },
		{ "ENFILE", 23 },
		{ "EMFILE", 24 },
		{ "ENOTTY", 25 },
		{ "ETXTBSY", 26 },
		{ "EFBIG", 27 },
		{ "ENOSPC", 28 },
		{ "ESPIPE", 29 },
		{ "EROFS", 30 },
		{ "EMLINK", 31 },
		{ "EPIPE", 32 },
		{ "EDOM", 33 },
		{ "ERANGE", 34 }
	}
};


void arm_sys_init(void)
{
	/* Host constants for 'errno' must match */
	M2S_HOST_GUEST_MATCH(EPERM, SIM_EPERM);
	M2S_HOST_GUEST_MATCH(ENOENT, SIM_ENOENT);
	M2S_HOST_GUEST_MATCH(ESRCH, SIM_ESRCH);
	M2S_HOST_GUEST_MATCH(EINTR, SIM_EINTR);
	M2S_HOST_GUEST_MATCH(EIO, SIM_EIO);
	M2S_HOST_GUEST_MATCH(ENXIO, SIM_ENXIO);
	M2S_HOST_GUEST_MATCH(E2BIG, SIM_E2BIG);
	M2S_HOST_GUEST_MATCH(ENOEXEC, SIM_ENOEXEC);
	M2S_HOST_GUEST_MATCH(EBADF, SIM_EBADF);
	M2S_HOST_GUEST_MATCH(ECHILD, SIM_ECHILD);
	M2S_HOST_GUEST_MATCH(EAGAIN, SIM_EAGAIN);
	M2S_HOST_GUEST_MATCH(ENOMEM, SIM_ENOMEM);
	M2S_HOST_GUEST_MATCH(EACCES, SIM_EACCES);
	M2S_HOST_GUEST_MATCH(EFAULT, SIM_EFAULT);
	M2S_HOST_GUEST_MATCH(ENOTBLK, SIM_ENOTBLK);
	M2S_HOST_GUEST_MATCH(EBUSY, SIM_EBUSY);
	M2S_HOST_GUEST_MATCH(EEXIST, SIM_EEXIST);
	M2S_HOST_GUEST_MATCH(EXDEV, SIM_EXDEV);
	M2S_HOST_GUEST_MATCH(ENODEV, SIM_ENODEV);
	M2S_HOST_GUEST_MATCH(ENOTDIR, SIM_ENOTDIR);
	M2S_HOST_GUEST_MATCH(EISDIR, SIM_EISDIR);
	M2S_HOST_GUEST_MATCH(EINVAL, SIM_EINVAL);
	M2S_HOST_GUEST_MATCH(ENFILE, SIM_ENFILE);
	M2S_HOST_GUEST_MATCH(EMFILE, SIM_EMFILE);
	M2S_HOST_GUEST_MATCH(ENOTTY, SIM_ENOTTY);
	M2S_HOST_GUEST_MATCH(ETXTBSY, SIM_ETXTBSY);
	M2S_HOST_GUEST_MATCH(EFBIG, SIM_EFBIG);
	M2S_HOST_GUEST_MATCH(ENOSPC, SIM_ENOSPC);
	M2S_HOST_GUEST_MATCH(ESPIPE, SIM_ESPIPE);
	M2S_HOST_GUEST_MATCH(EROFS, SIM_EROFS);
	M2S_HOST_GUEST_MATCH(EMLINK, SIM_EMLINK);
	M2S_HOST_GUEST_MATCH(EPIPE, SIM_EPIPE);
	M2S_HOST_GUEST_MATCH(EDOM, SIM_EDOM);
	M2S_HOST_GUEST_MATCH(ERANGE, SIM_ERANGE);
}

void arm_sys_done(void)
{
	/* Print summary
	if (debug_status(arm_sys_debug_category))
		arm_sys_dump(debug_file(arm_sys_debug_category));
	 */}

void arm_sys_call(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs = ctx->regs;

	int code;
	int err;

	/* System call code */
	code = regs->r7;
	if (code < 1 || code >= arm_sys_code_count)
		fatal("%s: invalid system call code (%d)", __FUNCTION__, code);

	/* Statistics */
	arm_sys_call_freq[code]++;

	/* Debug */
	arm_sys_debug("system call '%s' (code %d, inst %lld, pid %d)\n",
		arm_sys_call_name[code], code, asEmu(arm_emu)->instructions, ctx->pid);
	arm_isa_call_debug("system call '%s' (code %d, inst %lld, pid %d)\n",
		arm_sys_call_name[code], code, asEmu(arm_emu)->instructions, ctx->pid);

	/* Perform system call */
	err = arm_sys_call_func[code](ctx);

	/* Set return value in 'eax', except for 'sigreturn' system call. Also, if the
	 * context got suspended, the wake up routine will set the return value. */
	if (code != arm_sys_code_sigreturn && !arm_ctx_get_status(ctx, arm_ctx_suspended))
		regs->r0 = err;

	/* Debug */
	arm_sys_debug("  ret=(%d, 0x%x)", err, err);
	if (err < 0 && err >= -SIM_ERRNO_MAX)
		arm_sys_debug(", errno=%s)", str_map_value(&arm_sys_error_code_map, -err));
	arm_sys_debug("\n");
}




/*
 * System call 'close' (code 2)
 */

static int arm_sys_close_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs = ctx->regs;
	struct arm_file_desc_t *fd;

	int guest_fd;
	int host_fd;

	/* Arguments */
	guest_fd = regs->r0;
	arm_sys_debug("  guest_fd=%d\n", guest_fd);
	host_fd = arm_file_desc_table_get_host_fd(ctx->file_desc_table, guest_fd);
	arm_sys_debug("  host_fd=%d\n", host_fd);

	/* Get file descriptor table entry. */
	fd = arm_file_desc_table_entry_get(ctx->file_desc_table, guest_fd);
	if (!fd)
		return -EBADF;

	/* Close host file descriptor only if it is valid and not stdin/stdout/stderr. */
	if (host_fd > 2)
		close(host_fd);

	/* Free guest file descriptor. This will delete the host file if it's a virtual file. */
	if (fd->kind == arm_file_desc_virtual)
		arm_sys_debug("    host file '%s': temporary file deleted\n", fd->path);
	arm_file_desc_table_entry_free(ctx->file_desc_table, fd->guest_fd);

	/* Success */
	return 0;
}




/*
 * System call 'read' (code 3)
 */

static int arm_sys_read_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int buf_ptr;
	unsigned int count;

	int guest_fd;
	int host_fd;
	int err;

	void *buf;

	struct arm_file_desc_t *fd;
	struct pollfd fds;

	/* Arguments */
	guest_fd = regs->r0;
	buf_ptr = regs->r1;
	count = regs->r2;
	arm_sys_debug("  guest_fd=%d, buf_ptr=0x%x, count=0x%x\n",
		guest_fd, buf_ptr, count);

	/* Get file descriptor */
	fd = arm_file_desc_table_entry_get(ctx->file_desc_table, guest_fd);
	if (!fd)
		return -EBADF;
	host_fd = fd->host_fd;
	arm_sys_debug("  host_fd=%d\n", host_fd);

	/* Poll the file descriptor to check if read is blocking */
	buf = xcalloc(1, count);
	fds.fd = host_fd;
	fds.events = POLLIN;
	err = poll(&fds, 1, 0);
	if (err < 0)
		fatal("%s: error executing 'poll'", __FUNCTION__);

	/* Non-blocking read */
	if (fds.revents || (fd->flags & O_NONBLOCK))
	{
		/* Host system call */
		err = read(host_fd, buf, count);
		if (err == -1)
		{
			free(buf);
			return -errno;
		}

		/* Write in guest memory */
		if (err > 0)
		{
			mem_write(mem, buf_ptr, err, buf);
			arm_sys_debug_buffer("  buf", buf, err);
		}

		/* Return number of read bytes */
		free(buf);
		return err;
	}

	/* Blocking read - suspend thread */
	arm_sys_debug("  blocking read - process suspended\n");
	ctx->wakeup_fd = guest_fd;
	ctx->wakeup_events = 1;  /* POLLIN */
	arm_ctx_set_status(ctx, arm_ctx_suspended | arm_ctx_read);
	ARMEmuProcessEventsSchedule(arm_emu);

	/* Free allocated buffer. Return value doesn't matter,
	 * it will be overwritten when context wakes up from blocking call. */
	free(buf);
	return 0;
}




/*
 * System call 'write' (code 4)
 */

static int arm_sys_write_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int buf_ptr;
	unsigned int count;

	int guest_fd;
	int host_fd;
	int err;

	struct arm_file_desc_t *desc;
	void *buf;

	struct pollfd fds;

	/* Arguments */
	guest_fd = regs->r0;
	buf_ptr = regs->r1;
	count = regs->r2;
	arm_sys_debug("  guest_fd=%d, buf_ptr=0x%x, count=0x%x\n",
		guest_fd, buf_ptr, count);

	/* Get file descriptor */
	desc = arm_file_desc_table_entry_get(ctx->file_desc_table, guest_fd);
	if (!desc)
		return -EBADF;
	host_fd = desc->host_fd;
	arm_sys_debug("  host_fd=%d\n", host_fd);

	/* Read buffer from memory */
	buf = xcalloc(1, count);
	mem_read(mem, buf_ptr, count, buf);
	arm_sys_debug_buffer("  buf", buf, count);

	/* Poll the file descriptor to check if write is blocking */
	fds.fd = host_fd;
	fds.events = POLLOUT;
	poll(&fds, 1, 0);

	/* Non-blocking write */
	if (fds.revents)
	{
		/* Host write */
		err = write(host_fd, buf, count);
		if (err == -1)
			err = -errno;

		/* Return written bytes */
		free(buf);
		return err;
	}

	/* Blocking write - suspend thread */
	arm_sys_debug("  blocking write - process suspended\n");
	ctx->wakeup_fd = guest_fd;
	arm_ctx_set_status(ctx, arm_ctx_suspended | arm_ctx_write);
	ARMEmuProcessEventsSchedule(arm_emu);

	/* Return value doesn't matter here. It will be overwritten when the
	 * context wakes up after blocking call. */
	free(buf);
	return 0;
}




/*
 * System call 'open' (code 5)
 */

static struct str_map_t arm_sys_open_flags_map =
{
	16, {
		{ "O_RDONLY",        00000000 },
		{ "O_WRONLY",        00000001 },
		{ "O_RDWR",          00000002 },
		{ "O_CREAT",         00000100 },
		{ "O_EXCL",          00000200 },
		{ "O_NOCTTY",        00000400 },
		{ "O_TRUNC",         00001000 },
		{ "O_APPEND",        00002000 },
		{ "O_NONBLOCK",      00004000 },
		{ "O_SYNC",          00010000 },
		{ "FASYNC",          00020000 },
		{ "O_DIRECT",        00040000 },
		{ "O_LARGEFILE",     00100000 },
		{ "O_DIRECTORY",     00200000 },
		{ "O_NOFOLLOW",      00400000 },
		{ "O_NOATIME",       01000000 }
	}
};

static int arm_sys_open_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	struct arm_file_desc_t *desc;

	unsigned int file_name_ptr;

	int flags;
	int mode;
	int length;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];
	char temp_path[MAX_PATH_SIZE];
	char flags_str[MAX_STRING_SIZE];

	int host_fd;

	/* Arguments */
	file_name_ptr = regs->r0;
	flags = regs->r1;
	mode = regs->r2;
	length = mem_read_string(mem, file_name_ptr, sizeof file_name, file_name);
	if (length >= MAX_PATH_SIZE)
		fatal("syscall open: maximum path length exceeded");
	arm_ctx_loader_get_full_path(ctx, file_name, full_path, sizeof full_path);
	arm_sys_debug("  filename='%s' flags=0x%x, mode=0x%x\n",
		file_name, flags, mode);
	arm_sys_debug("  fullpath='%s'\n", full_path);
	str_map_flags(&arm_sys_open_flags_map, flags, flags_str, sizeof flags_str);
	arm_sys_debug("  flags=%s\n", flags_str);

	/* Virtual files */
	if (!strncmp(full_path, "/proc/", 6))
	{
		/* File /proc/self/maps */
		if (!strcmp(full_path, "/proc/self/maps"))
		{
			/* Create temporary file and open it. */
			arm_ctx_gen_proc_self_maps(ctx, temp_path);
			host_fd = open(temp_path, flags, mode);
			assert(host_fd > 0);

			/* Add file descriptor table entry. */
			desc = arm_file_desc_table_entry_new(ctx->file_desc_table, arm_file_desc_virtual, host_fd, temp_path, flags);
			arm_sys_debug("    host file '%s' opened: guest_fd=%d, host_fd=%d\n",
				temp_path, desc->guest_fd, desc->host_fd);
			return desc->guest_fd;
		}

		/* Unhandled virtual file. Let the application read the contents of the host
		 * version of the file as if it was a regular file. */
		arm_sys_debug("    warning: unhandled virtual file\n");
	}

	/* Regular file. */
	host_fd = open(full_path, flags, mode);
	if (host_fd == -1)
		return -errno;

	/* File opened, create a new file descriptor. */
	desc = arm_file_desc_table_entry_new(ctx->file_desc_table,
		arm_file_desc_regular, host_fd, full_path, flags);
	arm_sys_debug("    file descriptor opened: guest_fd=%d, host_fd=%d\n",
		desc->guest_fd, desc->host_fd);

	/* Return guest descriptor index */
	return desc->guest_fd;
}




/*
 * System call 'brk' (code 45)
 */

static int arm_sys_brk_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int old_heap_break;
	unsigned int new_heap_break;
	unsigned int size;

	unsigned int old_heap_break_aligned;
	unsigned int new_heap_break_aligned;

	/* Arguments */
	new_heap_break = regs->r0;
	old_heap_break = mem->heap_break;
	arm_sys_debug("  newbrk=0x%x (previous brk was 0x%x)\n",
		new_heap_break, old_heap_break);

	/* Align */
	new_heap_break_aligned = ROUND_UP(new_heap_break, MEM_PAGE_SIZE);
	old_heap_break_aligned = ROUND_UP(old_heap_break, MEM_PAGE_SIZE);

	/* If argument is zero, the system call is used to
	 * obtain the current top of the heap. */
	if (!new_heap_break)
		return old_heap_break;

	/* If the heap is increased: if some page in the way is
	 * allocated, do nothing and return old heap top. Otherwise,
	 * allocate pages and return new heap top. */
	if (new_heap_break > old_heap_break)
	{
		size = new_heap_break_aligned - old_heap_break_aligned;
		if (size)
		{
			if (mem_map_space(mem, old_heap_break_aligned, size) != old_heap_break_aligned)
				fatal("%s: out of memory", __FUNCTION__);
			mem_map(mem, old_heap_break_aligned, size,
				mem_access_read | mem_access_write);
		}
		mem->heap_break = new_heap_break;
		arm_sys_debug("  heap grows %u bytes\n", new_heap_break - old_heap_break);
		return new_heap_break;
	}

	/* Always allow to shrink the heap. */
	if (new_heap_break < old_heap_break)
	{
		size = old_heap_break_aligned - new_heap_break_aligned;
		if (size)
			mem_unmap(mem, new_heap_break_aligned, size);
		mem->heap_break = new_heap_break;
		arm_sys_debug("  heap shrinks %u bytes\n", old_heap_break - new_heap_break);
		return new_heap_break;
	}

	/* Heap stays the same */
	return 0;
}




/*
 * System call 'gettimeofday' (code 78)
 */

static int arm_sys_gettimeofday_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int tv_ptr;
	unsigned int tz_ptr;

	struct timeval tv;
	struct timezone tz;

	/* Arguments */
	tv_ptr = regs->r0;
	tz_ptr = regs->r1;
	arm_sys_debug("  tv_ptr=0x%x, tz_ptr=0x%x\n", tv_ptr, tz_ptr);

	/* Host call */
	gettimeofday(&tv, &tz);

	/* Write time value */
	if (tv_ptr)
	{
		mem_write(mem, tv_ptr, 4, &tv.tv_sec);
		mem_write(mem, tv_ptr + 4, 4, &tv.tv_usec);
	}

	/* Write time zone */
	if (tz_ptr)
	{
		mem_write(mem, tz_ptr, 4, &tz.tz_minuteswest);
		mem_write(mem, tz_ptr + 4, 4, &tz.tz_dsttime);
	}

	/* Return */
	return 0;
}




/*
 * System call 'mmap' (code 90)
 */

#define SYS_MMAP_BASE_ADDRESS  0xb7fb0000

static struct str_map_t sys_mmap_prot_map =
{
	6, {
		{ "PROT_READ",       0x1 },
		{ "PROT_WRITE",      0x2 },
		{ "PROT_EXEC",       0x4 },
		{ "PROT_SEM",        0x8 },
		{ "PROT_GROWSDOWN",  0x01000000 },
		{ "PROT_GROWSUP",    0x02000000 }
	}
};

static struct str_map_t sys_mmap_flags_map =
{
	11, {
		{ "MAP_SHARED",      0x01 },
		{ "MAP_PRIVATE",     0x02 },
		{ "MAP_FIXED",       0x10 },
		{ "MAP_ANONYMOUS",   0x20 },
		{ "MAP_GROWSDOWN",   0x00100 },
		{ "MAP_DENYWRITE",   0x00800 },
		{ "MAP_EXECUTABLE",  0x01000 },
		{ "MAP_LOCKED",      0x02000 },
		{ "MAP_NORESERVE",   0x04000 },
		{ "MAP_POPULATE",    0x08000 },
		{ "MAP_NONBLOCK",    0x10000 }
	}
};

static int arm_sys_mmap(struct arm_ctx_t *ctx, unsigned int addr, unsigned int len,
	int prot, int flags, int guest_fd, int offset)
{
	struct mem_t *mem = ctx->mem;

	unsigned int len_aligned;

	int perm;
	int host_fd;

	struct arm_file_desc_t *desc;

	/* Check that protection flags match in guest and host */
	assert(PROT_READ == 1);
	assert(PROT_WRITE == 2);
	assert(PROT_EXEC == 4);

	/* Check that mapping flags match */
	assert(MAP_SHARED == 0x01);
	assert(MAP_PRIVATE == 0x02);
	assert(MAP_FIXED == 0x10);
	assert(MAP_ANONYMOUS == 0x20);

	/* Translate file descriptor */
	desc = arm_file_desc_table_entry_get(ctx->file_desc_table, guest_fd);
	host_fd = desc ? desc->host_fd : -1;
	if (guest_fd > 0 && host_fd < 0)
		fatal("%s: invalid guest descriptor", __FUNCTION__);

	/* Permissions */
	perm = mem_access_init;
	perm |= prot & PROT_READ ? mem_access_read : 0;
	perm |= prot & PROT_WRITE ? mem_access_write : 0;
	perm |= prot & PROT_EXEC ? mem_access_exec : 0;

	/* Flag MAP_ANONYMOUS.
	 * If it is set, the 'fd' parameter is ignored. */
	if (flags & MAP_ANONYMOUS)
		host_fd = -1;

	/* 'addr' and 'offset' must be aligned to page size boundaries.
	 * 'len' is rounded up to page boundary. */
	if (offset & ~MEM_PAGE_MASK)
		fatal("%s: unaligned offset", __FUNCTION__);
	if (addr & ~MEM_PAGE_MASK)
		fatal("%s: unaligned address", __FUNCTION__);
	len_aligned = ROUND_UP(len, MEM_PAGE_SIZE);

	/* Find region for allocation */
	if (flags & MAP_FIXED)
	{
		/* If MAP_FIXED is set, the 'addr' parameter must be obeyed, and is not just a
		 * hint for a possible base address of the allocated range. */
		if (!addr)
			fatal("%s: no start specified for fixed mapping", __FUNCTION__);

		/* Any allocated page in the range specified by 'addr' and 'len'
		 * must be discarded. */
		mem_unmap(mem, addr, len_aligned);
	}
	else
	{
		if (!addr || mem_map_space_down(mem, addr, len_aligned) != addr)
			addr = SYS_MMAP_BASE_ADDRESS;
		addr = mem_map_space_down(mem, addr, len_aligned);
		if (addr == -1)
			fatal("%s: out of guest memory", __FUNCTION__);
	}

	/* Allocation of memory */
	mem_map(mem, addr, len_aligned, perm);

	/* Host mapping */
	if (host_fd >= 0)
	{
		char buf[MEM_PAGE_SIZE];

		unsigned int last_pos;
		unsigned int curr_addr;

		int size;
		int count;

		/* Save previous position */
		last_pos = lseek(host_fd, 0, SEEK_CUR);
		lseek(host_fd, offset, SEEK_SET);

		/* Read pages */
		assert(len_aligned % MEM_PAGE_SIZE == 0);
		assert(addr % MEM_PAGE_SIZE == 0);
		curr_addr = addr;
		for (size = len_aligned; size > 0; size -= MEM_PAGE_SIZE)
		{
			memset(buf, 0, MEM_PAGE_SIZE);
			count = read(host_fd, buf, MEM_PAGE_SIZE);
			if (count)
				mem_access(mem, curr_addr, MEM_PAGE_SIZE, buf, mem_access_init);
			curr_addr += MEM_PAGE_SIZE;
		}

		/* Return file to last position */
		lseek(host_fd, last_pos, SEEK_SET);
	}

	/* Return mapped address */
	return addr;
}




/*
 * System call 'munmap' (code 91)
 */

static int arm_sys_munmap_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int addr;
	unsigned int size;
	unsigned int size_aligned;

	/* Arguments */
	addr = regs->r0;
	size = regs->r1;
	arm_sys_debug("  addr=0x%x, size=0x%x\n", addr, size);

	/* Restrictions */
	if (addr & (MEM_PAGE_SIZE - 1))
		fatal("%s: address not aligned", __FUNCTION__);

	/* Unmap */
	size_aligned = ROUND_UP(size, MEM_PAGE_SIZE);
	mem_unmap(mem, addr, size_aligned);

	/* Return */
	return 0;
}




struct sim_user_desc
{
	unsigned int entry_number;
	unsigned int base_addr;
	unsigned int limit;
	unsigned int seg_32bit:1;
	unsigned int contents:2;
	unsigned int read_exec_only:1;
	unsigned int limit_in_pages:1;
	unsigned int seg_not_present:1;
	unsigned int useable:1;
};

/*
 * System call 'newuname' (code 122)
 */

struct sim_utsname
{
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
	char domainname[65];
} __attribute__((packed));

static struct sim_utsname sim_utsname =
{
	"Linux",
	"Multi2Sim",
	"3.1.9-1.fc16.armv7"
	"#1 Fri Jan 13 16:37:42 UTC 2012",
	"armv7"
	""
};

static int arm_sys_newuname_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int utsname_ptr;

	/* Arguments */
	utsname_ptr = regs->r0;
	arm_sys_debug("  putsname=0x%x\n", utsname_ptr);
	arm_sys_debug("  sysname='%s', nodename='%s'\n", sim_utsname.sysname, sim_utsname.nodename);
	arm_sys_debug("  relaese='%s', version='%s'\n", sim_utsname.release, sim_utsname.version);
	arm_sys_debug("  machine='%s', domainname='%s'\n", sim_utsname.machine, sim_utsname.domainname);

	/* Return structure */
	mem_write(mem, utsname_ptr, sizeof sim_utsname, &sim_utsname);
	return 0;
}




/*
 * System call 'mmap2' (code 192)
 */

static int arm_sys_mmap2_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs = ctx->regs;

	unsigned int addr;
	unsigned int len;

	int prot;
	int flags;
	int offset;
	int guest_fd;

	char prot_str[MAX_STRING_SIZE];
	char flags_str[MAX_STRING_SIZE];

	/* Arguments */
	addr = regs->r0;
	len = regs->r1;
	prot = regs->r2;
	flags = regs->r3;
	guest_fd = regs->r4;
	offset = regs->r5;

	/* Arm error handling for non-tls supported pointer mismatch */
	if(!len)
	{
		len = 0x1000;
	}

	/* Debug */
	arm_sys_debug("  addr=0x%x, len=%u, prot=0x%x, flags=0x%x, guest_fd=%d, offset=0x%x\n",
		addr, len, prot, flags, guest_fd, offset);
	str_map_flags(&sys_mmap_prot_map, prot, prot_str, MAX_STRING_SIZE);
	str_map_flags(&sys_mmap_flags_map, flags, flags_str, MAX_STRING_SIZE);
	arm_sys_debug("  prot=%s, flags=%s\n", prot_str, flags_str);

	/* System calls 'mmap' and 'mmap2' only differ in the interpretation of
	 * argument 'offset'. Here, it is given in memory pages. */
	return arm_sys_mmap(ctx, addr, len, prot, flags, guest_fd, offset << MEM_PAGE_SHIFT);
}




/*
 * System call 'stat64' (code 195)
 */

struct sim_stat64_t
{
	unsigned long long dev;  /* 0 8 */
	unsigned int pad1;  /* 8 4 */
	unsigned int __ino;  /* 12 4 */
	unsigned int mode;  /* 16 4 */
	unsigned int nlink;  /* 20 4 */
	unsigned int uid;  /* 24 4 */
	unsigned int gid;  /* 28 4 */
	unsigned long long rdev;  /* 32 8 */
	unsigned int pad2;  /* 40 4 */
	long long size;  /* 44 8 */
	unsigned int blksize;  /* 52 4 */
	unsigned long long blocks;  /* 56 8 */
	unsigned int atime;  /* 64 4 */
	unsigned int atime_nsec;  /* 68 4 */
	unsigned int mtime;  /* 72 4 */
	unsigned int mtime_nsec;  /* 76 4 */
	unsigned int ctime;  /* 80 4 */
	unsigned int ctime_nsec;  /* 84 4 */
	unsigned long long ino;  /* 88 8 */
} __attribute__((packed));

static void arm_sys_stat_host_to_guest(struct sim_stat64_t *guest, struct stat *host)
{
	M2S_HOST_GUEST_MATCH(sizeof(struct sim_stat64_t), 96);
	memset(guest, 0, sizeof(struct sim_stat64_t));

	guest->dev = host->st_dev;
	guest->__ino = host->st_ino;
	guest->mode = host->st_mode;
	guest->nlink = host->st_nlink;
	guest->uid = host->st_uid;
	guest->gid = host->st_gid;
	guest->rdev = host->st_rdev;
	guest->size = host->st_size;
	guest->blksize = host->st_blksize;
	guest->blocks = host->st_blocks;
	guest->atime = host->st_atime;
	guest->mtime = host->st_mtime;
	guest->ctime = host->st_ctime;
	guest->ino = host->st_ino;

	arm_sys_debug("  stat64 structure:\n");
	arm_sys_debug("    dev=%lld, ino=%lld, mode=%d, nlink=%d\n",
		guest->dev, guest->ino, guest->mode, guest->nlink);
	arm_sys_debug("    uid=%d, gid=%d, rdev=%lld\n",
		guest->uid, guest->gid, guest->rdev);
	arm_sys_debug("    size=%lld, blksize=%d, blocks=%lld\n",
		guest->size, guest->blksize, guest->blocks);
}




/*
 * System call 'fstat64' (code 197)
 */

static int arm_sys_fstat64_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	int fd;
	int host_fd;
	int err;

	unsigned int statbuf_ptr;

	struct stat statbuf;
	struct sim_stat64_t sim_statbuf;

	/* Arguments */
	fd = regs->r0;
	statbuf_ptr = regs->r1;
	arm_sys_debug("  fd=%d, statbuf_ptr=0x%x\n", fd, statbuf_ptr);

	/* Get host descriptor */
	host_fd = arm_file_desc_table_get_host_fd(ctx->file_desc_table, fd);
	arm_sys_debug("  host_fd=%d\n", host_fd);

	/* Host call */
	err = fstat(host_fd, &statbuf);
	if (err == -1)
		return -errno;

	/* Return */
	arm_sys_stat_host_to_guest(&sim_statbuf, &statbuf);
	mem_write(mem, statbuf_ptr, sizeof sim_statbuf, &sim_statbuf);
	return 0;
}




/*
 * System call 'getuid' (code 199)
 */

static int arm_sys_getuid_impl(struct arm_ctx_t *ctx)
{
	return getuid();
}



/*
 * System call 'getgid' (code 200)
 */

static int arm_sys_getgid_impl(struct arm_ctx_t *ctx)
{
	return getgid();
}




/*
 * System call 'geteuid' (code 201)
 */

static int arm_sys_geteuid_impl(struct arm_ctx_t *ctx)
{
	return geteuid();
}




/*
 * System call 'getegid' (code 202)
 */

static int arm_sys_getegid_impl(struct arm_ctx_t *ctx)
{
	return getegid();
}




/*
 * System call 'exit_group' (code 252)
 */

static int arm_sys_exit_group_impl(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs = ctx->regs;

	int status;

	/* Arguments */
	status = regs->r0;
	arm_sys_debug("  status=%d\n", status);

	/* Finish */
	arm_ctx_finish_group(ctx, status);
	return 0;
}




/*
 * System call 'ARM_set_tls' (code 330)
 */

static int arm_sys_ARM_set_tls_impl(struct arm_ctx_t *ctx)
{

	unsigned int newtls;

	/* Arguments */
	newtls = ctx->regs->r0;

	/* Set the tls value */
	ctx->regs->cp15.c13_tls3 = newtls;


	/*struct arm_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int uinfo_ptr;

	struct sim_user_desc uinfo;

	 Arguments
	uinfo_ptr = regs->r0;
	arm_sys_debug("  uinfo_ptr=0x%x\n", uinfo_ptr);

	 Read structure
	mem_read(mem, uinfo_ptr, sizeof uinfo, &uinfo);
	arm_sys_debug("  entry_number=0x%x, base_addr=0x%x, limit=0x%x\n",
		uinfo.entry_number, uinfo.base_addr, uinfo.limit);
	arm_sys_debug("  seg_32bit=0x%x, contents=0x%x, read_exec_only=0x%x\n",
		uinfo.seg_32bit, uinfo.contents, uinfo.read_exec_only);
	arm_sys_debug("  limit_in_pages=0x%x, seg_not_present=0x%x, useable=0x%x\n",
		uinfo.limit_in_pages, uinfo.seg_not_present, uinfo.useable);
	if (!uinfo.seg_32bit)
		fatal("syscall set_thread_area: only 32-bit segments supported");

	 Limit given in pages (4KB units)
	if (uinfo.limit_in_pages)
		uinfo.limit <<= 12;

	if (uinfo.entry_number == -1)
	{
		if (ctx->glibc_segment_base)
			fatal("%s: glibc segment already set", __FUNCTION__);

		ctx->glibc_segment_base = uinfo.base_addr;
		ctx->glibc_segment_limit = uinfo.limit;
		uinfo.entry_number = 6;
		mem_write(mem, uinfo_ptr, 4, &uinfo.entry_number);
	}
	else
	{
		if (uinfo.entry_number != 6)
			fatal("%s: invalid entry number", __FUNCTION__);
		if (!ctx->glibc_segment_base)
			fatal("%s: glibc segment not set", __FUNCTION__);
		ctx->glibc_segment_base = uinfo.base_addr;
		ctx->glibc_segment_limit = uinfo.limit;
	}
*/
	/* Return */
	return 0;

}




/*
 * Not implemented system calls
 */

#define SYS_NOT_IMPL(NAME) \
	static int arm_sys_##NAME##_impl(struct arm_ctx_t *ctx) \
	{ \
		struct arm_regs_t *regs = ctx->regs; \
		fatal("%s: system call not implemented (code %d, inst %lld, pid %d).\n%s", \
			__FUNCTION__, regs->r7, asEmu(arm_emu)->instructions, ctx->pid, \
			err_arm_sys_note); \
		return 0; \
	}

SYS_NOT_IMPL(restart_syscall)
SYS_NOT_IMPL(exit)
SYS_NOT_IMPL(fork)
SYS_NOT_IMPL(waitpid)
SYS_NOT_IMPL(creat)
SYS_NOT_IMPL(link)
SYS_NOT_IMPL(unlink)
SYS_NOT_IMPL(execve)
SYS_NOT_IMPL(chdir)
SYS_NOT_IMPL(time)
SYS_NOT_IMPL(mknod)
SYS_NOT_IMPL(chmod)
SYS_NOT_IMPL(lchown16)
SYS_NOT_IMPL(ni_syscall_17)
SYS_NOT_IMPL(stat)
SYS_NOT_IMPL(lseek)
SYS_NOT_IMPL(getpid)
SYS_NOT_IMPL(mount)
SYS_NOT_IMPL(oldumount)
SYS_NOT_IMPL(setuid16)
SYS_NOT_IMPL(getuid16)
SYS_NOT_IMPL(stime)
SYS_NOT_IMPL(ptrace)
SYS_NOT_IMPL(alarm)
SYS_NOT_IMPL(fstat)
SYS_NOT_IMPL(pause)
SYS_NOT_IMPL(utime)
SYS_NOT_IMPL(ni_syscall_31)
SYS_NOT_IMPL(ni_syscall_32)
SYS_NOT_IMPL(access)
SYS_NOT_IMPL(nice)
SYS_NOT_IMPL(ni_syscall_35)
SYS_NOT_IMPL(sync)
SYS_NOT_IMPL(kill)
SYS_NOT_IMPL(rename)
SYS_NOT_IMPL(mkdir)
SYS_NOT_IMPL(rmdir)
SYS_NOT_IMPL(dup)
SYS_NOT_IMPL(pipe)
SYS_NOT_IMPL(times)
SYS_NOT_IMPL(ni_syscall_44)
SYS_NOT_IMPL(setgid16)
SYS_NOT_IMPL(getgid16)
SYS_NOT_IMPL(signal)
SYS_NOT_IMPL(geteuid16)
SYS_NOT_IMPL(getegid16)
SYS_NOT_IMPL(acct)
SYS_NOT_IMPL(umount)
SYS_NOT_IMPL(ni_syscall_53)
SYS_NOT_IMPL(ioctl)
SYS_NOT_IMPL(fcntl)
SYS_NOT_IMPL(ni_syscall_56)
SYS_NOT_IMPL(setpgid)
SYS_NOT_IMPL(ni_syscall_58)
SYS_NOT_IMPL(olduname)
SYS_NOT_IMPL(umask)
SYS_NOT_IMPL(chroot)
SYS_NOT_IMPL(ustat)
SYS_NOT_IMPL(dup2)
SYS_NOT_IMPL(getppid)
SYS_NOT_IMPL(getpgrp)
SYS_NOT_IMPL(setsid)
SYS_NOT_IMPL(sigaction)
SYS_NOT_IMPL(sgetmask)
SYS_NOT_IMPL(ssetmask)
SYS_NOT_IMPL(setreuid16)
SYS_NOT_IMPL(setregid16)
SYS_NOT_IMPL(sigsuspend)
SYS_NOT_IMPL(sigpending)
SYS_NOT_IMPL(sethostname)
SYS_NOT_IMPL(setrlimit)
SYS_NOT_IMPL(old_getrlimit)
SYS_NOT_IMPL(getrusage)
SYS_NOT_IMPL(settimeofday)
SYS_NOT_IMPL(getgroups16)
SYS_NOT_IMPL(setgroups16)
SYS_NOT_IMPL(oldselect)
SYS_NOT_IMPL(symlink)
SYS_NOT_IMPL(lstat)
SYS_NOT_IMPL(readlink)
SYS_NOT_IMPL(uselib)
SYS_NOT_IMPL(swapon)
SYS_NOT_IMPL(reboot)
SYS_NOT_IMPL(readdir)
SYS_NOT_IMPL(mmap)
SYS_NOT_IMPL(truncate)
SYS_NOT_IMPL(ftruncate)
SYS_NOT_IMPL(fchmod)
SYS_NOT_IMPL(fchown16)
SYS_NOT_IMPL(getpriority)
SYS_NOT_IMPL(setpriority)
SYS_NOT_IMPL(ni_syscall_98)
SYS_NOT_IMPL(statfs)
SYS_NOT_IMPL(fstatfs)
SYS_NOT_IMPL(ioperm)
SYS_NOT_IMPL(socketcall)
SYS_NOT_IMPL(syslog)
SYS_NOT_IMPL(setitimer)
SYS_NOT_IMPL(getitimer)
SYS_NOT_IMPL(newstat)
SYS_NOT_IMPL(newlstat)
SYS_NOT_IMPL(newfstat)
SYS_NOT_IMPL(uname)
SYS_NOT_IMPL(iopl)
SYS_NOT_IMPL(vhangup)
SYS_NOT_IMPL(ni_syscall_112)
SYS_NOT_IMPL(vm86old)
SYS_NOT_IMPL(wait4)
SYS_NOT_IMPL(swapoff)
SYS_NOT_IMPL(sysinfo)
SYS_NOT_IMPL(ipc)
SYS_NOT_IMPL(fsync)
SYS_NOT_IMPL(sigreturn)
SYS_NOT_IMPL(clone)
SYS_NOT_IMPL(setdomainname)
SYS_NOT_IMPL(modify_ldt)
SYS_NOT_IMPL(adjtimex)
SYS_NOT_IMPL(mprotect)
SYS_NOT_IMPL(sigprocmask)
SYS_NOT_IMPL(ni_syscall_127)
SYS_NOT_IMPL(init_module)
SYS_NOT_IMPL(delete_module)
SYS_NOT_IMPL(ni_syscall_130)
SYS_NOT_IMPL(quotactl)
SYS_NOT_IMPL(getpgid)
SYS_NOT_IMPL(fchdir)
SYS_NOT_IMPL(bdflush)
SYS_NOT_IMPL(sysfs)
SYS_NOT_IMPL(personality)
SYS_NOT_IMPL(ni_syscall_137)
SYS_NOT_IMPL(setfsuid16)
SYS_NOT_IMPL(setfsgid16)
SYS_NOT_IMPL(llseek)
SYS_NOT_IMPL(getdents)
SYS_NOT_IMPL(select)
SYS_NOT_IMPL(flock)
SYS_NOT_IMPL(msync)
SYS_NOT_IMPL(readv)
SYS_NOT_IMPL(writev)
SYS_NOT_IMPL(getsid)
SYS_NOT_IMPL(fdatasync)
SYS_NOT_IMPL(sysctl)
SYS_NOT_IMPL(mlock)
SYS_NOT_IMPL(munlock)
SYS_NOT_IMPL(mlockall)
SYS_NOT_IMPL(munlockall)
SYS_NOT_IMPL(sched_setparam)
SYS_NOT_IMPL(sched_getparam)
SYS_NOT_IMPL(sched_setscheduler)
SYS_NOT_IMPL(sched_getscheduler)
SYS_NOT_IMPL(sched_yield)
SYS_NOT_IMPL(sched_get_priority_max)
SYS_NOT_IMPL(sched_get_priority_min)
SYS_NOT_IMPL(sched_rr_get_interval)
SYS_NOT_IMPL(nanosleep)
SYS_NOT_IMPL(mremap)
SYS_NOT_IMPL(setresuid16)
SYS_NOT_IMPL(getresuid16)
SYS_NOT_IMPL(vm86)
SYS_NOT_IMPL(ni_syscall_167)
SYS_NOT_IMPL(poll)
SYS_NOT_IMPL(nfsservctl)
SYS_NOT_IMPL(setresgid16)
SYS_NOT_IMPL(getresgid16)
SYS_NOT_IMPL(prctl)
SYS_NOT_IMPL(rt_sigreturn)
SYS_NOT_IMPL(rt_sigaction)
SYS_NOT_IMPL(rt_sigprocmask)
SYS_NOT_IMPL(rt_sigpending)
SYS_NOT_IMPL(rt_sigtimedwait)
SYS_NOT_IMPL(rt_sigqueueinfo)
SYS_NOT_IMPL(rt_sigsuspend)
SYS_NOT_IMPL(pread64)
SYS_NOT_IMPL(pwrite64)
SYS_NOT_IMPL(chown16)
SYS_NOT_IMPL(getcwd)
SYS_NOT_IMPL(capget)
SYS_NOT_IMPL(capset)
SYS_NOT_IMPL(sigaltstack)
SYS_NOT_IMPL(sendfile)
SYS_NOT_IMPL(ni_syscall_188)
SYS_NOT_IMPL(ni_syscall_189)
SYS_NOT_IMPL(vfork)
SYS_NOT_IMPL(getrlimit)
SYS_NOT_IMPL(truncate64)
SYS_NOT_IMPL(ftruncate64)
SYS_NOT_IMPL(stat64)
SYS_NOT_IMPL(lstat64)
SYS_NOT_IMPL(lchown)
SYS_NOT_IMPL(setreuid)
SYS_NOT_IMPL(setregid)
SYS_NOT_IMPL(getgroups)
SYS_NOT_IMPL(setgroups)
SYS_NOT_IMPL(fchown)
SYS_NOT_IMPL(setresuid)
SYS_NOT_IMPL(getresuid)
SYS_NOT_IMPL(setresgid)
SYS_NOT_IMPL(getresgid)
SYS_NOT_IMPL(chown)
SYS_NOT_IMPL(setuid)
SYS_NOT_IMPL(setgid)
SYS_NOT_IMPL(setfsuid)
SYS_NOT_IMPL(setfsgid)
SYS_NOT_IMPL(pivot_root)
SYS_NOT_IMPL(mincore)
SYS_NOT_IMPL(madvise)
SYS_NOT_IMPL(getdents64)
SYS_NOT_IMPL(fcntl64)
SYS_NOT_IMPL(ni_syscall_222)
SYS_NOT_IMPL(ni_syscall_223)
SYS_NOT_IMPL(gettid)
SYS_NOT_IMPL(readahead)
SYS_NOT_IMPL(setxattr)
SYS_NOT_IMPL(lsetxattr)
SYS_NOT_IMPL(fsetxattr)
SYS_NOT_IMPL(getxattr)
SYS_NOT_IMPL(lgetxattr)
SYS_NOT_IMPL(fgetxattr)
SYS_NOT_IMPL(listxattr)
SYS_NOT_IMPL(llistxattr)
SYS_NOT_IMPL(flistxattr)
SYS_NOT_IMPL(removexattr)
SYS_NOT_IMPL(lremovexattr)
SYS_NOT_IMPL(fremovexattr)
SYS_NOT_IMPL(tkill)
SYS_NOT_IMPL(sendfile64)
SYS_NOT_IMPL(futex)
SYS_NOT_IMPL(sched_setaffinity)
SYS_NOT_IMPL(sched_getaffinity)
SYS_NOT_IMPL(set_thread_area)
SYS_NOT_IMPL(get_thread_area)
SYS_NOT_IMPL(io_setup)
SYS_NOT_IMPL(io_destroy)
SYS_NOT_IMPL(io_getevents)
SYS_NOT_IMPL(io_submit)
SYS_NOT_IMPL(io_cancel)
SYS_NOT_IMPL(fadvise64)
SYS_NOT_IMPL(ni_syscall_251)
SYS_NOT_IMPL(lookup_dcookie)
SYS_NOT_IMPL(epoll_create)
SYS_NOT_IMPL(epoll_ctl)
SYS_NOT_IMPL(epoll_wait)
SYS_NOT_IMPL(remap_file_pages)
SYS_NOT_IMPL(set_tid_address)
SYS_NOT_IMPL(timer_create)
SYS_NOT_IMPL(timer_settime)
SYS_NOT_IMPL(timer_gettime)
SYS_NOT_IMPL(timer_getoverrun)
SYS_NOT_IMPL(timer_delete)
SYS_NOT_IMPL(clock_settime)
SYS_NOT_IMPL(clock_gettime)
SYS_NOT_IMPL(clock_getres)
SYS_NOT_IMPL(clock_nanosleep)
SYS_NOT_IMPL(statfs64)
SYS_NOT_IMPL(fstatfs64)
SYS_NOT_IMPL(tgkill)
SYS_NOT_IMPL(utimes)
SYS_NOT_IMPL(fadvise64_64)
SYS_NOT_IMPL(ni_syscall_273)
SYS_NOT_IMPL(mbind)
SYS_NOT_IMPL(get_mempolicy)
SYS_NOT_IMPL(set_mempolicy)
SYS_NOT_IMPL(mq_open)
SYS_NOT_IMPL(mq_unlink)
SYS_NOT_IMPL(mq_timedsend)
SYS_NOT_IMPL(mq_timedreceive)
SYS_NOT_IMPL(mq_notify)
SYS_NOT_IMPL(mq_getsetattr)
SYS_NOT_IMPL(kexec_load)
SYS_NOT_IMPL(waitid)
SYS_NOT_IMPL(ni_syscall_285)
SYS_NOT_IMPL(add_key)
SYS_NOT_IMPL(request_key)
SYS_NOT_IMPL(keyctl)
SYS_NOT_IMPL(ioprio_set)
SYS_NOT_IMPL(ioprio_get)
SYS_NOT_IMPL(inotify_init)
SYS_NOT_IMPL(inotify_add_watch)
SYS_NOT_IMPL(inotify_rm_watch)
SYS_NOT_IMPL(migrate_pages)
SYS_NOT_IMPL(openat)
SYS_NOT_IMPL(mkdirat)
SYS_NOT_IMPL(mknodat)
SYS_NOT_IMPL(fchownat)
SYS_NOT_IMPL(futimesat)
SYS_NOT_IMPL(fstatat64)
SYS_NOT_IMPL(unlinkat)
SYS_NOT_IMPL(renameat)
SYS_NOT_IMPL(linkat)
SYS_NOT_IMPL(symlinkat)
SYS_NOT_IMPL(readlinkat)
SYS_NOT_IMPL(fchmodat)
SYS_NOT_IMPL(faccessat)
SYS_NOT_IMPL(pselect6)
SYS_NOT_IMPL(ppoll)
SYS_NOT_IMPL(unshare)
SYS_NOT_IMPL(set_robust_list)
SYS_NOT_IMPL(get_robust_list)
SYS_NOT_IMPL(splice)
SYS_NOT_IMPL(sync_file_range)
SYS_NOT_IMPL(tee)
SYS_NOT_IMPL(vmsplice)
SYS_NOT_IMPL(move_pages)
SYS_NOT_IMPL(getcpu)
SYS_NOT_IMPL(epoll_pwait)
SYS_NOT_IMPL(utimensat)
SYS_NOT_IMPL(signalfd)
SYS_NOT_IMPL(timerfd)
SYS_NOT_IMPL(eventfd)
SYS_NOT_IMPL(fallocate)
SYS_NOT_IMPL(opencl)
SYS_NOT_IMPL(glut)
SYS_NOT_IMPL(opengl)
SYS_NOT_IMPL(cuda)
SYS_NOT_IMPL(clrt)

