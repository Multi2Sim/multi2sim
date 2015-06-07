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

#include "CPU.h"

namespace x86
{

//
// String maps
//

misc::StringMap CPU::recover_kind_map
{
	{"Writeback", RecoverKindWriteback},
	{"Commit", RecoverKindCommit}
};

misc::StringMap CPU::fetch_kind_map
{
	{"Shared", FetchKindShared},
	{"TimeSlice", FetchKindTimeslice},
	{"SwitchOnEvent", FetchKindSwitchonevent},
};

misc::StringMap CPU::dispatch_kind_map
{
	{"Shared", DispatchKindShared},
	{"TimeSlice", DispatchKindTimeslice},
};

misc::StringMap CPU::issue_kind_map
{
	{"Shared", IssueKindShared},
	{"TimeSlice", IssueKindTimeslice},
};

misc::StringMap CPU::commit_kind_map
{
	{"Shared", CommitKindShared},
	{"TimeSlice", CommitKindTimeslice},
};

misc::StringMap CPU::reorder_buffer_kind_map
{
	{"Private", ReorderBufferKindPrivate},
	{"Shared", ReorderBufferKindShared},
};

// static member
int CPU::num_cores = 1;
int CPU::num_threads = 1;
int CPU::context_quantum;
int CPU::thread_quantum;
int CPU::thread_switch_penalty;
int CPU::recover_penalty;
CPU::RecoverKind CPU::recover_kind;
CPU::FetchKind CPU::fetch_kind;
int CPU::decode_width;
int CPU::dispatch_width;
CPU::DispatchKind CPU::dispatch_kind;
int CPU::issue_width;
CPU::IssueKind CPU::issue_kind;
int CPU::commit_width;
CPU::CommitKind CPU::commit_kind;
bool CPU::process_prefetch_hints;
bool CPU::use_nc_store;
bool CPU::prefetch_history_size;
int CPU::occupancy_stats;
int CPU::reorder_buffer_size;
CPU::ReorderBufferKind CPU::reorder_buffer_kind;

CPU::CPU()
{
	// The prefix for each core
	std::string prefix = "Core";
	std::string core_name;

	// Initialize
	emu = Emu::getInstance();
	mmu = misc::new_unique<mem::MMU>("x86");

	// Create cores
	for (int i = 0; i < num_cores; i++)
	{
		core_name = prefix + misc::fmt("%d", i);
		cores.emplace_back(misc::new_unique<Core>(core_name, this, i));
	}
}


void CPU::ParseConfiguration(misc::IniFile *ini_file)
{
	// Local variable
	std::string section;

	// Section '[ General ]'
	section = "General";
	num_cores = ini_file->ReadInt(section, "Cores", num_cores);
	num_threads = ini_file->ReadInt(section, "Threads", num_threads);
	context_quantum = ini_file->ReadInt(section, "ContextQuantum", 100000);
	thread_quantum = ini_file->ReadInt(section, "ThreadQuantum", 1000);
	thread_switch_penalty = ini_file->ReadInt(section, "ThreadSwitchPenalty", 0);
	recover_kind = (RecoverKind)ini_file->ReadEnum(section, "RecoverKind",
			recover_kind_map, RecoverKindWriteback);
	recover_penalty = ini_file->ReadInt(section, "RecoverPenalty", 0);
	process_prefetch_hints = ini_file->ReadBool(section, "ProcessPrefetchHints", true);
	use_nc_store = ini_file->ReadBool(section,"UseNCStore", false);
	if (use_nc_store && (num_cores * num_threads > 1))
	{
		throw Error("When UseNCStore = True, Cores and Threads must be 1.");
	}
	prefetch_history_size = ini_file->ReadInt(section, "PrefetchHistorySize", 10);

	// Section '[ Pipeline ]'
	section = "Pipeline";
	fetch_kind = (FetchKind) ini_file->ReadEnum(section, "FetchKind",
			fetch_kind_map, FetchKindTimeslice);
	decode_width = ini_file->ReadInt(section, "DecodeWidth", 4);
	dispatch_kind = (DispatchKind) ini_file->ReadEnum(section, "DispatchKind",
			dispatch_kind_map, DispatchKindTimeslice);
	dispatch_width = ini_file->ReadInt(section, "DispatchWidth", 4);
	issue_kind = (IssueKind) ini_file->ReadEnum(section, "IssueKind",
			issue_kind_map, IssueKindTimeslice);
	issue_width = ini_file->ReadInt(section, "IssueWidth", 4);
	commit_kind = (CommitKind) ini_file->ReadEnum(section, "CommitKind",
			commit_kind_map, CommitKindShared);
	commit_width = ini_file->ReadInt(section, "CommitWidth", 4);
	occupancy_stats = ini_file->ReadBool(section, "OccupancyStats", false);

	// Section '[ Queues ]'
	section = "Queues";
	reorder_buffer_kind = (ReorderBufferKind) ini_file->ReadEnum(section, "RobKind",
				reorder_buffer_kind_map, ReorderBufferKindPrivate);
	reorder_buffer_size = ini_file->ReadInt(section, "RobSize", 64);
	// x86_fetch_queue_size = ini_file->ReadInt(section, "FetchQueueSize", 64);

	// x86_uop_queue_size = ini_file->ReadInt(section, "UopQueueSize", 32);

	// x86_iq_kind = ini_file->ReadEnum(section, "IqKind",
			//x86_iq_kind_private, x86_iq_kind_map, 2);
	// x86_iq_size = ini_file->ReadInt(section, "IqSize", 40);

	// x86_lsq_kind = ini_file->ReadEnum(section, "LsqKind",
			//x86_lsq_kind_private, x86_lsq_kind_map, 2);
	// x86_lsq_size = ini_file->ReadInt(section, "LsqSize", 20);*/
}

}
