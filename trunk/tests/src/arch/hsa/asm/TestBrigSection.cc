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

#include "gtest/gtest.h"

#include <lib/cpp/Error.h>

#include <arch/hsa/asm/BrigSection.h>

namespace HSA 
{

TEST(TestBrigSection, TestGetName) 
{
	// Setup buffer
	std::unique_ptr<char> buffer = std::unique_ptr<char>(new char[24]);	
	BrigSectionHeader *header = (BrigSectionHeader *)buffer.get();
	header->byteCount = 24;
	header->headerByteCount = 24;
	header->nameLength = 8;
	char *name = (char *)&header->name;
	strncpy(name, "abcdefgh", 8);

	// Create BrigSection
	BrigSection section(buffer.get());

	// Compare result
	EXPECT_STREQ("abcdefgh", section.getName().c_str());
}

}
