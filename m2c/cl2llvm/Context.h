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

#ifndef M2C_CL2LLVM_CONTEXT_H
#define M2C_CL2LLVM_CONTEXT_H

#include <iostream>
#include <memory>

#include <lib/cpp/Error.h>


namespace cl2llvm
{


/// Exception used for the OpenCL front-end
class Error : public misc::Error
{
public:

	Error(const std::string &message) : misc::Error(message)
	{
		AppendPrefix("OpenCL front-end");
	}
};


/// Singleton containing global information for the OpenCL front-end
class Context
{
	// Flag indicating whether the user activated the stand-alone OpenCL
	// front-end
	static bool active;

	// Optimization level set by the user
	static int optimization_level;

	// Singleton instance
	static std::unique_ptr<Context> instance;

public:

	/// Compile one file
	void Parse(const std::string &in, const std::string &out);

	/// Get instance of singleton
	static Context *getInstance();

	/// Return whether the user activated the stand-alone OpenCL front-end
	static bool isActive() { return active; }

	/// Register command-line options
	static void RegisterOptions();

	/// Process command-line options
	static void ProcessOptions();

};


}  // namespace llvm2si


#endif

