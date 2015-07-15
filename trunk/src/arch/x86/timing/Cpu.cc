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

#include "Cpu.h"


namespace x86
{

misc::StringMap Cpu::recover_kind_map =
{
	{"Writeback", RecoverKindWriteback},
	{"Commit", RecoverKindCommit}
};

misc::StringMap Cpu::fetch_kind_map =
{
	{"Shared", FetchKindShared},
	{"TimeSlice", FetchKindTimeslice}
};

misc::StringMap Cpu::dispatch_kind_map =
{
	{"Shared", DispatchKindShared},
	{"TimeSlice", DispatchKindTimeslice},
};

misc::StringMap Cpu::issue_kind_map =
{
	{"Shared", IssueKindShared},
	{"TimeSlice", IssueKindTimeslice},
};

misc::StringMap Cpu::commit_kind_map =
{
	{"Shared", CommitKindShared},
	{"TimeSlice", CommitKindTimeslice},
};

misc::StringMap Cpu::reorder_buffer_kind_map =
{
	{"Private", ReorderBufferKindPrivate},
	{"Shared", ReorderBufferKindShared},
};

misc::StringMap Cpu::instruction_queue_kind_map =
{
	{"Shared", InstructionQueueKindShared},
	{"Private", InstructionQueueKindPrivate},
};

misc::StringMap Cpu::load_store_queue_kind_map =
{
	{"Shared", LoadStoreQueueKindShared},
	{"Private", LoadStoreQueueKindPrivate},
};

int Cpu::num_cores = 1;
int Cpu::num_threads = 1;
int Cpu::context_quantum;
int Cpu::thread_quantum;
int Cpu::thread_switch_penalty;
int Cpu::recover_penalty;
Cpu::RecoverKind Cpu::recover_kind;
Cpu::FetchKind Cpu::fetch_kind;
int Cpu::decode_width;
int Cpu::dispatch_width;
Cpu::DispatchKind Cpu::dispatch_kind;
int Cpu::issue_width;
Cpu::IssueKind Cpu::issue_kind;
int Cpu::commit_width;
Cpu::CommitKind Cpu::commit_kind;
bool Cpu::occupancy_stats;
int Cpu::reorder_buffer_size;
Cpu::ReorderBufferKind Cpu::reorder_buffer_kind;
int Cpu::fetch_queue_size;
Cpu::InstructionQueueKind Cpu::instruction_queue_kind;
int Cpu::instruction_queue_size;
Cpu::LoadStoreQueueKind Cpu::load_store_queue_kind;
int Cpu::load_store_queue_size;
int Cpu::uop_queue_size;


Cpu::Cpu()
{
	// Initialize
	emulator = Emulator::getInstance();
	mmu = misc::new_unique<mem::MMU>("x86");

	// Create cores
	cores.reserve(num_cores);
	for (int i = 0; i < num_cores; i++)
		cores.emplace_back(misc::new_unique<Core>(this, i));
}


void Cpu::ParseConfiguration(misc::IniFile *ini_file)
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
	fetch_queue_size = ini_file->ReadInt(section, "FetchQueueSize", 64);
	instruction_queue_kind = (InstructionQueueKind) ini_file->ReadEnum(section, "IqKind",
			instruction_queue_kind_map, InstructionQueueKindPrivate);
	instruction_queue_size = ini_file->ReadInt(section, "IqSize", 40);

	load_store_queue_kind = (LoadStoreQueueKind) ini_file->ReadEnum(section, "LsqKind",
			load_store_queue_kind_map, LoadStoreQueueKindPrivate);
	load_store_queue_size = ini_file->ReadInt(section, "LsqSize", 20);
	uop_queue_size = ini_file->ReadInt(section, "UopQueueSize", 32);

}


void Cpu::Run()
{
	// Run all cores
	for (auto &core : cores)
		core->Run();
}

}
