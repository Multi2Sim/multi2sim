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

#include <ext/stdio_filebuf.h>

#include "Asm.h"
#include "Wrapper.h"


using namespace MIPS;


////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Asm
////////////////////////////////////////////////////////////////////////////////


struct MIPSAsmWrap *MIPSAsmWrapCreate(void)
{
	return (struct MIPSAsmWrap *) new Asm;
}


void MIPSAsmWrapFree(struct MIPSAsmWrap *self)
{
	delete (Asm *) self;
}


void MIPSAsmWrapDisassembleBinary(struct MIPSAsmWrap *self, char *path)
{
	Asm *as;
	as = (Asm *) self;
	as->DisassembleBinary(path);
}




////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Inst
////////////////////////////////////////////////////////////////////////////////


struct MIPSInstWrap *MIPSInstWrapCreate(struct MIPSAsmWrap *as)
{
	return (MIPSInstWrap *) new Inst((Asm *) as);
}


void MIPSInstWrapFree(struct MIPSInstWrap *self)
{
	delete (Inst *) self;
}


void MIPSInstWrapCopy(struct MIPSInstWrap *left, struct MIPSInstWrap *right)
{
	Inst *ileft = (Inst *) left;
	Inst *iright = (Inst *) right;
	*ileft = *iright;
}


void MIPSInstWrapDecode(struct MIPSInstWrap *self, unsigned addr, void *buf)
{
	Inst *inst = (Inst *) self;
	inst->Decode(addr, (const char *) buf);
}


void MIPSInstWrapDump(struct MIPSInstWrap *self, FILE *f)
{
	Inst *inst = (Inst *) self;
	__gnu_cxx::stdio_filebuf<char> filebuf(fileno(f), std::ios::out);
	std::ostream os(&filebuf);
	inst->Dump(os);
}


MIPSInstOpcode MIPSInstWrapGetOpcode(struct MIPSInstWrap *self)
{
	Inst *inst = (Inst *) self;
	return (MIPSInstOpcode) inst->GetOpcode();
}


const char *MIPSInstWrapGetName(struct MIPSInstWrap *self)
{
	Inst *inst = (Inst *) self;
	return inst->GetName().c_str();
}


unsigned MIPSInstWrapGetAddress(struct MIPSInstWrap *self)
{
	Inst *inst = (Inst *) self;
	return inst->GetAddress();
}


MIPSInstBytes *MIPSInstWrapGetBytes(struct MIPSInstWrap *self)
{
	Inst *inst = (Inst *) self;
	return (MIPSInstBytes *) inst->GetBytes();
}
