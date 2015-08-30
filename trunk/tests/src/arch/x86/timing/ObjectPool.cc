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

#include "ObjectPool.h"


namespace x86
{

// Singleton instance of object pool
std::unique_ptr<ObjectPool> ObjectPool::instance;

misc::IniFile ObjectPool::ini_file;


ObjectPool::ObjectPool()
{
	// Parse ini_file parameters.  If ini_file is empty, the
	// default values will be set.
	Timing::ParseConfiguration(&ini_file);

	// Timing simulator
	Timing *timing = Timing::getInstance();
	cpu = misc::new_unique<Cpu>(timing);
	core = misc::new_unique<Core>(cpu.get(), 0);
	thread = misc::new_unique<Thread>(core.get(), 0);

	// Create a context
	Emulator *emulator = Emulator::getInstance();
	context = emulator->newContext();
}

}
