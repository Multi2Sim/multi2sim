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

#ifndef ARCH_FERMI_EMU_THREAD_H
#define ARCH_FERMI_EMU_THREAD_H

#include <arch/fermi/asm/Inst.h>
#include <mem-system/Memory.h>

#include "Emu.h"


namespace Frm
{

/// Forward declarations
class Warp;
class ThreadBlock;
class Grid;

/// Abstract polymorphic class used to attach additional information to the
/// thread. The timing simulator can created objects derived from this class
/// and link them with the thread.
class ThreadData
{
public:
	virtual ~ThreadData();
};


class Thread
{
public:

	/// Memory accesses types
	union ThreadReg 
	{
		unsigned u32;
		int s32;
		float f;
	};

	/// Used for local memory accesses
	struct MemoryAccess
	{
		MemoryAccessType type;
		unsigned addr;
		unsigned size;
	};

private:
	// IDs
	int id;  // global ID
	int id_in_warp;
	int id_in_thread_block;  // local ID

	// 3-dimensional IDs
	int id_3d[3];  // global 3D IDs
	int id_in_thread_block_3d[3];  // local 3D IDs

	// Warp, work-group, and Grid where it belongs
	Warp *warp;
	ThreadBlock *thread_block;
	Grid *grid;

	// Thread state
	ThreadReg gpr[128];  // General purpose registers
	ThreadReg sr[InstSRegCount];  // Special registers
	unsigned int pr[8];  // Predicate registers

	/* This is a digest of the active mask updates for this thread. Every time
	 * an instruction updates the active mask of a warp, this digest is updated
	 * for active threads by XORing a random number common for the warp.
	 * At the end, threads with different 'branch_digest' numbers can be considered
	 * divergent threads. */
	unsigned int branch_digest;

	/* Last global memory access */
	unsigned int global_mem_access_addr;
	unsigned int global_mem_access_size;

	/* Last local memory access */
	int lds_access_count;  /* Number of local memory access performed by last instruction */
	unsigned int lds_access_addr[FRM_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	unsigned int lds_access_size[FRM_MAX_LOCAL_MEM_ACCESSES_PER_INST];
	int lds_access_type[FRM_MAX_LOCAL_MEM_ACCESSES_PER_INST];  /* 0-none, 1-read, 2-write */


	// Last LDS accesses by last instruction
	int lds_access_count;  // Number of LDS access by last instruction
	MemoryAccess lds_access[MaxLDSAccessesPerInst];

public:

	/// Constructor
	/// \param Warp Warp that it belongs to
	/// \id Global 1D identifier of the thread
	Thread(Warp *warp, int id);

	// FIXME - probably most functions below can be inline

	/// Get value of a GPR
	/// \param vreg GPR identifier
	unsigned ReadGPR(int gpr);

	/// Set value of a GPR
	/// \param gpr GPR idendifier
	/// \param value Value given as an \a unsigned typed value
	void WriteGPR(int gpr, unsigned value);

	/// Get value of the active thread mask
	int GetActive();

	/// Set value of the active thread mask
	/// \param value Value given as an \a unsigned typed value
	void SetActive(unsigned value);

	/// Get value of a predicate register
	/// \param pr Predicate register identifier
	int GetPred(int pr);

	/// Set value of a predicate register
	/// \param pr Predicate register idendifier
	/// \param value Value given as an \a unsigned typed value
	void SetPred(int pr, unsigned value);

	/// ?
	void UpdateBranchDigest(long long inst_count, unsigned int inst_addr);
};

}  /* namespace Frm */

#endif
