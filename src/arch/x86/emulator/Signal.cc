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

#include "Signal.h"


namespace x86
{

const misc::StringMap signal_map =
{
	{ "SIGHUP",           1 },
	{ "SIGINT",           2 },
	{ "SIGQUIT",          3 },
	{ "SIGILL",           4 },
	{ "SIGTRAP",          5 },
	{ "SIGABRT",          6 },
	{ "SIGBUS",           7 },
	{ "SIGFPE",           8 },
	{ "SIGKILL",          9 },
	{ "SIGUSR1",         10 },
	{ "SIGSEGV",         11 },
	{ "SIGUSR2",         12 },
	{ "SIGPIPE",         13 },
	{ "SIGALRM",         14 },
	{ "SIGTERM",         15 },
	{ "SIGSTKFLT",       16 },
	{ "SIGCHLD",         17 },
	{ "SIGCONT",         18 },
	{ "SIGSTOP",         19 },
	{ "SIGTSTP",         20 },
	{ "SIGTTIN",         21 },
	{ "SIGTTOU",         22 },
	{ "SIGURG",          23 },
	{ "SIGXCPU",         24 },
	{ "SIGXFSZ",         25 },
	{ "SIGVTALRM",       26 },
	{ "SIGPROF",         27 },
	{ "SIGWINCH",        28 },
	{ "SIGIO",           29 },
	{ "SIGPWR",          30 },
	{ "SIGSYS",          31 },
	{ "SIGWAITING",      32 },
	{ "SIGLWP",          33 }
};


const misc::StringMap signal_handler_flags_map =
{
	{ "SA_NOCLDSTOP",       0x00000001u },
	{ "SA_NOCLDWAIT",       0x00000002u },
	{ "SA_SIGINFO",         0x00000004u },
	{ "SA_ONSTACK",         0x08000000u },
	{ "SA_RESTART",         0x10000000u },
	{ "SA_NODEFER",         0x40000000u },
	{ "SA_RESETHAND", (int) 0x80000000u },
	{ "SA_INTERRUPT",       0x20000000u },
	{ "SA_RESTORER",        0x04000000u }
};


void SignalSet::Dump(std::ostream &os) const
{
	// All signals set
	if (!bitmap.CountZeros())
	{
		os << "{ <all> }";
		return;
	}

	// Print signals
	os << "{";
	std::string comma = " ";
	for (int i = 1; i <= 64; i++)
	{
		// Signal not present
		if (!isMember(i))
			continue;

		// Signal is present
		if (i < 32)
			os << comma << signal_map.MapValue(i);
		else
			os << comma << i;
		comma = ", ";
	}
	os << " }";
}


void SignalHandler::ReadFromMemory(mem::Memory *memory, unsigned address)
{
	memory->Read(address, 4, (char *) &handler);
	memory->Read(address + 4, 4, (char *) &flags);
	memory->Read(address + 8, 4, (char *) &restorer);
	mask.ReadFromMemory(memory, address + 12);
}


void SignalHandler::WriteToMemory(mem::Memory *memory, unsigned address)
{
	memory->Write(address, 4, (char *) &handler);
	memory->Write(address + 4, 4, (char *) &flags);
	memory->Write(address + 8, 4, (char *) &restorer);
	mask.WriteToMemory(memory, address + 12);
}


void SignalHandler::Dump(std::ostream &os) const
{
	os << misc::fmt("handler = 0x%x, ", handler)
			<< misc::fmt("flags = 0x%x, ", flags)
			<< misc::fmt("restorer = 0x%x, ", restorer)
			<< "mask = " << mask;
}


}  // namespace x86

