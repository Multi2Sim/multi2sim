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

#include "Arg.h"
#include "Context.h"
#include "Inst.h"
#include "Token.h"


using namespace misc;

namespace si2bin
{

void Inst::Initialize()
{
	size = 0;
	bytes.dword = 0;
	basic_block = nullptr;

	/* Assign argument indices */
	for (unsigned i = 0; i < args.size(); i++)
		args[i]->index = i;
}


void Inst::Initialize(SI::InstOpcode opcode)
{
	/* Common initialization */
	Initialize();

	/* Check opcode */
	this->opcode = opcode;
	if (!InRange(opcode, 1, SI::InstOpcodeCount - 1))
		fatal("%s: invalid opcode (%d)", __FUNCTION__, opcode);

	/* Get instruction information */
	info = context.getInstInfo(opcode);
	if (!info)
		fatal("%s: opcode %d not supported", __FUNCTION__, opcode);

	/* Check number of arguments */
	if (args.size() != info->getNumTokens())
		fatal("%s: invalid number of arguments (%d given, %d expected)",
				__FUNCTION__, (int) args.size(),
				(int) info->getNumTokens());

	/* Check argument types */
	for (unsigned i = 0; i < args.size(); i++)
	{
		/* Get formal argument from instruction info. Associate token with the
		 * instruction argument. */
		Token *token = info->getToken(i);
		Arg *arg = args[i].get();
		arg->token = token;
		assert(token);

		/* Check that actual argument type is acceptable for token */
		if (!token->IsArgAllowed(arg))
			fatal("%s: invalid type for argument %d", __FUNCTION__, i + 1);
	}
}


void Inst::Initialize(const std::string &name)
{
	/* Common initialization */
	Initialize();

	/* Try to create the instruction following all possible encodings for
	 * the same instruction name. */
	std::string error = "invalid instruction: " + name;
	for (info = context.getInstInfo(name); info; info = info->getNext())
	{
		/* Check number of arguments */
		if (args.size() != info->getNumTokens())
		{
			error = StringFmt("invalid number of arguments for %s "
					"(%d given, %d expected)",
					name.c_str(), (int) args.size(),
					(int) info->getNumTokens() - 1);
			continue;
		}

		/* Check arguments */
		error = "";
		for (unsigned i = 0; i < args.size(); i++)
		{
			/* Get formal argument from instruction info. We associate the
			 * instruction argument with the token. */
			Arg *arg = args[i].get();
			Token *token = info->getToken(i);
			arg->token = token;
			assert(token);

			/* Check that actual argument type is acceptable for token */
			if (!token->IsArgAllowed(arg))
			{
				error = StringFmt("invalid type for argument %d",
						arg->index + 1);
				break;
			}
		}

		/* Error while processing arguments */
		if (!error.empty())
			continue;
	
		/* All checks passed, instruction identified correctly as that
		 * represented by 'info'. */
		break;
	}

	/* Error identifying instruction */
	if (!info)
		fatal("%s", error.c_str());

	/* Initialize opcode */
	opcode = info->getOpcode();
}


void Inst::Dump(std::ostream &os)
{
        /* Comment attached to the instruction */
        if (!comment.empty())
        	os << "\n\t# " << comment << '\n';

        /* Dump instruction opcode */
        os << '\t' << info->getName() << ' ';

        /* Dump arguments */
        for (auto &arg : args)
        {
        	arg->Dump(os);
                if (arg->index < (int) args.size() - 1)
                	os << ", ";
	}

	/* End */
        os << '\n';
}


void Inst::EncodeArg(Arg *arg, Token *token)
{
	/* Check token */
	switch (token->GetType())
	{

	case TokenSimm16:
	{
		int value;

		ArgLiteral *literal = dynamic_cast<ArgLiteral *>(arg);
		if (literal)
		{
			/* Literal constant other than [-16...64] */
			if (literal->getValue() > 0xff00)
				fatal("%s: Literal in simm16 needs to fit in 16 bit field",
					__FUNCTION__);
			bytes.sopk.simm16 = literal->getValue();
		}
		else
		{
			/* Encode */
			value = arg->Encode();
			if (!InRange(value, 0, 255))
				fatal("invalid argument type");
			bytes.sopk.simm16 = value;
		}
		break;
	}
	
	case Token64Sdst:
	{
		/* Check range if scalar register range given */
		ArgScalarRegisterSeries *srs = dynamic_cast
				<ArgScalarRegisterSeries *>(arg);
		if (srs && srs->getHigh() != srs->getLow() + 1)
			fatal("register series must be s[x:x+1]");
		
		/* Encode */
		bytes.sop2.sdst = arg->Encode();
		break;
	}

	case Token64Ssrc0:
	{
		ArgLiteral *literal = dynamic_cast<ArgLiteral *>(arg);
		if (literal)
		{
			/* Literal constant other than [-16...64] is encoded by adding
			 * four more bits to the instruction. */
			if (size == 8)
				fatal("only one literal allowed");
			size = 8;
			bytes.sop2.ssrc0 = 0xff;
			bytes.sop2.lit_cnst = literal->getValue();
		}
		else
		{
			/* Check range of scalar registers */
			ArgScalarRegisterSeries *srs = dynamic_cast
					<ArgScalarRegisterSeries *>(arg);
			if (srs && srs->getHigh() != srs->getLow() + 1)
				fatal("invalid scalar register series, s[x:x+1] expected");

			/* Encode */
			int value = arg->Encode();
			if (!InRange(value, 0, 255))
				fatal("invalid argument type");
			bytes.sop2.ssrc0 = value;
		}
		break;
	}

	case Token64Ssrc1:
	{
		ArgLiteral *literal = dynamic_cast<ArgLiteral *>(arg);
		if (literal)
		{
			/* Literal constant other than [-16...64] is encoded by adding
			 * four more bits to the instruction. */
			if (size == 8)
				fatal("only one literal allowed");
			size = 8;
			bytes.sop2.ssrc1 = 0xff;
			bytes.sop2.lit_cnst = literal->getValue();
		}
		else
		{
			/* Check range of scalar registers */
			ArgScalarRegisterSeries *srs = dynamic_cast
					<ArgScalarRegisterSeries *>(arg);
			if (srs && srs->getHigh() != srs->getLow() + 1)
				fatal("invalid scalar register series, s[x:x+1] expected");

			/* Encode */
			int value = arg->Encode();
			if (!InRange(value, 0, 255))
				fatal("invalid argument type");
			bytes.sop2.ssrc1 = value;
		}
		break;
	}
	
	case TokenMtMaddr:
	{
		/* Get argument */
		ArgMaddr *maddr = dynamic_cast<ArgMaddr *>(arg);
		assert(maddr);

		/* Offset */
		int soffset = maddr->getSoffset()->Encode();
		if (!InRange(soffset, 0, 253))
			fatal("invalid offset");
		bytes.mtbuf.soffset = soffset;

		/* Data and number format */
		bytes.mtbuf.dfmt = maddr->getDataFormat();
		bytes.mtbuf.nfmt = maddr->getNumFormat();

		/* Qualifiers */
		ArgMaddrQual *qual = maddr->getQual();
		bytes.mtbuf.offen = qual->getOffen();
		bytes.mtbuf.idxen = qual->getIdxen();
		bytes.mtbuf.offset = qual->getOffset();

		break;
	}

	case TokenMtSeriesVdata:
	{
		int low = 0;
		int high = 0;
		int high_must = 0;

		/* Get registers */
		switch (arg->type)
		{

		case ArgTypeVectorRegister:
		{
			ArgVectorRegister *vr = dynamic_cast
					<ArgVectorRegister *>(arg);
			assert(vr);
			low = vr->getId();
			high = low;
			break;
		}

		case ArgTypeVectorRegisterSeries:
		{
			ArgVectorRegisterSeries *vrs = dynamic_cast
					<ArgVectorRegisterSeries *>(arg);
			low = vrs->getLow();
			high = vrs->getHigh();
			break;
		}

		default:
			panic("%s: invalid argument for TokenMtSeriesVdata",
					__FUNCTION__);
		}

		/* Restriction in vector register range */
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
			panic("%s: MUBUF/MTBUF instruction not recognized: %s",
					__FUNCTION__, info->getName().c_str());
		}

		/* Check range */
		if (high != high_must)
			fatal("invalid register series: v[%d:%d]", low, high);

		/* Encode */
		bytes.mtbuf.vdata = low;
		break;
	}

	case TokenOffset:
	{
		/* Depends of argument type */
		switch (arg->type)
		{

		case ArgTypeLiteral:
		case ArgTypeLiteralReduced:
		{
			ArgLiteral *literal = dynamic_cast
					<ArgLiteral *>(arg);
			if (literal->getValue() > 255)
				fatal("%s: offset needs to fit in 8 bit field",
					__FUNCTION__);

			bytes.smrd.imm = 1;
			bytes.smrd.offset = literal->getValue();
			/* FIXME - check valid range of literal */
			break;
		}
		case ArgTypeScalarRegister:
		{
			ArgScalarRegister *sr = dynamic_cast
					<ArgScalarRegister *>(arg);
			assert(sr);
			bytes.smrd.offset = sr->getId();
			break;
		}
		default:
			panic("%s: invalid argument type for TokenOffset",
				__FUNCTION__);
		}
		break;
	}

	case TokenSdst:
	{
		/* Encode */
		bytes.sop2.sdst = arg->Encode();
		break;
	}

	case TokenSeriesSbase:
	{

		/* Check that low register is multiple of 2 */
		ArgScalarRegisterSeries *srs = dynamic_cast
				<ArgScalarRegisterSeries *>(arg);
		assert(srs);
		if (srs->getLow() % 2)
			fatal("base register must be multiple of 2");

		/* Restrictions for high register */
		switch (opcode)
		{

		case SI::INST_S_LOAD_DWORDX2:
		case SI::INST_S_LOAD_DWORDX4:

			/* High register must be low plus 1 */
			if (srs->getHigh() != srs->getLow() + 1)
				fatal("register series must be s[x:x+1]");
			break;

		case SI::INST_S_BUFFER_LOAD_DWORD:
		case SI::INST_S_BUFFER_LOAD_DWORDX2:
		case SI::INST_S_BUFFER_LOAD_DWORDX4:

			/* High register must be low plus 3 */
			if (srs->getHigh() != srs->getLow() + 3)
				fatal("register series must be s[x:x+3]");
			break;

		default:
			fatal("%s: unsupported opcode for TokenSeriesSbase: %s",
					__FUNCTION__, info->getName().c_str());
		}

		/* Encode */
		bytes.smrd.sbase = srs->getLow() / 2;
		break;
	}

	case TokenSeriesSdst:
	{
		/* Get argument */
		ArgScalarRegisterSeries *srs = dynamic_cast
				<ArgScalarRegisterSeries *>(arg);
		assert(srs);

		/* Restrictions for high register */
		switch (opcode)
		{

		case SI::INST_S_LOAD_DWORDX2:
		case SI::INST_S_BUFFER_LOAD_DWORDX2:

			/* High register must be low plus 1 */
			if (srs->getHigh() != srs->getLow() + 1)
				fatal("register series must be s[low:low+1]");
			break;

		case SI::INST_S_LOAD_DWORDX4:
		case SI::INST_S_BUFFER_LOAD_DWORDX4:

			/* High register must be low plus 3 */
			if (srs->getHigh() != srs->getLow() + 3)
				fatal("register series must be s[low:low+3]");
			break;

		default:
			fatal("%s: unsupported opcode for 'series_sdst' token: %s",
					__FUNCTION__, info->getName().c_str());
		}

		/* Encode */
		bytes.smrd.sdst = srs->getLow();
		break;
	}

	case TokenSeriesSrsrc:
	{
		/* Get argument */
		ArgScalarRegisterSeries *srs = dynamic_cast
				<ArgScalarRegisterSeries *>(arg);
		assert(srs);
		int low = srs->getLow();
		int high = srs->getHigh();

		/* Base register must be multiple of 4 */
		if (low % 4)
			fatal("low register must be multiple of 4 in s[%d:%d]",
					low, high);

		/* High register must be low + 3 */
		if (high != low + 3)
			fatal("register series must span 4 registers in s[%d:%d]",
					low, high);

		/* Encode */
		bytes.mtbuf.srsrc = low >> 2;
		break;
	}

	case TokenSmrdSdst:
	{
		/* Encode */
		ArgScalarRegister *sr = dynamic_cast<ArgScalarRegister *>(arg);
		assert(sr);
		bytes.smrd.sdst = sr->getId();
		break;
	}

	case TokenSrc0:
	{
		if (arg->type == ArgTypeLiteral)
		{
			/* Literal constant other than [-16...64] is encoded by adding
			 * four more bits to the instruction. */
			if (size == 8)
				fatal("only one literal allowed");
			size = 8;

			/* Set literal */
			ArgLiteral *literal = dynamic_cast<ArgLiteral *>(arg);
			assert(literal);
			bytes.vopc.src0 = 0xff;
			bytes.vopc.lit_cnst = literal->getValue();
		}
		else
		{
			bytes.vopc.src0 = arg->Encode();
		}
		break;
	}

	case TokenSsrc0:
	{
		if (arg->type == ArgTypeLiteral)
		{
			/* Literal constant other than [-16...64] is encoded by adding
			 * four more bits to the instruction. */
			if (size == 8)
				fatal("only one literal allowed");
			size = 8;

			/* Set literal */
			ArgLiteral *literal = dynamic_cast<ArgLiteral *>(arg);
			assert(literal);
			bytes.sop2.ssrc0 = 0xff;
			bytes.sop2.lit_cnst = literal->getValue();
		}
		else
		{
			int value = arg->Encode();
			if (!InRange(value, 0, 255))
				fatal("invalid argument type");
			bytes.sop2.ssrc0 = value;
		}
		break;
	}

	case TokenSsrc1:
	{
		if (arg->type == ArgTypeLiteral)
		{
			/* Literal constant other than [-16...64] is encoded by adding
			 * four more bits to the instruction. */
			if (size == 8)
				fatal("only one literal allowed");
			size = 8;
			
			/* Set literal */
			ArgLiteral *literal = dynamic_cast<ArgLiteral *>(arg);
			assert(literal);
			bytes.sop2.ssrc1 = 0xff;
			bytes.sop2.lit_cnst = literal->getValue();
		}
		else
		{
			int value = arg->Encode();
			if (!InRange(value, 0, 255))
				fatal("invalid argument type");
			bytes.sop2.ssrc1 = value;
		}
		break;
	}

	case TokenVaddr:
	{
		switch (arg->type)
		{

		case ArgTypeVectorRegister:
		{
			ArgVectorRegister *vr = dynamic_cast<ArgVectorRegister *>(arg);
			assert(vr);
			bytes.mtbuf.vaddr = vr->getId();
			break;
		}
		
		case ArgTypeVectorRegisterSeries:
		{
			/* High register must be low plus 1 */
			ArgVectorRegisterSeries *vrs = dynamic_cast
					<ArgVectorRegisterSeries *>(arg);
			if (vrs->getHigh() != vrs->getLow() + 1)
				fatal("register series must be v[x:x+1]");

			bytes.mtbuf.vaddr = vrs->getLow();
			/* FIXME - Find way to verify that idxen and offen are set */
			break;
		}

		default:
			fatal("%s: invalid argument type for TokenVaddr",
				__FUNCTION__);
		}
		break;
	}

	case TokenVcc:
	{
		/* Not encoded */
		break;
	}

	case TokenSvdst:
	{
		assert(arg->type == ArgTypeScalarRegister);
		bytes.vop1.vdst = arg->Encode();
		break;
	}

	case TokenVdst:
	{
		ArgVectorRegister *vr = dynamic_cast<ArgVectorRegister *>(arg);
		assert(vr);
		bytes.vop1.vdst = vr->getId();
		break;
	}

	case Token64Src0:
	{
		int low;
		int high;

		/* Check argument type */
		switch (arg->type)
		{

		case ArgTypeScalarRegisterSeries:
		{
			ArgScalarRegisterSeries *srs = dynamic_cast
					<ArgScalarRegisterSeries *>(arg);
			assert(srs);
			low = srs->getLow();
			high = srs->getHigh();
			if (high != low + 1)
				fatal("register series must be s[low:low+1]");
			break;
		}

		case ArgTypeVectorRegisterSeries:
		{
			ArgVectorRegisterSeries *vrs = dynamic_cast
					<ArgVectorRegisterSeries *>(arg);
			assert(vrs);
			low = vrs->getLow();
			high = vrs->getHigh();
			if (high != low + 1)
				fatal("register series must be v[low:low+1]");
			break;
		}

		default:
			panic("%s: invalid argument type for Token64Src0",
					__FUNCTION__);
		}

		/* Encode */
		bytes.vop1.src0 = arg->Encode();
		break;
	}

	case Token64Vdst:
	{
		ArgVectorRegisterSeries *vrs = dynamic_cast
				<ArgVectorRegisterSeries *>(arg);
		assert(vrs);
		int low = vrs->getLow();
		int high = vrs->getHigh();
		if (high != low + 1)
			fatal("register series must be v[low:low+1]");
		
		/* Encode */
		bytes.vop1.vdst = low;
		break;
	}
	case TokenVop364Svdst:
	{
		/* If operand is a scalar register series, check range */
		if (arg->type == ArgTypeScalarRegisterSeries)
		{
			ArgScalarRegisterSeries *srs = dynamic_cast
					<ArgScalarRegisterSeries *>(arg);
			assert(srs);
			if (srs->getHigh() != srs->getLow() + 1)
				fatal("register series must be s[low:low+1]");
		}

		/* Encode */
		bytes.vop3a.vdst = arg->Encode();
		break;
	}

	case TokenVop3Src0:
	{
		bytes.vop3a.src0 = arg->Encode();
		if (arg->getAbs())
			bytes.vop3a.abs |= 0x1;
		if (arg->getNeg())
			bytes.vop3a.neg |= 0x1;
		break;
	}

	case TokenVop3Src1:
	{
		bytes.vop3a.src1 = arg->Encode();
		if (arg->getAbs())
			bytes.vop3a.abs |= 0x2;
		if (arg->getNeg())
			bytes.vop3a.neg |= 0x2;
		break;
	}

	case TokenVop3Src2:
	{
		bytes.vop3a.src2 = arg->Encode();
		if (arg->getAbs())
			bytes.vop3a.abs |= 0x4;
		if (arg->getNeg())
			bytes.vop3a.neg |= 0x4;
		break;
	}

	case TokenVop364Src0:
	{
		/* If operand is a scalar register series, check range */
		if (arg->type == ArgTypeScalarRegisterSeries)
		{
			ArgScalarRegisterSeries *srs = dynamic_cast
					<ArgScalarRegisterSeries *>(arg);
			if (srs->getHigh() != srs->getLow() + 1)
				fatal("register series must be s[low:low+1]");
		}
		else if (arg->type == ArgTypeVectorRegisterSeries)
		{
			ArgVectorRegisterSeries *vrs = dynamic_cast
					<ArgVectorRegisterSeries *>(arg);
			if (vrs->getHigh() != vrs->getLow() + 1)
				fatal("register series must be v[low:low+1]");
		}

		/* Encode */
		bytes.vop3a.src0 = arg->Encode();

		if (arg->getAbs())
			bytes.vop3a.abs |= 0x1;
		if (arg->getNeg())
			bytes.vop3a.neg |= 0x1;
		break;
	}
	case TokenVop364Src1:
	{
		/* If operand is a scalar register series, check range */
		if (arg->type == ArgTypeScalarRegisterSeries)
		{
			ArgScalarRegisterSeries *srs = dynamic_cast
					<ArgScalarRegisterSeries *>(arg);
			if (srs->getHigh() != srs->getLow() + 1)
				fatal("register series must be s[low:low+1]");
		}
		else if (arg->type == ArgTypeVectorRegisterSeries)
		{
			ArgVectorRegisterSeries *vrs = dynamic_cast
					<ArgVectorRegisterSeries *>(arg);
			if (vrs->getHigh() != vrs->getLow() + 1)
				fatal("register series must be v[low:low+1]");
		}

		/* Encode */
		bytes.vop3a.src1 = arg->Encode();
		if (arg->getAbs())
			bytes.vop3a.abs |= 0x2;
		if (arg->getAbs())
			bytes.vop3a.neg |= 0x2;
		break;
	}
	case TokenVop364Src2:
	{
		/* If operand is a scalar register series, check range */
		if (arg->type == ArgTypeScalarRegisterSeries)
		{
			ArgScalarRegisterSeries *srs = dynamic_cast
					<ArgScalarRegisterSeries *>(arg);
			if (srs->getHigh() != srs->getLow() + 1)
				fatal("register series must be s[low:low+1]");
		}
		else if (arg->type == ArgTypeVectorRegisterSeries)
		{
			ArgVectorRegisterSeries *vrs = dynamic_cast
					<ArgVectorRegisterSeries *>(arg);
			if (vrs->getHigh() != vrs->getLow() + 1)
				fatal("register series must be v[low:low+1]");
		}

		/* Encode */
		bytes.vop3a.src2 = arg->Encode();
		if (arg->getAbs())
			bytes.vop3a.abs |= 0x4;
		if (arg->getNeg())
			bytes.vop3a.neg |= 0x4;
		break;
	}

	case TokenVop364Sdst:
	{
		/* Encode */
		bytes.vop3b.sdst = arg->Encode();
		break;
	}

	case TokenVop3Vdst:
	{
		ArgVectorRegister *vr = dynamic_cast
				<ArgVectorRegister *>(arg);
		assert(vr);
		bytes.vop3a.vdst = vr->getId();
		break;
	}

	case TokenVop364Vdst:
	{
		ArgVectorRegisterSeries *vrs = dynamic_cast
				<ArgVectorRegisterSeries *>(arg);
		if (vrs->getHigh() != vrs->getLow() + 1)
			fatal("register series must be v[low:low+1]");

		bytes.vop3a.vdst = vrs->getLow();
		break;
	}

	case TokenVsrc1:
	{
		ArgVectorRegister *vr = dynamic_cast
				<ArgVectorRegister *>(arg);
		bytes.vopc.vsrc1 = vr->getId();
		break;
	}

	case TokenWaitCnt:
	{
		ArgWaitCnt *wait_cnt = dynamic_cast<ArgWaitCnt *>(arg);
		assert(wait_cnt);

		/* vmcnt(x) */
		if (wait_cnt->getVmcntActive())
		{
			if (!InRange(wait_cnt->getVmcntValue(), 0, 0xe))
				fatal("invalid value for vmcnt");
			bytes.sopp.simm16 = SetBits32(bytes.sopp.simm16,
					3, 0, wait_cnt->getVmcntValue());
		}
		else
		{
			bytes.sopp.simm16 = SetBits32(bytes.sopp.simm16,
					3, 0, 0xf);
		}

		/* lgkmcnt(x) */
		if (wait_cnt->getLgkmcntActive())
		{
			if (!InRange(wait_cnt->getLgkmcntValue(), 0, 0x1e))
				fatal("invalid value for lgkmcnt");
			bytes.sopp.simm16 = SetBits32(bytes.sopp.simm16,
					12, 8, wait_cnt->getLgkmcntValue());
		}
		else
		{
			bytes.sopp.simm16 = SetBits32(bytes.sopp.simm16,
					12, 8, 0x1f);
		}

		/* expcnt(x) */
		if (wait_cnt->getExpcntActive())
		{
			if (!InRange(wait_cnt->getExpcntValue(), 0, 0x6))
				fatal("invalid value for expcnt");
			bytes.sopp.simm16 = SetBits32(bytes.sopp.simm16,
					6, 4, wait_cnt->getExpcntValue());
		}
		else
		{
			bytes.sopp.simm16 = SetBits32(bytes.sopp.simm16,
					6, 4, 0x7);
		}
		break;
	}

	case TokenAddr:
	{
		ArgVectorRegister *vr = dynamic_cast
				<ArgVectorRegister *>(arg);
		assert(vr);
		bytes.ds.addr = vr->getId();
		break;
	}

	case TokenData0:
	{
		ArgVectorRegister *vr = dynamic_cast
				<ArgVectorRegister *>(arg);
		assert(vr);
		bytes.ds.data0 = vr->getId();
		break;
	}

	case TokenDsVdst:
	{
		ArgVectorRegister *vr = dynamic_cast
				<ArgVectorRegister *>(arg);
		assert(vr);
		bytes.ds.vdst = vr->getId();
		break;
	}
#if 0	
	case TokenLabel:
	{
		Si2binSymbol *symbol;
		Si2binTask *task;

		/* Search symbol in symbol table */
		assert(arg->type == ArgTypeLabel);
		symbol = asSi2binSymbol(HashTableGetString(si2bin->symbol_table,
				arg->value.label.name));

		/* Create symbol if it doesn't exist */
		if (!symbol)
		{
			symbol = new(Si2binSymbol, arg->value.label.name);
			HashTableInsert(si2bin->symbol_table, asObject(symbol->name),
					asObject(symbol));
		}

		/* If symbol is defined, resolve label right away. Otherwise,
		 * program a deferred task to resolve it. */
		if (symbol->defined)
		{
			bytes.sopp.simm16 = (symbol->value -
					si2bin_entry->text_section_buffer
					->offset) / 4 - 1;
		}
		else
		{
			task = new(Si2binTask, si2bin_entry->text_section_buffer
					->offset, symbol);
			ListAdd(si2bin->task_list, asObject(task));
		}
		break;
	}
#endif

	default:
		fatal("unsupported token for argument %d",
				arg->index + 1);
	}
}


void Inst::Encode()
{
	/* By default, the instruction has the number of bytes specified by its
	 * format. 4-bit instructions could be extended later to 8 bits upon
	 * the presence of a literal constant. */
	assert(info);
	SI::InstInfo *si_info = info->getInfo();
	size = si_info->size;

	/* Instruction opcode */
	switch (si_info->fmt)
	{

	/* encoding in [31:26], op in [18:16] */
	case SI::InstFormatMTBUF:

		bytes.mtbuf.enc = 0x3a;
		bytes.mtbuf.op = si_info->op;
		break;

	/* encoding in [:], op in [] */
	case SI::InstFormatMUBUF:

		bytes.mubuf.enc = 0x38;
		bytes.mubuf.op = si_info->op;
		break;

	/* encoding in [:], op in [] */
	case SI::InstFormatMIMG:
		
		bytes.mimg.enc = 0x3c;
		bytes.mimg.op = si_info->op;
		break;

	/* encoding in [31:27], op in [26:22] */
	case SI::InstFormatSMRD:

		bytes.smrd.enc = 0x18;
		bytes.smrd.op = si_info->op;
		break;

	/* encoding in [31:26], op in [25:28] */
	case SI::InstFormatDS:

		bytes.ds.enc = 0x36;
		bytes.ds.op = si_info->op;
		break;

	/* encoding in [31:23], op in [22:16] */
	case SI::InstFormatSOPC:

		bytes.sopc.enc = 0x17e;
		bytes.sopc.op = si_info->op;
		break;

	/* encoding in [31:23], op in [15:8] */
	case SI::InstFormatSOP1:

		bytes.sop1.enc = 0x17d;
		bytes.sop1.op = si_info->op;
		break;

	/* encoding in [31:30], op in [29:23] */
	case SI::InstFormatSOP2:

		bytes.sop2.enc = 0x2;
		bytes.sop2.op = si_info->op;
		break;

	/* encoding in [31:23], op in [22:16] */
	case SI::InstFormatSOPP:

		bytes.sopp.enc = 0x17f;
		bytes.sopp.op = si_info->op;
		break;

	/* encoding in [:], op in [] */
	case SI::InstFormatSOPK:

		bytes.sopk.enc = 0xb;
		bytes.sopk.op = si_info->op;
		break;

	/* encoding in [:], op in [] */
	case SI::InstFormatVOPC:

		bytes.vopc.enc = 0x3e;
		bytes.vopc.op = si_info->op;
		break;

	/* encoding in [31:25], op in [16:9] */
	case SI::InstFormatVOP1:

		bytes.vop1.enc = 0x3f;
		bytes.vop1.op = si_info->op;
		break;

	/* encoding in [31], op in [30:25] */
	case SI::InstFormatVOP2:

		bytes.vop2.enc = 0x0;
		bytes.vop2.op = si_info->op;
		break;

	/* encoding in [31:26], op in [25:17] */
	case SI::InstFormatVOP3a:

		bytes.vop3a.enc = 0x34;
		bytes.vop3a.op = si_info->op;
		break;

	/* encoding in [31:26], op in [25:17] */
	case SI::InstFormatVOP3b:

		bytes.vop3a.enc = 0x34;
		bytes.vop3a.op = si_info->op;
		break;

	/* encoding in [:], op in [] */
	case SI::InstFormatVINTRP:

		bytes.vintrp.enc = 0x32;
		bytes.vintrp.op = si_info->op;
		break;

	/* encoding in [:], op in [] */
	case SI::InstFormatEXP:

		bytes.exp.enc = 0x3e;
		/* No opcode: only 1 instruction */
		break;

	default:
		panic("%s: unsupported format",
				__FUNCTION__);
	}

	/* Arguments */
	assert(args.size() == info->getNumTokens());
	for (unsigned i = 0; i < args.size(); i++)
	{
		/* Get argument and token */
		Arg *arg = args[i].get();
		Token *token = info->getToken(i);
		assert(token);
		EncodeArg(arg, token);
	}
}

}  /* namespace si2bin */
