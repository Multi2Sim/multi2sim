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

#include <arch/southern-islands/disassembler/Disassembler.h>
#include <arch/southern-islands/emulator/WorkGroup.h>
#include <arch/southern-islands/emulator/Wavefront.h>
#include <arch/southern-islands/emulator/WorkItem.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/ELFReader.h>
#include <lib/cpp/Misc.h>

#include "Emulator.h"
#include "NDRange.h"


namespace SI
{

std::unique_ptr<Emulator> Emulator::instance;

misc::Debug Emulator::isa_debug;

std::string Emulator::isa_debug_file;

long long Emulator::max_instructions;

std::string Emulator::scheduler_debug_file;
 
misc::Debug Emulator::scheduler_debug;

Emulator *Emulator::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance = misc::new_unique<Emulator>();
	return instance.get();	
}


Emulator::Emulator() : comm::Emulator("SouthernIslands")
{
	// Disassemler
	disassembler = Disassembler::getInstance();
	
	// GPU memories
	video_memory = misc::new_unique<mem::Memory>();
	video_memory->setSafe(true);

	shared_memory = misc::new_unique<mem::Memory>();
	shared_memory->setSafe(true);
	global_memory = video_memory.get();
}


void Emulator::DumpSummary(std::ostream &os) const
{
	// FIXME: basic statistics, such as instructions, time...
	comm::Emulator::DumpSummary(os);

	// More statistics 
	os << "NDRangeCount = " << num_ndranges << std::endl;
	os << "WorkGroupCount = " << num_work_groups << std::endl;
	os << "BranchInstructions = " << num_branch_instructions << std::endl;
	os << "LDSInstructions = " << num_lds_instructions << std::endl;
	os << "ScalarALUInstructions = " << num_scalar_alu_instructions << std::endl;
	os << "ScalarMemInstructions = " << num_scalar_memory_instructions << std::endl;
	os << "VectorALUInstructions = " << num_vector_alu_instructions << std::endl;
	os << "VectorMemInstructions = " << num_vector_memory_instructions << std::endl;
}


bool Emulator::Run()
{
	// For efficiency when no Southern Islands emulation is selected, 
	// exit here if the list of existing ND-Ranges is empty. 
	if (!getNumNDRanges())
		return false;

	// NDRange list is shared by CL/GL driver
	for (auto it = getNDRangesBegin(), e = getNDRangesEnd(); it !=e; ++it)
	{
		// Get NDRange
		NDRange *ndrange = it->get();

		// Setup WorkGroup pointer
		WorkGroup *work_group = nullptr;
		
		// Move a single waiting work group to the running work groups 
		// list
		if (!ndrange->isWaitingWorkGroupsEmpty())
		{
			long work_group_id = ndrange->GetWaitingWorkGroup();
			work_group = ndrange->ScheduleWorkGroup(
					work_group_id);
		}

		// If there's no work groups to run, go to next nd-range 
		if (ndrange->isRunningWorkGroupsEmpty())
			continue;
	
		// Normally, we would iterate over the running work group list
		// but in this case there is only a single work group being
		// executed at a time so no loop is needed
		while (!work_group->getFinished())
		{
			// Execute an instruction for each wavefront
			for (auto wf_i = work_group->getWavefrontsBegin(), 
					wf_e = work_group->getWavefrontsEnd();
					wf_i != wf_e;
					++wf_i)
			{
				// Get current wavefront
				Wavefront *wavefront = (*wf_i).get();

				// Check if the wavefront is finished or not
				if (wavefront->getFinished() || wavefront->at_barrier)
					continue;
				
				// Execute the wavefront
				wavefront->Execute();
			}
		}
	
		// Now that the work group is finished, remove it from the
		// running work group list
		ndrange->RemoveWorkGroup(work_group);
		
		// If a context has been suspended while waiting for the ndrange
		// check if it can be woken up.
		ndrange->WakeupContext();
	}

	// Done with all the work
	return true;
}


void Emulator::createBufferDesc(unsigned base_addr, unsigned size,
		int num_elems, Argument::DataType data_type, 
		WorkItem::BufferDescriptor *buffer_descriptor)
{
	int num_format;                                                          
	int data_format;                                                         
	int elem_size;                                                           

	// Check size of buffer descriptor
	assert(sizeof(WorkItem::BufferDescriptor) == 16);                           

	// Initialize num_format and data_format
	num_format = BufDescNumFmtInvalid;
	data_format = BufDescDataFmtInvalid;                              

	// Set num_format and data_format
	switch (data_type)                                                       
	{                                                                        
	case Argument::DataTypeInt8:                                                          
	case Argument::DataTypeUInt8:                                                         

		num_format = BufDescNumFmtSint;                           
		switch (num_elems)                                               
		{                                                                
		case 1:      

			data_format = BufDescDataFmt8;                    
			break;                                                   

		case 2:      

			data_format = BufDescDataFmt8_8;                  
			break;                                                   

		case 4:

			data_format = BufDescDataFmt8_8_8_8;              
			break;                                                   

		default:

			throw Error(misc::fmt("%s: invalid number of i8/u8 elements (%d)", 
					__FUNCTION__, num_elems));
		}                                                                
		elem_size = 1 * num_elems;                                       
		break; 

	case Argument::DataTypeInt16:                                                         
	case Argument::DataTypeUInt16:                                                        

		num_format = BufDescNumFmtSint;                           
		switch (num_elems)                                               
		{                                                                
		case 1:    

			data_format = BufDescDataFmt16;                   
			break;                                                   

		case 2:       

			data_format = BufDescDataFmt16_16;                
			break;                                                   

		case 4:

			data_format = BufDescDataFmt16_16_16_16;          
			break;                                                   

		default:

			throw Error(misc::fmt("%s: invalid number of i16/u16 elements (%d)",     
					__FUNCTION__, num_elems));                
		}                                                                
		elem_size = 2 * num_elems;                                       
		break;    

	case Argument::DataTypeInt32:                                                         
	case Argument::DataTypeUInt32:                                                        

		num_format = BufDescNumFmtSint;                           
		switch (num_elems)                                               
		{                                                                
		case 1:   

			data_format = BufDescDataFmt32;                   
			break;                                                   

		case 2:  

			data_format = BufDescDataFmt32_32;                
			break;                                                   

		case 3: 

			data_format = BufDescDataFmt32_32_32;             
			break;                                                   

		case 4:   

			data_format = BufDescDataFmt32_32_32_32;          
			break;                                                   

		default:

			throw Error(misc::fmt("%s: invalid number of i32/u32 elements (%d)",     
					__FUNCTION__, num_elems));                
		}                                                                
		elem_size = 4 * num_elems;                                       
		break;

	case Argument::DataTypeFloat:                                                         

		num_format = BufDescNumFmtFloat;                          
		switch (num_elems)                                               
		{                                                                
		case 1:  

			data_format = BufDescDataFmt32;                   
			break;                                                   

		case 2:                                                          
			
			data_format = BufDescDataFmt32_32;                
			break;                                                   

		case 3: 

			data_format = BufDescDataFmt32_32_32;             
			break;                                                   

		case 4: 

			data_format = BufDescDataFmt32_32_32_32;          
			break;                                                   

		default:     

			throw Error(misc::fmt("%s: invalid number of float elements (%d)",       
				__FUNCTION__, num_elems));                
		}                                                                
		elem_size = 4 * num_elems;                                       
		break;

	case Argument::DataTypeDouble:                                                        

		num_format = BufDescNumFmtFloat;                          
		switch (num_elems)                                               
		{                                                                
		case 1:               

			data_format = BufDescDataFmt32_32;                
			break;                                                   

		case 2:    

			data_format = BufDescDataFmt32_32_32_32;          
			break;                                                   

		default:                                      
		
			throw Error(misc::fmt("%s: invalid number of double elements (%d)",      
					__FUNCTION__, num_elems));                
		}                                                                
		elem_size = 8 * num_elems;                                       
		break;   

	case Argument::DataTypeStruct:                                                        

		num_format = BufDescNumFmtUint;                           
		data_format = BufDescDataFmt8;                            
		elem_size = 1;                                                   
		break;                                                           

	default:

		throw Error(misc::fmt("%s: invalid data type for SI buffer (%d)",                
				__FUNCTION__, data_type));                                
	}

	// Make sure that num_format and data_format were set
	assert(num_format != BufDescNumFmtInvalid);                       
	assert(data_format != BufDescDataFmtInvalid);                     

	// Set fields of buffer description
	buffer_descriptor->base_addr = base_addr;                                      
	buffer_descriptor->num_format = num_format;                                    
	buffer_descriptor->data_format = data_format;                                  
	assert(!(size % elem_size));                                             
	buffer_descriptor->elem_size = elem_size;                                      
	buffer_descriptor->num_records = size/elem_size;                               

	// Return
	return;   
}


void Emulator::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();
	command_line->setCategory("Southern Islands");
	
	// Option --si-debug-isa <file>
	command_line->RegisterString("--si-debug-isa <file>", isa_debug_file,
			"Debug information for dynamic execution of Southern "
			"Islands instructions, tracking state changes in "
			"register values and memory positions.");

	// Option --si-max-inst <int>
	command_line->RegisterInt64("--si-max-inst <inst>", max_instructions,
			"Maximum number of ISA instructions. An instruction "
			"executed by an entire wavefront counts as 1 toward "
			"this limit. Use 0 (default) for no limit.");

	// Option --si-debug-scheduler
	command_line->RegisterString("--si-debug-scheduler <file>",
			scheduler_debug_file,
			"File to dump how the work groups, and wavefronts are "
			"scheduled on the Gpu's compute units.");
}


void Emulator::ProcessOptions()
{
	isa_debug.setPath(isa_debug_file);
	scheduler_debug.setPath(scheduler_debug_file);
}
	
	
NDRange *Emulator::addNDRange()
{
	// Create ND-range and add it to the list of ND-ranges
	auto it = ndranges.emplace(ndranges.end(), misc::new_unique<NDRange>());
	NDRange *ndrange = ndranges.back().get();

	// Save iterator to the position in the ND-range list
	ndrange->ndranges_iterator = it;

	// Debug info
	scheduler_debug << misc::fmt("NDRange %d added\n",
			ndrange->getId());

	// Return created ND-range
	return ndrange;
}


void Emulator::RemoveNDRange(NDRange *ndrange)
{
	//Debug info
	scheduler_debug << misc::fmt("NDRange %d removed\n",
			ndrange->getId());

	assert(ndrange->ndranges_iterator != ndranges.end());
	ndranges.erase(ndrange->ndranges_iterator);
}

NDRange *Emulator::getNDRangeById(unsigned id)
{
	// Iterate through NDRange list
	for (auto it = getNDRangesBegin(), e = getNDRangesEnd(); it != e; ++it)
	{
		// Get NDRange
		NDRange *ndrange = it->get();
			
		// Check ID
		if ((unsigned) ndrange->getId() == id)
			return ndrange;
	}

	// If no ndrange is found return null
	return nullptr;
}


} // SI namespace
