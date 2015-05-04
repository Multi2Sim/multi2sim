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

#include <lib/cpp/Misc.h>
#include <lib/cpp/Error.h>

#include "../BrigImmed.h"

namespace HSA
{

TEST(TestBrigImmed, should_dump_b128_right)
{
	// Set buffer
	std::unique_ptr<char> buffer = std::unique_ptr<char>(new char[16]);
	memset(buffer.get(), 12, 16);

	// Init immed
	BrigImmed immed((const unsigned char *)buffer.get(), BRIG_TYPE_B128);

	// Output
	std::ostringstream os;
	immed.Dump(os);

	// Result
	EXPECT_STREQ("u8x16(12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12)", 
			os.str().c_str());
}


TEST(TestBrigImmed, should_dump_signal_32_right)
{
	// Set buffer
	std::unique_ptr<char> buffer = std::unique_ptr<char>(new char[4]);
	*(unsigned int *)(buffer.get()) = 12;

	// Init immed
	BrigImmed immed((const unsigned char *)buffer.get(), BRIG_TYPE_SIG32);

	// Output
	std::ostringstream os;
	immed.Dump(os);

	// Result
	EXPECT_STREQ("sig32(12)", os.str().c_str());
}


TEST(TestBrigImmed, should_dump_signal_64_right)
{
	// Set buffer
	std::unique_ptr<char> buffer = std::unique_ptr<char>(new char[8]);
	*(unsigned long long *)(buffer.get()) = 23;

	// Init immed
	BrigImmed immed((const unsigned char *)buffer.get(), BRIG_TYPE_SIG64);

	// Output
	std::ostringstream os;
	immed.Dump(os);

	// Result
	EXPECT_STREQ("sig64(23)", os.str().c_str());
}


}


