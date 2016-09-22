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

#ifndef ARCH_KEPLER_TIMING_SM_H
#define ARCH_KEPLER_TIMING_SM_H

#include <list>

#include<memory/Module.h>

#include "Bru.h"
#include "Dpu.h"
#include "FetchBuffer.h"
#include "Imu.h"
#include "Lsu.h"
#include "Sfu.h"
#include "Spu.h"
#include "ScoreBoard.h"
#include "WarpPool.h"


namespace Kepler
{

// Forward declarations
class Timing;
class ThreadBlock;
class GPU;
class ScoreBoard;




/// Class representing an SM in the GPU device
class SM
{
	// Fetch an instruction from the given warp pool
	void Fetch(FetchBuffer *fetch_buffer, WarpPool *warp_pool);

	// Dispatch the instruction to the given execution
	void DispatchToExecutionUnit(std::unique_ptr<Uop> uop,
			ExecutionUnit *execution_unit);

	// Dispatch an instruction from the given fetch buffer into the appropriate
	// execution unit.
	void Dispatch(FetchBuffer *fetch_buffer, WarpPool *warp_pool);

	// Update the visualization states for non-dispatched instructions
	void UpdateFetchVisualization(FetchBuffer *fetch_buffer);

	// Associated timing simulator, saved for performance
	Timing *timing = nullptr;

	// Associated GPU
	GPU *gpu;

	// Index of SM in the GPU device, initialized in the constructor.
	int id;

	// List of blocks currently mapped to the SM
	std::list<ThreadBlock *> thread_blocks;

	// Variable number of warp pools
	std::vector<std::unique_ptr<WarpPool>> warp_pools;

	// variable number of fetch buffers
	std::vector<std::unique_ptr<FetchBuffer>> fetch_buffers;

	// Variable number of score board
	std::vector<std::unique_ptr<ScoreBoard>> scoreboard;

	// Variable number of private single precision units
	std::vector<std::unique_ptr<SPU>> private_spus;

	// Variable number of shared single precision unit
	std::vector<std::unique_ptr<SPU>> shared_spus;

	// Variable number of branch unit
	std::vector<std::unique_ptr<BRU>> private_brus;

	// Variable number of double precision units
	std::vector<std::unique_ptr<DPU>> shared_dpus;

	// Variable number of load store unit
	std::vector<std::unique_ptr<LSU>> shared_lsus;

	// Variable number of integer math unit
	std::vector<std::unique_ptr<IMU>> shared_imus;

	// Variable number of special function unit
	std::vector<std::unique_ptr<SFU>> shared_sfus;

	// Vector indicates the availability of shared spus
	std::vector<int> is_shared_spus_available;

	// Vector indicates the availability of shared dpus
	std::vector<int> is_shared_dpus_available;

	// Vector indicates the availability of shared lsus
	std::vector<int> is_shared_lsus_available;

	// Vector indicates the availability of shared imus
	std::vector<int> is_shared_imus_available;

	// Vector indicates the availability of shared sfus
	std::vector<int> is_shared_sfus_available;

	// Vector indicates the availability of private spus
	std::vector<int> is_private_spus_available;

	// Vector indicates the availability  of private brus
	std::vector<int> is_private_brus_available;

	// Vector indicates next available cycle of shared spus
	std::vector<long long> shared_spus_next_available_cycle;

	// Vector indicates next available cycle of shared dpus
	std::vector<long long> shared_dpus_next_available_cycle;

	// Vector indicates next available cycle of shared lsus
	std::vector<long long> shared_lsus_next_available_cycle;

	// Vector indicates next available cycle of shared imus
	std::vector<long long> shared_imus_next_available_cycle;

	// Vector indicates next available cycle of shared sfus
	std::vector<long long> shared_sfus_next_available_cycle;

	// Vector indicates next available cycle of private spus
	std::vector<long long> private_spus_next_available_cycle;

	// Vector indicates next available cycle of private brus
	std::vector<long long> private_brus_next_available_cycle;

    // Counter of identifiers assigned to uops in this SM
	long long uop_id_counter = 0;

	// Id of the warp pool which the previous warp from the block was mapped to
	int last_mapped_warp_pool_id;

	// Flag indicates number of instructions dispatched to shared IMU from all
	// warp pools per cycle. Every new cycle it starts from 0.
	int dispatched_to_shared_imu;

	// Flag indicates number of instructions dispatched to shared SFU from all
	// warp pools per cycle. Every new cycle it starts from 0.
	int dispatched_to_shared_sfu;

	// Flag indicates number of instructions dispatched to shared LSU from all
	// warp pools per cycle. Every new cycle it starts from 0.
	//int dispatched_to_shared_lsu;

	// Flag indicates number of instructions dispatched to shared DPUs from all
	// warp pools per cycle. Every new cycle it starts from 0.
	int dispatched_to_shared_dpu;

	// Flag indicated number of instructions dispatched to shared SPUs from all
	// warp pools per cycle. Every new cycle it stars from 0
	int dispatched_to_shared_spu;

	// Flag indicates if the warp pool is available. This flag is added for
	// different number of lanes in a unit.
	std::vector<bool> is_warp_pool_available;

	// Vector indicates next available cycle of this warp pool.
	long long warp_pool_next_available_cycle[4];

public:
	//
	// Static fields
	//

	/// Number of warp pools per SM, configured by the user
	static int num_warp_pools;

	/// Number of shared spus
	static int num_shared_spus;

	/// Number of shared imus
	static int num_shared_imus;

	/// Number of shared dpus
	static int num_shared_dpus;

	/// Number of shared lsu
	static int num_shared_lsus;

	/// Number of shared sfu
	static int num_shared_sfus;

	/// Fetch latency in cycles
	static int fetch_latency;

	/// Number of instructions fetched per cycle
	static int fetch_width;

	/// Maximum capacity of fetch buffer in number of instructions
	static int fetch_buffer_size;

	/// Dispatch latency in cycles
	static int dispatch_latency;

	/// Number of instructions dispatched per cycle
	static int dispatch_width;

	/// Maximum number of instructions dispatched in each cycle of each type
	///(branch, FP32, FP64)
	static int max_instructions_dispatched_per_type;

	/// The maximum number of warps in a warp pool
	static int max_warps_per_warp_pool;

	/// The maximum number of blocks in a warp pool
	static int max_blocks_per_warp_pool;

	/// The maximum number of warps in an SM
	static int max_warps_per_sm;

	/// The maximum number of blocks in an SM
	static int max_blocks_per_sm;


	//
	// Class members
	//

	/// Constructor
	SM(int id, GPU *gpu);

	/// Advance SM state by one cycle;
	void Run();

	/// Return the index of this SM in the GPU
	int getId() const { return id;}

	/// Return a new unique sequential identifier for the next uop in the SM
	long long getUopId() { return ++uop_id_counter; }

	/// Return a pointer to the associated GPU
	GPU *getGPU() { return gpu; }

	/// Return the associated timing simulator
	Timing *getTiming() const { return timing; }

	/// Return the scoreboard
	ScoreBoard *getScoreboard(unsigned id) { return scoreboard[id].get(); }

	/// Map a block to the SM
	void MapBlock(ThreadBlock *thread_block);

	/// Unmap a block from the SM
	void UnmapBlock(ThreadBlock *thread_block);

	/// Add a block pointer to the blocks list
	void AddBlock(ThreadBlock *thread_block);

	/// Remove a block pointer from the blocks list
	void RemoveBlock(ThreadBlock *thread_block);

	/// Iterator of the SM location in the available SMs list
	std::list<SM *>::iterator available_sms_iterator;

	/// Flag to indicate if the SM is currently available
	bool in_available_sms = false;

	/// Cache
	mem::Module *cache = nullptr;




	//
	// Statistics
	//

	// Number of total instructions
	long long num_total_instructions = 0;

	// Number of dispatched shared SPU instructions
	long long num_spu_instructions = 0;

	// Number of dispatched DPU instructions
	long long num_dpu_instructions = 0;

	// Number of dispatched SFU instructions
	long long num_sfu_instructions = 0;

	// Number of dispatched IMU instructions
	long long num_imu_instructions = 0;

	// Number of dispatched branch instructions
	long long num_branch_instructions = 0;

	// Number of dispatched memory instructions
	long long num_memory_instructions = 0;

	// Number of registers being read from
	long long num_regiters_reads = 0;

	// Number of registers being written to
	long long num_registers_writes = 0;

	// Number of total mapped blocks for the SM
	long long num_mapped_blocks = 0;
};

}

#endif
