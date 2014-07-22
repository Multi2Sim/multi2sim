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

#ifndef M2C_COMMON_PASS_H
#define M2C_COMMON_PASS_H

namespace comm
{

/// Abstract class to be inherited from when building a compiler pass.
/// Helps encapsulate data for each pass, and allow passes to share their
/// data.
class Pass
{

	// Counter of passes, preincremented on allocation of a Pass,
	// and assigned to private member `id`.
	static int id_counter;

	// The id of the Pass instance. Used to access data from
	// comm::PassInfoPool.
	int id;

public:

	/// Constructor of a pass.
	Pass();

	/// Return the unique identifier for this pass (starting at 1).
	int getId() const { return id; }

	/// This function MUST be implemented in all subclasses of Pass,
	/// defining the logic of the pass.
	virtual void run() = 0;

};

}  // namespace comm

#endif
