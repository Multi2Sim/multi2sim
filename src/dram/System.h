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
#include <lib/cpp/Error.h>
#include <lib/cpp/IniFile.h>
#include <lib/esim/FrequencyDomain.h>
#include <lib/esim/Event.h>


namespace dram
{

// Forward declarations
class Controller;
class Request;


/// Class representing a runtime error in dram system
class Error : public misc::Error
{
public:

	/// Constructor
	Error(const std::string &message) : misc::Error(message)
	{
		// Set module prefix
		AppendPrefix("DRAM");
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

	// Sizes of address components
	int physical_size = 0;
	int logical_size = 0;
	int rank_size = 0;
	int bank_size = 0;
	int row_size = 0;
	int column_size = 0;

	/// Frequency
	static int frequency;

	// Stand-alone simulator instantiator
	static bool stand_alone;

	// Message to display with '--net-help'
	static const std::string help_message;

	// Counter of commands created in the system.  This serves to let every
	// command have a unique id for logging purposes.
	int next_command_id = -1;

	/// Finds the integer base 2 log of a number.
	int Log2(unsigned num);

	/// Sets the sizes of each address component, in the number of bits
	/// required to represent it.
	void GenerateAddressSizes();

public:

	// Error messages
	static const char *err_config_note;

	// Show help for the dram configuration
	static bool help;

	/// Destroy the singleton if allocated.
	static void Destroy();

	/// Debugger
	static misc::Debug debug;

	/// Activity log
	static misc::Debug activity;

	// EventTypes and FrequencyDomains for DRAM
	static esim::FrequencyDomain *frequency_domain;
	static esim::Event *event_request;
	static esim::Event *event_command_return;

	/// Obtain the instance of the dram simulator singleton.
	static System *getInstance();

	/// Returns a channel that belongs to this controller with the
	/// specified id.
	Controller *getController(int id) { return controllers[id].get(); }

	/// Returns whether or not DRAM is running as a stand alone simulator.
	static bool isStandAlone() { return stand_alone; }

	/// Returns the size in bits of the physical channel address component.
	int getPhysicalSize() const { return physical_size; }

	/// Returns the size in bits of the logical channel address component.
	int getLogicalSize() const { return logical_size; }

	/// Returns the size in bits of the rank address component.
	int getRankSize() const { return rank_size; }

	/// Returns the size in bits of the bank address component.
	int getBankSize() const { return bank_size; }

	/// Returns the size in bits of the row address component.
	int getRowSize() const { return row_size; }

	/// Returns the size in bits of the column address component.
	int getColumnSize() const { return column_size; }

	/// Return the maximum address
	int getCapacity();

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

	/// Parse a configuration file
	void ParseConfiguration(misc::IniFile *ini_file);

	// Initialize the dram system by parsing the dram configuration
	// file passed with '--dram-config'
	void ReadConfiguration();

	/// Run the stand-alone DRAM simulation loop.
	void Run();

	/// Returns the next available unique command id.
	int getNextCommandId();

	/// Add a request to the system. Should be used in the standalone
	/// simulator only; during full simulation, requests should be
	/// sent to the controllers through a network.
	void AddRequest(std::shared_ptr<Request> request);

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

	/// Activate activity debug information for the dram simulator.
	///
	/// \param path
	///	Path to dump debug information. Strings \c stdout and \c stderr
	///	are special values referring to the standard output and standard
	///	error output, respectively.
	static void setActivityDebugPath(const std::string &path)
	{
		activity.setPath(path);
		activity.setPrefix("[dram-activity]");
	}

	/// Send a read request to the dram device
	void Read(long long address);

	/// Send a write request to the dram device
	void Write(long long address);

	/// Dump the object to an output stream.
	void Dump(std::ostream &os = std::cout) const;

	/// Dump object with the << operator
	friend std::ostream &operator<<(std::ostream &os,
			const System &object)
	{
		object.Dump(os);
		return os;
	}
};

}  // namespace dram

#endif
