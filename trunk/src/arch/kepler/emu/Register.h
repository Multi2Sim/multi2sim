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


namespace Kepler
{

/// GPR value
union RegValue
{
	unsigned u32;
	int s32;
	float f;
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
	bool cc;

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
	unsigned ReadSR(int sr_id) { return sr[sr_id].u32; };

	/// Set value of a SR
	/// \param gpr SR identifier
	/// \param value Value given as an \a unsigned typed value
	void WriteSR(int sr_id, unsigned value) { sr[sr_id].u32 = value; };

	/// Get value of a predicate register
	/// \param pr Predicate register identifier
	int ReadPred(int pr_id) { return pr[pr_id]; };

	/// Write value of a predicate register
	/// \param pr predicate register identifier
	void WritePred(int pr_id, unsigned value) { pr[pr_id] = value; };

	/// Read value of Condition Code register
	bool ReadCC() { return cc; };

	/// Write value of Condition register
	void WriteCC(bool value) { cc = value; };
};

}        // namespace

#endif
