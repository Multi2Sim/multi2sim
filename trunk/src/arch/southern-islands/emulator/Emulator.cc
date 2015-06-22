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

#include <arch/southern-islands/asm/Asm.h>
#include <arch/southern-islands/emulator/WorkGroup.h>
#include <arch/southern-islands/emulator/Wavefront.h>
#include <arch/southern-islands/emulator/WorkItem.h>
#include <lib/cpp/ELFReader.h>
#include <lib/cpp/Misc.h>

#include "Emulator.h"
#include "NDRange.h"


namespace SI
{

// UAV Table
const unsigned Emulator::MaxNumUAVs;
const unsigned Emulator::UAVTableEntrySize = 32;
const unsigned Emulator::UAVTableSize = MaxNumUAVs * UAVTableEntrySize;

/// Vertex buffer table
const unsigned Emulator::MaxNumVertexBuffers;
const unsigned Emulator::VertexBufferTableEntrySize = 32;
const unsigned Emulator::VertexBufferTableSize = 
	Emulator::MaxNumVertexBuffers * Emulator::VertexBufferTableEntrySize;

/// Constant buffer table
const unsigned Emulator::MaxNumConstBufs;
const unsigned Emulator::ConstBufTableEntrySize = 16;
const unsigned Emulator::ConstBufTableSize = Emulator::MaxNumConstBufs * 
		Emulator::ConstBufTableEntrySize;

/// Resource table
const unsigned Emulator::MaxNumResources;
const unsigned Emulator::ResourceTableEntrySize = 32;
const unsigned Emulator::ResourceTableSize = Emulator::MaxNumResources * 
		Emulator::ResourceTableEntrySize;

const unsigned Emulator::TotalTableSize = Emulator::UAVTableSize + 
		Emulator::ConstBufTableSize + Emulator::ResourceTableSize + 
		Emulator::VertexBufferTableSize;

/// Constant buffers
const unsigned Emulator::ConstBuf0Size = 160;  // Defined in Metadata.pdf
const unsigned Emulator::ConstBuf1Size = 1024; // FIXME
	
const unsigned Emulator::TotalConstBufSize = Emulator::ConstBuf0Size + Emulator::ConstBuf1Size;

// Singleton
std::unique_ptr<Emulator> Emulator::instance;

// Debugger
misc::Debug Emulator::debug;

Emulator *Emulator::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new Emulator());
	return instance.get();	
}

Emulator::Emulator()
{
	// Disassemler
	disassembler = Disassembler::getInstance();
	
	// GPU memories
	video_memory = misc::new_unique<mem::Memory>();
	video_memory->setSafe(true);

	shared_memory = misc::new_unique<mem::Memory>();
	global_memory = video_memory.get();
}

void Emulator::Dump(std::ostream &os) const
{
	// FIXME: basic statistics, such as instructions, time...

	// More statistics 
	os << "NDRangeCount = " << ndrange_count << std::endl;
	os << "WorkGroupCount = " << work_group_count << std::endl;
	os << "BranchInstructions = " << branch_inst_count << std::endl;
	os << "LDSInstructions = " << lds_inst_count << std::endl;
	os << "ScalarALUInstructions = " << scalar_alu_inst_count << std::endl;
	os << "ScalarMemInstructions = " << scalar_mem_inst_count << std::endl;
	os << "VectorALUInstructions = " << vector_alu_inst_count << std::endl;
	os << "VectorMemInstructions = " << vector_mem_inst_count << std::endl;
}

void Emulator::Run()
{
#if 0
	// For efficiency when no Southern Islands emulation is selected, 
	// exit here if the list of existing ND-Ranges is empty. 
	if (opencl_driver->isNDRangeListEmpty())
		return;

	// NDRange list is shared by CL/GL driver
	for (auto ndr_i = opencl_driver->getNDRangeBegin(); 
		ndr_i < opencl_driver->getNDRangeEnd(); ++ndr_i)
	{
		// Move waiting work groups to running work groups 
		(*ndr_i)->WaitingToRunning();

		// If there's no work groups to run, go to next nd-range 
		if ((*ndr_i)->isRunningWorkGroupsEmpty())
			continue;

		// Iterate over running work groups
		for (auto wg_i = (*ndr_i)->RunningWorkGroupBegin(), 
			wg_e = (*ndr_i)->RunningWorkGroupEnd(); wg_i != wg_e; ++wg_i)
		{
			std::unique_ptr<WorkGroup> workgroup(new WorkGroup((*ndr_i).get(), (*wg_i)));

			for (auto wf_i = workgroup->WavefrontsBegin(), 
				wf_e = workgroup->WavefrontsEnd(); wf_i != wf_e; ++wf_i)
				(*wf_i)->Execute();

			workgroup.reset();
		}

		// Let OpenCL driver know that all work-groups from this nd-range
		// have been run
		// opencl_driver->RequestWork((*ndr_i).get());
	}
#endif
}


/// Initialize a buffer description of type EmuBufferDesc
void Emulator::createBufferDesc(unsigned base_addr, unsigned size,
		int num_elems, ArgDataType data_type, 
		EmuBufferDesc *buffer_desc)
{
	int num_format;                                                          
	int data_format;                                                         
	int elem_size;                                                           

	// Check size of buffer descriptor
	assert(sizeof(EmuBufferDesc) == 16);                           

	// Initialize num_format and data_format
	num_format = BufDescNumFmtInvalid;
	data_format = BufDescDataFmtInvalid;                              

	// Set num_format and data_format
	switch (data_type)                                                       
	{                                                                        
	case ArgDataTypeInt8:                                                          
	case ArgDataTypeUInt8:                                                         

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

	case ArgDataTypeInt16:                                                         
	case ArgDataTypeUInt16:                                                        

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

	case ArgDataTypeInt32:                                                         
	case ArgDataTypeUInt32:                                                        

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

	case ArgDataTypeFloat:                                                         

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

	case ArgDataTypeDouble:                                                        

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

	case ArgDataTypeStruct:                                                        

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
	buffer_desc->base_addr = base_addr;                                      
	buffer_desc->num_format = num_format;                                    
	buffer_desc->data_format = data_format;                                  
	assert(!(size % elem_size));                                             
	buffer_desc->elem_size = elem_size;                                      
	buffer_desc->num_records = size/elem_size;                               

	// Return
	return;   
}


}  // namespace SI
