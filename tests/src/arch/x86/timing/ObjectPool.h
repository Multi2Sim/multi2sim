/*
 *  Multi2Sim
 *  Copyright (C) 2015  Shi Dong (dong.sh@husky.neu.edu)
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

#ifndef ARCH_X86_TIMING_OBJECT_POOL_H
#define ARCH_X86_TIMING_OBJECT_POOL_H

#include <lib/cpp/IniFile.h>
#include <arch/x86/emulator/Emulator.h>
#include <arch/x86/emulator/Uinst.h>
#include <arch/x86/timing/Core.h>
#include <arch/x86/timing/Cpu.h>
#include <arch/x86/timing/Thread.h>
#include <arch/x86/timing/Timing.h>


namespace x86
{

class ObjectPool
{
	// Timing simulator
	Timing *timing;

	// A context
	Context *context;

	// Unique instance of singleton
	static std::unique_ptr<ObjectPool> instance;

public:

	/// Contructor
	ObjectPool();

	static ObjectPool *getInstance()
	{
		// Instance already exists
		if (instance.get())
			return instance.get();

		// Create instance
		instance = misc::new_unique<ObjectPool>();
		return instance.get();
	}

	/// Destroy all singletons related with x86 simulation, including:
	///
	/// - ObjectPool singleton
	/// - Timing singleton
	/// - Emulator singleton
	/// - ArchPool singleton
	///
	static void Destroy();

	/// Return the CPU object.
	Cpu *getCpu() const { return timing->getCpu(); }

	/// Return the core.
	Core *getCore() const { return getCpu()->getCore(0); }

	/// Return the thread.
	Thread *getThread() const { return getCore()->getThread(0); }

	/// Return the context.
	Context *getContext() const { return context; }
};

}

#endif // ARCH_X86_TIMING_OBJECT_POOL_H
