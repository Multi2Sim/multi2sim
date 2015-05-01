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

#include "../BrigFile.h"
#include "../BrigCodeEntry.h"

namespace HSA
{

TEST(BrigCodeEntry, should_dump_module_directive_correctly)
{
	// Mock up an BrigFile Class
	class MockupBrigFile : public BrigFile
	{
	public: 
		const std::string getStringByOffset(unsigned int offset) const
			override
		{
			return "&module";
		}
	};
	MockupBrigFile *file = new MockupBrigFile();

	// Setup directive buffer
	auto module = misc::new_unique<BrigDirectiveModule>();
	module->base.byteCount = 20;
	module->base.kind = BRIG_KIND_DIRECTIVE_MODULE;
	module->name = 0;
	module->hsailMajor = 12;
	module->hsailMinor = 34;
	module->profile = BRIG_PROFILE_BASE;
	module->machineModel = BRIG_MACHINE_SMALL;
	module->defaultFloatRound = BRIG_ROUND_FLOAT_DEFAULT;

	// Create a string stream
	std::ostringstream os;

	// Init object
	BrigCodeEntry dir((char *)module.get());
	file->getStringByOffset(0);
	((BrigFile *)file)->getStringByOffset(0);
	dir.setBinary(file);

	// Dump
	dir.Dump(os);	

	// Result
	std::string output = os.str();
	EXPECT_STREQ("module &module:12:34:$base:$small:$default;\n",
			output.c_str());
}

}


