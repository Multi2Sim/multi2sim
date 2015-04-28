#include "gtest/gtest.h"

#include "../BrigFile.h"

namespace HSA 
{

TEST(TestBrigFile, TestIsBrigFile) 
{
	char file[] = "HSA BRIG";
	bool isBrigFile = BrigFile::isBrigFile(file);
	EXPECT_TRUE(isBrigFile);

	char file2[] = "ELF32";
	isBrigFile = BrigFile::isBrigFile(file2);
	EXPECT_FALSE(isBrigFile);
}

}
