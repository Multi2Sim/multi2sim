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
#include <lib/esim/esim.h>
#include <arch/common/Driver.h>
#include <arch/common/Runtime.h>

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

static struct misc::StringMap clone_flags_map =
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
	{ "CLONE_IO", (int) 0x80000000 }
};

static const unsigned clone_supported_flags =
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
	unsigned entry_number;
	unsigned base_addr;
	unsigned limit;
	unsigned seg_32bit:1;
	unsigned contents:2;
	unsigned read_exec_only:1;
	unsigned limit_in_pages:1;
	unsigned seg_not_present:1;
	unsigned useable:1;
};




//
// Main function
//

void Context::ExecuteSyscall()
{
	// Get system call code from register eax
	int code = regs.getEax();

	// Check valid code
	if (code < 1 || code >= SyscallCodeCount || !execute_syscall_fn[code])
		misc::fatal("invalid system call (code %d)", code);

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
	if (code != SyscallCode_sigreturn && !getState(StateSuspended))
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
	// Arguments
	int status = regs.getEbx();
	emu->syscall_debug << misc::fmt("  status=0x%x\n", status);

	// Finish context
	Finish(status);
	return 0;
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

void Context::SyscallReadWakeup()
{
}

bool Context::SyscallReadCanWakeup()
{
	// If the host thread is still running for this context, do nothing.
	if (host_thread_suspend_active)
		return false;

	// Context received a signal
	SignalSet pending_unblocked = signal_mask_table.getPending() &
			~signal_mask_table.getBlocked();
	if (pending_unblocked.Any())
	{
		CheckSignalHandlerIntr();
		emu->syscall_debug << misc::fmt("syscall 'read' - "
				"interrupted by signal (pid %d)\n", pid);
		return true;
	}

	// Get file descriptor
	comm::FileDescriptor *desc = file_table->getFileDescriptor(syscall_read_fd);
	if (!desc)
		misc::panic("%s: invalid file descriptor", __FUNCTION__);

	// Check if data is ready in file by polling it
	struct pollfd host_fds;
	host_fds.fd = desc->getHostIndex();
	host_fds.events = POLLIN;
	int err = poll(&host_fds, 1, 0);
	if (err < 0)
		misc::panic("%s: unexpected error in host call to 'poll'",
				__FUNCTION__);

	// If data is ready, perform host 'read' call and wake up
	if (host_fds.revents)
	{
		unsigned pbuf = regs.getEcx();
		int count = regs.getEdx();
		char *buf = new char[count];

		count = read(desc->getHostIndex(), buf, count);
		if (count < 0)
			misc::panic("%s: unexpected error in host 'read'",
					__FUNCTION__);

		regs.setEax(count);
		memory->Write(pbuf, count, buf);
		delete buf;

		emu->syscall_debug << misc::fmt("syscall 'read' - "
				"continue (pid %d)\n", pid);
		emu->syscall_debug << misc::fmt("  return=0x%x\n", regs.getEax());
		return true;
	}

	// Data is not ready. Launch host thread again
	host_thread_suspend_active = true;
	if (pthread_create(&host_thread_suspend, nullptr,
			&Context::HostThreadSuspend, this))
		misc::panic("%s: could not launch host thread", __FUNCTION__);
	return false;
}

int Context::ExecuteSyscall_read()
{
	// Arguments
	int guest_fd = regs.getEbx();
	unsigned buf_ptr = regs.getEcx();
	unsigned count = regs.getEdx();
	emu->syscall_debug << misc::fmt("  guest_fd=%d, "
			"buf_ptr=0x%x, count=0x%x\n",
			guest_fd, buf_ptr, count);

	// Get file descriptor
	comm::FileDescriptor *desc = file_table->getFileDescriptor(guest_fd);
	if (!desc)
		return -EBADF;
	int host_fd = desc->getHostIndex();
	emu->syscall_debug << misc::fmt("  host_fd=%d\n", host_fd);

	// Poll the file descriptor to check if read is blocking
	char *buf = new char[count]();
	struct pollfd fds;
	fds.fd = host_fd;
	fds.events = POLLIN;
	int err = poll(&fds, 1, 0);
	if (err < 0)
		misc::panic("%s: error executing 'poll'", __FUNCTION__);

	// Non-blocking read
	if (fds.revents || (desc->getFlags() & O_NONBLOCK))
	{
		// Host system call
		err = read(host_fd, buf, count);
		if (err == -1)
		{
			delete buf;
			return -errno;
		}

		// Write in guest memory
		if (err > 0)
		{
			memory->Write(buf_ptr, err, buf);
			emu->syscall_debug << misc::StringBinaryBuffer(buf,
					count, 40);
		}

		// Return number of read bytes
		delete buf;
		return err;
	}

	// Blocking read - suspend thread
	emu->syscall_debug << misc::fmt("  blocking read - process suspended\n");
	syscall_read_fd = guest_fd;
	Suspend(&Context::SyscallReadCanWakeup, &Context::SyscallReadWakeup,
			StateRead);
	emu->ProcessEventsSchedule();

	// Free allocated buffer. Return value doesn't matter,
	// it will be overwritten when context wakes up from blocking call.
	delete buf;
	return 0;
}




//
// System call 'write'
//

void Context::SyscallWriteWakeup()
{
}

bool Context::SyscallWriteCanWakeup()
{
	// If host thread is still running for this context, do nothing.
	if (host_thread_suspend_active)
		return false;

	// Context received a signal
	SignalSet pending_unblocked = signal_mask_table.getPending() &
			~signal_mask_table.getBlocked();
	if (pending_unblocked.Any())
	{
		CheckSignalHandlerIntr();
		emu->syscall_debug << misc::fmt("syscall 'write' - "
				"interrupted by signal (pid %d)\n", pid);
		return true;
	}

	// Get file descriptor
	comm::FileDescriptor *desc = file_table->getFileDescriptor(syscall_write_fd);
	if (!desc)
		misc::panic("%s: invalid file descriptor", __FUNCTION__);

	// Check if data is ready in file by polling it
	struct pollfd host_fds;
	host_fds.fd = desc->getHostIndex();
	host_fds.events = POLLOUT;
	int err = poll(&host_fds, 1, 0);
	if (err < 0)
		misc::panic("%s: unexpected error in host 'poll'",
				__FUNCTION__);

	// If data is ready in the file, wake up context
	if (host_fds.revents)
	{
		unsigned pbuf = regs.getEcx();
		int count = regs.getEdx();
		char *buf = new char[count];
		
		memory->Read(pbuf, count, buf);
		count = write(desc->getHostIndex(), buf, count);
		if (count < 0)
			misc::panic("%s: unexpected error in host 'write'",
					__FUNCTION__);

		regs.setEax(count);
		delete buf;

		emu->syscall_debug << misc::fmt("syscall write - "
				"continue (pid %d)\n", pid);
		emu->syscall_debug << misc::fmt("  return=0x%x\n", regs.getEax());
		return true;
	}

	// Data is not ready to be written - launch host thread again
	host_thread_suspend_active = true;
	if (pthread_create(&host_thread_suspend, nullptr,
			&Context::HostThreadSuspend, this))
		misc::panic("%s: could not create child thread",
				__FUNCTION__);
	return false;
}

int Context::ExecuteSyscall_write()
{
	// Arguments
	int guest_fd = regs.getEbx();
	unsigned buf_ptr = regs.getEcx();
	unsigned count = regs.getEdx();
	emu->syscall_debug << misc::fmt("  guest_fd=%d, buf_ptr=0x%x, count=0x%x\n",
			guest_fd, buf_ptr, count);

	// Get file descriptor
	comm::FileDescriptor *desc = file_table->getFileDescriptor(guest_fd);
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
	syscall_write_fd = guest_fd;
	Suspend(&Context::SyscallWriteCanWakeup, &Context::SyscallWriteWakeup,
			StateWrite);
	emu->ProcessEventsSchedule();

	// Return value doesn't matter here. It will be overwritten when the
	// context wakes up after blocking call.
	delete buf;
	return 0;
}




//
// System call 'open'
//

// Flags given in octal
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
	else if (path == "/proc/cpuinfo")
		temp_path = OpenProcCPUInfo();

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
	// Arguments
	unsigned file_name_ptr = regs.getEbx();
	int flags = regs.getEcx();
	int mode = regs.getEdx();
	std::string file_name = memory->ReadString(file_name_ptr);
	std::string full_path = getFullPath(file_name);
	emu->syscall_debug << misc::fmt("  filename='%s' flags=0x%x, mode=0x%x\n",
			file_name.c_str(), flags, mode);
	emu->syscall_debug << misc::fmt("  fullpath='%s'\n", full_path.c_str());
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

	// Regular file.
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




//
// System call 'close'
//

int Context::ExecuteSyscall_close()
{
	// Arguments
	int guest_fd = regs.getEbx();
	int host_fd = file_table->getHostIndex(guest_fd);
	emu->syscall_debug << misc::fmt("  guest_fd=%d\n", guest_fd);
	emu->syscall_debug << misc::fmt("  host_fd=%d\n", host_fd);

	// Get file descriptor table entry.
	comm::FileDescriptor *desc = file_table->getFileDescriptor(guest_fd);
	if (!desc)
		return -EBADF;

	// Close host file descriptor only if it is valid and not
	// stdin/stdout/stderr
	if (host_fd > 2)
		close(host_fd);

	// Free guest file descriptor. This will delete the host file if it's a
	// virtual file
	if (desc->getType() == comm::FileDescriptor::TypeVirtual)
		emu->syscall_debug << misc::fmt("    host file '%s': "
				"temporary file deleted\n",
				desc->getPath().c_str());
	file_table->freeFileDescriptor(desc->getGuestIndex());

	// Success
	return 0;
}




//
// System call 'waitpid'
//

static const misc::StringMap waitpid_options_map =
{
	{ "WNOHANG",       0x00000001 },
	{ "WUNTRACED",     0x00000002 },
	{ "WEXITED",       0x00000004 },
	{ "WCONTINUED",    0x00000008 },
	{ "WNOWAIT",       0x01000000 },
	{ "WNOTHREAD",     0x20000000 },
	{ "WALL",          0x40000000 },
	{ "WCLONE",        (int) 0x80000000 }
};

void Context::SyscallWaitpidWakeup()
{
}

bool Context::SyscallWaitpidCanWakeup()
{
	// A zombie child is available to 'waitpid' it
	Context *child = getZombie(syscall_waitpid_pid);
	if (child)
	{
		// Continue with 'waitpid' system call
		unsigned pstatus = regs.getEcx();
		regs.setEax(child->pid);
		if (pstatus)
			memory->Write(pstatus, 4, (char *) &child->exit_code);
		child->setState(StateFinished);

		emu->syscall_debug << misc::fmt("syscall waitpid - "
				"continue (pid %d)\n", pid)
				<< misc::fmt("  return=0x%x\n", regs.getEax());
		return true;
	}

	// No event available. Since this context won't wake up on its own, no
	// host thread is needed.
	return false;
}

int Context::ExecuteSyscall_waitpid()
{
	// Arguments
	int pid = regs.getEbx();
	unsigned status_ptr = regs.getEcx();
	int options = regs.getEdx();
	emu->syscall_debug << misc::fmt("  pid=%d, pstatus=0x%x, options=0x%x\n",
			pid, status_ptr, options);
	emu->syscall_debug << misc::fmt("  options=%s\n",
			waitpid_options_map.MapFlags(options).c_str());

	// Supported values for 'pid'
	if (pid != -1 && pid <= 0)
		misc::fatal("%s: only supported for pid=-1 or pid > 0.\n%s",
				__FUNCTION__, syscall_error_note);

	// Look for a zombie child.
	Context *child = getZombie(pid);

	// If there is no child and the flag WNOHANG was not specified,
	// we get suspended until the specified child finishes.
	if (!child && !(options & 0x1))
	{
		syscall_waitpid_pid = pid;
		Suspend(&Context::SyscallWaitpidCanWakeup,
				&Context::SyscallWaitpidWakeup,
				StateWaitpid);
		return 0;
	}

	// Context is not suspended. WNOHANG was specified, or some child
	// was found in the zombie list.
	if (child)
	{
		if (status_ptr)
			memory->Write(status_ptr, 4, (char *) &child->exit_code);
		child->setState(StateFinished);
		return child->pid;
	}

	// Return
	return 0;
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
	// Arguments
	unsigned file_name_ptr = regs.getEbx();
	std::string file_name = memory->ReadString(file_name_ptr);
	std::string full_path = getFullPath(file_name);
	emu->syscall_debug << misc::fmt("  file_name_ptr=0x%x\n",
			file_name_ptr);
	emu->syscall_debug << misc::fmt("  file_name=%s, full_path=%s\n",
			file_name.c_str(), full_path.c_str());

	// Host call
	int err = unlink(full_path.c_str());
	if (err == -1)
		return -errno;

	// Return
	return 0;
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
	// Arguments
	unsigned path_ptr = regs.getEbx();
	emu->syscall_debug << misc::fmt("  path_ptr=0x%x\n", path_ptr);

	// Read path
	std::string path = memory->ReadString(path_ptr);
	emu->syscall_debug << misc::fmt("  path='%s'\n", path.c_str());

	// Save old host path
	char old_host_path[200];
	if (!getcwd(old_host_path, sizeof old_host_path))
		misc::panic("%s: buffer 'old_host_path' too small",
				__FUNCTION__);

	// Change host path to guest working directory
	if (chdir(loader->cwd.c_str()))
		misc::panic("%s: %s: cannot cd to guest working directory",
				__FUNCTION__, loader->cwd.c_str());
	
	// Change to specified directory
	int err = chdir(path.c_str());
	if (!err)
	{
		char new_path[200];
		if (!getcwd(new_path, sizeof new_path))
			misc::panic("%s: buffer 'path' too small", __FUNCTION__);
		loader->cwd = new_path;
		emu->syscall_debug << misc::fmt("  New working directory "
				"is '%s'\n", loader->cwd.c_str());
	}

	// Go back to old host path
	if (chdir(old_host_path))
		misc::panic("%s: cannot cd back into old host path",
				__FUNCTION__);

	// Return error code received in host call
	return err;
}




//
// System call 'time'
//

int Context::ExecuteSyscall_time()
{
	// Arguments
	unsigned time_ptr = regs.getEbx();
	emu->syscall_debug << misc::fmt("  ptime=0x%x\n", time_ptr);

	// Host call
	int t = time(NULL);
	if (time_ptr)
		memory->Write(time_ptr, 4, (char *) &t);

	// Return
	return t;
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
	// Arguments
	unsigned file_name_ptr = regs.getEbx();
	unsigned mode = regs.getEcx();
	std::string file_name = memory->ReadString(file_name_ptr);
	std::string full_path = getFullPath(file_name);
	emu->syscall_debug << misc::fmt("  file_name_ptr=0x%x, mode=0x%x\n",
			file_name_ptr, mode);
	emu->syscall_debug << misc::fmt("  file_name='%s', full_path='%s'\n",
			file_name.c_str(), full_path.c_str());

	// Host call
	int err = chmod(full_path.c_str(), mode);
	if (err == -1)
		return -errno;

	// Return
	return err;
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
	return pid;
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

static const misc::StringMap access_mode_map =
{
	{ "X_OK",  1 },
	{ "W_OK",  2 },
	{ "R_OK",  4 }
};

int Context::ExecuteSyscall_access()
{
	// Arguments
	unsigned file_name_ptr = regs.getEbx();
	int mode = regs.getEcx();

	// Read file name
	std::string file_name = memory->ReadString(file_name_ptr);
	std::string full_path = getFullPath(file_name);

	// Debug
	emu->syscall_debug << misc::fmt("  file_name='%s', mode=0x%x\n",
			file_name.c_str(), mode);
	emu->syscall_debug << misc::fmt("  full_path='%s'\n",
			full_path.c_str());
	emu->syscall_debug << misc::fmt("  mode=%s\n",
			access_mode_map.MapFlags(mode).c_str());

	// A special case is introduced here for runtimes to detect whether
	// they run in native or simulated mode. If the file name is equal to
	// "/dev/multi2sim", this system call will return 0 (access granted).
	if (full_path == "/dev/multi2sim")
		return 0;

	// Host call
	int err = access(full_path.c_str(), mode);
	if (err == -1)
		return -errno;

	// Return
	return err;
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
	// Arguments
	int pid = regs.getEbx();
	int sig = regs.getEcx();
	emu->syscall_debug << misc::fmt("  pid=%d, sig=%d (%s)\n", pid,
			sig, signal_map.MapValue(sig));

	// Find context. We assume program correctness, so misc::fatal if
	// context is not found, rather than return error code.
	Context *context = emu->getContext(pid);
	if (!context)
		misc::fatal("%s: invalid pid %d", __FUNCTION__, pid);

	// Send signal
	context->signal_mask_table.getPending().Add(sig);
	context->HostThreadSuspendCancel();
	emu->ProcessEventsSchedule();
	emu->ProcessEvents();

	// Success
	return 0;
}




//
// System call 'rename'
//

int Context::ExecuteSyscall_rename()
{
	// Arguments
	unsigned old_path_ptr = regs.getEbx();
	unsigned new_path_ptr = regs.getEcx();
	emu->syscall_debug << misc::fmt("  old_path_ptr=0x%x, "
			"new_path_ptr=0x%x\n", old_path_ptr, new_path_ptr);

	// Get old path
	std::string old_path = memory->ReadString(old_path_ptr);
	std::string new_path = memory->ReadString(new_path_ptr);
	std::string old_full_path = getFullPath(old_path);
	std::string new_full_path = getFullPath(new_path);
	emu->syscall_debug << misc::fmt("  old_path='%s', new_path='%s'\n",
			old_path.c_str(), new_path.c_str());
	emu->syscall_debug << misc::fmt("  old_full_path='%s', "
			"new_full_path='%s'\n", old_full_path.c_str(),
			new_full_path.c_str());

	// Host call
	int err = rename(old_full_path.c_str(), new_full_path.c_str());
	if (err == -1)
		return -errno;

	// Return
	return err;
}




//
// System call 'mkdir'
//

int Context::ExecuteSyscall_mkdir()
{
	// Arguments
	unsigned path_ptr = regs.getEbx();
	int mode = regs.getEcx();
	emu->syscall_debug << misc::fmt("  path_ptr=0x%x, mode=0x%x\n",
			path_ptr, mode);

	// Read path
	std::string path = memory->ReadString(path_ptr);
	std::string full_path = getFullPath(path);
	emu->syscall_debug << misc::fmt("  path='%s', full_path='%s'\n",
			path.c_str(), full_path.c_str());

	// Host call
	int err = mkdir(full_path.c_str(), mode);
	if (err == -1)
		return -errno;

	// Return
	return err;
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
	// Arguments
	int guest_fd = regs.getEbx();
	emu->syscall_debug << misc::fmt("  guest_fd=%d\n", guest_fd);

	// Check that file descriptor is valid.
	comm::FileDescriptor *desc = file_table->getFileDescriptor(guest_fd);
	if (!desc)
		return -EBADF;
	int host_fd = desc->getHostIndex();
	emu->syscall_debug << misc::fmt("  host_fd=%d\n", host_fd);

	// Duplicate host file descriptor.
	int dup_host_fd = dup(host_fd);
	if (dup_host_fd == -1)
		return -errno;

	// Create a new entry in the file descriptor table.
	comm::FileDescriptor *dup_desc = file_table->newFileDescriptor(
			comm::FileDescriptor::TypeRegular,
			dup_host_fd, desc->getPath(), desc->getFlags());
	int dup_guest_fd = dup_desc->getGuestIndex();

	// Return new file descriptor.
	return dup_guest_fd;
}




//
// System call 'pipe'
//

int Context::ExecuteSyscall_pipe()
{
	// Arguments
	unsigned fd_ptr = regs.getEbx();
	emu->syscall_debug << misc::fmt("  fd_ptr=0x%x\n", fd_ptr);

	// Create host pipe
	int host_fd[2];
	int err = pipe(host_fd);
	if (err == -1)
		misc::fatal("%s: cannot create pipe", __FUNCTION__);
	emu->syscall_debug << misc::fmt("  host pipe created: fd={%d, %d}\n",
			host_fd[0], host_fd[1]);

	// Create guest pipe
	comm::FileDescriptor *read_desc = file_table->newFileDescriptor(
			comm::FileDescriptor::TypePipe,
			host_fd[0], "", O_RDONLY);
	comm::FileDescriptor *write_desc = file_table->newFileDescriptor(
			comm::FileDescriptor::TypePipe,
			host_fd[1], "", O_WRONLY);
	int guest_read_fd = read_desc->getGuestIndex();
	int guest_write_fd = write_desc->getGuestIndex();
	emu->syscall_debug << misc::fmt("  guest pipe created: fd={%d, %d}\n",
			guest_read_fd, guest_write_fd);

	// Return file descriptors.
	memory->Write(fd_ptr, 4, (char *) &guest_read_fd);
	memory->Write(fd_ptr + 4, 4, (char *) &guest_write_fd);
	return 0;
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
	unsigned new_heap_break_aligned = misc::RoundUp(new_heap_break, mem::Memory::PageSize);
	unsigned old_heap_break_aligned = misc::RoundUp(old_heap_break, mem::Memory::PageSize);

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
					mem::Memory::AccessRead | mem::Memory::AccessWrite);
		}
		memory->setHeapBreak(new_heap_break);
		emu->syscall_debug << misc::fmt("  heap grows %u bytes\n",
				new_heap_break - old_heap_break);
		return new_heap_break;
	}

	// Always allow to shrink the heap.
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
	// Arguments 
	int guest_fd = regs.getEbx();
	unsigned cmd = regs.getEcx();
	unsigned arg = regs.getEdx();
	emu->syscall_debug << misc::fmt("  guest_fd=%d, cmd=0x%x, arg=0x%x\n",
			guest_fd, cmd, arg);

	// File descriptor 
	comm::FileDescriptor *desc = file_table->getFileDescriptor(guest_fd);
	if (!desc)
		return -EBADF;

	// Check if this is communication with a Multi2Sim driver
	if (desc->getType() == comm::FileDescriptor::TypeDevice)
	{
		// Get the driver
		comm::Driver *driver = desc->getDriver();
		assert(driver);

		// Invoke the driver call
		return driver->Call(cmd, memory.get(), arg);
	}
	
	// Request on command
	if (cmd >= 0x5401 || cmd <= 0x5408)
	{
		// 'ioctl' commands using 'struct termios' as the argument.
		// This structure is 60 bytes long both for x86 and x86_64
		// architectures, so it doesn't vary between guest/host.
		// No translation needed, so just use a 60-byte I/O buffer. 
		char buf[60];

		// Read buffer 
		memory->Read(arg, sizeof buf, buf);
		int err = ioctl(desc->getHostIndex(), cmd, &buf);
		if (err == -1)
			return -errno;

		// Return in memory 
		memory->Write(arg, sizeof buf, buf);
		return err;
	}
	else
	{
		misc::fatal("%s: not implement for cmd = 0x%x.\n%s",
			__FUNCTION__, cmd, syscall_error_note);
	}

	// Return 
	return 0;
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
	// Return 1 if there is no parent
	if (!parent)
		return 1;

	// Return parent's ID
	return parent->pid;
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

static const misc::StringMap rlimit_res_map =
{
	{ "RLIMIT_CPU",              0 },
	{ "RLIMIT_FSIZE",            1 },
	{ "RLIMIT_DATA",             2 },
	{ "RLIMIT_STACK",            3 },
	{ "RLIMIT_CORE",             4 },
	{ "RLIMIT_RSS",              5 },
	{ "RLIMIT_NPROC",            6 },
	{ "RLIMIT_NOFILE",           7 },
	{ "RLIMIT_MEMLOCK",          8 },
	{ "RLIMIT_AS",               9 },
	{ "RLIMIT_LOCKS",            10 },
	{ "RLIMIT_SIGPENDING",       11 },
	{ "RLIMIT_MSGQUEUE",         12 },
	{ "RLIMIT_NICE",             13 },
	{ "RLIMIT_RTPRIO",           14 },
	{ "RLIM_NLIMITS",            15 }
};

struct sim_rlimit
{
	unsigned cur;
	unsigned max;
};

int Context::ExecuteSyscall_setrlimit()
{
	// Arguments
	unsigned res = regs.getEbx();
	unsigned rlim_ptr = regs.getEcx();
	emu->syscall_debug << misc::fmt("  res=0x%x, rlim_ptr=0x%x\n",
			res, rlim_ptr);
	emu->syscall_debug << misc::fmt("  res=%s\n",
			rlimit_res_map.MapValue(res));

	// Read structure
	struct sim_rlimit sim_rlimit;
	memory->Read(rlim_ptr, sizeof(struct sim_rlimit), (char *) &sim_rlimit);
	emu->syscall_debug << misc::fmt("  rlim->cur=0x%x, rlim->max=0x%x\n",
			sim_rlimit.cur, sim_rlimit.max);

	// Different actions depending on resource type
	switch (res)
	{

	case RLIMIT_DATA:
	{
		// Default limit is maximum. This system call is ignored.
		break;
	}

	case RLIMIT_STACK:
	{
		// A program should allocate its stack with calls to mmap.
		// This should be a limit for the stack, which is ignored here.
		break;
	}

	default:
		misc::fatal("%s: not implemented for res = %s.\n%s",
				__FUNCTION__, rlimit_res_map.MapValue(res),
				syscall_error_note);
	}

	// Return
	return 0;
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
	// Arguments
	unsigned tv_ptr = regs.getEbx();
	unsigned tz_ptr = regs.getEcx();
	emu->syscall_debug << misc::fmt("  tv_ptr=0x%x, tz_ptr=0x%x\n",
			tv_ptr, tz_ptr);

	// Host call
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);

	// Write time value
	if (tv_ptr)
	{
		memory->Write(tv_ptr, 4, (char *) &tv.tv_sec);
		memory->Write(tv_ptr + 4, 4, (char *) &tv.tv_usec);
	}

	// Write time zone
	if (tz_ptr)
	{
		memory->Write(tz_ptr, 4, (char *) &tz.tz_minuteswest);
		memory->Write(tz_ptr + 4, 4, (char *) &tz.tz_dsttime);
	}

	// Return
	return 0;
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

int Context::SyscallMmapAux(unsigned addr, unsigned len,
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
	comm::FileDescriptor *desc = file_table->getFileDescriptor(guest_fd);
	int host_fd = desc ? desc->getHostIndex() : -1;
	if (guest_fd > 0 && host_fd < 0)
		misc::fatal("%s: invalid guest descriptor", __FUNCTION__);

	// Permissions
	int perm = mem::Memory::AccessInit;
	perm |= prot & PROT_READ ? mem::Memory::AccessRead : 0;
	perm |= prot & PROT_WRITE ? mem::Memory::AccessWrite : 0;
	perm |= prot & PROT_EXEC ? mem::Memory::AccessExec : 0;

	// Flag MAP_ANONYMOUS.
	// If it is set, the 'fd' parameter is ignored.
	if (flags & MAP_ANONYMOUS)
		host_fd = -1;

	// 'addr' and 'offset' must be aligned to page size boundaries.
	// 'len' is rounded up to page boundary.
	if (offset & ~mem::Memory::PageMask)
		misc::fatal("%s: unaligned offset", __FUNCTION__);
	if (addr & ~mem::Memory::PageMask)
		misc::fatal("%s: unaligned address", __FUNCTION__);
	unsigned len_aligned = misc::RoundUp(len, mem::Memory::PageSize);

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
		assert(len_aligned % mem::Memory::PageSize == 0);
		assert(addr % mem::Memory::PageSize == 0);
		unsigned curr_addr = addr;
		for (int size = len_aligned; size > 0; size -= mem::Memory::PageSize)
		{
			char buf[mem::Memory::PageSize];
			memset(buf, 0, mem::Memory::PageSize);
			int count = read(host_fd, buf, mem::Memory::PageSize);
			if (count)
				memory->Access(curr_addr, mem::Memory::PageSize,
						buf, mem::Memory::AccessInit);
			curr_addr += mem::Memory::PageSize;
		}

		// Record map in call stack
		if (call_stack != nullptr && !desc->getPath().empty())
			call_stack->Map(desc->getPath(),
					offset,
					addr,
					len,
					true);

		// Return file to last position
		lseek(host_fd, last_pos, SEEK_SET);
	}

	// Return mapped address
	return addr;
}


int Context::ExecuteSyscall_mmap()
{
	// This system call takes the arguments from memory, at the address
	// pointed by 'ebx'.
	unsigned args_ptr = regs.getEbx();
	unsigned addr, len;
	int prot, flags, guest_fd, offset;
	memory->Read(args_ptr, 4, (char *) &addr);
	memory->Read(args_ptr + 4, 4, (char *) &len);
	memory->Read(args_ptr + 8, 4, (char *) &prot);
	memory->Read(args_ptr + 12, 4, (char *) &flags);
	memory->Read(args_ptr + 16, 4, (char *) &guest_fd);
	memory->Read(args_ptr + 20, 4, (char *) &offset);
	emu->syscall_debug << misc::fmt("  args_ptr=0x%x\n", args_ptr);
	emu->syscall_debug << misc::fmt("  addr=0x%x, len=%u, prot=0x%x, "
			"flags=0x%x, guest_fd=%d, offset=0x%x\n",
			addr, len, prot, flags, guest_fd, offset);
	emu->syscall_debug << misc::fmt("  prot=%s, flags=%s\n",
			mmap_prot_map.MapFlags(prot).c_str(),
			mmap_flags_map.MapFlags(flags).c_str());

	// Call
	return SyscallMmapAux(addr, len, prot, flags, guest_fd, offset);
}




//
// System call 'munmap'
//

int Context::ExecuteSyscall_munmap()
{
	// Arguments
	unsigned addr = regs.getEbx();
	unsigned size = regs.getEcx();
	emu->syscall_debug << misc::fmt("  addr=0x%x, size=0x%x\n", addr, size);

	// Restrictions
	if (addr & (mem::Memory::PageSize - 1))
		misc::fatal("%s: address not aligned", __FUNCTION__);

	// Unmap
	unsigned size_aligned = misc::RoundUp(size, mem::Memory::PageSize);
	memory->Unmap(addr, size_aligned);

	// Return
	return 0;
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
	// Arguments
	int fd = regs.getEbx();
	int mode = regs.getEcx();
	emu->syscall_debug << misc::fmt("  fd=%d, mode=%d\n", fd, mode);

	// Get host descriptor
	int host_fd = file_table->getHostIndex(fd);
	emu->syscall_debug << misc::fmt("  host_fd=%d\n", host_fd);

	// Host call
	int err = fchmod(host_fd, mode);
	if (err == -1)
		return -errno;

	// Return
	return err;
}




//
// System call 'fchown16'
//

int Context::ExecuteSyscall_fchown16()
{
	// Arguments
	unsigned file_name_ptr = regs.getEbx();
	int owner = regs.getEcx();
	int group = regs.getEdx();
	emu->syscall_debug << misc::fmt("  file_name_ptr=0x%x, owner=%d, "
			"group=%d\n", file_name_ptr, owner, group);

	// Read file name
	std::string file_name = memory->ReadString(file_name_ptr);
	std::string full_path = getFullPath(file_name);
	emu->syscall_debug << misc::fmt("  filename='%s', fullpath='%s'\n",
			file_name.c_str(), full_path.c_str());

	// Host call
	int err = chown(full_path.c_str(), owner, group);
	if (err == -1)
		return -errno;

	// Return
	return err;
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
	ReturnFromSignalHandler();
	emu->ProcessEventsSchedule();
	emu->ProcessEvents();
	return 0;
}




//
// System call 'clone'
//

int Context::ExecuteSyscall_clone()
{
	// Prototype: long sys_clone(unsigned long clone_flags, unsigned long newsp,
	// 	int __user *parent_tid, int unused, int __user *child_tid);
	// There is an unused parameter, that's why we read child_tidptr from edi
	// instead of esi.

	// Arguments
	unsigned flags = regs.getEbx();
	unsigned new_esp = regs.getEcx();
	unsigned parent_tid_ptr = regs.getEdx();
	unsigned child_tid_ptr = regs.getEdi();
	emu->syscall_debug << misc::fmt("  flags=0x%x, newsp=0x%x, "
			"parent_tidptr=0x%x, child_tidptr=0x%x\n",
			flags, new_esp, parent_tid_ptr, child_tid_ptr);

	// Exit signal is specified in the lower byte of 'flags'
	int exit_signal = flags & 0xff;
	flags &= ~0xff;

	// Debug
	emu->syscall_debug << misc::fmt("  flags=%s\n",
			clone_flags_map.MapFlags(flags).c_str());
	emu->syscall_debug << misc::fmt("  exit_signal=%d (%s)\n",
			exit_signal, signal_map.MapValue(exit_signal));

	// New stack pointer defaults to current
	if (!new_esp)
		new_esp = regs.getEsp();

	// Check not supported flags
	if (flags & ~clone_supported_flags)
		misc::fatal("%s: not supported flags: %s\n%s",
				__FUNCTION__, clone_flags_map.MapFlags(flags).c_str(),
				syscall_error_note);

	// Flag CLONE_VM
	Context *context = emu->newContext();
	if (flags & SIM_CLONE_VM)
	{
		// CLONE_FS, CLONE_FILES, CLONE_SIGHAND must be there, too
		if ((flags & (SIM_CLONE_FS | SIM_CLONE_FILES | SIM_CLONE_SIGHAND)) !=
				(SIM_CLONE_FS | SIM_CLONE_FILES | SIM_CLONE_SIGHAND))
			misc::fatal("%s: not supported flags with CLONE_VM.\n%s",
				__FUNCTION__, syscall_error_note);

		// Create new context sharing memory image
		context->Clone(this);
	}
	else
	{
		// CLONE_FS, CLONE_FILES, CLONE_SIGHAND must not be there either
		if (flags & (SIM_CLONE_FS | SIM_CLONE_FILES | SIM_CLONE_SIGHAND))
			misc::fatal("%s: not supported flags with CLONE_VM.\n%s",
				__FUNCTION__, syscall_error_note);

		// Create new context replicating memory image
		context->Fork(this);
	}

	// Flag CLONE_THREAD.
	// If specified, the exit signal is ignored. Otherwise, it is specified in the
	// lower byte of the flags. Also, this determines whether to create a group of
	// threads.
	if (flags & SIM_CLONE_THREAD)
	{
		context->exit_signal = 0;
		context->group_parent = group_parent ? group_parent : this;
	}
	else
	{
		context->exit_signal = exit_signal;
		context->group_parent = nullptr;
	}

	// Flag CLONE_PARENT_SETTID
	if (flags & SIM_CLONE_PARENT_SETTID)
		memory->Write(parent_tid_ptr, 4, (char *) &context->pid);

	// Flag CLONE_CHILD_SETTID
	if (flags & SIM_CLONE_CHILD_SETTID)
		context->memory->Write(child_tid_ptr, 4, (char *) &context->pid);

	// Flag CLONE_CHILD_CLEARTID
	if (flags & SIM_CLONE_CHILD_CLEARTID)
		context->clear_child_tid = child_tid_ptr;

	// Flag CLONE_SETTLS
	if (flags & SIM_CLONE_SETTLS)
	{
		unsigned uinfo_ptr = regs.getEsi();
		emu->syscall_debug << misc::fmt("  puinfo=0x%x\n", uinfo_ptr);

		struct sim_user_desc uinfo;
		memory->Read(uinfo_ptr, sizeof(struct sim_user_desc), (char *) &uinfo);
		emu->syscall_debug << misc::fmt("  entry_number=0x%x, base_addr=0x%x, limit=0x%x\n",
				uinfo.entry_number, uinfo.base_addr, uinfo.limit);
		emu->syscall_debug << misc::fmt("  seg_32bit=0x%x, contents=0x%x, read_exec_only=0x%x\n",
				uinfo.seg_32bit, uinfo.contents, uinfo.read_exec_only);
		emu->syscall_debug << misc::fmt("  limit_in_pages=0x%x, seg_not_present=0x%x, useable=0x%x\n",
				uinfo.limit_in_pages, uinfo.seg_not_present, uinfo.useable);
		if (!uinfo.seg_32bit)
			misc::fatal("%s: only 32-bit segments supported", __FUNCTION__);

		// Limit given in pages (4KB units)
		if (uinfo.limit_in_pages)
			uinfo.limit <<= 12;

		uinfo.entry_number = 6;
		memory->Write(uinfo_ptr, 4, (char *) &uinfo.entry_number);

		context->glibc_segment_base = uinfo.base_addr;
		context->glibc_segment_limit = uinfo.limit;
	}

	// New context returns 0.
	context->regs.setEsp(new_esp);
	context->regs.setEax(0);

	// Return PID of the new context
	emu->syscall_debug << misc::fmt("  context created with pid %d\n",
			context->pid);
	return context->pid;
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
	// Arguments
	unsigned start = regs.getEbx();
	unsigned len = regs.getEcx();
	int prot = regs.getEdx();
	emu->syscall_debug << misc::fmt("  start=0x%x, len=0x%x, prot=0x%x\n",
			start, len, prot);

	// Permissions
	int perm = 0;
	perm |= prot & 0x01 ? mem::Memory::AccessRead : 0;
	perm |= prot & 0x02 ? mem::Memory::AccessWrite : 0;
	perm |= prot & 0x04 ? mem::Memory::AccessExec : 0;
	memory->Protect(start, len, perm);

	// Return
	return 0;
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
	// Arguments
	int fd = regs.getEbx();
	unsigned offset_high = regs.getEcx();
	unsigned offset_low = regs.getEdx();
	long long offset = ((long long) offset_high << 32) | offset_low;
	unsigned result_ptr = regs.getEsi();
	int origin = regs.getEdi();
	
	// Debug
	emu->syscall_debug << misc::fmt("  fd=%d\n", fd)
			<< misc::fmt("  offset_high = 0x%x\n", offset_high)
			<< misc::fmt("  offset_low = 0x%x\n", offset_low)
			<< misc::fmt("  result_ptr = 0x%x\n,", result_ptr)
			<< misc::fmt("  origin = 0x%x\n", origin)
			<< misc::fmt("  offset = 0x%llx\n", (long long) offset);
	
	// Check file descriptor 
	comm::FileDescriptor *desc = file_table->getFileDescriptor(fd);
	if (!desc)
		return -EBADF;
	
	// Get host file descriptor
	int host_fd = desc->getHostIndex();
	emu->syscall_debug << misc::fmt("  host_fd = %d\n", host_fd);

	// Supported offset
	if (offset_high != (unsigned) -1 && offset_high)
		misc::fatal("%s: only supported for 32-bit files", __FUNCTION__);

	// Host call
	offset = lseek(host_fd, offset_low, origin);
	if (offset == -1)
		return -errno;

	// Copy offset to memory
	if (result_ptr)
		memory->Write(result_ptr, 8, (char *) &offset);

	// Success
	return 0;
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

static const misc::StringMap msync_flags_map =
{
	{ "MS_ASYNC", 1 },
	{ "MS_INAVLIAGE", 2 },
	{ "MS_SYNC", 4 }
};

int Context::ExecuteSyscall_msync()
{
	// Arguments
	unsigned start = regs.getEbx();
	unsigned len = regs.getEcx();
	int flags = regs.getEdx();
	emu->syscall_debug << misc::fmt("  start=0x%x, len=0x%x, flags=0x%x\n",
			start, len, flags);
	emu->syscall_debug << misc::fmt("  flags=%s\n",
			msync_flags_map.MapFlags(flags).c_str());

	// System call ignored
	return 0;
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
	int len;
	unsigned iov_base;
	unsigned iov_len;

	// Arguments 
	int guest_fd = regs.getEbx();
	unsigned iovec_ptr = regs.getEcx();
	unsigned vlen = regs.getEdx();
	emu->syscall_debug << misc::fmt("  guest_fd=%d, iovec_ptr = 0x%x, vlen=0x%x\n",
		guest_fd, iovec_ptr, vlen);

	// Check file descriptor 
	comm::FileDescriptor *desc = file_table->getFileDescriptor(guest_fd);
	if (!desc)
		return -EBADF;
	int host_fd = desc->getHostIndex();
	emu->syscall_debug << misc::fmt("  host_fd=%d\n", host_fd);

	// No pipes allowed 
	if (desc->getType() == comm::FileDescriptor::TypePipe)
		misc::fatal("%s: not supported for pipes.\n%s",
			__FUNCTION__, syscall_error_note);

	// Proceed 
	int total_len = 0;
	for (unsigned v = 0; v < vlen; v++)
	{
		// Read io vector element 
		memory->Read(iovec_ptr, 4, (char *)&iov_base);
		memory->Read(iovec_ptr + 4, 4, (char *)&iov_len);
		iovec_ptr += 8;

		// Read buffer from memory and write it to file 
		char *buf = (char *)malloc(iov_len);
		memory->Read(iov_base, iov_len, buf);
		len = write(host_fd, buf, iov_len);
		if (len == -1)
		{
			free(buf);
			return -errno;
		}

		// Accumulate written bytes 
		total_len += len;
		free(buf);
	}

	// Return total number of bytes written 
	return total_len;

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

struct sysctl_args
{
	unsigned pname;
	unsigned nlen;
	unsigned poldval;
	unsigned oldlenp;
	unsigned pnewval;
	unsigned newlen;
};

int Context::ExecuteSyscall_sysctl()
{
	// Arguments
	unsigned args_ptr = regs.getEbx();
	emu->syscall_debug << misc::fmt("  pargs=0x%x\n", args_ptr);

	// Access arguments in memory
	struct sysctl_args args;
	memory->Read(args_ptr, sizeof args, (char *) &args);
	emu->syscall_debug << misc::fmt("    pname=0x%x\n", args.pname);
	emu->syscall_debug << misc::fmt("    nlen=%d\n      ", args.nlen);
	for (unsigned i = 0; i < args.nlen; i++)
	{
		unsigned aux;
		memory->Read(args.pname + i * 4, 4, (char *) &aux);
		emu->syscall_debug << misc::fmt("name[%d]=%d ", i, aux);
	}
	emu->syscall_debug << misc::fmt("\n    poldval=0x%x\n", args.poldval);
	emu->syscall_debug << misc::fmt("    oldlenp=0x%x\n", args.oldlenp);
	emu->syscall_debug << misc::fmt("    pnewval=0x%x\n", args.pnewval);
	emu->syscall_debug << misc::fmt("    newlen=%d\n", args.newlen);

	// Supported values
	if (!args.oldlenp || !args.poldval)
		misc::fatal("%s: not supported for poldval=0 or oldlenp=0.\n%s",
			__FUNCTION__, syscall_error_note);
	if (args.pnewval || args.newlen)
		misc::fatal("%s: not supported for pnewval or newlen other than 0.\n%s",
			__FUNCTION__, syscall_error_note);

	// Return
	unsigned zero = 0;
	memory->Write(args.oldlenp, 4, (char *) &zero);
	memory->Write(args.poldval, 1, (char *) &zero);
	return 0;
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
	// Arguments
	int pid = regs.getEbx();
	unsigned param_ptr = regs.getEcx();
	emu->syscall_debug << misc::fmt("  pid=%d\n", pid)
			<< misc::fmt("  param_ptr=0x%x\n", param_ptr);

	// Read priority
	int sched_priority;
	memory->Read(param_ptr, 4, (char *) &sched_priority);
	emu->syscall_debug << misc::fmt("    param.sched_priority=%d\n",
			sched_priority);

	// Currently only works when pid matches calling context
	if (pid != this->pid)
		misc::panic("%s: only supported for same pid as current "
				"context.\n%s", __FUNCTION__,
				syscall_error_note);

	// Max and min priority
	int max_priority = 99;
	int min_priority = 0;
	switch (sched_policy)
	{

	case SCHED_OTHER:
		max_priority = 0;
		min_priority = 0;

	case SCHED_FIFO:
		max_priority = 99;
		min_priority = 1;

	case SCHED_RR:
		max_priority = 99;
		min_priority = 1;

	default:
		misc::fatal("%s: policy not supported.\n%s",
				__FUNCTION__, syscall_error_note);
	}

	if (sched_priority < min_priority || sched_priority > max_priority)
		misc::fatal("%s: invalid scheduling priority supplied (%d: "
				"min = %d, max = %d)\n", __FUNCTION__,
				sched_priority, min_priority, max_priority);

	// Ignore system call
	return 0;
}




//
// System call 'sched_getparam'
//

int Context::ExecuteSyscall_sched_getparam()
{
	// Arguments
	int pid = regs.getEbx();
	unsigned param_ptr = regs.getEcx();
	emu->syscall_debug << misc::fmt("  pid=%d\n", pid)
			<< misc::fmt("  param_ptr=0x%x\n", param_ptr);

	// Currently only works when pid matches calling context
	if (pid != this->pid)
		misc::panic("%s: only supported for same pid as current "
				"context.\n%s", __FUNCTION__,
				syscall_error_note);

	// Return scheduling priority
	memory->Write(param_ptr, 4, (char *) &sched_priority);
	emu->syscall_debug << misc::fmt("  returning sched priority %d\n",
			sched_priority);

	return 0;
}




//
// System call 'sched_setscheduler'
//

int Context::ExecuteSyscall_sched_setscheduler()
{
	// Arguments
	int pid = regs.getEbx();
	int policy = regs.getEcx();
	unsigned param_ptr = regs.getEdx();
	emu->syscall_debug << misc::fmt("  pid=%d\n", pid)
			<< misc::fmt("  policy=%d\n", policy);

	// Read scheduler info
	struct sched_param param;
	memory->Read(param_ptr, sizeof(struct sched_param), (char *) &param);
	int priority = param.sched_priority;
	emu->syscall_debug << misc::fmt("  param_ptr=0x%x (priority = %d)\n",
			param_ptr, param.sched_priority);

	switch (policy)
	{
	case SCHED_OTHER:
		if (priority != 0)
			misc::fatal("%s: invalid priority.\n%s",
					__FUNCTION__, syscall_error_note);
		break;

	case SCHED_FIFO:
		misc::Warning("%s: FIFO policy is not implemented to spec\n%s",
				__FUNCTION__, syscall_error_note);
		if (priority < 1 || priority > 99)
			misc::fatal("%s: invalid priority.\n%s",
					__FUNCTION__, syscall_error_note);
		break;

	case SCHED_RR:
		if (priority < 1 || priority > 99)
			misc::fatal("%s: invalid priority.\n%s",
					__FUNCTION__, syscall_error_note);
		break;

	default:
		misc::fatal("%s: policy not supported (%d).\n%s",
				__FUNCTION__, policy, syscall_error_note);
	}

	// Find context referred by pid.
	Context *context = emu->getContext(pid);
	if (!context)
		misc::fatal("%s: invalid pid (%d)", __FUNCTION__, pid);

	context->sched_policy = policy;
	context->sched_priority = priority;
	return 0;
}




//
// System call 'sched_getscheduler'
//

int Context::ExecuteSyscall_sched_getscheduler()
{
	// Arguments
	int pid = regs.getEbx();
	emu->syscall_debug << misc::fmt("  pid=%d\n", pid);

	// Currently only works when pid matches calling context
	if (pid != this->pid)
		misc::panic("%s: only supported for same pid as current "
				"context.\n%s", __FUNCTION__,
				syscall_error_note);

	// Debug
	emu->syscall_debug << misc::fmt("  returning scheduling policy %d\n",
			sched_policy);

	// Return
	return sched_policy;
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
	// Arguments
	int policy = regs.getEbx();
	emu->syscall_debug << misc::fmt("  policy=%d\n", policy);

	switch (policy)
	{

	case SCHED_OTHER:
		return 0;

	case SCHED_FIFO:
		return 99;

	case SCHED_RR:
		return 99;

	default:
		misc::fatal("%s: policy not supported.\n%s",
				__FUNCTION__, syscall_error_note);
	}

	// Dead code
	return 0;
}




//
// System call 'sched_get_priority_min'
//

int Context::ExecuteSyscall_sched_get_priority_min()
{
	// Arguments
	int policy = regs.getEbx();
	emu->syscall_debug << misc::fmt("  policy=%d\n", policy);

	switch (policy)
	{

	case SCHED_OTHER:
		return 0;

	case SCHED_FIFO:
		return 1;

	case SCHED_RR:
		return 1;

	default:
		misc::fatal("%s: policy not supported.\n%s",
				__FUNCTION__, syscall_error_note);
	}

	// Dead code
	return 0;
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

bool Context::SyscallNanosleepCanWakeup()
{
	// If suspension thread is still running for this context, do nothing.
	if (host_thread_suspend_active)
		return false;

	// Get current time
	esim::Engine *esim = esim::Engine::getInstance();
	long long now = esim->getRealTime();

	// Timeout expired
	unsigned rmtp = regs.getEcx();
	long long zero = 0;
	if (syscall_nanosleep_wakeup_time <= now)
	{
		if (rmtp)
			memory->Write(rmtp, 8, (char *) &zero);
		regs.setEax(0);
		emu->syscall_debug << misc::fmt("syscall 'nanosleep' - "
				"continue (pid %d)\n", pid);
		emu->syscall_debug << misc::fmt("  return=0x%x\n",
				regs.getEax());
		return true;
	}

	// Context received a signal that is not blocked
	SignalSet pending_unblocked = signal_mask_table.getPending() &
			~signal_mask_table.getBlocked();
	if (pending_unblocked.Any())
	{
		if (rmtp)
		{
			long long diff = syscall_nanosleep_wakeup_time - now;
			unsigned sec = diff / 1000000;
			unsigned usec = diff % 1000000 * 1000;
			memory->Write(rmtp, 4, (char *) &sec);
			memory->Write(rmtp + 4, 4, (char *) &usec);
		}
		regs.setEax(-EINTR);
		emu->syscall_debug << misc::fmt("syscall 'nanosleep' - "
				"interrupted by signal (pid %d)\n", pid);
		return true;
	}

	// No event available, launch host thread again
	host_thread_suspend_active = true;
	if (pthread_create(&host_thread_suspend, nullptr,
			&Context::HostThreadSuspend, this))
		misc::panic("%s: could not create child thread",
				__FUNCTION__);
	return false;
}

void Context::SyscallNanosleepWakeup()
{
	// Intentionally empty
}

int Context::ExecuteSyscall_nanosleep()
{
	// Arguments
	unsigned rqtp = regs.getEbx();
	unsigned rmtp = regs.getEcx();
	emu->syscall_debug << misc::fmt("  rqtp=0x%x, rmtp=0x%x\n", rqtp, rmtp);

	// Get current time
	esim::Engine *esim = esim::Engine::getInstance();
	long long now = esim->getRealTime();

	// Read structure
	unsigned sec;
	unsigned nsec;
	memory->Read(rqtp, 4, (char *) &sec);
	memory->Read(rqtp + 4, 4, (char *) &nsec);
	long long total = (long long) sec * 1000000 + (nsec / 1000);
	emu->syscall_debug << misc::fmt("  sleep time (us): %llu\n", total);

	// Suspend process
	syscall_nanosleep_wakeup_time = now + total;
	Suspend(&Context::SyscallNanosleepCanWakeup,
			&Context::SyscallNanosleepWakeup, StateNanosleep);
	emu->ProcessEventsSchedule();

	// Success
	return 0;
}




//
// System call 'mremap'
//

int Context::ExecuteSyscall_mremap()
{
	// Arguments
	unsigned addr = regs.getEbx();
	unsigned old_len = regs.getEcx();
	unsigned new_len = regs.getEdx();
	int flags = regs.getEsi();
	emu->syscall_debug << misc::fmt("  addr=0x%x, old_len=0x%x, "
			"new_len=0x%x flags=0x%x\n",
			addr, old_len, new_len, flags);

	// Restrictions
	assert(!(addr & (mem::Memory::PageSize - 1)));
	assert(!(old_len & (mem::Memory::PageSize - 1)));
	assert(!(new_len & (mem::Memory::PageSize - 1)));
	if (!(flags & 0x1))
		misc::fatal("%s: flags MAP_MAYMOVE must be present",
				__FUNCTION__);
	if (!old_len || !new_len)
		misc::fatal("%s: old_len or new_len cannot be zero",
				__FUNCTION__);

	// New size equals to old size means no action.
	if (new_len == old_len)
		return addr;

	// Shrink region. This is always possible.
	if (new_len < old_len)
	{
		memory->Unmap(addr + new_len, old_len - new_len);
		return addr;
	}

	/* Increase region at the same address. This is only possible if
	 * there is enough free space for the new region. */
	if (new_len > old_len && memory->MapSpace(addr + old_len,
			new_len - old_len) == addr + old_len)
	{
		memory->Map(addr + old_len, new_len - old_len,
				mem::Memory::AccessRead |
				mem::Memory::AccessWrite);
		return addr;
	}

	// A new region must be found for the new size.
	unsigned new_addr = memory->MapSpaceDown(mmap_base_address, new_len);
	if (new_addr == (unsigned) -1)
		misc::fatal("%s: out of guest memory", __FUNCTION__);

	// Map new region and copy old one
	memory->Map(new_addr, new_len,
			mem::Memory::AccessRead |
			mem::Memory::AccessWrite);
	memory->Copy(new_addr, addr, std::min(old_len, new_len));
	memory->Unmap(addr, old_len);

	// Return new address
	return new_addr;
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

static const misc::StringMap poll_event_map =
{
	{ "POLLIN",          0x0001 },
	{ "POLLPRI",         0x0002 },
	{ "POLLOUT",         0x0004 },
	{ "POLLERR",         0x0008 },
	{ "POLLHUP",         0x0010 },
	{ "POLLNVAL",        0x0020 }
};

struct sim_pollfd
{
	unsigned fd;
	unsigned short events;
	unsigned short revents;
};

void Context::SyscallPollWakeup()
{
}

bool Context::SyscallPollCanWakeup()
{
	// If host thread is still running for this context, do nothing.
	if (host_thread_suspend_active)
		return false;

	// Current time
	esim::Engine *esim = esim::Engine::getInstance();
	long long now = esim->getRealTime();

	// Get arguments
	unsigned prevents = regs.getEbx() + 6;
	comm::FileDescriptor *desc = file_table->getFileDescriptor(syscall_poll_fd);
	if (!desc)
		misc::panic("%s: invalid file descriptor (%d)",
				__FUNCTION__, syscall_poll_fd);

	// Context received a signal
	SignalSet pending_unblocked = signal_mask_table.getPending() &
			~signal_mask_table.getBlocked();
	if (pending_unblocked.Any())
	{
		CheckSignalHandlerIntr();
		emu->syscall_debug << misc::fmt("syscall 'poll' - "
				"interrupted by signal (pid %d)\n", pid);
		return true;
	}

	// Perform host 'poll' call
	unsigned short revents = 0;
	struct pollfd host_fds;
	host_fds.fd = desc->getHostIndex();
	host_fds.events = ((syscall_poll_events & 4) ? POLLOUT : 0) |
			((syscall_poll_events & 1) ? POLLIN : 0);
	int err = poll(&host_fds, 1, 0);
	if (err < 0)
		misc::panic("%s: unexpected error in host 'poll'",
				__FUNCTION__);

	// POLLOUT event available
	if (syscall_poll_events & host_fds.revents & POLLOUT)
	{
		revents = POLLOUT;
		memory->Write(prevents, 2, (char *) &revents);
		regs.setEax(1);
		emu->syscall_debug << misc::fmt("syscall poll - "
				"continue (pid %d) - POLLOUT "
				"occurred in file\n", pid);
		emu->syscall_debug << misc::fmt("  retval=%d\n",
				regs.getEax());
		return true;
	}

	// POLLIN event available
	if (syscall_poll_events & host_fds.revents & POLLIN)
	{
		revents = POLLIN;
		memory->Write(prevents, 2, (char *) &revents);
		regs.setEax(1);
		emu->syscall_debug << misc::fmt("syscall poll - "
				"continue (pid %d) - POLLIN "
				"occurred in file\n", pid);
		emu->syscall_debug << misc::fmt("  retval=%d\n",
				regs.getEax());
		return true;
	}

	// Timeout expired
	if (syscall_poll_time && syscall_poll_time < now)
	{
		revents = 0;
		memory->Write(prevents, 2, (char *) &revents);
		regs.setEax(0);
		emu->syscall_debug << misc::fmt("syscall poll - "
				"continue (pid %d) - time out\n", pid);
		emu->syscall_debug << misc::fmt("  retval=%d\n",
				regs.getEax());
		return true;
	}

	// No event available, launch host thread again
	host_thread_suspend_active = true;
	if (pthread_create(&host_thread_suspend, nullptr,
			&Context::HostThreadSuspend, this))
		misc::panic("%s: could not launch host thread", __FUNCTION__);
	return false;
}

int Context::ExecuteSyscall_poll()
{
	// Arguments
	unsigned pfds = regs.getEbx();
	unsigned nfds = regs.getEcx();
	int timeout = regs.getEdx();
	emu->syscall_debug << misc::fmt("  pfds=0x%x, nfds=%d, timeout=%d\n",
			pfds, nfds, timeout);

	// Assumptions on host architecture
	sim_pollfd guest_fds;
	assert(sizeof guest_fds == 8);
	assert(POLLIN == 1);
	assert(POLLPRI == 2);
	assert(POLLOUT == 4);

	// Supported value
	if (nfds != 1)
		misc::fatal("%s: not suported for nfds != 1\n%s",
				__FUNCTION__, syscall_error_note);

	// Read pollfd
	memory->Read(pfds, sizeof guest_fds, (char *) &guest_fds);
	int guest_fd = guest_fds.fd;
	emu->syscall_debug << misc::fmt("  guest_fd=%d, events=%s\n",
			guest_fd, poll_event_map.MapFlags(guest_fds.events).c_str());

	// Get file descriptor
	comm::FileDescriptor *desc = file_table->getFileDescriptor(guest_fd);
	if (!desc)
		return -EBADF;
	int host_fd = desc->getHostIndex();
	emu->syscall_debug << misc::fmt("  host_fd=%d\n", host_fd);

	// Only POLLIN (0x1) and POLLOUT (0x4) supported
	if (guest_fds.events & ~(POLLIN | POLLOUT))
		misc::fatal("%s: event not supported.\n%s",
				__FUNCTION__, syscall_error_note);

	// Not supported file descriptor
	if (host_fd < 0)
		misc::fatal("%s: not supported file descriptor.\n%s",
				__FUNCTION__, syscall_error_note);

	// Perform host 'poll' system call with a 0 timeout to distinguish
	// blocking from non-blocking cases.
	struct pollfd host_fds;
	host_fds.fd = host_fd;
	host_fds.events = guest_fds.events;
	int err = poll(&host_fds, 1, 0);
	if (err == -1)
		return -errno;

	// If host 'poll' returned a value greater than 0, the guest call is non-blocking,
	// since I/O is ready for the file descriptor.
	if (err > 0)
	{
		// Non-blocking POLLOUT on a file.
		if (guest_fds.events & host_fds.revents & POLLOUT)
		{
			emu->syscall_debug << misc::fmt("  non-blocking write "
					"to file guaranteed\n");
			guest_fds.revents = POLLOUT;
			memory->Write(pfds, sizeof guest_fds, (char *) &guest_fds);
			return 1;
		}

		// Non-blocking POLLIN on a file.
		if (guest_fds.events & host_fds.revents & POLLIN)
		{
			emu->syscall_debug << misc::fmt("  non-blocking read "
					"from file guaranteed\n");
			guest_fds.revents = POLLIN;
			memory->Write(pfds, sizeof guest_fds, (char *) &guest_fds);
			return 1;
		}

		// Never should get here
		misc::panic("%s: unexpected events", __FUNCTION__);
	}

	// At this point, host 'poll' returned 0, which means that none of the
	// requested events is ready on the file, so we must suspend until they
	// occur.
	emu->syscall_debug << misc::fmt("  process going to sleep waiting for "
			"events on file\n");
	
	// Calculate wakeup time
	syscall_poll_time = 0;
	if (timeout >= 0)
	{
		esim::Engine *esim = esim::Engine::getInstance();
		long long now = esim->getRealTime();
		syscall_poll_time = now + (long long) timeout * 1000;
	}

	// Other wakeup arguments
	syscall_poll_fd = guest_fd;
	syscall_poll_events = guest_fds.events;

	// Suspend
	Suspend(&Context::SyscallPollCanWakeup, &Context::SyscallPollWakeup,
			StatePoll);
	emu->ProcessEventsSchedule();
	return 0;
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
	// Arguments
	int sig = regs.getEbx();
	unsigned act_ptr = regs.getEcx();
	unsigned old_act_ptr = regs.getEdx();
	int sigsetsize = regs.getEsi();
	emu->syscall_debug << misc::fmt("  sig=%d, act_ptr=0x%x, "
			"old_act_ptr=0x%x, sigsetsize=0x%x\n",
			sig, act_ptr, old_act_ptr, sigsetsize);
	emu->syscall_debug << misc::fmt("  signal=%s\n",
			signal_map.MapValue(sig));

	// Invalid signal
	if (sig < 1 || sig > 64)
		misc::fatal("%s: invalid signal (%d)", __FUNCTION__, sig);

	// Read new sigaction
	SignalHandler act;
	if (act_ptr)
	{
		act.ReadFromMemory(memory.get(), act_ptr);
		emu->syscall_debug << misc::fmt("  act: ") << act
				<< misc::fmt("\n    flags: ")
				<< signal_handler_flags_map.MapFlags(act.getFlags())
				<< misc::fmt("\n    mask: ")
				<< act.getMask() << '\n';
	}

	// Store previous signal handler
	if (old_act_ptr)
	{
		SignalHandler *handler = signal_handler_table->getSignalHandler(sig);
		handler->WriteToMemory(memory.get(), old_act_ptr);
	}

	// Make new sigaction effective
	if (act_ptr)
	{
		SignalHandler *handler = signal_handler_table->getSignalHandler(sig);
		*handler = act;
	}

	// Return
	return 0;
}




//
// System call 'rt_sigprocmask'
//

static const misc::StringMap sigprocmask_how_map =
{
	{ "SIG_BLOCK",     0 },
	{ "SIG_UNBLOCK",   1 },
	{ "SIG_SETMASK",   2 }
};

int Context::ExecuteSyscall_rt_sigprocmask()
{
	// Arguments
	int how = regs.getEbx();
	unsigned set_ptr = regs.getEcx();
	unsigned old_set_ptr = regs.getEdx();
	int sigsetsize = regs.getEsi();
	emu->syscall_debug << misc::fmt("  how=0x%x, set_ptr=0x%x, "
			"old_set_ptr=0x%x, sigsetsize=0x%x\n",
			how, set_ptr, old_set_ptr, sigsetsize);
	emu->syscall_debug << misc::fmt("  how=%s\n",
			sigprocmask_how_map.MapValue(how));

	// Save old set
	SignalSet old_set = signal_mask_table.getBlocked();
			
	// New set
	if (set_ptr)
	{
		// Read it from memory
		SignalSet set;
		set.ReadFromMemory(memory.get(), set_ptr);
		emu->syscall_debug << "  set = " << set << '\n';

		// Set new set
		switch (how)
		{

		// SIG_BLOCK
		case 0:
			signal_mask_table.getBlocked().getBitmap()
					|= set.getBitmap();
			break;

		// SIG_UNBLOCK
		case 1:
			signal_mask_table.getBlocked().getBitmap()
					&= ~set.getBitmap();
			break;

		// SIG_SETMASK
		case 2:
			signal_mask_table.getBlocked() = set;
			break;

		default:
			misc::fatal("%s: invalid value for 'how'", __FUNCTION__);
		}
	}

	// Return old set
	if (old_set_ptr)
		memory->Write(old_set_ptr, 8, (char *) &old_set);


	// A change in the signal mask can cause pending signals to be
	// able to execute, so check this.
	emu->ProcessEventsSchedule();
	emu->ProcessEvents();

	return 0;
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

void Context::SyscallSigsuspendWakeup()
{
}

bool Context::SyscallSigsuspendCanWakeup()
{
	// Context received a signal
	SignalSet pending_unblocked = signal_mask_table.getPending() &
			~signal_mask_table.getBlocked();
	if (pending_unblocked.Any())
	{
		CheckSignalHandlerIntr();
		signal_mask_table.RestoreBlockedSignals();
		emu->syscall_debug << misc::fmt("syscall 'rt_sigsuspend' - "
				"interrupted by signal (pid %d)\n", pid);
		return true;
	}

	// No event available. The context will never awake on its own, so no
	// host thread creation is necessary.
	return false;
}

int Context::ExecuteSyscall_rt_sigsuspend()
{
	// Arguments
	unsigned new_set_ptr = regs.getEbx();
	int sigsetsize = regs.getEcx();
	emu->syscall_debug << misc::fmt("  new_set_ptr=0x%x, sigsetsize=%d\n",
		new_set_ptr, sigsetsize);

	// Read temporary signal mask
	SignalSet new_set;
	new_set.ReadFromMemory(memory.get(), new_set_ptr);

	// Debug
	emu->syscall_debug << "  old mask: " << signal_mask_table.getBlocked()
			<< "\n  new mask: " << new_set
			<< "\n  pending:  " << signal_mask_table.getPending()
			<< '\n';

	// Save old mask and set new one, then suspend.
	signal_mask_table.BackupBlockedSignals();
	signal_mask_table.setBlocked(new_set);
	Suspend(&Context::SyscallSigsuspendCanWakeup,
			&Context::SyscallSigsuspendWakeup,
			StateSigsuspend);
	
	// New signal mask may cause new events
	emu->ProcessEventsSchedule();
	emu->ProcessEvents();
	return 0;
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
	// Arguments
	unsigned buf_ptr = regs.getEbx();
	unsigned size = regs.getEcx();
	emu->syscall_debug << misc::fmt("  buf_ptr=0x%x, size=0x%x\n",
			buf_ptr, size);

	// Does not fit
	if (size <= loader->cwd.length())
		return -ERANGE;

	// Return
	memory->WriteString(buf_ptr, loader->cwd);
	return loader->cwd.length() + 1;
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
	// Arguments
	unsigned res = regs.getEbx();
	unsigned rlim_ptr = regs.getEcx();
	emu->syscall_debug << misc::fmt("  res=0x%x, rlim_ptr=0x%x\n",
			res, rlim_ptr);
	emu->syscall_debug << misc::fmt("  res=%s\n",
			rlimit_res_map.MapValue(res));

	struct sim_rlimit sim_rlimit;
	switch (res)
	{

	case 2:  // RLIMIT_DATA
	{
		sim_rlimit.cur = 0xffffffff;
		sim_rlimit.max = 0xffffffff;
		break;
	}

	case 3:  // RLIMIT_STACK
	{
		sim_rlimit.cur = loader->stack_size;
		sim_rlimit.max = 0xffffffff;
		break;
	}

	case 7:  // RLIMIT_NOFILE
	{
		sim_rlimit.cur = 0x400;
		sim_rlimit.max = 0x400;
		break;
	}

	default:
		misc::fatal("%s: not implemented for res = %s.\n%s",
			__FUNCTION__, rlimit_res_map.MapValue(res),
			syscall_error_note);
	}

	// Return structure
	memory->Write(rlim_ptr, sizeof(struct sim_rlimit), (char *) &sim_rlimit);
	emu->syscall_debug << misc::fmt("  ret: cur=0x%x, max=0x%x\n",
			sim_rlimit.cur, sim_rlimit.max);

	// Return
	return 0;
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
			offset << mem::Memory::LogPageSize);
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
	// Arguments
	int fd = regs.getEbx();
	unsigned length = regs.getEcx();
	emu->syscall_debug << misc::fmt("  fd=%d, length=0x%x\n", fd, length);

	// Get host descriptor
	int host_fd = file_table->getHostIndex(fd);
	emu->syscall_debug << misc::fmt("  host_fd=%d\n", host_fd);

	// Host call
	int err = ftruncate(host_fd, length);
	if (err == -1)
		return -errno;

	// Return
	return err;
}




//
// System call 'stat64'
//

struct sim_stat64_t
{
	unsigned long long dev;  // 0 8
	unsigned pad1;  // 8 4
	unsigned __ino;  // 12 4
	unsigned mode;  // 16 4
	unsigned nlink;  // 20 4
	unsigned uid;  // 24 4
	unsigned gid;  // 28 4
	unsigned long long rdev;  // 32 8
	unsigned pad2;  // 40 4
	long long size;  // 44 8
	unsigned blksize;  // 52 4
	unsigned long long blocks;  // 56 8
	unsigned atime;  // 64 4
	unsigned atime_nsec;  // 68 4
	unsigned mtime;  // 72 4
	unsigned mtime_nsec;  // 76 4
	unsigned ctime;  // 80 4
	unsigned ctime_nsec;  // 84 4
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
	struct stat statbuf;
	struct sim_stat64_t sim_statbuf;

	// Arguments 
	unsigned file_name_ptr = regs.getEbx();
	unsigned statbuf_ptr = regs.getEcx();
	emu->syscall_debug << misc::fmt("  file_name_ptr=0x%x, statbuf_ptr=0x%x\n",
			file_name_ptr, statbuf_ptr);

	// Read file name 
	std::string file_name = memory->ReadString(file_name_ptr);
	
	// Get full path 
	std::string full_path = getFullPath(file_name);
	emu->syscall_debug << misc::fmt("  file_name='%s', full_path='%s'\n", file_name.c_str(), full_path.c_str());

	/* Host call */
	int err = stat(full_path.c_str(), &statbuf);
	if (err == -1)
		return -errno;
	
	/* Copy guest structure */
	sys_stat_host_to_guest(&sim_statbuf, &statbuf);
	memory->Write(statbuf_ptr, sizeof(sim_statbuf), (const char*)&sim_statbuf);
	return 0;
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
	int host_fd = file_table->getHostIndex(fd);
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
	return getuid();
}




//
// System call 'getgid'
//

int Context::ExecuteSyscall_getgid()
{
	return getgid();
}




//
// System call 'geteuid'
//

int Context::ExecuteSyscall_geteuid()
{
	return geteuid();
}




//
// System call 'getegid'
//

int Context::ExecuteSyscall_getegid()
{
	return getegid();
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
	// Arguments
	unsigned start = regs.getEbx();
	unsigned len = regs.getEcx();
	int advice = regs.getEdx();
	emu->syscall_debug << misc::fmt("  start=0x%x, len=%d, advice=%d\n",
			start, len, advice);

	// System call ignored
	return 0;
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

static const misc::StringMap fcntl_cmd_map =
{
	{ "F_DUPFD", 0 },
	{ "F_GETFD", 1 },
	{ "F_SETFD", 2 },
	{ "F_GETFL", 3 },
	{ "F_SETFL", 4 },
	{ "F_GETLK", 5 },
	{ "F_SETLK", 6 },
	{ "F_SETLKW", 7 },
	{ "F_SETOWN", 8 },
	{ "F_GETOWN", 9 },
	{ "F_SETSIG", 10 },
	{ "F_GETSIG", 11 },
	{ "F_GETLK64", 12 },
	{ "F_SETLK64", 13 },
	{ "F_SETLKW64", 14 }
};

int Context::ExecuteSyscall_fcntl64()
{
	// Arguments
	int guest_fd = regs.getEbx();
	int cmd = regs.getEcx();
	unsigned arg = regs.getEdx();
	emu->syscall_debug << misc::fmt("  guest_fd=%d, cmd=%d, arg=0x%x\n",
			guest_fd, cmd, arg);
	emu->syscall_debug << misc::fmt("    cmd=%s\n",
			fcntl_cmd_map.MapValue(cmd));

	// Get file descriptor table entry
	comm::FileDescriptor *desc = file_table->getFileDescriptor(guest_fd);
	if (!desc)
		return -EBADF;
	if (desc->getHostIndex() < 0)
		misc::fatal("%s: not supported for this type of file",
				__FUNCTION__);
	emu->syscall_debug << misc::fmt("    host_fd=%d\n",
			desc->getHostIndex());

	// Process command
	int err = 0;
	switch (cmd)
	{

	// F_GETFD
	case 1:
	{
		err = fcntl(desc->getHostIndex(), F_GETFD);
		if (err == -1)
			err = -errno;
		break;
	}

	// F_SETFD
	case 2:
	{
		err = fcntl(desc->getHostIndex(), F_SETFD, arg);
		if (err == -1)
			err = -errno;
		break;
	}

	// F_GETFL
	case 3:
	{
		err = fcntl(desc->getHostIndex(), F_GETFL);
		if (err == -1)
			err = -errno;
		else
			emu->syscall_debug << misc::fmt("    ret=%s\n",
					open_flags_map.MapFlags(err).c_str());
		break;
	}

	// F_SETFL
	case 4:
	{
		emu->syscall_debug << misc::fmt("    arg=%s\n",
				open_flags_map.MapFlags(arg).c_str());
		desc->setFlags(arg);

		err = fcntl(desc->getHostIndex(), F_SETFL, arg);
		if (err == -1)
			err = -errno;
		break;
	}

	default:

		misc::fatal("%s: command %s not implemented.\n%s",
				__FUNCTION__, fcntl_cmd_map.MapValue(cmd),
				syscall_error_note);
	}

	// Return
	return err;
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
	// FIXME: return different 'tid' for threads, but the system call
	// 'getpid' should return the same 'pid' for threads from the same group
	// created with CLONE_THREAD flag.
	return pid;
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

static const misc::StringMap futex_cmd_map =
{
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
};

int Context::ExecuteSyscall_futex()
{
	// Prototype: sys_futex(void *addr1, int op, int val1, struct timespec *timeout,
	//   void *addr2, int val3);

	unsigned timeout_sec;
	unsigned timeout_usec;
	unsigned bitset;

	int ret;

	// Arguments
	unsigned addr1 = regs.getEbx();
	int op = regs.getEcx();
	int val1 = regs.getEdx();
	unsigned timeout_ptr = regs.getEsi();
	unsigned addr2 = regs.getEdi();
	int val3 = regs.getEbp();
	emu->syscall_debug << misc::fmt("  addr1=0x%x, op=%d, val1=%d, ptimeout=0x%x, addr2=0x%x, val3=%d\n",
		addr1, op, val1, timeout_ptr, addr2, val3);


	// Command - 'cmd' is obtained by removing 'FUTEX_PRIVATE_FLAG' (128) and
	// 'FUTEX_CLOCK_REALTIME' from 'op'.
	unsigned cmd = op & ~(256 | 128);
	unsigned futex;
	memory->Read(addr1, 4, (char *) &futex);
	emu->syscall_debug << misc::fmt("  futex=%d, cmd=%d (%s)\n",
			futex, cmd, futex_cmd_map.MapValue(cmd));

	switch (cmd)
	{

	case 0:  // FUTEX_WAIT
	case 9:  // FUTEX_WAIT_BITSET
	{
		// Default bitset value (all bits set)
		bitset = cmd == 9 ? val3 : 0xffffffff;

		/* First, we compare the value of the futex with val1. If it's not the
		 * same, we exit with the error EWOULDBLOCK (=EAGAIN). */
		if (futex != (unsigned) val1)
			return -EAGAIN;

		// Read timeout
		if (timeout_ptr)
		{
			misc::fatal("syscall futex: FUTEX_WAIT not supported with timeout");
			memory->Read(timeout_ptr, 4, (char *) &timeout_sec);
			memory->Read(timeout_ptr + 4, 4, (char *) &timeout_usec);
			emu->syscall_debug << misc::fmt("  timeout={sec %d, usec %d}\n",
				timeout_sec, timeout_usec);
		}
		else
		{
			timeout_sec = 0;
			timeout_usec = 0;
		}

		// Suspend thread in the futex.
		wakeup_futex = addr1;
		wakeup_futex_bitset = bitset;
		wakeup_futex_sleep = emu->incFutexSleepCount();
		setState(StateSuspended);
		setState(StateFutex);
		return 0;
	}

	case 1:  // FUTEX_WAKE
	case 10:  // FUTEX_WAKE_BITSET
	{
		// Default bitset value (all bits set)
		bitset = cmd == 10 ? val3 : 0xffffffff;
		ret = FutexWake(addr1, val1, bitset);
		emu->syscall_debug << misc::fmt("  futex at 0x%x: "
				"%d processes woken up\n", addr1, ret);
		return ret;
	}

	case 4: // FUTEX_CMP_REQUEUE
	{
		// 'ptimeout' is interpreted here as an integer; only supported for INTMAX
		if (timeout_ptr != 0x7fffffff)
			misc::fatal("%s: FUTEX_CMP_REQUEUE: only supported for ptimeout=INTMAX", __FUNCTION__);

		// The value of val3 must be the same as the value of the futex
		// at 'addr1' (stored in 'futex')
		if (futex != (unsigned) val3)
			return -EAGAIN;

		// Wake up 'val1' threads from futex at 'addr1'. The number of
		// woken up threads is the return value of the system call.
		ret = FutexWake(addr1, val1, 0xffffffff);
		emu->syscall_debug << misc::fmt("  futex at 0x%x: %d processes woken up\n", addr1, ret);

		// The rest of the threads waiting in futex 'addr1' are requeued
		// into futex 'addr2'
		int requeued = 0;
		for (Context *context : emu->getContextList(ListSuspended))
		{
			if (context->getState(StateFutex)
					&& context->wakeup_futex == addr1)
			{
				context->wakeup_futex = addr2;
				requeued++;
			}
		}
		emu->syscall_debug << misc::fmt("  futex at 0x%x: "
				"%d processes requeued to futex 0x%x\n",
				addr1, requeued, addr2);
		return ret;
	}

	case 5: // FUTEX_WAKE_OP
	{
		int op;
		int oparg;
		int cmp;
		int cmparg;

		int val2 = timeout_ptr;
		int oldval;
		int newval = 0;
		int cond = 0;
		int ret = 0;

		op = (val3 >> 28) & 0xf;
		cmp = (val3 >> 24) & 0xf;
		oparg = (val3 >> 12) & 0xfff;
		cmparg = val3 & 0xfff;

		memory->Read(addr2, 4, (char *) &oldval);
		switch (op)
		{
		case 0: // FUTEX_OP_SET
			newval = oparg;
			break;
		case 1: // FUTEX_OP_ADD
			newval = oldval + oparg;
			break;
		case 2: // FUTEX_OP_OR
			newval = oldval | oparg;
			break;
		case 3: // FUTEX_OP_AND
			newval = oldval & oparg;
			break;
		case 4: // FOTEX_OP_XOR
			newval = oldval ^ oparg;
			break;
		default:
			misc::fatal("%s: FUTEX_WAKE_OP: invalid operation", __FUNCTION__);
		}
		memory->Write(addr2, 4, (char *) &newval);

		ret = FutexWake(addr1, val1, 0xffffffff);

		switch (cmp)
		{
		case 0: // FUTEX_OP_CMP_EQ
			cond = oldval == cmparg;
			break;
		case 1: // FUTEX_OP_CMP_NE
			cond = oldval != cmparg;
			break;
		case 2: // FUTEX_OP_CMP_LT
			cond = oldval < cmparg;
			break;
		case 3: // FUTEX_OP_CMP_LE
			cond = oldval <= cmparg;
			break;
		case 4: // FUTEX_OP_CMP_GT
			cond = oldval > cmparg;
			break;
		case 5: // FUTEX_OP_CMP_GE
			cond = oldval >= cmparg;
			break;
		default:
			misc::fatal("%s: FUTEX_WAKE_OP: invalid condition", __FUNCTION__);
		}
		if (cond)
			ret += FutexWake(addr2, val2, 0xffffffff);

		// FIXME: we are returning the total number of threads waken up
		// counting both calls to x86_ctx_futex_wake. Is this correct?
		return ret;
	}

	default:
		misc::fatal("%s: not implemented for cmd=%d (%s).\n%s",
			__FUNCTION__, cmd, futex_cmd_map.MapValue(cmd),
			syscall_error_note);
	}

	// Dead code
	return 0;
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
	// Arguments
	int fd = regs.getEbx();
	unsigned off_lo = regs.getEcx();
	unsigned off_hi = regs.getEdx();
	unsigned len = regs.getEsi();
	int advice = regs.getEdi();
	emu->syscall_debug << misc::fmt("  fd=%d, off={0x%x, 0x%x}, "
			"len=%d, advice=%d\n", fd, off_hi, off_lo,
			len, advice);

	// System call ignored
	return 0;
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
	// Arguments
	unsigned tidptr = regs.getEbx();
	emu->syscall_debug << misc::fmt("  tidptr=0x%x\n", tidptr);

	clear_child_tid = tidptr;
	return pid;
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

static misc::StringMap x86_sys_clock_gettime_clk_id_map =
{
	{ "CLOCK_REALTIME", 0 },
	{ "CLOCK_MONOTONIC", 1 },
	{ "CLOCK_PROCESS_CPUTIME_ID", 2 },
	{ "CLOCK_THREAD_CPUTIME_ID", 3 },
	{ "CLOCK_MONOTONIC_RAW", 4 },
	{ "CLOCK_REALTIME_COARSE", 5 },
	{ "CLOCK_MONOTONIC_COARSE", 6 }
};

int Context::ExecuteSyscall_clock_gettime()
{
	long long now;

	struct {
		unsigned int sec;
		unsigned int nsec;
	} sim_ts;

	// Arguments 
	unsigned clk_id = regs.getEbx();
	unsigned ts_ptr = regs.getEcx();
	const char *clk_id_str = x86_sys_clock_gettime_clk_id_map.MapValue(clk_id);
	emu->syscall_debug << misc::fmt("  clk_id=0x%x (%s), ts_ptr=0x%x\n",
		clk_id, clk_id_str, ts_ptr);

	// Initialize 
	sim_ts.sec = 0;
	sim_ts.nsec = 0;

	// Clock type 
	switch (clk_id)
	{
	case 0:  /* CLOCK_REALTIME */

		/* For CLOCK_REALTIME, return the host real time. This is the same
		 * value that the guest application would see if it ran natively. */
		now = esim_real_time();
		sim_ts.sec = now / 1000000;
		sim_ts.nsec = (now % 1000000) * 1000;
		break;

	case 2:  /* CLOCK_PROCESS_CPUTIME_ID */
	case 3:  /* CLOCK_THREAD_CPUTIME_ID */
	case 5:  /* CLOCK_REALTIME_COARSE */
	case 6:  /* CLOCK_MONOTONIC_COARSE */

		// These clocks are not implemented in detail. To let applications
		// run without errors, we just provide a warning for now.
		misc::Warning("%s: not implemented for 'clk_id' = %d",
			__FUNCTION__, clk_id);

	case 1:  /* CLOCK_MONOTONIC */
	case 4:  /* CLOCK_MONOTONIC_RAW */

		/* For these two clocks, we want to return simulated time. This
		 * time is tricky to calculate when there could be iterations of the
		 * main simulation loop when no timing simulation happened, but
		 * which still need to be considered to avoid the application
		 * having the illusion of time not going by at all. This is the
		 * strategy assumed to calculate simulated time:
		 *   - A first component is based on the value of 'esim_time',
		 *     considering all simulation cycles when there was an
		 *     active timing simulation of any architecture.
		 *   - A second component will add a time increment for each
		 *     simulation main loop iteration where the global time
		 *     'esim_time' was not incremented. These iterations are
		 *     recorded in variable 'esim_no_forward_cycles'. A default
		 *     value of 1ns per each iteration is considered here.
		 */
		now = esim_time / 1000;  /* Obtain nsec */
		now += esim_no_forward_cycles;  /* One more nsec per iteration */
		sim_ts.sec = now / 1000000000ll;
		sim_ts.nsec = now % 1000000000ll;
		break;

	default:
		misc::fatal("%s: invalid value for 'clk_id' (%d)",
			__FUNCTION__, clk_id);
	}

	// Debug 
	emu->syscall_debug << misc::fmt("\tts.tv_sec = %u\n", sim_ts.sec);
	emu->syscall_debug << misc::fmt("\tts.tv_nsec = %u\n", sim_ts.nsec);

	// Write to guest memory 
	memory->Write(ts_ptr, sizeof sim_ts, (const char *)&sim_ts);
	return 0;
}




//
// System call 'clock_getres'
//

int Context::ExecuteSyscall_clock_getres()
{
	// Arguments
	unsigned clk_id = regs.getEbx();
	unsigned pres = regs.getEcx();
	emu->syscall_debug << misc::fmt("  clk_id=%d\n", clk_id);
	emu->syscall_debug << misc::fmt("  pres=0x%x\n", pres);

	// Return
	unsigned tv_sec = 0;
	unsigned tv_nsec = 1;
	memory->Write(pres, 4, (char *) &tv_sec);
	memory->Write(pres + 4, 4, (char *) &tv_nsec);
	return 0;
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
	// Arguments
	int tgid = regs.getEbx();
	int pid = regs.getEcx();
	int sig = regs.getEdx();
	emu->syscall_debug << misc::fmt("  tgid=%d, pid=%d, sig=%d (%s)\n",
			tgid, pid, sig, signal_map.MapValue(sig));

	// Implementation restrictions.
	if (tgid == -1)
		misc::panic("%s: not supported for tgid = -1\n%s",
				__FUNCTION__, syscall_error_note);

	// Find context referred by pid.
	Context *context = emu->getContext(pid);
	if (!context)
		misc::fatal("%s: invalid pid (%d)", __FUNCTION__, pid);

	// Send signal
	context->signal_mask_table.getPending().Add(sig);
	context->HostThreadSuspendCancel();
	emu->ProcessEventsSchedule();
	emu->ProcessEvents();
	return 0;
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
	// Arguments
	unsigned head = regs.getEbx();
	int len = regs.getEcx();
	emu->syscall_debug << misc::fmt("  head=0x%x, len=%d\n", head, len);

	// Support
	if (len != 12)
		misc::fatal("%s: not supported for len != 12\n%s",
			__FUNCTION__, syscall_error_note);

	// Set robust list
	robust_list_head = head;
	return 0;
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

