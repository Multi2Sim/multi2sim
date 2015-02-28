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

//using namespace Kepler;


////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Inst
////////////////////////////////////////////////////////////////////////////////

struct KplInstWrap *KplInstWrapCreate()
{
	return (KplInstWrap *) new Kepler::Inst();
}

void KplInstWrapFree(struct KplInstWrap *self)
{
	delete (Kepler::Inst *) self;
}

void KplInstWrapDecode(struct KplInstWrap *self, unsigned addr, void *ptr)
{
	Kepler::Inst *inst = (Kepler::Inst *) self;
	inst->Decode((const char *) ptr, addr);
}

//KplInstId KplInstWrapGetId(struct KplInstWrap *self)
//{
//	Kepler::Inst *inst = (Kepler::Inst *) self;
//	return (KplInstId) inst->getId();
//}

KplInstOpcode KplInstWrapGetOpcode(struct KplInstWrap *self)
{
	Kepler::Inst *inst = (Kepler::Inst *) self;
	return (KplInstOpcode) inst->getOpcode();
}

////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Asm
////////////////////////////////////////////////////////////////////////////////

struct KplAsm *KplAsmCreate()
{
	Kepler::Asm *as = Kepler::Asm::getInstance();
	return (KplAsm *) as;
}

void KplAsmFree(struct KplAsm *self)
{
}

/*
void KplAsmDisassembleBinary(struct KplAsm *self, const char *path)
{
	Kepler::Asm *as = (Kepler::Asm *) self;
	as->DisassembleBinary(path);
}
*/

