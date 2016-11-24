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

#include "Emulator.h"
#include "NDRange.h"
#include "WorkGroup.h"
#include "Wavefront.h"
#include "WorkItem.h"


namespace SI
{

// Private constant declaring wavefront size
const unsigned WorkGroup::WavefrontSize = 64;


WorkGroup::WorkGroup(NDRange *ndrange, unsigned id)
{
	// Initialize
	this->id = id;
	this->ndrange = ndrange;
	
	// Initially, the work-group's position in the ND-Range's list of
	// work-groups is invalid.
	work_groups_iterator = ndrange->getWorkGroupsEnd();

	// Initialize local memory
	local_memory.setSafe(false);

	// Emulator instance
	Emulator *emulator = Emulator::getInstance();

	unsigned lid;
	unsigned lidx, lidy, lidz;
	unsigned tid;
	unsigned work_item_gidx_start;
	unsigned work_item_gidy_start;
	unsigned work_item_gidz_start;

	// Number of work-items in work-group 
	unsigned work_items_per_group = ndrange->getLocalSize(0) * 
		ndrange->getLocalSize(1) * ndrange->getLocalSize(2);
	assert(work_items_per_group > 0);

	// Number of wavefronts in work-group 
	unsigned wavefronts_per_group = (work_items_per_group + 
		(WorkGroup::WavefrontSize - 1)) / WorkGroup::WavefrontSize;
	assert(wavefronts_per_group > 0);

	// Allocate wavefronts and work-items
	for (unsigned i = 0; i < wavefronts_per_group; ++i)
	{
		// Create wavefront
		wavefronts.emplace_back(misc::new_unique<Wavefront>(
				this,
				id * wavefronts_per_group + i));

		// Create work-items
		for (unsigned j = 0; j < WorkGroup::WavefrontSize; ++j)
		{
			unsigned work_item_id = i * wavefronts_per_group + j;
			work_items.emplace_back(misc::new_unique<WorkItem>(
					wavefronts[i].get(),
					work_item_id));
			
			// Set work item properties
			WorkItem *work_item = work_items.back().get();
			work_item->setWorkGroup(this);
			work_item->setGlobalMemory(emulator->getGlobalMemory());
			work_item->setIdInWavefront(j);
		}
	}

	// Initialize work-group and work-item metadata 
	id_3d[0] = id % ndrange->getGroupCount(0);
	id_3d[1] = (id / ndrange->getGroupCount(0)) % 
		ndrange->getGroupCount(1);
	id_3d[2] = id / (ndrange->getGroupCount(0) * 
		ndrange->getGroupCount(1));

	// Number of work-items in work-group 
	this->work_item_count = ndrange->getLocalSize(2) *
		ndrange->getLocalSize(1) * ndrange->getLocalSize(0);

	// Global ID of work-item (0,0,0) within the work group 
	work_item_gidx_start = id_3d[0] * ndrange->getLocalSize(0);
	work_item_gidy_start = id_3d[1] * ndrange->getLocalSize(1);
	work_item_gidz_start = id_3d[2] * ndrange->getLocalSize(2);

	// For NDRange created for Pixel Shader, we need to initialize LDS 
	switch(ndrange->getStage())
	{

	case NDRange::StagePixelShader:
		// FIXME:Initialize LDS(Parameter Cache) 
		break;

	default:
		break;

	}

	// Initialize work-item metadata
	lid = 0;
	tid = id * work_items_per_group;
	WorkItem *work_item;
	Wavefront *wavefront;
	for (lidz = 0; lidz < ndrange->getLocalSize(2); lidz++)
	{
		for (lidy = 0; lidy < ndrange->getLocalSize(1); lidy++)
		{
			for (lidx = 0; lidx < ndrange->getLocalSize(0); lidx++)
			{
				work_item = work_items[lid].get();

				// Global IDs 
				work_item->setGlobalId3D(0,work_item_gidx_start + lidx);
				work_item->setGlobalId3D(1,work_item_gidy_start + lidy);
				work_item->setGlobalId3D(2,work_item_gidz_start + lidz);
				work_item->setGlobalId(tid);

				// Local IDs 
				work_item->setLocalId3D(0, lidx);
				work_item->setLocalId3D(1, lidy);
				work_item->setLocalId3D(2, lidz);
				work_item->setLocalId(lid);

				// First, last, and number of work-items 
				// in wavefront 
				wavefront = work_item->getWavefront();
				if (!wavefront->getWorkItemCount())
				{
					assert((this->work_items.begin() + lid) < 
							this->work_items.end());
					wavefront->setWorkItemsBegin(
							this->work_items.begin() 
							+ lid);
				}
				// Need a past-the-end iterator so add tid + 1
				assert(this->work_items.begin() + lid + 1 <=
						this->work_items.end());
				wavefront->setWorkItemsEnd(
						this->work_items.begin() 
						+ lid + 1);
				wavefront->incWorkItemCount();

				// Next work-item
				tid++;
				lid++;

				// Initialize the execution mask
				if (work_item->getIdInWavefront() < 32)
				{
					unsigned exec_mask = wavefront->getSregUint(Instruction::RegisterExec);
					exec_mask |= 1 << work_item->getIdInWavefront();
					wavefront->setSregUint(Instruction::RegisterExec, exec_mask);
				}
				else if (work_item->getIdInWavefront() < 64)
				{
					unsigned exec_mask = wavefront->getSregUint(Instruction::RegisterExec + 1);
					exec_mask |= 1 << (work_item->getIdInWavefront() - 32);
					wavefront->setSregUint(Instruction::RegisterExec + 1, exec_mask);
				}
				else 
				{
					throw Emulator::Error(misc::fmt("Invalid "
							"work-item ID (%d)",
							work_item->
							getIdInWavefront()));
				}
			}
		}
	}

	// Intialize wavefront state 
	for (auto wf_i = getWavefrontsBegin(), wf_e = getWavefrontsEnd();
			wf_i != wf_e;
			++wf_i)
	{
		wavefront = (*wf_i).get();

		// Set PC 
		wavefront->setPC(0);

		switch(ndrange->getStage())
		{

		// Save work-group IDs in scalar registers 
		case NDRange::StageCompute:

			wavefront->setSregUint(ndrange->getWorkgroupIdSreg(), 
				wavefront->getWorkGroup()->getId3D(0));
			wavefront->setSregUint(ndrange->getWorkgroupIdSreg() + 1, 
				wavefront->getWorkGroup()->getId3D(1));
			wavefront->setSregUint(ndrange->getWorkgroupIdSreg() + 2, 
				wavefront->getWorkGroup()->getId3D(2));
			break;


		// Currently hard coded as LDS is exclusive to each primitive.
		// Primitives should share LDS module and use different offset
		case NDRange::StagePixelShader:

			wavefront->setSregUint(ndrange->getWorkgroupIdSreg(), 0x0); 
			break;

		default:

			// Save work-group IDs in scalar registers 
			wavefront->setSregUint(ndrange->getWorkgroupIdSreg(), 
				wavefront->getWorkGroup()->getId3D(0));
			wavefront->setSregUint(ndrange->getWorkgroupIdSreg() + 1, 
				wavefront->getWorkGroup()->getId3D(1));
			wavefront->setSregUint(ndrange->getWorkgroupIdSreg() + 2, 
				wavefront->getWorkGroup()->getId3D(2));
			break;
		}

		for (auto wi_i = wavefront->getWorkItemsBegin(),
				wi_e = wavefront->getWorkItemsEnd();
				wi_i != wi_e;
				++wi_i)
		{
			// Store work-item IDs in vector registers 
			work_item = (*wi_i).get();

			switch (ndrange->getStage())
			{

			// OpenCL convention 
			case NDRange::StageCompute:
				// V0 
				work_item->WriteVReg(0, work_item->getLocalId3D(0));
				// V1 
				work_item->WriteVReg(1, work_item->getLocalId3D(1));
				// V2 
				work_item->WriteVReg(2, work_item->getLocalId3D(2));
				break;					

			// Vertex shader initialization convention 
			case NDRange::StageVertexShader:
				// VSes load VGPR0 with the thread's vertex index 
				work_item->WriteVReg(0, work_item->getId());
				break;

			// Pixel shader initialization convention 
			case NDRange::StagePixelShader:
				// FIXME
				break;

			// Default is OpenCL convention 
			default:
				// V0 
				work_item->WriteVReg(0, work_item->getLocalId3D(0));
				// V1 
				work_item->WriteVReg(1, work_item->getLocalId3D(1));
				// V2 
				work_item->WriteVReg(2, work_item->getLocalId3D(2));
				break;
			}
		}

		// Initialize sreg pointers to internal data structures 
		unsigned user_element_count = ndrange->getUserElementCount();
		for (unsigned i = 0; i < user_element_count; i++)
		{
			BinaryUserElement *user_element = ndrange->getUserElement(i);
			if (user_element->dataClass == BinaryUserDataConstBuffer)
			{
				// Store CB pointer in sregs
				wavefront->setSRegWithConstantBuffer(
					user_element->startUserReg,
					user_element->userRegCount,
					user_element->apiSlot);
			}
			else if (user_element->dataClass == BinaryUserDataUAV)
			{
				// Store UAV pointer in sregs
				wavefront->setSRegWithUAV(
					user_element->startUserReg,
					user_element->userRegCount,
					user_element->apiSlot);
			}
			else if (user_element->dataClass ==
				BinaryUserDataConstBufferTable)
			{
				// Store CB table in sregs
				wavefront->setSRegWithConstantBufferTable(
					user_element->startUserReg,
					user_element->userRegCount);
			}
			else if (user_element->dataClass == BinaryUserDataUAVTable)
			{
				// Store UAV table in sregs
				wavefront->setSRegWithUAVTable(
					user_element->startUserReg,
					user_element->userRegCount);
			}
			// FIXME: PTR_VERTEX_BUFFER_TABLE doesn't match binary
			else if (user_element->dataClass == 21)
			{
			}
			// FIXME: SUB_PTR_FETCH_SHADER doesn't match binary
			else if (user_element->dataClass == 16)
			{
			}
			else if (user_element->dataClass == BinaryUserDataSampler)
			{
				// Store sampler in sregs
				assert(0);
			}
			else if (user_element->dataClass ==
				BinaryUserDataPtrResourceTable)
			{
				// Store resource table in sregs
				assert(0);
			}
			else if (user_element->dataClass ==
				BinaryUserDataInternalGlobalTable)
			{
				throw misc::Panic("PTR_INTERNAL_GLOBAL_TABLE "
						"not supported");
			}
			else
			{
				throw misc::Panic(misc::fmt("Unimplemented "
						"user element: dataClass:%d",
						user_element->dataClass));
			}
		}
	}

	// Statistics 
	ndrange->getEmulator()->incWorkGroupCount();
}
	
}  // namespace SI
