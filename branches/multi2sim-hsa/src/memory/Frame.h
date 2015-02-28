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

#ifndef MEMORY_FRAME_H
#define MEMORY_FRAME_H

#include <list>
#include <memory>

#include <lib/esim/Event.h>


namespace mem
{

// Forward declarations
class Module;


/// Event frame for memory system events.
class Frame : public esim::EventFrame
{
	// Counter for identifiers
	static long long id_counter;

	// Unique identifier
	long long id;

	// Current module
	Module *module;

	// Physical address
	unsigned address;

	// Block coordinates
	unsigned set_id = -1;
	unsigned way_id = -1;

	// Decoded tag from address
	unsigned tag = -1;

	// List of dependent waiting frames
	std::list<std::shared_ptr<Frame>> waiting_frames;

	// Event to be invoked when the frame wakes up from a waiting list
	esim::EventType *wakeup_event_type = nullptr;

public:

	/// Constructor
	Frame(long long id, Module *module, unsigned address) :
			id(id),
			module(module),
			address(address)
	{
	}

	/// Add a dependent frame to the list of waiting frames. Dependent
	/// frames are woken up when the access represented with this frame
	/// completes.
	///
	/// \param frame
	///	Shared pointer of the dependent frame.
	///
	/// \param event_type
	///	Event to schedule upon wakeup
	void AddWaitingFrame(std::shared_ptr<Frame> frame,
			esim::EventType *event_type);

	/// Wake up all dependent frames in the list of waiting frames.
	void WakeupWaitingFrames();
};


}  // namespace mem

#endif

