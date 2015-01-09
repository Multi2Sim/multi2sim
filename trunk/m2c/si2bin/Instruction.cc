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

#include <lib/cpp/Misc.h>

#include "Argument.h"
#include "Context.h"
#include "Instruction.h"
#include "Token.h"


namespace si2bin
{

Instruction::Instruction(llvm2si::BasicBlock *basic_block,
		SI::InstOpcode opcode) :
		basic_block(basic_block),
		opcode(opcode)
{
	// Initialize
	context = Context::getInstance();
	bytes.dword = 0;

	// Get instruction information
	info = context->getInstInfo(opcode);
	if (!info)
		throw misc::Panic(misc::fmt("Opcode not supported (%d)",
				opcode));
}


Instruction::Instruction(const std::string &name, std::vector<Argument *>
		&arguments)
{
	// Initialize
	context = Context::getInstance();
	bytes.dword = 0;

	// Create arguments
	// FIXME: transfer vector instead
	// FIXME: input should be vector of unique pointers
	for (auto &arg : arguments)
		this->arguments.emplace_back(arg);

	// Assign argument indices and belonging instruction
	for (unsigned i = 0; i < this->arguments.size(); i++)
	{
		this->arguments[i]->setInstruction(this);
		this->arguments[i]->setIndex(i);
	}

	// Get the instruction opcode
	InferOpcodeFromName(name);
}


void Instruction::InferOpcodeFromName(const std::string &name)
{
	// Try to create the instruction following all possible encodings for
	// the same instruction name.
	std::string error = "invalid instruction: " + name;
	for (info = context->getInstInfo(name); info; info = info->getNext())
	{
		// Check number of arguments
		if (arguments.size() != info->getNumTokens())
		{
			error = misc::fmt("Invalid number of arguments for %s "
					"(%d given, %d expected)",
					name.c_str(), (int) arguments.size(),
					(int) info->getNumTokens());
			continue;
		}

		// Check arguments
		error = "";
		for (unsigned i = 0; i < arguments.size(); i++)
		{
			// Get formal argument from instruction info. We
			// associate the instruction argument with the token.
			Argument *argument = arguments[i].get();
			Token *token = info->getToken(i);
			argument->setToken(token);
			assert(token);

			// Check that actual argument type is acceptable for
			// token
			if (!token->isArgumentAllowed(argument))
			{
				error = misc::fmt("Invalid type for argument "
						"%d", argument->getIndex() + 1);
				break;
			}
		}

		// Error while processing arguments
		if (!error.empty())
			continue;
	
		// All checks passed, instruction identified correctly as that
		// represented by 'info'.
		break;
	}

	// Error identifying instruction
	if (!info)
		throw Error(error);

	// Initialize opcode
	opcode = info->getOpcode();
}


bool Instruction::hasValidArguments()
{
	// The Phi instruction is a special case
	if (opcode == SI::INST_PHI)
	{
		// At least 3 arguments
		if (arguments.size() < 3)
			return false;

		// First argument must be a vector, scalar, vector series, or
		// scalar series.
		if (!arguments[0]->hasValidType(Argument::TypeVectorRegister,
				Argument::TypeVectorRegisterSeries,
				Argument::TypeScalarRegister,
				Argument::TypeScalarRegisterSeries))
			return false;
		
		// The rest of the argument must be Phi arguments
		for (unsigned i = 1; i < arguments.size(); i++)
			if (arguments[i]->getType() != Argument::TypePhi)
				return false;

		// All checks passed for Phi instruction
		return true;
	}

	// Check number of arguments
	if (arguments.size() != info->getNumTokens())
		return false;

	// Check argument types
	for (unsigned i = 0; i < arguments.size(); i++)
	{
		// Get formal argument from instruction info. Associate token
		// with the instruction argument.
		Token *token = info->getToken(i);
		Argument *argument = arguments[i].get();
		argument->setToken(token);
		assert(token);

		// Check that actual argument type is acceptable for token
		if (!token->isArgumentAllowed(argument))
			return false;
	}

	// All checks passed
	return true;
}


void Instruction::Dump(std::ostream &os)
{
        // Comment attached to the instruction
        if (!comment.empty())
        	os << "\n\t# " << comment << '\n';

        // Dump instruction opcode
        os << '\t' << info->getName() << ' ';

        // Dump arguments
        for (auto &arg : arguments)
        {
        	arg->Dump(os);
        	if (arg->getIndex() < (int) arguments.size() - 1)
                	os << ", ";
        }

        // End
        os << '\n';
}


void Instruction::EncodeArgument(Argument *argument, Token *token)
{
	// Check token
	switch (token->getType())
	{

	case TokenSimm16:
	{
		int value;

		ArgLiteral *literal = dynamic_cast<ArgLiteral *>(argument);
		if (literal)
		{
			// Literal constant other than [-16...64]
			if (literal->getValue() > 0xff00)
				misc::fatal("%s: Literal in simm16 needs to fit in 16 bit field",
					__FUNCTION__);
			bytes.sopk.simm16 = literal->getValue();
		}
		else
		{
			// Encode
			value = argument->Encode();
			if (!misc::inRange(value, 0, 255))
				misc::fatal("invalid argument type");
			bytes.sopk.simm16 = value;
		}
		break;
	}
	
	case Token64Sdst:
	{
		// Check range if scalar register range given
		ArgScalarRegisterSeries *srs = dynamic_cast
				<ArgScalarRegisterSeries *>(argument);
		if (srs && srs->getHigh() != srs->getLow() + 1)
			misc::fatal("register series must be s[x:x+1]");
		
		// Encode
		bytes.sop2.sdst = argument->Encode();
		break;
	}

	case Token64Ssrc0:
	{
		ArgLiteral *literal = dynamic_cast<ArgLiteral *>(argument);
		if (literal)
		{
			/* Literal constant other than [-16...64] is encoded by adding
			 * four more bits to the instruction. */
			if (size == 8)
				misc::fatal("only one literal allowed");
			size = 8;
			bytes.sop2.ssrc0 = 0xff;
			bytes.sop2.lit_cnst = literal->getValue();
		}
		else
		{
			// Check range of scalar registers
			ArgScalarRegisterSeries *srs = dynamic_cast
					<ArgScalarRegisterSeries *>(argument);
			if (srs && srs->getHigh() != srs->getLow() + 1)
				misc::fatal("invalid scalar register series, s[x:x+1] expected");

			// Encode
			int value = argument->Encode();
			if (!misc::inRange(value, 0, 255))
				throw Error("Invalid argument type");
			bytes.sop2.ssrc0 = value;
		}
		break;
	}

	case Token64Ssrc1:
	{
		ArgLiteral *literal = dynamic_cast<ArgLiteral *>(argument);
		if (literal)
		{
			/* Literal constant other than [-16...64] is encoded by adding
			 * four more bits to the instruction. */
			if (size == 8)
				misc::fatal("only one literal allowed");
			size = 8;
			bytes.sop2.ssrc1 = 0xff;
			bytes.sop2.lit_cnst = literal->getValue();
		}
		else
		{
			// Check range of scalar registers
			ArgScalarRegisterSeries *srs = dynamic_cast
					<ArgScalarRegisterSeries *>(argument);
			if (srs && srs->getHigh() != srs->getLow() + 1)
				misc::fatal("invalid scalar register series, s[x:x+1] expected");

			// Encode
			int value = argument->Encode();
			if (!misc::inRange(value, 0, 255))
				misc::fatal("invalid argument type");
			bytes.sop2.ssrc1 = value;
		}
		break;
	}
	
	case TokenMtMaddr:
	{
		// Get argument
		ArgMaddr *maddr = dynamic_cast<ArgMaddr *>(argument);
		assert(maddr);

		// Offset
		int soffset = maddr->getSoffset()->Encode();
		if (!misc::inRange(soffset, 0, 253))
			misc::fatal("invalid offset");
		bytes.mtbuf.soffset = soffset;

		// Data and number format
		bytes.mtbuf.dfmt = maddr->getDataFormat();
		bytes.mtbuf.nfmt = maddr->getNumFormat();

		// Qualifiers
		ArgMaddrQual *qual = maddr->getQual();
		bytes.mtbuf.offen = qual->getOffen();
		bytes.mtbuf.idxen = qual->getIdxen();
		bytes.mtbuf.offset = qual->getOffset();

		break;
	}

	case TokenMtSeriesVdataSrc:
	{
		int low = 0;
		int high = 0;
		int high_must = 0;

		// Get registers
		switch (argument->getType())
		{

		case Argument::TypeVectorRegister:
		{
			ArgVectorRegister *vr = dynamic_cast
					<ArgVectorRegister *>(argument);
			assert(vr);
			low = vr->getId();
			high = low;
			break;
		}

		case Argument::TypeVectorRegisterSeries:
		{
			ArgVectorRegisterSeries *vrs = dynamic_cast
					<ArgVectorRegisterSeries *>(argument);
			low = vrs->getLow();
			high = vrs->getHigh();
			break;
		}

		default:
			misc::panic("%s: invalid argument for TokenMtSeriesVdata",
					__FUNCTION__);
		}

		// Restriction in vector register range
		switch (opcode)
		{

		case SI::INST_TBUFFER_LOAD_FORMAT_X:
		case SI::INST_TBUFFER_STORE_FORMAT_X:

			high_must = low;
			break;

		case SI::INST_TBUFFER_LOAD_FORMAT_XY:
		case SI::INST_TBUFFER_STORE_FORMAT_XY:

			high_must = low + 1;
			break;

		case SI::INST_TBUFFER_LOAD_FORMAT_XYZW:
		case SI::INST_TBUFFER_STORE_FORMAT_XYZW:

			high_must = low + 3;
			break;

		default:
			misc::panic("%s: MUBUF/MTBUF instruction not recognized: %s",
					__FUNCTION__, info->getName().c_str());
		}

		// Check range
		if (high != high_must)
			misc::fatal("invalid register series: v[%d:%d]", low, high);

		// Encode
		bytes.mtbuf.vdata = low;
		break;
	}
	

	case TokenMtSeriesVdataDst:
	{
		int low = 0;
		int high = 0;
		int high_must = 0;

		// Get registers
		switch (argument->getType())
		{

		case Argument::TypeVectorRegister:
		{
			ArgVectorRegister *vr = dynamic_cast
					<ArgVectorRegister *>(argument);
			assert(vr);
			low = vr->getId();

			//One reg so high is same as low
			high = low;
			break;
		}

		case Argument::TypeVectorRegisterSeries:
		{
			ArgVectorRegisterSeries *vrs = dynamic_cast
					<ArgVectorRegisterSeries *>(argument);
			low = vrs->getLow();
			high = vrs->getHigh();
			break;
		}

		default:
			misc::panic("%s: invalid argument for TokenMtSeriesVdata",
					__FUNCTION__);
		}

		// Restriction in vector register range
		switch (opcode)
		{

		case SI::INST_TBUFFER_LOAD_FORMAT_X:
		case SI::INST_TBUFFER_STORE_FORMAT_X:

			high_must = low;
			break;

		case SI::INST_TBUFFER_LOAD_FORMAT_XY:
		case SI::INST_TBUFFER_STORE_FORMAT_XY:

			high_must = low + 1;
			break;

		case SI::INST_TBUFFER_LOAD_FORMAT_XYZW:
		case SI::INST_TBUFFER_STORE_FORMAT_XYZW:

			high_must = low + 3;
			break;

		default:
			misc::panic("%s: MUBUF/MTBUF instruction not recognized: %s",
					__FUNCTION__, info->getName().c_str());
		}

		// Check range
		if (high != high_must)
			misc::fatal("invalid register series: v[%d:%d]", low, high);

		// Encode
		bytes.mtbuf.vdata = low;
		break;
	}

	case TokenOffset:
	{
		// Depends of argument type
		switch (argument->getType())
		{

		case Argument::TypeLiteral:
		case Argument::TypeLiteralReduced:
		{
			ArgLiteral *literal = dynamic_cast
					<ArgLiteral *>(argument);
			if (literal->getValue() > 255)
				misc::fatal("%s: offset needs to fit in 8 bit field",
					__FUNCTION__);

			bytes.smrd.imm = 1;
			bytes.smrd.offset = literal->getValue();
			// FIXME - check valid range of literal
			break;
		}
		case Argument::TypeScalarRegister:
		{
			ArgScalarRegister *sr = dynamic_cast
					<ArgScalarRegister *>(argument);
			assert(sr);
			bytes.smrd.offset = sr->getId();
			break;
		}
		default:
			misc::panic("%s: invalid argument type for TokenOffset",
				__FUNCTION__);
		}
		break;
	}

	case TokenSdst:
	{
		// Encode
		bytes.sop2.sdst = argument->Encode();
		break;
	}

	case TokenSeriesSbase:
	{

		// Check that low register is multiple of 2
		ArgScalarRegisterSeries *srs = dynamic_cast
				<ArgScalarRegisterSeries *>(argument);
		assert(srs);
		if (srs->getLow() % 2)
			misc::fatal("base register must be multiple of 2");

		// Restrictions for high register
		switch (opcode)
		{

		case SI::INST_S_LOAD_DWORDX2:
		case SI::INST_S_LOAD_DWORDX4:

			// High register must be low plus 1
			if (srs->getHigh() != srs->getLow() + 1)
				misc::fatal("register series must be s[x:x+1]");
			break;

		case SI::INST_S_BUFFER_LOAD_DWORD:
		case SI::INST_S_BUFFER_LOAD_DWORDX2:
		case SI::INST_S_BUFFER_LOAD_DWORDX4:

			// High register must be low plus 3
			if (srs->getHigh() != srs->getLow() + 3)
				misc::fatal("register series must be s[x:x+3]");
			break;

		default:
			misc::fatal("%s: unsupported opcode for TokenSeriesSbase: %s",
					__FUNCTION__, info->getName().c_str());
		}

		// Encode
		bytes.smrd.sbase = srs->getLow() / 2;
		break;
	}

	case TokenSeriesSdst:
	{
		// Get argument
		ArgScalarRegisterSeries *srs = dynamic_cast
				<ArgScalarRegisterSeries *>(argument);
		assert(srs);

		// Restrictions for high register
		switch (opcode)
		{

		case SI::INST_S_LOAD_DWORDX2:
		case SI::INST_S_BUFFER_LOAD_DWORDX2:

			// High register must be low plus 1
			if (srs->getHigh() != srs->getLow() + 1)
				misc::fatal("register series must be s[low:low+1]");
			break;

		case SI::INST_S_LOAD_DWORDX4:
		case SI::INST_S_BUFFER_LOAD_DWORDX4:

			// High register must be low plus 3
			if (srs->getHigh() != srs->getLow() + 3)
				misc::fatal("register series must be s[low:low+3]");
			break;

		default:
			misc::fatal("%s: unsupported opcode for 'series_sdst' token: %s",
					__FUNCTION__, info->getName().c_str());
		}

		// Encode
		bytes.smrd.sdst = srs->getLow();
		break;
	}

	case TokenSeriesSrsrc:
	{
		// Get argument
		ArgScalarRegisterSeries *srs = dynamic_cast
				<ArgScalarRegisterSeries *>(argument);
		assert(srs);
		int low = srs->getLow();
		int high = srs->getHigh();

		// Base register must be multiple of 4
		if (low % 4)
			misc::fatal("low register must be multiple of 4 in s[%d:%d]",
					low, high);

		// High register must be low + 3
		if (high != low + 3)
			misc::fatal("register series must span 4 registers in s[%d:%d]",
					low, high);

		// Encode
		bytes.mtbuf.srsrc = low >> 2;
		break;
	}

	case TokenSmrdSdst:
	{
		// Encode
		ArgScalarRegister *sr = dynamic_cast<ArgScalarRegister *>(argument);
		assert(sr);
		bytes.smrd.sdst = sr->getId();
		break;
	}

	case TokenSrc0:
	{
		if (argument->getType() == Argument::TypeLiteral)
		{
			/* Literal constant other than [-16...64] is encoded by adding
			 * four more bits to the instruction. */
			if (size == 8)
				misc::fatal("only one literal allowed");
			size = 8;

			// Set literal
			ArgLiteral *literal = dynamic_cast<ArgLiteral *>(argument);
			assert(literal);
			bytes.vopc.src0 = 0xff;
			bytes.vopc.lit_cnst = literal->getValue();
		}
		else
		{
			bytes.vopc.src0 = argument->Encode();
		}
		break;
	}

	case TokenSsrc0:
	{
		if (argument->getType() == Argument::TypeLiteral)
		{
			/* Literal constant other than [-16...64] is encoded by adding
			 * four more bits to the instruction. */
			if (size == 8)
				misc::fatal("only one literal allowed");
			size = 8;

			// Set literal
			ArgLiteral *literal = dynamic_cast<ArgLiteral *>(argument);
			assert(literal);
			bytes.sop2.ssrc0 = 0xff;
			bytes.sop2.lit_cnst = literal->getValue();
		}
		else
		{
			int value = argument->Encode();
			if (!misc::inRange(value, 0, 255))
				misc::fatal("invalid argument type");
			bytes.sop2.ssrc0 = value;
		}
		break;
	}

	case TokenSsrc1:
	{
		if (argument->getType() == Argument::TypeLiteral)
		{
			/* Literal constant other than [-16...64] is encoded by adding
			 * four more bits to the instruction. */
			if (size == 8)
				misc::fatal("only one literal allowed");
			size = 8;
			
			// Set literal
			ArgLiteral *literal = dynamic_cast<ArgLiteral *>(argument);
			assert(literal);
			bytes.sop2.ssrc1 = 0xff;
			bytes.sop2.lit_cnst = literal->getValue();
		}
		else
		{
			int value = argument->Encode();
			if (!misc::inRange(value, 0, 255))
				misc::fatal("invalid argument type");
			bytes.sop2.ssrc1 = value;
		}
		break;
	}

	case TokenVaddr:
	{
		switch (argument->getType())
		{

		case Argument::TypeVectorRegister:
		{
			ArgVectorRegister *vr = dynamic_cast<ArgVectorRegister *>(argument);
			assert(vr);
			bytes.mtbuf.vaddr = vr->getId();
			break;
		}
		
		case Argument::TypeVectorRegisterSeries:
		{
			// High register must be low plus 1
			ArgVectorRegisterSeries *vrs = dynamic_cast
					<ArgVectorRegisterSeries *>(argument);
			if (vrs->getHigh() != vrs->getLow() + 1)
				misc::fatal("register series must be v[x:x+1]");

			bytes.mtbuf.vaddr = vrs->getLow();
			// FIXME - Find way to verify that idxen and offen are set
			break;
		}

		default:
			misc::fatal("%s: invalid argument type for TokenVaddr",
				__FUNCTION__);
		}
		break;
	}

	case TokenVcc:
	{
		// Not encoded
		break;
	}

	case TokenSvdst:
	{
		assert(argument->getType() == Argument::TypeScalarRegister);
		bytes.vop1.vdst = argument->Encode();
		break;
	}

	case TokenVdst:
	{
		ArgVectorRegister *vr = dynamic_cast<ArgVectorRegister *>(argument);
		assert(vr);
		bytes.vop1.vdst = vr->getId();
		break;
	}

	case Token64Src0:
	{
		int low;
		int high;

		// Check argument type
		switch (argument->getType())
		{

		case Argument::TypeScalarRegisterSeries:
		{
			ArgScalarRegisterSeries *srs = dynamic_cast
					<ArgScalarRegisterSeries *>(argument);
			assert(srs);
			low = srs->getLow();
			high = srs->getHigh();
			if (high != low + 1)
				misc::fatal("register series must be s[low:low+1]");
			break;
		}

		case Argument::TypeVectorRegisterSeries:
		{
			ArgVectorRegisterSeries *vrs = dynamic_cast
					<ArgVectorRegisterSeries *>(argument);
			assert(vrs);
			low = vrs->getLow();
			high = vrs->getHigh();
			if (high != low + 1)
				misc::fatal("register series must be v[low:low+1]");
			break;
		}

		default:
			misc::panic("%s: invalid argument type for Token64Src0",
					__FUNCTION__);
		}

		// Encode
		bytes.vop1.src0 = argument->Encode();
		break;
	}

	case Token64Vdst:
	{
		ArgVectorRegisterSeries *vrs = dynamic_cast
				<ArgVectorRegisterSeries *>(argument);
		assert(vrs);
		int low = vrs->getLow();
		int high = vrs->getHigh();
		if (high != low + 1)
			misc::fatal("register series must be v[low:low+1]");
		
		// Encode
		bytes.vop1.vdst = low;
		break;
	}
	case TokenVop364Svdst:
	{
		// If operand is a scalar register series, check range
		if (argument->getType() == Argument::TypeScalarRegisterSeries)
		{
			ArgScalarRegisterSeries *srs = dynamic_cast
					<ArgScalarRegisterSeries *>(argument);
			assert(srs);
			if (srs->getHigh() != srs->getLow() + 1)
				misc::fatal("register series must be s[low:low+1]");
		}

		// Encode
		bytes.vop3a.vdst = argument->Encode();
		break;
	}

	case TokenVop3Src0:
	{
		bytes.vop3a.src0 = argument->Encode();
		if (argument->getAbs())
			bytes.vop3a.abs |= 0x1;
		if (argument->getNeg())
			bytes.vop3a.neg |= 0x1;
		break;
	}

	case TokenVop3Src1:
	{
		bytes.vop3a.src1 = argument->Encode();
		if (argument->getAbs())
			bytes.vop3a.abs |= 0x2;
		if (argument->getNeg())
			bytes.vop3a.neg |= 0x2;
		break;
	}

	case TokenVop3Src2:
	{
		bytes.vop3a.src2 = argument->Encode();
		if (argument->getAbs())
			bytes.vop3a.abs |= 0x4;
		if (argument->getNeg())
			bytes.vop3a.neg |= 0x4;
		break;
	}

	case TokenVop364Src0:
	{
		// If operand is a scalar register series, check range
		if (argument->getType() == Argument::TypeScalarRegisterSeries)
		{
			ArgScalarRegisterSeries *srs = dynamic_cast
					<ArgScalarRegisterSeries *>(argument);
			if (srs->getHigh() != srs->getLow() + 1)
				misc::fatal("register series must be s[low:low+1]");
		}
		else if (argument->getType() == Argument::TypeVectorRegisterSeries)
		{
			ArgVectorRegisterSeries *vrs = dynamic_cast
					<ArgVectorRegisterSeries *>(argument);
			if (vrs->getHigh() != vrs->getLow() + 1)
				misc::fatal("register series must be v[low:low+1]");
		}

		// Encode
		bytes.vop3a.src0 = argument->Encode();

		if (argument->getAbs())
			bytes.vop3a.abs |= 0x1;
		if (argument->getNeg())
			bytes.vop3a.neg |= 0x1;
		break;
	}
	case TokenVop364Src1:
	{
		// If operand is a scalar register series, check range
		if (argument->getType() == Argument::TypeScalarRegisterSeries)
		{
			ArgScalarRegisterSeries *srs = dynamic_cast
					<ArgScalarRegisterSeries *>(argument);
			if (srs->getHigh() != srs->getLow() + 1)
				misc::fatal("register series must be s[low:low+1]");
		}
		else if (argument->getType() == Argument::TypeVectorRegisterSeries)
		{
			ArgVectorRegisterSeries *vrs = dynamic_cast
					<ArgVectorRegisterSeries *>(argument);
			if (vrs->getHigh() != vrs->getLow() + 1)
				misc::fatal("register series must be v[low:low+1]");
		}

		// Encode
		bytes.vop3a.src1 = argument->Encode();
		if (argument->getAbs())
			bytes.vop3a.abs |= 0x2;
		if (argument->getAbs())
			bytes.vop3a.neg |= 0x2;
		break;
	}
	case TokenVop364Src2:
	{
		// If operand is a scalar register series, check range
		if (argument->getType() == Argument::TypeScalarRegisterSeries)
		{
			ArgScalarRegisterSeries *srs = dynamic_cast
					<ArgScalarRegisterSeries *>(argument);
			if (srs->getHigh() != srs->getLow() + 1)
				misc::fatal("register series must be s[low:low+1]");
		}
		else if (argument->getType() == Argument::TypeVectorRegisterSeries)
		{
			ArgVectorRegisterSeries *vrs = dynamic_cast
					<ArgVectorRegisterSeries *>(argument);
			if (vrs->getHigh() != vrs->getLow() + 1)
				misc::fatal("register series must be v[low:low+1]");
		}

		// Encode
		bytes.vop3a.src2 = argument->Encode();
		if (argument->getAbs())
			bytes.vop3a.abs |= 0x4;
		if (argument->getNeg())
			bytes.vop3a.neg |= 0x4;
		break;
	}

	case TokenVop364Sdst:
	{
		// Encode
		bytes.vop3b.sdst = argument->Encode();
		break;
	}

	case TokenVop3Vdst:
	{
		ArgVectorRegister *vr = dynamic_cast
				<ArgVectorRegister *>(argument);
		assert(vr);
		bytes.vop3a.vdst = vr->getId();
		break;
	}

	case TokenVop364Vdst:
	{
		ArgVectorRegisterSeries *vrs = dynamic_cast
				<ArgVectorRegisterSeries *>(argument);
		if (vrs->getHigh() != vrs->getLow() + 1)
			misc::fatal("register series must be v[low:low+1]");

		bytes.vop3a.vdst = vrs->getLow();
		break;
	}

	case TokenVsrc1:
	{
		ArgVectorRegister *vr = dynamic_cast
				<ArgVectorRegister *>(argument);
		bytes.vopc.vsrc1 = vr->getId();
		break;
	}

	case TokenWaitCnt:
	{
		ArgWaitCounter *wait_cnt = dynamic_cast<ArgWaitCounter *>(argument);
		assert(wait_cnt);

		// vmcnt(x)
		if (wait_cnt->getVmcntActive())
		{
			if (!misc::inRange(wait_cnt->getVmcntValue(), 0, 0xe))
				misc::fatal("invalid value for vmcnt");
			bytes.sopp.simm16 = misc::setBits32(bytes.sopp.simm16,
					3, 0, wait_cnt->getVmcntValue());
		}
		else
		{
			bytes.sopp.simm16 = misc::setBits32(bytes.sopp.simm16,
					3, 0, 0xf);
		}

		// lgkmcnt(x)
		if (wait_cnt->getLgkmcntActive())
		{
			if (!misc::inRange(wait_cnt->getLgkmcntValue(), 0, 0x1e))
				misc::fatal("invalid value for lgkmcnt");
			bytes.sopp.simm16 = misc::setBits32(bytes.sopp.simm16,
					12, 8, wait_cnt->getLgkmcntValue());
		}
		else
		{
			bytes.sopp.simm16 = misc::setBits32(bytes.sopp.simm16,
					12, 8, 0x1f);
		}

		// expcnt(x)
		if (wait_cnt->getExpcntActive())
		{
			if (!misc::inRange(wait_cnt->getExpcntValue(), 0, 0x6))
				misc::fatal("invalid value for expcnt");
			bytes.sopp.simm16 = misc::setBits32(bytes.sopp.simm16,
					6, 4, wait_cnt->getExpcntValue());
		}
		else
		{
			bytes.sopp.simm16 = misc::setBits32(bytes.sopp.simm16,
					6, 4, 0x7);
		}
		break;
	}

	case TokenAddr:
	{
		ArgVectorRegister *vr = dynamic_cast
				<ArgVectorRegister *>(argument);
		assert(vr);
		bytes.ds.addr = vr->getId();
		break;
	}

	case TokenData0:
	{
		ArgVectorRegister *vr = dynamic_cast
				<ArgVectorRegister *>(argument);
		assert(vr);
		bytes.ds.data0 = vr->getId();
		break;
	}

	case TokenDsVdst:
	{
		ArgVectorRegister *vr = dynamic_cast
				<ArgVectorRegister *>(argument);
		assert(vr);
		bytes.ds.vdst = vr->getId();
		break;
	}
	case TokenLabel:
	{
		Symbol *symbol;
		
		ArgLabel *label = dynamic_cast <ArgLabel *>(argument);
		
		symbol = context->getSymbol(label->getName());
	
		// Search symbol in symbol table
		assert(argument->getType() == Argument::TypeLabel);

		// Create symbol if it doesn't exist
		if (!symbol)
		{
			symbol = context->newSymbol(label->getName());
		}

		/* If symbol is defined, resolve label right away. Otherwise,
		 * program a deferred task to resolve it. */
		if (symbol->getDefined())
		{
			bytes.sopp.simm16 = (symbol->getValue() -
					context->getTextBuffer()->getWritePosition()) / 4 - 1;
		}
		else
		{
			context->newTask(context->getTextBuffer()->getWritePosition(),
					symbol, context->getTextBuffer());
		}
		break;
	}

	default:
		si2bin_yyerror_fmt("unsupported token for argument %d",
				argument->getIndex() + 1);
	}
}


void Instruction::Encode()
{
	/* By default, the instruction has the number of bytes specified by its
	 * format. 4-bit instructions could be extended later to 8 bits upon
	 * the presence of a literal constant. */
	assert(info);
	SI::InstInfo *si_info = info->getInfo();
	size = si_info->size;

	// Instruction opcode
	switch (si_info->fmt)
	{

	// encoding in [31:26], op in [18:16]
	case SI::InstFormatMTBUF:

		bytes.mtbuf.enc = 0x3a;
		bytes.mtbuf.op = si_info->op;
		break;

	// encoding in [:], op in []
	case SI::InstFormatMUBUF:

		bytes.mubuf.enc = 0x38;
		bytes.mubuf.op = si_info->op;
		break;

	// encoding in [:], op in []
	case SI::InstFormatMIMG:
		
		bytes.mimg.enc = 0x3c;
		bytes.mimg.op = si_info->op;
		break;

	// encoding in [31:27], op in [26:22]
	case SI::InstFormatSMRD:

		bytes.smrd.enc = 0x18;
		bytes.smrd.op = si_info->op;
		break;

	// encoding in [31:26], op in [25:28]
	case SI::InstFormatDS:

		bytes.ds.enc = 0x36;
		bytes.ds.op = si_info->op;
		break;

	// encoding in [31:23], op in [22:16]
	case SI::InstFormatSOPC:

		bytes.sopc.enc = 0x17e;
		bytes.sopc.op = si_info->op;
		break;

	// encoding in [31:23], op in [15:8]
	case SI::InstFormatSOP1:

		bytes.sop1.enc = 0x17d;
		bytes.sop1.op = si_info->op;
		break;

	// encoding in [31:30], op in [29:23]
	case SI::InstFormatSOP2:

		bytes.sop2.enc = 0x2;
		bytes.sop2.op = si_info->op;
		break;

	// encoding in [31:23], op in [22:16]
	case SI::InstFormatSOPP:

		bytes.sopp.enc = 0x17f;
		bytes.sopp.op = si_info->op;
		break;

	// encoding in [:], op in []
	case SI::InstFormatSOPK:

		bytes.sopk.enc = 0xb;
		bytes.sopk.op = si_info->op;
		break;

	// encoding in [:], op in []
	case SI::InstFormatVOPC:

		bytes.vopc.enc = 0x3e;
		bytes.vopc.op = si_info->op;
		break;

	// encoding in [31:25], op in [16:9]
	case SI::InstFormatVOP1:

		bytes.vop1.enc = 0x3f;
		bytes.vop1.op = si_info->op;
		break;

	// encoding in [31], op in [30:25]
	case SI::InstFormatVOP2:

		bytes.vop2.enc = 0x0;
		bytes.vop2.op = si_info->op;
		break;

	// encoding in [31:26], op in [25:17]
	case SI::InstFormatVOP3a:

		bytes.vop3a.enc = 0x34;
		bytes.vop3a.op = si_info->op;
		break;

	// encoding in [31:26], op in [25:17]
	case SI::InstFormatVOP3b:

		bytes.vop3a.enc = 0x34;
		bytes.vop3a.op = si_info->op;
		break;

	// encoding in [:], op in []
	case SI::InstFormatVINTRP:

		bytes.vintrp.enc = 0x32;
		bytes.vintrp.op = si_info->op;
		break;

	// encoding in [:], op in []
	case SI::InstFormatEXP:

		bytes.exp.enc = 0x3e;
		// No opcode: only 1 instruction
		break;

	default:
		misc::panic("%s: unsupported format",
				__FUNCTION__);
	}

	// Arguments
	assert(arguments.size() == info->getNumTokens());
	for (unsigned i = 0; i < arguments.size(); i++)
	{
		// Get argument and token
		Argument *argument = arguments[i].get();
		Token *token = info->getToken(i);
		assert(token);
		EncodeArgument(argument, token);
	}
}

}  // namespace si2bin
