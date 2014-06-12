#include "Emu.h"

namespace HSA
{

//
// Class 'EmuConfig'
//
EmuConfig::EmuConfig()
{
	// Initialize
	sim_kind = comm::ArchSimFunctional;
}

void EmuConfig::Register(misc::CommandLine &command_line)
{
	// Option --hsa-debug <file>
	command_line.RegisterString("--hsa-debug <file>", hsa_debug_file, 
		"Dump debug information about hsa");
	
	// Option --hsa-sim <kind>
	command_line.RegisterEnum("--hsa-sim {functional|detailed} "
			"(default = functional)",
			(int &) sim_kind, comm::arch_sim_kind_map,
			"Level of accuracy of hsa simulation");
}

void EmuConfig::Process()
{
	Emu::hsa_debug.setPath(hsa_debug_file);
	// std::cout << "In" << __FUNCTION__ << "\n";	
}

//
// Class 'Emu'
//

misc::Debug Emu::hsa_debug;

// Emulator singleton
std::unique_ptr<Emu> Emu::instance;

// Configuration
EmuConfig Emu::config;

Emu::Emu() : comm::Emu("hsa")
{
	// Initialize
}

Emu *Emu::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	instance.reset(new Emu());
	return instance.get();
}

bool Emu::Run()
{
	return false;
}

}
