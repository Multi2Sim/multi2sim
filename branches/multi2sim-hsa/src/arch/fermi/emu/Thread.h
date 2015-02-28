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

#include <list>

#include <arch/fermi/asm/Inst.h>
#include <arch/fermi/asm/Wrapper.h>
#include <memory/Memory.h>

#include "Emu.h"


namespace Frm
{

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
	/// GPR value
	union RegValue
	{
		unsigned u32;
		int s32;
		float f;
	};

	/// Memory accesses types
	enum MemoryAccessType
	{
		MemoryAccessInvalid = 0,
		MemoryAccessRead,
		MemoryAccessWrite
	};

	/// Memory accesses attributes
	struct MemoryAccess
	{
		MemoryAccessType type;
		unsigned addr;
		unsigned size;
	};

private:
	// IDs
	int id;
	int id_in_thread_block;
	int id_in_warp;

	// 3D IDs
	int id_3d[3];
	int id_in_thread_block_3d[3];

	// Warp, thread-block, and grid where it belongs
	Warp *warp;
	ThreadBlock *thread_block;
	Grid *grid;

	// GPR
	RegValue gpr[128];

	// Special registers
	RegValue sr[FrmInstSRegCount];

	// Predicate registers
	unsigned pr[8];

	// Last global memory access
	unsigned global_mem_access_addr;
	unsigned global_mem_access_size;

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
	/// \param gpr GPR idenfifier
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
	/// \param pr Predicate register identifier
	/// \param value Value given as an \a unsigned typed value
	void SetPred(int pr, unsigned value);
};

}  /* namespace Frm */

#endif
