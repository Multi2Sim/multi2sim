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


void ObjectPool::Destroy()
{
	// Reset ObjectPool singleton
	instance = nullptr;

	// Reset the rest of the singletons
	Timing::Destroy();
	Emulator::Destroy();
	comm::ArchPool::Destroy();
}


ObjectPool::ObjectPool()
{
	// Timing simulator, getting default values for all instances.
	timing = Timing::getInstance();

	// Create a context
	Emulator *emulator = Emulator::getInstance();
	context = emulator->newContext();
}

}
