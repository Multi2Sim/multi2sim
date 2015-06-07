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
#include "CPU.h"

namespace x86
{

Thread::Thread(const std::string &name, CPU *cpu, Core *core, int id_in_core) :
		name(name),
		cpu(cpu),
		core(core),
		id_in_core(id_in_core)
{
	// Initialize Uop queue

	// Initialize Load/Store queue

	// Initialize Instruction queue

	// Initialize fetch queue

	// Initialize reorder buffer
	reorder_buffer_left_bound = this->id_in_core * CPU::getReorderBufferSize();
	reorder_buffer_right_bound = (this->id_in_core + 1) * CPU::getReorderBufferSize() - 1;
	reorder_buffer_head = reorder_buffer_left_bound;
	reorder_buffer_tail = reorder_buffer_left_bound;

	// Initialize branch predictor
	branch_predictor = misc::new_unique<BranchPredictor>(name +
			".BranchPredictor");

	// Initialize trace cache
	if (TraceCache::getPresent())
		trace_cache = misc::new_unique<TraceCache>(name +
				".TraceCache");

	// Initialize register file
	reg_file = misc::new_unique<RegisterFile>(core, this);
	reg_file->InitRegisterFile();
}

}
