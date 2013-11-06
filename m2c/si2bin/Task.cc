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

#include <lib/cpp/Misc.h>

#include "Symbol.h"
#include "Task.h"


using namespace misc;

namespace si2bin
{

void Task::Dump(std::ostream &os)
{
	os << "offset = 0x" << std::hex << offset << std::dec
		<< ", symbol={" << *symbol << "}";
}


void Task::Process()
{
	panic("%s: not implemented", __FUNCTION__);
#if 0
	SI::InstBytes *inst;

	/* Check whether symbol is resolved */
	if (!defined)
		fatal("undefined label: %s", symbol->GetName().c_str());

	/* Resolve label */
	assert(InRange(offset, 0, si2bin_entry->text_section_buffer->offset - 4));
	inst = si2bin_entry->text_section_buffer->ptr + self->offset;
	inst->sopp.simm16 = (label->value - self->offset) / 4 - 1;
#endif
}

}  /* namespace si2bin */

