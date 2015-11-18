/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#include <lib/cpp/Misc.h>
#include <memory/Memory.h>

#include "../../../../runtime/include/hsa.h"
#include "../../../../runtime/include/hsa_ext_finalize.h"
#include "SignalDestroyHandler.h"
#include "SignalManager.h"

namespace HSA
{

SignalDestroyHandler::SignalDestroyHandler(
		SignalManager *signal_manager) :
		signal_manager(signal_manager)
{
}


SignalDestroyHandler::~SignalDestroyHandler()
{
	// TODO Auto-generated destructor stub
}


void SignalDestroyHandler::Process(mem::Memory *memory, uint32_t args_ptr)
{
	// Retrieve data
	auto data = misc::new_unique<Data>();
	memory->Read(args_ptr, sizeof(Data), (char *)data.get());

	// Destory signal
	signal_manager->DestorySignal(data->signal);

	// Return hsa_status_t
	data->status = HSA_STATUS_SUCCESS;

	// Write return value back
	memory->Write(args_ptr, sizeof(Data), (char *)data.get());
}

}  // namespace HSA
