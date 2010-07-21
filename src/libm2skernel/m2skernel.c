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
	pipemgr_init();

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

	pipemgr_done();
	free(ke);
	isa_done();
	syscall_summary();
}


/* Execute one instruction from each running context. */
void ke_run(void)
{
	struct ctx_t *ctx;

	/* Kernel is stuck if all contexts are suspended and
	 * there is no timer pending. */
	if (!ke->running_list_head && !ke->event_timer_next)
		fatal("all contexts suspended");

	/* Run an instruction from every running process */
	for (ctx = ke->running_list_head; ctx; ctx = ctx->running_next)
		ctx_execute_inst(ctx);
	
	/* Free finished contexts */
	while (ke->finished_list_head)
		ctx_free(ke->finished_list_head);

	/* Check for timer events */
	if (ke->event_timer_next && ke->event_timer_next < ke_timer())
		ke_event_timer();
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


static void ke_event_timer_schedule(struct ctx_t *ctx, uint64_t now)
{
	if (!ke->event_timer_next || ke->event_timer_next > ctx->wakeup_time)
		ke->event_timer_next = ctx->wakeup_time;
}


/* Process the list of suspended contexts to check if
 * there is anyone that must be resumed */
void ke_event_timer()
{
	struct ctx_t *ctx, *next;
	uint64_t now = ke_timer();

	/* By default, no subsequent call to ke_event_timer */
	ke->event_timer_next = 0;

	/* Look at the list of suspended contexts and try to find
	 * one that needs to be woken up. */
	for (ctx = ke->suspended_list_head; ctx; ctx = next) {

		/* Save next */
		next = ctx->suspended_next;

		/* Context is suspended in 'nanosleep' system call. If
		 * it is woken up, it must update the 'rmtp' struct. */
		if (ctx_get_status(ctx, ctx_nanosleep)) {
			uint32_t rmtp;
			uint64_t zero = 0;

			if (ctx->wakeup_time > now) {
				ke_event_timer_schedule(ctx, now);
				continue;
			}
			rmtp = ctx->regs->ecx;
			if (rmtp)
				mem_write(ctx->mem, rmtp, 8, &zero);
			syscall_debug("syscall nanosleep - continue (pid %d)\n", ctx->pid);
			syscall_debug("  return=0x%x\n", ctx->regs->eax);
			ctx_clear_status(ctx, ctx_suspended | ctx_nanosleep);
		}

		/* Context suspended in 'poll' system call. It can be woken up due to
		 * timer only if wakup_time > 0. The action is to return 0 and set
		 * 'revents' to 0, since no event occured but timer expiration. */
		if (ctx_get_status(ctx, ctx_poll) && ctx->wakeup_time) {
			uint32_t prevents = ctx->regs->ebx + 6;
			uint16_t revents = 0;

			if (ctx->wakeup_time > now) {
				ke_event_timer_schedule(ctx, now);
				continue;
			}
			mem_write(ctx->mem, prevents, 2, &revents);
			syscall_debug("syscall poll - continue (pid %d)\n", ctx->pid);
			syscall_debug("  return=0x%x\n", ctx->regs->eax);
			ctx_clear_status(ctx, ctx_suspended | ctx_poll);
		}

	}
}


/* Process the list of suspended contexts to check if
 * there is anyone waiting for a read event */
void ke_event_read()
{
	struct ctx_t *ctx, *next;

	for (ctx = ke->suspended_list_head; ctx; ctx = next) {

		/* Save next */
		next = ctx->suspended_next;

		/* Context suspended in 'poll' system call with POLLOUT events.
		 * We can wake it up if the pipe is empty. */
		if (ctx_get_status(ctx, ctx_poll) &&
			(ctx->wakeup_events & 0x4 /*POLLOUT*/) &&
			!pipe_count(ctx->wakeup_fd))
		{
			uint32_t prevents = ctx->regs->ebx + 6;
			uint16_t revents = 0x4;
			mem_write(ctx->mem, prevents, 2, &revents);
			ctx->regs->eax = 1;
			syscall_debug("syscall poll - continue (pid %d)\n", ctx->pid);
			syscall_debug("  return=0x%x\n", ctx->regs->eax);
			ctx_clear_status(ctx, ctx_suspended | ctx_poll);
		}

		/* Context suspended in 'write' system call, waiting for pipe
		 * to be empty in order to dump data. */
		if (ctx_get_status(ctx, ctx_write) &&
			!pipe_count(ctx->wakeup_fd))
		{
			uint32_t pbuf, count;
			void *buf;

			pbuf = ctx->regs->ecx;
			count = ctx->regs->edx;
			buf = malloc(count);
			mem_read(ctx->mem, pbuf, count, buf);
			count = pipe_write(ctx->wakeup_fd, buf, count);
			ctx->regs->eax = count;
			free(buf);

			ke_event_write();
			syscall_debug("syscall write - continue (pid %d)\n", ctx->pid);
			syscall_debug("  return=0x%x\n", ctx->regs->eax);
			ctx_clear_status(ctx, ctx_suspended | ctx_write);
		}
	}

}


/* Process the list of suspended contexts to check if
 * there is anyone waiting for a write event */
void ke_event_write()
{
	struct ctx_t *ctx, *next;

	for (ctx = ke->suspended_list_head; ctx; ctx = next) {

		/* Save next */
		next = ctx->suspended_next;

		/* Context suspended in 'poll' system call with POLLIN events,
		 * waiting for some data to be available in the pipe. */
		if (ctx_get_status(ctx, ctx_poll) &&
			(ctx->wakeup_events & 0x1 /*POLLIN*/) &&
			pipe_count(ctx->wakeup_fd))
		{
			uint32_t prevents = ctx->regs->ebx + 6;
			uint16_t revents = 0x1;
			mem_write(ctx->mem, prevents, 2, &revents);
			ctx->regs->eax = 1;
			syscall_debug("syscall poll - continue (pid %d)\n", ctx->pid);
			syscall_debug("  return=0x%x\n", ctx->regs->eax);
			ctx_clear_status(ctx, ctx_suspended | ctx_poll);
		}

		/* Context suspended in 'read' system call,
		 * reading on a pipe. */
		if (ctx_get_status(ctx, ctx_read) &&
			pipe_count(ctx->wakeup_fd))
		{
			uint32_t pbuf, count;
			void *buf;

			pbuf = ctx->regs->ecx;
			count = ctx->regs->edx;
			buf = malloc(count);
			count = pipe_read(ctx->wakeup_fd, buf, count);
			ctx->regs->eax = count;
			mem_write(ctx->mem, pbuf, count, buf);
			free(buf);

			ke_event_read();
			syscall_debug("syscall read - continue (pid %d)\n", ctx->pid);
			syscall_debug("  return=0x%x\n", ctx->regs->eax);
			ctx_clear_status(ctx, ctx_suspended | ctx_read);
		}
	}

}


/* Process list of suspended context to check if
 * there is anyone waiting for a signal */
void ke_event_signal()
{
	struct ctx_t *ctx, *next;

	for (ctx = ke->suspended_list_head; ctx; ctx = next) {

		/* Save next */
		next = ctx->suspended_next;

		/* Context suspended in 'rt_sigsuspend'. The action on
		 * wakeup is to restore the saved signal mask. */
		if (ctx_get_status(ctx, ctx_sigsuspend) &&
			(ctx->signal_masks->pending & ~ctx->signal_masks->blocked))
		{
			ctx->signal_masks->blocked = ctx->signal_masks->backup;
			ctx->regs->eax = -4; /* EINTR */
			syscall_debug("syscall sigsuspend - continue (pid %d)\n", ctx->pid);
			syscall_debug("  return=0x%x\n", ctx->regs->eax);
			ctx_clear_status(ctx, ctx_suspended | ctx_sigsuspend);
		}

		/* Context is suspended in 'nanosleep' system call. If
		 * it is woken up, it must update the 'rmtp' struct. */
		if (ctx_get_status(ctx, ctx_nanosleep) &&
			(ctx->signal_masks->pending & ~ctx->signal_masks->blocked))
		{
			uint32_t rmtp, sec, usec;
			uint64_t diff, now;

			rmtp = ctx->regs->ecx;
			now = ke_timer();
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
		}

	}
}


/* Process list of suspended context to check if there is any that
 * is waiting for a child context to become zombie. */
void ke_event_finish()
{
	struct ctx_t *ctx, *next;

	for (ctx = ke->suspended_list_head; ctx; ctx = next) {

		/* Save next */
		next = ctx->suspended_next;

		/* Context suspended in 'waitpid'. The action on
		 * wakeup is to finish the system call actions. */
		if (ctx_get_status(ctx, ctx_waitpid))
		{
			struct ctx_t *child;
			uint32_t pstatus;

			/* Is this context really waiting for the recently
			 * finished context? */
			child = ctx_get_zombie(ctx, ctx->wakeup_pid);
			if (!child)
				continue;

			/* Continue with 'waitpid' system call */
			pstatus = ctx->regs->ecx;
			ctx->regs->eax = child->pid;
			if (pstatus)
				mem_write(ctx->mem, pstatus, 4, &child->exit_code);
			ctx_set_status(child, ctx_finished);

			syscall_debug("syscall waitpid - continue (pid %d)\n", ctx->pid);
			syscall_debug("  return=0x%x\n", ctx->regs->eax);
			ctx_clear_status(ctx, ctx_suspended | ctx_waitpid);
		}

	}
}

