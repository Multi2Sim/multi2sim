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

#include "Emu.h"
#include "Registers.h"

namespace HSA
{

Registers::Registers()
{
}


Registers::~Registers()
{
}


void Registers::setCRegister(const std::string &name, char *value)
{
	throw std::logic_error("Unimplemented setCRegister");
}


char *Registers::getCRegister(const std::string &name)
{
	throw std::logic_error("Unimplemented getCRegister");
	return nullptr;
}


unsigned int Registers::getSizeByName(const std::string &name) const
{

	// First byte is '$'
	if (name[0] != '$')
	{
		throw Error("Invalid register name " + name 
				+ ". Expect ($) to be register prefix");
		return 0;
	}

	// Get the index number of the register
	unsigned int register_index = atoi(name.c_str() + 2);

	switch (name[1])
	{
	case 'c':
		if (register_index >= 0 && register_index <= 7)
		{
			throw Error("Invalid register name " + name 
					+ ". Expecte register index between"
					"0 and 7");
			return 0;
		}
		return 1;
	case 's':
		if (register_index >= 0 && register_index <= 127)
		{
			throw Error("Invalid register name " + name 
					+ ". Expecte register index between"
					"0 and 127");
			return 0;
		}
		return 4;
	case 'd':
		if (register_index >= 0 && register_index <= 63)
		{
			throw Error("Invalid register name " + name 
					+ ". Expecte register index between"
					"0 and 63");
			return 0;
		}
		return 8;
	case 'q':
		if (register_index >= 0 && register_index <= 31)
		{
			throw Error("Invalid register name " + name 
					+ ". Expecte register index between"
					"0 and 31");
			return 0;
		}
		return 16;
	default:
		throw Error("Invalid register name " + name +
				+ ". Expect register type to be (c, s, d, q)");
		return 0;
	}
}


char *Registers::getRegister(const std::string &name)
{
	// Use special method for c registers
	if (name[1] == 'c')
	{
		return getCRegister(name);
	}

	// Check if the function is loaded
	std::map<std::string, unsigned int>::iterator it = 
			register_info.find(name);
	if (it == register_info.end())
	{
		// register not found
		throw std::logic_error("Register not allocated");
		return nullptr;
	}

	// Get offset to certain register
	unsigned int offset = register_info[name];
	return bytes + offset;
}


void Registers::setRegister(const std::string &name, char *value)
{
	// get the size of the register to be created
	unsigned int size = getSizeByName(name);
	if (!size)
	{
		// Invalid register name, throw error
		throw Error("Invalid register name " + name);
	}

	// Invoke other method to set C registers
	if (name[1] == 'c')
	{
		setCRegister(name, value);
	}

	// offset to the register in the register storage
	unsigned int offset;	

	// If such register has not been used, offset equals allocated_size
	std::map<std::string, unsigned int>::iterator it =
			register_info.find(name);
	if (it == register_info.end())
	{

		offset = allocated_size;

		// insert allocated register into info register info table
		register_info.emplace(name, allocated_size);
		allocated_size += size;
	}

	// Replace register value
	memcpy((void *)(bytes + offset), (void *)value, size);
}


void Registers::Dump(std::ostream &os = std::cout) const
{
	for (std::map<std::string, unsigned int>::const_iterator it 
			= register_info.begin(); 
			it != register_info.end(); it++)
	{
		unsigned char size = getSizeByName(it->first);
		os << it->first << ": 0X";
		for (int i=0; i<size; i++)
			os << std::hex << bytes[it->second + i];
		os << std::dec;
		os << "\n";
	}
}

}  // namespace HSA
