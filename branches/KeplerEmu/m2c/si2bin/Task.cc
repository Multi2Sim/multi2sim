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

#include <arch/southern-islands/asm/Inst.h>
#include <lib/cpp/Misc.h>

#include "Symbol.h"
#include "Task.h"



using namespace misc;

namespace si2bin
{

Task::Task(int offset, Symbol *symbol, ELFWriter::Buffer *buffer)
{
	this->offset = offset;
	this->symbol = symbol;
	this->buffer = buffer;

}


void Task::Dump(std::ostream &os)
{
	os << "offset = 0x" << std::hex << offset << std::dec
		<< ", symbol={" << *symbol << "}";
}


void Task::Process()
{
	SI::InstBytes inst;

	// Check whether symbol is resolved
	if (!symbol)
		fatal("undefined symbol");
	if (!symbol->GetDefined())
		fatal("undefined label: %s", symbol->GetName().c_str());
	
	// Resolve label
	assert((offset >= 0) && (buffer->getSize() - 4));
	buffer->setReadPosition(offset);
	buffer->Read((char *) &inst, sizeof(SI::InstBytesSOPP));
	inst.sopp.simm16 = (symbol->GetValue() - offset) / 4 - 1;
	buffer->setWritePosition(offset);
	buffer->Write((char *) &inst, sizeof(SI::InstBytesSOPP));
}

}  /* namespace si2bin */

