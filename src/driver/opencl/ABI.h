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


#include <string>


namespace Driver
{

int OpenCLABIInitImpl();

// List of OpenCL Runtime calls
enum OpenCLABICall
{
	OpenCLABIInvalid = 0,
#define OPENCL_ABI_CALL(name, code) OpenCLABI##name = code,
#include "ABI.dat"
#undef OPENCL_ABI_CALL
	OpenCLABICallCount
};

// List of OpenCL ABI call names 
std::string OpenCLABICallName[OpenCLABICallCount + 1] =
{
	nullptr,
#define OPENCL_ABI_CALL(name, code) #name,
#include "ABI.dat"
#undef OPENCL_ABI_CALL
	nullptr
};

/// List of OpenCL Runtime functions, initially it's all nullptr except 
/// OpenCLABICallTable[OpenGLABIInit].
/// Note that there's no nullptr following '#undef OPENCL_ABI_CALL'
typedef int (*OpenCLABICallFuncPtr)();
OpenCLABICallFuncPtr OpenCLABICallTable[OpenCLABICallCount + 1] =
{
	nullptr,
	&OpenCLABIInitImpl,
#define OPENCL_ABI_CALL(name, code) nullptr,
#include "ABI.dat"
#undef OPENCL_ABI_CALL
};

}  // namespace Driver

#endif
