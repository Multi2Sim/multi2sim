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

#include <arch/southern-islands/asm/ShaderBinary.h>
#include <driver/opengl/OpenGLDriver.h>
#include <lib/cpp/Misc.h>

#include "Program.h"

using namespace misc;

namespace SI
{

ProgramGL::ProgramGL(unsigned id)
{
	this->id = id;
	this->opengl_driver = Driver::OpenGLSIDriver::getInstance();
}

void ProgramGL::SetBinary(const char *buffer, unsigned size)
{
	if (buffer)
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

unsigned ProgramGL::getAttribLoc(const std::string &name)
{
	unsigned return_val;

	for( auto &symbol : program_bin->getVertexShader()->getSymbols())
	{
		std::string symbol_name = std::string(symbol->getName());
		if (symbol_name == name)
		{
			switch (symbol->getType())
			{
				case OpenGLSiSymbolAttribTexcoord0:
					return_val = 0;
					break;
				case OpenGLSiSymbolAttribTexcoord1:
					return_val = 1;
					break;
				case OpenGLSiSymbolAttribTexcoord2:
					return_val = 2;
					break;
				case OpenGLSiSymbolAttribTexcoord3:
					return_val = 3;
					break;
				case OpenGLSiSymbolAttribTexcoord4:
					return_val = 4;
					break;
				case OpenGLSiSymbolAttribTexcoord5:
					return_val = 5;
					break;
				case OpenGLSiSymbolAttribTexcoord6:
					return_val = 6;
					break;
				case OpenGLSiSymbolAttribTexcoord7:
					return_val = 7;
					break;
				case OpenGLSiSymbolAttribTexcoord8:
					return_val = 8;
					break;
				case OpenGLSiSymbolAttribTexcoord9:
					return_val = 9;
					break;
				case OpenGLSiSymbolAttribTexcoord10:
					return_val = 10;
					break;
				case OpenGLSiSymbolAttribTexcoord11:
					return_val = 11;
					break;
				case OpenGLSiSymbolAttribTexcoord12:
					return_val = 12;
					break;
				case OpenGLSiSymbolAttribTexcoord13:
					return_val = 13;
					break;
				case OpenGLSiSymbolAttribTexcoord14:
					return_val = 14;
					break;
				case OpenGLSiSymbolAttribTexcoord15:
					return_val = 15;
					break;
				case OpenGLSiSymbolAttribGeneric0:
					return_val = 0;
					break;
				case OpenGLSiSymbolAttribGeneric1:
					return_val = 1;
					break;
				case OpenGLSiSymbolAttribGeneric2:
					return_val = 2;
					break;
				case OpenGLSiSymbolAttribGeneric3:
					return_val = 3;
					break;
				case OpenGLSiSymbolAttribGeneric4:
					return_val = 4;
					break;
				case OpenGLSiSymbolAttribGeneric5:
					return_val = 5;
					break;
				case OpenGLSiSymbolAttribGeneric6:
					return_val = 6;
					break;
				case OpenGLSiSymbolAttribGeneric7:
					return_val = 7;
					break;
				case OpenGLSiSymbolAttribGeneric8:
					return_val = 8;
					break;
				case OpenGLSiSymbolAttribGeneric9:
					return_val = 9;
					break;
				case OpenGLSiSymbolAttribGeneric10:
					return_val = 10;
					break;
				case OpenGLSiSymbolAttribGeneric11:
					return_val = 11;
					break;
				case OpenGLSiSymbolAttribGeneric12:
					return_val = 12;
					break;
				case OpenGLSiSymbolAttribGeneric13:
					return_val = 13;
					break;
				case OpenGLSiSymbolAttribGeneric14:
					return_val = 14;
					break;
				case OpenGLSiSymbolAttribGeneric15:
					return_val = 15;
					break;
				case OpenGLSiSymbolAttribGeneric16:
					return_val = 16;
					break;
				case OpenGLSiSymbolAttribGeneric17:
					return_val = 17;
					break;
				case OpenGLSiSymbolAttribGeneric18:
					return_val = 18;
					break;
				case OpenGLSiSymbolAttribGeneric19:
					return_val = 19;
					break;
				case OpenGLSiSymbolAttribGeneric20:
					return_val = 20;
					break;
				case OpenGLSiSymbolAttribGeneric21:
					return_val = 21;
					break;
				case OpenGLSiSymbolAttribGeneric22:
					return_val = 22;
					break;
				case OpenGLSiSymbolAttribGeneric23:
					return_val = 23;
					break;
				case OpenGLSiSymbolAttribGeneric24:
					return_val = 24;
					break;
				case OpenGLSiSymbolAttribGeneric25:
					return_val = 25;
					break;
				case OpenGLSiSymbolAttribGeneric26:
					return_val = 26;
					break;
				case OpenGLSiSymbolAttribGeneric27:
					return_val = 27;
					break;
				case OpenGLSiSymbolAttribGeneric28:
					return_val = 28;
					break;
				case OpenGLSiSymbolAttribGeneric29:
					return_val = 29;
					break;
				case OpenGLSiSymbolAttribGeneric30:
					return_val = 20;
					break;
				case OpenGLSiSymbolAttribGeneric31:
					return_val = 31;
					break;
				case OpenGLSiSymbolAttribGeneric32:
					return_val = 32;
					break;
				case OpenGLSiSymbolAttribGeneric33:
					return_val = 33;
					break;
				case OpenGLSiSymbolAttribGeneric34:
					return_val = 34;
					break;
				case OpenGLSiSymbolAttribGeneric35:
					return_val = 35;
					break;
				case OpenGLSiSymbolAttribGeneric36:
					return_val = 36;
					break;
				case OpenGLSiSymbolAttribGeneric37:
					return_val = 37;
					break;
				case OpenGLSiSymbolAttribGeneric38:
					return_val = 38;
					break;
				case OpenGLSiSymbolAttribGeneric39:
					return_val = 39;
					break;
				default:
					return_val = -1;
					break;		
			}
		}
	}

	// Return
	return return_val;
}

unsigned ProgramGL::getUniformLoc(const std::string &string)
{
	fatal("Not implemented!");

	// Return
	return 0;
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