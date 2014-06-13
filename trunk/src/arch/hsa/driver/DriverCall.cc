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


namespace HSA
{


// ABI Call 'Init'
//
// ...
int Driver::CallInit(mem::Memory *memory, unsigned args_ptr)
{
	return 0;
}


// ABI Call 'PrintArgs'
//
// arg1
//	First argument to print
//
// arg2
//	Second
//
// arg3
//	Third
//
// Return value
//	This function returns 3.
//
int Driver::CallPrintArgs(mem::Memory *memory, unsigned args_ptr)
{
	// Read arguments
	int arg1;
	int arg2;
	int arg3;
	memory->Read(args_ptr, 4, (char *) &arg1);
	memory->Read(args_ptr + 4, 4, (char *) &arg2);
	memory->Read(args_ptr + 8, 4, (char *) &arg3);

	// Debug
	debug << misc::fmt("  arg1 = %d\n", arg1);
	debug << misc::fmt("  arg2 = %d\n", arg2);
	debug << misc::fmt("  arg3 = %d\n", arg3);

	// Print arguments
	std::cout << misc::fmt("Driver ABI call says: %d, %d, %d\n",
			arg1, arg2, arg3);
	
	// Done
	return 3;
}


}  // namepsace HSA

