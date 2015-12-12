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

#ifndef ARCH_HSA_EMULATOR_AQLPACKET_H
#define ARCH_HSA_EMULATOR_AQLPACKET_H

#include <lib/cpp/String.h>
#include <lib/cpp/Error.h>


namespace HSA
{

enum AQLFormat
{
	AQLFormatAlwaysReserved = 0,
	AQLFormatInvalid,
	AQLFormatDispatch,
	AQLFormatBarrier,
	AQLFormatAgentDispatch
};

/// Represent an AQL packet for HSA agent to dispatch task
class AQLPacket
{
protected:

	// Content of the packet, which is always 64 bytes long.
	char bytes[64];

	// Get field by offset
	template <typename Type>
	Type getByOffset(unsigned short offset) const
	{
		if (!misc::inRange(offset, 0, 63))
			throw misc::Panic(misc::fmt("Trying to access invalid"
					"index %d of an AQL packet", offset));
		return *((Type *)(&(bytes[offset])));
	}

	// Set field by offset
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
	~AQLPacket();

	/// Assign the packet to the HSA Packet Processor, by changing the AQL
	/// packet format field from INVALID or ALWAYS_RESERVED
	void Assign();

	/// Returns the pointer to the beginning of the packet buffer
	char *getBuffer(){ return bytes; }

	/// Get the format the field in the header
	unsigned char getFormat() const;

	/// Set format field in the header
	void setFormat(unsigned char format);

	/// Return the header as a whole in unsigned integer
	unsigned short getHeader() const{ return getByOffset<unsigned short>(0);}

	/// Set the header in whole
	void setHeader(unsigned short header){ setByOffset<unsigned int>(2, header); }



};


// An AQLDispatchPacket encapsulates information required to launch a kernel
class AQLDispatchPacket : public AQLPacket
{
public:

	// Constructor
	AQLDispatchPacket()
	{};

	// Destructor
	~AQLDispatchPacket()
	{};

	/// Return the dimension
	unsigned short getDimension() const 
	{ 
		return getByOffset<unsigned short>(2); 
	}

	/// Set the dimension field
	void setDimension(unsigned short dim)
	{ 
		setByOffset<unsigned short>(2, dim); 
	}

	/// Return the work group size x
	unsigned short getWorkGroupSizeX() const 
	{ 
		return getByOffset<unsigned short>(4); 
	}

	/// Set the work group size x
	void setWorkGroupSizeX(unsigned short wg_size_x)
	{ 
		setByOffset<unsigned short>(4, wg_size_x); 
	}

	/// Return the work group size y
	unsigned short getWorkGroupSizeY() const 
	{ 
		return getByOffset<unsigned short>(6); 
	}

	/// Set the work group size y
	void setWorkGroupSizeY(unsigned short wg_size_y)
	{ 
		setByOffset<unsigned short>(6, wg_size_y); 
	}

	/// Return the work group size z
	unsigned short getWorkGroupSizeZ() const 
	{ 
		return getByOffset<unsigned short>(8); 
	}

	/// Set the work group size z
	void setWorkGroupSizeZ(unsigned int wg_size_z)
	{ 
		setByOffset<unsigned short>(8, wg_size_z); 
	}


	/// Set 3 work group size at one time
	void setWorkGroupSize(unsigned x, unsigned y, unsigned z)
	{
		setWorkGroupSizeX(x);
		setWorkGroupSizeY(y);
		setWorkGroupSizeZ(z);
	}

	/// Return the grid size x
	unsigned int getGridSizeX() const 
	{ 
		return getByOffset<unsigned int>(12); 
	}

	/// Set the grid size x
	void setGridSizeX(unsigned int grid_size_x)
	{ 
		setByOffset<unsigned int>(12, grid_size_x); 
	}

	/// Return the grid size y
	unsigned int getGridSizeY() const 
	{ return getByOffset<unsigned int>(16); }

	/// Set the grid size y
	void setGridSizeY(unsigned int grid_size_y){ setByOffset<unsigned int>(16, grid_size_y); }

	/// Set 3 grid size at one time
	void setGridSize(unsigned x, unsigned y, unsigned z)
	{
		setGridSizeX(x);
		setGridSizeY(y);
		setGridSizeZ(z);
	}

	/// Return the grid size z
	unsigned int getGridSizeZ() const { return getByOffset<unsigned int>(20); }

	/// Set the grid size z
	void setGridSizeZ(unsigned int grid_size_z){ setByOffset<unsigned int>(20, grid_size_z); }

	/// Return primary segment size in bytes per work-item
	unsigned int getPrivateSegmentSizeBytes() const { return getByOffset<unsigned int>(24); }

	/// Set primary segment size in bytes per work-item
	void setPrivateSegmentSizeBytes(unsigned int size){ setByOffset<unsigned int>(24, size); }

	/// Return group memory size in bytes per work-group
	unsigned int getGroupSegmentSizeBytes() const { return getByOffset<unsigned int>(28); }

	/// Set group memory size in bytes per work-group
	void setGroupSegmentSizeBytes(unsigned int size){ setByOffset<unsigned int>(28, size); }

	/// Return the address to kernel object
	unsigned long long getKernalObjectAddress() const { return getByOffset<unsigned long long>(32); }

	/// Set the address to kernel object
	void setKernalObjectAddress(unsigned long long address){ setByOffset<unsigned long long>(32, address); }

	/// Return the address to kernel arguments
	unsigned long long getKernargAddress() const { return getByOffset<unsigned long long>(40); }

	/// Set the address to kernel argument
	void setKernargAddress(unsigned long long address){ setByOffset<unsigned long long>(40, address); }

	/// Return the completion signal
	uint64_t getCompletionSignal() const { return getByOffset<uint64_t>(56); }

	/// Dump the AQL dispatch packet
	void Dump(std::ostream &os) const;

	/// Operator \c << invoking the function Dump) on an output stream
	friend std::ostream &operator<<(std::ostream &os,
			const AQLDispatchPacket &packet)
	{
		packet.Dump(os);
		return os;
	}

};

}  // namespace HSA

#endif

