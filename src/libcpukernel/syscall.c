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
		retval = sys_madvise_impl();
		break;
	}


	/* 220 */
	case syscall_code_getdents64:
	{
		retval = sys_getdents64_impl();
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
		retval = sys_gettid_impl();
		break;
	}


	/* 240 */
	case syscall_code_futex:
	{
		retval = sys_futex_impl();
		break;
	}


	/* 241 */
	case syscall_code_sched_setaffinity:
	{
		retval = sys_sched_setaffinity_impl();
		break;
	}


	/* 242 */
	case syscall_code_sched_getaffinity:
	{
		retval = sys_sched_getaffinity_impl();
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

