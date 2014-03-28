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

#ifndef DRIVER_COMMON_SI_H
#define DRIVER_COMMON_SI_H

#include <memory>
#include <vector>
#include "Driver.h"

// Forward declarations
namespace SI
{
	class Emu;
	class Gpu;
	class NDRange;
}

namespace Driver
{

class SICommon : public Common
{
protected:
	// Device functional emulator
	SI::Emu *si_emu;

	// Device timing simulator
	SI::Gpu *si_gpu;

	// CPU/GPU fused device
	bool fused;

	// NDRange list contains NDRanges created by driver, shared by OpenCL/GL driver
	static std::vector<std::unique_ptr<SI::NDRange>> ndranges;

public:
	SICommon();

	static std::vector<std::unique_ptr<SI::NDRange>>::iterator 
		getNDRangeBegin() { return ndranges.begin(); }

	static std::vector<std::unique_ptr<SI::NDRange>>::iterator 
		getNDRangeEnd() { return ndranges.end(); }

	/// Get NDRange count
	static bool isNDRangeListEmpty() { return ndranges.empty(); }

	/// Get NDRange by id
	SI::NDRange *getNDRangeById(int id);

	/// Get fused information
	bool isFused() const { return fused; }

	/// Push NDRange to repository, NDRange comes fom Driver(Vertex Shader) or SPI 
	/// module(Pixel Shader). After added to repository, Driver/SPI modules lose
	/// ownership of NDRange instances.
	///
	/// \param ndrange NDRange instance created by Driver/SPI
	void AddNDRange(std::unique_ptr<SI::NDRange> ndrange);
};

}  // namespace Driver

#endif