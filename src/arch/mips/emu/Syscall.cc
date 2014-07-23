/*
 *  Multi2Sim
 *  Copyright (C) 2014  Sida Gu(dudlykoo@gmail.com)
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
#include <poll.h>
#include <unistd.h>

#include <lib/cpp/Misc.h>

#include "Context.h"
#include "Emu.h"

namespace MIPS
{
#define __UNIMPLEMENTED__ misc::fatal("%s: unimplemented system call.\n\n%s", \
		__FUNCTION__ + 15, syscall_error_note); return 0;

const char *Context::syscall_name[SyscallCodeCount + 1] =
{
#define DEFSYSCALL(name, code) #name,
#include "syscall.dat"
#undef DEFSYSCALL
		""
};

const Context::ExecuteSyscallFn Context::execute_syscall_fn[SyscallCodeCount + 1] =
{
#define DEFSYSCALL(name, code) &Context::ExecuteSyscall_##name,
#include "syscall.dat"
#undef DEFSYSCALL
		nullptr
};

void Context::ExecuteSyscall()
{
	// Get system call code from GPR
	int code = regs.getGPR(2) - __NR_Linux;
	if (code < 1 || code >= SyscallCodeCount)
		printf("invalid system call (code %d)", code);

	// Perform system call
	ExecuteSyscallFn fn = execute_syscall_fn[code];
	int ret = (this->*fn)();

	//FIXME: Syscallcode sigreturn not implemented
	// Set return value in 'eax', except for 'sigreturn' system call. Also, if the
	// context got suspended, the wake up routine will set the return value.
	if (/*code != SyscallCode_sigreturn &&*/ !getState(ContextSuspended))
		regs.setGPR(2,ret);
}


int Context::ExecuteSyscall_syscall()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_exit()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fork()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_read()
{
	throw misc::Panic("Unimplemented syscall");
}

//
// System call 'write'
//
void Context::SyscallWriteWakeup()
{
}

bool Context::SyscallWriteCanWakeup()
{
	return false;
}


int Context::ExecuteSyscall_write()
{
	// Arguments
	int guest_fd = regs.getGPR(4);
	unsigned buf_ptr = regs.getGPR(5);
	unsigned count = regs.getGPR(6);

	// Get file descriptor
	comm::FileDescriptor *desc = file_table->getFileDescriptor(guest_fd);
	if(!desc)
		return -EBADF;
	int host_fd = desc->getHostIndex();

	// Read buffer from memory
	char *buf = new char[count];
	memory->Read(buf_ptr,count, buf);

	// Poll the file descriptor to check if write is blocking
	struct pollfd fds;
	fds.fd = host_fd;
	fds.events = POLLOUT;
	poll(&fds, 1, 0);

	// Non-blocking write
	if (fds.revents)
	{
		// Host write
		int err = write(host_fd, buf, count);
		if (err == -1)
			err = -errno;

		// Return written bytes
		delete buf;
		return err;
	}

	// Blocking write - suspend thread
	syscall_write_fd = guest_fd;
	Suspend(&Context::SyscallWriteCanWakeup, &Context::SyscallWriteWakeup,
			ContextWrite);
	emu->ProcessEventsSchedule();

	// Return value doesn't matter here. It will be overwritten when the
	// context wakes up after blocking call.
	delete buf;
	return 0;
}


int Context::ExecuteSyscall_open()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_close()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_waitpid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_creat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_link()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_unlink()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_execve()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_chdir()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_time()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mknod()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_chmod()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_lchown()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_break()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_unused18()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_lseek()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getpid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mount()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_umount()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setuid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getuid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_stime()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_ptrace()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_alarm()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_unused28()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_pause()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_utime()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_stty()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_gtty()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_access()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_nice()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_ftime()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sync()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_kill()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_rename()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mkdir()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_rmdir()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_dup()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_pipe()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_times()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_prof()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_brk()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setgid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getgid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_signal()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_geteuid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getegid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_acct()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_umount2()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_lock()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_ioctl()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fcntl()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mpx()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setpgid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_ulimit()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_unused59()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_umask()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_chroot()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_ustat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_dup2()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getppid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getpgrp()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setsid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sigaction()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sgetmask()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_ssetmask()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setreuid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setregid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sigsuspend()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sigpending()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sethostname()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setrlimit()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getrlimit()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getrusage()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_gettimeofday()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_settimeofday()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getgroups()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setgroups()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_reserved82()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_symlink()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_unused84()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_readlink()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_uselib()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_swapon()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_reboot()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_readdir()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mmap()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_munmap()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_truncate()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_ftruncate()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fchmod()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fchown()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getpriority()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setpriority()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_profil()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_statfs()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fstatfs()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_ioperm()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_socketcall()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_syslog()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setitimer()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getitimer()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_stat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_lstat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fstat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_unused109()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_iopl()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_vhangup()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_idle()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_vm86()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_wait4()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_swapoff()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sysinfo()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_ipc()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fsync()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sigreturn()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_clone()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setdomainname()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_uname()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_modify_ldt()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_adjtimex()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mprotect()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sigprocmask()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_create_module()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_init_module()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_delete_module()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_get_kernel_syms()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_quotactl()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getpgid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fchdir()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_bdflush()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sysfs()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_personality()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_afs_syscall()
{
	throw misc::Panic("Unimplemented syscall");
}
/* Syscall for Andrew File System */

int Context::ExecuteSyscall_setfsuid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setfsgid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall__llseek()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getdents()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall__newselect()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_flock()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_msync()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_readv()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_writev()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_cacheflush()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_cachectl()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sysmips()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_unused150()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getsid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fdatasync()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall__sysctl()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall__mlock()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_munlock()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mlockall()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_munlockall()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sched_setparam()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sched_getparam()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sched_setscheduler()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sched_getscheduler()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sched_yield()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sched_get_priority_max()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sched_get_priority_min()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sched_rr_get_interval()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_nanosleep()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mremap()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_accept()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_bind()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_connect()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getpeername()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getsockname()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getsockopt()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_listen()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_recv()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_recvfrom()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_recvmsg()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_send()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sendmsg()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sendto()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setsockopt()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_shutdown()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_socket()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_socketpair()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setresuid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getresuid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_query_module()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_poll()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_nfsservctl()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setresgid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getresgid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_prctl()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_rt_sigreturn()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_rt_sigaction()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_rt_sigprocmask()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_rt_sigpending()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_rt_sigtimedwait()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_rt_sigqueueinfo()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_rt_sigsuspend()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_pread64()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_pwrite64()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_chown()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getcwd()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_capget()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_capset()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sigaltstack()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sendfile()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getpmsg()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_putpmsg()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mmap2()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_truncate64()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_ftruncate64()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_stat64()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_lstat64()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fstat64()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_pivot_root()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mincore()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_madvise()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getdents64()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fcntl64()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_reserved221()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_gettid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_readahead()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_setxattr()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_lsetxattr()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fsetxattr()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getxattr()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_lgetxattr()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fgetxattr()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_listxattr()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_llistxattr()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_flistxattr()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_removexattr()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_lremovexattr()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fremovexattr()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_tkill()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sendfile64()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_futex()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sched_setaffinity()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sched_getaffinity()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_io_setup()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_io_destroy()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_io_getevents()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_io_submit()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_io_cancel()
{
	throw misc::Panic("Unimplemented syscall");
}

//
// System call 'exit_group'
//


int Context::ExecuteSyscall_exit_group()
{
	// Arguments
	int status = regs.getGPR(4);

	// Finish context
	Finish(status);
	return 0;
}


int Context::ExecuteSyscall_lookup_dcookie()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_epoll_create()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_epoll_ctl()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_epoll_wait()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_remap_file_pages()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_set_tid_address()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_restart_syscall()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fadvise64()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_statfs64()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fstatfs64()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_timer_create()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_timer_settime()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_timer_gettime()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_timer_getoverrun()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_timer_delete()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_clock_settime()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_clock_gettime()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_clock_getres()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_clock_nanosleep()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_tgkill()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_utimes()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mbind()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_get_mempolicy()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_set_mempolicy()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mq_open()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mq_unlink()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mq_timedsend()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mq_timedreceive()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mq_notify()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mq_getsetattr()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_vserver()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_waitid()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sys_setaltroot()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_add_key()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_request_key()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_keyctl()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_set_thread_area()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_inotify_init()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_inotify_add_watch()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_inotify_rm_watch()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_migrate_pages()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_openat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mkdirat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_mknodat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fchownat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_futimesat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fstatat64()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_unlinkat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_renameat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_linkat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_symlinkat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_readlinkat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fchmodat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_faccessat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_pselect6()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_ppoll()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_unshare()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_splice()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_sync_file_range()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_tee()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_vmsplice()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_move_pages()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_set_robust_list()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_get_robust_list()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_kexec_load()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_getcpu()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_epoll_pwait()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_ioprio_set()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_ioprio_get()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_utimensat()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_signalfd()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_timerfd()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_eventfd()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_fallocate()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_timerfd_create()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_timerfd_gettime()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_timerfd_settime()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_signalfd4()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_eventfd2()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_epoll_create1()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_dup3()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_pipe2()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_inotify_init1()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_preadv()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_pwritev()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_rt_tgsigqueueinfo()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_perf_event_open()
{
	throw misc::Panic("Unimplemented syscall");
}


int Context::ExecuteSyscall_accept4()
{
	throw misc::Panic("Unimplemented syscall");
}

} // namespace MIPS











