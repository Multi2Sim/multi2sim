/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef M2C_SI2BIN_METADATA_H
#define M2C_SI2BIN_METADATA_H

#include <memory>
#include <vector>

#include <arch/southern-islands/asm/Arg.h>


namespace si2bin
{

/*
 * Metadata
 */
	
class Metadata
{
	friend class OuterBin;

	std::vector<std::unique_ptr<SI::Arg>> arg_list;
	
	int uniqueid;
	int uavprivate;
	int hwregion;
	int hwlocal;

	/* Constructor */
	Metadata();

public:

	
	/* Add args */

	/* getters */
	SI::Arg *getArg(unsigned int index) { return index < arg_list.size() ? 
			arg_list[index].get() : nullptr; }	
	unsigned int getArgCount() { return arg_list.size(); }
	const std::vector<std::unique_ptr<SI::Arg>> &getArgList() { return arg_list; }
	int getUniqueId() { return uniqueid; }
	int getUAVPrivate() { return uavprivate; }
	int getHWRegion() { return hwregion; }
	int getHWLocal() { return hwlocal; }

	/* setters */
	void setUniqueId(int uniqueid) { this->uniqueid = uniqueid; }
	void setUAVPrivate(int uavprivate) { this->uavprivate = uavprivate; }
	void setHWRegion(int hwregion) { this->hwregion = hwregion; }
	void setHWLocal(int hwlocal) { this->hwlocal = hwlocal; }
	
	SI::Arg *newArgValue(const std::string &name, SI::ArgDataType data_type,
			int num_elems, int constant_buffer_num, int constant_offset)
	{
		arg_list.push_back(std::unique_ptr<SI::Arg>(new SI::ArgValue(name, data_type,
				num_elems, constant_buffer_num, constant_offset)));
		return arg_list.back().get();
	}
	
	SI::Arg *newArgPointer(const std::string &name, SI::ArgDataType data_type,
			int num_elems, int constant_buffer_num, int constant_offset,
			SI::ArgScope scope, int buffer_num, int alignment,
			SI::ArgAccessType access_type)
	{
		arg_list.push_back(std::unique_ptr<SI::Arg>(new SI::ArgPointer(name, data_type,
				num_elems, constant_buffer_num, constant_offset, scope, buffer_num,
				alignment, access_type)));
		return arg_list.back().get();
	}
};

} /* namespace si2bin */

#endif

