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

#ifndef ARCH_EVERGREEN_TIMING_SCHED_H
#define ARCH_EVERGREEN_TIMING_SCHED_H

extern struct str_map_t evg_gpu_sched_policy_map;
extern enum evg_gpu_sched_policy_t
{
	evg_gpu_sched_invalid = 0,  /* For invalid user input */
	evg_gpu_sched_round_robin,
	evg_gpu_sched_greedy
} evg_gpu_sched_policy;


struct evg_wavefront_t *evg_schedule(struct evg_compute_unit_t *compute_unit);

#endif

