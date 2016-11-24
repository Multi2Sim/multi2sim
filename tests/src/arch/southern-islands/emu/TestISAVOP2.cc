/*
 *  Multi2Sim
 *  Copyright (C) 2015  Rafael Ubal (ubal@ece.neu.edu)
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

#include <gtest/gtest.h>

#include "ObjectPool.h"


namespace SI
{

// This test checks the implementation of the V_ADD_I32 instruction.
// It checks for basic addition, proper implementation of the carry-bit, 
// and proper implementation of the literal constant
TEST(TestISAVOP2, V_ADD_I32)
{
	// Initialize environment using the ObjectPool
	ObjectPool pool;
	WorkItem *work_item = pool.getWorkItem();
	Instruction *inst = pool.getInst();

	
	// Build instruction -  v_add_i32 v2, vcc, v0, v1
	// inst_bytes fields are defined in southern-islands/asm/Inst.h
	Instruction::BytesVOP2 inst_bytes = 
	{
		// src0 - v0
		256,

		// vsrc1 - v1
		1,

		// vdst - v2
		2,

		// op - v_add_i32 has opcode = 37                    
		37,

		// enc - 0
		0,

		// lit_cnst - 1
		0
	};
	inst->Decode((char *) &inst_bytes, 0);

	// Get registers
	int src0 = inst->getBytes()->vop2.src0;
	int vsrc1 = inst->getBytes()->vop2.vsrc1;
	int vdst = inst->getBytes()->vop2.vdst;
	int vcc = Instruction::RegisterVcc;
	



	//
	// Test basic addition
	//

	// Set values in registers - v0 = 1 and v1 = 2
	work_item->WriteVReg(src0 - 256, 1);
	work_item->WriteVReg(vsrc1, 2);

	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - v2 should be 3 and vcc should be 0
	EXPECT_EQ(3, work_item->ReadVReg(vdst));
	EXPECT_EQ(0, work_item->ReadReg(vcc));




	//
	// Test carry bit
	//

	// Set values in registers - v0 and v1 = max 32 bit unsigned value
	work_item->WriteVReg(src0 - 256, 0xffffffff);
	work_item->WriteVReg(vsrc1, 0xffffffff);

	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - v2 should be 3 and vcc should be 0
	EXPECT_EQ(4294967294, work_item->ReadVReg(vdst));
	EXPECT_EQ(1, work_item->ReadReg(vcc));




	//
	// Test literal constant support
	//

	// Build new instruction with literal constant
	inst_bytes.src0 = 0xff;
	inst_bytes.lit_cnst = 10;
	inst->Decode((char *) &inst_bytes, 0);

	// Set values in registers - v1 = 2, v0 is replaced with the literal
	work_item->WriteVReg(vsrc1, 2);
	
	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - v2 should be 12 and vcc should be 0
	EXPECT_EQ(12, work_item->ReadVReg(vdst));
	EXPECT_EQ(0, work_item->ReadReg(vcc));
}


}




