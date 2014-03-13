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

#include <src/driver/opengl/OpenGLDriver.h>
#include <src/lib/cpp/Misc.h>

#include "Program.h"

using namespace misc;

namespace SI
{

ProgramGL::ProgramGL(unsigned id, const char *buffer, unsigned size)
{
	this->id = id;
	opengl_driver = Driver::OpenGLSIDriver::getInstance();
	program_bin.reset(new OpenGLSiProgramBinary(buffer, size));
}

Shader *ProgramGL::getShaderByID(unsigned id)
{
	for (auto &shader : shaders )
	{
		if ( shader->getID() == id)
			return shader.get();
	}

	return nullptr;
}

Shader *ProgramGL::getShaderByStage(OpenGLSiShaderStage stage)
{
	switch(stage)
	{

	case OpenGLSiShaderVertex:
	{
		for( auto &shader : shaders)
		{
			if (shader->getStage() == OpenGLSiShaderVertex)
				return shader.get();
		}
		break;
	}
	
	case OpenGLSiShaderPixel:
	{
		for( auto &shader : shaders)
		{
			if (shader->getStage() == OpenGLSiShaderPixel)
				return shader.get();
		}
		break;
	}

	default:
		return nullptr;

	}

	return nullptr;
}

void ProgramGL::BindShader(unsigned id, OpenGLSiShaderStage stage)
{
	assert(program_bin);

	switch(stage)
	{

	case OpenGLSiShaderVertex:
	{
		shaders.push_back(std::unique_ptr<Shader>
			(new Shader(id, stage, program_bin->getVertexShader())));
		break;
	}

	case OpenGLSiShaderPixel:
	{
		shaders.push_back(std::unique_ptr<Shader>
			(new Shader(id, stage, program_bin->getPixelShader())));
		break;	
	}

	default:
	{
		fatal("Binding error: Shader type(%d) not supported!", stage);
		break;

	}

	}
}

}  // namespace SI