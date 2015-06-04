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
#include <map>
#include <memory>

#include <lib/cpp/String.h>


namespace comm
{

// Forward declarations
class Asm;
class Emu;
class Timing;

/// Class representing one of the supported architectures on Multi2Sim (x86,
/// ARM, Southern Islands, etc.)
class Arch
{

public:

	/// Type of simulation for each architecture
	enum SimKind
	{
		SimInvalid = 0,
		SimFunctional,
		SimDetailed
	};

	/// String map for SimKind
	static const misc::StringMap SimKindMap;

private:

	// Name of architecture (x86, ARM, etc.)
	std::string name;

	// Disassembler
	Asm *as = nullptr;

	// Emulator
	Emu *emu = nullptr;

	// Timing simulator
	Timing *timing = nullptr;

	// Simulation kind
	SimKind sim_kind = SimFunctional;

	// True if last iteration had an active simulation
	bool active = false;

public:

	/// Constructor of a new architecture. New architectures should be
	/// created only through ArchPool::Register(), not directly by the
	/// invocation of this constructor.
	///
	/// \param name
	///	Name of the architecture
	///
	Arch(const std::string &name) : name(name) { }

	/// Return the name of the architecture
	const std::string &getName() const { return name; }

	/// Return the associated disassembler
	Asm *getAsm() const { return as; }

	/// Return the associated emulator
	Emu *getEmu() const { return emu; }

	/// Return the associated timing simulator
	Timing *getTiming() const { return timing; }

	/// Associate a disassembler. This function should only be invoked
	/// internally by ArchPool::RegisterDisassembler()
	void setAsm(Asm *as)
	{
		assert(!this->as);
		this->as = as;
	}

	/// Associate an emulator. This function should only be invoked
	/// internally by ArchPool::RegisterEmulator()
	void setEmu(Emu *emu)
	{
		assert(!this->emu);
		this->emu = emu;
	}

	/// Associate a timing simulator. This function should only be invoked
	/// internally by ArchPool::RegisterTiming(). After this, the simulation
	/// kind is set to \a SimDetailed, as returned by a call to
	/// getSimKind().
	void setTiming(Timing *timing)
	{
		assert(!this->timing);
		this->timing = timing;
		sim_kind = SimDetailed;
	}

	/// Return the type of simulation for the architecture
	SimKind getSimKind() const { return sim_kind; }

	/// Return whether the architecture performed an active simulation in
	/// the last simulation iteration.
	bool isActive() const { return active; }

	/// Set the flag that indicates that the last simulation iteration was
	/// active. This is done only internally in the architecture pool (call
	/// ArchPool::Run()).
	void setActive(bool active) { this->active = active; }
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

	// Map of architectures, indexed by name
	std::map<std::string, Arch *> arch_map;

	// List of architectures with timing simulation
	std::list<Arch *> timing_arch_list;

	// Register a new architecture with the given name, and return the new
	// architecture object created. If an architecture with that name
	// already existed, the existing object is returned.
	Arch *Register(const std::string &name);

public:

	/// Return a unique instance of the singleton
	static ArchPool *getInstance();

	/// Register a disassembler for the architecture with the given name.
	/// For a given architecture, this function should be invoked at most
	/// once.
	void RegisterDisassembler(const std::string &name, Asm *as)
	{
		Arch *arch = Register(name);
		arch->setAsm(as);
	}

	/// Register an emulator for the architecture with the given name. For
	/// a given architecture, this function should be invoked at most once.
	void RegisterEmulator(const std::string &name, Emu *emu)
	{
		Arch *arch = Register(name);
		arch->setEmu(emu);
	}

	/// Register a timing simulator for the architecture with the given
	/// name. For a given architecture, this function should be invoked at
	/// most once.
	void RegisterTiming(const std::string &name, Timing *timing)
	{
		Arch *arch = Register(name);
		arch->setTiming(timing);
		timing_arch_list.push_back(arch);
	}

	/// Return an architecture given its name, or null if no architecture
	/// is found with that name. The string comparison is done without
	/// case sensitivity.
	Arch *getByName(const std::string &name);

	/// Get a list of all possible names for architectures. This function is
	/// useful to print an error message with the valid values for
	/// architectures when processing user input.
	std::string getArchNames();

	/// Run one iteration of the main loop for each architecture, using
	/// emulation or timing simulation, depending on the user configuration.
	///
	/// \param num_emu_active
	///	The function returns in this argument the number of
	///	architectures that performed an active emulation.
	///
	/// \param num_timing_active
	///	The function returns in this argument passed by reference the
	///	number of architectures that performed an active timing
	///	simulation.
	///
	/// \return
	///	The two arguments in this function are return values used to
	///	decide whether the main simulation loop should stop.
	void Run(int &num_emu_active, int &num_timing_active);

	/// Return an iterator to the first architecture in the architecture
	/// list.
	std::list<std::unique_ptr<Arch>>::iterator begin()
	{
		return arch_list.begin();
	}

	/// Return a past-the-end iterator to the architecture list.
	std::list<std::unique_ptr<Arch>>::iterator end()
	{
		return arch_list.end();
	}

	/// Return an iterator to the first architecture with timing simulation.
	std::list<Arch *>::iterator timing_begin()
	{
		return timing_arch_list.begin();
	}

	/// Return a past-the-end iterator to the list of architectures with
	/// timing simulation.
	std::list<Arch *>::iterator timing_end()
	{
		return timing_arch_list.end();
	}

	/// Return the number of architectures with timing simulation
	int getNumTiming() const { return timing_arch_list.size(); }
};


}  // namespace comm

#endif

