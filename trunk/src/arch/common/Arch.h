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
 *  You should have received as copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_COMMON_ARCH_H
#define ARCH_COMMON_ARCH_H

#include <list>
#include <memory>

#include <lib/cpp/String.h>


namespace comm
{

/// Type of simulation for each architecture
enum ArchSimKind
{
	ArchSimInvalid = 0,
	ArchSimFunctional,
	ArchSimDetailed
};

/// String map for type ArchSimKind
extern misc::StringMap arch_sim_kind_map;


/// Class representing one of the supported architectures on Multi2Sim (x86,
/// ARM, Southern Islands, etc.)
class Arch
{
	// Name of architecture (x86, ARM, etc.)
	std::string name;
	
	// Prefix used in command-line options and in code variables and functions.
	// E.g., 'evg' for Evergreen, or 'si' for Southern Islands.
	std::string prefix;

public:

	/// Constructor of a new architecture. New architectures should be
	/// created only through ArchPool::Register(), not directly by the
	/// invocation of this constructor.
	Arch(const std::string &name, const std::string &prefix);
};


/// Class containing a pool of all registered architectures. This class is a
/// singleton, and the only instance of it can be obtained through function
/// getInstance().
class ArchPool
{
	// Unique instance of the class
	static std::unique_ptr<ArchPool> instance;

	// List of architectures
	std::list<std::unique_ptr<Arch>> arch_list;

	// Private constructor for singleton
	ArchPool() { }

public:

	/// Return a unique instance of the singleton
	static ArchPool *getInstance();

	/// Register an architecture
	void Register(const std::string &name, const std::string &prefix);
};


}  // namespace comm

#endif

