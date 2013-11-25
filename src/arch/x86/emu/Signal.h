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

#include "lib/cpp/Bitmap.h"
#include "lib/cpp/String.h"


namespace x86
{


class Regs;


#if 0

// Run a signal handler
void X86ContextRunSignalHandler(X86Context *self, int sig);

// Return from a signal handler
void X86ContextReturnFromSignalHandler(X86Context *self);

void X86ContextCheckSignalHandler(X86Context *self);

/* Check any pending signal, and run the corresponding signal handler by
 * considering that the signal interrupted a system call ('syscall_intr').
 * This has the following implication on the return address from the signal
 * handler:
 *   -If flag 'SA_RESTART' is set for the handler, the return address is the
 *    system call itself, which must be repeated.
 *   -If flag 'SA_RESTART' is not set, the return address is the instruction
 *    next to the system call, and register 'eax' is set to -EINTR. */
void X86ContextCheckSignalHandlerIntr(X86Context *self);

#endif


extern const misc::StringMap signal_map;
extern const misc::StringMap signal_action_flags_map;

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
		if (misc::InRange(sig, 1, 64))
			bitmap.Set(sig - 1);
	}

	/// Delete a signal from the set. The value of \a sig must be between 1
	/// and 64. If the value is out of range, the call is ignored.
	void Delete(int sig) {
		if (misc::InRange(sig, 1, 64))
			bitmap.Set(sig - 1, false);
	}

	/// Return whether a signal is in the set. The value of \a sig must be
	/// between 1 and 64. If the value is out of range, return \c false.
	bool isMember(int sig) const {
		return misc::InRange(sig, 1, 64) ?
				bitmap.Test(sig - 1) : false;
	}

	/// Dump into output stream, or \c std::cout if \a os is omitted.
	void Dump(std::ostream &os = std::cout) const;

	/// Same as Dump()
	friend std::ostream &operator<<(std::ostream &os,
			const SignalSet &set) {
		set.Dump(os);
		return os;
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

	// Base address of a memory page allocated for retcode execution
	unsigned retcode_ptr;
};


/// Table of signal handlers. Multiple contexts can share the same time, so they
/// will be conveniently referenced using \c std::shared_ptr.
class SignalHandlerTable
{
	// Signal handlers
	struct SignalHandler
	{
		unsigned handler;
		unsigned flags;
		unsigned restorer;
		SignalSet mask;
	};
	
	// Table of signal handlers
	SignalHandler signal_handler[64];
};



#if 0
/*
 * Public Functions
 */

void x86_sigaction_dump(struct x86_sigaction_t *sim_sigaction, FILE *f);
void x86_sigset_dump(unsigned long long sim_sigset, FILE *f);


#endif

}  // namespace x86

#endif

