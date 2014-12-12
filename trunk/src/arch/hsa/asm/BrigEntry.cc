/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#include <lib/cpp/String.h>

#include "BrigDef.h"
#include "BrigSection.h"
#include "BrigEntry.h"

namespace HSA
{

BrigEntry::BrigEntry(const char *buf, const BrigSection *section):
		base(buf),
		section(section)
{
}


unsigned BrigEntry::getSize() const
{
	BrigBase *brig_base = (BrigBase *)base;
	return brig_base->byteCount;
}


unsigned BrigEntry::getKind() const
{
	BrigBase *brig_base = (BrigBase *)base;
	return (unsigned)brig_base->kind;
}


unsigned int BrigEntry::getOffset() const
{
	return base - section->getBuffer();
}


void BrigEntry::Dump(std::ostream &os = std::cout) const
{
	os << misc::fmt("Brig size: %d, kind: %x\n", getSize(), getKind());
}


std::unique_ptr<BrigEntry> BrigEntry::next() const
{
	unsigned int next_offset = getOffset() + getSize();
	return section->getEntryByOffset(next_offset);
}

}  // namespace HSA


