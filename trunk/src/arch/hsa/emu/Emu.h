#ifndef ARCH_HSA_EMU_EMU_H
#define ARCH_HSA_EMU_EMU_H

#include <arch/common/Arch.h>
#include <arch/common/Emu.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Debug.h>

namespace HSA
{

/// HSA Emulator configuration
class EmuConfig : public misc::CommandLineConfig
{
	// Debugger files
	std::string hsa_debug_file;

	// Simulation kind
	comm::ArchSimKind sim_kind;

public:

	/// Initilization of default command-line options
	EmuConfig();	

	/// Register command-line options related with the hsa emulator
	void Register(misc::CommandLine &command_line);

	/// Process command-line options related with the hsa emulator
	void Process();

};

/// HSA Emulator
class Emu : public comm::Emu
{
	// Unique instance of hsa emulator
	static std::unique_ptr<Emu> instance;

	// Private constructor. The only possible instance of the hsa emulator 
	// can be obtained with a call to getInstance()
	Emu();

	// Process ID to be assigned next. Process IDs are assigned in 
	// increasing order, using function Emu::getPid()
	int pid;

public:

	/// The hsa emulator is a singleton class. The only possible instance 
	/// of it will be allocated the first time this funcion is invoked
	static Emu *getInstance();

	/// Configuration for hsa emulator
	static EmuConfig config;

	/// Return a unique process ID. Contexts can call this function when 
	/// created to obtain their unique identifier
	int getPid(){ return pid++; }

	/// Run one iteration of the emulation loop
	/// \return This function \c true if the iteration had a useful emulation 
	/// and \c false if all contexts finished execution
	bool Run();

	/// Debugger for hsa
	static misc::Debug hsa_debug;
};

}

#endif
