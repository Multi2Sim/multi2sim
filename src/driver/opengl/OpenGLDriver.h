/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef DRIVER_OPENGL_DRIVER_H
#define DRIVER_OPENGL_DRIVER_H

#include <src/driver/common/SI.h>
#include "southern-islands/Program.h"
#include "southern-islands/Shader.h"

// Forward declaration
namespace SI
{
	class ProgramGL;
	class Shader;
	class ShaderExport;
	class SPI;
}  // namespace SI

namespace Driver
{

class OpenGLSIDriver : public virtual SICommon
{
	// Unique instance of OpenGL Driver
	static std::unique_ptr<OpenGLSIDriver> instance;

	// Private constructor. The only possible instance of the OpenGL Driver
	// can be obtained with a call to getInstance()
	OpenGLSIDriver();

	// List of Southern Islands OpenGL programs(container of shaders)
	std::vector<std::unique_ptr<SI::ProgramGL>> programs;

	// Shader Export module
	SI::ShaderExport *sx;

	// SPI module generates NDRanges for Pixel Shader
	SI::SPI *spi;

public:

	~OpenGLSIDriver();

	// Driver verision information
	static const unsigned major = 1;
	static const unsigned minor = 1000;

	/// Get the only instance of the OpenGL Driver. If the instance does not
	/// exist yet, it will be created, and will remain allocated until the
	/// end of the execution.
	static OpenGLSIDriver *getInstance();

	/// OpenCL driver call
	int DriverCall(x86::Context *ctx, int abi_code);

	/// Get SPI module
	SI::SPI *getSPIModule() const { return spi; }

	/// Get Shader Export Module
	SI::ShaderExport *getShaderExportModule() const { return sx; }

	/// Get program by id
	SI::ProgramGL *getProgramById(unsigned id) {
		for( auto &program : programs)
		{
			if (program->getId() == id)
				return program.get();
		}

		// Return
		return nullptr;
	}

	/// Remove program by id
	void RemoveProgramById(unsigned id) {
		for( auto &program : programs)
		{
			if (program->getId() == id)
				program.reset();
		}		
	}

	/// This function is called when all work groups from an ND-Range have
	/// been scheduled (i.e., ndrange->waiting_work_groups is empty)
	/// \param ndrange  
	void RequestWork(SI::NDRange *ndrange);

	/// This function is called when all work groups from an ND-Range have
	/// been scheduled and completed (i.e., ndrange->waiting_work_groups and 
	/// ndrange->running_work_groups are both empty)
	/// \param ndrange 
	void NDRangeComplete(SI::NDRange *ndrange);

	/// OpenGL driver call
	int DriverCall(x86::Context *context, unsigned abi_code);

	/// Return an iterator to the first NDRange in the NDRange list. The
	/// NDRanges can be conveniently traversed with a loop using these
	/// iterators. This is an example of how to dump all NDRanges in the
	/// NDRange list:
	/// \code
	///	for (auto i = opencl_driver->NDRangesBegin(),
	///			e = wavefront->NDRangesEnd(); i != e; ++i)
	///		i->Dump(std::cout);
	/// \endcode
	std::vector<std::unique_ptr<SI::NDRange>>::iterator NDRangesBegin() {
		return ndranges.begin();
	}

	/// Return a past-the-end iterator for the list of NDRanges in the
	/// NDRange list.
	std::vector<std::unique_ptr<SI::NDRange>>::iterator NDRangesEnd() {
		return ndranges.end();
	}

	/// Add program to program list
	void AddProgram(std::unique_ptr<SI::ProgramGL> program);
};

} // namespace Driver

#endif
