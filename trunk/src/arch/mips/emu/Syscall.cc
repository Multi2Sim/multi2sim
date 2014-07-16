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




int	Context::ExecuteSyscall_syscall()
{
	return	0;
}

int	Context::ExecuteSyscall_exit()
{
	return	0;
}

int	Context::ExecuteSyscall_fork()
{
	return	0;
}

int	Context::ExecuteSyscall_read()
{
	return	0;
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

int	Context::ExecuteSyscall_open()
{
	return	0;
}

int	Context::ExecuteSyscall_close()
{
	return	0;
}

int	Context::ExecuteSyscall_waitpid()
{
	return	0;
}

int	Context::ExecuteSyscall_creat()
{
	return	0;
}

int	Context::ExecuteSyscall_link()
{
	return	0;
}

int	Context::ExecuteSyscall_unlink()
{
	return	0;
}

int	Context::ExecuteSyscall_execve()
{
	return	0;
}

int	Context::ExecuteSyscall_chdir()
{
	return	0;
}

int	Context::ExecuteSyscall_time()
{
	return	0;
}

int	Context::ExecuteSyscall_mknod()
{
	return	0;
}

int	Context::ExecuteSyscall_chmod()
{
	return	0;
}

int	Context::ExecuteSyscall_lchown()
{
	return	0;
}

int	Context::ExecuteSyscall_break()
{
	return	0;
}

int	Context::ExecuteSyscall_unused18()
{
	return	0;
}

int	Context::ExecuteSyscall_lseek()
{
	return	0;
}

int	Context::ExecuteSyscall_getpid()
{
	return	0;
}

int	Context::ExecuteSyscall_mount()
{
	return	0;
}

int	Context::ExecuteSyscall_umount()
{
	return	0;
}

int	Context::ExecuteSyscall_setuid()
{
	return	0;
}

int	Context::ExecuteSyscall_getuid()
{
	return	0;
}

int	Context::ExecuteSyscall_stime()
{
	return	0;
}

int	Context::ExecuteSyscall_ptrace()
{
	return	0;
}

int	Context::ExecuteSyscall_alarm()
{
	return	0;
}

int	Context::ExecuteSyscall_unused28()
{
	return	0;
}

int	Context::ExecuteSyscall_pause()
{
	return	0;
}

int	Context::ExecuteSyscall_utime()
{
	return	0;
}

int	Context::ExecuteSyscall_stty()
{
	return	0;
}

int	Context::ExecuteSyscall_gtty()
{
	return	0;
}

int	Context::ExecuteSyscall_access()
{
	return	0;
}

int	Context::ExecuteSyscall_nice()
{
	return	0;
}

int	Context::ExecuteSyscall_ftime()
{
	return	0;
}

int	Context::ExecuteSyscall_sync()
{
	return	0;
}

int	Context::ExecuteSyscall_kill()
{
	return	0;
}

int	Context::ExecuteSyscall_rename()
{
	return	0;
}

int	Context::ExecuteSyscall_mkdir()
{
	return	0;
}

int	Context::ExecuteSyscall_rmdir()
{
	return	0;
}

int	Context::ExecuteSyscall_dup()
{
	return	0;
}

int	Context::ExecuteSyscall_pipe()
{
	return	0;
}

int	Context::ExecuteSyscall_times()
{
	return	0;
}

int	Context::ExecuteSyscall_prof()
{
	return	0;
}

int	Context::ExecuteSyscall_brk()
{
	return	0;
}

int	Context::ExecuteSyscall_setgid()
{
	return	0;
}

int	Context::ExecuteSyscall_getgid()
{
	return	0;
}

int	Context::ExecuteSyscall_signal()
{
	return	0;
}

int	Context::ExecuteSyscall_geteuid()
{
	return	0;
}

int	Context::ExecuteSyscall_getegid()
{
	return	0;
}

int	Context::ExecuteSyscall_acct()
{
	return	0;
}

int	Context::ExecuteSyscall_umount2()
{
	return	0;
}

int	Context::ExecuteSyscall_lock()
{
	return	0;
}

int	Context::ExecuteSyscall_ioctl()
{
	return	0;
}

int	Context::ExecuteSyscall_fcntl()
{
	return	0;
}

int	Context::ExecuteSyscall_mpx()
{
	return	0;
}

int	Context::ExecuteSyscall_setpgid()
{
	return	0;
}

int	Context::ExecuteSyscall_ulimit()
{
	return	0;
}

int	Context::ExecuteSyscall_unused59()
{
	return	0;
}

int	Context::ExecuteSyscall_umask()
{
	return	0;
}

int	Context::ExecuteSyscall_chroot()
{
	return	0;
}

int	Context::ExecuteSyscall_ustat()
{
	return	0;
}

int	Context::ExecuteSyscall_dup2()
{
	return	0;
}

int	Context::ExecuteSyscall_getppid()
{
	return	0;
}

int	Context::ExecuteSyscall_getpgrp()
{
	return	0;
}

int	Context::ExecuteSyscall_setsid()
{
	return	0;
}

int	Context::ExecuteSyscall_sigaction()
{
	return	0;
}

int	Context::ExecuteSyscall_sgetmask()
{
	return	0;
}

int	Context::ExecuteSyscall_ssetmask()
{
	return	0;
}

int	Context::ExecuteSyscall_setreuid()
{
	return	0;
}

int	Context::ExecuteSyscall_setregid()
{
	return	0;
}

int	Context::ExecuteSyscall_sigsuspend()
{
	return	0;
}

int	Context::ExecuteSyscall_sigpending()
{
	return	0;
}

int	Context::ExecuteSyscall_sethostname()
{
	return	0;
}

int	Context::ExecuteSyscall_setrlimit()
{
	return	0;
}

int	Context::ExecuteSyscall_getrlimit()
{
	return	0;
}

int	Context::ExecuteSyscall_getrusage()
{
	return	0;
}

int	Context::ExecuteSyscall_gettimeofday()
{
	return	0;
}

int	Context::ExecuteSyscall_settimeofday()
{
	return	0;
}

int	Context::ExecuteSyscall_getgroups()
{
	return	0;
}

int	Context::ExecuteSyscall_setgroups()
{
	return	0;
}

int	Context::ExecuteSyscall_reserved82()
{
	return	0;
}

int	Context::ExecuteSyscall_symlink()
{
	return	0;
}

int	Context::ExecuteSyscall_unused84()
{
	return	0;
}

int	Context::ExecuteSyscall_readlink()
{
	return	0;
}

int	Context::ExecuteSyscall_uselib()
{
	return	0;
}

int	Context::ExecuteSyscall_swapon()
{
	return	0;
}

int	Context::ExecuteSyscall_reboot()
{
	return	0;
}

int	Context::ExecuteSyscall_readdir()
{
	return	0;
}

int	Context::ExecuteSyscall_mmap()
{
	return	0;
}

int	Context::ExecuteSyscall_munmap()
{
	return	0;
}

int	Context::ExecuteSyscall_truncate()
{
	return	0;
}

int	Context::ExecuteSyscall_ftruncate()
{
	return	0;
}

int	Context::ExecuteSyscall_fchmod()
{
	return	0;
}

int	Context::ExecuteSyscall_fchown()
{
	return	0;
}

int	Context::ExecuteSyscall_getpriority()
{
	return	0;
}

int	Context::ExecuteSyscall_setpriority()
{
	return	0;
}

int	Context::ExecuteSyscall_profil()
{
	return	0;
}

int	Context::ExecuteSyscall_statfs()
{
	return	0;
}

int	Context::ExecuteSyscall_fstatfs()
{
	return	0;
}

int	Context::ExecuteSyscall_ioperm()
{
	return	0;
}

int	Context::ExecuteSyscall_socketcall()
{
	return	0;
}

int	Context::ExecuteSyscall_syslog()
{
	return	0;
}

int	Context::ExecuteSyscall_setitimer()
{
	return	0;
}

int	Context::ExecuteSyscall_getitimer()
{
	return	0;
}

int	Context::ExecuteSyscall_stat()
{
	return	0;
}

int	Context::ExecuteSyscall_lstat()
{
	return	0;
}

int	Context::ExecuteSyscall_fstat()
{
	return	0;
}

int	Context::ExecuteSyscall_unused109()
{
	return	0;
}

int	Context::ExecuteSyscall_iopl()
{
	return	0;
}

int	Context::ExecuteSyscall_vhangup()
{
	return	0;
}

int	Context::ExecuteSyscall_idle()
{
	return	0;
}

int	Context::ExecuteSyscall_vm86()
{
	return	0;
}

int	Context::ExecuteSyscall_wait4()
{
	return	0;
}

int	Context::ExecuteSyscall_swapoff()
{
	return	0;
}

int	Context::ExecuteSyscall_sysinfo()
{
	return	0;
}

int	Context::ExecuteSyscall_ipc()
{
	return	0;
}

int	Context::ExecuteSyscall_fsync()
{
	return	0;
}

int	Context::ExecuteSyscall_sigreturn()
{
	return	0;
}

int	Context::ExecuteSyscall_clone()
{
	return	0;
}

int	Context::ExecuteSyscall_setdomainname()
{
	return	0;
}

int	Context::ExecuteSyscall_uname()
{
	return	0;
}

int	Context::ExecuteSyscall_modify_ldt()
{
	return	0;
}

int	Context::ExecuteSyscall_adjtimex()
{
	return	0;
}

int	Context::ExecuteSyscall_mprotect()
{
	return	0;
}

int	Context::ExecuteSyscall_sigprocmask()
{
	return	0;
}

int	Context::ExecuteSyscall_create_module()
{
	return	0;
}

int	Context::ExecuteSyscall_init_module()
{
	return	0;
}

int	Context::ExecuteSyscall_delete_module()
{
	return	0;
}

int	Context::ExecuteSyscall_get_kernel_syms()
{
	return	0;
}

int	Context::ExecuteSyscall_quotactl()
{
	return	0;
}

int	Context::ExecuteSyscall_getpgid()
{
	return	0;
}

int	Context::ExecuteSyscall_fchdir()
{
	return	0;
}

int	Context::ExecuteSyscall_bdflush()
{
	return	0;
}

int	Context::ExecuteSyscall_sysfs()
{
	return	0;
}

int	Context::ExecuteSyscall_personality()
{
	return	0;
}

int	Context::ExecuteSyscall_afs_syscall()
{
	return	0;
}
/* Syscall for Andrew File System */
int	Context::ExecuteSyscall_setfsuid()
{
	return	0;
}

int	Context::ExecuteSyscall_setfsgid()
{
	return	0;
}

int	Context::ExecuteSyscall__llseek()
{
	return	0;
}

int	Context::ExecuteSyscall_getdents()
{
	return	0;
}

int	Context::ExecuteSyscall__newselect()
{
	return	0;
}

int	Context::ExecuteSyscall_flock()
{
	return	0;
}

int	Context::ExecuteSyscall_msync()
{
	return	0;
}

int	Context::ExecuteSyscall_readv()
{
	return	0;
}

int	Context::ExecuteSyscall_writev()
{
	return	0;
}

int	Context::ExecuteSyscall_cacheflush()
{
	return	0;
}

int	Context::ExecuteSyscall_cachectl()
{
	return	0;
}

int	Context::ExecuteSyscall_sysmips()
{
	return	0;
}

int	Context::ExecuteSyscall_unused150()
{
	return	0;
}

int	Context::ExecuteSyscall_getsid()
{
	return	0;
}

int	Context::ExecuteSyscall_fdatasync()
{
	return	0;
}

int	Context::ExecuteSyscall__sysctl()
{
	return	0;
}

int	Context::ExecuteSyscall__mlock()
{
	return	0;
}

int	Context::ExecuteSyscall_munlock()
{
	return	0;
}

int	Context::ExecuteSyscall_mlockall()
{
	return	0;
}

int	Context::ExecuteSyscall_munlockall()
{
	return	0;
}

int	Context::ExecuteSyscall_sched_setparam()
{
	return	0;
}

int	Context::ExecuteSyscall_sched_getparam()
{
	return	0;
}

int	Context::ExecuteSyscall_sched_setscheduler()
{
	return	0;
}

int	Context::ExecuteSyscall_sched_getscheduler()
{
	return	0;
}

int	Context::ExecuteSyscall_sched_yield()
{
	return	0;
}

int	Context::ExecuteSyscall_sched_get_priority_max()
{
	return	0;
}

int	Context::ExecuteSyscall_sched_get_priority_min()
{
	return	0;
}

int	Context::ExecuteSyscall_sched_rr_get_interval()
{
	return	0;
}

int	Context::ExecuteSyscall_nanosleep()
{
	return	0;
}

int	Context::ExecuteSyscall_mremap()
{
	return	0;
}

int	Context::ExecuteSyscall_accept()
{
	return	0;
}

int	Context::ExecuteSyscall_bind()
{
	return	0;
}

int	Context::ExecuteSyscall_connect()
{
	return	0;
}

int	Context::ExecuteSyscall_getpeername()
{
	return	0;
}

int	Context::ExecuteSyscall_getsockname()
{
	return	0;
}

int	Context::ExecuteSyscall_getsockopt()
{
	return	0;
}

int	Context::ExecuteSyscall_listen()
{
	return	0;
}

int	Context::ExecuteSyscall_recv()
{
	return	0;
}

int	Context::ExecuteSyscall_recvfrom()
{
	return	0;
}

int	Context::ExecuteSyscall_recvmsg()
{
	return	0;
}

int	Context::ExecuteSyscall_send()
{
	return	0;
}

int	Context::ExecuteSyscall_sendmsg()
{
	return	0;
}

int	Context::ExecuteSyscall_sendto()
{
	return	0;
}

int	Context::ExecuteSyscall_setsockopt()
{
	return	0;
}

int	Context::ExecuteSyscall_shutdown()
{
	return	0;
}

int	Context::ExecuteSyscall_socket()
{
	return	0;
}

int	Context::ExecuteSyscall_socketpair()
{
	return	0;
}

int	Context::ExecuteSyscall_setresuid()
{
	return	0;
}

int	Context::ExecuteSyscall_getresuid()
{
	return	0;
}

int	Context::ExecuteSyscall_query_module()
{
	return	0;
}

int	Context::ExecuteSyscall_poll()
{
	return	0;
}

int	Context::ExecuteSyscall_nfsservctl()
{
	return	0;
}

int	Context::ExecuteSyscall_setresgid()
{
	return	0;
}

int	Context::ExecuteSyscall_getresgid()
{
	return	0;
}

int	Context::ExecuteSyscall_prctl()
{
	return	0;
}

int	Context::ExecuteSyscall_rt_sigreturn()
{
	return	0;
}

int	Context::ExecuteSyscall_rt_sigaction()
{
	return	0;
}

int	Context::ExecuteSyscall_rt_sigprocmask()
{
	return	0;
}

int	Context::ExecuteSyscall_rt_sigpending()
{
	return	0;
}

int	Context::ExecuteSyscall_rt_sigtimedwait()
{
	return	0;
}

int	Context::ExecuteSyscall_rt_sigqueueinfo()
{
	return	0;
}

int	Context::ExecuteSyscall_rt_sigsuspend()
{
	return	0;
}

int	Context::ExecuteSyscall_pread64()
{
	return	0;
}

int	Context::ExecuteSyscall_pwrite64()
{
	return	0;
}

int	Context::ExecuteSyscall_chown()
{
	return	0;
}

int	Context::ExecuteSyscall_getcwd()
{
	return	0;
}

int	Context::ExecuteSyscall_capget()
{
	return	0;
}

int	Context::ExecuteSyscall_capset()
{
	return	0;
}

int	Context::ExecuteSyscall_sigaltstack()
{
	return	0;
}

int	Context::ExecuteSyscall_sendfile()
{
	return	0;
}

int	Context::ExecuteSyscall_getpmsg()
{
	return	0;
}

int	Context::ExecuteSyscall_putpmsg()
{
	return	0;
}

int	Context::ExecuteSyscall_mmap2()
{
	return	0;
}

int	Context::ExecuteSyscall_truncate64()
{
	return	0;
}

int	Context::ExecuteSyscall_ftruncate64()
{
	return	0;
}

int	Context::ExecuteSyscall_stat64()
{
	return	0;
}

int	Context::ExecuteSyscall_lstat64()
{
	return	0;
}

int	Context::ExecuteSyscall_fstat64()
{
	return	0;
}

int	Context::ExecuteSyscall_pivot_root()
{
	return	0;
}

int	Context::ExecuteSyscall_mincore()
{
	return	0;
}

int	Context::ExecuteSyscall_madvise()
{
	return	0;
}

int	Context::ExecuteSyscall_getdents64()
{
	return	0;
}

int	Context::ExecuteSyscall_fcntl64()
{
	return	0;
}

int	Context::ExecuteSyscall_reserved221()
{
	return	0;
}

int	Context::ExecuteSyscall_gettid()
{
	return	0;
}

int	Context::ExecuteSyscall_readahead()
{
	return	0;
}

int	Context::ExecuteSyscall_setxattr()
{
	return	0;
}

int	Context::ExecuteSyscall_lsetxattr()
{
	return	0;
}

int	Context::ExecuteSyscall_fsetxattr()
{
	return	0;
}

int	Context::ExecuteSyscall_getxattr()
{
	return	0;
}

int	Context::ExecuteSyscall_lgetxattr()
{
	return	0;
}

int	Context::ExecuteSyscall_fgetxattr()
{
	return	0;
}

int	Context::ExecuteSyscall_listxattr()
{
	return	0;
}

int	Context::ExecuteSyscall_llistxattr()
{
	return	0;
}

int	Context::ExecuteSyscall_flistxattr()
{
	return	0;
}

int	Context::ExecuteSyscall_removexattr()
{
	return	0;
}

int	Context::ExecuteSyscall_lremovexattr()
{
	return	0;
}

int	Context::ExecuteSyscall_fremovexattr()
{
	return	0;
}

int	Context::ExecuteSyscall_tkill()
{
	return	0;
}

int	Context::ExecuteSyscall_sendfile64()
{
	return	0;
}

int	Context::ExecuteSyscall_futex()
{
	return	0;
}

int	Context::ExecuteSyscall_sched_setaffinity()
{
	return	0;
}

int	Context::ExecuteSyscall_sched_getaffinity()
{
	return	0;
}

int	Context::ExecuteSyscall_io_setup()
{
	return	0;
}

int	Context::ExecuteSyscall_io_destroy()
{
	return	0;
}

int	Context::ExecuteSyscall_io_getevents()
{
	return	0;
}

int	Context::ExecuteSyscall_io_submit()
{
	return	0;
}

int	Context::ExecuteSyscall_io_cancel()
{
	return	0;
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

int	Context::ExecuteSyscall_lookup_dcookie()
{
	return	0;
}

int	Context::ExecuteSyscall_epoll_create()
{
	return	0;
}

int	Context::ExecuteSyscall_epoll_ctl()
{
	return	0;
}

int	Context::ExecuteSyscall_epoll_wait()
{
	return	0;
}

int	Context::ExecuteSyscall_remap_file_pages()
{
	return	0;
}

int	Context::ExecuteSyscall_set_tid_address()
{
	return	0;
}

int	Context::ExecuteSyscall_restart_syscall()
{
	return	0;
}

int	Context::ExecuteSyscall_fadvise64()
{
	return	0;
}

int	Context::ExecuteSyscall_statfs64()
{
	return	0;
}

int	Context::ExecuteSyscall_fstatfs64()
{
	return	0;
}

int	Context::ExecuteSyscall_timer_create()
{
	return	0;
}

int	Context::ExecuteSyscall_timer_settime()
{
	return	0;
}

int	Context::ExecuteSyscall_timer_gettime()
{
	return	0;
}

int	Context::ExecuteSyscall_timer_getoverrun()
{
	return	0;
}

int	Context::ExecuteSyscall_timer_delete()
{
	return	0;
}

int	Context::ExecuteSyscall_clock_settime()
{
	return	0;
}

int	Context::ExecuteSyscall_clock_gettime()
{
	return	0;
}

int	Context::ExecuteSyscall_clock_getres()
{
	return	0;
}

int	Context::ExecuteSyscall_clock_nanosleep()
{
	return	0;
}

int	Context::ExecuteSyscall_tgkill()
{
	return	0;
}

int	Context::ExecuteSyscall_utimes()
{
	return	0;
}

int	Context::ExecuteSyscall_mbind()
{
	return	0;
}

int	Context::ExecuteSyscall_get_mempolicy()
{
	return	0;
}

int	Context::ExecuteSyscall_set_mempolicy()
{
	return	0;
}

int	Context::ExecuteSyscall_mq_open()
{
	return	0;
}

int	Context::ExecuteSyscall_mq_unlink()
{
	return	0;
}

int	Context::ExecuteSyscall_mq_timedsend()
{
	return	0;
}

int	Context::ExecuteSyscall_mq_timedreceive()
{
	return	0;
}

int	Context::ExecuteSyscall_mq_notify()
{
	return	0;
}

int	Context::ExecuteSyscall_mq_getsetattr()
{
	return	0;
}

int	Context::ExecuteSyscall_vserver()
{
	return	0;
}

int	Context::ExecuteSyscall_waitid()
{
	return	0;
}

int	Context::ExecuteSyscall_sys_setaltroot()
{
	return	0;
}

int	Context::ExecuteSyscall_add_key()
{
	return	0;
}

int	Context::ExecuteSyscall_request_key()
{
	return	0;
}

int	Context::ExecuteSyscall_keyctl()
{
	return	0;
}

int	Context::ExecuteSyscall_set_thread_area()
{
	return	0;
}

int	Context::ExecuteSyscall_inotify_init()
{
	return	0;
}

int	Context::ExecuteSyscall_inotify_add_watch()
{
	return	0;
}

int	Context::ExecuteSyscall_inotify_rm_watch()
{
	return	0;
}

int	Context::ExecuteSyscall_migrate_pages()
{
	return	0;
}

int	Context::ExecuteSyscall_openat()
{
	return	0;
}

int	Context::ExecuteSyscall_mkdirat()
{
	return	0;
}

int	Context::ExecuteSyscall_mknodat()
{
	return	0;
}

int	Context::ExecuteSyscall_fchownat()
{
	return	0;
}

int	Context::ExecuteSyscall_futimesat()
{
	return	0;
}

int	Context::ExecuteSyscall_fstatat64()
{
	return	0;
}

int	Context::ExecuteSyscall_unlinkat()
{
	return	0;
}

int	Context::ExecuteSyscall_renameat()
{
	return	0;
}

int	Context::ExecuteSyscall_linkat()
{
	return	0;
}

int	Context::ExecuteSyscall_symlinkat()
{
	return	0;
}

int	Context::ExecuteSyscall_readlinkat()
{
	return	0;
}

int	Context::ExecuteSyscall_fchmodat()
{
	return	0;
}

int	Context::ExecuteSyscall_faccessat()
{
	return	0;
}

int	Context::ExecuteSyscall_pselect6()
{
	return	0;
}

int	Context::ExecuteSyscall_ppoll()
{
	return	0;
}

int	Context::ExecuteSyscall_unshare()
{
	return	0;
}

int	Context::ExecuteSyscall_splice()
{
	return	0;
}

int	Context::ExecuteSyscall_sync_file_range()
{
	return	0;
}

int	Context::ExecuteSyscall_tee()
{
	return	0;
}

int	Context::ExecuteSyscall_vmsplice()
{
	return	0;
}

int	Context::ExecuteSyscall_move_pages()
{
	return	0;
}

int	Context::ExecuteSyscall_set_robust_list()
{
	return	0;
}

int	Context::ExecuteSyscall_get_robust_list()
{
	return	0;
}

int	Context::ExecuteSyscall_kexec_load()
{
	return	0;
}

int	Context::ExecuteSyscall_getcpu()
{
	return	0;
}

int	Context::ExecuteSyscall_epoll_pwait()
{
	return	0;
}

int	Context::ExecuteSyscall_ioprio_set()
{
	return	0;
}

int	Context::ExecuteSyscall_ioprio_get()
{
	return	0;
}

int	Context::ExecuteSyscall_utimensat()
{
	return	0;
}

int	Context::ExecuteSyscall_signalfd()
{
	return	0;
}

int	Context::ExecuteSyscall_timerfd()
{
	return	0;
}

int	Context::ExecuteSyscall_eventfd()
{
	return	0;
}

int	Context::ExecuteSyscall_fallocate()
{
	return	0;
}

int	Context::ExecuteSyscall_timerfd_create()
{
	return	0;
}

int	Context::ExecuteSyscall_timerfd_gettime()
{
	return	0;
}

int	Context::ExecuteSyscall_timerfd_settime()
{
	return	0;
}

int	Context::ExecuteSyscall_signalfd4()
{
	return	0;
}

int	Context::ExecuteSyscall_eventfd2()
{
	return	0;
}

int	Context::ExecuteSyscall_epoll_create1()
{
	return	0;
}

int	Context::ExecuteSyscall_dup3()
{
	return	0;
}

int	Context::ExecuteSyscall_pipe2()
{
	return	0;
}

int	Context::ExecuteSyscall_inotify_init1()
{
	return	0;
}

int	Context::ExecuteSyscall_preadv()
{
	return	0;
}

int	Context::ExecuteSyscall_pwritev()
{
	return	0;
}

int	Context::ExecuteSyscall_rt_tgsigqueueinfo()
{
	return	0;
}

int	Context::ExecuteSyscall_perf_event_open()
{
	return	0;
}

int	Context::ExecuteSyscall_accept4()
{
	return	0;
}

}











