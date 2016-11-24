/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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
#include <syscall.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/time.h>
#include <sys/times.h>

#include <arch/common/Driver.h>
#include <arch/common/FileTable.h>
#include <arch/common/Runtime.h>
#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "Context.h"
#include "Emulator.h"
#include "Regs.h"


namespace ARM
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
#include "ContextSyscall.def"
#undef DEFSYSCALL
		""
};


const Context::ExecuteSyscallFn Context::execute_syscall_fn[SyscallCodeCount + 1] =
{
#define DEFSYSCALL(name, code) &Context::ExecuteSyscall_##name,
#include "ContextSyscall.def"
#undef DEFSYSCALL
		nullptr
};

// System call error codes
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

static misc::StringMap error_code_map =
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


void Context::ExecuteSyscall()
{
	// System call code 
	int code = regs.getRegister(7);
	if (code < 1 || code >= SyscallCodeCount)
		throw misc::Panic(misc::fmt("%s: invalid system call code (%d)", __FUNCTION__, code));

	// Debug 
	emulator->syscall_debug << misc::fmt("system call '%s' "
			"(code %d, inst %lld, pid %d)\n",
			Context::syscall_name[code],
			code,
			emulator->getNumInstructions(),
			pid);

	// Perform system call 
	ExecuteSyscallFn fn = execute_syscall_fn[code];
	int err = (this->*fn)();

	// Set return value in 'eax', except for 'sigreturn' system call. Also, if the
	// context got suspended, the wake up routine will set the return value.
	if (code != SyscallCode_sigreturn && !getState(ContextStateSuspended))
		regs.setRegister(0, err);

	// Debug 
	emulator->syscall_debug << misc::fmt("  ret=(%d, 0x%x)", err, err);
	if (err < 0 && err >= -SIM_ERRNO_MAX)
		emulator->syscall_debug << misc::fmt(", errno=%s)", error_code_map.MapValue(-err));
	emulator->syscall_debug << misc::fmt("\n");
}




//
// System call 'close' (code 2)
//

int Context::ExecuteSyscall_close()
{
	// Arguments 
	int guest_fd = regs.getRegister(0);
	int host_fd = file_table->getHostIndex(guest_fd);
	emulator->syscall_debug << misc::fmt("  guest_fd=%d\n", guest_fd);
	emulator->syscall_debug << misc::fmt("  host_fd=%d\n", host_fd);

	// Get file descriptor table entry. 
	comm::FileDescriptor *fd = file_table->getFileDescriptor(guest_fd);
	if (!fd)
		return -EBADF;

	// Close host file descriptor only if it is valid and not stdin/stdout/stderr. 
	if (host_fd > 2)
		close(host_fd);

	// Free guest file descriptor. This will delete the host file if it's a virtual file. 
	if (fd->getType() == comm::FileDescriptor::TypeVirtual)
		emulator->syscall_debug << misc::fmt("    host file '%s': temporary file deleted\n",
				fd->getPath().c_str());
	file_table->freeFileDescriptor(fd->getGuestIndex());

	// Success 
	return 0;
}


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
		emulator->syscall_debug << misc::fmt("syscall 'read' - "
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
		unsigned pbuf = regs.getRegister(1);
		int count = regs.getRegister(2);
		char *buf = new char[count];

		count = read(desc->getHostIndex(), buf, count);
		if (count < 0)
			misc::panic("%s: unexpected error in host 'read'",
					__FUNCTION__);

		// FIXME
		//regs.setRegister(7, count);
		memory->Write(pbuf, count, buf);
		delete buf;

		emulator->syscall_debug << misc::fmt("syscall 'read' - "
				"continue (pid %d)\n", pid);
		emulator->syscall_debug << misc::fmt("  return=0x%x\n", regs.getRegister(7));
		return true;
	}

	// Data is not ready. Launch host thread again
	host_thread_suspend_active = true;
	if (pthread_create(&host_thread_suspend, nullptr,
			&Context::HostThreadSuspend, this))
		misc::panic("%s: could not launch host thread", __FUNCTION__);
	return false;
}


// System call 'read' (code 3)
int Context::ExecuteSyscall_read()
{
	struct pollfd fds;

	// Arguments 
	int guest_fd = regs.getRegister(0);
	unsigned int buf_ptr = regs.getRegister(1);
	unsigned int count = regs.getRegister(2);
	emulator->syscall_debug << misc::fmt("  guest_fd=%d, buf_ptr=0x%x, count=0x%x\n",
			guest_fd, buf_ptr, count);

	// Get file descriptor 
	comm::FileDescriptor *fd = file_table->getFileDescriptor(guest_fd);
	if (!fd)
		return -EBADF;
	int host_fd = fd->getHostIndex();
	emulator->syscall_debug << misc::fmt("  host_fd=%d\n", host_fd);

	// Poll the file descriptor to check if read is blocking 
	char *buf = new char[count]();
	fds.fd = host_fd;
	fds.events = POLLIN;
	int err = poll(&fds, 1, 0);
	if (err < 0)
		misc::panic("%s: error executing 'poll'", __FUNCTION__);

	// Non-blocking read 
	if (fds.revents || (fd->getFlags() & O_NONBLOCK))
	{
		// Host system call 
		err = read(host_fd, buf, count);
		if (err == -1)
		{
			free(buf);
			return -errno;
		}

		// Write in guest memory 
		if (err > 0)
		{
			memory->Write(buf_ptr, err, buf);
			emulator->syscall_debug << misc::StringBinaryBuffer(buf,
					count, 40);
		}

		// Return number of read bytes 
		delete buf;
		return err;
	}

	// Blocking read - suspend thread 
	emulator->syscall_debug << misc::fmt("  blocking read - process suspended\n");
	syscall_read_fd = guest_fd;
	Suspend(&Context::SyscallReadCanWakeup, &Context::SyscallReadWakeup,
			ContextStateRead);
	emulator->ProcessEventsSchedule();

	// Free allocated buffer. Return value doesn't matter,
	// it will be overwritten when context wakes up from blocking call.
	delete buf;
	return 0;
}


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
		emulator->syscall_debug << misc::fmt("syscall 'write' - "
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
		unsigned pbuf = regs.getRegister(1);
		int count = regs.getRegister(2);
		char *buf = new char[count];

		memory->Read(pbuf, count, buf);
		count = write(desc->getHostIndex(), buf, count);
		if (count < 0)
			misc::panic("%s: unexpected error in host 'write'",
					__FUNCTION__);

		// FIXME
		//regs.setRegister(7, count);
		delete buf;

		emulator->syscall_debug << misc::fmt("syscall write - "
				"continue (pid %d)\n", pid);
		emulator->syscall_debug << misc::fmt("  return=0x%x\n", regs.getRegister(7));
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




//
// System call 'write' (code 4)
//

int Context::ExecuteSyscall_write()
{
	struct pollfd fds;

	// Arguments 
	int guest_fd = regs.getRegister(0);
	unsigned int buf_ptr = regs.getRegister(1);
	unsigned int count = regs.getRegister(2);
	emulator->syscall_debug << misc::fmt("  guest_fd=%d, buf_ptr=0x%x, count=0x%x\n",
			guest_fd, buf_ptr, count);

	// Get file descriptor 
	comm::FileDescriptor *desc = file_table->getFileDescriptor(guest_fd);
	if (!desc)
		return -EBADF;
	int host_fd = desc->getHostIndex();
	emulator->syscall_debug << misc::fmt("  host_fd=%d\n", host_fd);

	// Read buffer from memory 
	char *buf = new char[count]();
	memory->Read(buf_ptr,count, buf);
	emulator->syscall_debug << "  buf=\""
			<< misc::StringBinaryBuffer(buf, count, 40)
			<< "\"\n";

	// Poll the file descriptor to check if write is blocking 
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
	emulator->syscall_debug << misc::fmt(" blocking write - process suspended\n");
	syscall_write_fd = guest_fd;
	Suspend(&Context::SyscallWriteCanWakeup, &Context::SyscallWriteWakeup,
			ContextStateWrite);
	emulator->ProcessEventsSchedule();

	// Return value doesn't matter here. It will be overwritten when the
	// context wakes up after blocking call.
	delete buf;
	return 0;
}




//
// System call 'open' (code 5)
//

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
	emulator->syscall_debug << misc::fmt("    host file '%s' opened: "
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
	emulator->syscall_debug << misc::fmt("    host device '%s' opened: "
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
	unsigned int file_name_ptr = regs.getRegister(0);
	int flags = regs.getRegister(1);
	int mode = regs.getRegister(2);
	std::string file_name = memory->ReadString(file_name_ptr);
	std::string full_path = getFullPath(file_name);
	emulator->syscall_debug << misc::fmt("  filename='%s' flags=0x%x, mode=0x%x\n",
			file_name.c_str(), flags, mode);
	emulator->syscall_debug << misc::fmt("  fullpath='%s'\n", full_path.c_str());
	emulator->syscall_debug << misc::fmt("  flags=%s\n",
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
		emulator->syscall_debug << "    warning: unhandled virtual file\n";
	}

	// Regular file.
	int host_fd = open(full_path.c_str(), flags, mode);
	if (host_fd == -1)
		return -errno;

	// File opened, create a new file descriptor.
	comm::FileDescriptor *desc = file_table->newFileDescriptor(
			comm::FileDescriptor::TypeRegular,
			host_fd, full_path, flags);
	emulator->syscall_debug << misc::fmt("    file descriptor opened: "
			"guest_fd=%d, host_fd=%d\n",
			desc->getGuestIndex(), desc->getHostIndex());

	// Return guest descriptor index
	return desc->getGuestIndex();
}




//
// System call 'brk' (code 45)
//

int Context::ExecuteSyscall_brk()
{

	unsigned int old_heap_break;
	unsigned int new_heap_break;
	unsigned int size;

	unsigned int old_heap_break_aligned;
	unsigned int new_heap_break_aligned;

	// Arguments 
	new_heap_break = regs.getRegister(0);
	old_heap_break = memory->getHeapBreak();
	emulator->syscall_debug << misc::fmt(
		"  newbrk=0x%x (previous brk was 0x%x)\n",
		new_heap_break, old_heap_break);

	// Align 
	new_heap_break_aligned = misc::RoundUp(new_heap_break, memory->PageSize);
	old_heap_break_aligned = misc::RoundUp(old_heap_break, memory->PageSize);

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
			if (memory->MapSpace(old_heap_break_aligned, size) != old_heap_break_aligned)
				misc::Panic(misc::fmt("%s: out of memory", __FUNCTION__));
			memory->Map(old_heap_break_aligned, size,
				memory->AccessRead | memory->AccessWrite);
		}
		memory->setHeapBreak(new_heap_break);
		emulator->syscall_debug << misc::fmt("  heap grows %u bytes\n",
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
		emulator->syscall_debug << misc::fmt("  heap shrinks %u bytes\n",
				old_heap_break - new_heap_break);
		return new_heap_break;
	}

	// Heap stays the same 
	return 0;
}




//
// System call 'gettimeofday' (code 78)
//

int Context::ExecuteSyscall_gettimeofday()
{
	// Arguments 
	unsigned int tv_ptr = regs.getRegister(0);
	unsigned int tz_ptr = regs.getRegister(1);
	emulator->syscall_debug << misc::fmt("  tv_ptr=0x%x, tz_ptr=0x%x\n",
			tv_ptr, tz_ptr);

	// Host call 
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);

	// Write time value 
	if (tv_ptr)
	{
		memory->Write(tv_ptr, 4, (char *)&tv.tv_sec);
		memory->Write(tv_ptr + 4, 4, (char *)&tv.tv_usec);
	}

	// Write time zone 
	if (tz_ptr)
	{
		memory->Write(tz_ptr, 4, (char *)&tz.tz_minuteswest);
		memory->Write(tz_ptr + 4, 4, (char *)&tz.tz_dsttime);
	}

	// Return 
	return 0;
}




//
// System call 'mmap' (code 90)
//

static const unsigned mmap_base_address = 0xb7fb0000;

static misc::StringMap mmap_prot_map =
{
		{ "PROT_READ",       0x1 },
		{ "PROT_WRITE",      0x2 },
		{ "PROT_EXEC",       0x4 },
		{ "PROT_SEM",        0x8 },
		{ "PROT_GROWSDOWN",  0x01000000 },
		{ "PROT_GROWSUP",    0x02000000 }
};

static misc::StringMap mmap_flags_map =
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
	// If it is set, the 'fd' parameter is ignored. */
	if (flags & MAP_ANONYMOUS)
		host_fd = -1;

	// 'addr' and 'offset' must be aligned to page size boundaries.
	// 'len' is rounded up to page boundary. */
	if (offset & ~mem::Memory::PageMask)
		misc::fatal("%s: unaligned offset", __FUNCTION__);
	if (addr & ~mem::Memory::PageMask)
		misc::fatal("%s: unaligned address", __FUNCTION__);
	unsigned int len_aligned = misc::RoundUp(len, mem::Memory::PageSize);

	// Find region for allocation 
	if (flags & MAP_FIXED)
	{
		// If MAP_FIXED is set, the 'addr' parameter must be obeyed, and is not just a
		// hint for a possible base address of the allocated range. */
		if (!addr)
			misc::fatal("%s: no start specified for fixed mapping", __FUNCTION__);

		// Any allocated page in the range specified by 'addr' and 'len'
		// must be discarded.
		memory->Unmap(addr, len_aligned);
	}
	else
	{
		if (!addr || memory->MapSpaceDown(addr, len_aligned) != addr)
			addr = mmap_base_address;
		addr = memory->MapSpaceDown(addr, len_aligned);
		if (addr == (unsigned int)-1)
			misc::fatal("%s: out of guest memory", __FUNCTION__);
	}

	// Allocation of memory 
	memory->Map(addr, len_aligned, perm);

	// Host mapping 
	if (host_fd >= 0)
	{
		// Save previous position 
		unsigned int last_pos = lseek(host_fd, 0, SEEK_CUR);
		lseek(host_fd, offset, SEEK_SET);

		// Read pages 
		assert(len_aligned % mem::Memory::PageSize == 0);
		assert(addr % mem::Memory::PageSize == 0);
		unsigned int curr_addr = addr;
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




//
// System call 'munmap' (code 91)
//

int Context::ExecuteSyscall_munmap()
{
	// Arguments 
	unsigned int addr = regs.getRegister(0);
	unsigned int size = regs.getRegister(1);
	emulator->syscall_debug << misc::fmt("  addr=0x%x, size=0x%x\n", addr, size);

	// Restrictions 
	if (addr & (mem::Memory::PageSize - 1))
		misc::fatal("%s: address not aligned", __FUNCTION__);

	// Unmap 
	unsigned int size_aligned = misc::RoundUp(size, mem::Memory::PageSize);
	memory->Unmap(addr, size_aligned);

	// Return 
	return 0;
}




//
// System call 'mmap2' (code 192)
//

int Context::ExecuteSyscall_mmap2()
{
	// Arguments 
	unsigned int addr = regs.getRegister(0);
	unsigned int len = regs.getRegister(1);
	int prot = regs.getRegister(2);
	int flags = regs.getRegister(3);
	int guest_fd = regs.getRegister(4);
	int offset = regs.getRegister(5);

	// Arm error handling for non-tls supported pointer mismatch 
	if(!len)
	{
		len = 0x1000;
	}

	// Debug 
	emulator->syscall_debug << misc::fmt("  addr=0x%x, len=%u, prot=0x%x, flags=0x%x, guest_fd=%d, offset=0x%x\n",
		addr, len, prot, flags, guest_fd, offset);
	emulator->syscall_debug << misc::fmt("  prot=%s, flags=%s\n",
			mmap_prot_map.MapValue(prot), mmap_flags_map.MapValue(flags));

	// System calls 'mmap' and 'mmap2' only differ in the interpretation of
	// argument 'offset'. Here, it is given in memory pages.
	return SyscallMmapAux(addr, len, prot, flags, guest_fd, offset << mem::Memory::LogPageSize);
}




//
// System call 'newuname' (code 122)
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
	"3.1.9-1.fc16.armv7"
	"#1 Fri Jan 13 16:37:42 UTC 2012",
	"armv7"
	""
};

int Context::ExecuteSyscall_newuname()
{
	unsigned int utsname_ptr;

	// Arguments 
	utsname_ptr = regs.getRegister(0);
	emulator->syscall_debug << misc::fmt("  putsname=0x%x\n", utsname_ptr);
	emulator->syscall_debug << misc::fmt("  sysname='%s', nodename='%s'\n",
			sim_utsname.sysname, sim_utsname.nodename);
	emulator->syscall_debug << misc::fmt("  relaese='%s', version='%s'\n",
			sim_utsname.release, sim_utsname.version);
	emulator->syscall_debug << misc::fmt("  machine='%s', domainname='%s'\n",
			sim_utsname.machine, sim_utsname.domainname);

	// Return structure 
	memory->Write(utsname_ptr, sizeof sim_utsname, (char *)&sim_utsname);
	return 0;
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

	Emulator *emulator = Emulator::getInstance();
	emulator->syscall_debug << misc::fmt("  stat64 structure:\n");
	emulator->syscall_debug << misc::fmt("    dev=%lld, ino=%lld, mode=%d, nlink=%d\n",
		guest->dev, guest->ino, guest->mode, guest->nlink);
	emulator->syscall_debug << misc::fmt("    uid=%d, gid=%d, rdev=%lld\n",
		guest->uid, guest->gid, guest->rdev);
	emulator->syscall_debug << misc::fmt("    size=%lld, blksize=%d, blocks=%lld\n",
		guest->size, guest->blksize, guest->blocks);
}




//
// System call 'fstat64' (code 197)
//

int Context::ExecuteSyscall_fstat64()
{
	// Arguments 
	int fd = regs.getRegister(0);
	unsigned int statbuf_ptr = regs.getRegister(1);
	emulator->syscall_debug << misc::fmt("  fd=%d, statbuf_ptr=0x%x\n", fd, statbuf_ptr);

	// Get host descriptor 
	int host_fd = file_table->getHostIndex(fd);;
	emulator->syscall_debug << misc::fmt("  host_fd=%d\n", host_fd);

	// Host call 
	struct stat statbuf;
	int err = fstat(host_fd, &statbuf);
	if (err == -1)
		return -errno;

	// Return 
	struct sim_stat64_t sim_statbuf;
	sys_stat_host_to_guest(&sim_statbuf, &statbuf);
	memory->Write(statbuf_ptr, sizeof sim_statbuf, (char *)&sim_statbuf);
	return 0;
}




//
// System call 'getuid' (code 199)
//

int Context::ExecuteSyscall_getuid()
{
	return getuid();
}




//
// System call 'getgid' (code 200)
//

int Context::ExecuteSyscall_getgid()
{
	return getgid();
}




//
// System call 'geteuid' (code 201)
//

int Context::ExecuteSyscall_geteuid()
{
	return geteuid();
}




//
// System call 'getegid' (code 202)
//

int Context::ExecuteSyscall_getegid()
{
	return getegid();
}




//
// System call 'exit_group' (code 252)
//

int Context::ExecuteSyscall_exit_group()
{
	// Arguments
	int status = regs.getRegister(0);
	emulator->syscall_debug << misc::fmt("  status=%d\n", status);

	// Finish
	FinishGroup(status);
	return 0;
}




//
// System call 'ARM_set_tls' (code 330)
//

int Context::ExecuteSyscall_ARM_set_tls()
{
	// Arguments
	unsigned int newtls = regs.getRegister(0);

	// Set the tls value
	regs.getCP15().c13_tls3 = newtls;

	// Return
	return 0;

}


int Context::ExecuteSyscall_times()
{
	// Get the buffer address in the guest memory
	unsigned int tms_buff = regs.getRegister(0);
	emulator->syscall_debug << misc::fmt("  buff address: 0x%x\n", tms_buff);

	// Get the time value by using the host syscall
	struct tms sim_tmsbuff;
	clock_t ret =  times(&sim_tmsbuff);

	// Write guest memory
	memory->Write(tms_buff, 4, (char *)&(sim_tmsbuff.tms_utime));
	memory->Write(tms_buff + 4, 4, (char *)&(sim_tmsbuff.tms_stime));
	memory->Write(tms_buff + 8, 4, (char *)&(sim_tmsbuff.tms_cutime));
	memory->Write(tms_buff + 12, 4, (char *)&(sim_tmsbuff.tms_cstime));

	return (int)ret;
}


int Context::ExecuteSyscall_restart_syscall()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_exit()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fork()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_waitpid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_creat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_link()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_unlink()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_execve()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_chdir()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_time()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mknod()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_chmod()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_lchown16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_17()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_stat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_lseek()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getpid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mount()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_oldumount()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setuid16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getuid16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_stime()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ptrace()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_alarm()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fstat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_pause()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_utime()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_31()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_32()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_access()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_nice()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_35()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sync()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_kill()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_rename()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mkdir()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_rmdir()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_dup()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_pipe()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_44()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setgid16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getgid16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_signal()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_geteuid16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getegid16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_acct()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_umount()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_53()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ioctl()
{
	// Read arguments
	int guest_fd = regs.getRegister(0);
	unsigned cmd = regs.getRegister(1);
	unsigned arg = regs.getRegister(2);
	
	// Debug arguments
	Emulator::syscall_debug << misc::fmt(
			"  guest_fd=%d, "
			"cmd=0x%x, "
			"arg=0x%x\n",
			guest_fd,
			cmd,
			arg);
	
	// File descriptor 
	comm::FileDescriptor *desc = file_table->getFileDescriptor(guest_fd);
	if (!desc)
		return -EBADF;
	
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

	// Not supported
	throw misc::Panic(misc::fmt("Not implement for cmd = 0x%x", cmd));
}


int Context::ExecuteSyscall_fcntl()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_56()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setpgid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_58()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_olduname()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_umask()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_chroot()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ustat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_dup2()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getppid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getpgrp()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setsid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sigaction()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sgetmask()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ssetmask()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setreuid16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setregid16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sigsuspend()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sigpending()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sethostname()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setrlimit()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_old_getrlimit()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getrusage()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_settimeofday()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getgroups16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setgroups16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_oldselect()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_symlink()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_lstat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_readlink()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_uselib()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_swapon()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_reboot()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_readdir()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mmap()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_truncate()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ftruncate()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fchmod()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fchown16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getpriority()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setpriority()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_98()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_statfs()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fstatfs()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ioperm()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_socketcall()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_syslog()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setitimer()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getitimer()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_newstat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_newlstat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_newfstat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_uname()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_iopl()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_vhangup()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_112()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_vm86old()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_wait4()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_swapoff()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sysinfo()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ipc()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fsync()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sigreturn()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_clone()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setdomainname()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_modify_ldt()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_adjtimex()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mprotect()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sigprocmask()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_127()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_init_module()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_delete_module()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_130()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_quotactl()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getpgid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fchdir()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_bdflush()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sysfs()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_personality()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_137()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setfsuid16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setfsgid16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_llseek()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getdents()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_select()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_flock()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_msync()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_readv()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_writev()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getsid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fdatasync()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sysctl()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mlock()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_munlock()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mlockall()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_munlockall()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sched_setparam()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sched_getparam()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sched_setscheduler()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sched_getscheduler()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sched_yield()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sched_get_priority_max()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sched_get_priority_min()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sched_rr_get_interval()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_nanosleep()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mremap()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setresuid16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getresuid16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_vm86()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_167()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_poll()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_nfsservctl()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setresgid16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getresgid16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_prctl()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_rt_sigreturn()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_rt_sigaction()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_rt_sigprocmask()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_rt_sigpending()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_rt_sigtimedwait()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_rt_sigqueueinfo()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_rt_sigsuspend()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_pread64()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_pwrite64()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_chown16()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getcwd()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_capget()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_capset()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sigaltstack()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sendfile()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_188()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_189()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_vfork()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getrlimit()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_truncate64()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ftruncate64()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_stat64()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_lstat64()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_lchown()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setreuid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setregid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getgroups()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setgroups()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fchown()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setresuid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getresuid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setresgid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getresgid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_chown()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setuid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setgid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setfsuid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setfsgid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_pivot_root()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mincore()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_madvise()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getdents64()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fcntl64()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_222()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_223()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_gettid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_readahead()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_setxattr()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_lsetxattr()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fsetxattr()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getxattr()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_lgetxattr()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fgetxattr()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_listxattr()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_llistxattr()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_flistxattr()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_removexattr()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_lremovexattr()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fremovexattr()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_tkill()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sendfile64()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_futex()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sched_setaffinity()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sched_getaffinity()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_set_thread_area()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_get_thread_area()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_io_setup()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_io_destroy()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_io_getevents()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_io_submit()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_io_cancel()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fadvise64()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_251()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_lookup_dcookie()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_epoll_create()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_epoll_ctl()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_epoll_wait()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_remap_file_pages()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_set_tid_address()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_timer_create()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_timer_settime()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_timer_gettime()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_timer_getoverrun()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_timer_delete()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_clock_settime()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_clock_gettime()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_clock_getres()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_clock_nanosleep()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_statfs64()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fstatfs64()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_tgkill()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_utimes()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fadvise64_64()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_273()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mbind()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_get_mempolicy()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_set_mempolicy()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mq_open()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mq_unlink()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mq_timedsend()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mq_timedreceive()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mq_notify()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mq_getsetattr()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_kexec_load()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_waitid()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ni_syscall_285()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_add_key()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_request_key()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_keyctl()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ioprio_set()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ioprio_get()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_inotify_init()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_inotify_add_watch()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_inotify_rm_watch()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_migrate_pages()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_openat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mkdirat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_mknodat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fchownat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_futimesat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fstatat64()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_unlinkat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_renameat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_linkat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_symlinkat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_readlinkat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fchmodat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_faccessat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_pselect6()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_ppoll()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_unshare()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_set_robust_list()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_get_robust_list()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_splice()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_sync_file_range()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_tee()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_vmsplice()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_move_pages()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_getcpu()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_epoll_pwait()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_utimensat()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_signalfd()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_timerfd()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_eventfd()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_fallocate()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_opencl()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_glut()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_opengl()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_cuda()
{
	__UNIMPLEMENTED__
}


int Context::ExecuteSyscall_clrt()
{
	__UNIMPLEMENTED__
}

} // namespace ARM
