#include "gtest/gtest.h"

#include <lib/cpp/Error.h>

#include "../BrigFile.h"

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
	EXPECT_THROW(file.LoadFileByPath("no_such_file.brig"), misc::Error);

	// Should throw error if the file is not of brig type
	EXPECT_THROW(file.LoadFileByPath("Makefile"), misc::Error);
}

}
