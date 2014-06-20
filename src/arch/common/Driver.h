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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_COMMON_DRIVER_H
#define ARCH_COMMON_DRIVER_H

#include <list>
#include <memory>
#include <iostream>
#include <string>


// Forward declarations
namespace mem { class Memory; }


namespace comm
{


class Driver
{
	// Identification name
	std::string name;

	// Virtual device path
	std::string path;

public:
	
	/// Constructor
	///
	/// \param name
	///	Driver identifier
	///
	/// \param path
	///	Path of the virtual device used to access the driver (e.g.
	///	 `/dev/southern-islands`)
	Driver(const std::string &name, const std::string &path);

	/// Return the path of the virtual device
	const std::string &getPath() const { return path; }

	/// Invoke an ABI call of the driver. This is a pure virtual function
	/// that every child of class Driver should implement.
	///
	/// \param code
	///	The ABI call code
	///
	/// \param memory
	///	The memory of the host application where the arguments of the
	///	ABI call can be found.
	///
	/// \param args_ptr
	///	The address in the host application memory where the arguments
	///	of the call can be found, or 0 if no arguments are passed.
	///
	/// \return
	///	The functions forwards the return value of the ABI call. This
	///	value is provided as a return value of the system call and made
	///	available to the application.
	virtual int Call(int code, mem::Memory *memory, unsigned args_ptr) = 0;
};


/// This class keeps track of all drivers in the system. It is a singleton.
class DriverPool
{
	// Singleton
	static std::unique_ptr<DriverPool> instance;

	// List of drivers
	std::list<Driver *> driver_list;

	// Private constructor for singleton
	DriverPool() { }

public:

	/// Return a unique instance of the singleton
	static DriverPool *getInstance();

	/// Register a driver
	void Register(Driver *driver);

	/// Return a driver object given the path of its associated virtual
	/// device, or \c nullptr if no driver is found with that path.
	Driver *getDriverByPath(const std::string &path);
};

}  // namespace comm

#endif

