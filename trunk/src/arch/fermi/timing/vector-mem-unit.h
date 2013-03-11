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

#ifndef ARCH_FERMI_TIMING_VECTOR_MEM_H
#define ARCH_FERMI_TIMING_VECTOR_MEM_H

struct frm_vector_mem_unit_t
{
	struct list_t *issue_buffer;  /* Issued instructions */
	struct list_t *decode_buffer; /* Decoded instructions */
	struct list_t *read_buffer;   /* Register reads */
	struct list_t *mem_buffer;    /* Submitted memory accesses */
	struct list_t *write_buffer;  /* Register writes */

	struct frm_sm_t *sm;

	/* Statistics */
	long long inst_count;

	/* Spatial profiling statistics*/
	long long inflight_mem_accesses ;

};

#endif
