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
	std::vector<std::unique_ptr<SI::Arg>> arg_list;
	
	int uniqueid;
	int uavprivate;
	int hwregion;
	int hwlocal;

public:

	/* Constructor */
	Metadata();

	/* Add args */

	/* Getters */
	SI::Arg *GetArg(unsigned int index) { return index < arg_list.size() ? 
			arg_list[index].get() : nullptr; }	
	int GetUniqueId() { return uniqueid; }
	int GetUAVPrivate() { return uavprivate; }
	int GetHWRegion() { return hwregion; }
	int GetHWLocal() { return hwlocal; }
	unsigned int GetArgCount() { return arg_list.size(); }

	/* Setters */
	void SetUniqueId(int uniqueid) { this->uniqueid = uniqueid; }
	void SetUAVPrivate(int uavprivate) { this->uavprivate = uavprivate; }
	void SetHWRegion(int hwregion) { this->hwregion = hwregion; }
	void SetHWLocal(int hwlocal) { this->hwlocal = hwlocal; }
	
	void AddArg(SI::Arg *arg);
	
};

} /* namespace si2bin */

#endif

