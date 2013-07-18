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

#ifndef X86_ARCH_TIMING_UOP_H
#define X86_ARCH_TIMING_UOP_H

#include <arch/x86/emu/uinst.h>
#include <lib/util/class.h>



/*
 * Object 'x86_uop_t'
 */

struct x86_uop_t
{
	/* Micro-instruction */
	struct x86_uinst_t *uinst;
	enum x86_uinst_flag_t flags;

	/* Name and sequence numbers */
	long long magic;  /* Magic number for debugging */
	long long id;  /* Unique ID */
	long long id_in_core;  /* Unique ID in core */

	/* Software context and hardware thread where uop belongs */
	X86Context *ctx;
	X86Thread *thread;

	/* Fetch info */
	unsigned int eip;  /* Address of x86 macro-instruction */
	unsigned int neip;  /* Address of next non-speculative x86 macro-instruction */
	unsigned int pred_neip; /* Address of next predicted x86 macro-instruction (for branches) */
	unsigned int target_neip;  /* Address of target x86 macro-instruction assuming branch taken (for branches) */
	int specmode;
	unsigned int fetch_address;  /* Physical address of memory access to fetch this instruction */
	long long fetch_access;  /* Access identifier to fetch this instruction */
	int trace_cache;  /* Flag telling if uop came from trace cache */

	/* Fields associated with macroinstruction */
	char mop_name[40];
	int mop_index;  /* Index of uop within macroinstruction */
	int mop_count;  /* Number of uops within macroinstruction */
	int mop_size;  /* Corresponding macroinstruction size */
	long long mop_id;  /* Sequence number of macroinstruction */

	/* Logical dependencies */
	int idep_count;
	int odep_count;

	/* Physical mappings */
	int ph_int_idep_count, ph_fp_idep_count, ph_xmm_idep_count;
	int ph_int_odep_count, ph_fp_odep_count, ph_xmm_odep_count;
	int ph_idep[X86_UINST_MAX_IDEPS];
	int ph_odep[X86_UINST_MAX_ODEPS];
	int ph_oodep[X86_UINST_MAX_ODEPS];

	/* Queues where instruction is */
	int in_fetch_queue : 1;
	int in_uop_queue : 1;
	int in_iq : 1;
	int in_lq : 1;
	int in_sq : 1;
	int in_preq : 1;
	int in_event_queue : 1;
	int in_rob : 1;
	int in_uop_trace_list : 1;

	/* Instruction status */
	int ready;
	int issued;
	int completed;

	/* For memory uops */
	unsigned int phy_addr;  /* ... corresponding to 'uop->uinst->address' */

	/* Cycles */
	long long when;  /* cycle when ready */
	long long issue_try_when;  /* first cycle when f.u. is tried to be reserved */
	long long issue_when;  /* cycle when issued */

	/* Branch prediction */
	int pred;  /* Global prediction (0=not taken, 1=taken) */
	int bimod_index, bimod_pred;
	int twolevel_bht_index, twolevel_pht_row, twolevel_pht_col, twolevel_pred;
	int choice_index, choice_pred;
};

struct x86_uop_t *x86_uop_create(void);
void x86_uop_free_if_not_queued(struct x86_uop_t *uop);
void x86_uop_dump(struct x86_uop_t *uop, FILE *f);

int x86_uop_exists(struct x86_uop_t *uop);
void x86_uop_count_deps(struct x86_uop_t *uop);

struct linked_list_t;
void x86_uop_list_dump(struct list_t *uop_list, FILE *f);
void x86_uop_linked_list_dump(struct linked_list_t *uop_list, FILE *f);


#endif

