/*
 *  Multi2Sim
 *  Copyright (C) 2015  Rafael Ubal (ubal@ece.neu.edu)
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
 *  You should have received as copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_COMMON_CONTEXT_H
#define ARCH_COMMON_CONTEXT_H

#include <string>


namespace comm
{

// Forward declaration
class Emulator;


/// Class capturing basic functionality for a CPU context (i.e., a simulated
/// software thread of execution). This functionality is common for the
/// context classes in all CPU architectures.
class Context
{
	// Counter used to assign context IDs
	static int id_counter;

	// Unique context identifier, initialized in constructor
	int id;

	// Context name, initialized in constructor
	std::string name;

	// Associated emulator, initialized in constructor
	Emulator *emulator;

public:

	/// Constructor
	Context(Emulator *emulator);

	/// Return a unique integer identifier for this context. Identifiers
	/// are assigned to contexts starting at 1000, and in common for all
	/// architectures.
	int getId() const { return id; }

	/// Return the name of the context, formed of the name of the
	/// architecture, the word 'context', and its identifier.
	const std::string &getName() const { return name; }

	/// Suspend the context. The context must be in a non-suspended state,
	/// or a panic exception will occur.
	virtual void Suspend();

	/// Wake up the context. The context must be in a suspended state, or
	/// a panic exception will occur.
	virtual void Wakeup();

	/// Return whether the context is suspended.
	virtual bool isSuspended();
};


}  // namespace comm

#endif

