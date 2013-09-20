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

#include <iostream>

#include <arch/mips/asm/Asm.h>
#include <lib/cpp/Misc.h>

using namespace Misc;


extern "C" void Main(int argc, char **argv)
{
	MIPSAsm *as = new MIPSAsm;

	if (argc != 2)
		fatal("wrong syntax");

	MIPSAsmCreate(as);
	MIPSAsmDisassembleBinary(as, argv[1]);
	MIPSAsmDestroy(as);
	delete as;
}

