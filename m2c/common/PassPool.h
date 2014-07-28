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

#ifndef M2C_COMMON_PASS_POOL_H
#define M2C_COMMON_PASS_POOL_H

#include <memory>
#include <unordered_map>

#include "Pass.h"


namespace comm
{

/// Singleton containing a collection of the passes to run for
/// this incarnation of the compiler.
class PassPool
{

	// Static pointer to the only instance of PassPool.
	static std::unique_ptr<PassPool> instance;

	// The collection of Pass objects.
	std::unordered_map<int, Pass> passes;

	// Private constructor for singleton.
	PassPool() { }

public:

	/// Obtain instance of singleton.
	static PassPool *getInstance();

	/// Regiser a pass, inserting it into the PassPool.
	void registerPass(std::unique_ptr<Pass> &&pass);

	/// Return a pointer to a Pass given it's pass_id,
	/// if no Pass with that pass_id exists returns NULL.
	Pass *getPass(int pass_id);

	/// Run all of the passes.
	void runAll();

};


}  // namespace comm

#endif

