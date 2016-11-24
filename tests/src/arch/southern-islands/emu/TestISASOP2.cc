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

// This test checks the implementation of the S_ADD_U32 instruction.
// It checks for basic addition, proper implementation of the carry-bit, 
// and proper implementation of the literal constant
TEST(TestISASOP2, S_ADD_U32)
{
	// Initialize environment using the ObjectPool
	ObjectPool pool;
	WorkItem *work_item = pool.getWorkItem();
	Instruction *inst = pool.getInst();

	// Build instruction -  S_add_U32 s2, scc, s0, s1
	// inst_bytes fields are defined in southern-islands/asm/Inst.h
	Instruction::BytesSOP2 inst_bytes = 
	{
		// ssrc0 - s0
		0,

		// ssrc1 - s1
		1,

		// sdst - s2
		2,

		// op - S_add_U32 has opcode = 0                    
		0,

		// enc - 2
		2,

		// lit_cnst - 0
		0
	};
	inst->Decode((char *) &inst_bytes, 0);

	// Get registers
	int ssrc0 = inst->getBytes()->sop2.ssrc0;
	int ssrc1 = inst->getBytes()->sop2.ssrc1;
	int sdst = inst->getBytes()->sop2.sdst;
	int scc = Instruction::RegisterScc;




	//
	// Test basic addition
	//

	// Set values in registers - s0 = 1 and s1 = 2
	work_item->WriteSReg(ssrc0, 1);
	work_item->WriteSReg(ssrc1, 2);

	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - s2 should be 3 and scc should be 0
	EXPECT_EQ(3, work_item->ReadSReg(sdst)) << "add test: error with addition";
	EXPECT_EQ(0, work_item->ReadReg(scc)) << "add test: error with carry";




	//
	// Test carry bit
	//

	// Set values in registers - s0 and s1 = max 32 bit unsigned value
	work_item->WriteSReg(ssrc0, 0xffffffff);
	work_item->WriteSReg(ssrc1, 0xffffffff);

	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - s2 should be 4294967294 and scc should be 1
	EXPECT_EQ(4294967294, work_item->ReadSReg(sdst)) << "carry test: error with sum";
	EXPECT_EQ(1, work_item->ReadReg(scc)) << "carry test: error with carry";




	//
	// Test literal constant support
	//

	// Build new instruction with literal constant
	inst_bytes.ssrc0 = 0xff;
	inst_bytes.lit_cnst = 10;
	inst->Decode((char *) &inst_bytes, 0);

	// Set values in registers - s1 = 2, s0 is replaced with the literal
	work_item->WriteSReg(ssrc1, 2);
	
	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - s2 should be 12 and scc should be 0
	EXPECT_EQ(12, work_item->ReadSReg(sdst)) << "lit const: error with sum";
	EXPECT_EQ(0, work_item->ReadReg(scc)) << "lit const: error with carry";
}

// This test checks the implementation of the S_ADD_I32 instruction.
// It checks for basic addition, proper implementation of the overflow, 
// and proper implementation of the literal constant
TEST(TestISASOP2, S_ADD_I32)
{
	// Initialize environment using the ObjectPool
	ObjectPool pool;
	WorkItem *work_item = pool.getWorkItem();
	Instruction *inst = pool.getInst();

	
	// Build instruction -  S_add_I32 s2, scc, s0, s1
	// inst_bytes fields are defined in southern-islands/asm/Inst.h
	Instruction::BytesSOP2 inst_bytes = 
	{
		// ssrc0 - s0
		0,

		// ssrc1 - s1
		1,

		// sdst - s2
		2,

		// op - S_add_I32 has opcode = 2                    
		2,

		// enc - 2
		2,

		// lit_cnst - 1
		0
	};
	inst->Decode((char *) &inst_bytes, 0);

	// Get registers
	int ssrc0 = inst->getBytes()->sop2.ssrc0;
	int ssrc1 = inst->getBytes()->sop2.ssrc1;
	int sdst = inst->getBytes()->sop2.sdst;
	int scc = Instruction::RegisterScc;




	//
	// Test basic addition
	//

	// Set values in registers - s0 = 1 and s1 = 2
	work_item->WriteSReg(ssrc0, 1);
	work_item->WriteSReg(ssrc1, 2);

	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - s2 should be 3 and scc should be 0
	EXPECT_EQ(3, work_item->ReadSReg(sdst)) << "add test: sum error";
	EXPECT_EQ(0, work_item->ReadReg(scc)) << "add test: overflow error";




	//
	// Test overflow
	//

	// Set values in registers - s0 and s1 = max 32 bit unsigned value
	work_item->WriteSReg(ssrc0, 0xffffffff);
	work_item->WriteSReg(ssrc1, 0xffffffff);

	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - s2 should be 4294967294 and scc should be 0
	EXPECT_EQ(4294967294, work_item->ReadSReg(sdst)) << "overflow test: sum error";
	EXPECT_EQ(0, work_item->ReadReg(scc)) << "overflow test: overflow error";




	//
	// Test literal constant support
	//

	// Build new instruction with literal constant
	inst_bytes.ssrc0 = 0xff;
	inst_bytes.lit_cnst = 10;
	inst->Decode((char *) &inst_bytes, 0);

	// Set values in registers - s1 = 2, s0 is replaced with the literal
	work_item->WriteSReg(ssrc1, 2);
	
	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - s2 should be 12 and scc should be 0
	EXPECT_EQ(12, work_item->ReadSReg(sdst)) << "lit const: sum error";
	EXPECT_EQ(0, work_item->ReadReg(scc)) << "lit const: sum error";
}

// This test checks the implementation of the S_SUB_I32 instruction.
// It checks for basic subtraction, proper implementation of overflow, 
// and proper implementation of the literal constant
TEST(TestISASOP2, S_SUB_I32)
{
	// Initialize environment using the ObjectPool
	ObjectPool pool;
	WorkItem *work_item = pool.getWorkItem();
	Instruction *inst = pool.getInst();

	
	// Build instruction -  S_SUB_I32 s2, scc, s0, s1
	// inst_bytes fields are defined in southern-islands/asm/Inst.h
	Instruction::BytesSOP2 inst_bytes = 
	{
		// ssrc0 - s0
		0,

		// ssrc1 - s1
		1,

		// sdst - s2
		2,

		// op - S_SUB_I32 has opcode = 3                    
		3,

		// enc - 2
		2,

		// lit_cnst - 1
		0
	};
	inst->Decode((char *) &inst_bytes, 0);

	// Get registers
	int ssrc0 = inst->getBytes()->sop2.ssrc0;
	int ssrc1 = inst->getBytes()->sop2.ssrc1;
	int sdst = inst->getBytes()->sop2.sdst;
	int scc = Instruction::RegisterScc;




	//
	// Test basic subtraction
	//

	// Set values in registers - s0 = 4 and s1 = 2
	work_item->WriteSReg(ssrc0, 4);
	work_item->WriteSReg(ssrc1, 2);

	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - s2 should be 2 and scc should be 0
	EXPECT_EQ(2, work_item->ReadSReg(sdst));
	EXPECT_EQ(0, work_item->ReadReg(scc));




	//
	// Test overflow
	//

	// Set values in registers - s0 and s1 = max 32 bit signed value
	work_item->WriteSReg(ssrc0, 0x7fffffff);
	work_item->WriteSReg(ssrc1, 0xffffffff);

	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - s2 should be 0 and scc should be 0
	EXPECT_EQ(2147483648, work_item->ReadSReg(sdst)) << "overflow test: sum error";
	EXPECT_EQ(1, work_item->ReadReg(scc)) << "overflow test: overflow error";




	//
	// Test literal constant support
	//

	// Build new instruction with literal constant
	inst_bytes.ssrc0 = 0xff;
	inst_bytes.lit_cnst = 10;
	inst->Decode((char *) &inst_bytes, 0);

	// Set values in registers - s1 = 2, s0 is replaced with the literal
	work_item->WriteSReg(ssrc1, 2);
	
	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - s2 should be 8 and scc should be 0
	EXPECT_EQ(8, work_item->ReadSReg(sdst));
	EXPECT_EQ(0, work_item->ReadReg(scc));
}

// This test checks the implementation of the S_MIN_U32 instruction.
// It checks for basic less than operand, and proper implementation 
// of the literal constant
TEST(TestISASOP2, S_MIN_U32)
{
	// Initialize environment using the ObjectPool
	ObjectPool pool;
	WorkItem *work_item = pool.getWorkItem();
	Instruction *inst = pool.getInst();

	
	// Build instruction -  S_MIN_U32 s2, scc, s0, s1
	// inst_bytes fields are defined in southern-islands/asm/Inst.h
	Instruction::BytesSOP2 inst_bytes = 
	{
		// ssrc0 - s0
		0,

		// ssrc1 - s1
		1,

		// sdst - s2
		2,

		// op - S_MIN_U32 has opcode = 6                    
		7,

		// enc - 2
		2,

		// lit_cnst - 1
		0
	};
	inst->Decode((char *) &inst_bytes, 0);

	// Get registers
	int ssrc0 = inst->getBytes()->sop2.ssrc0;
	int ssrc1 = inst->getBytes()->sop2.ssrc1;
	int sdst = inst->getBytes()->sop2.sdst;
	int scc = Instruction::RegisterScc;




	//
	// Test basic less-than operand
	//

	// Set values in registers - s0 = 4 and s1 = 2
	work_item->WriteSReg(ssrc0, 4);
	work_item->WriteSReg(ssrc1, 2);

	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - s2 should be 2 and scc should be 0
	EXPECT_EQ(2, work_item->ReadSReg(sdst));
	EXPECT_EQ(0, work_item->ReadReg(scc));


	// Set values in registers - s0 and s1
	work_item->WriteSReg(ssrc0, 20);
	work_item->WriteSReg(ssrc1, 5000);

	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - s2 should be 20 and scc should be 1
	EXPECT_EQ(20, work_item->ReadSReg(sdst));
	EXPECT_EQ(1, work_item->ReadReg(scc));




	//
	// Test literal constant support
	//

	// Build new instruction with literal constant
	inst_bytes.ssrc0 = 0xff;
	inst_bytes.lit_cnst = 10;
	inst->Decode((char *) &inst_bytes, 0);

	// Set values in registers - s1 = 2, s0 is replaced with the literal
	work_item->WriteSReg(ssrc1, 2);
	
	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - s2 should be 2 and scc should be 0
	EXPECT_EQ(2, work_item->ReadSReg(sdst));
	EXPECT_EQ(0, work_item->ReadReg(scc));
}

// This test checks the implementation of the S_MAX_I32 instruction.
// It checks for basic greater than operand, and proper implementation 
// of the literal constant
TEST(TestISASOP2, S_MAX_I32)
{
	// Initialize environment using the ObjectPool
	ObjectPool pool;
	WorkItem *work_item = pool.getWorkItem();
	Instruction *inst = pool.getInst();

	
	// Build instruction -  S_MAX_I32 s2, scc, s0, s1
	// inst_bytes fields are defined in southern-islands/asm/Inst.h
	Instruction::BytesSOP2 inst_bytes = 
	{
		// ssrc0 - s0
		0,

		// ssrc1 - s1
		1,

		// sdst - s2
		2,

		// op - S_MAX_I32 has opcode = 8                    
		8,

		// enc - 2
		2,

		// lit_cnst - 1
		0
	};
	inst->Decode((char *) &inst_bytes, 0);

	// Get registers
	int ssrc0 = inst->getBytes()->sop2.ssrc0;
	int ssrc1 = inst->getBytes()->sop2.ssrc1;
	int sdst = inst->getBytes()->sop2.sdst;
	int scc = Instruction::RegisterScc;




	//
	// Test basic less-than operand
	//

	// Set values in registers - s0 = 4 and s1 = 2
	work_item->WriteSReg(ssrc0, 4);
	work_item->WriteSReg(ssrc1, 2);

	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - s2 should be 4 and scc should be 1
	EXPECT_EQ(4, work_item->ReadSReg(sdst));
	EXPECT_EQ(1, work_item->ReadReg(scc));


	// Set values in registers - s0 and s1
	work_item->WriteSReg(ssrc0, 20);
	work_item->WriteSReg(ssrc1, 5000);

	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - s2 should be 5000 and scc should be 0
	EXPECT_EQ(5000, work_item->ReadSReg(sdst));
	EXPECT_EQ(0, work_item->ReadReg(scc));




	//
	// Test literal constant support
	//

	// Build new instruction with literal constant
	inst_bytes.ssrc0 = 0xff;
	inst_bytes.lit_cnst = 10;
	inst->Decode((char *) &inst_bytes, 0);

	// Set values in registers - s1 = 2, s0 is replaced with the literal
	work_item->WriteSReg(ssrc1, 2);
	
	// Execute instruction
	work_item->Execute(inst->getOpcode(), inst);

	// Read results - s2 should be 10 and scc should be 1
	EXPECT_EQ(10, work_item->ReadSReg(sdst));
	EXPECT_EQ(1, work_item->ReadReg(scc));
}

} //SI

