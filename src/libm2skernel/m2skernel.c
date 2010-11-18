/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include <m2skernel.h>


/* Global Multi2Sim
 * Kernel Variable */
struct kernel_t *ke;


/* Initialization */

static uint64_t ke_init_time = 0;

void ke_init(void)
{
	uint32_t endian = 0x44332211;
	unsigned char *pendian = (unsigned char *) &endian;

	/* Endian check */
	if (pendian[0] != 0x11 || pendian[3] != 0x44)
		fatal("cannot run kernel on a big endian machine");
	
	isa_init();
	ke = calloc(1, sizeof(struct kernel_t));
	ke->current_pid = 1000;  /* Initial assigned pid */
	
	/* Initialize mutex for variables controlling calls to 'process_suspended()' */
	pthread_mutex_init(&ke->process_suspended_mutex, NULL);

	/* Debug categories */
	isa_inst_debug_category = debug_new_category();
	isa_call_debug_category = debug_new_category();
	elf_debug_category = debug_new_category();
	ld_debug_category = debug_new_category();
	syscall_debug_category = debug_new_category();
	ctx_debug_category = debug_new_category();

	/* Record start time */
	ke_init_time = ke_timer();
}


/* Finalization */
void ke_done(void)
{
	struct ctx_t *ctx;

	/* Finish all contexts */
	for (ctx = ke->context_list_head; ctx; ctx = ctx->context_next)
		if (!ctx_get_status(ctx, ctx_finished))
			ctx_finish(ctx, 0);

	/* Free contexts */
	while (ke->context_list_head)
		ctx_free(ke->context_list_head);

	free(ke);
	isa_done();
	syscall_summary();
}


/* Execute one instruction from each running context. */
void ke_run(void)
{
	struct ctx_t *ctx;

	/* Run an instruction from every running process */
	for (ctx = ke->running_list_head; ctx; ctx = ctx->running_next)
		ctx_execute_inst(ctx);
	
	/* Free finished contexts */
	while (ke->finished_list_head)
		ctx_free(ke->finished_list_head);
	
	/* Process list of suspended contexts */
	ke_process_suspended();

}


void ke_dump(FILE *f)
{
	struct ctx_t *ctx;
	int n = 0;
	ctx = ke->context_list_head;
	fprintf(f, "List of kernel contexts (arbitrary order):\n");
	while (ctx) {
		fprintf(f, "kernel context #%d:\n", n);
		ctx_dump(ctx, f);
		ctx = ctx->context_next;
		n++;
	}
}


#define LIST_INSERT_HEAD(name, ctx) { \
	assert(!ctx->name##_next && !ctx->name##_prev); \
	ctx->name##_next = ke->name##_list_head; \
	if (ctx->name##_next) ctx->name##_next->name##_prev = ctx; \
	ke->name##_list_head = ctx; \
	if (!ke->name##_list_tail) ke->name##_list_tail = ctx; \
	ke->name##_count++; \
	ke->name##_max = MAX(ke->name##_max, ke->name##_count); }

#define LIST_INSERT_TAIL(name, ctx) { \
	assert(!ctx->name##_next && !ctx->name##_prev); \
	ctx->name##_prev = ke->name##_list_tail; \
	if (ctx->name##_prev) ctx->name##_prev->name##_next = ctx; \
	ke->name##_list_tail = ctx; \
	if (!ke->name##_list_head) ke->name##_list_head = ctx; \
	ke->name##_count++; }

#define LIST_REMOVE(name, ctx) { \
	if (ctx == ke->name##_list_head) ke->name##_list_head = ke->name##_list_head->name##_next; \
	if (ctx == ke->name##_list_tail) ke->name##_list_tail = ke->name##_list_tail->name##_prev; \
	if (ctx->name##_prev) ctx->name##_prev->name##_next = ctx->name##_next; \
	if (ctx->name##_next) ctx->name##_next->name##_prev = ctx->name##_prev; \
	ctx->name##_prev = ctx->name##_next = NULL; \
	ke->name##_count--; }

#define LIST_MEMBER(name, ctx) \
	(ke->name##_list_head == ctx || ctx->name##_prev || ctx->name##_next)


void ke_list_insert_head(enum ke_list_enum list, struct ctx_t *ctx)
{
	assert(!ke_list_member(list, ctx));
	switch (list) {
	case ke_list_context: LIST_INSERT_HEAD(context, ctx); break;
	case ke_list_running: LIST_INSERT_HEAD(running, ctx); break;
	case ke_list_finished: LIST_INSERT_HEAD(finished, ctx); break;
	case ke_list_zombie: LIST_INSERT_HEAD(zombie, ctx); break;
	case ke_list_suspended: LIST_INSERT_HEAD(suspended, ctx); break;
	case ke_list_alloc: LIST_INSERT_HEAD(alloc, ctx); break;
	}
}


void ke_list_insert_tail(enum ke_list_enum list, struct ctx_t *ctx)
{
	assert(!ke_list_member(list, ctx));
	switch (list) {
	case ke_list_context: LIST_INSERT_TAIL(context, ctx); break;
	case ke_list_running: LIST_INSERT_TAIL(running, ctx); break;
	case ke_list_finished: LIST_INSERT_TAIL(finished, ctx); break;
	case ke_list_zombie: LIST_INSERT_TAIL(zombie, ctx); break;
	case ke_list_suspended: LIST_INSERT_TAIL(suspended, ctx); break;
	case ke_list_alloc: LIST_INSERT_TAIL(alloc, ctx); break;
	}
}


void ke_list_remove(enum ke_list_enum list, struct ctx_t *ctx)
{
	assert(ke_list_member(list, ctx));
	switch (list) {
	case ke_list_context: LIST_REMOVE(context, ctx); break;
	case ke_list_running: LIST_REMOVE(running, ctx); break;
	case ke_list_finished: LIST_REMOVE(finished, ctx); break;
	case ke_list_zombie: LIST_REMOVE(zombie, ctx); break;
	case ke_list_suspended: LIST_REMOVE(suspended, ctx); break;
	case ke_list_alloc: LIST_REMOVE(alloc, ctx); break;
	}
}


int ke_list_member(enum ke_list_enum list, struct ctx_t *ctx)
{
	switch (list) {
	case ke_list_context: return LIST_MEMBER(context, ctx);
	case ke_list_running: return LIST_MEMBER(running, ctx);
	case ke_list_finished: return LIST_MEMBER(finished, ctx);
	case ke_list_zombie: return LIST_MEMBER(zombie, ctx);
	case ke_list_suspended: return LIST_MEMBER(suspended, ctx);
	case ke_list_alloc: return LIST_MEMBER(alloc, ctx);
	}
	return 0;
}


#undef LIST_INSERT
#undef LIST_REMOVE
#undef LIST_MEMBER


/* Return a counter of microseconds. */
uint64_t ke_timer()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t) tv.tv_sec * 1000000 + tv.tv_usec - ke_init_time;
}


/* Schedule a call to 'ke_process_suspend' */
void ke_process_suspended_schedule()
{
	pthread_mutex_lock(&ke->process_suspended_mutex);
	ke->process_suspended_force = 1;
	pthread_mutex_unlock(&ke->process_suspended_mutex);
}


/* Thread function that suspends itself waiting for an event to occur, and
 * then schedules a call to 'ke_process_suspend' */
void *ke_process_suspended_thread(void *arg)
{
	struct ctx_t *ctx = (struct ctx_t *) arg;
	uint64_t now = ke_timer();

	/* Detach this thread - we don't want the parent to have to join it to release
	 * its resources. The thread termination can be observed by atomically checking
	 * the 'ctx->process_suspended_thread_active' flag. */
	assert(ctx_get_status(ctx, ctx_suspended));
	pthread_detach(pthread_self());

	/* Context suspended in 'poll' system call */
	if (ctx_get_status(ctx, ctx_poll))
	{
		struct fd_t *fd;
		struct pollfd host_fds;
		int err, timeout;
		
		/* Get file descriptor */
		fd = fdt_entry_get(ctx->fdt, ctx->wakeup_fd);
		if (!fd)
			fatal("syscall 'poll': invalid 'wakeup_fd'");

		/* Calculate timeout for host call in milliseconds from now */
		if (!ctx->wakeup_time)
			timeout = -1;
		else if (ctx->wakeup_time < now)
			timeout = 0;
		else
			timeout = (ctx->wakeup_time - now) / 1000;

		/* Perform blocking host 'poll' */
		host_fds.fd = fd->host_fd;
		host_fds.events = ((ctx->wakeup_events & 4) ? POLLOUT : 0) | ((ctx->wakeup_events & 1) ? POLLIN : 0);
		err = poll(&host_fds, 1, timeout);
		if (err < 0)
			fatal("syscall 'poll': unexpected error in host 'poll'");
	
	} else if (ctx_get_status(ctx, ctx_read)) {
		
		struct fd_t *fd;
		struct pollfd host_fds;
		int err;

		/* Get file descriptor */
		fd = fdt_entry_get(ctx->fdt, ctx->wakeup_fd);
		if (!fd)
			fatal("syscall 'read': invalid 'wakeup_fd'");

		/* Perform blocking host 'poll' */
		host_fds.fd = fd->host_fd;
		host_fds.events = POLLIN;
		err = poll(&host_fds, 1, -1);
		if (err < 0)
			fatal("syscall 'read': unexpected error in host 'poll'");
	
	} else if (ctx_get_status(ctx, ctx_write)) {
		
		struct fd_t *fd;
		struct pollfd host_fds;
		int err;

		/* Get file descriptor */
		fd = fdt_entry_get(ctx->fdt, ctx->wakeup_fd);
		if (!fd)
			fatal("syscall 'write': invalid 'wakeup_fd'");

		/* Perform blocking host 'poll' */
		host_fds.fd = fd->host_fd;
		host_fds.events = POLLOUT;
		err = poll(&host_fds, 1, -1);
		if (err < 0)
			fatal("syscall 'write': unexpected error in host 'write'");

	} else
		fatal("ke_process_suspended_thread: context status not handled");

	/* Event occurred - thread finishes */
	pthread_mutex_lock(&ke->process_suspended_mutex);
	ke->process_suspended_force = 1;
	ctx->process_suspended_thread_active = 0;
	pthread_mutex_unlock(&ke->process_suspended_mutex);
	return NULL;
}


/* Process list of suspended process and wake up those that are ready to resume,
 * only if 'process_suspended_force' or 'process_suspended_time' tell so. */
void ke_process_suspended()
{
	struct ctx_t *ctx, *next;
	uint64_t now = ke_timer();
	int do_process_suspended;

	/* Check if suspended contexts should be actually processed */
	pthread_mutex_lock(&ke->process_suspended_mutex);
	do_process_suspended = ke->process_suspended_force ||
		(ke->process_suspended_time && ke->process_suspended_time < now);
	if (!do_process_suspended) {
		pthread_mutex_unlock(&ke->process_suspended_mutex);
		return;
	}
	
	/* By default, no subsequent call to 'ke_process_suspended' is assumed */
	ke->process_suspended_force = 0;
	ke->process_suspended_time = 0;

	/* Look at the list of suspended contexts and try to find
	 * one that needs to be woken up. */
	for (ctx = ke->suspended_list_head; ctx; ctx = next) {

		/* Save next */
		next = ctx->suspended_next;

		/* Context is suspended in 'nanosleep' system call. */
		if (ctx_get_status(ctx, ctx_nanosleep))
		{
			uint32_t rmtp = ctx->regs->ecx;
			uint64_t zero = 0;
			uint32_t sec, usec;
			uint64_t diff;

			/* Timeout expired */
			if (ctx->wakeup_time <= now) {
				if (rmtp)
					mem_write(ctx->mem, rmtp, 8, &zero);
				syscall_debug("syscall nanosleep - continue (pid %d)\n", ctx->pid);
				syscall_debug("  return=0x%x\n", ctx->regs->eax);
				ctx_clear_status(ctx, ctx_suspended | ctx_nanosleep);
				continue;
			} else {
				/* Time to wake up not reached yet, schedule for later */
				if (!ke->process_suspended_time || ke->process_suspended_time > ctx->wakeup_time)
					ke->process_suspended_time = ctx->wakeup_time;
			}

			/* Wakeup signal received */
			if (ctx->signal_masks->pending & ~ctx->signal_masks->blocked) {
				if (rmtp) {
					diff = now < ctx->wakeup_time ? ctx->wakeup_time - now : 0;
					sec = diff / 1000000;
					usec = diff % 1000000;
					mem_write(ctx->mem, rmtp, 4, &sec);
					mem_write(ctx->mem, rmtp + 4, 4, &usec);
				}
				ctx->regs->eax = -4; /* EINTR */
				syscall_debug("syscall nanosleep - continue (pid %d)\n", ctx->pid);
				syscall_debug("  return=0x%x\n", ctx->regs->eax);
				ctx_clear_status(ctx, ctx_suspended | ctx_nanosleep);
				continue;
			}
		}

		/* Context suspended in 'poll' system call */
		if (ctx_get_status(ctx, ctx_poll))
		{
			uint32_t prevents = ctx->regs->ebx + 6;
			uint16_t revents = 0;
			struct fd_t *fd;
			struct pollfd host_fds;
			int err;

			/* Get file descriptor */
			fd = fdt_entry_get(ctx->fdt, ctx->wakeup_fd);
			if (!fd)
				fatal("syscall 'poll': invalid 'wakeup_fd'");

			/* If 'ke_process_suspended_thread' is still running, do nothing. */
			if (ctx->process_suspended_thread_active)
				continue;

			/* Perform host 'poll' call */
			host_fds.fd = fd->host_fd;
			host_fds.events = ((ctx->wakeup_events & 4) ? POLLOUT : 0) | ((ctx->wakeup_events & 1) ? POLLIN : 0);
			err = poll(&host_fds, 1, 0);
			if (err < 0)
				fatal("syscall 'poll': unexpected error in host 'poll'");

			/* POLLOUT event available */
			if (ctx->wakeup_events & host_fds.revents & POLLOUT) {
				revents = POLLOUT;
				mem_write(ctx->mem, prevents, 2, &revents);
				ctx->regs->eax = 1;
				syscall_debug("syscall poll - continue (pid %d) - POLLOUT occurred in file\n", ctx->pid);
				syscall_debug("  retval=%d\n", ctx->regs->eax);
				ctx_clear_status(ctx, ctx_suspended | ctx_poll);
				continue;
			}

			/* POLLIN event available */
			if (ctx->wakeup_events & host_fds.revents & POLLIN) {
				revents = POLLIN;
				mem_write(ctx->mem, prevents, 2, &revents);
				ctx->regs->eax = 1;
				syscall_debug("syscall poll - continue (pid %d) - POLLIN occurred in file\n", ctx->pid);
				syscall_debug("  retval=%d\n", ctx->regs->eax);
				ctx_clear_status(ctx, ctx_suspended | ctx_poll);
				continue;
			}

			/* Timeout expired */
			if (ctx->wakeup_time && ctx->wakeup_time < now) {
				revents = 0;
				mem_write(ctx->mem, prevents, 2, &revents);
				syscall_debug("syscall poll - continue (pid %d) - time out\n", ctx->pid);
				syscall_debug("  return=0x%x\n", ctx->regs->eax);
				ctx_clear_status(ctx, ctx_suspended | ctx_poll);
				continue;
			}

			/* No event available, launch 'ke_process_suspended_thread' again */
			ctx->process_suspended_thread_active = 1;
			if (pthread_create(&ctx->process_suspended_thread, NULL, ke_process_suspended_thread, ctx))
				fatal("syscall 'poll': could not create child thread");
			continue;
		}


		/* Context suspended in a 'write' system call  */
		if (ctx_get_status(ctx, ctx_write))
		{
			struct fd_t *fd;
			int count, err;
			uint32_t pbuf;
			void *buf;
			struct pollfd host_fds;

			/* If 'ke_process_suspended_thread' is still running, do nothing. */
			if (ctx->process_suspended_thread_active)
				continue;

			/* Get file descriptor */
			fd = fdt_entry_get(ctx->fdt, ctx->wakeup_fd);
			if (!fd)
				fatal("syscall 'write': invalid 'wakeup_fd'");

			/* Check if data is ready in file by polling it */
			host_fds.fd = fd->host_fd;
			host_fds.events = POLLOUT;
			err = poll(&host_fds, 1, 0);
			if (err < 0)
				fatal("syscall 'write': unexpected error in host 'poll'");

			/* If data is ready in the file, wake up context */
			if (host_fds.revents) {
				pbuf = ctx->regs->ecx;
				count = ctx->regs->edx;
				buf = malloc(count);
				mem_read(ctx->mem, pbuf, count, buf);

				count = write(fd->host_fd, buf, count);
				if (count < 0)
					fatal("syscall 'write': unexpected error in host 'write'");

				ctx->regs->eax = count;
				free(buf);

				syscall_debug("syscall write - continue (pid %d)\n", ctx->pid);
				syscall_debug("  return=0x%x\n", ctx->regs->eax);
				ctx_clear_status(ctx, ctx_suspended | ctx_write);
				continue;
			}

			/* Data is not ready to be written - launch 'ke_process_suspended_thread' again */
			ctx->process_suspended_thread_active = 1;
			if (pthread_create(&ctx->process_suspended_thread, NULL, ke_process_suspended_thread, ctx))
				fatal("syscall 'write': could not create child thread");
			continue;
		}

		/* Context suspended in 'read' system call */
		if (ctx_get_status(ctx, ctx_read))
		{
			struct fd_t *fd;
			uint32_t pbuf;
			int count, err;
			void *buf;
			struct pollfd host_fds;

			/* If 'ke_process_suspended_thread' is still running, do nothing. */
			if (ctx->process_suspended_thread_active)
				continue;

			/* Context received a signal */
			if (ctx->signal_masks->pending & ~ctx->signal_masks->blocked) {
				ctx->regs->eax = -EINTR;
				syscall_debug("syscall 'read' - interrupted by signal (pid %d)\n", ctx->pid);
				ctx_clear_status(ctx, ctx_suspended | ctx_read);
				continue;
			}

			/* Get file descriptor */
			fd = fdt_entry_get(ctx->fdt, ctx->wakeup_fd);
			if (!fd)
				fatal("syscall 'read': invalid 'wakeup_fd'");

			/* Check if data is ready in file by polling it */
			host_fds.fd = fd->host_fd;
			host_fds.events = POLLIN;
			err = poll(&host_fds, 1, 0);
			if (err < 0)
				fatal("syscall 'read': unexpected error in host 'poll'");

			/* If data is ready, perform host 'read' call and wake up */
			if (host_fds.revents) {
				pbuf = ctx->regs->ecx;
				count = ctx->regs->edx;
				buf = malloc(count);
				
				count = read(fd->host_fd, buf, count);
				if (count < 0)
					fatal("syscall 'read': unexpected error in host 'read'");

				ctx->regs->eax = count;
				mem_write(ctx->mem, pbuf, count, buf);
				free(buf);

				syscall_debug("syscall 'read' - continue (pid %d)\n", ctx->pid);
				syscall_debug("  return=0x%x\n", ctx->regs->eax);
				ctx_clear_status(ctx, ctx_suspended | ctx_read);
				continue;
			}

			/* Data is not ready. Launch 'ke_process_suspended_thread' again */
			ctx->process_suspended_thread_active = 1;
			if (pthread_create(&ctx->process_suspended_thread, NULL, ke_process_suspended_thread, ctx))
				fatal("syscall 'read': could not create child thread");
			continue;
		}

		/* Context suspended in 'rt_sigsuspend'.
		 * The action on wakeup is to restore the saved signal mask. */
		if (ctx_get_status(ctx, ctx_sigsuspend) &&
			(ctx->signal_masks->pending & ~ctx->signal_masks->blocked))
		{
			ctx->signal_masks->blocked = ctx->signal_masks->backup;
			ctx->regs->eax = -4; /* EINTR */
			syscall_debug("syscall sigsuspend - continue (pid %d)\n", ctx->pid);
			syscall_debug("  retval=%d\n", ctx->regs->eax);
			ctx_clear_status(ctx, ctx_suspended | ctx_sigsuspend);
			continue;
		}

		/* Context suspended in 'waitpid' */
		if (ctx_get_status(ctx, ctx_waitpid))
		{
			struct ctx_t *child;
			uint32_t pstatus;

			/* Is this context waiting for any zombie child? */
			child = ctx_get_zombie(ctx, ctx->wakeup_pid);
			if (child) {

				/* Continue with 'waitpid' system call */
				pstatus = ctx->regs->ecx;
				ctx->regs->eax = child->pid;
				if (pstatus)
					mem_write(ctx->mem, pstatus, 4, &child->exit_code);
				ctx_set_status(child, ctx_finished);

				syscall_debug("syscall waitpid - continue (pid %d)\n", ctx->pid);
				syscall_debug("  return=0x%x\n", ctx->regs->eax);
				ctx_clear_status(ctx, ctx_suspended | ctx_waitpid);
				continue;
			}
		}
	}

	/* Assume a deadlock when all contexts are suspended and there is no pending processing
	 * of the suspended contexts. */
	//if (!ke->running_list_head && !ke->process_suspended_time && !ke->process_suspended_force)
		//fatal("all contexts suspended");

	/* Processing of suspended contexts finished, unlock mutex. */
	pthread_mutex_unlock(&ke->process_suspended_mutex);
}

