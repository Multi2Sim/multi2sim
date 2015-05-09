/*
 *  Multi2Sim
 *  Copyright (C) 2014  Shi Dong (dong.sh@husky.neu.edu)
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

#include <lib/cpp/IniFile.h>
#include <arch/x86/timing/BranchPredictor.h>

namespace x86
{

TEST(ReadINIFile, TestBranchPredictor)
{
	// Setup configuration file
	std::string config =
		"[ BranchPredictor ]\n"
		"Kind = Perfect\n"
		"BTB.Sets = 128\n"
		"BTB.Assoc = 16\n"
		"Bimod.Size = 512\n"
		"Choice.Size = 2048\n"
		"RAS.Size = 64\n"
		"TwoLevel.L1Size = 2\n"
		"TwoLevel.L2Size = 2048\n"
		"TwoLevel.HistorySize = 16";

	// Set up INI file
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	// Setup Branch Predictor
	BranchPredictor branch_predictor;

	// Find target section
	for (int i = 0; i < ini_file.getNumSections(); i++)
	{
		std::string section_name = ini_file.getSection(i);
		if (misc::StringPrefix(section_name, "BranchPredictor"))
		{
			branch_predictor.ParseConfiguration(section_name, ini_file);
		}
	}

	// Output stream
	std::ostringstream oss;

	// Assertions
	branch_predictor.DumpConfig(oss);
	std::string target = "\n***** BranchPredictor *****\n"
			"\tBTB.Sets: 128\n"
			"\tBTB.Assoc: 16\n"
			"\tBimod.Size: 512\n"
			"\tChoice.Size: 2048\n"
			"\tRAS.Size: 64\n"
			"\tTwoLevel.L1Size: 2\n"
			"\tTwoLevel.L2Size: 2048\n"
			"\tTwoLevel.HistorySize: 16\n";
	EXPECT_STREQ(target.c_str(), oss.str().c_str());
}

}




