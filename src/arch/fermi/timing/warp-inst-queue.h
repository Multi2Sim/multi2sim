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

#ifndef FERMI_WARP_POOL_H
#define FERMI_WARP_POOL_H


struct frm_warp_inst_queue_entry_t
{
	unsigned int valid : 1; /* Valid if warp assigned to entry */

	int id_in_warp_inst_queue;
	struct frm_warp_inst_queue_t *warp_inst_queue;

	struct frm_warp_t *warp;
	long long int cycle_fetched;
	struct frm_uop_t *uop;

	/* Status (not mutually exclusive) */
	unsigned int ready : 1;              /* Ready to fetch next instruction */
	unsigned int ready_next_cycle : 1;   /* Will be ready next cycle */
	/* TOOD Break wait_for_mem into waiting for each memory type */
	unsigned int wait_for_mem : 1;       /* Waiting for memory instructions */
	unsigned int wait_for_barrier : 1;   /* Waiting at barrier */
	unsigned int warp_finished : 1; /* Wavefront executed last inst */

	/* Outstanding memory accesses */
	unsigned int vm_cnt;     /* Vector memory count */
	unsigned int exp_cnt;    /* Export count */
	unsigned int lgkm_cnt;   /* LDS, GDS, Constant, and message count */
};

struct frm_warp_inst_queue_t
{
	/* List of currently mapped warps */
	int warp_count;
	struct frm_warp_inst_queue_entry_t **entries;

	/* SM it belongs to */
	struct frm_sm_t *sm;
};

#endif
