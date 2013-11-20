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
#include "Inst.h"
#include "Wrapper.h"

using namespace Fermi;


////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Asm
////////////////////////////////////////////////////////////////////////////////

struct FrmAsmWrap *FrmAsmWrapCreate(void)
{
	return (struct FrmAsmWrap *) new Asm();
}


void FrmAsmWrapFree(struct FrmAsmWrap *self)
{
	delete (Asm *) self;
}


void FrmAsmWrapDisassembleBinary(struct FrmAsmWrap *self, char *path)
{
	Asm *as = (Asm *) self;
	as->DisassembleBinary(path);
}


void FrmAsmWrapDisassembleBuffer(struct FrmAsmWrap *self, char *buffer, unsigned int size)
{
	Asm *as = (Asm *) self;
	as->DisassembleBuffer(buffer, size);
}


FrmInstInfo *FrmAsmWrapGetInstInfo(struct FrmAsmWrap *self, FrmInstOpcode opcode)
{
	Asm *as = (Asm *) self;
	return (FrmInstInfo *) as->GetInstInfo((InstOpcode) opcode);
}


FrmInstInfo *FrmAsmWrapGetDecTable(struct FrmAsmWrap *self, int cat, int func)
{
	Asm *as = (Asm *) self;
	return (FrmInstInfo *) as->GetDecTable(cat, func);
}



////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Inst
////////////////////////////////////////////////////////////////////////////////


struct FrmInstWrap *FrmInstWrapCreate(struct FrmAsmWrap *as)
{
	return (FrmInstWrap *) new Fermi::Inst((Fermi::Asm *) as);
}


void FrmInstWrapFree(struct FrmInstWrap *self)
{
	delete (Fermi::Inst *) self;
}


void FrmInstWrapCopy(struct FrmInstWrap *left, struct frmInstWrap *right)
{
	Fermi::Inst *ileft = (Fermi::Inst *) left;
	Fermi::Inst *iright = (Fermi::Inst *) right;
	*ileft = *iright;
}


void FrmInstWrapDecode(struct FrmInstWrap *self, unsigned int addr, void *ptr)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	inst->Decode(addr, (const char *) ptr);
}


FrmInstBytes *FrmInstWrapGetBytes(struct FrmInstWrap *self)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	return (FrmInstBytes *) inst->GetBytes();
}

const char *FrmInstWrapGetName(struct FrmInstWrap *self)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	return inst->GetName().c_str();
}


FrmInstOpcode FrmInstWrapGetOpcode(struct FrmInstWrap *self)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	return (FrmInstOpcode) inst->GetOpcode();
}


FrmInstCategory FrmInstWrapGetCategory(struct FrmInstWrap *self)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	return (FrmInstCategory) inst->GetCategory();
}
