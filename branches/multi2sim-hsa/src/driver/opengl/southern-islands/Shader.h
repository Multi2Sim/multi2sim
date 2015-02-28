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

#ifndef DRIVER_OPENGL_SI_SHADER_H
#define DRIVER_OPENGL_SI_SHADER_H

#include <arch/southern-islands/asm/ShaderBinary.h>
#include "Input.h"

namespace SI
{

class Shader
{
	unsigned id;
	OpenGLSiShaderStage stage;

	// Points to Shader binary inside Program binary
	OpenGLSiShaderBinaryCommon *shader_bin;
	
	// OpenGL input interface doesn't always match GLSL input interface
	// If they don't, we rely on Fetch Shader to load correct data to GPRs
	std::vector<std::unique_ptr<Input>> inputs;

public:
	Shader(unsigned id, OpenGLSiShaderStage stage, OpenGLSiShaderBinaryCommon *shader_bin);

	/// Getters
	///
	unsigned getID() const { return id; }

	///
	OpenGLSiShaderStage getStage() const { return stage; }
	
	/// Insert an input object to inputs list
	void AddInput(unsigned index, unsigned device_buffer_size, unsigned device_buffer_ptr, 
		SIInputDataType data_type, unsigned num_elems, bool isNormalized, 
		unsigned stride, unsigned device_buffer_offset);

};

} // namespace SI

#endif
