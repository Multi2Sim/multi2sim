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

#include <ext/stdio_filebuf.h>
#include <sstream>

#include "Asm.h"
#include "Binary.h"
#include "Inst.h"
#include "Wrapper.h"

using namespace SI;



////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Inst
////////////////////////////////////////////////////////////////////////////////


struct StringMapWrap *si_inst_special_reg_map = (StringMapWrap *) &inst_special_reg_map;
struct StringMapWrap *si_inst_buf_data_format_map = (StringMapWrap *) &inst_buf_data_format_map;
struct StringMapWrap *si_inst_buf_num_format_map = (StringMapWrap *) &inst_buf_num_format_map;
struct StringMapWrap *si_inst_format_map = (StringMapWrap *) &inst_format_map;

struct SIInstWrap *SIInstWrapCreate(SIAsmWrap *as)
{
	Inst *inst = new Inst((Asm *) as);
	return (SIInstWrap *) inst;
}


void SIInstWrapFree(struct SIInstWrap *self)
{
	Inst *inst = (Inst *) self;
	delete inst;
}


void SIInstWrapDecode(struct SIInstWrap *self, char *buffer, unsigned int offset)
{
	Inst *inst = (Inst *) self;
	inst->Decode(buffer, offset);
}


void SIInstWrapDump(struct SIInstWrap *self, FILE *f)
{
	Inst *inst = (Inst *) self;
	__gnu_cxx::stdio_filebuf<char> filebuf(fileno(f), std::ios::out);
	std::ostream os(&filebuf);
	inst->Dump(os);
}


void SIInstWrapDumpBuf(struct SIInstWrap *self, char *buffer, int size)
{
	std::stringstream ss;
	Inst *inst = (Inst *) self;
	inst->Dump(ss);
	snprintf(buffer, size, "%s", ss.str().c_str());
}


void SIInstWrapClear(struct SIInstWrap *self)
{
	Inst *inst = (Inst *) self;
	inst->Clear();
}


int SIInstWrapGetOp(struct SIInstWrap *self)
{
	Inst *inst = (Inst *) self;
	return inst->getOp();
}


SIInstOpcode SIInstWrapGetOpcode(struct SIInstWrap *self)
{
	Inst *inst = (Inst *) self;
	return (SIInstOpcode) inst->getOpcode();
}


SIInstBytes *SIInstWrapGetBytes(struct SIInstWrap *self)
{
	Inst *inst = (Inst *) self;
	return (SIInstBytes *) inst->getBytes();
}


const char *SIInstWrapGetName(struct SIInstWrap *self)
{
	Inst *inst = (Inst *) self;
	return inst->getName();
}


SIInstFormat SIInstWrapGetFormat(struct SIInstWrap *self)
{
	Inst *inst = (Inst *) self;
	return (SIInstFormat) inst->getFormat();
}


int SIInstWrapGetSize(struct SIInstWrap *self)
{
	Inst *inst = (Inst *) self;
	return inst->getSize();
}



////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Asm
////////////////////////////////////////////////////////////////////////////////


struct SIAsmWrap *SIAsmWrapCreate()
{
	Asm *as = new Asm();
	return (SIAsmWrap *) as;
}


void SIAsmWrapFree(struct SIAsmWrap *self)
{
	Asm *as = (Asm *) self;
	delete as;
}


void SIAsmWrapDisassembleBinary(struct SIAsmWrap *self, char *path)
{
	Asm *as = (Asm *) self;
	as->DisassembleBinary(path);
}


void SIAsmWrapDisassembleOpenGLBinary(struct SIAsmWrap *self, char *path,
		int shader_index)
{
	Asm *as = (Asm *) self;
	as->DisassembleOpenGLBinary(path, shader_index);
}


void *SIAsmWrapGetInstInfo(struct SIAsmWrap *self, int index)
{
	Asm *as = (Asm *) self;
	return (void *) as->getInstInfo(index);
}


////////////////////////////////////////////////////////////////////////////////
// Wrapper for class Binary
////////////////////////////////////////////////////////////////////////////////


StringMapWrap *si_binary_machine_map = (StringMapWrap *) &binary_machine_map;
StringMapWrap *si_binary_note_map = (StringMapWrap *) &binary_note_map;
StringMapWrap *si_binary_prog_info_map = (StringMapWrap *) &binary_prog_info_map;
StringMapWrap *si_binary_user_data_map = (StringMapWrap *) &binary_user_data_map;

int SIBinaryDictEntryGetNumVgpr(struct SIBinaryDictEntry *self)
{
	BinaryDictEntry *entry = (BinaryDictEntry *) self;
	return entry->num_vgpr;
}

int SIBinaryDictEntryGetNumSgpr(struct SIBinaryDictEntry *self)
{
	BinaryDictEntry *entry = (BinaryDictEntry *) self;
	return entry->num_sgpr;
}

int SIBinaryDictEntryGetLDSSize(struct SIBinaryDictEntry *self)
{
	BinaryDictEntry *entry = (BinaryDictEntry *) self;
	return entry->lds_size;
}

int SIBinaryDictEntryGetStackSize(struct SIBinaryDictEntry *self)
{
	BinaryDictEntry *entry = (BinaryDictEntry *) self;
	return entry->stack_size;
}

unsigned int SIBinaryDictEntryGetNumUserElements(struct SIBinaryDictEntry *self)
{
	BinaryDictEntry *entry = (BinaryDictEntry *) self;
	return entry->num_user_elements;
}

struct SIBinaryUserElement *SIBinaryDictEntryGetUserElements(struct SIBinaryDictEntry *self)
{
	BinaryDictEntry *entry = (BinaryDictEntry *) self;
	assert(sizeof(SIBinaryUserElement) == sizeof(BinaryUserElement));
	return (SIBinaryUserElement *) &entry->user_elements;
}

struct SIBinaryComputePgmRsrc2 *SIBinaryDictEntryGetComputePgmRsrc2(struct SIBinaryDictEntry *self)
{
	BinaryDictEntry *entry = (BinaryDictEntry *) self;
	assert(sizeof(SIBinaryComputePgmRsrc2) == sizeof(BinaryComputePgmRsrc2));
	return (SIBinaryComputePgmRsrc2 *) entry->compute_pgm_rsrc2;
}

const char *SIBinaryDictEntryGetTextBuffer(struct SIBinaryDictEntry *self)
{
	BinaryDictEntry *entry = (BinaryDictEntry *) self;
	return entry->text_section->getBuffer();
}

unsigned int SIBinaryDictEntryGetTextSize(struct SIBinaryDictEntry *self)
{
	BinaryDictEntry *entry = (BinaryDictEntry *) self;
	return entry->text_section->getSize();
}

struct SIBinary *SIBinaryCreate(const char *buffer, unsigned int size, char *name)
{
	Binary *self = new Binary(buffer, size, name);
	return (struct SIBinary *) self;
}

void SIBinaryFree(struct SIBinary *self)
{
	Binary *bin = (Binary *) self;
	delete bin;
}

void SIBinarySetDebugFile(const char *path)
{
	Binary::debug.setPath(path);
}

struct SIBinaryDictEntry *SIBinaryGetSIDictEntry(struct SIBinary *self)
{
	Binary *bin = (Binary *) self;
	return (SIBinaryDictEntry *) bin->GetSIDictEntry();
}

