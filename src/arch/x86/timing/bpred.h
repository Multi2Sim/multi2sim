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

#ifndef X86_ARCH_TIMING_BPRED_H
#define X86_ARCH_TIMING_BPRED_H

#include <lib/util/class.h>

/* Forward declarations */
struct x86_uop_t;
struct config_t;


/*
 * Class 'X86Thread'
 */

void X86ThreadInitBranchPred(X86Thread *self);
void X86ThreadFreeBranchPred(X86Thread *self);

int X86ThreadLookupBranchPred(X86Thread *self, struct x86_uop_t *uop);
int X86ThreadLookupBranchPredMultiple(X86Thread *self, unsigned int eip, int count);
void X86ThreadUpdateBranchPred(X86Thread *self, struct x86_uop_t *uop);

unsigned int X86ThreadLookupBTB(X86Thread *self, struct x86_uop_t *uop);
void X86ThreadUpdateBTB(X86Thread *self, struct x86_uop_t *uop);
unsigned int X86ThreadGetNextBranch(X86Thread *self, unsigned int eip,
		unsigned int bsize);




/*
 * Object 'x86_bpred_t'
 */

#define X86_BPRED_BTB_ENTRY(SET, WAY) \
		(&bpred->btb[(SET) * x86_bpred_btb_assoc + (WAY)])

/* BTB Entry */
struct x86_bpred_btb_entry_t
{
	unsigned int source;  /* eip */
	unsigned int target;  /* neip */
	int counter;  /* LRU counter */
};


/* Branch Predictor Structure */
struct x86_bpred_t
{
	char *name;

	/* RAS */
	unsigned int *ras;
	int ras_index;

	/* BTB - array of x86_bpred_btb_sets*x86_bpred_btb_assoc entries of
	 * type x86_bpred_btb_entry_t. */
	struct x86_bpred_btb_entry_t *btb;

	/* bimod - array of bimodal counters indexed by PC
	 *   0,1 - Branch not taken.
	 *   2,3 - Branch taken. */
	char *bimod;

	/* Two-level adaptive branch predictor. It contains a
	 * BHT (branch history table) and PHT (pattern history table). */
	unsigned int *twolevel_bht;  /* array of level1_size branch history registers */
	char *twolevel_pht;  /* array of level2_size*2^hist_size 2-bit counters */

	/* choice - array of bimodal counters indexed by PC
	 *   0,1 - Use bimodal predictor.
	 *   2,3 - Use two-level adaptive predictor */
	char *choice;

	/* Stats */
	long long accesses;
	long long hits;
};

struct x86_bpred_t *x86_bpred_create(char *name);
void x86_bpred_free(struct x86_bpred_t *bpred);




/*
 * Public
 */


extern char *x86_bpred_kind_map[];
extern enum x86_bpred_kind_t
{
	x86_bpred_kind_perfect = 0,
	x86_bpred_kind_taken,
	x86_bpred_kind_nottaken,
	x86_bpred_kind_bimod,
	x86_bpred_kind_twolevel,
	x86_bpred_kind_comb
} x86_bpred_kind;

extern int x86_bpred_btb_sets;
extern int x86_bpred_btb_assoc;
extern int x86_bpred_ras_size;
extern int x86_bpred_bimod_size;
extern int x86_bpred_choice_size;

extern int x86_bpred_twolevel_l1size;
extern int x86_bpred_twolevel_l2size;
extern int x86_bpred_twolevel_hist_size;
extern int x86_bpred_twolevel_l2height;


void X86ReadBranchPredConfig(struct config_t *config);

#endif

