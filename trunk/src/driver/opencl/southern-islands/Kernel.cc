/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

 
#include <lib/cpp/Misc.h>
#include "Kernel.h"

namespace SI
{

static const char *OpenCLErrSIKernelMetadata =
	"The kernel binary loaded by your application is a valid ELF file. In this"
	"file, a '.rodata' section contains specific information about the OpenCL"
	"kernel. However, this information is only partially supported by Multi2Sim."
	"To request support for this error, please email 'development@multi2sim.org'.";

// Private functions

void Kernel::Expect(std::vector<std::string> &token_list, std::string head_token)
{
	std::string token = token_list.at(0);

	if (token == head_token)
		misc::fatal("%s: token '%s' expected, '%s' found.\n%s",
				__FUNCTION__, head_token.c_str(), token_list[0].c_str(),
				OpenCLErrSIKernelMetadata);
}

void Kernel::ExpectInt(std::vector<std::string> &token_list)
{
	std::string token = token_list.at(0);
	misc::StringError err;

	StringToInt(token, err);
	if (err)
	{
		misc::fatal("%s: integer number expected, '%s' found.\n%s",
				__FUNCTION__, token.c_str(),
				OpenCLErrSIKernelMetadata);		
	}
}

void Kernel::ExpectCount(std::vector<std::string> &token_list, unsigned count)
{
	std::string head_token = token_list.at(0);

	if (token_list.size() != count)
	{
		misc::fatal("%s: %d tokens expected for '%s', %d found.\n%s",
				__FUNCTION__, count, head_token.c_str(), token_list.size(),
				OpenCLErrSIKernelMetadata);
	}
}

void Kernel::LoadMetaDataV3()
{

}

void Kernel::LoadMetaData()
{

}


// Public functions

Kernel::Kernel(int id, std::string name, Program *program)
{
	this->id = id;
	this->name = name;
	this->program = program;
}



}  // namespace SI
