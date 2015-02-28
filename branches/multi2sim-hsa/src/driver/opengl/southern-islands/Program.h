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

#ifndef DRIVER_OPENGL_SI_PROGRAM_H
#define DRIVER_OPENGL_SI_PROGRAM_H

#include <memory>
#include <src/arch/southern-islands/asm/ShaderBinary.h>
#include "Shader.h"

// Forward declarations
namespace Driver
{
	class OpenGLSIDriver;
}

namespace SI
{

class ProgramGL
{
	unsigned id;
	::Driver::OpenGLSIDriver *opengl_driver;

	std::unique_ptr<OpenGLSiProgramBinary> program_bin;
	std::vector<std::unique_ptr<Shader>> shaders;

public:
	ProgramGL(unsigned id);

	/// Getters
	///
	/// Return pointer of a shader based on shader id
	Shader *getShaderByID(unsigned id);

	/// Return pointer of a shader based on shader type
	Shader *getShaderByStage(OpenGLSiShaderStage stage);

	/// Get program id
	unsigned getId() const { return id; }

	/// Get attribute location
	unsigned getAttribLoc(const std::string &string);

	/// Get uniform location
	unsigned getUniformLoc(const std::string &string);

	/// Bind a shader to program
	void BindShader(unsigned shader_id, OpenGLSiShaderStage stage);

	/// Set Program binary
	void SetBinary(const char*buffer, unsigned size);

};

} // namespace SI

#endif
