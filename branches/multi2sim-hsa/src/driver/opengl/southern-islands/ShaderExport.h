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

#ifndef DRIVER_OPENGL_SI_SHADER_EXPORT_H
#define DRIVER_OPENGL_SI_SHADER_EXPORT_H

#include <memory>
#include <vector>
#include <cstdint>

namespace SI
{

#define F_shift 13
#define F_shiftSign 16

#define F_infN 0x7F800000 // flt32 infinity
#define F_maxN 0x477FE000 // max flt16 normal as a flt32
#define F_minN 0x38800000 // min flt16 normal as a flt32
#define F_signN 0x80000000 // flt32 sign bit

#define F_infC (F_infN >> F_shift)
#define F_nanN ((F_infC + 1) << F_shift) // minimum flt16 nan as a flt32
#define F_maxC (F_maxN >> F_shift)
#define F_minC (F_minN >> F_shift)
#define F_signC (F_signN >> F_shiftSign) // flt16 sign bit

#define F_mulN 0x52000000 // (1 << 23) / F_minN
#define F_mulC 0x33800000 // F_minN / (1 << (23 - F_shift))

#define F_subC 0x003FF // max flt32 subnormal down shifted
#define F_norC 0x00400 // min flt32 normal down shifted

#define F_maxD (F_infC - F_maxC - 1)
#define F_minD (F_minC - F_subC - 1)

union Bits
{
	float f;
	int32_t si;
	uint32_t ui;
};

// Halfpack
union hfpack
{
	uint32_t as_uint32;
	struct
	{
		uint16_t s1f;
		uint16_t s0f;
	} as_f16f16;
};

class ExportData
{
	union Element
	{
		float f;
		unsigned u;
	};

	Element x;
	Element y;
	Element z;
	Element w;

public:
	ExportData(float x, float y, float z, float w);
	ExportData(unsigned x, unsigned y, unsigned z, unsigned w);

	/// Getters
	float getXAsFloat() const { return x.f; }
	float getYAsFloat() const { return y.f; }
	float getZAsFloat() const { return z.f; }
	float getWAsFloat() const { return w.f; }

	float getXAsUInt() const { return x.u; }
	float getYAsUInt() const { return y.u; }
	float getZAsUInt() const { return z.u; }
	float getWAsUInt() const { return w.u; }
};

class ShaderExport
{
	// Unique instance of ShaderExport
	static std::unique_ptr<ShaderExport> instance;

	// FIXME: move export targets to global memory and
	// use resource table in Emu to store the locations

	// Number of MRTs (Multiple Rendering Target)
	static const int MRT_COUNT = 8;

	// Number of Position Buffers
	static const int POS_COUNT = 4;

	// Number of Parameter Buffers
	static const int PARAM_COUNT = 32;

	// Export Target: MRTs
	std::vector<std::unique_ptr<ExportData>> mrt[MRT_COUNT];

	// Export Target: Z Buffer
	std::vector<std::unique_ptr<ExportData>> z;

	// Export Target: Position Buffers
	std::vector<std::unique_ptr<ExportData>> pos[POS_COUNT];

	// Export Target: Parameter Buffers
	std::vector<std::unique_ptr<ExportData>> param[POS_COUNT];

	// Private constructor. The only possible instance of the ShaderExport
	// can be obtained with a call to getInstance()
	ShaderExport();

	// Convert Float 32 to Float 16
	uint16_t Float32to16(float value);
	
	// Convert Float 16 to Float 32
	float Float16to32(uint16_t value);

public:
	static ShaderExport *getInstance();

	std::vector<std::unique_ptr<ExportData>> &getExportTarget(unsigned target);

	void Export(unsigned target, unsigned index, unsigned x, unsigned y, unsigned z, unsigned w);

	void Export(unsigned target, unsigned index, float x, float y, float z, float w);

	void Clear(unsigned target);

	void ClearAll();
};

} // namespace SI

#endif
