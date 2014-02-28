/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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
#include <sched.h>
#include <string.h>
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

#include <lib/cpp/Misc.h>

#include "Context.h"
#include "Emu.h"

	
namespace x86
{

#define __UNIMPLEMENTED__ misc::fatal("%s: unimplemented system call.\n\n%s", \
		__FUNCTION__ + 15, syscall_error_note); return 0;

static const char *syscall_error_note =
	"The most common Linux system calls are currently supported by "
	"Multi2Sim, but your application might perform specific unsupported "
	"system calls or unsupported combinations of its arguments. To request "
	"support for a given system call, please report a bug in "
	"www.multi2sim.org.";


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



//
// System call error codes
//

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
static misc::StringMap syscall_error_map =
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
};



//
// Structures for system call 'clone' and 'set_thread_area'
//

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

static struct misc::StringMap syscall_clone_flags_map =
{
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
};

static const unsigned int syscall_clone_supported_flags =
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




//
// Main function
//

void Context::ExecuteSyscall()
{
	// Get system call code from register eax
	int code = regs.getEax();

	// Check for special system call codes outside of the standard range
	// defined in 'syscall.dat'.
	// FIXME
	#if 0
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

	// Statistics
	x86_sys_call_freq[code]++;
	#endif

	// Debug
	emu->call_debug << misc::fmt("system call '%s' "
			"(code %d, inst %lld, pid %d)\n",
			Context::syscall_name[code], code,
			emu->getInstructions(), pid);
	emu->syscall_debug << misc::fmt("system call '%s' "
			"(code %d, inst %lld, pid %d)\n",
			Context::syscall_name[code], code,
			emu->getInstructions(), pid);

	// Perform system call
	ExecuteSyscallFn fn = execute_syscall_fn[code];
	int ret = (this->*fn)();

	// Set return value in 'eax', except for 'sigreturn' system call. Also, if the
	// context got suspended, the wake up routine will set the return value.
	if (code != SyscallCode_sigreturn && !getState(ContextSuspended))
		regs.setEax(ret);

	// Debug
	emu->syscall_debug << misc::fmt("  ret = (%d, 0x%x)", ret, ret);
	if (ret < 0 && ret >= -SIM_ERRNO_MAX)
		emu->syscall_debug << misc::fmt(", errno = %s)",
				syscall_error_map.MapValue(-ret));
	emu->syscall_debug << '\n';
}




//
// System call 'restart_syscall'
//

int Context::ExecuteSyscall_restart_syscall()
{
	__UNIMPLEMENTED__
}




//
// System call 'exit'
//

int Context::ExecuteSyscall_exit()
{
	__UNIMPLEMENTED__
}




//
// System call 'fork'
//

int Context::ExecuteSyscall_fork()
{
	__UNIMPLEMENTED__
}




//
// System call 'read'
//

int Context::ExecuteSyscall_read()
{
	__UNIMPLEMENTED__
}




//
// System call 'write'
//

int Context::ExecuteSyscall_write()
{
	// Arguments
	int guest_fd = regs.getEbx();
	unsigned buf_ptr = regs.getEcx();
	unsigned count = regs.getEdx();
	emu->syscall_debug << misc::fmt("  guest_fd=%d, buf_ptr=0x%x, count=0x%x\n",
			guest_fd, buf_ptr, count);

	// Get file descriptor
	FileDesc *desc = file_table.getFileDesc(guest_fd);
	if (!desc)
		return -EBADF;
	int host_fd = desc->getHostIndex();
	emu->syscall_debug << misc::fmt("  host_fd=%d\n", host_fd);

	// Read buffer from memory
	char *buf = new char[count];
	memory->Read(buf_ptr, count, buf);
	emu->syscall_debug << "  buf=\""
			<< misc::StringBinaryBuffer(buf, count, 40)
			<< "\"\n";

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
	emu->syscall_debug << misc::fmt("  blocking write - process suspended\n");
	wakeup_fd = guest_fd;
	setState(ContextSuspended);
	setState(ContextWrite);
	emu->ProcessEventsSchedule();

	// Return value doesn't matter here. It will be overwritten when the
	// context wakes up after blocking call.
	delete buf;
	return 0;
}




//
// System call 'open'
//

int Context::ExecuteSyscall_open()
{
	__UNIMPLEMENTED__
}




//
// System call 'close'
//

int Context::ExecuteSyscall_close()
{
	__UNIMPLEMENTED__
}




//
// System call 'waitpid'
//

int Context::ExecuteSyscall_waitpid()
{
	__UNIMPLEMENTED__
}




//
// System call 'creat'
//

int Context::ExecuteSyscall_creat()
{
	__UNIMPLEMENTED__
}




//
// System call 'link'
//

int Context::ExecuteSyscall_link()
{
	__UNIMPLEMENTED__
}




//
// System call 'unlink'
//

int Context::ExecuteSyscall_unlink()
{
	__UNIMPLEMENTED__
}




//
// System call 'execve'
//

int Context::ExecuteSyscall_execve()
{
	__UNIMPLEMENTED__
}




//
// System call 'chdir'
//

int Context::ExecuteSyscall_chdir()
{
	__UNIMPLEMENTED__
}




//
// System call 'time'
//

int Context::ExecuteSyscall_time()
{
	__UNIMPLEMENTED__
}




//
// System call 'mknod'
//

int Context::ExecuteSyscall_mknod()
{
	__UNIMPLEMENTED__
}




//
// System call 'chmod'
//

int Context::ExecuteSyscall_chmod()
{
	__UNIMPLEMENTED__
}




//
// System call 'lchown16'
//

int Context::ExecuteSyscall_lchown16()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_17'
//

int Context::ExecuteSyscall_ni_syscall_17()
{
	__UNIMPLEMENTED__
}




//
// System call 'stat'
//

int Context::ExecuteSyscall_stat()
{
	__UNIMPLEMENTED__
}




//
// System call 'lseek'
//

int Context::ExecuteSyscall_lseek()
{
	__UNIMPLEMENTED__
}




//
// System call 'getpid'
//

int Context::ExecuteSyscall_getpid()
{
	__UNIMPLEMENTED__
}




//
// System call 'mount'
//

int Context::ExecuteSyscall_mount()
{
	__UNIMPLEMENTED__
}




//
// System call 'oldumount'
//

int Context::ExecuteSyscall_oldumount()
{
	__UNIMPLEMENTED__
}




//
// System call 'setuid16'
//

int Context::ExecuteSyscall_setuid16()
{
	__UNIMPLEMENTED__
}




//
// System call 'getuid16'
//

int Context::ExecuteSyscall_getuid16()
{
	__UNIMPLEMENTED__
}




//
// System call 'stime'
//

int Context::ExecuteSyscall_stime()
{
	__UNIMPLEMENTED__
}




//
// System call 'ptrace'
//

int Context::ExecuteSyscall_ptrace()
{
	__UNIMPLEMENTED__
}




//
// System call 'alarm'
//

int Context::ExecuteSyscall_alarm()
{
	__UNIMPLEMENTED__
}




//
// System call 'fstat'
//

int Context::ExecuteSyscall_fstat()
{
	__UNIMPLEMENTED__
}




//
// System call 'pause'
//

int Context::ExecuteSyscall_pause()
{
	__UNIMPLEMENTED__
}




//
// System call 'utime'
//

int Context::ExecuteSyscall_utime()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_31'
//

int Context::ExecuteSyscall_ni_syscall_31()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_32'
//

int Context::ExecuteSyscall_ni_syscall_32()
{
	__UNIMPLEMENTED__
}




//
// System call 'access'
//

int Context::ExecuteSyscall_access()
{
	__UNIMPLEMENTED__
}




//
// System call 'nice'
//

int Context::ExecuteSyscall_nice()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_35'
//

int Context::ExecuteSyscall_ni_syscall_35()
{
	__UNIMPLEMENTED__
}




//
// System call 'sync'
//

int Context::ExecuteSyscall_sync()
{
	__UNIMPLEMENTED__
}




//
// System call 'kill'
//

int Context::ExecuteSyscall_kill()
{
	__UNIMPLEMENTED__
}




//
// System call 'rename'
//

int Context::ExecuteSyscall_rename()
{
	__UNIMPLEMENTED__
}




//
// System call 'mkdir'
//

int Context::ExecuteSyscall_mkdir()
{
	__UNIMPLEMENTED__
}




//
// System call 'rmdir'
//

int Context::ExecuteSyscall_rmdir()
{
	__UNIMPLEMENTED__
}




//
// System call 'dup'
//

int Context::ExecuteSyscall_dup()
{
	__UNIMPLEMENTED__
}




//
// System call 'pipe'
//

int Context::ExecuteSyscall_pipe()
{
	__UNIMPLEMENTED__
}




//
// System call 'times'
//

int Context::ExecuteSyscall_times()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_44'
//

int Context::ExecuteSyscall_ni_syscall_44()
{
	__UNIMPLEMENTED__
}




//
// System call 'brk'
//

int Context::ExecuteSyscall_brk()
{
	// Arguments
	unsigned new_heap_break = regs.getEbx();
	unsigned old_heap_break = memory->getHeapBreak();
	emu->syscall_debug << misc::fmt("  newbrk = 0x%x (previous brk was 0x%x)\n",
			new_heap_break, old_heap_break);

	// Align
	unsigned new_heap_break_aligned = misc::RoundUp(new_heap_break, mem::MemoryPageSize);
	unsigned old_heap_break_aligned = misc::RoundUp(old_heap_break, mem::MemoryPageSize);

	// If argument is zero, the system call is used to obtain the current
	// top of the heap
	if (!new_heap_break)
		return old_heap_break;

	// If the heap is increased: if some page in the way is allocated, do
	// nothing and return old heap top. Otherwise, allocate pages and return
	// new heap top.
	if (new_heap_break > old_heap_break)
	{
		unsigned size = new_heap_break_aligned - old_heap_break_aligned;
		if (size)
		{
			if (memory->MapSpace(old_heap_break_aligned, size) !=
					old_heap_break_aligned)
				misc::fatal("%s: out of memory", __FUNCTION__);
			memory->Map(old_heap_break_aligned, size,
					mem::MemoryAccessRead | mem::MemoryAccessWrite);
		}
		memory->setHeapBreak(new_heap_break);
		emu->syscall_debug << misc::fmt("  heap grows %u bytes\n",
				new_heap_break - old_heap_break);
		return new_heap_break;
	}

	/* Always allow to shrink the heap. */
	if (new_heap_break < old_heap_break)
	{
		unsigned size = old_heap_break_aligned - new_heap_break_aligned;
		if (size)
			memory->Unmap(new_heap_break_aligned, size);
		memory->setHeapBreak(new_heap_break);
		emu->syscall_debug << misc::fmt("  heap shrinks %u bytes\n",
				old_heap_break - new_heap_break);
		return new_heap_break;
	}

	// Heap stays the same
	return 0;
}




//
// System call 'setgid16'
//

int Context::ExecuteSyscall_setgid16()
{
	__UNIMPLEMENTED__
}




//
// System call 'getgid16'
//

int Context::ExecuteSyscall_getgid16()
{
	__UNIMPLEMENTED__
}




//
// System call 'signal'
//

int Context::ExecuteSyscall_signal()
{
	__UNIMPLEMENTED__
}




//
// System call 'geteuid16'
//

int Context::ExecuteSyscall_geteuid16()
{
	__UNIMPLEMENTED__
}




//
// System call 'getegid16'
//

int Context::ExecuteSyscall_getegid16()
{
	__UNIMPLEMENTED__
}




//
// System call 'acct'
//

int Context::ExecuteSyscall_acct()
{
	__UNIMPLEMENTED__
}




//
// System call 'umount'
//

int Context::ExecuteSyscall_umount()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_53'
//

int Context::ExecuteSyscall_ni_syscall_53()
{
	__UNIMPLEMENTED__
}




//
// System call 'ioctl'
//

int Context::ExecuteSyscall_ioctl()
{
	__UNIMPLEMENTED__
}




//
// System call 'fcntl'
//

int Context::ExecuteSyscall_fcntl()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_56'
//

int Context::ExecuteSyscall_ni_syscall_56()
{
	__UNIMPLEMENTED__
}




//
// System call 'setpgid'
//

int Context::ExecuteSyscall_setpgid()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_58'
//

int Context::ExecuteSyscall_ni_syscall_58()
{
	__UNIMPLEMENTED__
}




//
// System call 'olduname'
//

int Context::ExecuteSyscall_olduname()
{
	__UNIMPLEMENTED__
}




//
// System call 'umask'
//

int Context::ExecuteSyscall_umask()
{
	__UNIMPLEMENTED__
}




//
// System call 'chroot'
//

int Context::ExecuteSyscall_chroot()
{
	__UNIMPLEMENTED__
}




//
// System call 'ustat'
//

int Context::ExecuteSyscall_ustat()
{
	__UNIMPLEMENTED__
}




//
// System call 'dup2'
//

int Context::ExecuteSyscall_dup2()
{
	__UNIMPLEMENTED__
}




//
// System call 'getppid'
//

int Context::ExecuteSyscall_getppid()
{
	__UNIMPLEMENTED__
}




//
// System call 'getpgrp'
//

int Context::ExecuteSyscall_getpgrp()
{
	__UNIMPLEMENTED__
}




//
// System call 'setsid'
//

int Context::ExecuteSyscall_setsid()
{
	__UNIMPLEMENTED__
}




//
// System call 'sigaction'
//

int Context::ExecuteSyscall_sigaction()
{
	__UNIMPLEMENTED__
}




//
// System call 'sgetmask'
//

int Context::ExecuteSyscall_sgetmask()
{
	__UNIMPLEMENTED__
}




//
// System call 'ssetmask'
//

int Context::ExecuteSyscall_ssetmask()
{
	__UNIMPLEMENTED__
}




//
// System call 'setreuid16'
//

int Context::ExecuteSyscall_setreuid16()
{
	__UNIMPLEMENTED__
}




//
// System call 'setregid16'
//

int Context::ExecuteSyscall_setregid16()
{
	__UNIMPLEMENTED__
}




//
// System call 'sigsuspend'
//

int Context::ExecuteSyscall_sigsuspend()
{
	__UNIMPLEMENTED__
}




//
// System call 'sigpending'
//

int Context::ExecuteSyscall_sigpending()
{
	__UNIMPLEMENTED__
}




//
// System call 'sethostname'
//

int Context::ExecuteSyscall_sethostname()
{
	__UNIMPLEMENTED__
}




//
// System call 'setrlimit'
//

int Context::ExecuteSyscall_setrlimit()
{
	__UNIMPLEMENTED__
}




//
// System call 'old_getrlimit'
//

int Context::ExecuteSyscall_old_getrlimit()
{
	__UNIMPLEMENTED__
}




//
// System call 'getrusage'
//

int Context::ExecuteSyscall_getrusage()
{
	__UNIMPLEMENTED__
}




//
// System call 'gettimeofday'
//

int Context::ExecuteSyscall_gettimeofday()
{
	__UNIMPLEMENTED__
}




//
// System call 'settimeofday'
//

int Context::ExecuteSyscall_settimeofday()
{
	__UNIMPLEMENTED__
}




//
// System call 'getgroups16'
//

int Context::ExecuteSyscall_getgroups16()
{
	__UNIMPLEMENTED__
}




//
// System call 'setgroups16'
//

int Context::ExecuteSyscall_setgroups16()
{
	__UNIMPLEMENTED__
}




//
// System call 'oldselect'
//

int Context::ExecuteSyscall_oldselect()
{
	__UNIMPLEMENTED__
}




//
// System call 'symlink'
//

int Context::ExecuteSyscall_symlink()
{
	__UNIMPLEMENTED__
}




//
// System call 'lstat'
//

int Context::ExecuteSyscall_lstat()
{
	__UNIMPLEMENTED__
}




//
// System call 'readlink'
//

int Context::ExecuteSyscall_readlink()
{
	__UNIMPLEMENTED__
}




//
// System call 'uselib'
//

int Context::ExecuteSyscall_uselib()
{
	__UNIMPLEMENTED__
}




//
// System call 'swapon'
//

int Context::ExecuteSyscall_swapon()
{
	__UNIMPLEMENTED__
}




//
// System call 'reboot'
//

int Context::ExecuteSyscall_reboot()
{
	__UNIMPLEMENTED__
}




//
// System call 'readdir'
//

int Context::ExecuteSyscall_readdir()
{
	__UNIMPLEMENTED__
}




//
// System call 'mmap'
//

static const unsigned mmap_base_address = 0xb7fb0000;

static const misc::StringMap mmap_prot_map =
{
	{ "PROT_READ",       0x1 },
	{ "PROT_WRITE",      0x2 },
	{ "PROT_EXEC",       0x4 },
	{ "PROT_SEM",        0x8 },
	{ "PROT_GROWSDOWN",  0x01000000 },
	{ "PROT_GROWSUP",    0x02000000 }
};

static const misc::StringMap mmap_flags_map =
{
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
};

int Context::SyscallMmapAux(unsigned int addr, unsigned int len,
		int prot, int flags, int guest_fd, int offset)
{
	// Check that protection flags match in guest and host
	assert(PROT_READ == 1);
	assert(PROT_WRITE == 2);
	assert(PROT_EXEC == 4);

	// Check that mapping flags match
	assert(MAP_SHARED == 0x01);
	assert(MAP_PRIVATE == 0x02);
	assert(MAP_FIXED == 0x10);
	assert(MAP_ANONYMOUS == 0x20);

	// Translate file descriptor
	FileDesc *desc = file_table.getFileDesc(guest_fd);
	int host_fd = desc ? desc->getHostIndex() : -1;
	if (guest_fd > 0 && host_fd < 0)
		misc::fatal("%s: invalid guest descriptor", __FUNCTION__);

	// Permissions
	int perm = mem::MemoryAccessInit;
	perm |= prot & PROT_READ ? mem::MemoryAccessRead : 0;
	perm |= prot & PROT_WRITE ? mem::MemoryAccessWrite : 0;
	perm |= prot & PROT_EXEC ? mem::MemoryAccessExec : 0;

	// Flag MAP_ANONYMOUS.
	// If it is set, the 'fd' parameter is ignored.
	if (flags & MAP_ANONYMOUS)
		host_fd = -1;

	// 'addr' and 'offset' must be aligned to page size boundaries.
	// 'len' is rounded up to page boundary.
	if (offset & ~mem::MemoryPageMask)
		misc::fatal("%s: unaligned offset", __FUNCTION__);
	if (addr & ~mem::MemoryPageMask)
		misc::fatal("%s: unaligned address", __FUNCTION__);
	unsigned len_aligned = misc::RoundUp(len, mem::MemoryPageSize);

	// Find region for allocation
	if (flags & MAP_FIXED)
	{
		// If MAP_FIXED is set, the 'addr' parameter must be obeyed, and
		// is not just a hint for a possible base address of the
		// allocated range.
		if (!addr)
			misc::fatal("%s: no start specified for fixed mapping",
					__FUNCTION__);

		// Any allocated page in the range specified by 'addr' and 'len'
		// must be discarded.
		memory->Unmap(addr, len_aligned);
	}
	else
	{
		if (!addr || memory->MapSpaceDown(addr, len_aligned) != addr)
			addr = mmap_base_address;
		addr = memory->MapSpaceDown(addr, len_aligned);
		if (addr == (unsigned) -1)
			misc::fatal("%s: out of guest memory", __FUNCTION__);
	}

	// Allocation of memory
	memory->Map(addr, len_aligned, perm);

	// Host mapping
	if (host_fd >= 0)
	{
		// Save previous position
		unsigned last_pos = lseek(host_fd, 0, SEEK_CUR);
		lseek(host_fd, offset, SEEK_SET);

		// Read pages
		assert(len_aligned % mem::MemoryPageSize == 0);
		assert(addr % mem::MemoryPageSize == 0);
		unsigned curr_addr = addr;
		for (int size = len_aligned; size > 0; size -= mem::MemoryPageSize)
		{
			char buf[mem::MemoryPageSize];
			memset(buf, 0, mem::MemoryPageSize);
			int count = read(host_fd, buf, mem::MemoryPageSize);
			if (count)
				memory->Access(curr_addr, mem::MemoryPageSize,
						buf, mem::MemoryAccessInit);
			curr_addr += mem::MemoryPageSize;
		}

		// Return file to last position
		lseek(host_fd, last_pos, SEEK_SET);
	}

	// Return mapped address
	return addr;
}


int Context::ExecuteSyscall_mmap()
{
	__UNIMPLEMENTED__
}




//
// System call 'munmap'
//

int Context::ExecuteSyscall_munmap()
{
	__UNIMPLEMENTED__
}




//
// System call 'truncate'
//

int Context::ExecuteSyscall_truncate()
{
	__UNIMPLEMENTED__
}




//
// System call 'ftruncate'
//

int Context::ExecuteSyscall_ftruncate()
{
	__UNIMPLEMENTED__
}




//
// System call 'fchmod'
//

int Context::ExecuteSyscall_fchmod()
{
	__UNIMPLEMENTED__
}




//
// System call 'fchown16'
//

int Context::ExecuteSyscall_fchown16()
{
	__UNIMPLEMENTED__
}




//
// System call 'getpriority'
//

int Context::ExecuteSyscall_getpriority()
{
	__UNIMPLEMENTED__
}




//
// System call 'setpriority'
//

int Context::ExecuteSyscall_setpriority()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_98'
//

int Context::ExecuteSyscall_ni_syscall_98()
{
	__UNIMPLEMENTED__
}




//
// System call 'statfs'
//

int Context::ExecuteSyscall_statfs()
{
	__UNIMPLEMENTED__
}




//
// System call 'fstatfs'
//

int Context::ExecuteSyscall_fstatfs()
{
	__UNIMPLEMENTED__
}




//
// System call 'ioperm'
//

int Context::ExecuteSyscall_ioperm()
{
	__UNIMPLEMENTED__
}




//
// System call 'socketcall'
//

int Context::ExecuteSyscall_socketcall()
{
	__UNIMPLEMENTED__
}




//
// System call 'syslog'
//

int Context::ExecuteSyscall_syslog()
{
	__UNIMPLEMENTED__
}




//
// System call 'setitimer'
//

int Context::ExecuteSyscall_setitimer()
{
	__UNIMPLEMENTED__
}




//
// System call 'getitimer'
//

int Context::ExecuteSyscall_getitimer()
{
	__UNIMPLEMENTED__
}




//
// System call 'newstat'
//

int Context::ExecuteSyscall_newstat()
{
	__UNIMPLEMENTED__
}




//
// System call 'newlstat'
//

int Context::ExecuteSyscall_newlstat()
{
	__UNIMPLEMENTED__
}




//
// System call 'newfstat'
//

int Context::ExecuteSyscall_newfstat()
{
	__UNIMPLEMENTED__
}




//
// System call 'uname'
//

int Context::ExecuteSyscall_uname()
{
	__UNIMPLEMENTED__
}




//
// System call 'iopl'
//

int Context::ExecuteSyscall_iopl()
{
	__UNIMPLEMENTED__
}




//
// System call 'vhangup'
//

int Context::ExecuteSyscall_vhangup()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_112'
//

int Context::ExecuteSyscall_ni_syscall_112()
{
	__UNIMPLEMENTED__
}




//
// System call 'vm86old'
//

int Context::ExecuteSyscall_vm86old()
{
	__UNIMPLEMENTED__
}




//
// System call 'wait4'
//

int Context::ExecuteSyscall_wait4()
{
	__UNIMPLEMENTED__
}




//
// System call 'swapoff'
//

int Context::ExecuteSyscall_swapoff()
{
	__UNIMPLEMENTED__
}




//
// System call 'sysinfo'
//

int Context::ExecuteSyscall_sysinfo()
{
	__UNIMPLEMENTED__
}




//
// System call 'ipc'
//

int Context::ExecuteSyscall_ipc()
{
	__UNIMPLEMENTED__
}




//
// System call 'fsync'
//

int Context::ExecuteSyscall_fsync()
{
	__UNIMPLEMENTED__
}




//
// System call 'sigreturn'
//

int Context::ExecuteSyscall_sigreturn()
{
	__UNIMPLEMENTED__
}




//
// System call 'clone'
//

int Context::ExecuteSyscall_clone()
{
	__UNIMPLEMENTED__
}




//
// System call 'setdomainname'
//

int Context::ExecuteSyscall_setdomainname()
{
	__UNIMPLEMENTED__
}




//
// System call 'newuname'
//

struct simUtsName
{
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
	char domainname[65];
} __attribute__((packed));

static simUtsName sim_utsname =
{
	"Linux",
	"Multi2Sim",
	"3.1.9-1.fc16.i686"
	"#1 Fri Jan 13 16:37:42 UTC 2012",
	"i686"
	""
};

int Context::ExecuteSyscall_newuname()
{
	// Arguments
	unsigned utsname_ptr = regs.getEbx();
	emu->syscall_debug << misc::fmt("  putsname=0x%x\n", utsname_ptr);
	emu->syscall_debug << misc::fmt("  sysname='%s', nodename='%s'\n",
			sim_utsname.sysname, sim_utsname.nodename);
	emu->syscall_debug << misc::fmt("  relaese='%s', version='%s'\n",
			sim_utsname.release, sim_utsname.version);
	emu->syscall_debug << misc::fmt("  machine='%s', domainname='%s'\n",
			sim_utsname.machine, sim_utsname.domainname);

	// Return structure
	memory->Write(utsname_ptr, sizeof sim_utsname, (char *) &sim_utsname);
	return 0;
}




//
// System call 'modify_ldt'
//

int Context::ExecuteSyscall_modify_ldt()
{
	__UNIMPLEMENTED__
}




//
// System call 'adjtimex'
//

int Context::ExecuteSyscall_adjtimex()
{
	__UNIMPLEMENTED__
}




//
// System call 'mprotect'
//

int Context::ExecuteSyscall_mprotect()
{
	__UNIMPLEMENTED__
}




//
// System call 'sigprocmask'
//

int Context::ExecuteSyscall_sigprocmask()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_127'
//

int Context::ExecuteSyscall_ni_syscall_127()
{
	__UNIMPLEMENTED__
}




//
// System call 'init_module'
//

int Context::ExecuteSyscall_init_module()
{
	__UNIMPLEMENTED__
}




//
// System call 'delete_module'
//

int Context::ExecuteSyscall_delete_module()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_130'
//

int Context::ExecuteSyscall_ni_syscall_130()
{
	__UNIMPLEMENTED__
}




//
// System call 'quotactl'
//

int Context::ExecuteSyscall_quotactl()
{
	__UNIMPLEMENTED__
}




//
// System call 'getpgid'
//

int Context::ExecuteSyscall_getpgid()
{
	__UNIMPLEMENTED__
}




//
// System call 'fchdir'
//

int Context::ExecuteSyscall_fchdir()
{
	__UNIMPLEMENTED__
}




//
// System call 'bdflush'
//

int Context::ExecuteSyscall_bdflush()
{
	__UNIMPLEMENTED__
}




//
// System call 'sysfs'
//

int Context::ExecuteSyscall_sysfs()
{
	__UNIMPLEMENTED__
}




//
// System call 'personality'
//

int Context::ExecuteSyscall_personality()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_137'
//

int Context::ExecuteSyscall_ni_syscall_137()
{
	__UNIMPLEMENTED__
}




//
// System call 'setfsuid16'
//

int Context::ExecuteSyscall_setfsuid16()
{
	__UNIMPLEMENTED__
}




//
// System call 'setfsgid16'
//

int Context::ExecuteSyscall_setfsgid16()
{
	__UNIMPLEMENTED__
}




//
// System call 'llseek'
//

int Context::ExecuteSyscall_llseek()
{
	__UNIMPLEMENTED__
}




//
// System call 'getdents'
//

int Context::ExecuteSyscall_getdents()
{
	__UNIMPLEMENTED__
}




//
// System call 'select'
//

int Context::ExecuteSyscall_select()
{
	__UNIMPLEMENTED__
}




//
// System call 'flock'
//

int Context::ExecuteSyscall_flock()
{
	__UNIMPLEMENTED__
}




//
// System call 'msync'
//

int Context::ExecuteSyscall_msync()
{
	__UNIMPLEMENTED__
}




//
// System call 'readv'
//

int Context::ExecuteSyscall_readv()
{
	__UNIMPLEMENTED__
}




//
// System call 'writev'
//

int Context::ExecuteSyscall_writev()
{
	__UNIMPLEMENTED__
}




//
// System call 'getsid'
//

int Context::ExecuteSyscall_getsid()
{
	__UNIMPLEMENTED__
}




//
// System call 'fdatasync'
//

int Context::ExecuteSyscall_fdatasync()
{
	__UNIMPLEMENTED__
}




//
// System call 'sysctl'
//

int Context::ExecuteSyscall_sysctl()
{
	__UNIMPLEMENTED__
}




//
// System call 'mlock'
//

int Context::ExecuteSyscall_mlock()
{
	__UNIMPLEMENTED__
}




//
// System call 'munlock'
//

int Context::ExecuteSyscall_munlock()
{
	__UNIMPLEMENTED__
}




//
// System call 'mlockall'
//

int Context::ExecuteSyscall_mlockall()
{
	__UNIMPLEMENTED__
}




//
// System call 'munlockall'
//

int Context::ExecuteSyscall_munlockall()
{
	__UNIMPLEMENTED__
}




//
// System call 'sched_setparam'
//

int Context::ExecuteSyscall_sched_setparam()
{
	__UNIMPLEMENTED__
}




//
// System call 'sched_getparam'
//

int Context::ExecuteSyscall_sched_getparam()
{
	__UNIMPLEMENTED__
}




//
// System call 'sched_setscheduler'
//

int Context::ExecuteSyscall_sched_setscheduler()
{
	__UNIMPLEMENTED__
}




//
// System call 'sched_getscheduler'
//

int Context::ExecuteSyscall_sched_getscheduler()
{
	__UNIMPLEMENTED__
}




//
// System call 'sched_yield'
//

int Context::ExecuteSyscall_sched_yield()
{
	__UNIMPLEMENTED__
}




//
// System call 'sched_get_priority_max'
//

int Context::ExecuteSyscall_sched_get_priority_max()
{
	__UNIMPLEMENTED__
}




//
// System call 'sched_get_priority_min'
//

int Context::ExecuteSyscall_sched_get_priority_min()
{
	__UNIMPLEMENTED__
}




//
// System call 'sched_rr_get_interval'
//

int Context::ExecuteSyscall_sched_rr_get_interval()
{
	__UNIMPLEMENTED__
}




//
// System call 'nanosleep'
//

int Context::ExecuteSyscall_nanosleep()
{
	__UNIMPLEMENTED__
}




//
// System call 'mremap'
//

int Context::ExecuteSyscall_mremap()
{
	__UNIMPLEMENTED__
}




//
// System call 'setresuid16'
//

int Context::ExecuteSyscall_setresuid16()
{
	__UNIMPLEMENTED__
}




//
// System call 'getresuid16'
//

int Context::ExecuteSyscall_getresuid16()
{
	__UNIMPLEMENTED__
}




//
// System call 'vm86'
//

int Context::ExecuteSyscall_vm86()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_167'
//

int Context::ExecuteSyscall_ni_syscall_167()
{
	__UNIMPLEMENTED__
}




//
// System call 'poll'
//

int Context::ExecuteSyscall_poll()
{
	__UNIMPLEMENTED__
}




//
// System call 'nfsservctl'
//

int Context::ExecuteSyscall_nfsservctl()
{
	__UNIMPLEMENTED__
}




//
// System call 'setresgid16'
//

int Context::ExecuteSyscall_setresgid16()
{
	__UNIMPLEMENTED__
}




//
// System call 'getresgid16'
//

int Context::ExecuteSyscall_getresgid16()
{
	__UNIMPLEMENTED__
}




//
// System call 'prctl'
//

int Context::ExecuteSyscall_prctl()
{
	__UNIMPLEMENTED__
}




//
// System call 'rt_sigreturn'
//

int Context::ExecuteSyscall_rt_sigreturn()
{
	__UNIMPLEMENTED__
}




//
// System call 'rt_sigaction'
//

int Context::ExecuteSyscall_rt_sigaction()
{
	__UNIMPLEMENTED__
}




//
// System call 'rt_sigprocmask'
//

int Context::ExecuteSyscall_rt_sigprocmask()
{
	__UNIMPLEMENTED__
}




//
// System call 'rt_sigpending'
//

int Context::ExecuteSyscall_rt_sigpending()
{
	__UNIMPLEMENTED__
}




//
// System call 'rt_sigtimedwait'
//

int Context::ExecuteSyscall_rt_sigtimedwait()
{
	__UNIMPLEMENTED__
}




//
// System call 'rt_sigqueueinfo'
//

int Context::ExecuteSyscall_rt_sigqueueinfo()
{
	__UNIMPLEMENTED__
}




//
// System call 'rt_sigsuspend'
//

int Context::ExecuteSyscall_rt_sigsuspend()
{
	__UNIMPLEMENTED__
}




//
// System call 'pread64'
//

int Context::ExecuteSyscall_pread64()
{
	__UNIMPLEMENTED__
}




//
// System call 'pwrite64'
//

int Context::ExecuteSyscall_pwrite64()
{
	__UNIMPLEMENTED__
}




//
// System call 'chown16'
//

int Context::ExecuteSyscall_chown16()
{
	__UNIMPLEMENTED__
}




//
// System call 'getcwd'
//

int Context::ExecuteSyscall_getcwd()
{
	__UNIMPLEMENTED__
}




//
// System call 'capget'
//

int Context::ExecuteSyscall_capget()
{
	__UNIMPLEMENTED__
}




//
// System call 'capset'
//

int Context::ExecuteSyscall_capset()
{
	__UNIMPLEMENTED__
}




//
// System call 'sigaltstack'
//

int Context::ExecuteSyscall_sigaltstack()
{
	__UNIMPLEMENTED__
}




//
// System call 'sendfile'
//

int Context::ExecuteSyscall_sendfile()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_188'
//

int Context::ExecuteSyscall_ni_syscall_188()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_189'
//

int Context::ExecuteSyscall_ni_syscall_189()
{
	__UNIMPLEMENTED__
}




//
// System call 'vfork'
//

int Context::ExecuteSyscall_vfork()
{
	__UNIMPLEMENTED__
}




//
// System call 'getrlimit'
//

int Context::ExecuteSyscall_getrlimit()
{
	__UNIMPLEMENTED__
}




//
// System call 'mmap2'
//

int Context::ExecuteSyscall_mmap2()
{
	// Arguments
	unsigned addr = regs.getEbx();
	unsigned len = regs.getEcx();
	int prot = regs.getEdx();
	int flags = regs.getEsi();
	int guest_fd = regs.getEdi();
	int offset = regs.getEbp();

	// Debug
	emu->syscall_debug << misc::fmt("  addr=0x%x, len=%u, prot=0x%x, "
			"flags=0x%x, guest_fd=%d, offset=0x%x\n",
			addr, len, prot, flags, guest_fd, offset);
	emu->syscall_debug << misc::fmt("  prot=%s, flags=%s\n",
			mmap_prot_map.MapFlags(prot).c_str(),
			mmap_flags_map.MapFlags(flags).c_str());

	// System calls 'mmap' and 'mmap2' only differ in the interpretation of
	// argument 'offset'. Here, it is given in memory pages.
	return SyscallMmapAux(addr, len, prot, flags, guest_fd,
			offset << mem::MemoryPageShift);
}




//
// System call 'truncate64'
//

int Context::ExecuteSyscall_truncate64()
{
	__UNIMPLEMENTED__
}




//
// System call 'ftruncate64'
//

int Context::ExecuteSyscall_ftruncate64()
{
	__UNIMPLEMENTED__
}




//
// System call 'stat64'
//

struct sim_stat64_t
{
	unsigned long long dev;  // 0 8
	unsigned int pad1;  // 8 4
	unsigned int __ino;  // 12 4
	unsigned int mode;  // 16 4
	unsigned int nlink;  // 20 4
	unsigned int uid;  // 24 4
	unsigned int gid;  // 28 4
	unsigned long long rdev;  // 32 8
	unsigned int pad2;  // 40 4
	long long size;  // 44 8
	unsigned int blksize;  // 52 4
	unsigned long long blocks;  // 56 8
	unsigned int atime;  // 64 4
	unsigned int atime_nsec;  // 68 4
	unsigned int mtime;  // 72 4
	unsigned int mtime_nsec;  // 76 4
	unsigned int ctime;  // 80 4
	unsigned int ctime_nsec;  // 84 4
	unsigned long long ino;  // 88 8
} __attribute__((packed));

static void sys_stat_host_to_guest(struct sim_stat64_t *guest, struct stat *host)
{
	assert(sizeof(struct sim_stat64_t) == 96);
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

	Emu *emu = Emu::getInstance();
	emu->syscall_debug << misc::fmt("  stat64 structure:\n");
	emu->syscall_debug << misc::fmt("    dev=%lld, ino=%lld, mode=%d, nlink=%d\n",
		guest->dev, guest->ino, guest->mode, guest->nlink);
	emu->syscall_debug << misc::fmt("    uid=%d, gid=%d, rdev=%lld\n",
		guest->uid, guest->gid, guest->rdev);
	emu->syscall_debug << misc::fmt("    size=%lld, blksize=%d, blocks=%lld\n",
		guest->size, guest->blksize, guest->blocks);
}

int Context::ExecuteSyscall_stat64()
{
	__UNIMPLEMENTED__
}




//
// System call 'lstat64'
//

int Context::ExecuteSyscall_lstat64()
{
	__UNIMPLEMENTED__
}




//
// System call 'fstat64'
//

int Context::ExecuteSyscall_fstat64()
{
	// Arguments
	int fd = regs.getEbx();
	unsigned statbuf_ptr = regs.getEcx();
	emu->syscall_debug << misc::fmt("  fd=%d, statbuf_ptr=0x%x\n",
			fd, statbuf_ptr);

	// Get host descriptor
	int host_fd = file_table.getHostIndex(fd);
	emu->syscall_debug << misc::fmt("  host_fd=%d\n", host_fd);

	// Host call
	struct stat statbuf;
	int err = fstat(host_fd, &statbuf);
	if (err == -1)
		return -errno;

	// Return
	struct sim_stat64_t sim_statbuf;
	sys_stat_host_to_guest(&sim_statbuf, &statbuf);
	memory->Write(statbuf_ptr, sizeof sim_statbuf, (char *) &sim_statbuf);
	return 0;
}




//
// System call 'lchown'
//

int Context::ExecuteSyscall_lchown()
{
	__UNIMPLEMENTED__
}




//
// System call 'getuid'
//

int Context::ExecuteSyscall_getuid()
{
	__UNIMPLEMENTED__
}




//
// System call 'getgid'
//

int Context::ExecuteSyscall_getgid()
{
	__UNIMPLEMENTED__
}




//
// System call 'geteuid'
//

int Context::ExecuteSyscall_geteuid()
{
	__UNIMPLEMENTED__
}




//
// System call 'getegid'
//

int Context::ExecuteSyscall_getegid()
{
	__UNIMPLEMENTED__
}




//
// System call 'setreuid'
//

int Context::ExecuteSyscall_setreuid()
{
	__UNIMPLEMENTED__
}




//
// System call 'setregid'
//

int Context::ExecuteSyscall_setregid()
{
	__UNIMPLEMENTED__
}




//
// System call 'getgroups'
//

int Context::ExecuteSyscall_getgroups()
{
	__UNIMPLEMENTED__
}




//
// System call 'setgroups'
//

int Context::ExecuteSyscall_setgroups()
{
	__UNIMPLEMENTED__
}




//
// System call 'fchown'
//

int Context::ExecuteSyscall_fchown()
{
	__UNIMPLEMENTED__
}




//
// System call 'setresuid'
//

int Context::ExecuteSyscall_setresuid()
{
	__UNIMPLEMENTED__
}




//
// System call 'getresuid'
//

int Context::ExecuteSyscall_getresuid()
{
	__UNIMPLEMENTED__
}




//
// System call 'setresgid'
//

int Context::ExecuteSyscall_setresgid()
{
	__UNIMPLEMENTED__
}




//
// System call 'getresgid'
//

int Context::ExecuteSyscall_getresgid()
{
	__UNIMPLEMENTED__
}




//
// System call 'chown'
//

int Context::ExecuteSyscall_chown()
{
	__UNIMPLEMENTED__
}




//
// System call 'setuid'
//

int Context::ExecuteSyscall_setuid()
{
	__UNIMPLEMENTED__
}




//
// System call 'setgid'
//

int Context::ExecuteSyscall_setgid()
{
	__UNIMPLEMENTED__
}




//
// System call 'setfsuid'
//

int Context::ExecuteSyscall_setfsuid()
{
	__UNIMPLEMENTED__
}




//
// System call 'setfsgid'
//

int Context::ExecuteSyscall_setfsgid()
{
	__UNIMPLEMENTED__
}




//
// System call 'pivot_root'
//

int Context::ExecuteSyscall_pivot_root()
{
	__UNIMPLEMENTED__
}




//
// System call 'mincore'
//

int Context::ExecuteSyscall_mincore()
{
	__UNIMPLEMENTED__
}




//
// System call 'madvise'
//

int Context::ExecuteSyscall_madvise()
{
	__UNIMPLEMENTED__
}




//
// System call 'getdents64'
//

int Context::ExecuteSyscall_getdents64()
{
	__UNIMPLEMENTED__
}




//
// System call 'fcntl64'
//

int Context::ExecuteSyscall_fcntl64()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_222'
//

int Context::ExecuteSyscall_ni_syscall_222()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_223'
//

int Context::ExecuteSyscall_ni_syscall_223()
{
	__UNIMPLEMENTED__
}




//
// System call 'gettid'
//

int Context::ExecuteSyscall_gettid()
{
	__UNIMPLEMENTED__
}




//
// System call 'readahead'
//

int Context::ExecuteSyscall_readahead()
{
	__UNIMPLEMENTED__
}




//
// System call 'setxattr'
//

int Context::ExecuteSyscall_setxattr()
{
	__UNIMPLEMENTED__
}




//
// System call 'lsetxattr'
//

int Context::ExecuteSyscall_lsetxattr()
{
	__UNIMPLEMENTED__
}




//
// System call 'fsetxattr'
//

int Context::ExecuteSyscall_fsetxattr()
{
	__UNIMPLEMENTED__
}




//
// System call 'getxattr'
//

int Context::ExecuteSyscall_getxattr()
{
	__UNIMPLEMENTED__
}




//
// System call 'lgetxattr'
//

int Context::ExecuteSyscall_lgetxattr()
{
	__UNIMPLEMENTED__
}




//
// System call 'fgetxattr'
//

int Context::ExecuteSyscall_fgetxattr()
{
	__UNIMPLEMENTED__
}




//
// System call 'listxattr'
//

int Context::ExecuteSyscall_listxattr()
{
	__UNIMPLEMENTED__
}




//
// System call 'llistxattr'
//

int Context::ExecuteSyscall_llistxattr()
{
	__UNIMPLEMENTED__
}




//
// System call 'flistxattr'
//

int Context::ExecuteSyscall_flistxattr()
{
	__UNIMPLEMENTED__
}




//
// System call 'removexattr'
//

int Context::ExecuteSyscall_removexattr()
{
	__UNIMPLEMENTED__
}




//
// System call 'lremovexattr'
//

int Context::ExecuteSyscall_lremovexattr()
{
	__UNIMPLEMENTED__
}




//
// System call 'fremovexattr'
//

int Context::ExecuteSyscall_fremovexattr()
{
	__UNIMPLEMENTED__
}




//
// System call 'tkill'
//

int Context::ExecuteSyscall_tkill()
{
	__UNIMPLEMENTED__
}




//
// System call 'sendfile64'
//

int Context::ExecuteSyscall_sendfile64()
{
	__UNIMPLEMENTED__
}




//
// System call 'futex'
//

int Context::ExecuteSyscall_futex()
{
	__UNIMPLEMENTED__
}




//
// System call 'sched_setaffinity'
//

int Context::ExecuteSyscall_sched_setaffinity()
{
	__UNIMPLEMENTED__
}




//
// System call 'sched_getaffinity'
//

int Context::ExecuteSyscall_sched_getaffinity()
{
	__UNIMPLEMENTED__
}




//
// System call 'set_thread_area'
//

int Context::ExecuteSyscall_set_thread_area()
{
	// Arguments
	unsigned uinfo_ptr = regs.getEbx();
	emu->syscall_debug << misc::fmt("  uinfo_ptr=0x%x\n", uinfo_ptr);

	// Read structure
	struct sim_user_desc uinfo;
	memory->Read(uinfo_ptr, sizeof uinfo, (char *) &uinfo);
	emu->syscall_debug << misc::fmt("  entry_number=0x%x, base_addr=0x%x, limit=0x%x\n",
		uinfo.entry_number, uinfo.base_addr, uinfo.limit);
	emu->syscall_debug << misc::fmt("  seg_32bit=0x%x, contents=0x%x, read_exec_only=0x%x\n",
		uinfo.seg_32bit, uinfo.contents, uinfo.read_exec_only);
	emu->syscall_debug << misc::fmt("  limit_in_pages=0x%x, seg_not_present=0x%x, useable=0x%x\n",
		uinfo.limit_in_pages, uinfo.seg_not_present, uinfo.useable);
	if (!uinfo.seg_32bit)
		misc::fatal("syscall set_thread_area: only 32-bit segments supported");

	// Limit given in pages (4KB units)
	if (uinfo.limit_in_pages)
		uinfo.limit <<= 12;

	if (uinfo.entry_number == (unsigned) -1)
	{
		if (glibc_segment_base)
			misc::fatal("%s: glibc segment already set", __FUNCTION__);

		glibc_segment_base = uinfo.base_addr;
		glibc_segment_limit = uinfo.limit;
		uinfo.entry_number = 6;
		memory->Write(uinfo_ptr, 4, (char *) &uinfo.entry_number);
	}
	else
	{
		if (uinfo.entry_number != 6)
			misc::fatal("%s: invalid entry number", __FUNCTION__);
		if (!glibc_segment_base)
			misc::fatal("%s: glibc segment not set", __FUNCTION__);
		glibc_segment_base = uinfo.base_addr;
		glibc_segment_limit = uinfo.limit;
	}

	// Done
	return 0;
}




//
// System call 'get_thread_area'
//

int Context::ExecuteSyscall_get_thread_area()
{
	__UNIMPLEMENTED__
}




//
// System call 'io_setup'
//

int Context::ExecuteSyscall_io_setup()
{
	__UNIMPLEMENTED__
}




//
// System call 'io_destroy'
//

int Context::ExecuteSyscall_io_destroy()
{
	__UNIMPLEMENTED__
}




//
// System call 'io_getevents'
//

int Context::ExecuteSyscall_io_getevents()
{
	__UNIMPLEMENTED__
}




//
// System call 'io_submit'
//

int Context::ExecuteSyscall_io_submit()
{
	__UNIMPLEMENTED__
}




//
// System call 'io_cancel'
//

int Context::ExecuteSyscall_io_cancel()
{
	__UNIMPLEMENTED__
}




//
// System call 'fadvise64'
//

int Context::ExecuteSyscall_fadvise64()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_251'
//

int Context::ExecuteSyscall_ni_syscall_251()
{
	__UNIMPLEMENTED__
}




//
// System call 'exit_group'
//

int Context::ExecuteSyscall_exit_group()
{
	// Arguments
	int status = regs.getEbx();
	emu->syscall_debug << misc::fmt("  status=%d\n", status);

	// Finish
	FinishGroup(status);
	return 0;
}




//
// System call 'lookup_dcookie'
//

int Context::ExecuteSyscall_lookup_dcookie()
{
	__UNIMPLEMENTED__
}




//
// System call 'epoll_create'
//

int Context::ExecuteSyscall_epoll_create()
{
	__UNIMPLEMENTED__
}




//
// System call 'epoll_ctl'
//

int Context::ExecuteSyscall_epoll_ctl()
{
	__UNIMPLEMENTED__
}




//
// System call 'epoll_wait'
//

int Context::ExecuteSyscall_epoll_wait()
{
	__UNIMPLEMENTED__
}




//
// System call 'remap_file_pages'
//

int Context::ExecuteSyscall_remap_file_pages()
{
	__UNIMPLEMENTED__
}




//
// System call 'set_tid_address'
//

int Context::ExecuteSyscall_set_tid_address()
{
	__UNIMPLEMENTED__
}




//
// System call 'timer_create'
//

int Context::ExecuteSyscall_timer_create()
{
	__UNIMPLEMENTED__
}




//
// System call 'timer_settime'
//

int Context::ExecuteSyscall_timer_settime()
{
	__UNIMPLEMENTED__
}




//
// System call 'timer_gettime'
//

int Context::ExecuteSyscall_timer_gettime()
{
	__UNIMPLEMENTED__
}




//
// System call 'timer_getoverrun'
//

int Context::ExecuteSyscall_timer_getoverrun()
{
	__UNIMPLEMENTED__
}




//
// System call 'timer_delete'
//

int Context::ExecuteSyscall_timer_delete()
{
	__UNIMPLEMENTED__
}




//
// System call 'clock_settime'
//

int Context::ExecuteSyscall_clock_settime()
{
	__UNIMPLEMENTED__
}




//
// System call 'clock_gettime'
//

int Context::ExecuteSyscall_clock_gettime()
{
	__UNIMPLEMENTED__
}




//
// System call 'clock_getres'
//

int Context::ExecuteSyscall_clock_getres()
{
	__UNIMPLEMENTED__
}




//
// System call 'clock_nanosleep'
//

int Context::ExecuteSyscall_clock_nanosleep()
{
	__UNIMPLEMENTED__
}




//
// System call 'statfs64'
//

int Context::ExecuteSyscall_statfs64()
{
	__UNIMPLEMENTED__
}




//
// System call 'fstatfs64'
//

int Context::ExecuteSyscall_fstatfs64()
{
	__UNIMPLEMENTED__
}




//
// System call 'tgkill'
//

int Context::ExecuteSyscall_tgkill()
{
	__UNIMPLEMENTED__
}




//
// System call 'utimes'
//

int Context::ExecuteSyscall_utimes()
{
	__UNIMPLEMENTED__
}




//
// System call 'fadvise64_64'
//

int Context::ExecuteSyscall_fadvise64_64()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_273'
//

int Context::ExecuteSyscall_ni_syscall_273()
{
	__UNIMPLEMENTED__
}




//
// System call 'mbind'
//

int Context::ExecuteSyscall_mbind()
{
	__UNIMPLEMENTED__
}




//
// System call 'get_mempolicy'
//

int Context::ExecuteSyscall_get_mempolicy()
{
	__UNIMPLEMENTED__
}




//
// System call 'set_mempolicy'
//

int Context::ExecuteSyscall_set_mempolicy()
{
	__UNIMPLEMENTED__
}




//
// System call 'mq_open'
//

int Context::ExecuteSyscall_mq_open()
{
	__UNIMPLEMENTED__
}




//
// System call 'mq_unlink'
//

int Context::ExecuteSyscall_mq_unlink()
{
	__UNIMPLEMENTED__
}




//
// System call 'mq_timedsend'
//

int Context::ExecuteSyscall_mq_timedsend()
{
	__UNIMPLEMENTED__
}




//
// System call 'mq_timedreceive'
//

int Context::ExecuteSyscall_mq_timedreceive()
{
	__UNIMPLEMENTED__
}




//
// System call 'mq_notify'
//

int Context::ExecuteSyscall_mq_notify()
{
	__UNIMPLEMENTED__
}




//
// System call 'mq_getsetattr'
//

int Context::ExecuteSyscall_mq_getsetattr()
{
	__UNIMPLEMENTED__
}




//
// System call 'kexec_load'
//

int Context::ExecuteSyscall_kexec_load()
{
	__UNIMPLEMENTED__
}




//
// System call 'waitid'
//

int Context::ExecuteSyscall_waitid()
{
	__UNIMPLEMENTED__
}




//
// System call 'ni_syscall_285'
//

int Context::ExecuteSyscall_ni_syscall_285()
{
	__UNIMPLEMENTED__
}




//
// System call 'add_key'
//

int Context::ExecuteSyscall_add_key()
{
	__UNIMPLEMENTED__
}




//
// System call 'request_key'
//

int Context::ExecuteSyscall_request_key()
{
	__UNIMPLEMENTED__
}




//
// System call 'keyctl'
//

int Context::ExecuteSyscall_keyctl()
{
	__UNIMPLEMENTED__
}




//
// System call 'ioprio_set'
//

int Context::ExecuteSyscall_ioprio_set()
{
	__UNIMPLEMENTED__
}




//
// System call 'ioprio_get'
//

int Context::ExecuteSyscall_ioprio_get()
{
	__UNIMPLEMENTED__
}




//
// System call 'inotify_init'
//

int Context::ExecuteSyscall_inotify_init()
{
	__UNIMPLEMENTED__
}




//
// System call 'inotify_add_watch'
//

int Context::ExecuteSyscall_inotify_add_watch()
{
	__UNIMPLEMENTED__
}




//
// System call 'inotify_rm_watch'
//

int Context::ExecuteSyscall_inotify_rm_watch()
{
	__UNIMPLEMENTED__
}




//
// System call 'migrate_pages'
//

int Context::ExecuteSyscall_migrate_pages()
{
	__UNIMPLEMENTED__
}




//
// System call 'openat'
//

int Context::ExecuteSyscall_openat()
{
	__UNIMPLEMENTED__
}




//
// System call 'mkdirat'
//

int Context::ExecuteSyscall_mkdirat()
{
	__UNIMPLEMENTED__
}




//
// System call 'mknodat'
//

int Context::ExecuteSyscall_mknodat()
{
	__UNIMPLEMENTED__
}




//
// System call 'fchownat'
//

int Context::ExecuteSyscall_fchownat()
{
	__UNIMPLEMENTED__
}




//
// System call 'futimesat'
//

int Context::ExecuteSyscall_futimesat()
{
	__UNIMPLEMENTED__
}




//
// System call 'fstatat64'
//

int Context::ExecuteSyscall_fstatat64()
{
	__UNIMPLEMENTED__
}




//
// System call 'unlinkat'
//

int Context::ExecuteSyscall_unlinkat()
{
	__UNIMPLEMENTED__
}




//
// System call 'renameat'
//

int Context::ExecuteSyscall_renameat()
{
	__UNIMPLEMENTED__
}




//
// System call 'linkat'
//

int Context::ExecuteSyscall_linkat()
{
	__UNIMPLEMENTED__
}




//
// System call 'symlinkat'
//

int Context::ExecuteSyscall_symlinkat()
{
	__UNIMPLEMENTED__
}




//
// System call 'readlinkat'
//

int Context::ExecuteSyscall_readlinkat()
{
	__UNIMPLEMENTED__
}




//
// System call 'fchmodat'
//

int Context::ExecuteSyscall_fchmodat()
{
	__UNIMPLEMENTED__
}




//
// System call 'faccessat'
//

int Context::ExecuteSyscall_faccessat()
{
	__UNIMPLEMENTED__
}




//
// System call 'pselect6'
//

int Context::ExecuteSyscall_pselect6()
{
	__UNIMPLEMENTED__
}




//
// System call 'ppoll'
//

int Context::ExecuteSyscall_ppoll()
{
	__UNIMPLEMENTED__
}




//
// System call 'unshare'
//

int Context::ExecuteSyscall_unshare()
{
	__UNIMPLEMENTED__
}




//
// System call 'set_robust_list'
//

int Context::ExecuteSyscall_set_robust_list()
{
	__UNIMPLEMENTED__
}




//
// System call 'get_robust_list'
//

int Context::ExecuteSyscall_get_robust_list()
{
	__UNIMPLEMENTED__
}




//
// System call 'splice'
//

int Context::ExecuteSyscall_splice()
{
	__UNIMPLEMENTED__
}




//
// System call 'sync_file_range'
//

int Context::ExecuteSyscall_sync_file_range()
{
	__UNIMPLEMENTED__
}




//
// System call 'tee'
//

int Context::ExecuteSyscall_tee()
{
	__UNIMPLEMENTED__
}




//
// System call 'vmsplice'
//

int Context::ExecuteSyscall_vmsplice()
{
	__UNIMPLEMENTED__
}




//
// System call 'move_pages'
//

int Context::ExecuteSyscall_move_pages()
{
	__UNIMPLEMENTED__
}




//
// System call 'getcpu'
//

int Context::ExecuteSyscall_getcpu()
{
	__UNIMPLEMENTED__
}




//
// System call 'epoll_pwait'
//

int Context::ExecuteSyscall_epoll_pwait()
{
	__UNIMPLEMENTED__
}




//
// System call 'utimensat'
//

int Context::ExecuteSyscall_utimensat()
{
	__UNIMPLEMENTED__
}




//
// System call 'signalfd'
//

int Context::ExecuteSyscall_signalfd()
{
	__UNIMPLEMENTED__
}




//
// System call 'timerfd'
//

int Context::ExecuteSyscall_timerfd()
{
	__UNIMPLEMENTED__
}




//
// System call 'eventfd'
//

int Context::ExecuteSyscall_eventfd()
{
	__UNIMPLEMENTED__
}




//
// System call 'fallocate'
//

int Context::ExecuteSyscall_fallocate()
{
	__UNIMPLEMENTED__
}





}  // namespace x86

