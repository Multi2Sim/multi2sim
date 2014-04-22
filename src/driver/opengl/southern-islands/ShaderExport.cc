/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#include "ShaderExport.h"

namespace SI
{

std::unique_ptr<ShaderExport> ShaderExport::instance;

ExportData::ExportData(float x, float y, float z, float w)
{
	this->x.f = x;
	this->y.f = y;
	this->z.f = z;
	this->w.f = w;
}

ExportData::ExportData(unsigned x, unsigned y, unsigned z, unsigned w)
{
	this->x.u = x;
	this->y.u = y;
	this->z.u = z;
	this->w.u = w;
}

ShaderExport::ShaderExport()
{

}

ShaderExport *ShaderExport::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();
	
	// Create instance
	instance.reset(new ShaderExport());
	return instance.get();	
}

uint16_t ShaderExport::Float32to16(float value)
{
	union Bits v, s;
	v.f = value;
	uint32_t sign = v.si & F_signN;
	v.si ^= sign;
	sign >>= F_shiftSign; // logical F_shift
	s.si = F_mulN;
	s.si = s.f * v.f; // correct subnormals
	v.si ^= (s.si ^ v.si) & -(F_minN > v.si);
	v.si ^= (F_infN ^ v.si) & -((F_infN > v.si) & (v.si > F_maxN));
	v.si ^= (F_nanN ^ v.si) & -((F_nanN > v.si) & (v.si > F_infN));
	v.ui >>= F_shift; // logical F_shift
	v.si ^= ((v.si - F_maxD) ^ v.si) & -(v.si > F_maxC);
	v.si ^= ((v.si - F_minD) ^ v.si) & -(v.si > F_subC);
	return v.ui | sign;
}
	
float ShaderExport::Float16to32(uint16_t value)
{
	union Bits v;
	v.ui = value;
	int32_t sign = v.si & F_signC;
	v.si ^= sign;
	sign <<= F_shiftSign;
	v.si ^= ((v.si + F_minD) ^ v.si) & -(v.si > F_subC);
	v.si ^= ((v.si + F_maxD) ^ v.si) & -(v.si > F_maxC);
	union Bits s;
	s.si = F_mulC;
	s.f *= v.si;
	int32_t mask = -(F_norC > v.si);
	v.si <<= F_shift;
	v.si ^= (s.si ^ v.si) & mask;
	v.si |= sign;
	return v.f;	
}

std::vector<std::unique_ptr<ExportData>> &ShaderExport::getExportTarget(unsigned target)
{
	if (target >= 0 && target <= 7)
		return mrt[target];
	else if (target == 9)
		return z;
	else if (target >= 12 && target <= 15)
		return pos[target - 12];
	else if (target >= 32 && target <= 63)
		return param[target - 32];

	// Return
	return mrt[0];
}

void ShaderExport::Export(unsigned target, unsigned index, 
	unsigned x, unsigned y, unsigned z, unsigned w)
{
	if (target >= 0 && target <= 7)
	{
		mrt[target].insert(mrt[target].begin() + index, 
			(std::unique_ptr<ExportData>)new ExportData(x, y, z, w));
		return;
	}
	else if (target == 9)
	{
		this->z.insert(this->z.begin() + index, 
			(std::unique_ptr<ExportData>)new ExportData(x, y, z, w));
		return;
	}
	else if (target >= 12 && target <= 15)
	{
		pos[target - 12].insert(pos[target - 12].begin() + index, 
			(std::unique_ptr<ExportData>)new ExportData(x, y, z, w));
		return;
	}
	else if (target >= 32 && target <= 63)
	{
		param[target - 32].insert(param[target - 32].begin() + index, 
			(std::unique_ptr<ExportData>)new ExportData(x, y, z, w));
	}

	// Return
	return;
}

void ShaderExport::Export(unsigned target, unsigned index, 
	float x, float y, float z, float w)
{
	if (target >= 0 && target <= 7)
	{
		mrt[target].insert(mrt[target].begin() + index, 
			(std::unique_ptr<ExportData>)new ExportData(x, y, z, w));
		return;
	}
	else if (target == 9)
	{
		this->z.insert(this->z.begin() + index, 
			(std::unique_ptr<ExportData>)new ExportData(x, y, z, w));
		return;
	}
	else if (target >= 12 && target <= 15)
	{
		pos[target - 12].insert(pos[target - 12].begin() + index, 
			(std::unique_ptr<ExportData>)new ExportData(x, y, z, w));
		return;
	}
	else if (target >= 32 && target <= 63)
	{
		param[target - 32].insert(param[target - 32].begin() + index, 
			(std::unique_ptr<ExportData>)new ExportData(x, y, z, w));
	}

	// Return
	return;
}

void ShaderExport::Clear(unsigned target)
{
	if (target >= 0 && target <= 7)
	{
		mrt[target].clear();
		return;
	}
	else if (target == 9)
	{
		this->z.clear();
		return;
	}
	else if (target >= 12 && target <= 15)
	{
		pos[target - 12].clear();
		return;
	}
	else if (target >= 32 && target <= 63)
	{
		param[target - 32].clear();
		return;
	}

	// Return
	return;

}

void ShaderExport::ClearAll()
{
	for (int i = 0; i < MRT_COUNT; ++i)
		mrt[i].clear();

	this->z.clear();

	for (int i = 0; i < POS_COUNT; ++i)
		pos[i].clear();

	for (int i = 0; i < PARAM_COUNT; ++i)
		param[i].clear();

	// Return
	return;
}

}  // namespace SI
