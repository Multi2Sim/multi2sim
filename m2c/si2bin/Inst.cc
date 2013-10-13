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
#include "Builder.h"
#include "Inst.h"
#include "Token.h"


using namespace Misc;

namespace si2bin
{

Inst::Inst(SI::InstOpcode opcode, std::list<Arg *> &args)
{
	/* Initialize */
	size = 0;
	bytes.dword = 0;

	/* Copy argument list */
	int arg_index = 0;
	for (auto arg : args)
	{
		this->args.emplace_back(arg);
		arg->index = arg_index++;
	}

	/* Check opcode */
	this->opcode = opcode;
	if (!InRange(opcode, 1, SI::InstOpcodeCount - 1))
		fatal("%s: invalid opcode (%d)", __FUNCTION__, opcode);

	/* Get instruction information */
	InstInfo *info = builder.GetInstInfo(opcode);
	if (!info)
		fatal("%s: opcode %d not supported", __FUNCTION__, opcode);

	/* Check number of arguments */
	if (args.size() != info->tokens.size())
		fatal("%s: invalid number of arguments (%d given, %d expected)",
				__FUNCTION__, (int) args.size(),
				(int) info->tokens.size());

	/* Check argument types */
	auto tokens_iterator = info->tokens.begin();
	for (auto arg : args)
	{
		/* Get formal argument from instruction info. Associate token with the
		 * instruction argument. */
		Token *token = tokens_iterator->get();
		arg->token = token;
		assert(token);

		/* Check that actual argument type is acceptable for token */
		if (!token->IsArgAllowed(arg))
			fatal("%s: invalid type for argument %d", __FUNCTION__,
					arg->index + 1);

		/* Next */
		++tokens_iterator;
	}
}


Inst::Inst(std::string name, std::list<Arg *> &args)
{
	/* Initialize */
	size = 0;
	bytes.dword = 0;
	
	/* Copy argument list */
	int arg_index = 0;
	for (auto arg : args)
	{
		this->args.emplace_back(arg);
		arg->index = arg_index++;
	}
	
	/* Try to create the instruction following all possible encodings for
	 * the same instruction name. */
	std::string error = "invalid instruction: " + name;
	for (InstInfo *info = builder.GetInstInfo(name); info; info = info->next)
	{
		/* Check number of arguments */
		if (args.size() != info->tokens.size())
		{
			error = StringFormat("invalid number of arguments for %s "
					"(%d given, %d expected)",
					name.c_str(), (int) args.size(),
					(int) info->tokens.size() - 1);
			continue;
		}

		/* Check arguments */
		error = "";
		auto token_iterator = info->tokens.begin();
		for (auto arg : args)
		{
			/* Get formal argument from instruction info. We associate the
			 * instruction argument with the token. */
			Token *token = token_iterator->get();
			arg->token = token;
			assert(token);

			/* Check that actual argument type is acceptable for token */
			if (!token->IsArgAllowed(arg))
			{
				error = StringFormat("invalid type for argument %d",
						arg->index + 1);
				break;
			}

			/* Next */
			++token_iterator;
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
	this->info = info;
	this->opcode = info->info->opcode;
}


void Inst::Dump(std::ostream &os)
{
        /* Comment attached to the instruction */
        if (!comment.empty())
        	os << "\n\t# " << comment << '\n';

        /* Dump instruction opcode */
        os << '\t' << info->name << ' ';

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


void Inst::Encode()
{
	/* By default, the instruction has the number of bytes specified by its
	 * format. 4-bit instructions could be extended later to 8 bits upon
	 * the presence of a literal constant. */
	assert(info && info->info);
	size = info->info->size;

	/* Instruction opcode */
	switch (info->info->fmt)
	{

	/* encoding in [31:26], op in [18:16] */
	case SI::InstFormatMTBUF:

		bytes.mtbuf.enc = 0x3a;
		bytes.mtbuf.op = info->info->op;
		break;
	
	/* encoding in [:], op in [] */
	case SI::InstFormatMUBUF:
		
		bytes.mubuf.enc = 0x38;
		bytes.mubuf.op = info->info->op;
		break;

	/* encoding in [:], op in [] */
	case SI::InstFormatMIMG:
		
		bytes.mimg.enc = 0x3c;
		bytes.mimg.op = info->info->op;
		break;

	/* encoding in [31:27], op in [26:22] */
	case SI::InstFormatSMRD:

		bytes.smrd.enc = 0x18;
		bytes.smrd.op = info->info->op;
		break;
	
	/* encoding in [31:26], op in [25:28] */
	case SI::InstFormatDS:
		
		bytes.ds.enc = 0x36;
		bytes.ds.op = info->info->op;
		break;

	/* encoding in [31:23], op in [22:16] */
	case SI::InstFormatSOPC:
		
		bytes.sopc.enc = 0x17e;
		bytes.sopc.op = info->info->op;
		break;

	/* encoding in [31:23], op in [15:8] */
	case SI::InstFormatSOP1:

		bytes.sop1.enc = 0x17d;
		bytes.sop1.op = info->info->op;
		break;

	/* encoding in [31:30], op in [29:23] */
	case SI::InstFormatSOP2:

		bytes.sop2.enc = 0x2;
		bytes.sop2.op = info->info->op;
		break;

	/* encoding in [31:23], op in [22:16] */
	case SI::InstFormatSOPP:

		bytes.sopp.enc = 0x17f;
		bytes.sopp.op = info->info->op;
		break;
	
	/* encoding in [:], op in [] */
	case SI::InstFormatSOPK:
		
		bytes.sopk.enc = 0xb;
		bytes.sopk.op = info->info->op;
		break;

	/* encoding in [:], op in [] */
	case SI::InstFormatVOPC:
		
		bytes.vopc.enc = 0x3e;
		bytes.vopc.op = info->info->op;
		break;

	/* encoding in [31:25], op in [16:9] */
	case SI::InstFormatVOP1:

		bytes.vop1.enc = 0x3f;
		bytes.vop1.op = info->info->op;
		break;

	/* encoding in [31], op in [30:25] */
	case SI::InstFormatVOP2:

		bytes.vop2.enc = 0x0;
		bytes.vop2.op = info->info->op;
		break;

	/* encoding in [31:26], op in [25:17] */
	case SI::InstFormatVOP3a:

		bytes.vop3a.enc = 0x34;
		bytes.vop3a.op = info->info->op;
		break;

	/* encoding in [31:26], op in [25:17] */
	case SI::InstFormatVOP3b:

		bytes.vop3a.enc = 0x34;
		bytes.vop3a.op = info->info->op;
		break;

	/* encoding in [:], op in [] */
	case SI::InstFormatVINTRP:
		
		bytes.vintrp.enc = 0x32;
		bytes.vintrp.op = info->info->op;
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
	assert(args.size() == info->tokens.size());
	auto token_iterator = info->tokens.begin();
	for (auto &arg_ptr : args)
	{
		/* Get argument and token */
		Arg *arg = arg_ptr.get();
		Token *token = arg->token;
		assert(token);

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
				if (literal->GetValue() > 0xff00)
					fatal("%s: Literal in simm16 needs to fit in 16 bit field",
						__FUNCTION__);
				bytes.sopk.simm16 = literal->GetValue();
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
			if (srs && srs->GetHigh() != srs->GetLow() + 1)
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
				bytes.sop2.lit_cnst = literal->GetValue();
			}
			else
			{
				/* Check range of scalar registers */
				ArgScalarRegisterSeries *srs = dynamic_cast
						<ArgScalarRegisterSeries *>(arg);
				if (srs && srs->GetHigh() != srs->GetLow() + 1)
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
				bytes.sop2.lit_cnst = literal->GetValue();
			}
			else
			{
				/* Check range of scalar registers */
				ArgScalarRegisterSeries *srs = dynamic_cast
						<ArgScalarRegisterSeries *>(arg);
				if (srs && srs->GetHigh() != srs->GetLow() + 1)
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
			int soffset = maddr->GetSoffset()->Encode();
			if (!InRange(soffset, 0, 253))
				fatal("invalid offset");
			bytes.mtbuf.soffset = soffset;

			/* Data and number format */
			bytes.mtbuf.dfmt = maddr->GetDataFormat();
			bytes.mtbuf.nfmt = maddr->GetNumFormat();

			/* Qualifiers */
			ArgMaddrQual *qual = maddr->GetQual();
			bytes.mtbuf.offen = qual->GetOffen();
			bytes.mtbuf.idxen = qual->GetIdxen();
			bytes.mtbuf.offset = qual->GetOffset();

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
				low = vr->GetId();
				high = low;
				break;
			}

			case ArgTypeVectorRegisterSeries:
			{
				ArgVectorRegisterSeries *vrs = dynamic_cast
						<ArgVectorRegisterSeries *>(arg);
				low = vrs->GetLow();
				high = vrs->GetHigh();
				break;
			}

			default:
				panic("%s: invalid argument for TokenMtSeriesVdata",
						__FUNCTION__);
			}

			/* Restriction in vector register range */
			switch (opcode)
			{

			case SI_INST_TBUFFER_LOAD_FORMAT_X:
			case SI_INST_TBUFFER_STORE_FORMAT_X:

				high_must = low;
				break;

			case SI_INST_TBUFFER_LOAD_FORMAT_XY:
			case SI_INST_TBUFFER_STORE_FORMAT_XY:

				high_must = low + 1;
				break;

			case SI_INST_TBUFFER_LOAD_FORMAT_XYZW:
			case SI_INST_TBUFFER_STORE_FORMAT_XYZW:

				high_must = low + 3;
				break;

			default:
				panic("%s: MUBUF/MTBUF instruction not recognized: %s",
						__FUNCTION__, info->name.c_str());
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
				if (literal->GetValue() > 255)
					fatal("%s: offset needs to fit in 8 bit field",
						__FUNCTION__);
				
				bytes.smrd.imm = 1;
				bytes.smrd.offset = literal->GetValue();
				/* FIXME - check valid range of literal */
				break;
			}
			case ArgTypeScalarRegister:
			{
				ArgScalarRegister *sr = dynamic_cast
						<ArgScalarRegister *>(arg);
				assert(sr);
				bytes.smrd.offset = sr->GetId();
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
			if (srs->GetLow() % 2)
				fatal("base register must be multiple of 2");

			/* Restrictions for high register */
			switch (opcode)
			{

			case SI_INST_S_LOAD_DWORDX2:
			case SI_INST_S_LOAD_DWORDX4:

				/* High register must be low plus 1 */
				if (srs->GetHigh() != srs->GetLow() + 1)
					fatal("register series must be s[x:x+1]");
				break;

			case SI_INST_S_BUFFER_LOAD_DWORD:
			case SI_INST_S_BUFFER_LOAD_DWORDX2:
			case SI_INST_S_BUFFER_LOAD_DWORDX4:

				/* High register must be low plus 3 */
				if (srs->GetHigh() != srs->GetLow() + 3)
					fatal("register series must be s[x:x+3]");
				break;

			default:
				fatal("%s: unsupported opcode for TokenSeriesSbase: %s",
						__FUNCTION__, info->name.c_str());
			}

			/* Encode */
			bytes.smrd.sbase = srs->GetLow() / 2;
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

			case SI_INST_S_LOAD_DWORDX2:
			case SI_INST_S_BUFFER_LOAD_DWORDX2:

				/* High register must be low plus 1 */
				if (srs->GetHigh() != srs->GetLow() + 1)
					fatal("register series must be s[low:low+1]");
				break;

			case SI_INST_S_LOAD_DWORDX4:
			case SI_INST_S_BUFFER_LOAD_DWORDX4:

				/* High register must be low plus 3 */
				if (srs->GetHigh() != srs->GetLow() + 3)
					fatal("register series must be s[low:low+3]");
				break;

			default:
				fatal("%s: unsupported opcode for 'series_sdst' token: %s",
						__FUNCTION__, info->name.c_str());
			}

			/* Encode */
			bytes.smrd.sdst = srs->GetLow();
			break;
		}

		case TokenSeriesSrsrc:
		{
			/* Get argument */
			ArgScalarRegisterSeries *srs = dynamic_cast
					<ArgScalarRegisterSeries *>(arg);
			assert(srs);
			int low = srs->GetLow();
			int high = srs->GetHigh();

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
			bytes.smrd.sdst = sr->GetId();
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
				bytes.vopc.lit_cnst = literal->GetValue();
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
				bytes.sop2.lit_cnst = literal->GetValue();
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
				bytes.sop2.lit_cnst = literal->GetValue();
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
				bytes.mtbuf.vaddr = vr->GetId();
				break;
			}
			
			case ArgTypeVectorRegisterSeries:
			{
				/* High register must be low plus 1 */
				ArgVectorRegisterSeries *vrs = dynamic_cast
						<ArgVectorRegisterSeries *>(arg);
				if (vrs->GetHigh() != vrs->GetLow() + 1)
					fatal("register series must be v[x:x+1]");
				
				bytes.mtbuf.vaddr = vrs->GetLow();
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
			bytes.vop1.vdst = vr->GetId();
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
				low = srs->GetLow();
				high = srs->GetHigh();
				if (high != low + 1)
					fatal("register series must be s[low:low+1]");
				break;
			}

			case ArgTypeVectorRegisterSeries:
			{
				ArgVectorRegisterSeries *vrs = dynamic_cast
						<ArgVectorRegisterSeries *>(arg);
				assert(vrs);
				low = vrs->GetLow();
				high = vrs->GetHigh();
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
			int low = vrs->GetLow();
			int high = vrs->GetHigh();
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
				if (srs->GetHigh() != srs->GetLow() + 1)
					fatal("register series must be s[low:low+1]");
			}

			/* Encode */
			bytes.vop3a.vdst = arg->Encode();
			break;
		}
#if 0
		case TokenVop3Src0:

			bytes.vop3a.src0 = ArgTypeEncodeOperand(arg);
			if (arg->abs)
				bytes.vop3a.abs |= 0x1;
			if (arg->neg)
				bytes.vop3a.neg |= 0x1;
			break;

		case TokenVop3Src1:

			bytes.vop3a.src1 = ArgTypeEncodeOperand(arg);
			if (arg->abs)
				bytes.vop3a.abs |= 0x2;
			if (arg->neg)
				bytes.vop3a.neg |= 0x2;
			break;

		case TokenVop3Src2:

			bytes.vop3a.src2 = ArgTypeEncodeOperand(arg);
			if (arg->abs)
				bytes.vop3a.abs |= 0x4;
			if (arg->neg)
				bytes.vop3a.neg |= 0x4;
			break;

		case TokenVop364Src0:
		{
			
			int low;
			int high;

			/* If operand is a scalar register series, check range */
			if (arg->type == ArgTypeScalarRegisterSeries)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be s[low:low+1]");
			}
			else if (arg->type == ArgTypeVectorRegisterSeries)
			{
				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be v[low:low+1]");
			}

			/* Encode */
			bytes.vop3a.src0 = ArgTypeEncodeOperand(arg);
			
			if (arg->abs)
				bytes.vop3a.abs |= 0x1;
			if (arg->neg)
				bytes.vop3a.neg |= 0x1;
			break;
		}
		case TokenVop364Src1:
		{	
			int low;
			int high;

			/* If operand is a scalar register series, check range */
			if (arg->type == ArgTypeScalarRegisterSeries)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be s[low:low+1]");
			}
			else if (arg->type == ArgTypeVectorRegisterSeries)
			{
				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be v[low:low+1]");
			}

			/* Encode */
			bytes.vop3a.src1 = ArgTypeEncodeOperand(arg);
			
			if (arg->abs)
				bytes.vop3a.abs |= 0x2;
			if (arg->neg)
				bytes.vop3a.neg |= 0x2;
			break;
		}
		case TokenVop364Src2:
		{
			int low;
			int high;

			/* If operand is a scalar register series, check range */
			if (arg->type == ArgTypeScalarRegisterSeries)
			{
				low = arg->value.scalar_register_series.low;
				high = arg->value.scalar_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be s[low:low+1]");
			}
			else if (arg->type == ArgTypeVectorRegisterSeries)
			{
				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be v[low:low+1]");
			}

			/* Encode */
			bytes.vop3a.src2 = ArgTypeEncodeOperand(arg);
			
			if (arg->abs)
				bytes.vop3a.abs |= 0x4;
			if (arg->neg)
				bytes.vop3a.neg |= 0x4;
			break;
		}
		
		case TokenVop364Sdst:
			
			/* Encode */
			bytes.vop3b.sdst = ArgTypeEncodeOperand(arg);
			
			break;

		case TokenVop3Vdst:
			
			/* Encode */
			bytes.vop3a.vdst = arg->value.vector_register.id;
			break;

		case TokenVop364Vdst:
		{
			int low;
			int high;

			/* Check argument type */
			if (arg->type == ArgTypeVectorRegisterSeries)
			{
				low = arg->value.vector_register_series.low;
				high = arg->value.vector_register_series.high;
				if (high != low + 1)
					si2bin_yyerror("register series must be v[low:low+1]");
			}
			
			/* Encode */
			bytes.vop3a.vdst = arg->value.vector_register_series.low;
			break;
		}

		case TokenVsrc1:

			/* Make sure argument is a vector register */
			assert(arg->type == ArgTypeVectorRegister);

			/* Encode */
			bytes.vopc.vsrc1 = arg->value.vector_register.id;
			
			break;

		case TokenWaitCnt:
			/* vmcnt(x) */
			if (arg->value.wait_cnt.vmcnt_active)
			{
				if (!IN_RANGE(arg->value.wait_cnt.vmcnt_value, 0, 0xe))
					si2bin_yyerror("invalid value for vmcnt");
				bytes.sopp.simm16 = SET_BITS_32(bytes.sopp.simm16,
						3, 0, arg->value.wait_cnt.vmcnt_value);
			}
			else
			{
				bytes.sopp.simm16 = SET_BITS_32(bytes.sopp.simm16,
						3, 0, 0xf);
			}

			/* lgkmcnt(x) */
			if (arg->value.wait_cnt.lgkmcnt_active)
			{
				if (!IN_RANGE(arg->value.wait_cnt.lgkmcnt_value, 0, 0x1e))
					si2bin_yyerror("invalid value for lgkmcnt");
				bytes.sopp.simm16 = SET_BITS_32(bytes.sopp.simm16,
						12, 8, arg->value.wait_cnt.lgkmcnt_value);
			}
			else
			{
				bytes.sopp.simm16 = SET_BITS_32(bytes.sopp.simm16,
						12, 8, 0x1f);
			}

			/* expcnt(x) */
			if (arg->value.wait_cnt.expcnt_active)
			{
				if (!IN_RANGE(arg->value.wait_cnt.expcnt_value, 0, 0x6))
					si2bin_yyerror("invalid value for expcnt");
				bytes.sopp.simm16 = SET_BITS_32(bytes.sopp.simm16,
						6, 4, arg->value.wait_cnt.expcnt_value);
			}
			else
			{
				bytes.sopp.simm16 = SET_BITS_32(bytes.sopp.simm16,
						6, 4, 0x7);
			}
			break;

		case TokenAddr:
			
			/* Make sure argument is a vector register */
			assert(arg->type == ArgTypeVectorRegister);
			
			/* Encode */
			bytes.ds.addr = arg->value.vector_register.id;
			break;

		case TokenData0:

			/* Make sure argument is a vector register */
			assert(arg->type == ArgTypeVectorRegister);

			/* Encode */
			bytes.ds.data0 = arg->value.vector_register.id;
			break;

		case TokenDsVdst:

			/* Make sure argument is a vector register */
			assert(arg->type == ArgTypeVectorRegister);

			/* Encode */
			bytes.ds.vdst = arg->value.vector_register.id;
			break;
		
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

		/* Next */
		++token_iterator;
	}
}

}  /* namespace si2bin */
