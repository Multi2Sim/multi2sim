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


#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> 

#include <driver/common/Driver.h>
#include <lib/cpp/String.h>

#include "Runtime.h"

namespace comm
{

std::unique_ptr<RuntimePool> RuntimePool::instance;

Runtime::Runtime(const std::string &name, const std::string &lib_name, 
	const std::string &redirect_lib_name, const std::string &dev_path, Driver::Common *driver)
{
	this->name = name;
	this->lib_name = lib_name;
	this->redirect_lib_name = redirect_lib_name;
	this->dev_path = dev_path;
	this->driver = driver;

	// Replace /dev with /tmp
	host_dev_path = dev_path;
	misc::StringTrimLeft(host_dev_path, "/dev/");
	host_dev_path = "/tmp/" + host_dev_path;

	// Clean up if already exists
	remove(host_dev_path.c_str());

	// Create virtual device in /tmp/
	if ((dev_desc = open(host_dev_path.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH)) == -1)
		misc::fatal("%s: cannot open %s", __FUNCTION__, dev_path.c_str());
}

Runtime::~Runtime()
{
	// Close virtual device
	if (dev_desc)
		close(dev_desc);

	// Clean up
	if (host_dev_path.c_str());
		remove(host_dev_path.c_str());	
}

RuntimePool* RuntimePool::getInstance()
{
	// Return existing instance
	if (instance.get())
		return instance.get();
	
	// Create new runtime pool
	instance.reset(new RuntimePool());
	return instance.get();	
}

void RuntimePool::Register(const std::string &name, const std::string &lib_name, 
	const std::string &redirect_lib_name, const std::string &dev_path, Driver::Common *driver)
{
	// Create new runtime in place
	runtime_list.emplace_back(new Runtime(name, lib_name, redirect_lib_name, dev_path, driver));	
}

}  // namespace comm