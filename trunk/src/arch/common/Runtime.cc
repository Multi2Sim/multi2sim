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

#include <driver/common/Driver.h>

#include "Runtime.h"

namespace comm
{

std::unique_ptr<RuntimePool> RuntimePool::instance;

Runtime::Runtime(const std::string &name, const std::string &lib_name, 
	const std::string &redirect_lib_name, unsigned ioctl_code, Driver::Common *driver)
{
	this->name = name;
	this->lib_name = lib_name;
	this->redirect_lib_name = redirect_lib_name;
	this->ioctl_code = ioctl_code;
	this->driver = driver;
}

RuntimePool* RuntimePool::getInstance()
{
	// Return existing instance
	if (instance.get())
		return instance.get();
	
	// Create new architecture pool
	instance.reset(new RuntimePool());
	return instance.get();	
}

void RuntimePool::Register(const std::string &name, const std::string &lib_name, 
	const std::string &redirect_lib_name, unsigned ioctl_code, Driver::Common *driver)
{
	// Create new architecture in place
	runtime_list.emplace_back(new Runtime(name, lib_name, redirect_lib_name, ioctl_code, driver));	
}

}  // namespace comm