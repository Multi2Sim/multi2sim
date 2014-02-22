/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef LIB_CPP_ESIM_H
#define LIB_CPP_ESIM_H

#include <memory>

#include <lib/cpp/String.h>


namespace esim
{

extern misc::StringMap esim_finish_map;

enum ESimFinish
{
	ESimFinishNone,
	ESimFinishCtx,

	ESimFinishX86LastInst,
	ESimFinishX86MaxInst,
	ESimFinishX86MaxCycles,

	ESimFinishArmMaxInst,
	ESimFinishArmMaxCycles,
	ESimFinishArmLastInst,

	ESimFinishMipsMaxInst,
	ESimFinishMipsMaxCycles,
	ESimFinishMipsLastInst,

	ESimFinishEvgMaxInst,
	ESimFinishEvgMaxCycles,
	ESimFinishEvgMaxKernels,
	ESimFinishEvgNoFaults,

	ESimFinishFrmMaxInst,
	ESimFinishFrmMaxCycles,
	ESimFinishFrmMaxFunctions,

	ESimFinishSiMaxInst,
	ESimFinishSiMaxCycles,
	ESimFinishSiMaxKernels,

	ESimFinishMaxTime,
	ESimFinishSignal,
	ESimFinishStall
};



/// Event-driven simulator engine
class ESim
{
	// Unique instance of this class
	static std::unique_ptr<ESim> instance;

	// Reason to finish, or ESimFinishNone if not finished
	ESimFinish finish;

	// Private constructor, used internally to instantiate a singleton. Use
	// a call to getInstance() instead.
	ESim();

public:

	/// Obtain the instance of the event-driven simulator singleton.
	static ESim *getInstance();

	/// Force end of simulation with a specific reason.
	void Finish(ESimFinish why) { finish = why; }

	/// Return whether the simulation finished
	bool hasFinished() { return finish; }
};


}  // namespace esim

#endif

