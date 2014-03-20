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
#ifndef DRIVER_OPENGL_SI_FETCH_SHADER
#define DRIVER_OPENGL_SI_FETCH_SHADER

#include <memory>

namespace SI
{

class Shader;

// Fetch shader generates instructions to initialize GPRs. Normally it's a subroutine
// at the beginning of Vertex Shader. The .input section in Vertex Shader ELF binary
// has the interface information of the GLSL shader. The input list in Vertex Shader 
// object are created in ABI calls, which contains vertex attribute data and information 
// such as size/format of the data. 
// ABI created inputs sometimes don't match GLSL shader input interface. We need
// fetch shader to expand data to 4 components( it seems AMD always use 4 elements
// data in vertex shader )
class FetchShader
{
	std::unique_ptr<char[ ]> inst_buffer;
	unsigned inst_buffer_size;

public:
	FetchShader(const Shader *shader);
};

}  // namespace SI

#endif
