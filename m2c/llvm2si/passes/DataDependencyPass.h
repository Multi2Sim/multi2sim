/*
 *  Multi2Sim
 *  Copyright (C) 2014  Nathan Lilienthal (nathan@nixpulvis.com)
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

#ifndef M2C_LLVM2SI_PASSES_DATA_DEPENDENCY_PASS_H
#define M2C_LLVM2SI_PASSES_DATA_DEPENDENCY_PASS_H

#include <iostream>
#include <unordered_map>

#include <m2c/common/Pass.h>
#include <m2c/common/PassInfo.h>


namespace llvm2si
{

/// Function specific data for the data dependency pass.
/// TODO: Elaborate.
class FunctionDataDependencyPassInfo : public comm::FunctionPassInfo
{
	// TODO: remove this.
	bool foo;
};

/// Data dependecy graph creation.
/// TODO: Elaborate.
class DataDependencyPass : public comm::Pass
{

	// Vector register type map.
	// TODO: refine this datatype. (don't use bool)
	std::unordered_map<int, bool> vector_register_map;

	// Scalar register type map.
	// TODO: refine this datatype. (don't use bool)
	std::unordered_map<int, bool> scalar_register_map;

public:
	
	// Run this pass.
	void run();

};


}  // namespace llvm2si


#endif

