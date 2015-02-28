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

#ifndef ARCH_X86_EMU_SIGNAL_H
#define ARCH_X86_EMU_SIGNAL_H

#include <cassert>

#include <lib/cpp/Bitmap.h>
#include <lib/cpp/String.h>
#include <memory/Memory.h>

#include "Regs.h"


namespace x86
{


class Regs;


extern const misc::StringMap signal_map;
extern const misc::StringMap signal_handler_flags_map;


/// Structure representing the signal stack frame set up when a signal handler
/// is launched.
struct SignalFrame
{
	// Pointer to return code
	unsigned ret_code_ptr;

	// Signal received
	unsigned sig;

	unsigned gs;
	unsigned fs;
	unsigned es;
	unsigned ds;

	unsigned edi;
	unsigned esi;
	unsigned ebp;
	unsigned esp;

	unsigned ebx;
	unsigned edx;
	unsigned ecx;
	unsigned eax;

	unsigned trapno;
	unsigned err;
	unsigned eip;
	unsigned cs;

	unsigned eflags;
	unsigned esp_at_signal;
	unsigned ss;
	unsigned pfpstate;  // Pointer to floating-point state
	unsigned oldmask;
	unsigned cr2;
};


/// Signal return code. The return address in the signal handler points
/// to this code, which performs system call 'sigreturn'. The disassembled
/// corresponding code is:
///     mov eax, 0x77
///     int 0x80
static const char signal_ret_code[] = "\x58\xb8\x77\x00\x00\x00\xcd\x80";


/// Set of signals
class SignalSet
{
	misc::Bitmap bitmap;
public:

	/// Constructor
	SignalSet() : bitmap(64) { }

	/// Add a signal to the set. The value of \a sig must be between 1 and
	/// 64. If the value is out of range, the call is ignored.
	void Add(int sig) {
		if (misc::inRange(sig, 1, 64))
			bitmap.Set(sig - 1);
	}

	/// Delete a signal from the set. The value of \a sig must be between 1
	/// and 64. If the value is out of range, the call is ignored.
	void Delete(int sig) {
		if (misc::inRange(sig, 1, 64))
			bitmap.Set(sig - 1, false);
	}

	/// Return whether a signal is in the set. The value of \a sig must be
	/// between 1 and 64. If the value is out of range, return \c false.
	bool isMember(int sig) const {
		return misc::inRange(sig, 1, 64) ?
				bitmap.Test(sig - 1) : false;
	}

	/// Return a constant reference the bitmap representing the signal set
	const misc::Bitmap &getBitmap() const { return bitmap; }
	
	/// Return the bitmap representing the signal set
	misc::Bitmap &getBitmap() { return bitmap; }
	
	/// Return \c true if any signal is set
	bool Any() const { return bitmap.Any(); }

	/// Return \c true if no signal is set
	bool None() const { return bitmap.None(); }
	
	/// Operator "~"
	SignalSet operator~() const {
		SignalSet set = *this;
		set.bitmap.Flip();
		return set;
	}
	
	/// Operator "&"
	SignalSet operator&(const SignalSet &set) const {
		SignalSet result;
		result.bitmap = bitmap & set.bitmap;
		return result;
	}

	/// Operator "|"
	SignalSet operator|(const SignalSet &set) const {
		SignalSet result;
		result.bitmap = bitmap | set.bitmap;
		return result;
	}

	/// Operator "^"
	SignalSet operator^(const SignalSet &set) const {
		SignalSet result;
		result.bitmap = bitmap ^ set.bitmap;
		return result;
	}

	/// Operator "=="
	bool operator==(const SignalSet &set) const {
		return bitmap == set.bitmap;
	}
	
	/// Operator "&="
	SignalSet &operator&=(const SignalSet &set) {
		bitmap &= set.bitmap;
		return *this;
	}

	/// Operator "|="
	SignalSet &operator|=(const SignalSet &set) {
		bitmap |= set.bitmap;
		return *this;
	}

	/// Operator "^="
	SignalSet &operator^=(const SignalSet &set) {
		bitmap ^= set.bitmap;
		return *this;
	}

	/// Dump into output stream, or \c std::cout if \a os is omitted.
	void Dump(std::ostream &os = std::cout) const;

	/// Same as Dump()
	friend std::ostream &operator<<(std::ostream &os,
			const SignalSet &set) {
		set.Dump(os);
		return os;
	}

	/// Read signal set from memory
	void ReadFromMemory(mem::Memory *memory, unsigned address) {
		assert(bitmap.getSizeInBytes() == 8);
		memory->Read(address, 8, bitmap.getBuffer());
	}

	/// Write signal set to memory
	void WriteToMemory(mem::Memory *memory, unsigned address) {
		assert(bitmap.getSizeInBytes() == 8);
		memory->Write(address, 8, bitmap.getBuffer());
	}
};


/// Signal mask table. Each context has its own table, including parent and
/// child contexts.
class SignalMaskTable
{
	// Masks of pending, blocks, and backed up blocked signals
	SignalSet pending;
	SignalSet blocked;
	SignalSet backup;

	// Backup of register file while executing signal handler
	std::unique_ptr<Regs> regs;

	// Base address of a memory page allocated for execution of return code
	unsigned ret_code_ptr;

public:

	/// Store a backup copy of the register file passed as an argument.
	void setRegs(const Regs &regs) { this->regs.reset(new Regs(regs)); }

	/// Return a constant reference to the registers storing a copy. Before
	/// calling this function, the user has to make sure that registers have
	/// been stored with a previous call to setRegs().
	const Regs &getRegs() {
		assert(regs.get());
		return *regs;
	}

	/// Free the copy of the register file stored. The caller must be sure
	/// that a register file was previously stored with a call to setRegs().
	/// The call to freeRegs() is optional for optimization. Not calling it
	/// will not cause any memory leak.
	void freeRegs() {
		assert(regs.get());
		regs.reset();
	}

	/// Set the address where the return code can be found.
	void setRetCodePtr(unsigned ret_code_ptr) {
			this->ret_code_ptr = ret_code_ptr;
	}

	/// Return reference to pending signal mask
	const SignalSet &getPending() const { return pending; }
	SignalSet &getPending() { return pending; }

	/// Return reference to blocked signal mask
	const SignalSet &getBlocked() const { return blocked; }
	SignalSet &getBlocked() { return blocked; }

	/// Set the blocked signal mask
	void setBlocked(const SignalSet &blocked) { this->blocked = blocked; }

	/// Save a copy of the blocked signal mask
	void BackupBlockedSignals() { backup = blocked; }

	/// Restore the blocked signal mask previously backed up with a call to
	/// BackupBlockedSignals()
	void RestoreBlockedSignals() { blocked = backup; }

	/// Return address where the return code can be found.
	unsigned getRetCodePtr() const { return ret_code_ptr; }
};


/// Signal handler information. This structure corresponds to the Unix \c
/// sigaction data structure.
class SignalHandler
{
	unsigned handler;
	unsigned flags;
	unsigned restorer;
	SignalSet mask;

public:

	/// Constructor
	SignalHandler() : handler(0), flags(0), restorer(0) { }

	/// Dump information about signal handler
	void Dump(std::ostream &os = std::cout) const;

	/// Same as Dump()
	friend std::ostream &operator<<(std::ostream &os,
			const SignalHandler &handler) {
		handler.Dump(os);
		return os;
	}

	/// Return the address in the guest program where the handler for the
	/// signal has been installed.
	unsigned getHandler() const { return handler; }

	/// Return the flags associated with the signal handler
	unsigned getFlags() const { return flags; }

	/// Return the mask associated with the signal handler
	SignalSet &getMask() { return mask; }

	/// Read the content of the signal handler from memory
	void ReadFromMemory(mem::Memory *memory, unsigned address);

	/// Write the content of the signal handler to memory
	void WriteToMemory(mem::Memory *memory, unsigned address);
};


/// Table of signal handlers. Multiple contexts can share the same time, so they
/// will be conveniently referenced using \c std::shared_ptr.
class SignalHandlerTable
{
	// Table of signal handlers
	SignalHandler signal_handler[64];

public:

	/// Return signal handler for signal \a sig, an integer value between 1
	/// and 64. This number should not be out of range.
	SignalHandler *getSignalHandler(int sig) {
		assert(misc::inRange(sig, 1, 64));
		return &signal_handler[sig - 1];
	}
};


}  // namespace x86

#endif

