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

#include <cpukernel.h>


/*
 * System call error codes
 */

#define	SIM_EPERM	 1
#define	SIM_ENOENT	 2
#define	SIM_ESRCH	 3
#define	SIM_EINTR	 4
#define	SIM_EIO		 5
#define	SIM_ENXIO	 6
#define	SIM_E2BIG	 7
#define	SIM_ENOEXEC	 8
#define	SIM_EBADF	 9
#define	SIM_ECHILD	10
#define	SIM_EAGAIN	11
#define	SIM_ENOMEM	12
#define	SIM_EACCES	13
#define	SIM_EFAULT	14
#define	SIM_ENOTBLK	15
#define	SIM_EBUSY	16
#define	SIM_EEXIST	17
#define	SIM_EXDEV	18
#define	SIM_ENODEV	19
#define	SIM_ENOTDIR	20
#define	SIM_EISDIR	21
#define	SIM_EINVAL	22
#define	SIM_ENFILE	23
#define	SIM_EMFILE	24
#define	SIM_ENOTTY	25
#define	SIM_ETXTBSY	26
#define	SIM_EFBIG	27
#define	SIM_ENOSPC	28
#define	SIM_ESPIPE	29
#define	SIM_EROFS	30
#define	SIM_EMLINK	31
#define	SIM_EPIPE	32
#define	SIM_EDOM	33
#define	SIM_ERANGE	34

struct string_map_t sys_error_code_map =
{
	34,
	{
		{ "SIM_EPERM", 1 },
		{ "SIM_ENOENT", 2 },
		{ "SIM_ESRCH", 3 },
		{ "SIM_EINTR", 4 },
		{ "SIM_EIO", 5 },
		{ "SIM_ENXIO", 6 },
		{ "SIM_E2BIG", 7 },
		{ "SIM_ENOEXEC", 8 },
		{ "SIM_EBADF", 9 },
		{ "SIM_ECHILD", 10 },
		{ "SIM_EAGAIN", 11 },
		{ "SIM_ENOMEM", 12 },
		{ "SIM_EACCES", 13 },
		{ "SIM_EFAULT", 14 },
		{ "SIM_ENOTBLK", 15 },
		{ "SIM_EBUSY", 16 },
		{ "SIM_EEXIST", 17 },
		{ "SIM_EXDEV", 18 },
		{ "SIM_ENODEV", 19 },
		{ "SIM_ENOTDIR", 20 },
		{ "SIM_EISDIR", 21 },
		{ "SIM_EINVAL", 22 },
		{ "SIM_ENFILE", 23 },
		{ "SIM_EMFILE", 24 },
		{ "SIM_ENOTTY", 25 },
		{ "SIM_ETXTBSY", 26 },
		{ "SIM_EFBIG", 27 },
		{ "SIM_ENOSPC", 28 },
		{ "SIM_ESPIPE", 29 },
		{ "SIM_EROFS", 30 },
		{ "SIM_EMLINK", 31 },
		{ "SIM_EPIPE", 32 },
		{ "SIM_EDOM", 33 },
		{ "SIM_ERANGE", 34 }
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
	struct fd_t *fd;

	/* Arguments */
	guest_fd = isa_regs->ebx;
	syscall_debug("  guest_fd=%d\n", guest_fd);
	host_fd = fdt_get_host_fd(isa_ctx->fdt, guest_fd);
	syscall_debug("  host_fd=%d\n", host_fd);

	/* Get file descriptor table entry. */
	fd = fdt_entry_get(isa_ctx->fdt, guest_fd);
	if (!fd)
		return -SIM_EBADF;

	/* Close host file descriptor only if it is valid and not stdin/stdout/stderr. */
	if (host_fd > 2)
		close(host_fd);

	/* Free guest file descriptor. This will delete the host file if it's a virtual file. */
	if (fd->kind == fd_kind_virtual)
		syscall_debug("    host file '%s': temporary file deleted\n", fd->path);
	fdt_entry_free(isa_ctx->fdt, fd->guest_fd);

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

	struct fd_t *fd;
	struct pollfd fds;

	/* Arguments */
	guest_fd = isa_regs->ebx;
	buf_ptr = isa_regs->ecx;
	count = isa_regs->edx;
	syscall_debug("  guest_fd=%d, buf_ptr=0x%x, count=0x%x\n",
		guest_fd, buf_ptr, count);

	/* Get file descriptor */
	fd = fdt_entry_get(isa_ctx->fdt, guest_fd);
	if (!fd)
		return -SIM_EBADF;
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

	struct fd_t *fd;
	void *buf;

	struct pollfd fds;

	/* Arguments */
	guest_fd = isa_regs->ebx;
	buf_ptr = isa_regs->ecx;
	count = isa_regs->edx;
	syscall_debug("  guest_fd=%d, buf_ptr=0x%x, count=0x%x\n",
		guest_fd, buf_ptr, count);

	/* Get file descriptor */
	fd = fdt_entry_get(isa_ctx->fdt, guest_fd);
	if (!fd)
		return -SIM_EBADF;
	host_fd = fd->host_fd;
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
