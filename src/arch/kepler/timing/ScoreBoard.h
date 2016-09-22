/*
 *  Multi2Sim
 *  Copyright (C) 2015  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_KEPLER_TIMING_SCOREBOARD_H
#define ARCH_KEPLER_TIMING_SCOREBOARD_H

#include <vector>
#include <set>


namespace Kepler
{

// Forward declaration
class Instruction;
class SM;
class Warp;
class Uop;

// Class representing the score board in an SM
class ScoreBoard
{
	// Keep track of pending writes to registers.
	std::vector<std::set<unsigned>> register_table;

	// Keep track of pending writes to predicate registers
	std::vector<std::set<unsigned>> predicate_table;

	// Keep track of long operation pending writes to registers such as global
	// store
	std::vector<std::set<unsigned>> long_operation_register_table;

	// SM that it belongs to, assigned in constructor
	SM *sm;

	// Scoreboard in SM
	int id;

public:

	/// Constructor
	ScoreBoard(int id, SM *sm);

	/// Reserve the given register in the given warp
	void ReserveRegister(Warp *warp, unsigned register_index);

	/// Reserve registers destination registers in the given instruction which
	/// belongs to the given warp
	void ReserveRegisters(Warp *warp, Uop *uop);

	/// Reserve the predicate register in the given warp
	void ReservePredicate(Warp *warp, unsigned register_index);

	/// Release the given register in the given warp
	void ReleaseRegister(Warp *warp, unsigned register_index);

	/// Release the given predicate in the given warp
	void ReleasePredicate(Warp *warp, unsigned predicate_index);

	/// Release registers used in the given instruction which belongs to the
	/// given warp
	void ReleaseRegisters(Warp *warp, Uop *uop);

	/// Check if the given instruction has collision with the score board
	/// register table. Return true if WAW or RAW hazard detected.(no WAR
	/// since in-order issue)
	bool CheckCollision(Warp *warp, Uop *uop);
};

} // namespace Kepler

#endif
