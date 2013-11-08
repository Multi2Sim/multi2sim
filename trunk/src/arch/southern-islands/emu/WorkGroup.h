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

#ifndef ARCH_SOUTHERN_ISLANDS_EMU_WORK_GROUP_H
#define ARCH_SOUTHERN_ISLANDS_EMU_WORK_GROUP_H


namespace SI
{

class NDRange;

class WorkGroup
{
	// Identifiers
	int id;
	int id_3d[3];

	// Status
	bool wavefronts_at_barrier;
	bool wavefronts_completed_emu;
	bool wavefronts_completed_timing;
	bool finished_emu;
	bool finished_timing;

	// ND-Range that it belongs to
	NDRange *ndrange;

	// List of work-items in the work-group
	std::vector<std::unique_ptr<WorkItem>> work_items;

	// List of wavefronts in the work-group
	std::vector<std::unique_ptr<Wavefront>> wavefronts;

	// Pool of wavefronts
	WavefrontPool wavefront_pool;

	// Field introduced for architectural simulation
	int id_in_compute_unit;

	// Local memory
	Memory lds;

	// Statistics
	long long sreg_read_count;
	long long sreg_write_count;
	long long vreg_read_count;
	long long vreg_write_count;

public:

	/// Constructor
	///
	/// \param ndrange Instance of class NDRange that it belongs to.
	/// \param id Work-group global 1D identifier
	WorkGroup(NDRange *ndrange, unsigned id);

	/// Dump work-group in human readable format into output stream
	void Dump(std::ostream &os);

	/// Equivalent to WorkGroup::Dump()
	friend std::ostream &operator<<(std::ostream &os,
			const WorkGroup &work_group) {
		work_group.Dump(os);
		return os;
	}

	/// Statistic showing the number of reads from scalar registers
	long long getSRegReadCount() { return sreg_read_count; }

	// FIXME - getters for other statistics


}  // namespace

#endif
