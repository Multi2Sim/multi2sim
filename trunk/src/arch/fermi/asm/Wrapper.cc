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
// Wrapper for class Inst
////////////////////////////////////////////////////////////////////////////////

struct FrmInstWrap *FrmInstWrapCreate()
{
	return (FrmInstWrap *) new Fermi::Inst();
}


void FrmInstWrapFree(struct FrmInstWrap *self)
{
	delete (Fermi::Inst *) self;
}


void FrmInstWrapCopy(struct FrmInstWrap *left, struct FrmInstWrap *right)
{
	Fermi::Inst *ileft = (Fermi::Inst *) left;
	Fermi::Inst *iright = (Fermi::Inst *) right;
	*ileft = *iright;
}


void FrmInstWrapDecode(struct FrmInstWrap *self, unsigned addr, void *ptr)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	inst->Decode(addr, (const char *) ptr);
}


FrmInstBytes *FrmInstWrapGetBytes(struct FrmInstWrap *self)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	return (FrmInstBytes *) inst->getBytes();
}


FrmInstId FrmInstWrapGetId(struct FrmInstWrap *self)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	return (FrmInstId) inst->getId();
}


FrmInstOp FrmInstWrapGetOpcode(struct FrmInstWrap *self)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	return (FrmInstOp) inst->getOp();
}


FrmInstCategory FrmInstWrapGetCategory(struct FrmInstWrap *self)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	return (FrmInstCategory) inst->getCategory();
}


char *FrmInstWrapGetName(struct FrmInstWrap *self)
{
	Fermi::Inst *inst = (Fermi::Inst *) self;
	return (char *) inst->getName().c_str();
}


////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Asm
////////////////////////////////////////////////////////////////////////////////

struct FrmAsmWrap *FrmAsmWrapCreate(void)
{
	return (struct FrmAsmWrap *) Asm::getInstance();
}


void FrmAsmWrapFree(struct FrmAsmWrap *self)
{
}


void FrmAsmWrapDisassembleBinary(struct FrmAsmWrap *self, char *path)
{
	Asm *as = (Asm *) self;
	as->DisassembleBinary(path);
}



FrmInstInfo *FrmAsmWrapGetInstInfo(struct FrmAsmWrap *self, unsigned cat,
		unsigned op)
{
	Asm *as = (Asm *) self;
	return (FrmInstInfo *) as->GetInstInfo(cat, op);
}
