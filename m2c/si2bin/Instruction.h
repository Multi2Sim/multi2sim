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


class Instruction
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

	// Common construction
	void Initialize();

	// Construction based on opcode + argument list
	void Initialize(SI::InstOpcode opcode);
	template<typename... Args> void Initialize(SI::InstOpcode opcode,
			Argument *arg, Args&&... args)
	{
		this->arguments.emplace_back(arg);
		Initialize(opcode, args...);
	}
	
	// Construction based on name + argument list
	void Initialize(const std::string &name);
	template<typename... Args> void Initialize(const std::string &name,
			Argument *arg, Args&&... args)
	{
		this->arguments.emplace_back(arg);
		Initialize(name, args...);
	}

	void EncodeArg(Argument *arg, Token *token);

	// Add an argument
	template<typename T, typename... Args> T *addArgument(Args&&... args)
	{
		arguments.emplace_back(misc::new_unique<T>(args...));
		T *argument = misc::cast<T *>(arguments.back().get());

		// FIXME Instead, the Argument object should take an initial
		// Instruction argument, from which it will deduce its index.
		argument->setIndex(arguments.size() - 1);

		return argument;
	}

public:
	
	/// Create a new instruction with the specified opcode, as defined in
	/// the Southern Islands disassembler. The arguments contained in the
	/// list will be freed automatically in the destructor of this class.
	template<typename... Args> Instruction(SI::InstOpcode opcode,
			Args&&... args)
	{
		Initialize(opcode, args...);
	}
	
	/// Create a new instruction with one of the possible opcodes
	/// corresponding to a name. The arguments contained in the list will be
	/// adopted by the instruction and freed in the destructor.
	template<typename... Args> Instruction(const std::string &name,
			Args&&... args)
	{
		Initialize(name, args...);
	}

	// Construction based on opcode + argument list as vector
	Instruction(SI::InstOpcode opcode, std::vector<Argument *> &arg_list)
	{
		for (auto &arg : arg_list)
		{
			arguments.emplace_back(arg);
		}
		Initialize(opcode);
	}

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
	Instruction(const std::string &name, std::vector<Argument *> &arg_list)
	{
		for (auto &arg : arg_list)
		{
			arguments.emplace_back(arg);
		}
		Initialize(name);
	}

	/// Add an argument to the list of arguments. The instruction will take
	/// ownership of this argument.
	/// FIXME - This function is temporary and should be removed once all
	/// memory allocation is made with symmetric patterns and smart pointers.
	void addArgument(Argument *argument)
	{
		arguments.emplace_back(argument);
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
	ArgWaitCounter *addWaitCounter(ArgWaitCounter::CounterType type
			= ArgWaitCounter::CounterTypeInvalid)
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

	/// Check that the number and type of the arguments added to the
	/// instruction with previous calls to addVectorRegister,
	/// addScalarRegister, ... are valid. An exception is thrown if any
	/// error is detected.
	void VerifyArguments();

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
