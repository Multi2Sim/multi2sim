/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yuqing Shi (shi.yuq@husky.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ARCH_KEPLER_EMU_REGISTER_H
#define ARCH_KEPLER_EMU_REGISTER_H

#include<cstring>


namespace Kepler
{

/// GPR value
union RegValue
{
	unsigned u32;
	int s32;
	float f;
};

struct CC
{
	unsigned zf;
	unsigned sf;
	unsigned cf;
	unsigned of;
};

enum RegValueType
{
	RegValueTypeU32 = 0,
	RegValueTypeS32 =1,
	RegValueTypeF
};


// This class includes all kinds of registers used in Thread
class Register
{

private:

	RegValue gpr[256];  /* General purpose registers */
	RegValue sr[82];  /* Special registers */
	unsigned pr[8];  /* Predicate registers */
	CC cc;

public:

	/// Get value of a GPR
	/// \param vreg GPR identifier
	unsigned ReadGPR(int gpr_id) const { return gpr[gpr_id].u32; }

	/// Get float type value of a GPR
	/// \param vreg GPR identifier
	float ReadFloatGPR(int gpr_id) const { return gpr[gpr_id].f; }

	/// Set value of a GPR
	/// \param gpr GPR idenfifier
	/// \param value Value given as an \a unsigned typed value
	void WriteGPR(int gpr_id, unsigned value)
	{
		gpr[gpr_id].u32 = value;
	}

	/// Set float value of a GPR
	/// \param gpr GPR idenfifier
	/// \param value Value given as an \a float typed value
	void WriteFloatGPR(int gpr_id, float value)
	{
		gpr[gpr_id].f = value;
	}

	/// Get value of a SR
	/// \param vreg SR identifier
	unsigned ReadSpecialRegister(int special_register_id)
	{
		return sr[special_register_id].u32;
	}

	/// Set value of a SR
	/// \param gpr SR identifier
	/// \param value Value given as an \a unsigned typed value
	void WriteSpecialRegister(int special_register_id, unsigned value)
	{
		sr[special_register_id].u32 = value;
	}

	/// Get value of a predicate register
	/// \param pr Predicate register identifier
	int ReadPredicate(int predicate_id) { return pr[predicate_id]; };

	/// Write value of a predicate register
	/// \param pr predicate register identifier
	void WritePredicate(int predicate_id, unsigned value)
	{
		pr[predicate_id] = value;
	};

	/// Read value of Condition Code register
	unsigned ReadCC_ZF() { return cc.zf; };

	/// Read value of Condition Code register
	unsigned ReadCC_SF() { return cc.sf; };

	/// Read value of Condition Code register
	unsigned ReadCC_CF() { return cc.cf; };

	/// Read value of Condition Code register
	unsigned ReadCC_OF() { return cc.of; };

	/// Write value of Condition register
	void WriteCC_ZF(unsigned value) { cc.zf = value; };

	/// Write value of Condition register
	void WriteCC_SF(unsigned value) { cc.sf = value; };

	/// Write value of Condition register
	void WriteCC_CF(unsigned value) { cc.cf = value; };

	/// Write value of Condition register
	void WriteCC_OF(unsigned value) { cc.of = value; };

	/// Read value of register
	void Read_register(unsigned *dst, int gpr_id)
	{
		memcpy(dst, &gpr[gpr_id], 4);
	}

	/// Write to register
	void Write_register(unsigned *src, int gpr_id)
	{
		memcpy(&gpr[gpr_id], src, 4);
	}
};

}        // namespace

#endif
