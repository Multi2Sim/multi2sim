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

#include "Core.h"
#include "Uop.h"
#include "Thread.h"


namespace x86
{

long long Uop::id_counter = 0;


Uop::Uop(Thread *thread,
		Context *context,
		std::shared_ptr<Uinst> uinst) :
		thread(thread),
		context(context),
		uinst(uinst)
{
	// Initialize
	core = thread->getCore();
	id = ++id_counter;
	id_in_core = core->getUopId();

	// Assign flags from associated micro-instruction
	flags = uinst->getFlags();

	// Populate dependency fields
	CountDependencies();
}


void Uop::CountDependencies()
{
	// Output dependences
	int int_count = 0;
	int fp_count = 0;
	int flag_count = 0;
	int xmm_count = 0;
	for (int dep = 0; dep < Uinst::MaxODeps; dep++)
	{
		Uinst::Dep loreg = uinst->getODep(dep);
		if (Uinst::isFlagDependency(loreg))
			flag_count++;
		else if (Uinst::isIntegerDependency(loreg))
			int_count++;
		else if (Uinst::isFloatingPointDependency(loreg))
			fp_count++;
		else if (Uinst::isXmmDependency(loreg))
			xmm_count++;
	}
	num_outputs = flag_count + int_count + fp_count + xmm_count;
	num_integer_outputs = flag_count && !int_count ? 1 : int_count;
	num_floating_point_outputs = fp_count;
	num_xmm_outputs = xmm_count;

	// Input dependences
	int_count = 0;
	fp_count = 0;
	flag_count = 0;
	xmm_count = 0;
	for (int dep = 0; dep < Uinst::MaxIDeps; dep++)
	{
		Uinst::Dep loreg = uinst->getIDep(dep);
		if (Uinst::isFlagDependency(loreg))
			flag_count++;
		else if (Uinst::isIntegerDependency(loreg))
			int_count++;
		else if (Uinst::isFloatingPointDependency(loreg))
			fp_count++;
		else if (Uinst::isXmmDependency(loreg))
			xmm_count++;
	}
	num_inputs = flag_count + int_count + fp_count + xmm_count;
	num_integer_inputs = flag_count + int_count;
	num_floating_point_inputs = fp_count;
	num_xmm_inputs = xmm_count;
}


void Uop::Dump(std::ostream &os) const
{
	// Fields
	os << "id = " << id << ", ";
	os << misc::fmt("eip = 0x%x, ", eip);
	os << misc::fmt("spec = %c, ", speculative_mode ? 't' : 'f');
	os << misc::fmt("first_spec = %c, ", first_speculative_mode ? 't' : 'f');
	os << misc::fmt("trace_cache = %c, ", from_trace_cache ? 't' : 'f');

	// Memory access
	if (memory_access)
		os << misc::fmt("memory_access = %lld, ", memory_access);

	// Micro-instruction
	os << "uinst = '" << *uinst << "'";
}


}
