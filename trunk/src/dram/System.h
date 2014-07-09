/*
 * Multi2Sim
 * Copyright (C) 2014 Agamemnon Despopoulos (agdespopoulos@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef DRAM_DRAM_H
#define DRAM_DRAM_H

#include <map>
#include <memory>
#include <iostream>
#include <vector>

#include <lib/cpp/Debug.h>
#include <lib/esim/FrequencyDomain.h>
#include <lib/esim/Event.h>


namespace dram
{

// Forward declarations
class Controller;
class Request;


struct Address
{
	long long encoded;
	int physical;
	int logical;
	int rank;
	int bank;
	int row;
	int column;

	/// Dump the object to an output stream.
	void dump(std::ostream &os = std::cout) const;

	/// Dump object with the << operator
	friend std::ostream &operator<<(std::ostream &os,
			const Address &object)
	{
		object.dump(os);
		return os;
	}
};

class System
{
	// Unique instance of this class
	static std::unique_ptr<System> instance;

	// Private constructor, used internally to instantiate a singleton. Use
	// a call to getInstance() instead.
	System();

	// List of all the memory controllers
	std::vector<std::unique_ptr<Controller>> controllers;

public:

	/// Debugger
	static misc::Debug debug;

	// EventTypes and FrequencyDomains for DRAM
	static esim::FrequencyDomain *DRAM_DOMAIN;
	static esim::EventType *ACTION_REQUEST;
	static esim::EventType *COMMAND_RETURN;

	/// Obtain the instance of the dram simulator singleton.
	static System *getInstance();

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

	/// Parse a configuration file
	void ParseConfiguration(const std::string &path);

	/// Run the standalone dram simulation loop.
	void Run();

	/// Add a request to the system. Should be used in the standalone
	/// simulator only; during full simulation, requests should be
	/// sent to the controllers through a network.
	void AddRequest(std::shared_ptr<Request> request);

	/// Decode an address to its component locations. For now, this decodes
	/// in the order physical:logical:rank:bank:row:column. This will
	/// eventually be confiurable.
	///
	/// \param address
	/// The Address object passed in should be initalized with the encoded
	/// field.  The function will fill in the rest of the fields based on it.
	void DecodeAddress(Address &address);
	int Log2(unsigned num);

	/// Activate debug information for the dram simulator.
	///
	/// \param path
	///	Path to dump debug information. Strings \c stdout and \c stderr
	///	are special values referring to the standard output and standard
	///	error output, respectively.
	static void setDebugPath(const std::string &path)
	{
		debug.setPath(path);
		debug.setPrefix("[dram]");
	}

	/// Dump the object to an output stream.
	void dump(std::ostream &os = std::cout) const;

	/// Dump object with the << operator
	friend std::ostream &operator<<(std::ostream &os,
			const System &object)
	{
		object.dump(os);
		return os;
	}
};

}  // namespace dram

#endif
