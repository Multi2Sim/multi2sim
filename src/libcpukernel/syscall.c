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

#include <unistd.h>
#include <utime.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <linux/unistd.h>


int sys_debug_category;

static char *syscall_name[] = {
#define DEFSYSCALL(name,code) #name,
#include "syscall.dat"
#undef DEFSYSCALL
	""
};

enum {
#define DEFSYSCALL(name,code) syscall_code_##name = code,
#include "syscall.dat"
#undef DEFSYSCALL
	syscall_code_count
};

static uint64_t syscall_freq[syscall_code_count + 1];


/* Print a string in debug output.
 * If 'force' is set, keep printing after \0 is found. */
void sys_debug_string(char *text, char *s, int len, int force)
{
	char buf[200], *bufptr;
	int trunc = 0;
	if (!debug_status(sys_debug_category))
		return;

	memset(buf, 0, 200);
	strcpy(buf, "\"");
	bufptr = &buf[1];
	if (len > 40) {
		len = 40;
		trunc = 1;
	}
	for (;;) {
		if (!len || (!*s && !force)) {
			strcpy(bufptr, !len && trunc ? "\"..." : "\"");
			break;
		}
		if ((unsigned char) *s >= 32) {
			*bufptr = *s;
			bufptr++;
		} else if (*s == '\0') {
			strcpy(bufptr, "\\0");
			bufptr += 2;
		} else if (*s == '\n') {
			strcpy(bufptr, "\\n");
			bufptr += 2;
		} else if (*s == '\t') {
			strcpy(bufptr, "\\t");
			bufptr += 2;
		} else {
			sprintf(bufptr, "\\%02x", *s);
			bufptr += 3;
		}
		s++;
		len--;
	}
	sys_debug("%s=%s\n", text, buf);
}


/* For 'futex' */

struct string_map_t futex_cmd_map =
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


/* For 'sysctl' */

struct sysctl_args_t
{
	uint32_t pname;
	uint32_t nlen;
	uint32_t poldval;
	uint32_t oldlenp;
	uint32_t pnewval;
	uint32_t newlen;
};



/* Summary of performed system calls */
void syscall_summary()
{
	int i;
	sys_debug("\nSystem calls summary:\n");
	for (i = 1; i < 325; i++)
	{
		if (!syscall_freq[i])
			continue;
		sys_debug("%s  %lld\n", syscall_name[i],
			(long long) syscall_freq[i]);
	}
}


/* Usually, glibc wrappers to system calls return -1 on error and set the
 * 'errno' variable to the appropriate value. However, the ABI with the
 * operating system is different - a return value less than 0 specifies
 * the error code. We use this macro for this kind of system calls. */
#define RETVAL(X) \
{ \
	retval = (X); \
	if (retval == -1) \
		retval = -errno; \
}


/* Simulation of system calls.
 * The system call code is in eax.
 * The parameters are given in ebx, ecx, edx, esi, edi, ebp.
 * The return value is placed in eax. */
void syscall_do()
{
	int syscode = isa_regs->eax;
	int retval = 0;

	/* Debug */
	sys_debug("syscall '%s' (code %d, inst %lld, pid %d)\n",
		syscode < syscall_code_count ? syscall_name[syscode] : "",
		syscode, (long long) isa_inst_count, isa_ctx->pid);
	if (syscode < syscall_code_count)
		syscall_freq[syscode]++;
	if (debug_status(isa_call_debug_category))
	{
		int i;
		for (i = 0; i < isa_function_level; i++)
			isa_call_debug("| ");
		isa_call_debug("syscall '%s' (code %d, inst %lld, pid %d)\n",
			syscode < syscall_code_count ? syscall_name[syscode] : "",
			syscode, (long long) isa_inst_count, isa_ctx->pid);
	}
	
	/* System call emulation */
	switch (syscode)
	{


	/* 1 */
	case syscall_code_exit:
	{
		retval = sys_exit_impl();
		break;
	}

	/* 2 */
	case syscall_code_close:
	{
		retval = sys_close_impl();
		break;
	}


	/* 3 */
	case syscall_code_read:
	{
		retval = sys_read_impl();
		break;
	}


	/* 4 */
	case syscall_code_write:
	{
		retval = sys_write_impl();
		break;
	}

	/* 5 */
	case syscall_code_open:
	{
		retval = sys_open_impl();
		break;
	}


	/* 7 */
	case syscall_code_waitpid:
	{
		retval = sys_waitpid_impl();
		break;
	}


	/* 10 */
	case syscall_code_unlink:
	{
		retval = sys_unlink_impl();
		break;
	}


	/* 11 */
	case syscall_code_execve:
	{
		retval = sys_execve_impl();
		break;
	}


	/* 13 */
	case syscall_code_time:
	{
		retval = sys_time_impl();
		break;
	}


	/* 15 */
	case syscall_code_chmod:
	{
		retval = sys_chmod_impl();
		break;
	}


	/* 19 */
	case syscall_code_lseek:
	{
		retval = sys_lseek_impl();
		break;
	}


	/* 20 */
	case syscall_code_getpid:
	{
		retval = sys_getpid_impl();
		break;
	}


	/* 30 */
	case syscall_code_utime:
	{
		retval = sys_utime_impl();
		break;
	}


	/* 33 */
	case syscall_code_access:
	{
		retval = sys_access_impl();
		break;
	}


	/* 37 */
	case syscall_code_kill:
	{
		retval = sys_kill_impl();
		break;
	}


	/* 38 */
	case syscall_code_rename:
	{
		retval = sys_rename_impl();
		break;
	}


	/* 39 */
	case syscall_code_mkdir:
	{
		retval = sys_mkdir_impl();
		break;
	}


	/* 41 */
	case syscall_code_dup:
	{
		retval = sys_dup_impl();
		break;
	}


	/* 42 */
	case syscall_code_pipe:
	{
		retval = sys_pipe_impl();
		break;
	}


	/* 43 */
	case syscall_code_times:
	{
		retval = sys_times_impl();
		break;
	}


	/* 45 */
	case syscall_code_brk:
	{
		retval = sys_brk_impl();
		break;
	}


	/* 54 */
	case syscall_code_ioctl:
	{
		retval = sys_ioctl_impl();
		break;
	}


	/* 64 */
	case syscall_code_getppid:
	{
		retval = sys_getppid_impl();
		break;
	}


	/* 75 */
	case syscall_code_setrlimit:
	{
		retval = sys_setrlimit_impl();
		break;
	}


	/* 77 */
	case syscall_code_getrusage:
	{
		retval = sys_getrusage_impl();
		break;
	}


	/* 78 */
	case syscall_code_gettimeofday:
	{
		retval = sys_gettimeofday_impl();
		break;
	}


	/* 85 */
	case syscall_code_readlink:
	{
		retval = sys_readlink_impl();
		break;
	}
	

	/* 90 */
	case syscall_code_mmap:
	{
		retval = sys_mmap_impl();
		break;
	}


	/* 91 */
	case syscall_code_munmap:
	{
		retval = sys_munmap_impl();
		break;
	}


	/* 94 */
	case syscall_code_fchmod:
	{
		retval = sys_fchmod_impl();
		break;
	}


	/* 102 */
	case syscall_code_socketcall:
	{
		retval = sys_socketcall_impl();
		break;
	}


	/* 104 */
	case syscall_code_setitimer:
	{
		retval = sys_setitimer_impl();
		break;
	}


	/* 105 */
	case syscall_code_getitimer:
	{
		retval = sys_getitimer_impl();
		break;
	}


	/* 119 */
	case syscall_code_sigreturn:
	{
		retval = sys_sigreturn_impl();
		break;
	}


	/* 120 */
	case syscall_code_clone:
	{
		retval = sys_clone_impl();
		break;
	}


	/* 122 */
	case syscall_code_newuname:
	{
		retval = sys_newuname_impl();
		break;
	}


	/* 125 */
	case syscall_code_mprotect:
	{
		retval = sys_mprotect_impl();
		break;
	}


	/* 140 */
	case syscall_code_llseek:
	{
		retval = sys_llseek_impl();
		break;
	}


	/* 141 */
	case syscall_code_getdents:
	{
		retval = sys_getdents_impl();
		break;
	}


	/* 142 */
	case syscall_code_select:
	{
		retval = sys_select_impl();
		break;
	}


	/* 144 */
	case syscall_code_msync:
	{
		retval = sys_msync_impl();
		break;
	}


	/* 146 */
	case syscall_code_writev:
	{
		retval = sys_writev_impl();
		break;
	}


	/* 149 */
	case syscall_code_sysctl:
	{
		retval = sys_sysctl_impl();
		break;
	}


	/* 154 */
	case syscall_code_sched_setparam:
	{
		retval = sys_sched_setparam_impl();
		break;
	}


	/* 155 */
	case syscall_code_sched_getparam:
	{
		retval = sys_sched_getparam_impl();
		break;
	}


	/* 157 */
	case syscall_code_sched_getscheduler:
	{
		retval = sys_sched_getscheduler_impl();
		break;
	}


	/* 159 */
	case syscall_code_sched_get_priority_max:
	{
		retval = sys_sched_get_priority_max_impl();
		break;
	}


	/* 160 */
	case syscall_code_sched_get_priority_min:
	{
		retval = sys_sched_get_priority_min_impl();
		break;
	}


	/* 162 */
	case syscall_code_nanosleep:
	{
		retval = sys_nanosleep_impl();
		break;
	}


	/* 163 */
	case syscall_code_mremap:
	{
		retval = sys_mremap_impl();
		break;
	}


	/* 168 */
	case syscall_code_poll:
	{
		retval = sys_poll_impl();
		break;
	}


	/* 174 */
	case syscall_code_rt_sigaction:
	{
		retval = sys_rt_sigaction_impl();
		break;
	}


	/* 175 */
	case syscall_code_rt_sigprocmask:
	{
		retval = sys_rt_sigprocmask_impl();
		break;
	}


	/* 179 */
	case syscall_code_rt_sigsuspend:
	{
		retval = sys_rt_sigsuspend_impl();
		break;
	}


	/* 183 */
	case syscall_code_getcwd:
	{
		retval = sys_getcwd_impl();
		break;
	}


	/* 191 */
	case syscall_code_getrlimit:
	{
		retval = sys_getrlimit_impl();
		break;
	}


	/* 192 */
	case syscall_code_mmap2:
	{
		retval = sys_mmap2_impl();
		break;
	}


	/* 194 */
	case syscall_code_ftruncate64:
	{
		retval = sys_ftruncate64_impl();
		break;
	}


	/* 195 */
	case syscall_code_stat64:
	{
		retval = sys_stat64_impl();
		break;
	}


	/* 196 */
	case syscall_code_lstat64:
	{
		retval = sys_lstat64_impl();
		break;
	}
		

	/* 197 */
	case syscall_code_fstat64:
	{
		retval = sys_fstat64_impl();
		break;
	}
		

	/* 199 */
	case syscall_code_getuid:
	{
		retval = sys_getuid_impl();
		break;
	}


	/* 200 */
	case syscall_code_getgid:
	{
		retval = sys_getgid_impl();
		break;
	}


	/* 201 */
	case syscall_code_geteuid:
	{
		retval = sys_geteuid_impl();
		break;
	}


	/* 202 */
	case syscall_code_getegid:
	{
		retval = sys_getegid_impl();
		break;
	}


	/* 212 */
	case syscall_code_chown:
	{
		retval = sys_chown_impl();
		break;
	}


	/* 219 */
	case syscall_code_madvise:
	{
		uint32_t start, len, advice;

		start = isa_regs->ebx;
		len = isa_regs->ecx;
		advice = isa_regs->edx;
		sys_debug("  start=0x%x, len=%d, advice=%d\n",
			start, len, advice);
		break;
	}


	/* 220 */
	case syscall_code_getdents64:
	{
		uint32_t fd, pdirent, count, host_fd;
		void *buf;
		int nread, host_offs, guest_offs;

		struct linux_dirent {
			long d_ino;
			off_t d_off;
			unsigned short d_reclen;
			char d_name[];
		} *dirent;

		struct sim_linux_dirent64 {
			uint64_t d_ino;
			int64_t d_off;
			uint16_t d_reclen;
			unsigned char d_type;
			char d_name[];
		} __attribute__((packed)) sim_dirent;

		/* Read parameters */
		fd = isa_regs->ebx;
		pdirent = isa_regs->ecx;
		count = isa_regs->edx;
		host_fd = file_desc_table_get_host_fd(isa_ctx->file_desc_table, fd);
		sys_debug("  fd=%d, pdirent=0x%x, count=%d\n",
			fd, pdirent, count);
		sys_debug("  host_fd=%d\n", host_fd);

		/* Call host getdents */
		buf = calloc(1, count);
		if (!buf)
			fatal("getdents: cannot allocate buffer");
		nread = syscall(SYS_getdents, host_fd, buf, count);

		/* Error or no more entries */
		if (nread < 0)
			fatal("getdents: call to host system call returned error");
		if (!nread) {
			retval = 0;
			free(buf);
			break;
		}

		/* Copy to host memory */
		host_offs = 0;
		guest_offs = 0;
		while (host_offs < nread) {
			dirent = (struct linux_dirent *) (buf + host_offs);
			sim_dirent.d_ino = dirent->d_ino;
			sim_dirent.d_off = dirent->d_off;
			sim_dirent.d_reclen = (27 + strlen(dirent->d_name)) / 8 * 8;
			sim_dirent.d_type = * (char *) (buf + host_offs + dirent->d_reclen - 1);

			sys_debug("    d_ino=%lld ", (long long) sim_dirent.d_ino);
			sys_debug("d_off=%lld ", (long long) sim_dirent.d_off);
			sys_debug("d_reclen=%u(host),%u(guest) ", dirent->d_reclen, sim_dirent.d_reclen);
			sys_debug("d_name='%s'\n", dirent->d_name);

			mem_write(isa_mem, pdirent + guest_offs, 8, &sim_dirent.d_ino);
			mem_write(isa_mem, pdirent + guest_offs + 8, 8, &sim_dirent.d_off);
			mem_write(isa_mem, pdirent + guest_offs + 16, 2, &sim_dirent.d_reclen);
			mem_write(isa_mem, pdirent + guest_offs + 18, 1, &sim_dirent.d_type);
			mem_write_string(isa_mem, pdirent + guest_offs + 19, dirent->d_name);
			
			host_offs += dirent->d_reclen;
			guest_offs += sim_dirent.d_reclen;
			if (guest_offs > count)
				fatal("getdents: host buffer too small");
		}
		sys_debug("  ret=%d(host),%d(guest)\n", host_offs, guest_offs);
		free(buf);
		retval = guest_offs;
		break;
	}



	/* 221 */
	case syscall_code_fcntl64:
	{
		retval = sys_fcntl64_impl();
		break;
	}


	/* 224 */
	case syscall_code_gettid:
	{
		/* FIXME: return different 'tid' for threads, but the system call
		 * 'getpid' should return the same 'pid' for threads from the same group
		 * created with CLONE_THREAD flag. */
		retval = isa_ctx->pid;
		break;
	}


	/* 240
	 * Prototype: sys_futex(void *addr1, int op, int val1, struct timespec *timeout,
	 *   void *addr2, int val3); */
	case syscall_code_futex:
	{
		uint32_t addr1, op, val1, ptimeout, addr2, val3;
		uint32_t timeout_sec, timeout_usec;
		uint32_t futex, cmd, bitset;

		addr1 = isa_regs->ebx;
		op = isa_regs->ecx;
		val1 = isa_regs->edx;
		ptimeout = isa_regs->esi;
		addr2 = isa_regs->edi;
		val3 = isa_regs->ebp;
		sys_debug("  addr1=0x%x, op=%d, val1=%d, ptimeout=0x%x, addr2=0x%x, val3=%d\n",
			addr1, op, val1, ptimeout, addr2, val3);

	
		/* Command - 'cmd' is obtained by removing 'FUTEX_PRIVATE_FLAG' (128) and
		 * 'FUTEX_CLOCK_REALTIME' from 'op'. */
		cmd = op & ~(256|128);
		mem_read(isa_mem, addr1, 4, &futex);
		sys_debug("  futex=%d, cmd=%d (%s)\n",
			futex, cmd, map_value(&futex_cmd_map, cmd));
		
		switch (cmd) {

		case 0:  /* FUTEX_WAIT */
		case 9:  /* FUTEX_WAIT_BITSET */
			
			/* Default bitset value (all bits set) */
			bitset = cmd == 9 ? val3 : 0xffffffff;

			/* First, we compare the value of the futex with val1. If it's not the
			 * same, we exit with the error EWOULDBLOCK. */
			if (futex != val1) {
				retval = -11;  /* EAGAIN = EWOULDBLOCK */
				break;
			}

			/* Read timeout */
			if (ptimeout) {
				fatal("syscall futex: FUTEX_WAIT not supported with timeout");
				mem_read(isa_mem, ptimeout, 4, &timeout_sec);
				mem_read(isa_mem, ptimeout + 4, 4, &timeout_usec);
				sys_debug("  timeout={sec %d, usec %d}\n",
					timeout_sec, timeout_usec);
			} else {
				timeout_sec = 0;
				timeout_usec = 0;
			}
			
			/* Suspend thread in the futex. */
			isa_ctx->wakeup_futex = addr1;
			isa_ctx->wakeup_futex_bitset = bitset;
			isa_ctx->wakeup_futex_sleep = ++ke->futex_sleep_count;
			ctx_set_status(isa_ctx, ctx_suspended | ctx_futex);
			break;

		case 1:  /* FUTEX_WAKE */
		case 10:  /* FUTEX_WAKE_BITSET */

			/* Default bitset value (all bits set) */
			bitset = cmd == 10 ? val3 : 0xffffffff;
			retval = ctx_futex_wake(isa_ctx, addr1, val1, bitset);
			sys_debug("  futex at 0x%x: %d processes woken up\n", addr1, retval);
			break;

		case 4: /* FUTEX_CMP_REQUEUE */
		{
			int requeued = 0;
			struct ctx_t *ctx;
			
			/* 'ptimeout' is interpreted here as an integer; only supported for INTMAX */
			if (ptimeout != 0x7fffffff)
				fatal("syscall futex, cmd=FUTEX_CMP_REQUEUE: only supported for ptimeout=INTMAX");
			
			/* The value of val3 must be the same as the value of the futex
			 * at 'addr1' (stored in 'futex') */
			if (futex != val3) {
				retval = -11; /* EAGAIN */
				break;
			}

			/* Wake up 'val1' threads from futex at 'addr1'. The number of woken up threads
			 * is the return value of the system call. */
			retval = ctx_futex_wake(isa_ctx, addr1, val1, 0xffffffff);
			sys_debug("  futex at 0x%x: %d processes woken up\n", addr1, retval);

			/* The rest of the threads waiting in futex 'addr1' are requeued into futex 'addr2' */
			for (ctx = ke->suspended_list_head; ctx; ctx = ctx->suspended_list_next) {
				if (ctx_get_status(ctx, ctx_futex) && ctx->wakeup_futex == addr1) {
					ctx->wakeup_futex = addr2;
					requeued++;
				}
			}
			sys_debug("  futex at 0x%x: %d processes requeued to futex 0x%x\n",
				addr1, requeued, addr2);
			break;
		}

		case 5: /* FUTEX_WAKE_OP */
		{
			int32_t op, oparg, cmp, cmparg;
			int32_t val2 = ptimeout;
			int32_t oldval, newval = 0, cond = 0;

			op = (val3 >> 28) & 0xf;
			cmp = (val3 >> 24) & 0xf;
			oparg = (val3 >> 12) & 0xfff;
			cmparg = val3 & 0xfff;

			mem_read(isa_mem, addr2, 4, &oldval);
			switch (op) {
			case 0: newval = oparg; break;  /* FUTEX_OP_SET */
			case 1: newval = oldval + oparg; break;  /* FUTEX_OP_ADD */
			case 2: newval = oldval | oparg; break;  /* FUTEX_OP_OR */
			case 3: newval = oldval & oparg; break;  /* FUTEX_OP_AND */
			case 4: newval = oldval ^ oparg; break;  /* FOTEX_OP_XOR */
			default: fatal("FUTEX_WAKE_OP: invalid operation");
			}
			mem_write(isa_mem, addr2, 4, &newval);

			retval = ctx_futex_wake(isa_ctx, addr1, val1, 0xffffffff);

			switch (cmp) {
			case 0: cond = oldval == cmparg; break;  /* FUTEX_OP_CMP_EQ */
			case 1: cond = oldval != cmparg; break;  /* FUTEX_OP_CMP_NE */
			case 2: cond = oldval < cmparg; break;  /* FUTEX_OP_CMP_LT */
			case 3: cond = oldval <= cmparg; break;  /* FUTEX_OP_CMP_LE */
			case 4: cond = oldval > cmparg; break;  /* FUTEX_OP_CMP_GT */
			case 5: cond = oldval >= cmparg; break;  /* FUTEX_OP_CMP_GE */
			default: fatal("FUTEX_WAKE_OP: invalid condition");
			}
			if (cond)
				retval += ctx_futex_wake(isa_ctx, addr2, val2, 0xffffffff);
			/* FIXME: we are returning the total number of threads woken up
			 * counting both calls to ctx_futex_wake. Is this correct? */
			break;
		}

		default:
			fatal("syscall futex: not implemented for cmd=%d (%s)",
				cmd, map_value(&futex_cmd_map, cmd));
		}
		break;
	}


	/* 241 */
	case syscall_code_sched_setaffinity:
	{
		uint32_t pid, len, pmask;
		uint32_t num_procs = 4;
		uint32_t mask;

		pid = isa_regs->ebx;
		len = isa_regs->ecx;
		pmask = isa_regs->edx;

		mem_read(isa_mem, pmask, 4, &mask);
		sys_debug("  pid=%d, len=%d, pmask=0x%x\n", pid, len, pmask);
		sys_debug("  mask=0x%x\n", mask);

		/* FIXME: system call ignored. Return the number of procs. */
		retval = num_procs;
		break;
	}


	/* 242 */
	case syscall_code_sched_getaffinity:
	{
		uint32_t pid, len, pmask;
		uint32_t num_procs = 4;
		uint32_t mask = (1 << num_procs) - 1;

		pid = isa_regs->ebx;
		len = isa_regs->ecx;
		pmask = isa_regs->edx;
		sys_debug("  pid=%d, len=%d, pmask=0x%x\n", pid, len, pmask);

		/* FIXME: the affinity is set to 1 for num_procs processors and only the 4 LSBytes are set.
		 * The return value is set to num_procs. This is the behavior on a 4-core processor
		 * in a real system. */
		mem_write(isa_mem, pmask, 4, &mask);
		retval = num_procs;
		break;
	}


	/* 243 */
	case syscall_code_set_thread_area:
	{
		retval = sys_set_thread_area_impl();
		break;
	}


	/* 250 */
	case syscall_code_fadvise64:
	{
		uint32_t fd;
		uint32_t off_hi, off_lo;
		uint32_t len, advice;

		fd = isa_regs->ebx;
		off_lo = isa_regs->ecx;
		off_hi = isa_regs->edx;
		len = isa_regs->esi;
		advice = isa_regs->edi;

		sys_debug("  fd=%d, off={0x%x, 0x%x}, len=%d, advice=%d\n",
			fd, off_hi, off_lo, len, advice);
		break;
	}


	/* 252 */
	case syscall_code_exit_group:
	{
		int status;

		status = isa_regs->ebx;
		sys_debug("  status=0x%x\n", status);

		ctx_finish_group(isa_ctx, status);
		break;
	}


	/* 258 */
	case syscall_code_set_tid_address:
	{
		uint32_t tidptr;

		tidptr = isa_regs->ebx;
		sys_debug("  tidptr=0x%x\n", tidptr);

		isa_ctx->clear_child_tid = tidptr;
		retval = isa_ctx->pid;
		break;
	}


	/* 266 */
	case syscall_code_clock_getres:
	{
		uint32_t clk_id, pres;
		uint32_t tv_sec, tv_nsec;

		clk_id = isa_regs->ebx;
		pres = isa_regs->ecx;
		sys_debug("  clk_id=%d\n", clk_id);
		sys_debug("  pres=0x%x\n", pres);

		tv_sec = 0;
		tv_nsec = 1;
		mem_write(isa_mem, pres, 4, &tv_sec);
		mem_write(isa_mem, pres + 4, 4, &tv_nsec);
		break;
	}


	/* 270 */
	case syscall_code_tgkill:
	{
		uint32_t tgid, pid, sig;
		struct ctx_t *ctx;

		tgid = isa_regs->ebx;
		pid = isa_regs->ecx;
		sig = isa_regs->edx;
		sys_debug("  tgid=%d, pid=%d, sig=%d (%s)\n",
			tgid, pid, sig, sim_signal_name(sig));

		/* Implementation restrictions. */
		if ((int) tgid == -1)
			fatal("syscall 'tgkill': not implemented for tgid = -1");

		/* Find context referred by pid. */
		ctx = ctx_get(pid);
		if (!ctx)
			fatal("syscall 'tgkill': pid %d does not exist", pid);

		/* Send signal */
		sim_sigset_add(&ctx->signal_mask_table->pending, sig);
		ctx_host_thread_suspend_cancel(ctx);  /* Target ctx might wake up */
		ke_process_events_schedule();
		ke_process_events();
		break;
	}


	/* 311 */
	case syscall_code_set_robust_list:
	{
		uint32_t head, len;

		head = isa_regs->ebx;
		len = isa_regs->ecx;
		sys_debug("  head=0x%x, len=%d\n", head, len);
		if (len != 12)
			fatal("set_robust_list: only working for len = 12");
		isa_ctx->robust_list_head = head;
		break;
	}


	/* 325 */
	/* Artificial system call used to implement the OpenCL 1.1 interface. */
	case syscall_code_opencl:
	{
		uint32_t func_code, pargs;
		uint32_t args[OPENCL_MAX_ARGS];
		int i;
		char *func_name;
		int func_argc;

		func_code = isa_regs->ebx;
		pargs = isa_regs->ecx;

		/* Check 'func_code' range */
		if (func_code < OPENCL_FUNC_FIRST || func_code > OPENCL_FUNC_LAST)
			fatal("syscall 'opencl': func_code out of range");
		
		/* Get function info */
		func_name = opencl_func_names[func_code - OPENCL_FUNC_FIRST];
		func_argc = opencl_func_argc[func_code - OPENCL_FUNC_FIRST];
		sys_debug("  func_code=%d (%s, %d arguments), pargs=0x%x\n",
			func_code, func_name, func_argc, pargs);

		/* Read function args */
		assert(func_argc <= OPENCL_MAX_ARGS);
		mem_read(isa_mem, pargs, func_argc * 4, args);
		for (i = 0; i < func_argc; i++)
			sys_debug("    args[%d] = %d (0x%x)\n",
				i, args[i], args[i]);

		/* Run OpenCL function */
		retval = opencl_func_run(func_code, args);
		break;
	}


	/* 326 */
	/* OpenCL system call for libm2s-opencl.so Version 2.0 */
	case 326:
	{
		fatal("OpenCL system call for libm2s-opencl.so 2.0 not implemented");
		break;
	}


	default:
		if (syscode >= syscall_code_count) {
			retval = -38;
		} else {
			fatal("not implemented system call '%s' (code %d) at 0x%x",
				syscode < syscall_code_count ? syscall_name[syscode] : "",
				syscode, isa_regs->eip);
		}
	}

	/* Return value (for all system calls except 'sigreturn') */
	if (syscode != syscall_code_sigreturn && !ctx_get_status(isa_ctx, ctx_suspended))
		isa_regs->eax = retval;
}

