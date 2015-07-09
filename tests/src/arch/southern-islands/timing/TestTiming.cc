/*
 *  Multi2Sim
 *  Copyright (C) 2015  Rafael Ubal (ubal@ece.neu.edu)
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

#include <gtest/gtest.h>

#include <arch/southern-islands/timing/Timing.h>
#include <lib/cpp/IniFile.h>

namespace SI {

/*
TEST(TestTiming, ParseMemoryConfigurationEntry)
{
	//Initialize instance of the SI timing simulator
	std::unique_ptr<Timing> timing = Timing::getInstance();

	//create inifile to store generated trace
	misc::IniFile *sample;

	//write memory configuration to ini_file
	Timing->WriteMemoryConfiguration(ini_file);

	//print out config somehow...

	//create inifile
	std::string config =
			"blarg";
	
	//convert string to inifile
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);


	//insert tests for thrown errors here
	timing.ParseMemoryConfigurationEntry(&ini_file, section??);
}
*/
TEST(TestTiming, ParseConfiguration)
{
	//get instance of timing simulator
	Timing* timing = Timing::getInstance();

	//create config file
	std::string config = 
			"[ General ]\n"
			"Frequency = 1001";
	
	//load config into IniFile object
	misc::IniFile ini_file;
	ini_file.LoadFromString(config);

	//try ParseConfiguration for invalid frequency
	try
	{
		timing->ParseConfiguration(&ini_file);
	}
	catch(std::string error)
	{
		//test to confirm correct error message
		EXPECT_EQ("test", error) << "Thrown error not correct";
	}

	//try frequency lower than bounds
	config = 
		"[ General ]\n"
		"Frequency = .5";
	
	//store config in ini_file
	misc::IniFile ini_file_b;
	ini_file_b.LoadFromString(config);

	//try ParseConfiguration for invalid frequency
	try
	{
		timing->ParseConfiguration(&ini_file);
	}
	catch(std::string error)
	{
		EXPECT_EQ("test", error) << "Thrown error not correct";
	}

}

} //SI
