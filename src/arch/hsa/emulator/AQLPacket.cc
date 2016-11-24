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

#include <cinttypes>
#include "AQLPacket.h"

namespace HSA
{

AQLPacket::AQLPacket()
{}


AQLPacket::~AQLPacket()
{}


void AQLPacket::Assign()
{
	setFormat(AQLFormatDispatch);
}


void AQLPacket::setFormat(unsigned char format)
{
	setByOffset<unsigned char>(0, format);
}


unsigned char AQLPacket::getFormat() const
{
	return getByOffset<unsigned char>(0);
}


void AQLDispatchPacket::Dump(std::ostream &os = std::cout) const
{
	os << "\t***** Packet *****\n";
	os << misc::fmt("\tformat: 0x%x, \n", getFormat());
	os << misc::fmt("\tdimension: %d, \n", getDimension());
	os << misc::fmt("\twork group size: (%d, %d, %d), \n", getWorkGroupSizeX(),
			getWorkGroupSizeY(), getWorkGroupSizeZ());
	os << misc::fmt("\tgrid size: (%d, %d, %d), \n", getGridSizeX(),
				getGridSizeY(), getGridSizeZ());
	os << misc::fmt("\tprivate segment size: %d, \n",
			getPrivateSegmentSizeBytes());
	os << misc::fmt("\tgroup segment size: %d, \n",
				getGroupSegmentSizeBytes());
	os << misc::fmt("\tkernel object address: 0x%llx, \n",
			getKernalObjectAddress());
	os << misc::fmt("\tkernel argument address: 0x%llx, \n",
			getKernargAddress());
	os << misc::fmt("\tcompletion signal: 0x%" PRIx64 "\n",
			getCompletionSignal());
	os << "\t***** ****** *****\n";
}

}  // namespace HSA


