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

#include "Asm.h"
#include "Wrapper.h"


////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Asm
////////////////////////////////////////////////////////////////////////////////

struct KplAsm *KplAsmCreate()
{
	Kepler::Asm *as = new Kepler::Asm();
	return (KplAsm *) as;
}

void KplAsmFree(struct KplAsm *self)
{
	Kepler::Asm *as = (Kepler::Asm *) self;
	delete as;
}

void KplAsmDisassembleBinary(struct KplAsm *self, const char *path)
{
	Kepler::Asm *as = (Kepler::Asm *) self;
	as->DisassembleBinary(path);
}

