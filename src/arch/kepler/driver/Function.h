/*
d *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_KEPLER_DRIVER_FUNCTION_H
#define ARCH_KEPLER_DRIVER_FUNCTION_H

#include <memory>
#include <vector>

#include"Function-arg.h"
#include"Module.h"


namespace Kepler
{

class Function
{
	// ID
	int id;

	// Name
	std::unique_ptr<char> name;

	// Module ID
	unsigned module_id;

	// Instruction binary
	int inst_bin_size;
	unsigned long long *inst_buffer;

	// Number of GPRs used by one thread
	int num_gpr;

	// Arguments
	int arg_count;
	Argument **arg_array;

public:

	/// Constructor
	Function(Module *module, char *function_name);

	/// Destructor
	~Function();

	/// Static Function List
	static std::vector<Function*> function_list;

	/// Get Instruction binary
	int getInstructionBinarySize() const { return inst_bin_size; }

	/// Get Instruction buffer
	unsigned long long* getInstructionBuffer() const { return inst_buffer; }

	/// Get Number of GPRS
	int getNumberofGPR() const { return num_gpr; }

	/// Get Argument count
	int getArgCount() const { return arg_count; }

	/// Get Argument member
	Argument* getArgMember (int i) const {return arg_array[i];}

	/// Set Argument count
	void setArgCount(int arg_count)	{ this->arg_count = arg_count; }

	/// Set Argument member
	void setArgMember (Argument* arg, int index) { this->arg_array[index] = arg; }
};

} // namespace Kepler

#endif
