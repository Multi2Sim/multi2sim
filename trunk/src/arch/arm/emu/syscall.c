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

#include <sched.h>
#include <syscall.h>
#include <time.h>
#include <utime.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/times.h>

#include <mem-system.h>
#include <mhandle.h>
#include <arm-emu.h>


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


static struct string_map_t arm_sys_error_code_map =
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
		arm_sys_call_name[code], code, arm_emu->inst_count, ctx->pid);
	arm_isa_call_debug("system call '%s' (code %d, inst %lld, pid %d)\n",
		arm_sys_call_name[code], code, arm_emu->inst_count, ctx->pid);

	/* Perform system call */
	err = arm_sys_call_func[code](ctx);

	/* Set return value in 'eax', except for 'sigreturn' system call. Also, if the
	 * context got suspended, the wake up routine will set the return value. */
	if (code != arm_sys_code_sigreturn && !arm_ctx_get_status(ctx, arm_ctx_suspended))
		regs->r0 = err;

	/* Debug */
	arm_sys_debug("  ret=(%d, 0x%x)", err, err);
	if (err < 0 && err >= -SIM_ERRNO_MAX)
		arm_sys_debug(", errno=%s)", map_value(&arm_sys_error_code_map, -err));
	arm_sys_debug("\n");
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
 * System call 'ARM_set_tls' (code 330)
 */

static int arm_sys_ARM_set_tls_impl(struct arm_ctx_t *ctx)
{
	unsigned int newtls;

	/* Arguments */
	newtls = ctx->regs->r0;

	/* Set the tls value */
	ctx->regs->cp15.c13_tls3 = newtls;

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
			__FUNCTION__, regs->r7, arm_emu->inst_count, ctx->pid, \
			err_arm_sys_note); \
		return 0; \
	}

SYS_NOT_IMPL(restart_syscall)
SYS_NOT_IMPL(exit)
SYS_NOT_IMPL(fork)
SYS_NOT_IMPL(read)
SYS_NOT_IMPL(write)
SYS_NOT_IMPL(open)
SYS_NOT_IMPL(close)
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
SYS_NOT_IMPL(gettimeofday)
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
SYS_NOT_IMPL(munmap)
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
SYS_NOT_IMPL(mmap2)
SYS_NOT_IMPL(truncate64)
SYS_NOT_IMPL(ftruncate64)
SYS_NOT_IMPL(stat64)
SYS_NOT_IMPL(lstat64)
SYS_NOT_IMPL(fstat64)
SYS_NOT_IMPL(lchown)
SYS_NOT_IMPL(getuid)
SYS_NOT_IMPL(getgid)
SYS_NOT_IMPL(geteuid)
SYS_NOT_IMPL(getegid)
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
SYS_NOT_IMPL(exit_group)
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

