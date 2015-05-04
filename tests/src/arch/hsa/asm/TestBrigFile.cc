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

#include <arch/hsa/asm/BrigFile.h>

namespace HSA 
{

TEST(TestBrigFile, TestIsBrigFile) 
{
	// Should return true if the identification is "HSA BRIG"
	char file[] = "HSA BRIG";
	bool isBrigFile = BrigFile::isBrigFile(file);
	EXPECT_TRUE(isBrigFile);

	// Should return false if the file is of some other types
	char file2[] = "ELF32";
	isBrigFile = BrigFile::isBrigFile(file2);
	EXPECT_FALSE(isBrigFile);
}


TEST(TestBrigFile, TestOpenFileByPath)
{
	// Should throw error if no such file
	BrigFile file;
	EXPECT_THROW(file.LoadFileByPath("no_such_file.brig"), 
			misc::Error);

	// Should throw error if the file is not of brig type
	EXPECT_THROW(file.LoadFileByPath("Makefile"), misc::Error);
}

}
