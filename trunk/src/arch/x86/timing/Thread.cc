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

#include "Thread.h"

namespace x86
{

Thread::Thread(const std::string &name, CPU *cpu, Core *core, int id_in_core)
	:
	name(name), cpu(cpu), core(core), id_in_core(id_in_core)
{
	// Initialize Uop queue

	// Initialize Load/Store queue

	// Initialize Instruction queue

	// Initialize fetch queue

	// Initialize branch predictor
	std::string branch_predictor_name = this->name + ".Branch Predictor";
	branch_predictor.reset(new BranchPredictor(branch_predictor_name));

	// Initialize trace cache
	if (TraceCache::getPresent())
	{
		std::string trace_cache_name = this->name + ".Trace Cache";
		trace_cache.reset(new TraceCache(trace_cache_name));
	}

	// Initialize register file
	reg_file.reset(new RegisterFile(this->core, this));
	reg_file->InitRegisterFile();

}

}
