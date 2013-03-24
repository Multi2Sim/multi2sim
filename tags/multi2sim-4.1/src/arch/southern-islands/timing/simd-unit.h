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

#ifndef ARCH_SOUTHERN_ISLANDS_TIMING_SIMD_UNIT_H
#define ARCH_SOUTHERN_ISLANDS_TIMING_SIMD_UNIT_H

struct si_subwavefront_pool_t
{
	struct si_uop_t *uop;
	unsigned int num_subwavefronts_executed;
};

/* Utilization Coefficient */
struct si_util_t
{
	long long cycles_utilized;
	long long cycles_considered;
};

struct si_simd_t
{
	int id_in_compute_unit;

	struct list_t *issue_buffer;  /* Issued instructions */
	struct list_t *decode_buffer; /* Decoded instructions */
	struct list_t *exec_buffer;   /* Execution */

	struct si_subwavefront_pool_t *subwavefront_pool;  
	struct si_wavefront_pool_t *wavefront_pool;

	struct si_compute_unit_t *compute_unit;

	/* Statistics */
	long long inst_count;

	/* In order of highest to lowest precedence (scope). All utilized 
	 * functional * units are considered for each level of scope. If a 
	 * functional unit is not utilized in a cycle, the specialized metric 
	 * of the highest precedence whose characteristics are met is the only 
	 * specialized metric considered. The total utilization metric is 
	 * always considered for all functional units. */
	struct si_util_t *wkg_util; /* Work group mapped to compute unit. */
	struct si_util_t *wvf_util; /* Wavefront mapped to instruction buffer. */
	struct si_util_t *rdy_util; /* Wavefront with appropriate next instruction 
								 * (vector ALU) is on its way to execute. */
	struct si_util_t *occ_util; /* Wavefront exists in the in the previous 
								 * buffer ready to be executed. */
	struct si_util_t *wki_util; /* Work item mapped to stream core. */
	struct si_util_t *act_util; /* Work item mapped to stream core is active. */
	struct si_util_t *tot_util; /* Total SIMD utilization. */
};

#endif
