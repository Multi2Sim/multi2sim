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

#ifndef ARCH_MIPS_EMU_SIGNAL_H
#define ARCH_MIPS_EMU_SIGNAL_H

#include <lib/cpp/Bitmap.h>
#include <lib/cpp/String.h>
#include <mem-system/Memory.h>

#include "Regs.h"

namespace MIPS
{
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
class SignalHandlerTable{
	// Table of signal handlers
	SignalHandler signal_handler[64];

};
}

#endif
