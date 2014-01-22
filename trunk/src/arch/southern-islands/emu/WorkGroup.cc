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

#include "NDRange.h"
#include "WorkGroup.h"
#include "Wavefront.h"
#include "WorkItem.h"


using namespace misc;

namespace SI
{

WorkGroup::WorkGroup(NDRange *ndrange, unsigned id)
{
	this->id = id;
	this->ndrange = ndrange;

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
		(wavefront_size - 1)) / wavefront_size;
	assert(wavefronts_per_group > 0);

	// Allocate wavefronts and work-items
	for (unsigned i = 0; i < wavefronts_per_group; ++i)
	{
		this->wavefronts.push_back(std::unique_ptr<Wavefront>(new 
			Wavefront(this, i)));
		for (unsigned j = 0; j < wavefront_size; ++j)
		{
			unsigned work_item_id = i * wavefronts_per_group + j;
			this->work_items.push_back(std::unique_ptr<WorkItem>(new 
				WorkItem(wavefronts[i].get(), work_item_id)));
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

	case NDRangeStagePixelShader:
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
					wavefront->setWorkItemsBegin(this->work_items.begin() + lid);
				}
				wavefront->setWorkItemsEnd(this->work_items.begin() + tid);
				wavefront->incWorkItemCount();

				// Next work-item
				tid++;
				lid++;

				// Initialize the execution mask
				if (work_item->getIdInWavefront() < 32)
				{
					unsigned exec_mask = wavefront->getSregUint(SI_EXEC);
					exec_mask |= 1 << work_item->getIdInWavefront();
					wavefront->setSregUint(SI_EXEC, exec_mask);
				}
				else if (work_item->getIdInWavefront() < 64)
				{
					unsigned exec_mask = wavefront->getSregUint(SI_EXEC + 1);
					exec_mask |= 1 << work_item->getIdInWavefront();
					wavefront->setSregUint(SI_EXEC, exec_mask);
				}
				else 
				{
					fatal("%s: invalid work-item id (%d)",
						__FUNCTION__, 
						work_item->getIdInWavefront());
				}
			}
		}
	}

	// Intialize wavefront state 
	for(auto wf_i = WavefrontsBegin(),
			wf_e = WavefrontsEnd(); wf_i != wf_e; ++wf_i)
	{
		wavefront = (*wf_i).get();

		// Set PC 
		wavefront->setPC(0);

		switch(ndrange->getStage())
		{

		// Save work-group IDs in scalar registers 
		case NDRangeStageCompute:
			wavefront->setSregUint(ndrange->getWorkgroupIdSreg(), 
				wavefront->getWorkgroup()->getId3D(0));
			wavefront->setSregUint(ndrange->getWorkgroupIdSreg() + 1, 
				wavefront->getWorkgroup()->getId3D(1));
			wavefront->setSregUint(ndrange->getWorkgroupIdSreg() + 2, 
				wavefront->getWorkgroup()->getId3D(2));
			break;


		// Currently hard coded as LDS is exclusive to each primitive.
		// Primitives should share LDS module and use different offset
		case NDRangeStagePixelShader:
			wavefront->setSregUint(ndrange->getWorkgroupIdSreg(), 0x0); 
			break;

		default:
			// Save work-group IDs in scalar registers 
			wavefront->setSregUint(ndrange->getWorkgroupIdSreg(), 
				wavefront->getWorkgroup()->getId3D(0));
			wavefront->setSregUint(ndrange->getWorkgroupIdSreg() + 1, 
				wavefront->getWorkgroup()->getId3D(1));
			wavefront->setSregUint(ndrange->getWorkgroupIdSreg() + 2, 
				wavefront->getWorkgroup()->getId3D(2));
			break;
		}

		for (auto wi_i = wavefront->WorkItemsBegin(),
				wi_e = wavefront->WorkItemsEnd(); wi_i != wi_e; ++wi_i)
		{
			// Store work-item IDs in vector registers 
			work_item = (*wi_i).get();

			switch(ndrange->getStage())
			{

			// OpenCL convention 
			case NDRangeStageCompute:
				// V0 
				work_item->WriteVReg(0, work_item->getLocalId3D(0));
				// V1 
				work_item->WriteVReg(1, work_item->getLocalId3D(1));
				// V2 
				work_item->WriteVReg(2, work_item->getLocalId3D(2));
				break;					

			// Vertex shader initialization convention 
			case NDRangeStageVertexShader:
				// VSes load VGPR0 with the thread's vertex index 
				work_item->WriteVReg(0, work_item->getId());
				break;

			// Pixel shader initialization convention 
			case NDRangeStagePixelShader:
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
				// Store VB table in sregs
				wavefront->setSRegWithVertexBufferTable(
					user_element->startUserReg,
					user_element->userRegCount);
			}
			// FIXME: SUB_PTR_FETCH_SHADER doesn't match binary
			else if (user_element->dataClass == 16)
			{
				// Store Fetch Shader pointer in sregs
				wavefront->setSRegWithFetchShader(
					user_element->startUserReg,
					user_element->userRegCount);
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
				fatal("%s: PTR_INTERNAL_GLOBAL_TABLE not "
					"supported", __FUNCTION__);
			}
			else
			{
				fatal("%s: Unimplemented User Element: "
					"dataClass:%d", __FUNCTION__,
					user_element->dataClass);
			}
		}
	}

	// Statistics 
	ndrange->getEmu()->incWorkGroupCount();
}
	
}  // namespace SI
