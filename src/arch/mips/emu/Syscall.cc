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

#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/uio.h>

#include <arch/common/Driver.h>
#include <arch/common/FileTable.h>
#include <arch/common/Runtime.h>
#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "Context.h"
#include "Emu.h"
#include "Regs.h"

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
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_exit()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fork()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_read()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
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
	// Debug
	if(emu->syscall_debug)
		emu->syscall_debug << misc::fmt("write syscall\n");

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

		// Set reg a3 t 0 in case of success
		if (err != 0)
			regs.setGPR(7, 0);
		else
			regs.setGPR(7, 1);

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

//
// System call open
//

// Flags
static misc::StringMap open_flags_map =
{
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
};

comm::FileDescriptor *Context::SyscallOpenVirtualFile(const std::string &path,
		int flags, int mode)
{
	// Assume no file found
	std::string temp_path;

	// Virtual file /proc/self/maps
	if (path == "/proc/self/maps")
		temp_path = OpenProcSelfMaps();

	// Virtual file /proc/cpuinfo
//	else if (path == "/proc/cpuinfo")
//		temp_path = OpenProcCPUInfo();

	// No file found
	if (temp_path.empty())
		return nullptr;

	// File found, create descriptor
	int host_fd = open(temp_path.c_str(), flags, mode);
	assert(host_fd > 0);

	// Add file descriptor table entry.
	comm::FileDescriptor *desc = file_table->newFileDescriptor(
			comm::FileDescriptor::TypeVirtual, host_fd,
			temp_path, flags);
	emu->syscall_debug << misc::fmt("    host file '%s' opened: "
			"guest_fd=%d, host_fd=%d\n",
			temp_path.c_str(), desc->getGuestIndex(),
			desc->getHostIndex());
	return desc;
}

comm::FileDescriptor *Context::SyscallOpenVirtualDevice(const std::string &path,
		int flags, int mode)
{
	// Check if this is a Multi2Sim driver
	comm::DriverPool *driver_pool = comm::DriverPool::getInstance();
	comm::Driver *driver = driver_pool->getDriverByPath(path);
	if (!driver)
		misc::fatal("%s: Cannot find device in %s", __FUNCTION__,
				path.c_str());

	// Create new file descriptor
	comm::FileDescriptor *desc = file_table->newFileDescriptor(
			comm::FileDescriptor::TypeDevice,
			0,  // Host descriptor doesn't matter
			path,
			flags);
	desc->setDriver(driver);

	// Debug
	emu->syscall_debug << misc::fmt("    host device '%s' opened: "
				"guest_fd=%d, host_fd=%d\n",
				path.c_str(),
				desc->getGuestIndex(),
				desc->getHostIndex());
	// Return the descriptor
	return desc;
}

int Context::ExecuteSyscall_open()
{
	emu->syscall_debug << misc::fmt("open syscall\n");

	// Arguments
	unsigned int file_name_ptr = regs.getGPR(4);
	int flags = regs.getGPR(5);
	int mode = regs.getGPR(6);

	std::string file_name = memory->ReadString(file_name_ptr);
	std::string full_path = getFullPath(file_name);
	emu->syscall_debug << misc::fmt("  file_name='%s' flags=0x%x, mode=0x%x\n",
			file_name.c_str(), flags, mode);
	emu->syscall_debug << misc::fmt("  fullpath=%s\n", full_path.c_str());
	emu->syscall_debug << misc::fmt("  flags=%s\n",
			open_flags_map.MapFlags(flags).c_str());

	// The dynamic linker uses the 'open' system call to open shared libraries.
	// We need to intercept here attempts to access runtime libraries and
	// redirect them to our own Multi2Sim runtimes.
	comm::RuntimePool *runtime_pool = comm::RuntimePool::getInstance();
	std::string runtime_redirect_path;
	if (runtime_pool->Redirect(full_path, runtime_redirect_path))
		full_path = runtime_redirect_path;

	// Driver devices
	if (misc::StringPrefix(full_path, "/dev/"))
	{
		// Attempt to open virtual file
		comm::FileDescriptor *desc = SyscallOpenVirtualDevice(
				full_path, flags, mode);
		return desc->getGuestIndex();
	}

	// Virtual files
	if (misc::StringPrefix(full_path, "/proc/"))
	{
		// Attempt to open virtual file
		comm::FileDescriptor *desc = SyscallOpenVirtualFile(
				full_path, flags, mode);
		if (desc)
			return desc->getGuestIndex();

		// Unhandled virtual file. Let the application read the contents
		// of the host version of the file as if it was a regular file.
		emu->syscall_debug << "    warning: unhandled virtual file\n";
	}

	// Regular file
	int host_fd = open(full_path.c_str(), flags, mode);
	if (host_fd == -1)
		return -errno;

	// File opened, create a new file descriptor.
	comm::FileDescriptor *desc = file_table->newFileDescriptor(
			comm::FileDescriptor::TypeRegular,
			host_fd, full_path, flags);
	emu->syscall_debug << misc::fmt("    file descriptor opened: "
			"guest_fd=%d, host_fd=%d\n",
			desc->getGuestIndex(), desc->getHostIndex());

	// Return guest descriptor index
	return desc->getGuestIndex();
}


int Context::ExecuteSyscall_close()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_waitpid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_creat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_link()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_unlink()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_execve()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_chdir()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_time()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mknod()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_chmod()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_lchown()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_break()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_unused18()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_lseek()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getpid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mount()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_umount()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_setuid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getuid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_stime()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_ptrace()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_alarm()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_unused28()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_pause()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_utime()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_stty()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_gtty()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_access()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_nice()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_ftime()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sync()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_kill()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_rename()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mkdir()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_rmdir()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_dup()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_pipe()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_times()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_prof()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_brk()
{
	// Debug
	emu->syscall_debug << misc::fmt("brk syscall\n");

	unsigned int old_heap_break;
	unsigned int new_heap_break;
	unsigned int size;

	unsigned int old_heap_break_aligned;
	unsigned int new_heap_break_aligned;

	// Arguments
	new_heap_break = regs.getGPR(4);
	old_heap_break = memory->getHeapBreak();
	emu->syscall_debug << misc::fmt(
			"  newbrk=0x%x (previous brk was 0x%x)\n",
			new_heap_break, old_heap_break);

	// Align
	new_heap_break_aligned = misc::RoundUp(new_heap_break,
			memory->PageSize);
	old_heap_break_aligned = misc::RoundUp(old_heap_break,
			memory->PageSize);

	// If argument is zero, the system call is used to
	//  obtain the current top of the heap.
	if (!new_heap_break)
		return old_heap_break;

	// If the heap is increased: if some page in the way is
	// allocated, do nothing and return old heap top. Otherwise,
	// allocate pages and return new heap top.
	if (new_heap_break > old_heap_break)
	{
		size = new_heap_break_aligned - old_heap_break_aligned;
		if (size)
		{
			if (memory->MapSpace(old_heap_break_aligned, size)
					!= old_heap_break_aligned)
			{
				misc::Panic(misc::fmt("%s: out of memory", __FUNCTION__));
			}
			memory->Map(old_heap_break_aligned, size,
					memory->AccessRead | memory->AccessWrite);
		}
		memory->setHeapBreak(new_heap_break);
		emu->syscall_debug << misc::fmt("  heap grows %u bytes\n",
				new_heap_break - old_heap_break);

		return new_heap_break;
	}

	// Always allow to shrink the heap.
	if (new_heap_break < old_heap_break)
	{
		size = old_heap_break_aligned - new_heap_break_aligned;
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


int Context::ExecuteSyscall_setgid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getgid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_signal()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_geteuid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getegid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_acct()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_umount2()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_lock()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_ioctl()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fcntl()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mpx()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_setpgid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_ulimit()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_unused59()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_umask()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_chroot()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_ustat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_dup2()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getppid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getpgrp()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_setsid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sigaction()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sgetmask()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_ssetmask()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_setreuid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_setregid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sigsuspend()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sigpending()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sethostname()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_setrlimit()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getrlimit()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getrusage()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_gettimeofday()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_settimeofday()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getgroups()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_setgroups()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_reserved82()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_symlink()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_unused84()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_readlink()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_uselib()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_swapon()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_reboot()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_readdir()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


/*
 * System call 'mmap' (code 90)
 */
//FIXME: Fix this implementation
#define SYS_MMAP_BASE_ADDRESS  0xb7fb0000


/*static int Context::MipsSysMmap(unsigned int addr, unsigned int len,
	int prot, int flags, int guest_fd, int offset)
{

}
*/
int Context::ExecuteSyscall_mmap()
{
	// Debug
	if(emu->syscall_debug)
		emu->syscall_debug << misc::fmt("mmap syscall\n");

	misc::StringMap sys_mmap_prot_map =
	{
			{ "PROT_READ",       0x1 },
			{ "PROT_WRITE",      0x2 },
			{ "PROT_EXEC",       0x4 },
			{ "PROT_SEM",        0x8 },
			{ "PROT_GROWSDOWN",  0x01000000 },
			{ "PROT_GROWSUP",    0x02000000 }
	};

	misc::StringMap sys_mmap_flags_map =
	{
			// These flags are taken from
			// /usr/src/linux-headers-3.13.0-35/arch/mips/include/uapi/asm/mman.h

			 // Flags for mmap
			{ "MAP_SHARED",      0x001 },         // Share changes
			{ "MAP_PRIVATE",     0x002 },         // Changes are private
			{ "MAP_TYPE",        0x00f },         // Mask for type of mapping
			{ "MAP_FIXED",       0x010 },         // Interpret addr exactly
			// These are linux-specific
			{ "MAP_NORESERVE",   0x0400 },        // don't check for reservations
			{ "MAP_ANONYMOUS",   0x0800 },        // don't use a file
			{ "MAP_GROWSDOWN",   0x1000 },        // stack-like segment
			{ "MAP_DENYWRITE",   0x2000 },        // ETXTBSY
			{ "MAP_EXECUTABLE",  0x4000 },        // mark it as an executable
			{ "MAP_LOCKED",      0x8000 },        // pages are locked
			{ "MAP_POPULATE",    0x10000 },       // populate (prefault) pagetables
			{ "MAP_NONBLOCK",    0x20000 },       // do not block on IO
			{ "MAP_STACK",       0x40000 },       // give out an address that is best suited for process/thread stacks
			{ "MAP_HUGETLB",     0x80000 }        // create a huge page mapping
	};

	int offset;
	int guest_fd;

	std::string prot_str;
	std::string flags_str;

	unsigned int addr = regs.getGPR(4);
	unsigned int len = regs.getGPR(5);

	unsigned int prot = regs.getGPR(6);
	unsigned int flags = regs.getGPR(7);

	memory->Read(regs.getGPR(29) + 16, 4, (char *)&guest_fd);
	memory->Read(regs.getGPR(29) + 20, 4, (char *)&offset);

	if (emu->syscall_debug)
		emu->syscall_debug << misc::fmt("  addr=0x%x, len=%d, prot=0x%x, flags=0x%x, "
				"guest_fd=%d, offset=0x%x\n",
				addr, len, prot, flags, guest_fd, offset);
	prot_str = sys_mmap_prot_map.MapFlags(prot);
	flags_str = sys_mmap_flags_map.MapFlags(flags);
	if(emu->syscall_debug)
		emu->syscall_debug << misc::fmt("  prot=") << prot_str
		<< misc::fmt(", flags=") << flags_str << misc::fmt("\n");
////////////////////////////////////
	unsigned int len_aligned;

	int perm;
	int host_fd;

	comm::FileDescriptor *desc;

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
	desc =  file_table->getFileDescriptor(guest_fd);
	host_fd = desc ? desc->getHostIndex() : -1;
	if (guest_fd > 0 && host_fd < 0)
		misc::fatal("%s: invalid guest descriptor", __FUNCTION__);

	// Permissions
	perm = memory->AccessInit;
	perm |= prot & PROT_READ ? memory->AccessRead : 0;
	perm |= prot & PROT_WRITE ? memory->AccessWrite : 0;
	perm |= prot & PROT_EXEC ? memory->AccessExec : 0;

		/* Flag MAP_ANONYMOUS.
		 * If it is set, the 'fd' parameter is ignored. */
		if (flags & MAP_ANONYMOUS)
			host_fd = -1;

		/* 'addr' and 'offset' must be aligned to page size boundaries.
		 * 'len' is rounded up to page boundary. */
		if (offset & ~memory->PageMask)
			misc::fatal("%s: unaligned offset", __FUNCTION__);
		if (addr & ~memory->PageMask)
			misc::fatal("%s: unaligned address", __FUNCTION__);
		len_aligned = misc::RoundUp(len, memory->PageSize);

		/* Find region for allocation */
		if (flags & MAP_FIXED)
		{
			/* If MAP_FIXED is set, the 'addr' parameter must be obeyed, and is not just a
			 * hint for a possible base address of the allocated range. */
			if (!addr)
				misc::fatal("%s: no start specified for fixed mapping", __FUNCTION__);

			/* Any allocated page in the range specified by 'addr' and 'len'
			 * must be discarded. */
			memory->Unmap(addr, len_aligned);
		}
		else
		{
			if (!addr || memory->MapSpaceDown(addr, len_aligned) != addr)
				addr = SYS_MMAP_BASE_ADDRESS;
			addr = memory->MapSpaceDown(addr, len_aligned);
//			if (addr == -1)
//				misc::fatal("%s: out of guest memory", __FUNCTION__);
		}

		/* Allocation of memory */
		memory->Map(addr, len_aligned, perm);

		/* Host mapping */
		if (host_fd >= 0)
		{
			char buf[memory->PageSize];

			unsigned int last_pos;
			unsigned int curr_addr;

			int size;
			int count;

			/* Save previous position */
			last_pos = lseek(host_fd, 0, SEEK_CUR);
			lseek(host_fd, offset, SEEK_SET);

			/* Read pages */
			assert(len_aligned % memory->PageSize == 0);
			assert(addr % memory->PageSize == 0);
			curr_addr = addr;
			for (size = len_aligned; size > 0; size -= memory->PageSize)
			{
				memset(buf, 0, memory->PageSize);
				count = read(host_fd, buf, memory->PageSize);
				if (count)
					memory->Access(curr_addr, memory->PageSize, buf, memory->AccessInit);
				curr_addr += memory->PageSize;
			}

			/* Return file to last position */
			lseek(host_fd, last_pos, SEEK_SET);
		}

		/* Return mapped address */
		return addr;
		//////////////////////////////////////
}



int Context::ExecuteSyscall_munmap()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_truncate()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_ftruncate()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fchmod()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fchown()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getpriority()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_setpriority()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_profil()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_statfs()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fstatfs()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_ioperm()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_socketcall()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_syslog()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_setitimer()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getitimer()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_stat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_lstat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fstat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_unused109()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_iopl()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_vhangup()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_idle()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_vm86()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_wait4()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_swapoff()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sysinfo()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_ipc()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fsync()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sigreturn()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_clone()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_setdomainname()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


// utsname for uname syscall
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
	"3.1.9-1.fc16.mips32"
	"#1 Fri Jan 13 16:37:42 UTC 2012",
	"MIPS"
	""
};


int Context::ExecuteSyscall_uname()
{
	// Debug
	emu->syscall_debug << misc::fmt("uname syscall\n");

	unsigned int addr = regs.getGPR(4);

	emu->syscall_debug << misc::fmt("  addr is 0x%x\n", addr);
	memory->Write(addr, sizeof(sim_utsname), (char *)&sim_utsname);

	return 0;
}


int Context::ExecuteSyscall_modify_ldt()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_adjtimex()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mprotect()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sigprocmask()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_create_module()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_init_module()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_delete_module()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_get_kernel_syms()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_quotactl()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getpgid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fchdir()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_bdflush()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sysfs()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_personality()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_afs_syscall()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}
/* Syscall for Andrew File System */

int Context::ExecuteSyscall_setfsuid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_setfsgid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall__llseek()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getdents()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall__newselect()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_flock()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_msync()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_readv()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_writev()
{
	if(emu->syscall_debug)
		emu->syscall_debug << misc::fmt("writev syscall\n");

	comm::FileDescriptor *desc;
	int v;
	unsigned int iov_base;
	unsigned int iov_len;
	void *buf;

	/* Arguments */
	int guest_fd = regs.getGPR(4);
	unsigned int iovec_ptr = regs.getGPR(5);
	int vlen = regs.getGPR(6);

	if(emu->syscall_debug)
		emu->syscall_debug << misc::fmt("  guest_fd=%d, iovec_ptr = 0x%x, vlen=0x%x\n",
				guest_fd, iovec_ptr, vlen);

	/* Check file descriptor */
	desc = file_table->getFileDescriptor(guest_fd);
	if (!desc)
	{
		if(emu->syscall_debug)
			emu->syscall_debug << misc::fmt("  no file descriptor found");
		return -EBADF;
	}
	int host_fd = desc->getHostIndex();
	if(emu->syscall_debug)
		emu->syscall_debug << misc::fmt("  host_fd=%d\n", host_fd);

	//	/* No pipes allowed */
	//	if (desc->kind == file_desc_pipe)
	//		fatal("%s: not supported for pipes.\n%s",
	//			__FUNCTION__, err_mips_sys_note);

	/* Proceed */
	int total_len = 0;
	for (v = 0; v < vlen; v++)
	{
		/* Read io vector element */
		memory->Read(iovec_ptr, 4, (char *)&iov_base);
		memory->Read(iovec_ptr + 4, 4, (char *)&iov_len);
		iovec_ptr += 8;

		/* Read buffer from memory and write it to file */
		buf = malloc(iov_len);
		memory->Read(iov_base, iov_len, (char *)buf);

		int len = writev(host_fd, (struct iovec *)buf, iov_len);
		if (len == -1)
		{
			if(emu->syscall_debug)
				emu->syscall_debug << misc::fmt("  writev returned len = -1\n");
			free(buf);
			return -errno;
		}

		/* Accumulate written bytes */
		total_len += len;
		free(buf);
	}

	if(emu->syscall_debug)
		emu->syscall_debug << misc::fmt("  total_len for writev: %d", total_len);
	/* Return total number of bytes written */
	return total_len;
}


int Context::ExecuteSyscall_cacheflush()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_cachectl()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sysmips()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_unused150()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getsid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fdatasync()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall__sysctl()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall__mlock()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_munlock()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mlockall()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_munlockall()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sched_setparam()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sched_getparam()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sched_setscheduler()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sched_getscheduler()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sched_yield()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sched_get_priority_max()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sched_get_priority_min()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sched_rr_get_interval()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_nanosleep()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mremap()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_accept()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_bind()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_connect()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getpeername()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getsockname()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getsockopt()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_listen()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_recv()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_recvfrom()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_recvmsg()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_send()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sendmsg()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sendto()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_setsockopt()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_shutdown()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_socket()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_socketpair()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_setresuid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getresuid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_query_module()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_poll()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_nfsservctl()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_setresgid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getresgid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_prctl()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_rt_sigreturn()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_rt_sigaction()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_rt_sigprocmask()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_rt_sigpending()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_rt_sigtimedwait()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_rt_sigqueueinfo()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_rt_sigsuspend()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_pread64()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_pwrite64()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_chown()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getcwd()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_capget()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_capset()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sigaltstack()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sendfile()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getpmsg()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_putpmsg()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mmap2()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_truncate64()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_ftruncate64()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}

// stat64 syscall implementation
struct sim_stat64_t
{
	unsigned long long dev;  /* 0 8 */
	unsigned int pad1;  /* 8 4 */
	unsigned int __ino;  /* 12 4 */
	unsigned int mode;  /* 16 4 */
	unsigned int nlink;  /* 20 4 */
	unsigned int uid;  /* 24 4 */
	unsigned int gid;  /* 28 4 */
	unsigned long long rdev;  /* 32 8 */
	unsigned int pad2;  /* 40 4 */
	long long size;  /* 44 8 */
	unsigned int blksize;  /* 52 4 */
	unsigned long long blocks;  /* 56 8 */
	unsigned int atime;  /* 64 4 */
	unsigned int atime_nsec;  /* 68 4 */
	unsigned int mtime;  /* 72 4 */
	unsigned int mtime_nsec;  /* 76 4 */
	unsigned int ctime;  /* 80 4 */
	unsigned int ctime_nsec;  /* 84 4 */
	unsigned long long ino;  /* 88 8 */
} __attribute__((packed));

int Context::ExecuteSyscall_stat64()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_lstat64()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fstat64()
{
	emu->syscall_debug << misc::fmt("fstat64 syscall\n");

	// Arguments
	int fd = regs.getGPR(4);
	unsigned int statbuf_ptr = regs.getGPR(5);

	struct stat statbuf;
	struct sim_stat64_t sim_statbuf;

	emu->syscall_debug << misc::fmt("  fd=%d, statbuf_ptr=0x%x\n",
			fd, statbuf_ptr);

	// Get host descripptor
	int host_fd = file_table->getHostIndex(fd);
	emu->syscall_debug << misc::fmt("  host_fd=%d\n", host_fd);

	// Host call
	int err = fstat(host_fd, &statbuf);
	if (err == -1)
		return -errno;

	if (sizeof(struct sim_stat64_t) != 96)
		misc::Panic(misc::fmt("host stat and guest stat do not match in size"));
	memset(&sim_statbuf, 0, sizeof(struct sim_stat64_t));

	sim_statbuf.dev = statbuf.st_dev;
	sim_statbuf.__ino = statbuf.st_ino;
	sim_statbuf.mode = statbuf.st_mode;
	sim_statbuf.nlink = statbuf.st_nlink;
	sim_statbuf.uid = statbuf.st_uid;
	sim_statbuf.gid = statbuf.st_gid;
	sim_statbuf.rdev = statbuf.st_rdev;
	sim_statbuf.size = statbuf.st_size;
	sim_statbuf.blksize = statbuf.st_blksize;
	sim_statbuf.blocks = statbuf.st_blocks;
	sim_statbuf.atime = statbuf.st_atime;
	sim_statbuf.mtime = statbuf.st_mtime;
	sim_statbuf.ctime = statbuf.st_ctime;
	sim_statbuf.ino = statbuf.st_ino;

	// Debug
	emu->syscall_debug << misc::fmt("  stat64 structure:\n");
	emu->syscall_debug << misc::fmt("  dev=%lld, ino=%lld, mode=%d, nlink=%d\n",
			sim_statbuf.dev, sim_statbuf.ino, sim_statbuf.mode, sim_statbuf.nlink);
	emu->syscall_debug << misc::fmt("  uid=%d, gid=%d, rdev=%lld\n",
			sim_statbuf.uid, sim_statbuf.gid, sim_statbuf.rdev);
	emu->syscall_debug << misc::fmt("size=%lld, blksize=%d, blocks=%lld\n",
			sim_statbuf.size, sim_statbuf.blksize, sim_statbuf.blocks);

	// Return
	memory->Write(statbuf_ptr, sizeof sim_statbuf, (char *)&sim_statbuf);
	return 0;
}


int Context::ExecuteSyscall_pivot_root()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mincore()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_madvise()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getdents64()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fcntl64()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_reserved221()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_gettid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_readahead()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_setxattr()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_lsetxattr()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fsetxattr()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getxattr()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_lgetxattr()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fgetxattr()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_listxattr()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_llistxattr()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_flistxattr()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_removexattr()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_lremovexattr()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fremovexattr()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_tkill()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sendfile64()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_futex()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sched_setaffinity()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sched_getaffinity()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_io_setup()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_io_destroy()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_io_getevents()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_io_submit()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_io_cancel()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}

//
// System call 'exit_group'
//

int Context::ExecuteSyscall_exit_group()
{
	// Debug
	if (emu->syscall_debug)
		emu->syscall_debug << misc::fmt("exit_group syscall\n");
	// Arguments
	int status = regs.getGPR(4);

	// Finish context
	Finish(status);
	return 0;
}


int Context::ExecuteSyscall_lookup_dcookie()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_epoll_create()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_epoll_ctl()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_epoll_wait()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_remap_file_pages()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_set_tid_address()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_restart_syscall()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fadvise64()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_statfs64()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fstatfs64()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_timer_create()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_timer_settime()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_timer_gettime()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_timer_getoverrun()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_timer_delete()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_clock_settime()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_clock_gettime()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_clock_getres()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_clock_nanosleep()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_tgkill()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_utimes()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mbind()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_get_mempolicy()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_set_mempolicy()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mq_open()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mq_unlink()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mq_timedsend()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mq_timedreceive()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mq_notify()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mq_getsetattr()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_vserver()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_waitid()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sys_setaltroot()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_add_key()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_request_key()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_keyctl()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_set_thread_area()
{
	// Debug
	if (emu->syscall_debug)
		emu->syscall_debug << misc::fmt("set_thread_area syscall\n");

	// Get argument
	unsigned int uinfo_ptr = regs.getGPR(4);

	//Debug
	emu->syscall_debug << misc::fmt("  uinfo_ptr = 0x%x\n", uinfo_ptr);

	// Perform syscall operation
	regs.setCoprocessor0GPR(29, uinfo_ptr);
	regs.setGPR(7, 0);

	// Return
	return 0;
}


int Context::ExecuteSyscall_inotify_init()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_inotify_add_watch()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_inotify_rm_watch()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_migrate_pages()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_openat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mkdirat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_mknodat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fchownat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_futimesat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fstatat64()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_unlinkat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_renameat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_linkat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_symlinkat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_readlinkat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fchmodat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_faccessat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_pselect6()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_ppoll()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_unshare()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_splice()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_sync_file_range()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_tee()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_vmsplice()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_move_pages()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_set_robust_list()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_get_robust_list()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_kexec_load()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_getcpu()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_epoll_pwait()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_ioprio_set()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_ioprio_get()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_utimensat()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_signalfd()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_timerfd()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_eventfd()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_fallocate()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_timerfd_create()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_timerfd_gettime()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_timerfd_settime()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_signalfd4()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_eventfd2()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_epoll_create1()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_dup3()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_pipe2()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_inotify_init1()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_preadv()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_pwritev()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_rt_tgsigqueueinfo()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_perf_event_open()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d)", regs.getGPR(2) - __NR_Linux));
}


int Context::ExecuteSyscall_accept4()
{
	throw misc::Panic(misc::fmt("Unimplemented syscall (code %d))", regs.getGPR(2) - __NR_Linux));
}

} // namespace MIPS











