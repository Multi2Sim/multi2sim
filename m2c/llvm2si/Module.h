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

#ifndef M2C_LLVM2SI_MODULE_H
#define M2C_LLVM2SI_MODULE_H

#include <iostream>
#include <memory>
#include <list>

#include <llvm/IR/Module.h>
#include <m2c/common/Module.h>

#include "Function.h"


namespace llvm2si
{

class Module : public comm::Module
{
	// Associated LLVM module
	llvm::Module *llvm_module;

	// List of functions belonging to the module
	std::vector<std::unique_ptr<Function>> functions;

public:

	/// Constructor
	///
	/// \param llvm_module
	///	Associated module in the LLVM code.
	explicit Module(llvm::Module *llvm_module) : llvm_module(llvm_module) {}

	/// Return the vector of functions of this module.
	std::vector<std::unique_ptr<Function>> *getFunctions() { return &functions; }

	/// Create a function that belongs to the module, and return a
	/// pointer to it. The new function will be internally freed when the
	/// module object is destroyed.
	Function *newFunction(llvm::Function *llvm_function)
	{
		functions.emplace_back(new Function(this, llvm_function));
		return functions.back().get();
	}
};

}  // namespace llvm2si

#endif

