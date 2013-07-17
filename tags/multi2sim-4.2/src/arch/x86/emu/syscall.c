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
#include <syscall.h>
#include <unistd.h>
#include <utime.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/time.h>
#include <sys/times.h>

#include <arch/common/runtime.h>
#include <arch/x86/timing/cpu.h>
#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/bit-map.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "context.h"
#include "emu.h"
#include "file-desc.h"
#include "isa.h"
#include "loader.h"
#include "regs.h"
#include "signal.h"
#include "syscall.h"



/*
 * Public Variables
 */


int x86_sys_debug_category;




/*
 * Private Variables
 */


static char *err_x86_sys_note =
	"\tThe system calls performed by the executed application are intercepted by\n"
	"\tMulti2Sim and emulated in file 'syscall.c'. The most common system calls are\n"
	"\tcurrently supported, but your application might perform specific unsupported\n"
	"\tsystem calls or combinations of parameters. To request support for a given\n"
	"\tsystem call, please email 'development@multi2sim.org'.\n";


 /* System call names */
static char *x86_sys_call_name[] =
{
#define DEFSYSCALL(name, code) #name,
#include "syscall.dat"
#undef DEFSYSCALL
	""
};


/* System call codes */
enum
{
#define DEFSYSCALL(name, code) x86_sys_code_##name = code,
#include "syscall.dat"
#undef DEFSYSCALL
	x86_sys_code_count
};


/* Forward declarations of system calls */
#define DEFSYSCALL(name, code) \
	static int x86_sys_##name##_impl(X86Context *ctx);
#include "syscall.dat"
#undef DEFSYSCALL


/* System call functions */
static int (*x86_sys_call_func[x86_sys_code_count + 1])(X86Context *ctx) =
{
#define DEFSYSCALL(name, code) x86_sys_##name##_impl,
#include "syscall.dat"
#undef DEFSYSCALL
	NULL
};


/* Statistics */
static int x86_sys_call_freq[x86_sys_code_count + 1];




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

static struct str_map_t x86_sys_error_code_map =
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


void x86_sys_dump_stats(FILE *f)
{
	int i;

	/* Header */
	fprintf(f, "\n\n**\n** System calls summary:\n**\n\n");
	fprintf(f, "%-20s %s\n", "System call", "Count");
	for (i = 0; i < 30; i++)
		fprintf(f, "-");
	fprintf(f, "\n");

	/* Summary */
	for (i = 1; i <= x86_sys_code_count; i++)
		if (x86_sys_call_freq[i])
			fprintf(f, "%-20s %d\n", x86_sys_call_name[i],
					x86_sys_call_freq[i]);
	fprintf(f, "\n");
}


void X86ContextSyscall(X86Context *self)
{
	X86Emu *emu = self->emu;

	struct x86_regs_t *regs = self->regs;
	struct runtime_t *runtime;

	int code;
	int err;

	/* Get system call code from 'eax' */
	code = regs->eax;

	/* Check for special system call codes outside of the standard range
	 * defined in 'syscall.dat'. */
	if (code < 1 || code >= x86_sys_code_count)
	{
		/* Check if it is a special code registered by a runtime ABI */
		runtime = runtime_get_from_syscall_code(code);
		if (!runtime)
			fatal("%s: invalid system call code (%d)", __FUNCTION__, code);

		/* Debug */
		x86_sys_debug("%s runtime ABI call (code %d, inst %lld, pid %d)\n",
			runtime->name, code, asEmu(emu)->instructions, self->pid);

		/* Run runtime ABI call */
		err = runtime_abi_call(runtime, self);

		/* Set return value in 'eax'. */
		regs->eax = err;

		/* Debug and done */
		x86_sys_debug("  ret=(%d, 0x%x)\n", err, err);
		return;
	}

	/* Statistics */
	x86_sys_call_freq[code]++;

	/* Debug */
	x86_sys_debug("system call '%s' (code %d, inst %lld, pid %d)\n",
		x86_sys_call_name[code], code, asEmu(emu)->instructions, self->pid);
	X86ContextDebugCall("system call '%s' (code %d, inst %lld, pid %d)\n",
		x86_sys_call_name[code], code, asEmu(emu)->instructions, self->pid);

	/* Perform system call */
	err = x86_sys_call_func[code](self);

	/* Set return value in 'eax', except for 'sigreturn' system call. Also, if the
	 * context got suspended, the wake up routine will set the return value. */
	if (code != x86_sys_code_sigreturn && !X86ContextGetState(self, X86ContextSuspended))
		regs->eax = err;

	/* Debug */
	x86_sys_debug("  ret=(%d, 0x%x)", err, err);
	if (err < 0 && err >= -SIM_ERRNO_MAX)
		x86_sys_debug(", errno=%s)", str_map_value(&x86_sys_error_code_map, -err));
	x86_sys_debug("\n");
}






/*
 * System call 'exit' (code 1)
 */

static int x86_sys_exit_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	int status;

	/* Arguments */
	status = regs->ebx;
	x86_sys_debug("  status=0x%x\n", status);

	/* Finish context */
	X86ContextFinish(ctx, status);
	return 0;
}




/*
 * System call 'close' (code 2)
 */

static int x86_sys_close_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct x86_file_desc_t *fd;

	int guest_fd;
	int host_fd;

	/* Arguments */
	guest_fd = regs->ebx;
	x86_sys_debug("  guest_fd=%d\n", guest_fd);
	host_fd = x86_file_desc_table_get_host_fd(ctx->file_desc_table, guest_fd);
	x86_sys_debug("  host_fd=%d\n", host_fd);

	/* Get file descriptor table entry. */
	fd = x86_file_desc_table_entry_get(ctx->file_desc_table, guest_fd);
	if (!fd)
		return -EBADF;

	/* Close host file descriptor only if it is valid and not stdin/stdout/stderr. */
	if (host_fd > 2)
		close(host_fd);

	/* Free guest file descriptor. This will delete the host file if it's a virtual file. */
	if (fd->kind == file_desc_virtual)
		x86_sys_debug("    host file '%s': temporary file deleted\n", fd->path);
	x86_file_desc_table_entry_free(ctx->file_desc_table, fd->guest_fd);

	/* Success */
	return 0;
}



/*
 * System call 'read' (code 3)
 */

static int x86_sys_read_impl(X86Context *ctx)
{
	X86Emu *emu = ctx->emu;

	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int buf_ptr;
	unsigned int count;

	int guest_fd;
	int host_fd;
	int err;

	void *buf;

	struct x86_file_desc_t *fd;
	struct pollfd fds;

	/* Arguments */
	guest_fd = regs->ebx;
	buf_ptr = regs->ecx;
	count = regs->edx;
	x86_sys_debug("  guest_fd=%d, buf_ptr=0x%x, count=0x%x\n",
		guest_fd, buf_ptr, count);

	/* Get file descriptor */
	fd = x86_file_desc_table_entry_get(ctx->file_desc_table, guest_fd);
	if (!fd)
		return -EBADF;
	host_fd = fd->host_fd;
	x86_sys_debug("  host_fd=%d\n", host_fd);

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
			x86_sys_debug_buffer("  buf", buf, err);
		}

		/* Return number of read bytes */
		free(buf);
		return err;
	}

	/* Blocking read - suspend thread */
	x86_sys_debug("  blocking read - process suspended\n");
	ctx->wakeup_fd = guest_fd;
	ctx->wakeup_events = 1;  /* POLLIN */
	X86ContextSetState(ctx, X86ContextSuspended | X86ContextRead);
	X86EmuProcessEventsSchedule(emu);

	/* Free allocated buffer. Return value doesn't matter,
	 * it will be overwritten when context wakes up from blocking call. */
	free(buf);
	return 0;
}




/*
 * System call 'write' (code 4)
 */

static int x86_sys_write_impl(X86Context *ctx)
{
	X86Emu *emu = ctx->emu;

	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int buf_ptr;
	unsigned int count;

	int guest_fd;
	int host_fd;
	int err;

	struct x86_file_desc_t *desc;
	void *buf;

	struct pollfd fds;

	/* Arguments */
	guest_fd = regs->ebx;
	buf_ptr = regs->ecx;
	count = regs->edx;
	x86_sys_debug("  guest_fd=%d, buf_ptr=0x%x, count=0x%x\n",
		guest_fd, buf_ptr, count);

	/* Get file descriptor */
	desc = x86_file_desc_table_entry_get(ctx->file_desc_table, guest_fd);
	if (!desc)
		return -EBADF;
	host_fd = desc->host_fd;
	x86_sys_debug("  host_fd=%d\n", host_fd);

	/* Read buffer from memory */
	buf = xcalloc(1, count);
	mem_read(mem, buf_ptr, count, buf);
	x86_sys_debug_buffer("  buf", buf, count);

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
	x86_sys_debug("  blocking write - process suspended\n");
	ctx->wakeup_fd = guest_fd;
	X86ContextSetState(ctx, X86ContextSuspended | X86ContextWrite);
	X86EmuProcessEventsSchedule(emu);

	/* Return value doesn't matter here. It will be overwritten when the
	 * context wakes up after blocking call. */
	free(buf);
	return 0;
}




/*
 * System call 'open' (code 5)
 */

static struct str_map_t sys_open_flags_map =
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

static struct x86_file_desc_t *x86_sys_open_virtual(X86Context *ctx,
		char *path, int flags, int mode)
{
	char temp_path[MAX_PATH_SIZE];
	struct x86_file_desc_t *desc;
	int host_fd;

	/* Assume no file found */
	temp_path[0] = '\0';

	/* Virtual file /proc/self/maps */
	if (!strcmp(path, "/proc/self/maps"))
		X86ContextProcSelfMaps(ctx, temp_path, sizeof temp_path);
	
	/* Virtual file /proc/cpuinfo */
	else if (!strcmp(path, "/proc/cpuinfo"))
		X86ContextProcCPUInfo(ctx, temp_path, sizeof temp_path);

	/* No file found */
	if (!temp_path[0])
		return NULL;

	/* File found, create descriptor */
	host_fd = open(temp_path, flags, mode);
	assert(host_fd > 0);

	/* Add file descriptor table entry. */
	desc = x86_file_desc_table_entry_new(ctx->file_desc_table, file_desc_virtual, host_fd, temp_path, flags);
	x86_sys_debug("    host file '%s' opened: guest_fd=%d, host_fd=%d\n",
			temp_path, desc->guest_fd, desc->host_fd);
	return desc;
}

static int x86_sys_open_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	struct x86_file_desc_t *desc;

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
	file_name_ptr = regs->ebx;
	flags = regs->ecx;
	mode = regs->edx;
	length = mem_read_string(mem, file_name_ptr, sizeof file_name, file_name);
	if (length >= MAX_PATH_SIZE)
		fatal("syscall open: maximum path length exceeded");
	X86ContextGetFullPath(ctx, file_name, full_path, sizeof full_path);
	x86_sys_debug("  filename='%s' flags=0x%x, mode=0x%x\n",
		file_name, flags, mode);
	x86_sys_debug("  fullpath='%s'\n", full_path);
	str_map_flags(&sys_open_flags_map, flags, flags_str, sizeof flags_str);
	x86_sys_debug("  flags=%s\n", flags_str);
	
	/* The dynamic linker uses the 'open' system call to open shared libraries.
	 * We need to intercept here attempts to access runtime libraries and
	 * redirect them to our own Multi2Sim runtimes. */
	if (runtime_redirect(full_path, temp_path, sizeof temp_path))
		snprintf(full_path, sizeof full_path, "%s", temp_path);

	/* Virtual files */
	if (!strncmp(full_path, "/proc/", 6))
	{
		/* Attempt to open virtual file */
		desc = x86_sys_open_virtual(ctx, full_path, flags, mode);
		if (desc)
			return desc->guest_fd;
		
		/* Unhandled virtual file. Let the application read the contents of the host
		 * version of the file as if it was a regular file. */
		x86_sys_debug("    warning: unhandled virtual file\n");
	}

	/* Regular file. */
	host_fd = open(full_path, flags, mode);
	if (host_fd == -1)
		return -errno;

	/* File opened, create a new file descriptor. */
	desc = x86_file_desc_table_entry_new(ctx->file_desc_table,
		file_desc_regular, host_fd, full_path, flags);
	x86_sys_debug("    file descriptor opened: guest_fd=%d, host_fd=%d\n",
		desc->guest_fd, desc->host_fd);

	/* Return guest descriptor index */
	return desc->guest_fd;
}




/*
 * System call 'waitpid' (code 7)
 */

static struct str_map_t sys_waitpid_options_map =
{
	8, {
		{ "WNOHANG",       0x00000001 },
		{ "WUNTRACED",     0x00000002 },
		{ "WEXITED",       0x00000004 },
		{ "WCONTINUED",    0x00000008 },
		{ "WNOWAIT",       0x01000000 },
		{ "WNOTHREAD",     0x20000000 },
		{ "WALL",          0x40000000 },
		{ "WCLONE",        0x80000000 }
	}
};

static int x86_sys_waitpid_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	X86Context *child;

	int pid;
	int options;
	unsigned int status_ptr;

	char options_str[MAX_STRING_SIZE];

	/* Arguments */
	pid = regs->ebx;
	status_ptr = regs->ecx;
	options = regs->edx;
	x86_sys_debug("  pid=%d, pstatus=0x%x, options=0x%x\n",
		pid, status_ptr, options);
	str_map_flags(&sys_waitpid_options_map, options, options_str, sizeof options_str);
	x86_sys_debug("  options=%s\n", options_str);

	/* Supported values for 'pid' */
	if (pid != -1 && pid <= 0)
		fatal("%s: only supported for pid=-1 or pid > 0.\n%s",
			__FUNCTION__, err_x86_sys_note);

	/* Look for a zombie child. */
	child = X86ContextGetZombie(ctx, pid);

	/* If there is no child and the flag WNOHANG was not specified,
	 * we get suspended until the specified child finishes. */
	if (!child && !(options & 0x1))
	{
		ctx->wakeup_pid = pid;
		X86ContextSetState(ctx, X86ContextSuspended | X86ContextWaitpid);
		return 0;
	}

	/* Context is not suspended. WNOHANG was specified, or some child
	 * was found in the zombie list. */
	if (child)
	{
		if (status_ptr)
			mem_write(mem, status_ptr, 4, &child->exit_code);
		X86ContextSetState(child, X86ContextFinished);
		return child->pid;
	}

	/* Return */
	return 0;
}




/*
 * System call 'unlink' (code 10)
 */

static int x86_sys_unlink_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int file_name_ptr;

	int length;
	int err;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];

	/* Arguments */
	file_name_ptr = regs->ebx;
	length = mem_read_string(mem, file_name_ptr, sizeof file_name, file_name);
	if (length >= MAX_PATH_SIZE)
		fatal("%s: buffer too small", __FUNCTION__);
	X86ContextGetFullPath(ctx, file_name, full_path, sizeof full_path);
	x86_sys_debug("  file_name_ptr=0x%x\n", file_name_ptr);
	x86_sys_debug("  file_name=%s, full_path=%s\n", file_name, full_path);

	/* Host call */
	err = unlink(full_path);
	if (err == -1)
		return -errno;

	/* Return */
	return 0;
}




/*
 * System call 'execve' (code 11)
 */

static char *err_sys_execve_note =
	"\tA system call 'execve' is trying to run a command prefixed with '/bin/sh -c'.\n"
	"\tThis is usually the result of the execution of the 'system()' function from\n"
	"\tthe guest application to run a shell command. Multi2Sim will execute this\n"
	"\tcommand natively, and then finish the calling context.\n";

static int x86_sys_execve_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int name_ptr;
	unsigned int argv;
	unsigned int envp;
	unsigned int regs_ptr;

	char name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];
	int length;

	struct list_t *arg_list;
	char arg_str[MAX_STRING_SIZE];
	char *arg;

	char env[MAX_LONG_STRING_SIZE];
	int i;

	/* Arguments */
	name_ptr = regs->ebx;
	argv = regs->ecx;
	envp = regs->edx;
	regs_ptr = regs->esi;
	x86_sys_debug("  name_ptr=0x%x, argv=0x%x, envp=0x%x, regs=0x%x\n",
		name_ptr, argv, envp, regs_ptr);

	/* Get command name */
	length = mem_read_string(mem, name_ptr, sizeof name, name);
	if (length >= sizeof name)
		fatal("%s: buffer too small", __FUNCTION__);
	X86ContextGetFullPath(ctx, name, full_path, sizeof full_path);
	x86_sys_debug("  name='%s', full_path='%s'\n", name, full_path);

	/* Arguments */
	arg_list = list_create();
	for (;;)
	{
		unsigned int arg_ptr;

		/* Argument pointer */
		mem_read(mem, argv + arg_list->count * 4, 4, &arg_ptr);
		if (!arg_ptr)
			break;

		/* Argument */
		length = mem_read_string(mem, arg_ptr, sizeof arg_str, arg_str);
		if (length >= sizeof arg_str)
			fatal("%s: buffer too small", __FUNCTION__);

		/* Add to argument list */
		arg = xstrdup(arg_str);
		list_add(arg_list, arg);
		x86_sys_debug("    argv[%d]='%s'\n", arg_list->count, arg);
	}

	/* Environment variables */
	x86_sys_debug("\n");
	for (i = 0; ; i++)
	{
		unsigned int env_ptr;

		/* Variable pointer */
		mem_read(mem, envp + i * 4, 4, &env_ptr);
		if (!env_ptr)
			break;

		/* Variable */
		length = mem_read_string(mem, env_ptr, sizeof env, env);
		if (length >= sizeof env)
			fatal("%s: buffer too small", __FUNCTION__);

		/* Debug */
		x86_sys_debug("    envp[%d]='%s'\n", i, env);
	}

	/* In the special case that the command line is 'sh -c <...>', this system
	 * call is the result of a program running the 'system' libc function. The
	 * host and guest architecture might be different and incompatible, so the
	 * safest option here is running the system command natively.
	 */
	if (!strcmp(full_path, "/bin/sh") && list_count(arg_list) == 3 &&
		!strcmp(list_get(arg_list, 0), "sh") &&
		!strcmp(list_get(arg_list, 1), "-c"))
	{
		int exit_code;

		/* Execute program natively and finish context */
		warning("%s: child context executed natively.\n%s",
			__FUNCTION__, err_sys_execve_note);
		exit_code = system(list_get(arg_list, 2));
		X86ContextFinish(ctx, exit_code);

		/* Free arguments and exit */
		for (i = 0; i < list_count(arg_list); i++)
			free(list_get(arg_list, i));
		list_free(arg_list);
		return 0;
	}

	/* Free arguments */
	for (i = 0; i < list_count(arg_list); i++)
		free(list_get(arg_list, i));
	list_free(arg_list);

	/* Return */
	fatal("%s: not implemented.\n%s", __FUNCTION__, err_x86_sys_note);
	return 0;
}




/*
 * System call 'chdir' (code 12)
 */

static int x86_sys_chdir_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct x86_loader_t *loader = ctx->loader;
	struct mem_t *mem = ctx->mem;

	char path[MAX_STRING_SIZE];
	char old_host_path[MAX_STRING_SIZE];

	unsigned int path_ptr;

	int length;
	int err;

	/* Arguments */
	path_ptr = regs->ebx;
	x86_sys_debug("  path_ptr=0x%x\n", path_ptr);

	/* Read path */
	length = mem_read_string(mem, path_ptr, sizeof path, path);
	if (length >= sizeof path)
		fatal("%s: buffer 'path' too small", __FUNCTION__);
	x86_sys_debug("  path='%s'\n", path);

	/* Save old host path */
	if (!getcwd(old_host_path, sizeof old_host_path))
		fatal("%s: buffer 'old_host_path' too small", __FUNCTION__);

	/* Change host path to guest working directory */
	if (chdir(loader->cwd))
		fatal("%s: %s: cannot cd to guest working directory",
			__FUNCTION__, loader->cwd);
	
	/* Change to specified directory */
	err = chdir(path);
	if (!err)
	{
		if (!getcwd(path, sizeof path))
			fatal("%s: buffer 'path' too small", __FUNCTION__);
		loader->cwd = str_set(loader->cwd, path);
		x86_sys_debug("  New working directory is '%s'\n", loader->cwd);
	}

	/* Go back to old host path */
	if (chdir(old_host_path))
		fatal("%s: cannot cd back into old host path", __FUNCTION__);

	/* Return error code received in host call */
	return err;
}




/*
 * System call 'time' (code 13)
 */

static int x86_sys_time_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int time_ptr;
	int t;

	/* Arguments */
	time_ptr = regs->ebx;
	x86_sys_debug("  ptime=0x%x\n", time_ptr);

	/* Host call */
	t = time(NULL);
	if (time_ptr)
		mem_write(mem, time_ptr, 4, &t);

	/* Return */
	return t;
}




/*
 * System call 'chmod' (code 15)
 */

static int x86_sys_chmod_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int file_name_ptr;
	unsigned int mode;

	int len;
	int err;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];

	/* Arguments */
	file_name_ptr = regs->ebx;
	mode = regs->ecx;
	len = mem_read_string(mem, file_name_ptr, sizeof file_name, file_name);
	if (len >= sizeof file_name)
		fatal("%s: buffer too small", __FUNCTION__);
	X86ContextGetFullPath(ctx, file_name, full_path, sizeof full_path);
	x86_sys_debug("  file_name_ptr=0x%x, mode=0x%x\n", file_name_ptr, mode);
	x86_sys_debug("  file_name='%s', full_path='%s'\n", file_name, full_path);

	/* Host call */
	err = chmod(full_path, mode);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
}




/*
 * System call 'lseek' (code 19)
 */

static int x86_sys_lseek_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int offset;

	int fd;
	int origin;
	int host_fd;
	int err;

	/* Arguments */
	fd = regs->ebx;
	offset = regs->ecx;
	origin = regs->edx;
	host_fd = x86_file_desc_table_get_host_fd(ctx->file_desc_table, fd);
	x86_sys_debug("  fd=%d, offset=0x%x, origin=0x%x\n",
		fd, offset, origin);
	x86_sys_debug("  host_fd=%d\n", host_fd);

	/* Host call */
	err = lseek(host_fd, offset, origin);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
}




/*
 * System call 'getpid' (code 20)
 */

static int x86_sys_getpid_impl(X86Context *ctx)
{
	return ctx->pid;
}




/*
 * System call 'utime' (code 30)
 */

struct sim_utimbuf
{
	unsigned int actime;
	unsigned int modtime;
};

static void sys_utime_guest_to_host(struct utimbuf *host, struct sim_utimbuf *guest)
{
	host->actime = guest->actime;
	host->modtime = guest->modtime;
}

static int x86_sys_utime_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int file_name_ptr;
	unsigned int utimbuf_ptr;

	struct utimbuf utimbuf;
	struct sim_utimbuf sim_utimbuf;

	int len;
	int err;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];

	/* Arguments */
	file_name_ptr = regs->ebx;
	utimbuf_ptr = regs->ecx;
	x86_sys_debug("  file_name_ptr=0x%x, utimbuf_ptr=0x%x\n",
		file_name_ptr, utimbuf_ptr);

	/* Read file name */
	len = mem_read_string(mem, file_name_ptr, sizeof file_name, file_name);
	if (len >= MAX_PATH_SIZE)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get full path */
	X86ContextGetFullPath(ctx, file_name, full_path, sizeof full_path);
	x86_sys_debug("  file_name='%s', full_path='%s'\n", file_name, full_path);

	/* Read time buffer */
	mem_read(mem, utimbuf_ptr, sizeof(struct sim_utimbuf), &sim_utimbuf);
	sys_utime_guest_to_host(&utimbuf, &sim_utimbuf);
	x86_sys_debug("  utimbuf.actime = %u, utimbuf.modtime = %u\n",
		sim_utimbuf.actime, sim_utimbuf.modtime);

	/* Host call */
	err = utime(full_path, &utimbuf);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
}




/*
 * System call 'access' (code 33)
 */

static struct str_map_t sys_access_mode_map =
{
	3, {
		{ "X_OK",  1 },
		{ "W_OK",  2 },
		{ "R_OK",  4 }
	}
};

static int x86_sys_access_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int file_name_ptr;

	int mode;
	int len;
	int err;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];
	char mode_str[MAX_STRING_SIZE];

	/* Arguments */
	file_name_ptr = regs->ebx;
	mode = regs->ecx;

	/* Read file name */
	len = mem_read_string(mem, file_name_ptr, sizeof file_name, file_name);
	if (len >= sizeof file_name)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get full path */
	X86ContextGetFullPath(ctx, file_name, full_path, sizeof full_path);

	/* Debug */
	str_map_flags(&sys_access_mode_map, mode, mode_str, sizeof mode_str);
	x86_sys_debug("  file_name='%s', mode=0x%x\n", file_name, mode);
	x86_sys_debug("  full_path='%s'\n", full_path);
	x86_sys_debug("  mode=%s\n", mode_str);

	/* Host call */
	err = access(full_path, mode);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
}




/*
 * System call 'kill' (code 37)
 */

static int x86_sys_kill_impl(X86Context *ctx)
{
	X86Emu *emu = ctx->emu;

	struct x86_regs_t *regs = ctx->regs;
	X86Context *temp_ctx;

	int pid;
	int sig;

	/* Arguments */
	pid = regs->ebx;
	sig = regs->ecx;
	x86_sys_debug("  pid=%d, sig=%d (%s)\n", pid,
		sig, x86_signal_name(sig));

	/* Find context. We assume program correctness, so fatal if context is
	 * not found, rather than return error code. */
	temp_ctx = X86EmuGetContext(emu, pid);
	if (!temp_ctx)
		fatal("%s: invalid pid %d", __FUNCTION__, pid);

	/* Send signal */
	x86_sigset_add(&temp_ctx->signal_mask_table->pending, sig);
	X86ContextHostThreadSuspendCancel(temp_ctx);
	X86EmuProcessEventsSchedule(emu);
	X86EmuProcessEvents(emu);

	/* Success */
	return 0;
}




/*
 * System call 'rename' (code 38)
 */

static int x86_sys_rename_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int old_path_ptr;
	unsigned int new_path_ptr;

	char old_path[MAX_PATH_SIZE];
	char new_path[MAX_PATH_SIZE];
	char old_full_path[MAX_PATH_SIZE];
	char new_full_path[MAX_PATH_SIZE];

	int len;
	int err;

	/* Arguments */
	old_path_ptr = regs->ebx;
	new_path_ptr = regs->ecx;
	x86_sys_debug("  old_path_ptr=0x%x, new_path_ptr=0x%x\n",
		old_path_ptr, new_path_ptr);

	/* Get old path */
	len = mem_read_string(mem, old_path_ptr, sizeof old_path, old_path);
	if (len >= sizeof old_path)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get new path */
	len = mem_read_string(mem, new_path_ptr, sizeof new_path, new_path);
	if (len >= sizeof new_path)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get full paths */
	X86ContextGetFullPath(ctx, old_path, old_full_path, sizeof old_full_path);
	X86ContextGetFullPath(ctx, new_path, new_full_path, sizeof new_full_path);
	x86_sys_debug("  old_path='%s', new_path='%s'\n", old_path, new_path);
	x86_sys_debug("  old_full_path='%s', new_full_path='%s'\n", old_full_path, new_full_path);

	/* Host call */
	err = rename(old_full_path, new_full_path);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
}




/*
 * System call 'mkdir' (code 39)
 */

static int x86_sys_mkdir_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int path_ptr;

	int mode;
	int len;
	int err;

	char path[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];

	/* Arguments */
	path_ptr = regs->ebx;
	mode = regs->ecx;
	x86_sys_debug("  path_ptr=0x%x, mode=0x%x\n", path_ptr, mode);

	/* Read path */
	len = mem_read_string(mem, path_ptr, sizeof path, path);
	if (len >= sizeof path)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Read full path */
	X86ContextGetFullPath(ctx, path, full_path, MAX_PATH_SIZE);
	x86_sys_debug("  path='%s', full_path='%s'\n", path, full_path);

	/* Host call */
	err = mkdir(full_path, mode);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
}




/*
 * System call 'dup' (code 41)
 */

static int x86_sys_dup_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	int guest_fd;
	int dup_guest_fd;
	int host_fd;
	int dup_host_fd;

	struct x86_file_desc_t *desc;
	struct x86_file_desc_t *dup_desc;

	/* Arguments */
	guest_fd = regs->ebx;
	x86_sys_debug("  guest_fd=%d\n", guest_fd);

	/* Check that file descriptor is valid. */
	desc = x86_file_desc_table_entry_get(ctx->file_desc_table, guest_fd);
	if (!desc)
		return -EBADF;
	host_fd = desc->host_fd;
	x86_sys_debug("  host_fd=%d\n", host_fd);

	/* Duplicate host file descriptor. */
	dup_host_fd = dup(host_fd);
	if (dup_host_fd == -1)
		return -errno;

	/* Create a new entry in the file descriptor table. */
	dup_desc = x86_file_desc_table_entry_new(ctx->file_desc_table,
		file_desc_regular, dup_host_fd, desc->path, desc->flags);
	dup_guest_fd = dup_desc->guest_fd;

	/* Return new file descriptor. */
	return dup_guest_fd;
}




/*
 * System call 'pipe' (code 42)
 */

static int x86_sys_pipe_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	struct x86_file_desc_t *read_desc;
	struct x86_file_desc_t *write_desc;

	unsigned int fd_ptr;

	int guest_read_fd;
	int guest_write_fd;

	int host_fd[2];
	int err;

	/* Arguments */
	fd_ptr = regs->ebx;
	x86_sys_debug("  fd_ptr=0x%x\n", fd_ptr);

	/* Create host pipe */
	err = pipe(host_fd);
	if (err == -1)
		fatal("%s: cannot create pipe", __FUNCTION__);
	x86_sys_debug("  host pipe created: fd={%d, %d}\n",
		host_fd[0], host_fd[1]);

	/* Create guest pipe */
	read_desc = x86_file_desc_table_entry_new(ctx->file_desc_table,
		file_desc_pipe, host_fd[0], "", O_RDONLY);
	write_desc = x86_file_desc_table_entry_new(ctx->file_desc_table,
		file_desc_pipe, host_fd[1], "", O_WRONLY);
	x86_sys_debug("  guest pipe created: fd={%d, %d}\n",
		read_desc->guest_fd, write_desc->guest_fd);
	guest_read_fd = read_desc->guest_fd;
	guest_write_fd = write_desc->guest_fd;

	/* Return file descriptors. */
	mem_write(mem, fd_ptr, 4, &guest_read_fd);
	mem_write(mem, fd_ptr + 4, 4, &guest_write_fd);
	return 0;
}




/*
 * System call 'times' (code 43)
 */

struct sim_tms
{
	unsigned int utime;
	unsigned int stime;
	unsigned int cutime;
	unsigned int cstime;
};

static void sys_times_host_to_guest(struct sim_tms *guest, struct tms *host)
{
	guest->utime = host->tms_utime;
	guest->stime = host->tms_stime;
	guest->cutime = host->tms_cutime;
	guest->cstime = host->tms_cstime;
}

static int x86_sys_times_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int tms_ptr;

	struct tms tms;
	struct sim_tms sim_tms;

	int err;

	/* Arguments */
	tms_ptr = regs->ebx;
	x86_sys_debug("  tms_ptr=0x%x\n", tms_ptr);

	/* Host call */
	err = times(&tms);

	/* Write result in memory */
	if (tms_ptr)
	{
		sys_times_host_to_guest(&sim_tms, &tms);
		mem_write(mem, tms_ptr, sizeof(sim_tms), &sim_tms);
	}

	/* Return */
	return err;
}




/*
 * System call 'brk' (code 45)
 */

static int x86_sys_brk_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int old_heap_break;
	unsigned int new_heap_break;
	unsigned int size;

	unsigned int old_heap_break_aligned;
	unsigned int new_heap_break_aligned;

	/* Arguments */
	new_heap_break = regs->ebx;
	old_heap_break = mem->heap_break;
	x86_sys_debug("  newbrk=0x%x (previous brk was 0x%x)\n",
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
		x86_sys_debug("  heap grows %u bytes\n", new_heap_break - old_heap_break);
		return new_heap_break;
	}

	/* Always allow to shrink the heap. */
	if (new_heap_break < old_heap_break)
	{
		size = old_heap_break_aligned - new_heap_break_aligned;
		if (size)
			mem_unmap(mem, new_heap_break_aligned, size);
		mem->heap_break = new_heap_break;
		x86_sys_debug("  heap shrinks %u bytes\n", old_heap_break - new_heap_break);
		return new_heap_break;
	}

	/* Heap stays the same */
	return 0;
}




/*
 * System call 'ioctl' (code 54)
 */

/* An 'ioctl' code (first argument) is a 32-bit word split into 4 fields:
 *   -NR [7..0]: ioctl code number.
 *   -TYPE [15..8]: ioctl category.
 *   -SIZE [29..16]: size of the structure passed as 2nd argument.
 *   -DIR [31..30]: direction (01=Write, 10=Read, 11=R/W).
 */

static int x86_sys_ioctl_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int cmd;
	unsigned int arg;

	int guest_fd;
	int err;

	struct x86_file_desc_t *desc;

	/* Arguments */
	guest_fd = regs->ebx;
	cmd = regs->ecx;
	arg = regs->edx;
	x86_sys_debug("  guest_fd=%d, cmd=0x%x, arg=0x%x\n",
		guest_fd, cmd, arg);

	/* File descriptor */
	desc = x86_file_desc_table_entry_get(ctx->file_desc_table, guest_fd);
	if (!desc)
		return -EBADF;

	/* Process IOCTL */
	if (cmd >= 0x5401 || cmd <= 0x5408)
	{
		/* 'ioctl' commands using 'struct termios' as the argument.
		 * This structure is 60 bytes long both for x86 and x86_64
		 * architectures, so it doesn't vary between guest/host.
		 * No translation needed, so just use a 60-byte I/O buffer. */
		unsigned char buf[60];

		/* Read buffer */
		mem_read(mem, arg, sizeof buf, buf);
		err = ioctl(desc->host_fd, cmd, &buf);
		if (err == -1)
			return -errno;

		/* Return in memory */
		mem_write(mem, arg, sizeof buf, buf);
		return err;
	}
	else
	{
		fatal("%s: not implement for cmd = 0x%x.\n%s",
			__FUNCTION__, cmd, err_x86_sys_note);
	}

	/* Return */
	return 0;
}




/*
 * System call 'getppid' (code 64)
 */

static int x86_sys_getppid_impl(X86Context *ctx)
{
	/* Return 1 if there is no parent */
	if (!ctx->parent)
		return 1;

	/* Return parent's ID */
	return ctx->parent->pid;
}




/*
 * System call 'setrlimit' (code 75)
 */

static struct str_map_t sys_rlimit_res_map =
{
	16, {

		{ "RLIMIT_CPU",              0 },
		{ "RLIMIT_FSIZE",            1 },
		{ "RLIMIT_DATA",             2 },
		{ "RLIMIT_STACK",            3 },
		{ "RLIMIT_CORE",             4 },
		{ "RLIMIT_RSS",              5 },
		{ "RLIMIT_NPROC",            6 },
		{ "RLIMIT_NOFILE",           7 },
		{ "RLIMIT_MEMLOCK",          8 },
		{ "RLIMIT_AS",               9 },
		{ "RLIMIT_LOCKS",            10 },
		{ "RLIMIT_SIGPENDING",       11 },
		{ "RLIMIT_MSGQUEUE",         12 },
		{ "RLIMIT_NICE",             13 },
		{ "RLIMIT_RTPRIO",           14 },
		{ "RLIM_NLIMITS",            15 }
	}
};

struct sim_rlimit
{
	unsigned int cur;
	unsigned int max;
};

static int x86_sys_setrlimit_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int res;
	unsigned int rlim_ptr;

	char *res_str;

	struct sim_rlimit sim_rlimit;

	/* Arguments */
	res = regs->ebx;
	rlim_ptr = regs->ecx;
	res_str = str_map_value(&sys_rlimit_res_map, res);
	x86_sys_debug("  res=0x%x, rlim_ptr=0x%x\n", res, rlim_ptr);
	x86_sys_debug("  res=%s\n", res_str);

	/* Read structure */
	mem_read(mem, rlim_ptr, sizeof(struct sim_rlimit), &sim_rlimit);
	x86_sys_debug("  rlim->cur=0x%x, rlim->max=0x%x\n",
		sim_rlimit.cur, sim_rlimit.max);

	/* Different actions depending on resource type */
	switch (res)
	{

	case RLIMIT_DATA:
	{
		/* Default limit is maximum.
		 * This system call is ignored. */
		break;
	}

	case RLIMIT_STACK:
	{
		/* A program should allocate its stack with calls to mmap.
		 * This should be a limit for the stack, which is ignored here. */
		break;
	}

	default:
		fatal("%s: not implemented for res = %s.\n%s",
			__FUNCTION__, res_str, err_x86_sys_note);
	}

	/* Return */
	return 0;
}




/*
 * System call 'getrusage' (code 77)
 */

struct sim_rusage
{
	unsigned int utime_sec, utime_usec;
	unsigned int stime_sec, stime_usec;
	unsigned int maxrss;
	unsigned int ixrss;
	unsigned int idrss;
	unsigned int isrss;
	unsigned int minflt;
	unsigned int majflt;
	unsigned int nswap;
	unsigned int inblock;
	unsigned int oublock;
	unsigned int msgsnd;
	unsigned int msgrcv;
	unsigned int nsignals;
	unsigned int nvcsw;
	unsigned int nivcsw;
};

static void sys_rusage_host_to_guest(struct sim_rusage *guest, struct rusage *host)
{
	guest->utime_sec = host->ru_utime.tv_sec;
	guest->utime_usec = host->ru_utime.tv_usec;
	guest->stime_sec = host->ru_stime.tv_sec;
	guest->stime_usec = host->ru_stime.tv_usec;
	guest->maxrss = host->ru_maxrss;
	guest->ixrss = host->ru_ixrss;
	guest->idrss = host->ru_idrss;
	guest->isrss = host->ru_isrss;
	guest->minflt = host->ru_minflt;
	guest->majflt = host->ru_majflt;
	guest->nswap = host->ru_nswap;
	guest->inblock = host->ru_inblock;
	guest->oublock = host->ru_oublock;
	guest->msgsnd = host->ru_msgsnd;
	guest->msgrcv = host->ru_msgrcv;
	guest->nsignals = host->ru_nsignals;
	guest->nvcsw = host->ru_nvcsw;
	guest->nivcsw = host->ru_nivcsw;
}

static int x86_sys_getrusage_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int who;
	unsigned int u_ptr;

	struct rusage rusage;
	struct sim_rusage sim_rusage;

	int err;

	/* Arguments */
	who = regs->ebx;
	u_ptr = regs->ecx;
	x86_sys_debug("  who=0x%x, pru=0x%x\n", who, u_ptr);

	/* Supported values */
	if (who != 0)  /* RUSAGE_SELF */
		fatal("%s: not implemented for who != RUSAGE_SELF.\n%s",
			__FUNCTION__, err_x86_sys_note);

	/* Host call */
	err = getrusage(RUSAGE_SELF, &rusage);
	if (err == -1)
		return -errno;

	/* Return structure */
	sys_rusage_host_to_guest(&sim_rusage, &rusage);
	mem_write(mem, u_ptr, sizeof sim_rusage, &sim_rusage);

	/* Application expects this additional values updated:
	 * ru_maxrss: maximum resident set size
	 * ru_ixrss: integral shared memory size
	 * ru_idrss: integral unshared data size
	 * ru_isrss: integral unshared stack size */
	return 0;
}




/*
 * System call 'gettimeofday' (code 78)
 */

static int x86_sys_gettimeofday_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int tv_ptr;
	unsigned int tz_ptr;

	struct timeval tv;
	struct timezone tz;

	/* Arguments */
	tv_ptr = regs->ebx;
	tz_ptr = regs->ecx;
	x86_sys_debug("  tv_ptr=0x%x, tz_ptr=0x%x\n", tv_ptr, tz_ptr);

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
 * System call 'readlink' (code 85)
 */

static int x86_sys_readlink_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int path_ptr;
	unsigned int buf;
	unsigned int bufsz;

	char path[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];
	char dest_path[MAX_PATH_SIZE];

	int dest_size;
	int len;
	int err;

	/* Arguments */
	path_ptr = regs->ebx;
	buf = regs->ecx;
	bufsz = regs->edx;
	x86_sys_debug("  path_ptr=0x%x, buf=0x%x, bufsz=%d\n", path_ptr, buf, bufsz);

	/* Read path */
	len = mem_read_string(mem, path_ptr, sizeof path, path);
	if (len == sizeof path)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get full path */
	X86ContextGetFullPath(ctx, path, full_path, sizeof full_path);
	x86_sys_debug("  path='%s', full_path='%s'\n", path, full_path);

	/* Special file '/proc/self/exe' intercepted */
	if (!strcmp(full_path, "/proc/self/exe"))
	{
		/* Return path to simulated executable */
		if (strlen(ctx->loader->exe) >= sizeof dest_path)
			fatal("%s: buffer too small", __FUNCTION__);
		strcpy(dest_path, ctx->loader->exe);
	}
	else
	{
		/* Host call */
		memset(dest_path, 0, sizeof dest_path);
		err = readlink(full_path, dest_path, sizeof dest_path);
		if (err == sizeof dest_path)
			fatal("%s: buffer too small", __FUNCTION__);
		if (err == -1)
			return -errno;
	}

	/* Copy name to guest memory. The string is not null-terminated. */
	dest_size = MIN(strlen(dest_path), bufsz);
	mem_write(mem, buf, dest_size, dest_path);
	x86_sys_debug("  dest_path='%s'\n", dest_path);

	/* Return number of bytes copied */
	return dest_size;
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

static int x86_sys_mmap(X86Context *ctx, unsigned int addr, unsigned int len,
	int prot, int flags, int guest_fd, int offset)
{
	struct mem_t *mem = ctx->mem;

	unsigned int len_aligned;

	int perm;
	int host_fd;

	struct x86_file_desc_t *desc;

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
	desc = x86_file_desc_table_entry_get(ctx->file_desc_table, guest_fd);
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

static int x86_sys_mmap_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int args_ptr;
	unsigned int addr;
	unsigned int len;

	int prot;
	int flags;
	int offset;
	int guest_fd;

	char prot_str[MAX_STRING_SIZE];
	char flags_str[MAX_STRING_SIZE];

	/* This system call takes the arguments from memory, at the address
	 * pointed by 'ebx'. */
	args_ptr = regs->ebx;
	mem_read(mem, args_ptr, 4, &addr);
	mem_read(mem, args_ptr + 4, 4, &len);
	mem_read(mem, args_ptr + 8, 4, &prot);
	mem_read(mem, args_ptr + 12, 4, &flags);
	mem_read(mem, args_ptr + 16, 4, &guest_fd);
	mem_read(mem, args_ptr + 20, 4, &offset);

	x86_sys_debug("  args_ptr=0x%x\n", args_ptr);
	x86_sys_debug("  addr=0x%x, len=%u, prot=0x%x, flags=0x%x, "
		"guest_fd=%d, offset=0x%x\n",
		addr, len, prot, flags, guest_fd, offset);
	str_map_flags(&sys_mmap_prot_map, prot, prot_str, sizeof prot_str);
	str_map_flags(&sys_mmap_flags_map, flags, flags_str, sizeof flags_str);
	x86_sys_debug("  prot=%s, flags=%s\n", prot_str, flags_str);

	/* Call */
	return x86_sys_mmap(ctx, addr, len, prot, flags, guest_fd, offset);
}




/*
 * System call 'munmap' (code 91)
 */

static int x86_sys_munmap_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int addr;
	unsigned int size;
	unsigned int size_aligned;

	/* Arguments */
	addr = regs->ebx;
	size = regs->ecx;
	x86_sys_debug("  addr=0x%x, size=0x%x\n", addr, size);

	/* Restrictions */
	if (addr & (MEM_PAGE_SIZE - 1))
		fatal("%s: address not aligned", __FUNCTION__);

	/* Unmap */
	size_aligned = ROUND_UP(size, MEM_PAGE_SIZE);
	mem_unmap(mem, addr, size_aligned);

	/* Return */
	return 0;
}




/*
 * System call 'fchmod' (code 94)
 */

static int x86_sys_fchmod_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	int fd;
	int host_fd;
	int mode;
	int err;

	/* Arguments */
	fd = regs->ebx;
	mode = regs->ecx;
	x86_sys_debug("  fd=%d, mode=%d\n", fd, mode);

	/* Get host descriptor */
	host_fd = x86_file_desc_table_get_host_fd(ctx->file_desc_table, fd);
	x86_sys_debug("  host_fd=%d\n", host_fd);

	/* Host call */
	err = fchmod(host_fd, mode);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
}




/*
 * System call 'statfs' (code 99)
 */

struct sim_statfs_t
{					/* off	sz */
	unsigned int type;		/* 0	4 */
	unsigned int bsize;		/* 4	4 */
	unsigned int blocks;		/* 8	4 */
	unsigned int bfree;		/* 12	4 */
	unsigned int bavail;		/* 16	4 */
	unsigned int files;		/* 20	4 */
	unsigned int ffree;		/* 24	4 */
	unsigned int fsid[2];		/* 28	8 */
	unsigned int namelen;		/* 36	4 */
	unsigned int frsize;		/* 40	4 */
	unsigned int flags;		/* 44	4 */
	unsigned int spare[4];		/* 48	16 */
};

static void sim_statfs_host_to_guest(struct sim_statfs_t *host, struct statfs *guest)
{
	M2S_HOST_GUEST_MATCH(sizeof(*host), 64);
	memset(host, 0, sizeof(*host));
	host->type = guest->f_type;
	host->bsize = guest->f_bsize;
	host->blocks = guest->f_blocks;
	host->bfree = guest->f_bfree;
	host->bavail = guest->f_bavail;
	host->files = guest->f_files;
	host->ffree = guest->f_ffree;
	memcpy(&host->fsid, &guest->f_fsid, MIN(sizeof host->fsid, sizeof guest->f_fsid));
	host->namelen = guest->f_namelen;
	host->frsize = guest->f_frsize;
}

static int x86_sys_statfs_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int path_ptr;
	unsigned int statfs_buf_ptr;

	char path[MAX_PATH_SIZE];

	struct statfs statfs_buf;
	struct sim_statfs_t sim_statfs_buf;

	int length;
	int err;

	/* Arguments */
	path_ptr = regs->ebx;
	statfs_buf_ptr = regs->ecx;
	x86_sys_debug("  path_ptr=0x%x, statsf_buf_ptr=0x%x\n",
			path_ptr, statfs_buf_ptr);
	
	/* Read path */
	length = mem_read_string(mem, path_ptr, sizeof path, path);
	if (length == sizeof path)
		fatal("%s: buffer too small", __FUNCTION__);
	x86_sys_debug("  path='%s'\n", path);
	
	/* Host call */
	err = statfs(path, &statfs_buf);
	if (err == -1)
		return -errno;
	
	/* Copy guest structure */
	sim_statfs_host_to_guest(&sim_statfs_buf, &statfs_buf);
	mem_write(mem, statfs_buf_ptr, sizeof sim_statfs_buf, &sim_statfs_buf);
	return 0;
}




/*
 * System call 'socketcall' (code 102)
 */

static struct str_map_t sys_socketcall_call_map =
{
	17, {
		{ "SYS_SOCKET",		1 },
		{ "SYS_BIND",		2 },
		{ "SYS_CONNECT",	3 },
		{ "SYS_LISTEN",		4 },
		{ "SYS_ACCEPT",		5 },
		{ "SYS_GETSOCKNAME",	6 },
		{ "SYS_GETPEERNAME",	7 },
		{ "SYS_SOCKETPAIR",	8 },
		{ "SYS_SEND",		9 },
		{ "SYS_RECV",		10 },
		{ "SYS_SENDTO",		11 },
		{ "SYS_RECVFROM",	12 },
		{ "SYS_SHUTDOWN",	13 },
		{ "SYS_SETSOCKOPT",	14 },
		{ "SYS_GETSOCKOPT",	15 },
		{ "SYS_SENDMSG",	16 },
		{ "SYS_RECVMSG",	17 }
	}
};

static struct str_map_t sys_socket_family_map =
{
	29, {
		{ "PF_UNSPEC",		0 },
		{ "PF_UNIX",		1 },
		{ "PF_INET",		2 },
		{ "PF_AX25",		3 },
		{ "PF_IPX",		4 },
		{ "PF_APPLETALK",	5 },
		{ "PF_NETROM",		6 },
		{ "PF_BRIDGE",		7 },
		{ "PF_ATMPVC",		8 },
		{ "PF_X25",		9 },
		{ "PF_INET6",		10 },
		{ "PF_ROSE",		11 },
		{ "PF_DECnet",		12 },
		{ "PF_NETBEUI",		13 },
		{ "PF_SECURITY",	14 },
		{ "PF_KEY",		15 },
		{ "PF_NETLINK",		16 },
		{ "PF_PACKET",		17 },
		{ "PF_ASH",		18 },
		{ "PF_ECONET",		19 },
		{ "PF_ATMSVC",		20 },
		{ "PF_SNA",		22 },
		{ "PF_IRDA",		23 },
		{ "PF_PPPOX",		24 },
		{ "PF_WANPIPE",		25 },
		{ "PF_LLC",		26 },
		{ "PF_TIPC",		30 },
		{ "PF_BLUETOOTH",	31 },
		{ "PF_IUCV",		32 }
	}
};

static struct str_map_t sys_socket_type_map =
{
	7, {
		{ "SOCK_STREAM",	1 },
		{ "SOCK_DGRAM",		2 },
		{ "SOCK_RAW",		3 },
		{ "SOCK_RDM",		4 },
		{ "SOCK_SEQPACKET",	5 },
		{ "SOCK_DCCP",		6 },
		{ "SOCK_PACKET",	10 }
	}
};

static int x86_sys_socketcall_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	int call;
	unsigned int args;
	char *call_str;

	/* Arguments */
	call = regs->ebx;
	args = regs->ecx;
	call_str = str_map_value(&sys_socketcall_call_map, call);
	x86_sys_debug("  call=%d (%s), args=0x%x\n", call, call_str, args);

	/* Process call */
	switch (call)
	{

	/* SYS_SOCKET */
	case 1:
	{
		unsigned int family;
		unsigned int type;
		unsigned int protocol;

		char *family_str;
		char type_str[MAX_STRING_SIZE];

		int host_fd;

		struct x86_file_desc_t *desc;

		/* Read arguments */
		mem_read(mem, args, 4, &family);
		mem_read(mem, args + 4, 4, &type);
		mem_read(mem, args + 8, 4, &protocol);

		/* Debug */
		family_str = str_map_value(&sys_socket_family_map, family);
		snprintf(type_str, sizeof type_str, "%s%s%s",
				str_map_value(&sys_socket_type_map, type & 0xff),
				type & 0x80000 ? "|SOCK_CLOEXEC" : "",
				type & 0x800 ? "|SOCK_NONBLOCK" : "");
		x86_sys_debug("  family=%d (%s)\n", family, family_str);
		x86_sys_debug("  type=0x%x (%s)\n", type, type_str);
		x86_sys_debug("  protocol=%d\n", protocol);

		/* Allow only sockets of type SOCK_STREAM */
		if ((type & 0xff) != 1)
			fatal("%s: SYS_SOCKET: only type SOCK_STREAM supported",
					__FUNCTION__);

		/* Create socket */
		host_fd = socket(family, type, protocol);
		if (host_fd == -1)
			return -errno;

		/* Create new file descriptor table entry. */
		desc = x86_file_desc_table_entry_new(ctx->file_desc_table,
				file_desc_socket, host_fd, "", O_RDWR);
		x86_sys_debug("    socket created: guest_fd=%d, host_fd=%d\n",
			desc->guest_fd, desc->host_fd);

		/* Return socket */
		return desc->guest_fd;
	}

	/* SYS_CONNECT */
	case 3:
	{
		unsigned int guest_fd;
		unsigned int addr_ptr;
		unsigned int addr_len;

		struct x86_file_desc_t *desc;

		char buf[MAX_STRING_SIZE];

		struct sockaddr *addr;

		int err;

		/* Read arguments */
		mem_read(mem, args, 4, &guest_fd);
		mem_read(mem, args + 4, 4, &addr_ptr);
		mem_read(mem, args + 8, 4, &addr_len);
		x86_sys_debug("  guest_fd=%d, paddr=0x%x, addrlen=%d\n",
			guest_fd, addr_ptr, addr_len);

		/* Check buffer size */
		if (addr_len > sizeof buf)
			fatal("%s: SYS_CONNECT: buffer too small", __FUNCTION__);

		/* Host architecture assumptions */
		M2S_HOST_GUEST_MATCH(sizeof addr->sa_family, 2);
		M2S_HOST_GUEST_MATCH((void *) &addr->sa_data - (void *) &addr->sa_family, 2);

		/* Get 'sockaddr' structure, read family and data */
		addr = (struct sockaddr *) &buf[0];
		mem_read(mem, addr_ptr, addr_len, addr);
		x86_sys_debug("    sockaddr.family=%s\n", str_map_value(&sys_socket_family_map, addr->sa_family));
		x86_sys_debug_buffer("    sockaddr.data", addr->sa_data, addr_len - 2);

		/* Get file descriptor */
		desc = x86_file_desc_table_entry_get(ctx->file_desc_table, guest_fd);
		if (!desc)
			return -EBADF;

		/* Check that it's a socket */
		if (desc->kind != file_desc_socket)
			fatal("%s: SYS_CONNECT: file not a socket", __FUNCTION__);
		x86_sys_debug("    host_fd=%d\n", desc->host_fd);

		/* Connect socket */
		err = connect(desc->host_fd, addr, addr_len);
		if (err == -1)
			return -errno;

		/* Return */
		return err;
	}

	/* SYS_GETPEERNAME */
	case 7:
	{
		int guest_fd;
		int addr_len;
		int err;

		unsigned int addr_ptr;
		unsigned int addr_len_ptr;

		struct x86_file_desc_t *desc;
		struct sockaddr *addr;
		socklen_t host_addr_len;

		mem_read(mem, args, 4, &guest_fd);
		mem_read(mem, args + 4, 4, &addr_ptr);
		mem_read(mem, args + 8, 4, &addr_len_ptr);
		x86_sys_debug("  guest_fd=%d, paddr=0x%x, paddrlen=0x%x\n",
			guest_fd, addr_ptr, addr_len_ptr);

		/* Get file descriptor */
		desc = x86_file_desc_table_entry_get(ctx->file_desc_table, guest_fd);
		if (!desc)
			return -EBADF;

		/* Read current buffer size and allocate buffer. */
		mem_read(mem, addr_len_ptr, 4, &addr_len);
		x86_sys_debug("    addrlen=%d\n", addr_len);
		host_addr_len = addr_len;
		addr = xmalloc(addr_len);

		/* Get peer name */
		err = getpeername(desc->host_fd, addr, &host_addr_len);
		if (err == -1)
		{
			free(addr);
			return -errno;
		}

		/* Address length returned */
		addr_len = host_addr_len;
		x86_sys_debug("  result:\n");
		x86_sys_debug("    addrlen=%d\n", host_addr_len);
		x86_sys_debug_buffer("    sockaddr.data", addr->sa_data, addr_len - 2);

		/* Copy result to guest memory */
		mem_write(mem, addr_len_ptr, 4, &addr_len);
		mem_write(mem, addr_ptr, addr_len, addr);

		/* Free and return */
		free(addr);
		return err;
	}

	default:

		fatal("%s: call '%s' not implemented",
			__FUNCTION__, call_str);
	}

	/* Dead code */
	return 0;
}




/*
 * System call 'setitimer' (code 104)
 */

static struct str_map_t sys_itimer_which_map =
{
	3, {
		{"ITIMER_REAL",		0},
		{"ITIMER_VIRTUAL",	1},
		{"ITIMER_PROF",		2}
	}
};

struct sim_timeval
{
	unsigned int tv_sec;
	unsigned int tv_usec;
};

struct sim_itimerval
{
	struct sim_timeval it_interval;
	struct sim_timeval it_value;
};

static void sim_timeval_dump(struct sim_timeval *sim_timeval)
{
	x86_sys_debug("    tv_sec=%u, tv_usec=%u\n",
		sim_timeval->tv_sec, sim_timeval->tv_usec);
}

static void sim_itimerval_dump(struct sim_itimerval *sim_itimerval)
{
	x86_sys_debug("    it_interval: tv_sec=%u, tv_usec=%u\n",
		sim_itimerval->it_interval.tv_sec, sim_itimerval->it_interval.tv_usec);
	x86_sys_debug("    it_value: tv_sec=%u, tv_usec=%u\n",
		sim_itimerval->it_value.tv_sec, sim_itimerval->it_value.tv_usec);
}

static int x86_sys_setitimer_impl(X86Context *ctx)
{
	X86Emu *emu = ctx->emu;

	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int which;
	unsigned int value_ptr;
	unsigned int old_value_ptr;

	struct sim_itimerval itimerval;

	long long now;

	/* Arguments */
	which = regs->ebx;
	value_ptr = regs->ecx;
	old_value_ptr = regs->edx;
	x86_sys_debug("  which=%d (%s), value_ptr=0x%x, old_value_ptr=0x%x\n",
		which, str_map_value(&sys_itimer_which_map, which), value_ptr, old_value_ptr);

	/* Get current time */
	now = esim_real_time();

	/* Read value */
	if (value_ptr)
	{
		mem_read(mem, value_ptr, sizeof itimerval, &itimerval);
		x86_sys_debug("  itimerval at 'value_ptr':\n");
		sim_itimerval_dump(&itimerval);
	}

	/* Check range of 'which' */
	if (which >= 3)
		fatal("%s: invalid value for 'which'", __FUNCTION__);

	/* Set 'itimer_value' and 'itimer_interval' (usec) */
	ctx->itimer_value[which] = now + itimerval.it_value.tv_sec * 1000000
		+ itimerval.it_value.tv_usec;
	ctx->itimer_interval[which] = itimerval.it_interval.tv_sec * 1000000
		+ itimerval.it_interval.tv_usec;

	/* New timer inserted, so interrupt current 'ke_host_thread_timer'
	 * waiting for the next timer expiration. */
	X86ContextHostThreadTimerCancel(ctx);
	X86EmuProcessEventsSchedule(emu);

	/* Return */
	return 0;
}




/*
 * System call 'getitimer' (code 105)
 */

static int x86_sys_getitimer_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int which;
	unsigned int value_ptr;

	struct sim_itimerval itimerval;

	long long now;
	long long rem;

	/* Arguments */
	which = regs->ebx;
	value_ptr = regs->ecx;
	x86_sys_debug("  which=%d (%s), value_ptr=0x%x\n",
		which, str_map_value(&sys_itimer_which_map, which), value_ptr);

	/* Get current time */
	now = esim_real_time();

	/* Check range of 'which' */
	if (which >= 3)
		fatal("syscall 'getitimer': wrong value for 'which' argument");

	/* Return value in structure */
	rem = now < ctx->itimer_value[which] ? ctx->itimer_value[which] - now : 0;
	itimerval.it_value.tv_sec = rem / 1000000;
	itimerval.it_value.tv_usec = rem % 1000000;
	itimerval.it_interval.tv_sec = ctx->itimer_interval[which] / 1000000;
	itimerval.it_interval.tv_usec = ctx->itimer_interval[which] % 1000000;
	mem_write(mem, value_ptr, sizeof itimerval, &itimerval);

	/* Return */
	return 0;
}




/*
 * System call 'sigreturn' (code 119)
 */

static int x86_sys_sigreturn_impl(X86Context *ctx)
{
	X86Emu *emu = ctx->emu;

	X86ContextReturnFromSignalHandler(ctx);

	X86EmuProcessEventsSchedule(emu);
	X86EmuProcessEvents(emu);

	return 0;
}




/*
 * System call 'clone' (code 120)
 */

#define SIM_CLONE_VM			0x00000100
#define SIM_CLONE_FS			0x00000200
#define SIM_CLONE_FILES			0x00000400
#define SIM_CLONE_SIGHAND		0x00000800
#define SIM_CLONE_PTRACE		0x00002000
#define SIM_CLONE_VFORK			0x00004000
#define SIM_CLONE_PARENT		0x00008000
#define SIM_CLONE_THREAD		0x00010000
#define SIM_CLONE_NEWNS			0x00020000
#define SIM_CLONE_SYSVSEM		0x00040000
#define SIM_CLONE_SETTLS		0x00080000
#define SIM_CLONE_PARENT_SETTID		0x00100000
#define SIM_CLONE_CHILD_CLEARTID	0x00200000
#define SIM_CLONE_DETACHED		0x00400000
#define SIM_CLONE_UNTRACED		0x00800000
#define SIM_CLONE_CHILD_SETTID		0x01000000
#define SIM_CLONE_STOPPED		0x02000000
#define SIM_CLONE_NEWUTS		0x04000000
#define SIM_CLONE_NEWIPC		0x08000000
#define SIM_CLONE_NEWUSER		0x10000000
#define SIM_CLONE_NEWPID		0x20000000
#define SIM_CLONE_NEWNET		0x40000000
#define SIM_CLONE_IO			0x80000000

static struct str_map_t sys_clone_flags_map =
{
	23, {
		{ "CLONE_VM", 0x00000100 },
		{ "CLONE_FS", 0x00000200 },
		{ "CLONE_FILES", 0x00000400 },
		{ "CLONE_SIGHAND", 0x00000800 },
		{ "CLONE_PTRACE", 0x00002000 },
		{ "CLONE_VFORK", 0x00004000 },
		{ "CLONE_PARENT", 0x00008000 },
		{ "CLONE_THREAD", 0x00010000 },
		{ "CLONE_NEWNS", 0x00020000 },
		{ "CLONE_SYSVSEM", 0x00040000 },
		{ "CLONE_SETTLS", 0x00080000 },
		{ "CLONE_PARENT_SETTID", 0x00100000 },
		{ "CLONE_CHILD_CLEARTID", 0x00200000 },
		{ "CLONE_DETACHED", 0x00400000 },
		{ "CLONE_UNTRACED", 0x00800000 },
		{ "CLONE_CHILD_SETTID", 0x01000000 },
		{ "CLONE_STOPPED", 0x02000000 },
		{ "CLONE_NEWUTS", 0x04000000 },
		{ "CLONE_NEWIPC", 0x08000000 },
		{ "CLONE_NEWUSER", 0x10000000 },
		{ "CLONE_NEWPID", 0x20000000 },
		{ "CLONE_NEWNET", 0x40000000 },
		{ "CLONE_IO", 0x80000000 }
	}
};

static const unsigned int sys_clone_supported_flags =
	SIM_CLONE_VM |
	SIM_CLONE_FS |
	SIM_CLONE_FILES |
	SIM_CLONE_SIGHAND |
	SIM_CLONE_THREAD |
	SIM_CLONE_SYSVSEM |
	SIM_CLONE_SETTLS |
	SIM_CLONE_PARENT_SETTID |
	SIM_CLONE_CHILD_CLEARTID |
	SIM_CLONE_CHILD_SETTID;

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

static int x86_sys_clone_impl(X86Context *ctx)
{
	/* Prototype: long sys_clone(unsigned long clone_flags, unsigned long newsp,
	 * 	int __user *parent_tid, int unused, int __user *child_tid);
	 * There is an unused parameter, that's why we read child_tidptr from edi
	 * instead of esi. */

	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int flags;
	unsigned int new_esp;
	unsigned int parent_tid_ptr;
	unsigned int child_tid_ptr;

	int exit_signal;

	char flags_str[MAX_STRING_SIZE];

	X86Context *new_ctx;

	/* Arguments */
	flags = regs->ebx;
	new_esp = regs->ecx;
	parent_tid_ptr = regs->edx;
	child_tid_ptr = regs->edi;
	x86_sys_debug("  flags=0x%x, newsp=0x%x, parent_tidptr=0x%x, child_tidptr=0x%x\n",
		flags, new_esp, parent_tid_ptr, child_tid_ptr);

	/* Exit signal is specified in the lower byte of 'flags' */
	exit_signal = flags & 0xff;
	flags &= ~0xff;

	/* Debug */
	str_map_flags(&sys_clone_flags_map, flags, flags_str, MAX_STRING_SIZE);
	x86_sys_debug("  flags=%s\n", flags_str);
	x86_sys_debug("  exit_signal=%d (%s)\n", exit_signal, x86_signal_name(exit_signal));

	/* New stack pointer defaults to current */
	if (!new_esp)
		new_esp = regs->esp;

	/* Check not supported flags */
	if (flags & ~sys_clone_supported_flags)
	{
		str_map_flags(&sys_clone_flags_map, flags & ~sys_clone_supported_flags,
			flags_str, MAX_STRING_SIZE);
		fatal("%s: not supported flags: %s\n%s",
			__FUNCTION__, flags_str, err_x86_sys_note);
	}

	/* Flag CLONE_VM */
	if (flags & SIM_CLONE_VM)
	{
		/* CLONE_FS, CLONE_FILES, CLONE_SIGHAND must be there, too */
		if ((flags & (SIM_CLONE_FS | SIM_CLONE_FILES | SIM_CLONE_SIGHAND)) !=
			(SIM_CLONE_FS | SIM_CLONE_FILES | SIM_CLONE_SIGHAND))
			fatal("%s: not supported flags with CLONE_VM.\n%s",
				__FUNCTION__, err_x86_sys_note);

		/* Create new context sharing memory image */
		new_ctx = new_ctor(X86Context, CreateAndClone, ctx);
	}
	else
	{
		/* CLONE_FS, CLONE_FILES, CLONE_SIGHAND must not be there either */
		if (flags & (SIM_CLONE_FS | SIM_CLONE_FILES | SIM_CLONE_SIGHAND))
			fatal("%s: not supported flags with CLONE_VM.\n%s",
				__FUNCTION__, err_x86_sys_note);

		/* Create new context replicating memory image */
		new_ctx = new_ctor(X86Context, CreateAndFork, ctx);
	}

	/* Flag CLONE_THREAD.
	 * If specified, the exit signal is ignored. Otherwise, it is specified in the
	 * lower byte of the flags. Also, this determines whether to create a group of
	 * threads. */
	if (flags & SIM_CLONE_THREAD)
	{
		new_ctx->exit_signal = 0;
		new_ctx->group_parent = ctx->group_parent ?
			ctx->group_parent : ctx;
	}
	else
	{
		new_ctx->exit_signal = exit_signal;
		new_ctx->group_parent = NULL;
	}

	/* Flag CLONE_PARENT_SETTID */
	if (flags & SIM_CLONE_PARENT_SETTID)
		mem_write(ctx->mem, parent_tid_ptr, 4, &new_ctx->pid);

	/* Flag CLONE_CHILD_SETTID */
	if (flags & SIM_CLONE_CHILD_SETTID)
		mem_write(new_ctx->mem, child_tid_ptr, 4, &new_ctx->pid);

	/* Flag CLONE_CHILD_CLEARTID */
	if (flags & SIM_CLONE_CHILD_CLEARTID)
		new_ctx->clear_child_tid = child_tid_ptr;

	/* Flag CLONE_SETTLS */
	if (flags & SIM_CLONE_SETTLS)
	{
		struct sim_user_desc uinfo;
		unsigned int uinfo_ptr;

		uinfo_ptr = regs->esi;
		x86_sys_debug("  puinfo=0x%x\n", uinfo_ptr);

		mem_read(mem, uinfo_ptr, sizeof(struct sim_user_desc), &uinfo);
		x86_sys_debug("  entry_number=0x%x, base_addr=0x%x, limit=0x%x\n",
				uinfo.entry_number, uinfo.base_addr, uinfo.limit);
		x86_sys_debug("  seg_32bit=0x%x, contents=0x%x, read_exec_only=0x%x\n",
				uinfo.seg_32bit, uinfo.contents, uinfo.read_exec_only);
		x86_sys_debug("  limit_in_pages=0x%x, seg_not_present=0x%x, useable=0x%x\n",
				uinfo.limit_in_pages, uinfo.seg_not_present, uinfo.useable);
		if (!uinfo.seg_32bit)
			fatal("%s: only 32-bit segments supported", __FUNCTION__);

		/* Limit given in pages (4KB units) */
		if (uinfo.limit_in_pages)
			uinfo.limit <<= 12;

		uinfo.entry_number = 6;
		mem_write(mem, uinfo_ptr, 4, &uinfo.entry_number);

		new_ctx->glibc_segment_base = uinfo.base_addr;
		new_ctx->glibc_segment_limit = uinfo.limit;
	}

	/* New context returns 0. */
	new_ctx->regs->esp = new_esp;
	new_ctx->regs->eax = 0;

	/* Return PID of the new context */
	x86_sys_debug("  context created with pid %d\n", new_ctx->pid);
	return new_ctx->pid;
}




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
	"3.1.9-1.fc16.i686"
	"#1 Fri Jan 13 16:37:42 UTC 2012",
	"i686"
	""
};

static int x86_sys_newuname_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int utsname_ptr;

	/* Arguments */
	utsname_ptr = regs->ebx;
	x86_sys_debug("  putsname=0x%x\n", utsname_ptr);
	x86_sys_debug("  sysname='%s', nodename='%s'\n", sim_utsname.sysname, sim_utsname.nodename);
	x86_sys_debug("  relaese='%s', version='%s'\n", sim_utsname.release, sim_utsname.version);
	x86_sys_debug("  machine='%s', domainname='%s'\n", sim_utsname.machine, sim_utsname.domainname);

	/* Return structure */
	mem_write(mem, utsname_ptr, sizeof sim_utsname, &sim_utsname);
	return 0;
}




/*
 * System call 'mprotect' (code 125)
 */

static int x86_sys_mprotect_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int start;
	unsigned int len;

	int prot;

	enum mem_access_t perm = 0;

	/* Arguments */
	start = regs->ebx;
	len = regs->ecx;
	prot = regs->edx;
	x86_sys_debug("  start=0x%x, len=0x%x, prot=0x%x\n", start, len, prot);

	/* Permissions */
	perm |= prot & 0x01 ? mem_access_read : 0;
	perm |= prot & 0x02 ? mem_access_write : 0;
	perm |= prot & 0x04 ? mem_access_exec : 0;
	mem_protect(mem, start, len, perm);

	/* Return */
	return 0;
}




/*
 * System call 'llseek' (code 140)
 */

static int x86_sys_llseek_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int fd;
	unsigned int result_ptr;

	int origin;
	int host_fd;
	int offset_high;
	int offset_low;

	long long offset;

	/* Arguments */
	fd = regs->ebx;
	offset_high = regs->ecx;
	offset_low = regs->edx;
	offset = ((long long) offset_high << 32) | offset_low;
	result_ptr = regs->esi;
	origin = regs->edi;
	host_fd = x86_file_desc_table_get_host_fd(ctx->file_desc_table, fd);
	x86_sys_debug("  fd=%d, offset_high=0x%x, offset_low=0x%x, result_ptr=0x%x, origin=0x%x\n",
		fd, offset_high, offset_low, result_ptr, origin);
	x86_sys_debug("  host_fd=%d\n", host_fd);
	x86_sys_debug("  offset=0x%llx\n", (long long) offset);

	/* Supported offset */
	if (offset_high != -1 && offset_high)
		fatal("%s: only supported for 32-bit files", __FUNCTION__);

	/* Host call */
	offset = lseek(host_fd, offset_low, origin);
	if (offset == -1)
		return -errno;

	/* Copy offset to memory */
	if (result_ptr)
		mem_write(mem, result_ptr, 8, &offset);

	/* Return */
	return 0;
}




/*
 * System call 'getdents' (code 141)
 */

struct sys_host_dirent_t
{
	long d_ino;
	off_t d_off;
	unsigned short d_reclen;
	char d_name[];
};

struct sys_guest_dirent_t
{
	unsigned int d_ino;
	unsigned int d_off;
	unsigned short d_reclen;
	char d_name[];
} __attribute__((packed));

static int x86_sys_getdents_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int pdirent;

	int fd;
	int count;
	int host_fd;

	void *buf;

	int nread;
	int host_offs;
	int guest_offs;

	char d_type;

	struct sys_host_dirent_t *dirent;
	struct sys_guest_dirent_t sim_dirent;

	/* Read parameters */
	fd = regs->ebx;
	pdirent = regs->ecx;
	count = regs->edx;
	host_fd = x86_file_desc_table_get_host_fd(ctx->file_desc_table, fd);
	x86_sys_debug("  fd=%d, pdirent=0x%x, count=%d\n",
		fd, pdirent, count);
	x86_sys_debug("  host_fd=%d\n", host_fd);

	/* Call host getdents */
	buf = xcalloc(1, count);
	nread = syscall(SYS_getdents, host_fd, buf, count);
	if (nread == -1)
		fatal("%s: host call failed", __FUNCTION__);

	/* No more entries */
	if (!nread)
	{
		free(buf);
		return 0;
	}

	/* Copy to host memory */
	host_offs = 0;
	guest_offs = 0;
	while (host_offs < nread)
	{
		dirent = (struct sys_host_dirent_t *) (buf + host_offs);
		sim_dirent.d_ino = dirent->d_ino;
		sim_dirent.d_off = dirent->d_off;
		sim_dirent.d_reclen = (15 + strlen(dirent->d_name)) / 4 * 4;
		d_type = * (char *) (buf + host_offs + dirent->d_reclen - 1);

		x86_sys_debug("    d_ino=%u ", sim_dirent.d_ino);
		x86_sys_debug("d_off=%u ", sim_dirent.d_off);
		x86_sys_debug("d_reclen=%u(host),%u(guest) ", dirent->d_reclen, sim_dirent.d_reclen);
		x86_sys_debug("d_name='%s'\n", dirent->d_name);

		mem_write(mem, pdirent + guest_offs, 4, &sim_dirent.d_ino);
		mem_write(mem, pdirent + guest_offs + 4, 4, &sim_dirent.d_off);
		mem_write(mem, pdirent + guest_offs + 8, 2, &sim_dirent.d_reclen);
		mem_write_string(mem, pdirent + guest_offs + 10, dirent->d_name);
		mem_write(mem, pdirent + guest_offs + sim_dirent.d_reclen - 1, 1, &d_type);

		host_offs += dirent->d_reclen;
		guest_offs += sim_dirent.d_reclen;
		if (guest_offs > count)
			fatal("%s: buffer too small", __FUNCTION__);
	}
	x86_sys_debug("  ret=%d(host),%d(guest)\n", host_offs, guest_offs);
	free(buf);
	return guest_offs;
}




/*
 * System call 'select' (code 142)
 */

/* Dump host 'fd_set' structure */
static void sim_fd_set_dump(char *fd_set_name, fd_set *fds, int n)
{
	int i;

	char *comma;

	/* Set empty */
	if (!n || !fds)
	{
		x86_sys_debug("    %s={}\n", fd_set_name);
		return;
	}

	/* Dump set */
	x86_sys_debug("    %s={", fd_set_name);
	comma = "";
	for (i = 0; i < n; i++)
	{
		if (!FD_ISSET(i, fds))
			continue;
		x86_sys_debug("%s%d", comma, i);
		comma = ",";
	}
	x86_sys_debug("}\n");
}

/* Read bitmap of 'guest_fd's from guest memory, and store it into
 * a bitmap of 'host_fd's into host memory. */
static int sim_fd_set_read(X86Context *ctx, uint32_t addr,
	fd_set *fds, int n)
{
	struct mem_t *mem = ctx->mem;

	int nbyte;
	int nbit;
	int host_fd;
	int i;

	unsigned char c;

	FD_ZERO(fds);
	for (i = 0; i < n; i++)
	{
		/* Check if fd is set */
		nbyte = i >> 3;
		nbit = i & 7;
		mem_read(mem, addr + nbyte, 1, &c);
		if (!(c & (1 << nbit)))
			continue;

		/* Obtain 'host_fd' */
		host_fd = x86_file_desc_table_get_host_fd(ctx->file_desc_table, i);
		if (host_fd < 0)
			return 0;
		FD_SET(host_fd, fds);
	}
	return 1;
}

/* Read bitmap of 'host_fd's from host memory, and store it into
 * a bitmap of 'guest_fd's into guest memory. */
static void sim_fd_set_write(X86Context *ctx, unsigned int addr,
	fd_set *fds, int n)
{
	struct mem_t *mem = ctx->mem;

	int nbyte;
	int nbit;
	int guest_fd;
	int i;

	unsigned char c;

	/* No valid address given */
	if (!addr)
		return;

	/* Write */
	mem_zero(mem, addr, (n + 7) / 8);
	for (i = 0; i < n; i++)
	{
		/* Check if fd is set */
		if (!FD_ISSET(i, fds))
			continue;

		/* Obtain 'guest_fd' and write */
		guest_fd = x86_file_desc_table_get_guest_fd(ctx->file_desc_table, i);
		assert(guest_fd >= 0);
		nbyte = guest_fd >> 3;
		nbit = guest_fd & 7;
		mem_read(mem, addr + nbyte, 1, &c);
		c |= 1 << nbit;
		mem_write(mem, addr + nbyte, 1, &c);
	}
}

static int x86_sys_select_impl(X86Context *ctx)
{
	/* System call prototype:
	 * int select(int n, fd_set *inp, fd_set *outp, fd_set *exp, struct timeval *tvp);
	 */

	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int n;
	unsigned int inp;
	unsigned int outp;
	unsigned int exp;
	unsigned int tvp;

	fd_set in;
	fd_set out;
	fd_set ex;

	struct sim_timeval sim_tv;
	struct timeval tv;

	int err;

	/* Arguments */
	n = regs->ebx;
	inp = regs->ecx;
	outp = regs->edx;
	exp = regs->esi;
	tvp = regs->edi;
	x86_sys_debug("  n=%d, inp=0x%x, outp=0x%x, exp=0x%x, tvp=0x%x\n",
		n, inp, outp, exp, tvp);

	/* Read file descriptor bitmaps. If any file descriptor is invalid, return EBADF. */
	if (!sim_fd_set_read(ctx, inp, &in, n)
		|| !sim_fd_set_read(ctx, outp, &out, n)
		|| !sim_fd_set_read(ctx, exp, &ex, n))
	{
		return -EBADF;
	}

	/* Dump file descriptors */
	sim_fd_set_dump("inp", &in, n);
	sim_fd_set_dump("outp", &out, n);
	sim_fd_set_dump("exp", &ex, n);

	/* Read and dump 'sim_tv' */
	memset(&sim_tv, 0, sizeof sim_tv);
	if (tvp)
		mem_read(mem, tvp, sizeof sim_tv, &sim_tv);
	x86_sys_debug("  tv:\n");
	sim_timeval_dump(&sim_tv);

	/* Blocking 'select' not supported */
	if (sim_tv.tv_sec || sim_tv.tv_usec)
		fatal("%s: not supported for 'tv' other than 0", __FUNCTION__);

	/* Host system call */
	memset(&tv, 0, sizeof(tv));
	err = select(n, &in, &out, &ex, &tv);
	if (err == -1)
		return -errno;

	/* Write result */
	sim_fd_set_write(ctx, inp, &in, n);
	sim_fd_set_write(ctx, outp, &out, n);
	sim_fd_set_write(ctx, exp, &ex, n);

	/* Return */
	return err;
}




/*
 * System call 'msync' (code 144)
 */

static struct str_map_t sys_msync_flags_map =
{
	3, {
		{ "MS_ASYNC", 1 },
		{ "MS_INAVLIAGE", 2 },
		{ "MS_SYNC", 4 }
	}
};

static int x86_sys_msync_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int start;
	unsigned int len;

	int flags;

	char flags_str[MAX_STRING_SIZE];

	/* Arguments */
	start = regs->ebx;
	len = regs->ecx;
	flags = regs->edx;
	str_map_flags(&sys_msync_flags_map, flags, flags_str, sizeof flags_str);
	x86_sys_debug("  start=0x%x, len=0x%x, flags=0x%x\n", start, len, flags);
	x86_sys_debug("  flags=%s\n", flags_str);

	/* System call ignored */
	return 0;
}




/*
 * System call 'writev' (code 146)
 */

static int x86_sys_writev_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	int v;
	int len;
	int guest_fd;
	int host_fd;
	int total_len;

	struct x86_file_desc_t *desc;

	unsigned int iovec_ptr;
	unsigned int vlen;
	unsigned int iov_base;
	unsigned int iov_len;

	void *buf;

	/* Arguments */
	guest_fd = regs->ebx;
	iovec_ptr = regs->ecx;
	vlen = regs->edx;
	x86_sys_debug("  guest_fd=%d, iovec_ptr = 0x%x, vlen=0x%x\n",
		guest_fd, iovec_ptr, vlen);

	/* Check file descriptor */
	desc = x86_file_desc_table_entry_get(ctx->file_desc_table, guest_fd);
	if (!desc)
		return -EBADF;
	host_fd = desc->host_fd;
	x86_sys_debug("  host_fd=%d\n", host_fd);

	/* No pipes allowed */
	if (desc->kind == file_desc_pipe)
		fatal("%s: not supported for pipes.\n%s",
			__FUNCTION__, err_x86_sys_note);

	/* Proceed */
	total_len = 0;
	for (v = 0; v < vlen; v++)
	{
		/* Read io vector element */
		mem_read(mem, iovec_ptr, 4, &iov_base);
		mem_read(mem, iovec_ptr + 4, 4, &iov_len);
		iovec_ptr += 8;

		/* Read buffer from memory and write it to file */
		buf = xmalloc(iov_len);
		mem_read(mem, iov_base, iov_len, buf);
		len = write(host_fd, buf, iov_len);
		if (len == -1)
		{
			free(buf);
			return -errno;
		}

		/* Accumulate written bytes */
		total_len += len;
		free(buf);
	}

	/* Return total number of bytes written */
	return total_len;
}




/*
 * System call 'sysctl' (code 149)
 */

struct sys_sysctl_args_t
{
	unsigned int pname;
	unsigned int nlen;
	unsigned int poldval;
	unsigned int oldlenp;
	unsigned int pnewval;
	unsigned int newlen;
};

static int x86_sys_sysctl_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	int i;

	unsigned int args_ptr;
	unsigned int aux;
	unsigned int zero = 0;

	struct sys_sysctl_args_t args;

	/* Arguments */
	args_ptr = regs->ebx;
	x86_sys_debug("  pargs=0x%x\n", args_ptr);

	/* Access arguments in memory */
	mem_read(mem, args_ptr, sizeof args, &args);
	x86_sys_debug("    pname=0x%x\n", args.pname);
	x86_sys_debug("    nlen=%d\n      ", args.nlen);
	for (i = 0; i < args.nlen; i++)
	{
		mem_read(mem, args.pname + i * 4, 4, &aux);
		x86_sys_debug("name[%d]=%d ", i, aux);
	}
	x86_sys_debug("\n    poldval=0x%x\n", args.poldval);
	x86_sys_debug("    oldlenp=0x%x\n", args.oldlenp);
	x86_sys_debug("    pnewval=0x%x\n", args.pnewval);
	x86_sys_debug("    newlen=%d\n", args.newlen);

	/* Supported values */
	if (!args.oldlenp || !args.poldval)
		fatal("%s: not supported for poldval=0 or oldlenp=0.\n%s",
			__FUNCTION__, err_x86_sys_note);
	if (args.pnewval || args.newlen)
		fatal("%s: not supported for pnewval or newlen other than 0.\n%s",
			__FUNCTION__, err_x86_sys_note);

	/* Return */
	mem_write(mem, args.oldlenp, 4, &zero);
	mem_write(mem, args.poldval, 1, &zero);
	return 0;
}




/*
 * System call 'sched_setparam' (code 154)
 */

static int x86_sys_sched_setparam_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int param_ptr;

	int sched_priority;
	int pid;

	pid = regs->ebx;
	param_ptr = regs->ecx;
	x86_sys_debug("  pid=%d\n", pid);
	x86_sys_debug("  param_ptr=0x%x\n", param_ptr);
	mem_read(mem, param_ptr, 4, &sched_priority);
	x86_sys_debug("    param.sched_priority=%d\n", sched_priority);

	/* Ignore system call */
	return 0;
}




/*
 * System call 'sched_getparam' (code 155)
 */

static int x86_sys_sched_getparam_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int param_ptr;
	unsigned int zero = 0;

	int pid;

	/* Arguments */
	pid = regs->ebx;
	param_ptr = regs->ecx;
	x86_sys_debug("  pid=%d\n", pid);
	x86_sys_debug("  param_ptr=0x%x\n", param_ptr);

	/* Return 0 in param_ptr->sched_priority */
	mem_write(mem, param_ptr, 4, &zero);
	return 0;
}




/*
 * System call 'sched_getscheduler' (code 157)
 */

static int x86_sys_sched_getscheduler_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	int pid;

	/* Arguments */
	pid = regs->ebx;
	x86_sys_debug("  pid=%d\n", pid);

	/* System call ignored */
	return 0;
}




/*
 * System call 'sched_get_priority_max' (code 159)
 */

static int x86_sys_sched_get_priority_max_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	int policy;

	/* Arguments */
	policy = regs->ebx;
	x86_sys_debug("  policy=%d\n", policy);

	switch (policy)
	{

	/* SCHED_OTHER */
	case 0:
		return 0;

	/* SCHED_FIFO */
	case 1:
		return 99;

	/* SCHED_RR */
	case 2:
		return 99;

	default:
		fatal("%s: policy not supported.\n%s",
			__FUNCTION__, err_x86_sys_note);
	}

	/* Dead code */
	return 0;
}




/*
 * System call 'sched_get_priority_min' (code 160)
 */

static int x86_sys_sched_get_priority_min_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	int policy;

	/* Arguments */
	policy = regs->ebx;
	x86_sys_debug("  policy=%d\n", policy);

	switch (policy)
	{

	/* SCHED_OTHER */
	case 0:
		return 0;

	/* SCHED_FIFO */
	case 1:
		return 1;

	/* SCHED_RR */
	case 2:
		return 1;

	default:
		fatal("%s: policy not supported.\n%s",
			__FUNCTION__, err_x86_sys_note);
	}

	/* Dead code */
	return 0;
}




/*
 * System call 'nanosleep' (code 162)
 */

static int x86_sys_nanosleep_impl(X86Context *ctx)
{
	X86Emu *emu = ctx->emu;

	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int rqtp;
	unsigned int rmtp;
	unsigned int sec;
	unsigned int nsec;

	long long total;
	long long now;

	/* Arguments */
	rqtp = regs->ebx;
	rmtp = regs->ecx;
	x86_sys_debug("  rqtp=0x%x, rmtp=0x%x\n", rqtp, rmtp);

	/* Get current time */
	now = esim_real_time();

	/* Read structure */
	mem_read(mem, rqtp, 4, &sec);
	mem_read(mem, rqtp + 4, 4, &nsec);
	total = (long long) sec * 1000000 + (nsec / 1000);
	x86_sys_debug("  sleep time (us): %llu\n", total);

	/* Suspend process */
	ctx->wakeup_time = now + total;
	X86ContextSetState(ctx, X86ContextSuspended | X86ContextNanosleep);
	X86EmuProcessEventsSchedule(emu);
	return 0;
}




/*
 * System call 'mremap' (code 163)
 */

static int x86_sys_mremap_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int addr;
	unsigned int old_len;
	unsigned int new_len;
	unsigned int new_addr;

	int flags;

	/* Arguments */
	addr = regs->ebx;
	old_len = regs->ecx;
	new_len = regs->edx;
	flags = regs->esi;
	x86_sys_debug("  addr=0x%x, old_len=0x%x, new_len=0x%x flags=0x%x\n",
		addr, old_len, new_len, flags);

	/* Restrictions */
	assert(!(addr & (MEM_PAGE_SIZE-1)));
	assert(!(old_len & (MEM_PAGE_SIZE-1)));
	assert(!(new_len & (MEM_PAGE_SIZE-1)));
	if (!(flags & 0x1))
		fatal("%s: flags MAP_MAYMOVE must be present", __FUNCTION__);
	if (!old_len || !new_len)
		fatal("%s: old_len or new_len cannot be zero", __FUNCTION__);

	/* New size equals to old size means no action. */
	if (new_len == old_len)
		return addr;

	/* Shrink region. This is always possible. */
	if (new_len < old_len)
	{
		mem_unmap(mem, addr + new_len, old_len - new_len);
		return addr;
	}

	/* Increase region at the same address. This is only possible if
	 * there is enough free space for the new region. */
	if (new_len > old_len && mem_map_space(mem, addr + old_len,
		new_len - old_len) == addr + old_len)
	{
		mem_map(mem, addr + old_len, new_len - old_len,
			mem_access_read | mem_access_write);
		return addr;
	}

	/* A new region must be found for the new size. */
	new_addr = mem_map_space_down(mem, SYS_MMAP_BASE_ADDRESS, new_len);
	if (new_addr == -1)
		fatal("%s: out of guest memory", __FUNCTION__);

	/* Map new region and copy old one */
	mem_map(mem, new_addr, new_len,
		mem_access_read | mem_access_write);
	mem_copy(mem, new_addr, addr, MIN(old_len, new_len));
	mem_unmap(mem, addr, old_len);

	/* Return new address */
	return new_addr;
}




/*
 * System call 'clock_gettime' (code 165)
 */

static struct str_map_t x86_sys_clock_gettime_clk_id_map =
{
	7, {
		{ "CLOCK_REALTIME", 0 },
		{ "CLOCK_MONOTONIC", 1 },
		{ "CLOCK_PROCESS_CPUTIME_ID", 2 },
		{ "CLOCK_THREAD_CPUTIME_ID", 3 },
		{ "CLOCK_MONOTONIC_RAW", 4 },
		{ "CLOCK_REALTIME_COARSE", 5 },
		{ "CLOCK_MONOTONIC_COARSE", 6 }
	}
};

static int x86_sys_clock_gettime_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int clk_id;
	unsigned int ts_ptr;

	long long now;

	char *clk_id_str;

	struct {
		unsigned int sec;
		unsigned int nsec;
	} sim_ts;

	/* Arguments */
	clk_id = regs->ebx;
	ts_ptr = regs->ecx;
	clk_id_str = str_map_value(&x86_sys_clock_gettime_clk_id_map, clk_id);
	x86_sys_debug("  clk_id=0x%x (%s), ts_ptr=0x%x\n",
		clk_id, clk_id_str, ts_ptr);

	/* Initialize */
	sim_ts.sec = 0;
	sim_ts.nsec = 0;

	/* Clock type */
	switch (clk_id)
	{
	case 0:  /* CLOCK_REALTIME */

		/* For CLOCK_REALTIME, return the host real time. This is the same
		 * value that the guest application would see if it ran natively. */
		now = esim_real_time();
		sim_ts.sec = now / 1000000;
		sim_ts.nsec = (now % 1000000) * 1000;
		break;

	case 1:  /* CLOCK_MONOTONIC */
	case 4:  /* CLOCK_MONOTONIC_RAW */

		/* For these two clocks, we want to return simulated time. This
		 * time is tricky to calculate when there could be iterations of the
		 * main simulation loop when no timing simulation happened, but
		 * which still need to be considered to avoid the application
		 * having the illusion of time not going by at all. This is the
		 * strategy assumed to calculate simulated time:
		 *   - A first component is based on the value of 'esim_time',
		 *     considering all simulation cycles when there was an
		 *     active timing simulation of any architecture.
		 *   - A second component will add a time increment for each
		 *     simulation main loop iteration where the global time
		 *     'esim_time' was not incremented. These iterations are
		 *     recorded in variable 'esim_no_forward_cycles'. A default
		 *     value of 1ns per each iteration is considered here.
		 */
		now = esim_time / 1000;  /* Obtain nsec */
		now += esim_no_forward_cycles;  /* One more nsec per iteration */
		sim_ts.sec = now / 1000000000ll;
		sim_ts.nsec = now % 1000000000ll;
		break;

	case 2:  /* CLOCK_PROCESS_CPUTIME_ID */
	case 3:  /* CLOCK_THREAD_CPUTIME_ID */
	case 5:  /* CLOCK_REALTIME_COARSE */
	case 6:  /* CLOCK_MONOTONIC_COARSE */

		fatal("%s: not implemented for 'clk_id' = %d",
			__FUNCTION__, clk_id);
		break;
	
	default:
		fatal("%s: invalid value for 'clk_id' (%d)",
			__FUNCTION__, clk_id);
	}

	/* Debug */
	x86_sys_debug("\tts.tv_sec = %u\n", sim_ts.sec);
	x86_sys_debug("\tts.tv_nsec = %u\n", sim_ts.nsec);

	/* Write to guest memory */
	mem_write(mem, ts_ptr, sizeof sim_ts, &sim_ts);
	return 0;
}




/*
 * System call 'poll' (code 168)
 */

static struct str_map_t sys_poll_event_map =
{
	6, {
		{ "POLLIN",          0x0001 },
		{ "POLLPRI",         0x0002 },
		{ "POLLOUT",         0x0004 },
		{ "POLLERR",         0x0008 },
		{ "POLLHUP",         0x0010 },
		{ "POLLNVAL",        0x0020 }
	}
};

struct sim_pollfd_t
{
	unsigned int fd;
	unsigned short events;
	unsigned short revents;
};

static int x86_sys_poll_impl(X86Context *ctx)
{
	X86Emu *emu = ctx->emu;

	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int pfds;
	unsigned int nfds;

	int timeout;
	int guest_fd;
	int host_fd;
	int err;

	struct sim_pollfd_t guest_fds;
	struct pollfd host_fds;
	struct x86_file_desc_t *desc;

	char events_str[MAX_STRING_SIZE];

	long long now = esim_real_time();

	/* Arguments */
	pfds = regs->ebx;
	nfds = regs->ecx;
	timeout = regs->edx;
	x86_sys_debug("  pfds=0x%x, nfds=%d, timeout=%d\n",
		pfds, nfds, timeout);

	/* Assumptions on host architecture */
	M2S_HOST_GUEST_MATCH(sizeof guest_fds, 8);
	M2S_HOST_GUEST_MATCH(POLLIN, 1);
	M2S_HOST_GUEST_MATCH(POLLPRI, 2);
	M2S_HOST_GUEST_MATCH(POLLOUT, 4);

	/* Supported value */
	if (nfds != 1)
		fatal("%s: not suported for nfds != 1\n%s", __FUNCTION__, err_x86_sys_note);

	/* Read pollfd */
	mem_read(mem, pfds, sizeof guest_fds, &guest_fds);
	guest_fd = guest_fds.fd;
	str_map_flags(&sys_poll_event_map, guest_fds.events, events_str, MAX_STRING_SIZE);
	x86_sys_debug("  guest_fd=%d, events=%s\n", guest_fd, events_str);

	/* Get file descriptor */
	desc = x86_file_desc_table_entry_get(ctx->file_desc_table, guest_fd);
	if (!desc)
		return -EBADF;
	host_fd = desc->host_fd;
	x86_sys_debug("  host_fd=%d\n", host_fd);

	/* Only POLLIN (0x1) and POLLOUT (0x4) supported */
	if (guest_fds.events & ~(POLLIN | POLLOUT))
		fatal("%s: event not supported.\n%s",
			__FUNCTION__, err_x86_sys_note);

	/* Not supported file descriptor */
	if (host_fd < 0)
		fatal("%s: not supported file descriptor.\n%s",
			__FUNCTION__, err_x86_sys_note);

	/* Perform host 'poll' system call with a 0 timeout to distinguish
	 * blocking from non-blocking cases. */
	host_fds.fd = host_fd;
	host_fds.events = guest_fds.events;
	err = poll(&host_fds, 1, 0);
	if (err == -1)
		return -errno;

	/* If host 'poll' returned a value greater than 0, the guest call is non-blocking,
	 * since I/O is ready for the file descriptor. */
	if (err > 0)
	{
		/* Non-blocking POLLOUT on a file. */
		if (guest_fds.events & host_fds.revents & POLLOUT)
		{
			x86_sys_debug("  non-blocking write to file guaranteed\n");
			guest_fds.revents = POLLOUT;
			mem_write(mem, pfds, sizeof guest_fds, &guest_fds);
			return 1;
		}

		/* Non-blocking POLLIN on a file. */
		if (guest_fds.events & host_fds.revents & POLLIN)
		{
			x86_sys_debug("  non-blocking read from file guaranteed\n");
			guest_fds.revents = POLLIN;
			mem_write(mem, pfds, sizeof guest_fds, &guest_fds);
			return 1;
		}

		/* Never should get here */
		panic("%s: unexpected events", __FUNCTION__);
	}

	/* At this point, host 'poll' returned 0, which means that none of the requested
	 * events is ready on the file, so we must suspend until they occur. */
	x86_sys_debug("  process going to sleep waiting for events on file\n");
	ctx->wakeup_time = 0;
	if (timeout >= 0)
		ctx->wakeup_time = now + (long long) timeout * 1000;
	ctx->wakeup_fd = guest_fd;
	ctx->wakeup_events = guest_fds.events;
	X86ContextSetState(ctx, X86ContextSuspended | X86ContextPoll);
	X86EmuProcessEventsSchedule(emu);
	return 0;
}




/*
 * System call 'rt_sigaction' (code 174)
 */

static int x86_sys_rt_sigaction_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	int sig;
	int sigsetsize;

	unsigned int act_ptr;
	unsigned int old_act_ptr;

	struct x86_sigaction_t act;

	/* Arguments */
	sig = regs->ebx;
	act_ptr = regs->ecx;
	old_act_ptr = regs->edx;
	sigsetsize = regs->esi;
	x86_sys_debug("  sig=%d, act_ptr=0x%x, old_act_ptr=0x%x, sigsetsize=0x%x\n",
		sig, act_ptr, old_act_ptr, sigsetsize);
	x86_sys_debug("  signal=%s\n", x86_signal_name(sig));

	/* Invalid signal */
	if (sig < 1 || sig > 64)
		fatal("%s: invalid signal (%d)", __FUNCTION__, sig);

	/* Read new sigaction */
	if (act_ptr)
	{
		mem_read(mem, act_ptr, sizeof act, &act);
		if (debug_status(x86_sys_debug_category))
		{
			FILE *f = debug_file(x86_sys_debug_category);
			x86_sys_debug("  act: ");
			x86_sigaction_dump(&act, f);
			x86_sys_debug("\n    flags: ");
			x86_sigaction_flags_dump(act.flags, f);
			x86_sys_debug("\n    mask: ");
			x86_sigset_dump(act.mask, f);
			x86_sys_debug("\n");
		}
	}

	/* Store previous sigaction */
	if (old_act_ptr)
		mem_write(mem, old_act_ptr, sizeof(struct x86_sigaction_t),
			&ctx->signal_handler_table->sigaction[sig - 1]);

	/* Make new sigaction effective */
	if (act_ptr)
		ctx->signal_handler_table->sigaction[sig - 1] = act;

	/* Return */
	return 0;
}




/*
 * System call 'rt_sigprocmask' (code 175)
 */

static struct str_map_t sys_sigprocmask_how_map =
{
	3, {
		{ "SIG_BLOCK",     0 },
		{ "SIG_UNBLOCK",   1 },
		{ "SIG_SETMASK",   2 }
	}
};

static int x86_sys_rt_sigprocmask_impl(X86Context *ctx)
{
	X86Emu *emu = ctx->emu;

	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int set_ptr;
	unsigned int old_set_ptr;

	int how;
	int sigsetsize;

	unsigned long long set;
	unsigned long long old_set;

	how = regs->ebx;
	set_ptr = regs->ecx;
	old_set_ptr = regs->edx;
	sigsetsize = regs->esi;
	x86_sys_debug("  how=0x%x, set_ptr=0x%x, old_set_ptr=0x%x, sigsetsize=0x%x\n",
		how, set_ptr, old_set_ptr, sigsetsize);
	x86_sys_debug("  how=%s\n", str_map_value(&sys_sigprocmask_how_map, how));

	/* Save old set */
	old_set = ctx->signal_mask_table->blocked;

	/* New set */
	if (set_ptr)
	{
		/* Read it from memory */
		mem_read(mem, set_ptr, 8, &set);
		if (debug_status(x86_sys_debug_category))
		{
			x86_sys_debug("  set=0x%llx ", set);
			x86_sigset_dump(set, debug_file(x86_sys_debug_category));
			x86_sys_debug("\n");
		}

		/* Set new set */
		switch (how)
		{

		/* SIG_BLOCK */
		case 0:
			ctx->signal_mask_table->blocked |= set;
			break;

		/* SIG_UNBLOCK */
		case 1:
			ctx->signal_mask_table->blocked &= ~set;
			break;

		/* SIG_SETMASK */
		case 2:
			ctx->signal_mask_table->blocked = set;
			break;

		default:
			fatal("%s: invalid value for 'how'", __FUNCTION__);
		}
	}

	/* Return old set */
	if (old_set_ptr)
		mem_write(mem, old_set_ptr, 8, &old_set);

	/* A change in the signal mask can cause pending signals to be
	 * able to execute, so check this. */
	X86EmuProcessEventsSchedule(emu);
	X86EmuProcessEvents(emu);

	return 0;
}




/*
 * System call 'rt_sigsuspend' (code 179)
 */

static int x86_sys_rt_sigsuspend_impl(X86Context *ctx)
{
	X86Emu *emu = ctx->emu;

	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int new_set_ptr;
	unsigned long long new_set;
	int sigsetsize;

	new_set_ptr = regs->ebx;
	sigsetsize = regs->ecx;
	x86_sys_debug("  new_set_ptr=0x%x, sigsetsize=%d\n",
		new_set_ptr, sigsetsize);

	/* Read temporary signal mask */
	mem_read(mem, new_set_ptr, 8, &new_set);
	if (debug_status(x86_sys_debug_category))
	{
		FILE *f = debug_file(x86_sys_debug_category);
		x86_sys_debug("  old mask: ");
		x86_sigset_dump(ctx->signal_mask_table->blocked, f);
		x86_sys_debug("\n  new mask: ");
		x86_sigset_dump(new_set, f);
		x86_sys_debug("\n  pending:  ");
		x86_sigset_dump(ctx->signal_mask_table->pending, f);
		x86_sys_debug("\n");
	}

	/* Save old mask and set new one, then suspend. */
	ctx->signal_mask_table->backup = ctx->signal_mask_table->blocked;
	ctx->signal_mask_table->blocked = new_set;
	X86ContextSetState(ctx, X86ContextSuspended | X86ContextSigsuspend);

	/* New signal mask may cause new events */
	X86EmuProcessEventsSchedule(emu);
	X86EmuProcessEvents(emu);
	return 0;
}




/*
 * System call 'getcwd' (code 183)
 */

static int x86_sys_getcwd_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int buf_ptr;

	int size;
	int len;

	char *cwd;

	/* Arguments */
	buf_ptr = regs->ebx;
	size = regs->ecx;
	x86_sys_debug("  buf_ptr=0x%x, size=0x%x\n", buf_ptr, size);

	/* Get working directory */
	cwd = ctx->loader->cwd;
	len = strlen(cwd);

	/* Does not fit */
	if (size <= len)
		return -ERANGE;

	/* Return */
	mem_write_string(mem, buf_ptr, cwd);
	return len + 1;
}




/*
 * System call 'getrlimit' (code 191)
 */

static int x86_sys_getrlimit_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int res;
	unsigned int rlim_ptr;

	char *res_str;

	struct sim_rlimit sim_rlimit;

	/* Arguments */
	res = regs->ebx;
	rlim_ptr = regs->ecx;
	res_str = str_map_value(&sys_rlimit_res_map, res);
	x86_sys_debug("  res=0x%x, rlim_ptr=0x%x\n", res, rlim_ptr);
	x86_sys_debug("  res=%s\n", res_str);

	switch (res)
	{

	case 2:  /* RLIMIT_DATA */
	{
		sim_rlimit.cur = 0xffffffff;
		sim_rlimit.max = 0xffffffff;
		break;
	}

	case 3:  /* RLIMIT_STACK */
	{
		sim_rlimit.cur = ctx->loader->stack_size;
		sim_rlimit.max = 0xffffffff;
		break;
	}

	case 7:  /* RLIMIT_NOFILE */
	{
		sim_rlimit.cur = 0x400;
		sim_rlimit.max = 0x400;
		break;
	}

	default:
		fatal("%s: not implemented for res = %s.\n%s",
			__FUNCTION__, res_str, err_x86_sys_note);
	}

	/* Return structure */
	mem_write(mem, rlim_ptr, sizeof(struct sim_rlimit), &sim_rlimit);
	x86_sys_debug("  ret: cur=0x%x, max=0x%x\n", sim_rlimit.cur, sim_rlimit.max);

	/* Return */
	return 0;
}




/*
 * System call 'mmap2' (code 192)
 */

static int x86_sys_mmap2_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int addr;
	unsigned int len;

	int prot;
	int flags;
	int offset;
	int guest_fd;

	char prot_str[MAX_STRING_SIZE];
	char flags_str[MAX_STRING_SIZE];

	/* Arguments */
	addr = regs->ebx;
	len = regs->ecx;
	prot = regs->edx;
	flags = regs->esi;
	guest_fd = regs->edi;
	offset = regs->ebp;

	/* Debug */
	x86_sys_debug("  addr=0x%x, len=%u, prot=0x%x, flags=0x%x, guest_fd=%d, offset=0x%x\n",
		addr, len, prot, flags, guest_fd, offset);
	str_map_flags(&sys_mmap_prot_map, prot, prot_str, MAX_STRING_SIZE);
	str_map_flags(&sys_mmap_flags_map, flags, flags_str, MAX_STRING_SIZE);
	x86_sys_debug("  prot=%s, flags=%s\n", prot_str, flags_str);

	/* System calls 'mmap' and 'mmap2' only differ in the interpretation of
	 * argument 'offset'. Here, it is given in memory pages. */
	return x86_sys_mmap(ctx, addr, len, prot, flags, guest_fd, offset << MEM_PAGE_SHIFT);
}




/*
 * System call 'ftruncate64' (code 194)
 */

static int x86_sys_ftruncate64_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	int fd;
	int host_fd;
	int err;
	
	unsigned int length;

	/* Arguments */
	fd = regs->ebx;
	length = regs->ecx;
	x86_sys_debug("  fd=%d, length=0x%x\n", fd, length);

	/* Get host descriptor */
	host_fd = x86_file_desc_table_get_host_fd(ctx->file_desc_table, fd);
	x86_sys_debug("  host_fd=%d\n", host_fd);

	err = ftruncate(host_fd, length);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
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

static void sys_stat_host_to_guest(struct sim_stat64_t *guest, struct stat *host)
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

	x86_sys_debug("  stat64 structure:\n");
	x86_sys_debug("    dev=%lld, ino=%lld, mode=%d, nlink=%d\n",
		guest->dev, guest->ino, guest->mode, guest->nlink);
	x86_sys_debug("    uid=%d, gid=%d, rdev=%lld\n",
		guest->uid, guest->gid, guest->rdev);
	x86_sys_debug("    size=%lld, blksize=%d, blocks=%lld\n",
		guest->size, guest->blksize, guest->blocks);
}

static int x86_sys_stat64_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int file_name_ptr;
	unsigned int statbuf_ptr;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];

	struct stat statbuf;
	struct sim_stat64_t sim_statbuf;

	int length;
	int err;

	/* Arguments */
	file_name_ptr = regs->ebx;
	statbuf_ptr = regs->ecx;
	x86_sys_debug("  file_name_ptr=0x%x, statbuf_ptr=0x%x\n",
			file_name_ptr, statbuf_ptr);

	/* Read file name */
	length = mem_read_string(mem, file_name_ptr, sizeof file_name, file_name);
	if (length == sizeof file_name)
		fatal("%s: buffer too small", __FUNCTION__);
	
	/* Get full path */
	X86ContextGetFullPath(ctx, file_name, full_path, sizeof full_path);
	x86_sys_debug("  file_name='%s', full_path='%s'\n", file_name, full_path);

	/* Host call */
	err = stat(full_path, &statbuf);
	if (err == -1)
		return -errno;
	
	/* Copy guest structure */
	sys_stat_host_to_guest(&sim_statbuf, &statbuf);
	mem_write(mem, statbuf_ptr, sizeof(sim_statbuf), &sim_statbuf);
	return 0;
}




/*
 * System call 'lstat64' (code 196)
 */

static int x86_sys_lstat64_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int file_name_ptr;
	unsigned int statbuf_ptr;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];

	int length;
	int err;

	struct stat statbuf;
	struct sim_stat64_t sim_statbuf;

	/* Arguments */
	file_name_ptr = regs->ebx;
	statbuf_ptr = regs->ecx;
	x86_sys_debug("  file_name_ptr=0x%x, statbuf_ptr=0x%x\n", file_name_ptr, statbuf_ptr);

	/* Read file name */
	length = mem_read_string(mem, file_name_ptr, sizeof file_name, file_name);
	if (length == sizeof file_name)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get full path */
	X86ContextGetFullPath(ctx, file_name, full_path, sizeof full_path);
	x86_sys_debug("  file_name='%s', full_path='%s'\n", file_name, full_path);

	/* Host call */
	err = lstat(full_path, &statbuf);
	if (err == -1)
		return -errno;
	
	/* Return */
	sys_stat_host_to_guest(&sim_statbuf, &statbuf);
	mem_write(mem, statbuf_ptr, sizeof sim_statbuf, &sim_statbuf);
	return 0;
}




/*
 * System call 'fstat64' (code 197)
 */

static int x86_sys_fstat64_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	int fd;
	int host_fd;
	int err;

	unsigned int statbuf_ptr;

	struct stat statbuf;
	struct sim_stat64_t sim_statbuf;

	/* Arguments */
	fd = regs->ebx;
	statbuf_ptr = regs->ecx;
	x86_sys_debug("  fd=%d, statbuf_ptr=0x%x\n", fd, statbuf_ptr);

	/* Get host descriptor */
	host_fd = x86_file_desc_table_get_host_fd(ctx->file_desc_table, fd);
	x86_sys_debug("  host_fd=%d\n", host_fd);

	/* Host call */
	err = fstat(host_fd, &statbuf);
	if (err == -1)
		return -errno;

	/* Return */
	sys_stat_host_to_guest(&sim_statbuf, &statbuf);
	mem_write(mem, statbuf_ptr, sizeof sim_statbuf, &sim_statbuf);
	return 0;
}




/*
 * System call 'getuid' (code 199)
 */

static int x86_sys_getuid_impl(X86Context *ctx)
{
	return getuid();
}




/*
 * System call 'getgid' (code 200)
 */

static int x86_sys_getgid_impl(X86Context *ctx)
{
	return getgid();
}




/*
 * System call 'geteuid' (code 201)
 */

static int x86_sys_geteuid_impl(X86Context *ctx)
{
	return geteuid();
}




/*
 * System call 'getegid' (code 202)
 */

static int x86_sys_getegid_impl(X86Context *ctx)
{
	return getegid();
}




/*
 * System call 'chown' (code 212)
 */

static int x86_sys_chown_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int file_name_ptr;

	int owner;
	int group;
	int len;
	int err;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];

	/* Arguments */
	file_name_ptr = regs->ebx;
	owner = regs->ecx;
	group = regs->edx;
	x86_sys_debug("  file_name_ptr=0x%x, owner=%d, group=%d\n", file_name_ptr, owner, group);

	/* Read file name */
	len = mem_read_string(mem, file_name_ptr, sizeof file_name, file_name);
	if (len == sizeof file_name)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get full path */
	X86ContextGetFullPath(ctx, file_name, full_path, sizeof full_path);
	x86_sys_debug("  filename='%s', fullpath='%s'\n", file_name, full_path);

	/* Host call */
	err = chown(full_path, owner, group);
	if (err == -1)
		return -errno;

	/* Return */
	return err;
}




/*
 * System call 'madvise' (219)
 */

static int x86_sys_madvise_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int start;
	unsigned int len;

	int advice;

	/* Arguments */
	start = regs->ebx;
	len = regs->ecx;
	advice = regs->edx;
	x86_sys_debug("  start=0x%x, len=%d, advice=%d\n", start, len, advice);

	/* System call ignored */
	return 0;
}




/*
 * System call 'getdents64'
 */

struct host_dirent_t
{
	long d_ino;
	off_t d_off;
	unsigned short d_reclen;
	char d_name[];
};

struct guest_dirent64_t
{
	unsigned long long d_ino;
	long long d_off;
	unsigned short d_reclen;
	unsigned char d_type;
	char d_name[];
} __attribute__((packed));

static int x86_sys_getdents64_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int pdirent;
	unsigned int count;

	int fd;
	int host_fd;
	int nread;
	int host_offs;
	int guest_offs;

	void *buf;

	struct host_dirent_t *host_dirent;
	struct guest_dirent64_t guest_dirent;

	/* Arguments */
	fd = regs->ebx;
	pdirent = regs->ecx;
	count = regs->edx;
	x86_sys_debug("  fd=%d, pdirent=0x%x, count=%d\n", fd, pdirent, count);

	/* Get host descriptor */
	host_fd = x86_file_desc_table_get_host_fd(ctx->file_desc_table, fd);
	x86_sys_debug("  host_fd=%d\n", host_fd);
	if (host_fd < 0)
		return -EBADF;

	/* Host call */
	buf = xcalloc(1, count);
	nread = syscall(SYS_getdents, host_fd, buf, count);
	if (nread < 0)
		fatal("%s: host call failed", __FUNCTION__);

	/* Error or no more entries */
	if (!nread)
	{
		free(buf);
		return 0;
	}

	/* Copy to host memory */
	host_offs = 0;
	guest_offs = 0;
	while (host_offs < nread)
	{
		host_dirent = (struct host_dirent_t *) (buf + host_offs);
		guest_dirent.d_ino = host_dirent->d_ino;
		guest_dirent.d_off = host_dirent->d_off;
		guest_dirent.d_reclen = (27 + strlen(host_dirent->d_name)) / 8 * 8;
		guest_dirent.d_type = * (char *) (buf + host_offs + host_dirent->d_reclen - 1);

		x86_sys_debug("    d_ino=%lld ", guest_dirent.d_ino);
		x86_sys_debug("d_off=%lld ", guest_dirent.d_off);
		x86_sys_debug("d_reclen=%u(host),%u(guest) ", host_dirent->d_reclen, guest_dirent.d_reclen);
		x86_sys_debug("d_name='%s'\n", host_dirent->d_name);

		mem_write(mem, pdirent + guest_offs, 8, &guest_dirent.d_ino);
		mem_write(mem, pdirent + guest_offs + 8, 8, &guest_dirent.d_off);
		mem_write(mem, pdirent + guest_offs + 16, 2, &guest_dirent.d_reclen);
		mem_write(mem, pdirent + guest_offs + 18, 1, &guest_dirent.d_type);
		mem_write_string(mem, pdirent + guest_offs + 19, host_dirent->d_name);

		host_offs += host_dirent->d_reclen;
		guest_offs += guest_dirent.d_reclen;
		if (guest_offs > count)
			fatal("getdents: host buffer too small");
	}

	/* Return */
	free(buf);
	x86_sys_debug("  ret=%d (host), %d (guest)\n", host_offs, guest_offs);
	return guest_offs;
}




/*
 * System call 'fcntl64' (code 221)
 */

static struct str_map_t sys_fcntl_cmp_map =
{
	15, {
		{ "F_DUPFD", 0 },
		{ "F_GETFD", 1 },
		{ "F_SETFD", 2 },
		{ "F_GETFL", 3 },
		{ "F_SETFL", 4 },
		{ "F_GETLK", 5 },
		{ "F_SETLK", 6 },
		{ "F_SETLKW", 7 },
		{ "F_SETOWN", 8 },
		{ "F_GETOWN", 9 },
		{ "F_SETSIG", 10 },
		{ "F_GETSIG", 11 },
		{ "F_GETLK64", 12 },
		{ "F_SETLK64", 13 },
		{ "F_SETLKW64", 14 }
	}
};

static int x86_sys_fcntl64_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	int guest_fd;
	int cmd;
	int err;

	unsigned int arg;

	char *cmd_name;
	char flags_str[MAX_STRING_SIZE];

	struct x86_file_desc_t *desc;

	/* Arguments */
	guest_fd = regs->ebx;
	cmd = regs->ecx;
	arg = regs->edx;
	x86_sys_debug("  guest_fd=%d, cmd=%d, arg=0x%x\n",
		guest_fd, cmd, arg);
	cmd_name = str_map_value(&sys_fcntl_cmp_map, cmd);
	x86_sys_debug("    cmd=%s\n", cmd_name);

	/* Get file descriptor table entry */
	desc = x86_file_desc_table_entry_get(ctx->file_desc_table, guest_fd);
	if (!desc)
		return -EBADF;
	if (desc->host_fd < 0)
		fatal("%s: not supported for this type of file", __FUNCTION__);
	x86_sys_debug("    host_fd=%d\n", desc->host_fd);

	/* Process command */
	switch (cmd)
	{

	/* F_GETFD */
	case 1:
		err = fcntl(desc->host_fd, F_GETFD);
		if (err == -1)
			err = -errno;
		break;

	/* F_SETFD */
	case 2:
		err = fcntl(desc->host_fd, F_SETFD, arg);
		if (err == -1)
			err = -errno;
		break;

	/* F_GETFL */
	case 3:
		err = fcntl(desc->host_fd, F_GETFL);
		if (err == -1)
			err = -errno;
		else
		{
			str_map_flags(&sys_open_flags_map, err, flags_str, MAX_STRING_SIZE);
			x86_sys_debug("    ret=%s\n", flags_str);
		}
		break;

	/* F_SETFL */
	case 4:
		str_map_flags(&sys_open_flags_map, arg, flags_str, MAX_STRING_SIZE);
		x86_sys_debug("    arg=%s\n", flags_str);
		desc->flags = arg;

		err = fcntl(desc->host_fd, F_SETFL, arg);
		if (err == -1)
			err = -errno;
		break;

	default:

		err = 0;
		fatal("%s: command %s not implemented.\n%s",
			__FUNCTION__, cmd_name, err_x86_sys_note);
	}

	/* Return */
	return err;
}




/*
 * System call 'gettid' (code 224)
 */

static int x86_sys_gettid_impl(X86Context *ctx)
{
	/* FIXME: return different 'tid' for threads, but the system call
	 * 'getpid' should return the same 'pid' for threads from the same group
	 * created with CLONE_THREAD flag. */
	return ctx->pid;
}




/*
 * System call 'futex' (code 240)
 */

static struct str_map_t sys_futex_cmd_map =
{
	13, {
		{ "FUTEX_WAIT",              0 },
		{ "FUTEX_WAKE",              1 },
		{ "FUTEX_FD",                2 },
		{ "FUTEX_REQUEUE",           3 },
		{ "FUTEX_CMP_REQUEUE",       4 },
		{ "FUTEX_WAKE_OP",           5 },
		{ "FUTEX_LOCK_PI",           6 },
		{ "FUTEX_UNLOCK_PI",         7 },
		{ "FUTEX_TRYLOCK_PI",        8 },
		{ "FUTEX_WAIT_BITSET",       9 },
		{ "FUTEX_WAKE_BITSET",       10 },
		{ "FUTEX_WAIT_REQUEUE_PI",   11 },
		{ "FUTEX_CMP_REQUEUE_PI",    12 }
	}
};

static int x86_sys_futex_impl(X86Context *ctx)
{
	X86Emu *emu = ctx->emu;

	/* Prototype: sys_futex(void *addr1, int op, int val1, struct timespec *timeout,
	 *   void *addr2, int val3); */

	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int addr1;
	unsigned int timeout_ptr;
	unsigned int addr2;
	unsigned int timeout_sec;
	unsigned int timeout_usec;

	unsigned int cmd;
	unsigned int futex;
	unsigned int bitset;

	int op;
	int val1;
	int val3;
	int ret;

	/* Arguments */
	addr1 = regs->ebx;
	op = regs->ecx;
	val1 = regs->edx;
	timeout_ptr = regs->esi;
	addr2 = regs->edi;
	val3 = regs->ebp;
	x86_sys_debug("  addr1=0x%x, op=%d, val1=%d, ptimeout=0x%x, addr2=0x%x, val3=%d\n",
		addr1, op, val1, timeout_ptr, addr2, val3);


	/* Command - 'cmd' is obtained by removing 'FUTEX_PRIVATE_FLAG' (128) and
	 * 'FUTEX_CLOCK_REALTIME' from 'op'. */
	cmd = op & ~(256|128);
	mem_read(mem, addr1, 4, &futex);
	x86_sys_debug("  futex=%d, cmd=%d (%s)\n",
		futex, cmd, str_map_value(&sys_futex_cmd_map, cmd));

	switch (cmd)
	{

	case 0:  /* FUTEX_WAIT */
	case 9:  /* FUTEX_WAIT_BITSET */
	{
		/* Default bitset value (all bits set) */
		bitset = cmd == 9 ? val3 : 0xffffffff;

		/* First, we compare the value of the futex with val1. If it's not the
		 * same, we exit with the error EWOULDBLOCK (=EAGAIN). */
		if (futex != val1)
			return -EAGAIN;

		/* Read timeout */
		if (timeout_ptr)
		{
			fatal("syscall futex: FUTEX_WAIT not supported with timeout");
			mem_read(mem, timeout_ptr, 4, &timeout_sec);
			mem_read(mem, timeout_ptr + 4, 4, &timeout_usec);
			x86_sys_debug("  timeout={sec %d, usec %d}\n",
				timeout_sec, timeout_usec);
		}
		else
		{
			timeout_sec = 0;
			timeout_usec = 0;
		}

		/* Suspend thread in the futex. */
		ctx->wakeup_futex = addr1;
		ctx->wakeup_futex_bitset = bitset;
		ctx->wakeup_futex_sleep = ++emu->futex_sleep_count;
		X86ContextSetState(ctx, X86ContextSuspended | X86ContextFutex);
		return 0;
	}

	case 1:  /* FUTEX_WAKE */
	case 10:  /* FUTEX_WAKE_BITSET */
	{
		/* Default bitset value (all bits set) */
		bitset = cmd == 10 ? val3 : 0xffffffff;
		ret = X86ContextFutexWake(ctx, addr1, val1, bitset);
		x86_sys_debug("  futex at 0x%x: %d processes woken up\n", addr1, ret);
		return ret;
	}

	case 4: /* FUTEX_CMP_REQUEUE */
	{
		int requeued = 0;
		X86Context *temp_ctx;

		/* 'ptimeout' is interpreted here as an integer; only supported for INTMAX */
		if (timeout_ptr != 0x7fffffff)
			fatal("%s: FUTEX_CMP_REQUEUE: only supported for ptimeout=INTMAX", __FUNCTION__);

		/* The value of val3 must be the same as the value of the futex
		 * at 'addr1' (stored in 'futex') */
		if (futex != val3)
			return -EAGAIN;

		/* Wake up 'val1' threads from futex at 'addr1'. The number of woken up threads
		 * is the return value of the system call. */
		ret = X86ContextFutexWake(ctx, addr1, val1, 0xffffffff);
		x86_sys_debug("  futex at 0x%x: %d processes woken up\n", addr1, ret);

		/* The rest of the threads waiting in futex 'addr1' are requeued into futex 'addr2' */
		for (temp_ctx = emu->suspended_list_head; temp_ctx;
				temp_ctx = temp_ctx->suspended_list_next)
		{
			if (X86ContextGetState(temp_ctx, X86ContextFutex)
					&& temp_ctx->wakeup_futex == addr1)
			{
				temp_ctx->wakeup_futex = addr2;
				requeued++;
			}
		}
		x86_sys_debug("  futex at 0x%x: %d processes requeued to futex 0x%x\n",
			addr1, requeued, addr2);
		return ret;
	}

	case 5: /* FUTEX_WAKE_OP */
	{
		int op;
		int oparg;
		int cmp;
		int cmparg;

		int val2 = timeout_ptr;
		int oldval;
		int newval = 0;
		int cond = 0;
		int ret = 0;

		op = (val3 >> 28) & 0xf;
		cmp = (val3 >> 24) & 0xf;
		oparg = (val3 >> 12) & 0xfff;
		cmparg = val3 & 0xfff;

		mem_read(mem, addr2, 4, &oldval);
		switch (op)
		{
		case 0: /* FUTEX_OP_SET */
			newval = oparg;
			break;
		case 1: /* FUTEX_OP_ADD */
			newval = oldval + oparg;
			break;
		case 2: /* FUTEX_OP_OR */
			newval = oldval | oparg;
			break;
		case 3: /* FUTEX_OP_AND */
			newval = oldval & oparg;
			break;
		case 4: /* FOTEX_OP_XOR */
			newval = oldval ^ oparg;
			break;
		default:
			fatal("%s: FUTEX_WAKE_OP: invalid operation", __FUNCTION__);
		}
		mem_write(mem, addr2, 4, &newval);

		ret = X86ContextFutexWake(ctx, addr1, val1, 0xffffffff);

		switch (cmp)
		{
		case 0: /* FUTEX_OP_CMP_EQ */
			cond = oldval == cmparg;
			break;
		case 1: /* FUTEX_OP_CMP_NE */
			cond = oldval != cmparg;
			break;
		case 2: /* FUTEX_OP_CMP_LT */
			cond = oldval < cmparg;
			break;
		case 3: /* FUTEX_OP_CMP_LE */
			cond = oldval <= cmparg;
			break;
		case 4: /* FUTEX_OP_CMP_GT */
			cond = oldval > cmparg;
			break;
		case 5: /* FUTEX_OP_CMP_GE */
			cond = oldval >= cmparg;
			break;
		default:
			fatal("%s: FUTEX_WAKE_OP: invalid condition", __FUNCTION__);
		}
		if (cond)
			ret += X86ContextFutexWake(ctx, addr2, val2, 0xffffffff);

		/* FIXME: we are returning the total number of threads waken up
		 * counting both calls to x86_ctx_futex_wake. Is this correct? */
		return ret;
	}

	default:
		fatal("%s: not implemented for cmd=%d (%s).\n%s",
			__FUNCTION__, cmd, str_map_value(&sys_futex_cmd_map, cmd), err_x86_sys_note);
	}

	/* Dead code */
	return 0;
}




/*
 * System call 'sched_setaffinity' (code 241)
 */

static int x86_sys_sched_setaffinity_impl(X86Context *ctx)
{
	X86Emu *emu = ctx->emu;

	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;
	X86Context *target_ctx;

	int err = 0;
	int pid;
	int size;
	int node;
	int num_nodes;
	int num_bits;

	int i;
	int j;

	unsigned int mask_ptr;
	unsigned char *mask;

	/* Arguments */
	pid = regs->ebx;
	size = regs->ecx;
	mask_ptr = regs->edx;
	x86_sys_debug("  pid=%d, size=%d, mask_ptr=0x%x\n",
			pid, size, mask_ptr);

	/* Check valid size (assume reasonable maximum of 1KB) */
	if (!IN_RANGE(size, 0, 1 << 10))
		fatal("%s: invalid range for 'size' (%d)", __FUNCTION__, size);

	/* Read mask */
	mask = xcalloc(1, size);
	mem_read(mem, mask_ptr, size, mask);

	/* Dump it */
	x86_sys_debug("  CPUs = {");
	for (i = 0; i < size; i++)
		for (j = 0; j < 8; j++)
			if (mask[i] & (1 << j))
				x86_sys_debug(" %d", i * 8 + j);
	x86_sys_debug(" }\n");

	/* Find context associated with 'pid'. If the value given in 'pid' is
	 * zero, the current context is used. */
	target_ctx = pid ? X86EmuGetContext(emu, pid) : ctx;
	if (!target_ctx)
	{
		err = -ESRCH;
		goto out;
	}

	/* Count number of effective valid bits in the mask. We need at least
	 * one bit to be set for the context to make progress hereafter. */
	num_bits = 0;
	node = 0;
	num_nodes = x86_cpu_num_cores * x86_cpu_num_threads;
	for (i = 0; i < size && node < num_nodes; i++)
	{
		for (j = 0; j < 8 && node < num_nodes; j++)
		{
			num_bits += mask[i] & (1 << j) ? 1 : 0;
			node++;
		}
	}
	if (!num_bits)
	{
		err = -EINVAL;
		goto out;
	}

	/* Set context affinity */
	node = 0;
	for (i = 0; i < size && node < num_nodes; i++)
		for (j = 0; j < 8 && node < num_nodes; j++)
			bit_map_set(target_ctx->affinity, node++,
					1, mask[i] & (1 << j) ? 1 : 0);

	/* Changing the context affinity might force it to be evicted and unmapped
	 * from the current node where it is running (timing simulation only). We
	 * need to force a call to the scheduler. */
	emu->schedule_signal = 1;

out:
	/* Success */
	free(mask);
	return err;
}




/*
 * System call 'sched_getaffinity' (code 242)
 */

static int x86_sys_sched_getaffinity_impl(X86Context *ctx)
{
	X86Emu *emu = ctx->emu;

	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;
	X86Context *target_ctx;

	int pid;
	int size;
	int node;
	int num_nodes;

	int i;
	int j;

	unsigned int mask_ptr;
	unsigned char *mask;

	/* Arguments */
	pid = regs->ebx;
	size = regs->ecx;
	mask_ptr = regs->edx;
	x86_sys_debug("  pid=%d, size=%d, mask_ptr=0x%x\n",
			pid, size, mask_ptr);
	
	/* Check valid size (assume reasonable maximum of 1KB) */
	if (!IN_RANGE(size, 0, 1 << 10))
		fatal("%s: invalid range for 'size' (%d)", __FUNCTION__, size);

	/* Find context associated with 'pid'. If the value given in 'pid' is
	 * zero, the current context is used. */
	target_ctx = pid ? X86EmuGetContext(emu, pid) : ctx;
	if (!target_ctx)
		return -ESRCH;

	/* Allocate mask */
	mask = xcalloc(1, size);

	/* Read mask from context affinity bitmap */
	node = 0;
	num_nodes = x86_cpu_num_cores * x86_cpu_num_threads;
	for (i = 0; i < size && node < num_nodes; i++)
		for (j = 0; j < 8 && node < num_nodes; j++)
			mask[i] |= bit_map_get(target_ctx->affinity,
					node++, 1) << j;
	
	/* Return mask */
	mem_write(mem, mask_ptr, size, mask);
	free(mask);

	/* Return sizeof(cpu_set_t) = 32 */
	return 32;
}




/*
 * System call 'set_thread_area' (code 243)
 */

static int x86_sys_set_thread_area_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int uinfo_ptr;

	struct sim_user_desc uinfo;

	/* Arguments */
	uinfo_ptr = regs->ebx;
	x86_sys_debug("  uinfo_ptr=0x%x\n", uinfo_ptr);

	/* Read structure */
	mem_read(mem, uinfo_ptr, sizeof uinfo, &uinfo);
	x86_sys_debug("  entry_number=0x%x, base_addr=0x%x, limit=0x%x\n",
		uinfo.entry_number, uinfo.base_addr, uinfo.limit);
	x86_sys_debug("  seg_32bit=0x%x, contents=0x%x, read_exec_only=0x%x\n",
		uinfo.seg_32bit, uinfo.contents, uinfo.read_exec_only);
	x86_sys_debug("  limit_in_pages=0x%x, seg_not_present=0x%x, useable=0x%x\n",
		uinfo.limit_in_pages, uinfo.seg_not_present, uinfo.useable);
	if (!uinfo.seg_32bit)
		fatal("syscall set_thread_area: only 32-bit segments supported");

	/* Limit given in pages (4KB units) */
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

	/* Return */
	return 0;
}




/*
 * System call 'fadvise64' (code 250)
 */

static int x86_sys_fadvise64_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	int fd;
	int advice;

	unsigned int off_hi;
	unsigned int off_lo;
	unsigned int len;

	/* Arguments */
	fd = regs->ebx;
	off_lo = regs->ecx;
	off_hi = regs->edx;
	len = regs->esi;
	advice = regs->edi;
	x86_sys_debug("  fd=%d, off={0x%x, 0x%x}, len=%d, advice=%d\n",
		fd, off_hi, off_lo, len, advice);

	/* System call ignored */
	return 0;
}




/*
 * System call 'exit_group' (code 252)
 */

static int x86_sys_exit_group_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	int status;

	/* Arguments */
	status = regs->ebx;
	x86_sys_debug("  status=%d\n", status);

	/* Finish */
	X86ContextFinishGroup(ctx, status);
	return 0;
}




/*
 * System call 'set_tid_address' (code 258)
 */

static int x86_sys_set_tid_address_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int tidptr;

	/* Arguments */
	tidptr = regs->ebx;
	x86_sys_debug("  tidptr=0x%x\n", tidptr);

	ctx->clear_child_tid = tidptr;
	return ctx->pid;
}




/*
 * System call 'clock_getres' (code 266)
 */

static int x86_sys_clock_getres_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int clk_id;
	unsigned int pres;
	unsigned int tv_sec;
	unsigned int tv_nsec;

	/* Arguments */
	clk_id = regs->ebx;
	pres = regs->ecx;
	x86_sys_debug("  clk_id=%d\n", clk_id);
	x86_sys_debug("  pres=0x%x\n", pres);

	/* Return */
	tv_sec = 0;
	tv_nsec = 1;
	mem_write(mem, pres, 4, &tv_sec);
	mem_write(mem, pres + 4, 4, &tv_nsec);
	return 0;
}




/*
 * System call 'statfs64' (code 268)
 */

struct sim_statfs64_t
{					/* off	sz */
	unsigned int type;		/* 0	4 */
	unsigned int bsize;		/* 4	4 */
	unsigned long long blocks;	/* 8	8 */
	unsigned long long bfree;	/* 16	8 */
	unsigned long long bavail;	/* 24	8 */
	unsigned long long files;	/* 32	8 */
	unsigned long long ffree;	/* 40	8 */
	unsigned int fsid[2];		/* 48	8 */
	unsigned int namelen;		/* 56	4 */
	unsigned int frsize;		/* 60	4 */
	unsigned int flags;		/* 64	4 */
	unsigned int spare[4];		/* 68	16 */
} __attribute__((packed));

static void sim_statfs64_host_to_guest(struct sim_statfs64_t *host, struct statfs *guest)
{
	M2S_HOST_GUEST_MATCH(sizeof(*host), 84);
	memset(host, 0, sizeof(*host));
	host->type = guest->f_type;
	host->bsize = guest->f_bsize;
	host->blocks = guest->f_blocks;
	host->bfree = guest->f_bfree;
	host->bavail = guest->f_bavail;
	host->files = guest->f_files;
	host->ffree = guest->f_ffree;
	memcpy(&host->fsid, &guest->f_fsid, MIN(sizeof host->fsid, sizeof guest->f_fsid));
	host->namelen = guest->f_namelen;
	host->frsize = guest->f_frsize;
}

static int x86_sys_statfs64_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int path_ptr;
	unsigned int sz;
	unsigned int statfs_buf_ptr;

	char path[MAX_PATH_SIZE];

	struct statfs statfs_buf;
	struct sim_statfs64_t sim_statfs_buf;

	int length;
	int err;

	/* Arguments */
	path_ptr = regs->ebx;
	sz = regs->ecx;
	statfs_buf_ptr = regs->edx;
	x86_sys_debug("  path_ptr=0x%x, sz=%d, statsf_buf_ptr=0x%x\n",
			path_ptr, sz, statfs_buf_ptr);
	
	/* Check 'sz' argument */
	if (sz != sizeof sim_statfs_buf)
		fatal("%s: incompatible size of 'statfs' structure (sz = %d != %d)\n",
			__FUNCTION__, sz, (int) sizeof sim_statfs_buf);

	/* Read path */
	length = mem_read_string(mem, path_ptr, sizeof path, path);
	if (length == sizeof path)
		fatal("%s: buffer too small", __FUNCTION__);
	x86_sys_debug("  path='%s'\n", path);
	
	/* Host call */
	err = statfs(path, &statfs_buf);
	if (err == -1)
		return -errno;
	
	/* Copy guest structure */
	sim_statfs64_host_to_guest(&sim_statfs_buf, &statfs_buf);
	mem_write(mem, statfs_buf_ptr, sizeof sim_statfs_buf, &sim_statfs_buf);
	return 0;
}



/*
 * System call 'tgkill' (code 270)
 */

static int x86_sys_tgkill_impl(X86Context *ctx)
{
	X86Emu *emu = ctx->emu;

	struct x86_regs_t *regs = ctx->regs;

	int tgid;
	int pid;
	int sig;

	X86Context *temp_ctx;

	/* Arguments */
	tgid = regs->ebx;
	pid = regs->ecx;
	sig = regs->edx;
	x86_sys_debug("  tgid=%d, pid=%d, sig=%d (%s)\n",
		tgid, pid, sig, x86_signal_name(sig));

	/* Implementation restrictions. */
	if (tgid == -1)
		fatal("%s: not supported for tgid = -1\n%s",
			__FUNCTION__, err_x86_sys_note);

	/* Find context referred by pid. */
	temp_ctx = X86EmuGetContext(emu, pid);
	if (!temp_ctx)
		fatal("%s: invalid pid (%d)", __FUNCTION__, pid);

	/* Send signal */
	x86_sigset_add(&temp_ctx->signal_mask_table->pending, sig);
	X86ContextHostThreadSuspendCancel(temp_ctx);
	X86EmuProcessEventsSchedule(emu);
	X86EmuProcessEvents(emu);
	return 0;
}




/*
 * System call 'openat' (code 295)
 */

static int x86_sys_openat_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;
	struct x86_file_desc_t *desc;

	unsigned int path_ptr;

	char path[MAX_STRING_SIZE];
	char full_path[MAX_STRING_SIZE];
	char temp_path[MAX_STRING_SIZE];
	char flags_str[MAX_STRING_SIZE];

	int dirfd;
	int flags;
	int mode;
	int length;
	int host_fd;

	/* Arguments */
	dirfd = regs->ebx;
	path_ptr = regs->ecx;
	flags = regs->edx;
	mode = regs->esi;

	/* Debug */
	str_map_flags(&sys_open_flags_map, flags, flags_str, sizeof flags_str);
	x86_sys_debug("  dirfd=%d, path_ptr=0x%x, flags=0x%x %s, mode=0x%x\n",
		dirfd, path_ptr, flags, flags_str, mode);

	/* Read path */
	length = mem_read_string(mem, path_ptr, sizeof path, path);
	if (length == sizeof path)
		fatal("%s: buffer too small", __FUNCTION__);
	x86_sys_debug("  path='%s'\n", path);

	/* Implemented cases:
	 * dirfd = AT_FDCWD (-100), path is relative -> path is relative to current directory.
	 * path is absolute -> dirfd ignored
	 * dirfd != AT_FDCWD, path is relative -> path is relative to 'difd' (not implemented)
	 */
	if (dirfd != -100 && path[0] != '/')
		fatal("%s: difd != AT_FDCWD with relative path not implemented", __FUNCTION__);

	/* Full path */
	X86ContextGetFullPath(ctx, path, full_path, sizeof full_path);
	x86_sys_debug("  full_path='%s'\n", full_path);

	/* The dynamic linker uses the 'open' system call to open shared libraries.
	 * We need to intercept here attempts to access runtime libraries and
	 * redirect them to our own Multi2Sim runtimes. */
	if (runtime_redirect(full_path, temp_path, sizeof temp_path))
		snprintf(full_path, sizeof full_path, "%s", temp_path);

	/* Virtual files */
	if (!strncmp(full_path, "/proc/", 6))
		fatal("%s: virtual files not supported", __FUNCTION__);

	/* Regular file. */
	host_fd = open(full_path, flags, mode);
	if (host_fd == -1)
		return -errno;

	/* File opened, create a new file descriptor. */
	desc = x86_file_desc_table_entry_new(ctx->file_desc_table,
		file_desc_regular, host_fd, full_path, flags);
	x86_sys_debug("    file descriptor opened: guest_fd=%d, host_fd=%d\n",
		desc->guest_fd, desc->host_fd);

	/* Return guest descriptor index */
	return desc->guest_fd;
}




/*
 * System call 'set_robust_list' (code 311)
 */

static int x86_sys_set_robust_list_impl(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	unsigned int head;
	int len;

	/* Arguments */
	head = regs->ebx;
	len = regs->ecx;
	x86_sys_debug("  head=0x%x, len=%d\n", head, len);

	/* Support */
	if (len != 12)
		fatal("%s: not supported for len != 12\n%s",
			__FUNCTION__, err_x86_sys_note);

	/* Set robust list */
	ctx->robust_list_head = head;
	return 0;
}




/*
 * Not implemented system calls
 */

#define SYS_NOT_IMPL(NAME) \
	static int x86_sys_##NAME##_impl(X86Context *ctx) \
	{ \
		X86Emu *emu = ctx->emu; \
		struct x86_regs_t *regs = ctx->regs; \
		fatal("%s: system call not implemented (code %d, inst %lld, pid %d).\n%s", \
			__FUNCTION__, regs->eax, asEmu(emu)->instructions, ctx->pid, \
			err_x86_sys_note); \
		return 0; \
	}

SYS_NOT_IMPL(restart_syscall)
SYS_NOT_IMPL(fork)
SYS_NOT_IMPL(creat)
SYS_NOT_IMPL(link)
SYS_NOT_IMPL(mknod)
SYS_NOT_IMPL(lchown16)
SYS_NOT_IMPL(ni_syscall_17)
SYS_NOT_IMPL(stat)
SYS_NOT_IMPL(mount)
SYS_NOT_IMPL(oldumount)
SYS_NOT_IMPL(setuid16)
SYS_NOT_IMPL(getuid16)
SYS_NOT_IMPL(stime)
SYS_NOT_IMPL(ptrace)
SYS_NOT_IMPL(alarm)
SYS_NOT_IMPL(fstat)
SYS_NOT_IMPL(pause)
SYS_NOT_IMPL(ni_syscall_31)
SYS_NOT_IMPL(ni_syscall_32)
SYS_NOT_IMPL(nice)
SYS_NOT_IMPL(ni_syscall_35)
SYS_NOT_IMPL(sync)
SYS_NOT_IMPL(rmdir)
SYS_NOT_IMPL(ni_syscall_44)
SYS_NOT_IMPL(setgid16)
SYS_NOT_IMPL(getgid16)
SYS_NOT_IMPL(signal)
SYS_NOT_IMPL(geteuid16)
SYS_NOT_IMPL(getegid16)
SYS_NOT_IMPL(acct)
SYS_NOT_IMPL(umount)
SYS_NOT_IMPL(ni_syscall_53)
SYS_NOT_IMPL(fcntl)
SYS_NOT_IMPL(ni_syscall_56)
SYS_NOT_IMPL(setpgid)
SYS_NOT_IMPL(ni_syscall_58)
SYS_NOT_IMPL(olduname)
SYS_NOT_IMPL(umask)
SYS_NOT_IMPL(chroot)
SYS_NOT_IMPL(ustat)
SYS_NOT_IMPL(dup2)
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
SYS_NOT_IMPL(old_getrlimit)
SYS_NOT_IMPL(settimeofday)
SYS_NOT_IMPL(getgroups16)
SYS_NOT_IMPL(setgroups16)
SYS_NOT_IMPL(oldselect)
SYS_NOT_IMPL(symlink)
SYS_NOT_IMPL(lstat)
SYS_NOT_IMPL(uselib)
SYS_NOT_IMPL(swapon)
SYS_NOT_IMPL(reboot)
SYS_NOT_IMPL(readdir)
SYS_NOT_IMPL(truncate)
SYS_NOT_IMPL(ftruncate)
SYS_NOT_IMPL(fchown16)
SYS_NOT_IMPL(getpriority)
SYS_NOT_IMPL(setpriority)
SYS_NOT_IMPL(ni_syscall_98)
SYS_NOT_IMPL(fstatfs)
SYS_NOT_IMPL(ioperm)
SYS_NOT_IMPL(syslog)
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
SYS_NOT_IMPL(setdomainname)
SYS_NOT_IMPL(modify_ldt)
SYS_NOT_IMPL(adjtimex)
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
SYS_NOT_IMPL(flock)
SYS_NOT_IMPL(readv)
SYS_NOT_IMPL(getsid)
SYS_NOT_IMPL(fdatasync)
SYS_NOT_IMPL(mlock)
SYS_NOT_IMPL(munlock)
SYS_NOT_IMPL(mlockall)
SYS_NOT_IMPL(munlockall)
SYS_NOT_IMPL(sched_setscheduler)
SYS_NOT_IMPL(sched_yield)
SYS_NOT_IMPL(sched_rr_get_interval)
SYS_NOT_IMPL(setresuid16)
SYS_NOT_IMPL(getresuid16)
SYS_NOT_IMPL(vm86)
SYS_NOT_IMPL(ni_syscall_167)
SYS_NOT_IMPL(nfsservctl)
SYS_NOT_IMPL(setresgid16)
SYS_NOT_IMPL(getresgid16)
SYS_NOT_IMPL(prctl)
SYS_NOT_IMPL(rt_sigreturn)
SYS_NOT_IMPL(rt_sigpending)
SYS_NOT_IMPL(rt_sigtimedwait)
SYS_NOT_IMPL(rt_sigqueueinfo)
SYS_NOT_IMPL(pread64)
SYS_NOT_IMPL(pwrite64)
SYS_NOT_IMPL(chown16)
SYS_NOT_IMPL(capget)
SYS_NOT_IMPL(capset)
SYS_NOT_IMPL(sigaltstack)
SYS_NOT_IMPL(sendfile)
SYS_NOT_IMPL(ni_syscall_188)
SYS_NOT_IMPL(ni_syscall_189)
SYS_NOT_IMPL(vfork)
SYS_NOT_IMPL(truncate64)
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
SYS_NOT_IMPL(setuid)
SYS_NOT_IMPL(setgid)
SYS_NOT_IMPL(setfsuid)
SYS_NOT_IMPL(setfsgid)
SYS_NOT_IMPL(pivot_root)
SYS_NOT_IMPL(mincore)
SYS_NOT_IMPL(ni_syscall_222)
SYS_NOT_IMPL(ni_syscall_223)
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
SYS_NOT_IMPL(get_thread_area)
SYS_NOT_IMPL(io_setup)
SYS_NOT_IMPL(io_destroy)
SYS_NOT_IMPL(io_getevents)
SYS_NOT_IMPL(io_submit)
SYS_NOT_IMPL(io_cancel)
SYS_NOT_IMPL(ni_syscall_251)
SYS_NOT_IMPL(lookup_dcookie)
SYS_NOT_IMPL(epoll_create)
SYS_NOT_IMPL(epoll_ctl)
SYS_NOT_IMPL(epoll_wait)
SYS_NOT_IMPL(remap_file_pages)
SYS_NOT_IMPL(timer_create)
SYS_NOT_IMPL(timer_settime)
SYS_NOT_IMPL(timer_gettime)
SYS_NOT_IMPL(timer_getoverrun)
SYS_NOT_IMPL(timer_delete)
SYS_NOT_IMPL(clock_settime)
SYS_NOT_IMPL(clock_nanosleep)
SYS_NOT_IMPL(fstatfs64)
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
