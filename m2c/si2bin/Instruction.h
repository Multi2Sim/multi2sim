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

#ifndef M2C_SI2BIN_INSTRUCTION_H
#define M2C_SI2BIN_INSTRUCTION_H

#include <iostream>
#include <list>
#include <memory>
#include <vector>

#include <arch/southern-islands/asm/Inst.h>
#include <src/lib/cpp/Bitmap.h>
#include <src/lib/cpp/Misc.h>
#include <m2c/common/Instruction.h>

#include "Argument.h"
#include "Context.h"
#include "Token.h"


// Forward declarations
namespace llvm2si { class BasicBlock; }


namespace si2bin
{

// Forward declarations
class InstInfo;
class Context;


class Instruction : public comm::Instruction
{
	// For LLVM-to-SI back-end: basic block that the instruction
	// belongs to.
	llvm2si::BasicBlock *basic_block = nullptr;

	// Instruction opcode. This field should match the content of
	// info->info->opcode.
	SI::InstOpcode opcode = SI::InstOpcodeInvalid;

	// Instruction size in bytes (4 or 8). This value is produced after a
	// call to Inst::Encode()
	int size = 0;

	// Instruction bytes. This value is produced after a call to
	// Inst::Encode().
	SI::InstBytes bytes;

	// Invariable information related with this instruction
	InstInfo *info;

	// List of arguments
	std::vector<std::unique_ptr<Argument>> arguments;

	// Comment attached to the instruction, which will be dumped together
	// with it.
	std::string comment;

	// This flag is set if the instruction has been emitted during the
	// control flow generation pass, for active mask manipulation purposes.
	bool control_flow = false;

	// Obtain back-end context in a field for efficiency
	Context *context;

	// Function used in the constructor of an instruction by its name,
	// inferring the opcode from the name and the type of the arguments.
	void InferOpcodeFromName(const std::string &name);

	// Populate the instruction bits for the given argument
	void EncodeArgument(Argument *argument, Token *token);

	// Add an argument. This function is invoked by public functions with
	// the same prefix but specific argument types (e.g., addVectorRegister)
	template<typename T, typename... Args> T *addArgument(Args&&... args)
	{
		arguments.emplace_back(misc::new_unique<T>(args...));
		T *argument = misc::cast<T *>(arguments.back().get());
		argument->setInstruction(this);
		argument->setIndex(arguments.size() - 1);
		return argument;
	}

public:
	
	/// Create an instruction with the given \a opcode.
	///
	/// \param basic_block
	///	Basic block that the instruction belongs to.
	///
	/// \param opcode
	///	Instruction opcode.
	Instruction(llvm2si::BasicBlock *basic_block, SI::InstOpcode opcode);
	
	/// Construction based on an instruction opcode and a list of arguments.
	/// The argument in the list are given as newly allocated object that
	/// the instruction will take ownership from.
	Instruction(const std::string &name, std::vector<Argument *> &arg_list);

	/// Add an argument to the list of arguments. The instruction will take
	/// ownership of this argument.
	void addArgument(std::unique_ptr<Argument> &&argument)
	{
		argument->setInstruction(this);
		argument->setIndex(arguments.size());
		arguments.emplace_back(std::move(argument));
	}

	/// Add a scalar register argument.
	ArgScalarRegister *addScalarRegister(int id)
	{
		return addArgument<ArgScalarRegister>(id);
	}

	/// Add a scalar register series argument.
	ArgScalarRegisterSeries *addScalarRegisterSeries(int low, int high)
	{
		return addArgument<ArgScalarRegisterSeries>(low, high);
	}

	/// Add a vector register argument.
	ArgVectorRegister *addVectorRegister(int id)
	{
		return addArgument<ArgVectorRegister>(id);
	}

	/// Add a vector register series argument.
	ArgVectorRegisterSeries *addVectorRegisterSeries(int low, int high)
	{
		return addArgument<ArgVectorRegisterSeries>(low, high);
	}

	/// Add a literal argument.
	ArgLiteral *addLiteral(int value)
	{
		return addArgument<ArgLiteral>(value);
	}

	/// Add a floating-point literal argument.
	ArgLiteralFloat *addLiteralFloat(float value)
	{
		return addArgument<ArgLiteralFloat>(value);
	}

	/// Add a wait counter argument
	ArgWaitCounter *addWaitCounter(ArgWaitCounter::CounterType type)
	{
		return addArgument<ArgWaitCounter>(type);
	}

	/// Add a memory register argument
	ArgMemRegister *addMemRegister(int id)
	{
		return addArgument<ArgMemRegister>(id);
	}

	/// Add a memory address argument
	ArgMaddr *addMemoryAddress(Argument *soffset,
			ArgMaddrQual *qual,
			SI::InstBufDataFormat data_format,
			SI::InstBufNumFormat num_format)
	{
		return addArgument<ArgMaddr>(soffset, qual, data_format,
				num_format);
	}

	/// Add a special register argument
	ArgSpecialRegister *addSpecialRegister(SI::InstSpecialReg reg)
	{
		return addArgument<ArgSpecialRegister>(reg);
	}

	/// Add a label argument
	ArgLabel *addLabel(const std::string &name)
	{
		return addArgument<ArgLabel>(name);
	}

	/// Add a phi argument
	ArgPhi *addPhi(const std::string &label_name)
	{
		return addArgument<ArgPhi>(label_name);
	}

	/// Return whether the number and type of the arguments added to the
	/// instruction with previous calls to addVectorRegister,
	/// addScalarRegister, ... are valid.
	bool hasValidArguments();

	/// Dump instruction in plain text
	void Dump(std::ostream &os);

	/// Alternative syntax for Dump()
	friend std::ostream &operator<<(std::ostream &os,
			Instruction &instruction)
	{
		instruction.Dump(os);
		return os;
	}

	/// Return a constant reference to the instruction arguments
	const std::vector<std::unique_ptr<Argument>> &getArguments()
	{
		return arguments;
	}

	/// Return the number of arguments
	int getNumArguments() const { return arguments.size(); }

	/// Return the argument with the given index. The index must be a valid
	/// number between 0 and the number of arguments minux 1.
	Argument *getArgument(int index)
	{
		assert(misc::inRange(index, 0, (int) arguments.size() - 1));
		return arguments[index].get();
	}

	/// Return the basic block that the instruction belongs to
	llvm2si::BasicBlock *getBasicBlock() { return basic_block; }

	/// Return the instruction opcode
	SI::InstOpcode getOpcode() const { return opcode; }

	/// Associate the instruction to a basic block
	void setBasicBlock(llvm2si::BasicBlock *basic_block)
	{
		this->basic_block = basic_block;
	}

	/// Attach a comment to the instruction
	void setComment(const std::string &comment) { this->comment = comment; }

	/// Encode the instruction internally. After this, a call to Write()
	/// can be made to dump the instruction bytes into a file.
	void Encode();

	/// Write the instruction bytes into output stream.
	void Write(std::ostream &os)
	{
		os.write((char *) bytes.byte, size);
	};

	/// Label this instruction as an instruction emitted by the control
	/// flow pass.
	void setControlFlow(bool control_flow)
	{
		this->control_flow = control_flow;
	}

	/// Return whether the instruction was emitted by the control flow pass
	bool getControlFlow() const { return control_flow; }
};


}  // namespace si2bin

#endif
