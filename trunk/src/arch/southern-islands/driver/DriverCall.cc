/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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

#include <lib/cpp/String.h>
#include <mem-system/Memory.h>

#include "Driver.h"


namespace SI
{


// ABI Call 'Init'
//
// ...
int Driver::CallInit(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}

// ABI Call 'MEM Alloc'
//
// ...
int Driver::CallMemAlloc(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'MEM Read'
//
// ...
int Driver::CallMemRead(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'MEM Write'
//
// ...
int Driver::CallMemWrite(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'MEM Copy'
//
// ...
int Driver::CallMemCopy(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'MEM Free'
//
// ...
int Driver::CallMemFree(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'Program Create'
//
// ...
int Driver::CallProgramCreate(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'Program Set Binary'
//
// ...
int Driver::CallProgramSetBinary(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'Kernel Create'
//
// ...
int Driver::CallKernelCreate(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'Kernel Set Arg Value'
//
// ...
int Driver::CallKernelSetArgValue(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'KernelSet Arg Pointer'
//
// ...
int Driver::CallKernelSetArgPointer(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'Kernel Set Arg Sampler'
//
// ...
int Driver::CallKernelSetArgSampler(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'KernelSetArgImage'
//
// ...
int Driver::CallKernelSetArgImage(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}


// ABI Call 'NdrangeCreate'
//
// ...
int Driver::CallNdrangeCreate(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}


// ABI Call 'NdrangeGetNumBufferEntries'
//
// ...
int Driver::CallNdrangeGetNumBufferEntries(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'NdrangeSendWorkGroups'
//
// ...
int Driver::CallNdrangeSendWorkGroups(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'NdrangeFinish'
//
// ...
int Driver::CallNdrangeFinish(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'NdrangePassMemObjs'
//
// ...
int Driver::CallNdrangePassMemObjs(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'NdrangeSetFused'
//
// ...
int Driver::CallNdrangeSetFused(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'NdrangeFlush'
//
// ...
int Driver::CallNdrangeFlush(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'NdrangeFree'
//
// ...
int Driver::CallNdrangeFree(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'NdrangeStart'
//
// ...
int Driver::CallNdrangeStart(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'NdrangeEnd'
//
// ...
int Driver::CallNdrangeEnd(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

// ABI Call 'RuntimeDebug'
//
// ...
int Driver::CallRuntimeDebug(mem::Memory *memory, unsigned args_ptr)
{
	misc::fatal("%s:function call is not currently implemented in multi2sim", __FUNCTION__);
	return 0;
}

}  // namepsace SI

