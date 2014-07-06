/*
 *  Multi2Sim
 *  Copyright (C) 2013  Chris Barton (barton.ch@husky.neu.edu)
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

#include <lib/mhandle/mhandle.h>

#include "cl2llvm.h"
#include "Wrapper.h"


struct Cl2llvmContext *Cl2llvmContextCreate()
{
	CLASS_REGISTER(Cl2llvm);
	Cl2llvm *cl2llvm = new(Cl2llvm);
	return (struct Cl2llvmContext *) cl2llvm;
}


void Cl2llvmContextDestroy(struct Cl2llvmContext *context)
{
	Cl2llvm *cl2llvm = (Cl2llvm *) context;
	delete(cl2llvm);
}


void Cl2llvmContextParse(struct Cl2llvmContext *context,
		const char *in,
		const char *out,
		int opt_level)
{
	Cl2llvm *cl2llvm = (Cl2llvm *) context;
	Cl2llvmParse(cl2llvm, in, out, opt_level);
}

