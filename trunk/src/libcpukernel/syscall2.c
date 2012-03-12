/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <time.h>
#include <utime.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/times.h>

#include <cpukernel.h>
#include <mhandle.h>



static char *err_sys_note =
	"\tThe system calls performed by the executed application are intercepted by\n"
	"\tMulti2Sim and emulated in file 'syscall.c'. The most common system calls are\n"
	"\tcurrently supported, but your application might perform specific unsupported\n"
	"\tsystem calls or combinations of parameters. To request support for a given\n"
	"\tsystem call, please email 'development@multi2sim.org'.\n";



/*
 * System call error codes
 */

struct string_map_t sys_error_code_map =
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




/*
 * System call 'exit' (code 1)
 */

int sys_exit_impl(void)
{
	int status;

	/* Arguments */
	status = isa_regs->ebx;
	syscall_debug("  status=0x%x\n", status);

	/* Finish context */
	ctx_finish(isa_ctx, status);
	return 0;
}




/*
 * System call 'close' (code 2)
 */

int sys_close_impl(void)
{
	int guest_fd;
	int host_fd;
	struct file_desc_t *fd;

	/* Arguments */
	guest_fd = isa_regs->ebx;
	syscall_debug("  guest_fd=%d\n", guest_fd);
	host_fd = file_desc_table_get_host_fd(isa_ctx->file_desc_table, guest_fd);
	syscall_debug("  host_fd=%d\n", host_fd);

	/* Get file descriptor table entry. */
	fd = file_desc_table_entry_get(isa_ctx->file_desc_table, guest_fd);
	if (!fd)
		return -EBADF;

	/* Close host file descriptor only if it is valid and not stdin/stdout/stderr. */
	if (host_fd > 2)
		close(host_fd);

	/* Free guest file descriptor. This will delete the host file if it's a virtual file. */
	if (fd->kind == file_desc_virtual)
		syscall_debug("    host file '%s': temporary file deleted\n", fd->path);
	file_desc_table_entry_free(isa_ctx->file_desc_table, fd->guest_fd);

	/* Success */
	return 0;
}



/*
 * System call 'read' (code 3)
 */

int sys_read_impl(void)
{
	unsigned int buf_ptr;
	unsigned int count;

	int guest_fd;
	int host_fd;
	int err;

	void *buf;

	struct file_desc_t *fd;
	struct pollfd fds;

	/* Arguments */
	guest_fd = isa_regs->ebx;
	buf_ptr = isa_regs->ecx;
	count = isa_regs->edx;
	syscall_debug("  guest_fd=%d, buf_ptr=0x%x, count=0x%x\n",
		guest_fd, buf_ptr, count);

	/* Get file descriptor */
	fd = file_desc_table_entry_get(isa_ctx->file_desc_table, guest_fd);
	if (!fd)
		return -EBADF;
	host_fd = fd->host_fd;
	syscall_debug("  host_fd=%d\n", host_fd);

	/* Allocate buffer */
	buf = calloc(1, count);
	if (!buf)
		fatal("%s: out of memory", __FUNCTION__);

	/* Poll the file descriptor to check if read is blocking */
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
			mem_write(isa_mem, buf_ptr, err, buf);
			syscall_debug_string("  buf", buf, err, 1);
		}

		/* Return number of read bytes */
		free(buf);
		return err;
	}

	/* Blocking read - suspend thread */
	syscall_debug("  blocking read - process suspended\n");
	isa_ctx->wakeup_fd = guest_fd;
	isa_ctx->wakeup_events = 1;  /* POLLIN */
	ctx_set_status(isa_ctx, ctx_suspended | ctx_read);
	ke_process_events_schedule();

	/* Free allocated buffer. Return value doesn't matter,
	 * it will be overwritten when context wakes up from blocking call. */
	free(buf);
	return 0;
}




/*
 * System call 'write' (code 4)
 */

int sys_write_impl(void)
{
	unsigned int buf_ptr;
	unsigned int count;

	int guest_fd;
	int host_fd;
	int err;

	struct file_desc_t *desc;
	void *buf;

	struct pollfd fds;

	/* Arguments */
	guest_fd = isa_regs->ebx;
	buf_ptr = isa_regs->ecx;
	count = isa_regs->edx;
	syscall_debug("  guest_fd=%d, buf_ptr=0x%x, count=0x%x\n",
		guest_fd, buf_ptr, count);

	/* Get file descriptor */
	desc = file_desc_table_entry_get(isa_ctx->file_desc_table, guest_fd);
	if (!desc)
		return -EBADF;
	host_fd = desc->host_fd;
	syscall_debug("  host_fd=%d\n", host_fd);

	/* Allocate buffer */
	buf = calloc(1, count);
	if (!buf)
		fatal("%s: out of memory", __FUNCTION__);

	/* Read buffer from memory */
	mem_read(isa_mem, buf_ptr, count, buf);
	syscall_debug_string("  buf", buf, count, 0);

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
	syscall_debug("  blocking write - process suspended\n");
	isa_ctx->wakeup_fd = guest_fd;
	ctx_set_status(isa_ctx, ctx_suspended | ctx_write);
	ke_process_events_schedule();

	/* Return value doesn't matter here. It will be overwritten when the
	 * context wakes up after blocking call. */
	free(buf);
	return 0;
}




/*
 * System call 'open' (code 5)
 */

static struct string_map_t sys_open_flags_map =
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

int sys_open_impl(void)
{
	unsigned int file_name_ptr;

	int flags;
	int mode;
	int length;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];
	char temp_path[MAX_PATH_SIZE];
	char flags_str[MAX_STRING_SIZE];

	int host_fd;
	struct file_desc_t *desc;

	/* Arguments */
	file_name_ptr = isa_regs->ebx;
	flags = isa_regs->ecx;
	mode = isa_regs->edx;
	length = mem_read_string(isa_mem, file_name_ptr, sizeof file_name, file_name);
	if (length >= MAX_PATH_SIZE)
		fatal("syscall open: maximum path length exceeded");
	ld_get_full_path(isa_ctx, file_name, full_path, sizeof full_path);
	syscall_debug("  filename='%s' flags=0x%x, mode=0x%x\n",
		file_name, flags, mode);
	syscall_debug("  fullpath='%s'\n", full_path);
	map_flags(&sys_open_flags_map, flags, flags_str, sizeof flags_str);
	syscall_debug("  flags=%s\n", flags_str);

	/* Intercept attempt to access OpenCL library and redirect to 'm2s-opencl.so' */
	gk_libopencl_redirect(full_path, sizeof full_path);

	/* Virtual files */
	if (!strncmp(full_path, "/proc/", 6))
	{
		/* File /proc/self/maps */
		if (!strcmp(full_path, "/proc/self/maps"))
		{
			/* Create temporary file and open it. */
			ctx_gen_proc_self_maps(isa_ctx, temp_path);
			host_fd = open(temp_path, flags, mode);
			assert(host_fd > 0);

			/* Add file descriptor table entry. */
			desc = file_desc_table_entry_new(isa_ctx->file_desc_table, file_desc_virtual, host_fd, temp_path, flags);
			syscall_debug("    host file '%s' opened: guest_fd=%d, host_fd=%d\n",
				temp_path, desc->guest_fd, desc->host_fd);
			return desc->guest_fd;
		}

		/* Unhandled virtual file. Let the application read the contents of the host
		 * version of the file as if it was a regular file. */
		syscall_debug("    warning: unhandled virtual file\n");
	}

	/* Regular file. */
	host_fd = open(full_path, flags, mode);
	if (host_fd == -1)
		return -errno;

	/* File opened, create a new file descriptor. */
	desc = file_desc_table_entry_new(isa_ctx->file_desc_table,
		file_desc_regular, host_fd, full_path, flags);
	syscall_debug("    file descriptor opened: guest_fd=%d, host_fd=%d\n",
		desc->guest_fd, desc->host_fd);

	/* Return guest descriptor index */
	return desc->guest_fd;
}




/*
 * System call 'waitpid' (code 7)
 */

static struct string_map_t sys_waitpid_options_map =
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

int sys_waitpid_impl()
{
	int pid;
	int options;
	unsigned int status_ptr;

	char options_str[MAX_STRING_SIZE];

	struct ctx_t *child;

	/* Arguments */
	pid = isa_regs->ebx;
	status_ptr = isa_regs->ecx;
	options = isa_regs->edx;
	syscall_debug("  pid=%d, pstatus=0x%x, options=0x%x\n",
		pid, status_ptr, options);
	map_flags(&sys_waitpid_options_map, options, options_str, sizeof options_str);
	syscall_debug("  options=%s\n", options_str);

	/* Supported values for 'pid' */
	if (pid != -1 && pid <= 0)
		fatal("%s: only supported for pid=-1 or pid > 0.\n%s",
			__FUNCTION__, err_sys_note);

	/* Look for a zombie child. */
	child = ctx_get_zombie(isa_ctx, pid);

	/* If there is no child and the flag WNOHANG was not specified,
	 * we get suspended until the specified child finishes. */
	if (!child && !(options & 0x1))
	{
		isa_ctx->wakeup_pid = pid;
		ctx_set_status(isa_ctx, ctx_suspended | ctx_waitpid);
		return 0;
	}

	/* Context is not suspended. WNOHANG was specified, or some child
	 * was found in the zombie list. */
	if (child)
	{
		if (status_ptr)
			mem_write(isa_mem, status_ptr, 4, &child->exit_code);
		ctx_set_status(child, ctx_finished);
		return child->pid;
	}

	/* Return */
	return 0;
}




/*
 * System call 'unlink' (code 10)
 */

int sys_unlink_impl(void)
{
	unsigned int file_name_ptr;

	int length;
	int err;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];

	/* Arguments */
	file_name_ptr = isa_regs->ebx;
	length = mem_read_string(isa_mem, file_name_ptr, sizeof file_name, file_name);
	if (length >= MAX_PATH_SIZE)
		fatal("%s: buffer too small", __FUNCTION__);
	ld_get_full_path(isa_ctx, file_name, full_path, sizeof full_path);
	syscall_debug("  file_name_ptr=0x%x\n", file_name_ptr);
	syscall_debug("  file_name=%s, full_path=%s\n", file_name, full_path);

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

int sys_execve_impl(void)
{
	unsigned int name_ptr;
	unsigned int argv;
	unsigned int envp;
	unsigned int regs;

	char name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];
	int length;

	struct list_t *arg_list;
	char arg_str[MAX_STRING_SIZE];
	char *arg;

	char env[MAX_LONG_STRING_SIZE];
	int i;

	/* Arguments */
	name_ptr = isa_regs->ebx;
	argv = isa_regs->ecx;
	envp = isa_regs->edx;
	regs = isa_regs->esi;
	syscall_debug("  name_ptr=0x%x, argv=0x%x, envp=0x%x, regs=0x%x\n",
		name_ptr, argv, envp, regs);

	/* Get command name */
	length = mem_read_string(isa_mem, name_ptr, sizeof name, name);
	if (length >= sizeof name)
		fatal("%s: buffer too small", __FUNCTION__);
	ld_get_full_path(isa_ctx, name, full_path, sizeof full_path);
	syscall_debug("  name='%s', full_path='%s'\n", name, full_path);

	/* Arguments */
	arg_list = list_create();
	for (;;)
	{
		unsigned int arg_ptr;

		/* Argument pointer */
		mem_read(isa_mem, argv + arg_list->count * 4, 4, &arg_ptr);
		if (!arg_ptr)
			break;

		/* Argument */
		length = mem_read_string(isa_mem, arg_ptr, sizeof arg_str, arg_str);
		if (length >= sizeof arg_str)
			fatal("%s: buffer too small", __FUNCTION__);

		/* Duplicate */
		arg = strdup(arg_str);
		if (!arg)
			fatal("%s: out of memory", __FUNCTION__);

		/* Add to argument list */
		list_add(arg_list, arg);
		syscall_debug("    argv[%d]='%s'\n", arg_list->count, arg);
	}

	/* Environment variables */
	syscall_debug("\n");
	for (i = 0; ; i++)
	{
		unsigned int env_ptr;

		/* Variable pointer */
		mem_read(isa_mem, envp + i * 4, 4, &env_ptr);
		if (!env_ptr)
			break;

		/* Variable */
		length = mem_read_string(isa_mem, env_ptr, sizeof env, env);
		if (length >= sizeof env)
			fatal("%s: buffer too small", __FUNCTION__);

		/* Debug */
		syscall_debug("    envp[%d]='%s'\n", i, env);
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
		ctx_finish(isa_ctx, exit_code);

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
	fatal("%s: not implemented.\n%s", __FUNCTION__, err_sys_note);
	return 0;
}




/*
 * System call 'time' (code 13)
 */

int sys_time_impl(void)
{

	unsigned int time_ptr;
	int t;

	/* Arguments */
	time_ptr = isa_regs->ebx;
	syscall_debug("  ptime=0x%x\n", time_ptr);

	/* Host call */
	t = time(NULL);
	if (time_ptr)
		mem_write(isa_mem, time_ptr, 4, &t);

	/* Return */
	return t;
}




/*
 * System call 'chmod' (code 15)
 */

int sys_chmod_impl(void)
{
	unsigned int file_name_ptr;
	unsigned int mode;

	int len;
	int err;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];

	/* Arguments */
	file_name_ptr = isa_regs->ebx;
	mode = isa_regs->ecx;
	len = mem_read_string(isa_mem, file_name_ptr, sizeof file_name, file_name);
	if (len >= sizeof file_name)
		fatal("%s: buffer too small", __FUNCTION__);
	ld_get_full_path(isa_ctx, file_name, full_path, sizeof full_path);
	syscall_debug("  file_name_ptr=0x%x, mode=0x%x\n", file_name_ptr, mode);
	syscall_debug("  file_name='%s', full_path='%s'\n", file_name, full_path);

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

int sys_lseek_impl(void)
{
	unsigned int offset;

	int fd;
	int origin;
	int host_fd;
	int err;

	/* Arguments */
	fd = isa_regs->ebx;
	offset = isa_regs->ecx;
	origin = isa_regs->edx;
	host_fd = file_desc_table_get_host_fd(isa_ctx->file_desc_table, fd);
	syscall_debug("  fd=%d, offset=0x%x, origin=0x%x\n",
		fd, offset, origin);
	syscall_debug("  host_fd=%d\n", host_fd);

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

int sys_getpid_impl(void)
{
	return isa_ctx->pid;
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

int sys_utime_impl(void)
{
	unsigned int file_name_ptr;
	unsigned int utimbuf_ptr;

	struct utimbuf utimbuf;
	struct sim_utimbuf sim_utimbuf;

	int len;
	int err;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];

	/* Arguments */
	file_name_ptr = isa_regs->ebx;
	utimbuf_ptr = isa_regs->ecx;
	syscall_debug("  file_name_ptr=0x%x, utimbuf_ptr=0x%x\n",
		file_name_ptr, utimbuf_ptr);

	/* Read file name */
	len = mem_read_string(isa_mem, file_name_ptr, sizeof file_name, file_name);
	if (len >= MAX_PATH_SIZE)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get full path */
	ld_get_full_path(isa_ctx, file_name, full_path, sizeof full_path);
	syscall_debug("  file_name='%s', full_path='%s'\n", file_name, full_path);

	/* Read time buffer */
	mem_read(isa_mem, utimbuf_ptr, sizeof(struct sim_utimbuf), &sim_utimbuf);
	sys_utime_guest_to_host(&utimbuf, &sim_utimbuf);
	syscall_debug("  utimbuf.actime = %u, utimbuf.modtime = %u\n",
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

static struct string_map_t sys_access_mode_map =
{
	3, {
		{ "X_OK",  1 },
		{ "W_OK",  2 },
		{ "R_OK",  4 }
	}
};

int sys_access_impl(void)
{
	unsigned int file_name_ptr;

	int mode;
	int len;
	int err;

	char file_name[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];
	char mode_str[MAX_STRING_SIZE];

	/* Arguments */
	file_name_ptr = isa_regs->ebx;
	mode = isa_regs->ecx;

	/* Read file name */
	len = mem_read_string(isa_mem, file_name_ptr, sizeof file_name, file_name);
	if (len >= sizeof file_name)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get full path */
	ld_get_full_path(isa_ctx, file_name, full_path, sizeof full_path);

	/* Debug */
	map_flags(&sys_access_mode_map, mode, mode_str, sizeof mode_str);
	syscall_debug("  file_name='%s', mode=0x%x\n", file_name, mode);
	syscall_debug("  full_path='%s'\n", full_path);
	syscall_debug("  mode=%s\n", mode_str);

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

int sys_kill_impl(void)
{
	int pid;
	int sig;

	struct ctx_t *ctx;

	/* Arguments */
	pid = isa_regs->ebx;
	sig = isa_regs->ecx;
	syscall_debug("  pid=%d, sig=%d (%s)\n", pid,
		sig, sim_signal_name(sig));

	/* Find context. We assume program correctness, so fatal if context is
	 * not found, rather than return error code. */
	ctx = ctx_get(pid);
	if (!ctx)
		fatal("%s: invalid pid %d", __FUNCTION__, pid);

	/* Send signal */
	sim_sigset_add(&ctx->signal_mask_table->pending, sig);
	ctx_host_thread_suspend_cancel(ctx);
	ke_process_events_schedule();
	ke_process_events();

	/* Success */
	return 0;
}




/*
 * System call 'rename' (code 38)
 */

int sys_rename_impl(void)
{
	unsigned int old_path_ptr;
	unsigned int new_path_ptr;

	char old_path[MAX_PATH_SIZE];
	char new_path[MAX_PATH_SIZE];
	char old_full_path[MAX_PATH_SIZE];
	char new_full_path[MAX_PATH_SIZE];

	int len;
	int err;

	/* Arguments */
	old_path_ptr = isa_regs->ebx;
	new_path_ptr = isa_regs->ecx;
	syscall_debug("  old_path_ptr=0x%x, new_path_ptr=0x%x\n",
		old_path_ptr, new_path_ptr);

	/* Get old path */
	len = mem_read_string(isa_mem, old_path_ptr, sizeof old_path, old_path);
	if (len >= sizeof old_path)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get new path */
	len = mem_read_string(isa_mem, new_path_ptr, sizeof new_path, new_path);
	if (len >= sizeof new_path)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Get full paths */
	ld_get_full_path(isa_ctx, old_path, old_full_path, sizeof old_full_path);
	ld_get_full_path(isa_ctx, new_path, new_full_path, sizeof new_full_path);
	syscall_debug("  old_path='%s', new_path='%s'\n", old_path, new_path);
	syscall_debug("  old_full_path='%s', new_full_path='%s'\n", old_full_path, new_full_path);

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

int sys_mkdir_impl(void)
{
	unsigned int path_ptr;

	int mode;
	int len;
	int err;

	char path[MAX_PATH_SIZE];
	char full_path[MAX_PATH_SIZE];

	/* Arguments */
	path_ptr = isa_regs->ebx;
	mode = isa_regs->ecx;
	syscall_debug("  path_ptr=0x%x, mode=0x%x\n", path_ptr, mode);

	/* Read path */
	len = mem_read_string(isa_mem, path_ptr, sizeof path, path);
	if (len >= sizeof path)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Read full path */
	ld_get_full_path(isa_ctx, path, full_path, MAX_PATH_SIZE);
	syscall_debug("  path='%s', full_path='%s'\n", path, full_path);

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

int sys_dup_impl(void)
{
	int guest_fd;
	int dup_guest_fd;
	int host_fd;
	int dup_host_fd;

	struct file_desc_t *desc;
	struct file_desc_t *dup_desc;

	/* Arguments */
	guest_fd = isa_regs->ebx;
	syscall_debug("  guest_fd=%d\n", guest_fd);

	/* Check that file descriptor is valid. */
	desc = file_desc_table_entry_get(isa_ctx->file_desc_table, guest_fd);
	if (!desc)
		return -EBADF;
	host_fd = desc->host_fd;
	syscall_debug("  host_fd=%d\n", host_fd);

	/* Duplicate host file descriptor. */
	dup_host_fd = dup(host_fd);
	if (dup_host_fd == -1)
		return -errno;

	/* Create a new entry in the file descriptor table. */
	dup_desc = file_desc_table_entry_new(isa_ctx->file_desc_table,
		file_desc_regular, dup_host_fd, desc->path, desc->flags);
	dup_guest_fd = dup_desc->guest_fd;

	/* Return new file descriptor. */
	return dup_guest_fd;
}




/*
 * System call 'pipe' (code 42)
 */

int sys_pipe_impl(void)
{
	unsigned int fd_ptr;

	struct file_desc_t *read_desc;
	struct file_desc_t *write_desc;

	int guest_read_fd;
	int guest_write_fd;

	int host_fd[2];
	int err;

	/* Arguments */
	fd_ptr = isa_regs->ebx;
	syscall_debug("  fd_ptr=0x%x\n", fd_ptr);

	/* Create host pipe */
	err = pipe(host_fd);
	if (err == -1)
		fatal("%s: cannot create pipe", __FUNCTION__);
	syscall_debug("  host pipe created: fd={%d, %d}\n",
		host_fd[0], host_fd[1]);

	/* Create guest pipe */
	read_desc = file_desc_table_entry_new(isa_ctx->file_desc_table,
		file_desc_pipe, host_fd[0], "", O_RDONLY);
	write_desc = file_desc_table_entry_new(isa_ctx->file_desc_table,
		file_desc_pipe, host_fd[1], "", O_WRONLY);
	syscall_debug("  guest pipe created: fd={%d, %d}\n",
		read_desc->guest_fd, write_desc->guest_fd);
	guest_read_fd = read_desc->guest_fd;
	guest_write_fd = write_desc->guest_fd;

	/* Return file descriptors. */
	mem_write(isa_mem, fd_ptr, 4, &guest_read_fd);
	mem_write(isa_mem, fd_ptr + 4, 4, &guest_write_fd);
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

int sys_times_impl(void)
{
	unsigned int tms_ptr;

	struct tms tms;
	struct sim_tms sim_tms;

	int err;

	/* Arguments */
	tms_ptr = isa_regs->ebx;
	syscall_debug("  tms_ptr=0x%x\n", tms_ptr);

	/* Host call */
	err = times(&tms);

	/* Write result in memory */
	if (tms_ptr)
	{
		sys_times_host_to_guest(&sim_tms, &tms);
		mem_write(isa_mem, tms_ptr, sizeof(sim_tms), &sim_tms);
	}

	/* Return */
	return err;
}




/*
 * System call 'brk' (code 45)
 */

int sys_brk_impl(void)
{
	unsigned int old_heap_break;
	unsigned int new_heap_break;
	unsigned int size;

	unsigned int old_heap_break_aligned;
	unsigned int new_heap_break_aligned;

	/* Arguments */
	new_heap_break = isa_regs->ebx;
	old_heap_break = isa_mem->heap_break;
	syscall_debug("  newbrk=0x%x (previous brk was 0x%x)\n",
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
			if (mem_map_space(isa_mem, old_heap_break_aligned, size) != old_heap_break_aligned)
				fatal("%s: out of memory", __FUNCTION__);
			mem_map(isa_mem, old_heap_break_aligned, size,
				mem_access_read | mem_access_write);
		}
		isa_mem->heap_break = new_heap_break;
		syscall_debug("  heap grows %u bytes\n", new_heap_break - old_heap_break);
		return new_heap_break;
	}

	/* Always allow to shrink the heap. */
	if (new_heap_break < old_heap_break)
	{
		size = old_heap_break_aligned - new_heap_break_aligned;
		if (size)
			mem_unmap(isa_mem, new_heap_break_aligned, size);
		isa_mem->heap_break = new_heap_break;
		syscall_debug("  heap shrinks %u bytes\n", old_heap_break - new_heap_break);
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

int sys_ioctl_impl(void)
{
	unsigned int cmd;
	unsigned int arg;

	int guest_fd;
	int err;

	struct file_desc_t *desc;

	/* Arguments */
	guest_fd = isa_regs->ebx;
	cmd = isa_regs->ecx;
	arg = isa_regs->edx;
	syscall_debug("  guest_fd=%d, cmd=0x%x, arg=0x%x\n",
		guest_fd, cmd, arg);

	/* File descriptor */
	desc = file_desc_table_entry_get(isa_ctx->file_desc_table, guest_fd);
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
		mem_read(isa_mem, arg, sizeof buf, buf);
		err = ioctl(desc->host_fd, cmd, &buf);
		if (err == -1)
			return -errno;

		/* Return in memory */
		mem_write(isa_mem, arg, sizeof buf, buf);
		return err;
	}
	else
	{
		fatal("%s: not implement for cmd = 0x%x.\n%s",
			__FUNCTION__, cmd, err_sys_note);
	}

	/* Return */
	return 0;
}




/*
 * System call 'fcntl64' (code 221)
 */

static struct string_map_t sys_fcntl_cmp_map =
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

int sys_fcntl64_impl(void)
{
	int guest_fd;
	int cmd;
	int err;

	unsigned int arg;

	char *cmd_name;
	char flags_str[MAX_STRING_SIZE];

	struct file_desc_t *desc;

	/* Arguments */
	guest_fd = isa_regs->ebx;
	cmd = isa_regs->ecx;
	arg = isa_regs->edx;
	syscall_debug("  guest_fd=%d, cmd=%d, arg=0x%x\n",
		guest_fd, cmd, arg);
	cmd_name = map_value(&sys_fcntl_cmp_map, cmd);
	syscall_debug("    cmd=%s\n", cmd_name);

	/* Get file descriptor table entry */
	desc = file_desc_table_entry_get(isa_ctx->file_desc_table, guest_fd);
	if (!desc)
		return -EBADF;
	if (desc->host_fd < 0)
		fatal("%s: not supported for this type of file", __FUNCTION__);
	syscall_debug("    host_fd=%d\n", desc->host_fd);

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
			map_flags(&sys_open_flags_map, err, flags_str, MAX_STRING_SIZE);
			syscall_debug("    ret=%s\n", flags_str);
		}
		break;

	/* F_SETFL */
	case 4:
		map_flags(&sys_open_flags_map, arg, flags_str, MAX_STRING_SIZE);
		syscall_debug("    arg=%s\n", flags_str);
		desc->flags = arg;

		err = fcntl(desc->host_fd, F_SETFL, arg);
		if (err == -1)
			err = -errno;
		break;

	default:

		err = 0;
		fatal("%s: command %s not implemented.\n%s",
			__FUNCTION__, cmd_name, err_sys_note);
	}

	/* Return */
	return err;
}
