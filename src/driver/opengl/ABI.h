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

#ifndef DRIVER_OPENGL_ABI_H
#define DRIVER_OPENGL_ABI_H

namespace Driver
{

/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef DRIVER_OPENCL_ABI_H
#define DRIVER_OPENCL_ABI_H

#include "southern-islands/ABI.h"

namespace Driver
{

// List of OpenGL Runtime calls
enum OpenGLABICall
{
	OpenGLABIInvalid = 0,

// Shared ABIs for both CL/GL driver
#define SI_ABI_CALL(space, name, code) OpenGLABI##space##name = code,
#include "../common/SI-ABI.dat"
#undef SI_ABI_CALL

// Unique ABIs
#define OPENGL_ABI_CALL(space, name, code) OpenGLABI##space##name = code,
#include "ABI.dat"
#undef OPENGL_ABI_CALL

	OpenGLABICallCount
};

// Forward declarations of OpenGL Runtime functions

// Shared ABIs for both CL/GL driver
#define SI_ABI_CALL(space, name, code) \
	int SIABI##name##Impl();
#include "../common/SI-ABI.dat"
#undef SI_ABI_CALL

// Unique ABIs for GL driver
#define OPENGL_ABI_CALL(space, name, code) \
	int OpenGLABI##name##Impl();
#include "ABI.dat"
#undef OPENGL_ABI_CALL

// List of OpenGL ABI call names
std::string OpenGLABICallName[OpenGLABICallCount + 1] =
{
	nullptr,
#define SI_ABI_CALL(space, name, code) #space #name,
#include "../common/SI-ABI.dat"
#undef SI_ABI_CALL

#define OPENGL_ABI_CALL(space, name, code) #space #name,
#include "ABI.dat"
#undef OPENGL_ABI_CALL
	nullptr
};

/// List of OpenGL Runtime functions
typedef int (*OpenGLABICallFuncPtr)();
OpenGLABICallFuncPtr OpenGLABICallTable[OpenGLABICallCount + 1] =
{
	nullptr,

// Shared ABIs for both CL/GL driver
#define SI_ABI_CALL(space, name, code) &space::SIABI##name##Impl,
#include "../common/SI-ABI.dat"
#undef SI_ABI_CALL

// Unique ABIs for GL driver
#define OPENGL_ABI_CALL(space, name, code) &space::OpenGLABI##name##Impl,
#include "ABI.dat"
#undef OPENGL_ABI_CALL
	nullptr
};

} // namespace Driver

#endif
