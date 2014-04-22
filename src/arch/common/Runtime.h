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

#ifndef ARCH_COMMON_RUNTIME_H
#define ARCH_COMMON_RUNTIME_H

#include <list>
#include <memory>
#include <iostream>
#include <string>

// Forward declarations
namespace Driver
{
	class Common;
}  // namespace Driver

namespace x86
{
	class Context;
}  // namespace x86

namespace comm
{

class Runtime
{
	// Runtime name, just used for debug info. 
	std::string name;

	// Name of the library to intercept during the execution of the
	// 'open' system call. 
	std::string lib_name;

	// Name of the library to redirect it to, searching either in the
	// source tree or Multi2Sim installation path. 
	std::string redirect_lib_name;

	// Record here failed attempts to access this runtime. This is used
	// to issue a warning to the user at the end of the simulation. 
	unsigned open_attempt;

	// Identifier used for the driver ABI. When the runtime needs
	// to communicate with the driver, it is done through ioctl . 
	unsigned ioctl_code;

	// Associate driver instance
	Driver::Common *driver;

public:
	Runtime(const std::string &name, const std::string &lib_name, const std::string &redirect_lib_name,
		unsigned ioctl_code, Driver::Common *driver);

	/// Getters
	///
	/// Get ioctl_code
	unsigned getCode() const { return ioctl_code; }

	/// Get Driver
	Driver::Common *getDriver() const { return driver; }
};

class RuntimePool
{
	// Unique instance of the class
	static std::unique_ptr<RuntimePool> instance;

	// List of runtimes
	std::list<std::unique_ptr<Runtime>> runtime_list;

	// Private constructor for singleton
	RuntimePool() { }

public:

	/// Return a unique instance of the singleton
	static RuntimePool *getInstance();

	/// Getters
	///
	/// Get runtime by ioctl_code
	Runtime *getRuntimeByCode(unsigned code) {
		for( auto &runtime : runtime_list)
		{
			if (runtime->getCode() == code)
				return runtime.get();
		}
		return nullptr;
	}

	/// Check if ioctl_code is used by any registered runtime
	bool isRegistered(unsigned value) const {
		for( auto &runtime : runtime_list)
		{
			if (runtime->getCode() == value)
				return true;
		}
		return false;
	}

	/// Register a runtime
	void Register(const std::string &name, const std::string &lib_name, const std::string &redirect_lib_name,
		unsigned ioctl_code, Driver::Common *driver);

};

}  // namespace comm

#endif
