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

#ifndef M2C_COMMON_PASS_H
#define M2C_COMMON_PASS_H


namespace comm
{

// Forward declarations.
class Module;
class Function;
class BasicBlock;
class Instruction;
class Argument;

/// Abstract class to be inherited from when building a compiler pass.
/// Helps encapsulate data for each pass, and allow passes to share their
/// data.
class Pass
{

	// Counter of passes, preincremented on allocation of a Pass,
	// and assigned to private member `id`.
	static int id_counter;

	// The id of the Pass instance. Used to access data from
	// comm::PassInfoPool.
	int id;

public:

	/// Constructor of a pass.
	Pass();

	// Virtual destructor to make class polymorphic.
	virtual ~Pass() { }

	/// Return the unique identifier for this pass (starting at 1).
	int getId() const { return id; }
/*
	/// Return true if there is a ModulePassInfo for the given Module
	/// for this pass.
	bool hasInfo(Module *module) { };

	/// Return true if there is a FunctionPassInfo for the given Function
	/// for this pass.
	bool hasInfo(Function *function) { };

	/// Return true if there is a BasicBlockPassInfo for the given
	/// BasicBlock for this pass.
	bool hasInfo(BasicBlock *basic_block) { };

	/// Return true if there is a InstructionPassInfo for the given
	/// Instruction for this pass.
	bool hasInfo(Instruction *instruction) { };

	/// Return true if there is a ArgumentPassInfo for the given Argument
	/// for this pass.
	bool hasInfo(Argument *argument) { };

	/// Return a pointer to the ModulePassInfo of the given Module
	/// for this pass.
	template<typename T> T *getInfo(Module *module) { };

	/// Return a pointer to the FunctionPassInfo of the given Function
	/// for this pass.
	template<typename T> T *getInfo(Function *function) { };

	/// Return a pointer to the BasicBlockPassInfo of the given BaiscBlock
	/// for this pass.
	template<typename T> T *getInfo(BasicBlock *basic_block) { };

	/// Return a pointer to the InstructionPassInfo of the given
	/// Instruction for this pass.
	template<typename T> T *getInfo(Instruction *instruction) { };

	/// Return a pointer to the ArgumentPassInfo of the given Argument
	/// for this pass.
	template<typename T> T *getInfo(Argument *argument) { };

	/// Resets the ModulePassInfo of the given Module.
	void resetInfo(Module *module, bool recursive = false) { };

	/// Resets the FunctionPassInfo of the given Function.
	void resetInfo(Function *function, bool recursive = false) { };

	/// Resets the BasicBlockPassInfo of the given BasicBlock.
	void resetInfo(BasicBlock *basic_block, bool recursive = false) { };

	/// Resets the InstructionPassInfo of the given Instruction.
	void resetInfo(Instruction *instruction, bool recursive = false) { };

	/// Resets the ArgumentPassInfo of the given Argument.
	void resetInfo(Argument *argument, bool recursive = false) { };

*/

	/// This function MUST be implemented in all subclasses of Pass,
	/// defining the logic of the pass.
	virtual void run() = 0;

};

}  // namespace comm

#endif
