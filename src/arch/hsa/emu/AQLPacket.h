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

#ifndef ARCH_HSA_EMU_AQLPACKET_H
#define ARCH_HSA_EMU_AQLPACKET_H

#include <lib/cpp/String.h>
#include <lib/cpp/Error.h>

namespace HSA
{

enum AQLFormat
{
	AQL_FORMAT_ALWAYS_RESERVED = 0,
	AQL_FORMAT_INVALID,
	AQL_FORMAT_DISPATCH,
	AQL_FORMAT_BARRIER,
	AQL_FORMAT_AGENT_DISPATCH
};

/// Represent an AQL packet for HSA agent to dispatch task
class AQLPacket
{

	// Content of the packet, which is always 64 bytes long.
	char bytes[64];

	/// Get field by offset
	template <typename Type>
	Type getByOffset(unsigned short offset) const
	{
		if (!misc::inRange(offset, 0, 63))
			throw misc::Panic(misc::fmt("Trying to access invalid"
					"index %d of an AQL packet", offset));
		return *((Type *)(&(bytes[offset])));
	}

	/// Set field by offset
	template <typename Type>
	void setByOffset(unsigned short offset, Type value)
	{
		if (!misc::inRange(offset, 0, 63))
			throw misc::Panic(misc::fmt("Trying to access invalid"
					"index %d of an AQL packet", offset));
		unsigned int size = sizeof(Type);
		char *buffer = (char *)(&value);
		for (unsigned int i = 0; i < size; i++)
		{
			bytes[offset + i] = buffer[i];
		}
	}

public:

	/// Constructor
	AQLPacket();

	/// Destructor
	virtual ~AQLPacket();

	/// Assign the packet to the HSA Packet Processor, by changing the AQL
	/// packet format field from INVALID or ALWAYS_RESERVED
	void Assign();




	//
	// Setters and getters
	//

	/// Returns the pointer to the beginning of the packet buffer
	char *getBuffer(){ return bytes; }

	/// Get the format the field in the header
	unsigned char getFormat() const;

	/// Set format field in the header
	void setFormat(unsigned char format);

	/// Returns the header as a whole in unsigned integer
	unsigned short getHeader() const{ return getByOffset<unsigned short>(0);}

	/// Set the header in whole
	void setHeader(unsigned short header){ setByOffset<unsigned int>(2, header); }

	/// Returns the dimension
	unsigned short getDimension() const{ return getByOffset<unsigned short>(2); }

	/// Set the dimension field
	void setDimension(unsigned short dim){ setByOffset<unsigned short>(2, dim); }

	/// Returns the work group size x
	unsigned short getWorkGroupSizeX() const{ return getByOffset<unsigned short>(4); }

	/// Set the work group size x
	void setWorkGroupSizeX(unsigned short wg_size_x){ setByOffset<unsigned short>(4, wg_size_x); }

	/// Returns the work group size y
	unsigned short getWorkGroupSizeY() const{ return getByOffset<unsigned short>(6); }

	/// Set the work group size y
	void setWorkGroupSizeY(unsigned short wg_size_y){ setByOffset<unsigned short>(6, wg_size_y); }

	/// Returns the work group size z
	unsigned short getWorkGroupSizeZ() const{ return getByOffset<unsigned short>(8); }

	/// Set the work group size z
	void setWorkGroupSizeZ(unsigned int wg_size_z){ setByOffset<unsigned short>(8, wg_size_z); }

	/// Returns the grid size x
	unsigned int getGridSizeX() const{ return getByOffset<unsigned int>(12); }

	/// Set the grid size x
	void setGridSizeX(unsigned int grid_size_x){ setByOffset<unsigned int>(12, grid_size_x); }

	/// Returns the grid size y
	unsigned int getGridSizeY() const{ return getByOffset<unsigned int>(16); }

	/// Set the grid size y
	void setGridSizeY(unsigned int grid_size_y){ setByOffset<unsigned int>(16, grid_size_y); }

	/// Returns the grid size z
	unsigned int getGridSizeZ() const{ return getByOffset<unsigned int>(20); }

	/// Set the grid size z
	void setGridSizeZ(unsigned int grid_size_z){ setByOffset<unsigned int>(20, grid_size_z); }

	/// Returns primary segment size in bytes per work-item
	unsigned int getPrivateSegmentSizeBytes() const{ return getByOffset<unsigned int>(24); }

	/// Set primary segment size in bytes per work-item
	void setPrivateSegmentSizeBytes(unsigned int size){ setByOffset<unsigned int>(24, size); }

	/// Returns group memory size in bytes per work-group
	unsigned int getGroupSegmentSizeBytes() const{ return getByOffset<unsigned int>(28); }

	/// Set group memory size in bytes per work-group
	void setGroupSegmentSizeBytes(unsigned int size){ setByOffset<unsigned int>(28, size); }

	/// Returns the address to kernel object
	unsigned long long getKernalObjectAddress() const{ return getByOffset<unsigned long long>(32); }

	/// Set the address to kernel object
	void setKernalObjectAddress(unsigned long long address){ setByOffset<unsigned long long>(32, address); }

	/// Returns the address to kernel arguments
	unsigned long long getKernargAddress() const{ return getByOffset<unsigned long long>(40); }

	/// Set the address to kernel argument
	void setKernargAddress(unsigned long long address){ setByOffset<unsigned long long>(40, address); }

	/// Returns the completion signal
	unsigned long long getCompletionSignal() const{ return getByOffset<unsigned long long>(56); }

	/// Set the address to kernel argument
	void setCompletionSignal(unsigned long long signal){ setByOffset<unsigned long long>(56, signal); }

};

}  // namespace HSA

#endif

