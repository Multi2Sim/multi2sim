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

#include "FunctionUnit.h"

namespace x86
{

std::string FunctionUnit::name[TypeCount] =
{
		// Invalid
		"<invalid>",

		// Integer register operation
		"IntAdd",
		"IntMult",
		"IntDiv",

		// Logic operation
		"EffAddr",
		"Logic",

		// Floating point register operation
		"FloatSimple",
		"FloatAdd",
		"FloatComp",
		"FloatMult",
		"FloatDiv",
		"FloatComplex",

		// XMM register operation
		"XMMIntAdd",
		"XMMIntMult",
		"XMMIntDiv",
		"XMMLogic",
		"XMMFloatAdd",
		"XMMFloatComp",
		"XMMFloatMult",
		"XMMFloatDiv",
		"XMMFloatConv",
		"XMMFloatComplex"
};

FunctionUnit::ReservationPool FunctionUnit::reservation_pool[TypeCount] =
{
		{ 0, 0, 0 },  // Unused

		{ 3, 1, 1 },  // IntAdd
		{ 1, 3, 1 },  // IntMult
		{ 1, 14, 11 },  // IntDiv

		{ 3, 2, 2 },  // Effaddr
		{ 3, 1, 1 },  // Logic

		{ 1, 2, 1 },  // FloatSimple
		{ 1, 3, 1 },  // FloatAdd
		{ 1, 3, 1 },  // FloatComp
		{ 1, 5, 1 },  // FloatMult
		{ 1, 12, 5 },  // FloatDiv
		{ 1, 22, 14 },  // FloatComplex

		{ 1, 1, 1 },  // XmmIntAdd
		{ 1, 3, 1 },  // XmmIntMult
		{ 1, 14, 11 },  // XmmIntDiv

		{ 1, 1, 1 },  // XmmLogic

		{ 1, 3, 1 },  // XmmFloatAdd
		{ 1, 3, 1 },  // XmmFloatComp
		{ 1, 5, 1 },  // XmmFloatMult
		{ 1, 12, 6 },  // XmmFloatDiv
		{ 1, 3, 1 },  // XmmFloatConv
		{ 1, 22, 14 }  // XmmFloatComplex
};

FunctionUnit::Type FunctionUnit::type_table[UInstOpcodeCount] =
{
		TypeNone,  // UInstNop

		TypeNone,  // UInstMove
		TypeIntAdd,  // UInstAdd
		TypeIntAdd,  // UInstSub
		TypeIntMult,  // UInstMult
		TypeIntDiv,  // UInstDiv
		TypeEffaddr,  // UInstEffaddr

		TypeLogic,  // UInstAnd
		TypeLogic,  // UInstOr
		TypeLogic,  // UInstXor
		TypeLogic,  // UInstNot
		TypeLogic,  // UInstShift
		TypeLogic,  // UInstSign

		TypeNone,  // UInstFpMove
		TypeFloatSimple,  // UInstFpSign
		TypeFloatSimple,  // UInstFpRound

		TypeFloatAdd,  // UInstFpAdd
		TypeFloatAdd,  // UInstFpSub
		TypeFloatComp,  // UInstFpComp
		TypeFloatMult,  // UInstFpMult
		TypeFloatDiv,  // UInstFpDiv

		TypeFloatComplex,  // UInstFpExp
		TypeFloatComplex,  // UInstFpLog
		TypeFloatComplex,  // UInstFpSin
		TypeFloatComplex,  // UInstFpCos
		TypeFloatComplex,  // UInstFpSincos
		TypeFloatComplex,  // UInstFpTan
		TypeFloatComplex,  // UInstFpAtan
		TypeFloatComplex,  // UInstFpSqrt
};

}
