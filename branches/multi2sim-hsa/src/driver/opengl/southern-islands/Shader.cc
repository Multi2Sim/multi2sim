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

#include "Input.h"
#include "Shader.h"

namespace SI
{

Shader::Shader(unsigned id, OpenGLSiShaderStage stage, OpenGLSiShaderBinaryCommon *shader_bin)
{
	this->id = id;
	this->stage = stage;
	this->shader_bin = shader_bin;
}

void Shader::AddInput(unsigned index, unsigned device_buffer_size, unsigned device_buffer_ptr, 
	SIInputDataType data_type, unsigned num_elems, bool isNormalized, 
	unsigned stride, unsigned device_buffer_offset)
{
	inputs.push_back(std::unique_ptr<Input> (new Input(index, device_buffer_size, 
		device_buffer_ptr, data_type, num_elems, isNormalized, stride, device_buffer_offset)));
}

}  // namespace SI