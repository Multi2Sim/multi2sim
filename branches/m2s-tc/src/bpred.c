/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include <m2s.h>

uint32_t bpred_btb_sets = 128;
uint32_t bpred_btb_ways = 32;

uint32_t bpred_bhr_size = 8;

uint32_t bpred_ras_size = 128;


void bpred_reg_options()
{
	opt_reg_uint32("-bpred:btb_sets", "BTB sets", &bpred_btb_sets);
	opt_reg_uint32("-bpred:btb_assoc", "BTB assoc", &bpred_btb_ways);
	opt_reg_uint32("-bpred:bhr_size", "Branch history register size", &bpred_bhr_size);
	opt_reg_uint32("-bpred:ras_size", "Return address stack size", &bpred_ras_size);
}


void bpred_init()
{
	int core, thread, i;
	struct bpred_t *bpred;

	FOREACH_CORE FOREACH_THREAD {
		THREAD.bpred = bpred = calloc(1, sizeof(struct bpred_t));
		bpred->btb = calloc(bpred_btb_sets * bpred_btb_ways, sizeof(struct btb_entry_t));
		bpred->twolev_pht = calloc(1 << bpred_bhr_size, sizeof(char));
		for (i = 0; i < (1 << bpred_bhr_size); i++)
			bpred->twolev_pht[i] = 2;
		bpred->ras = calloc(bpred_ras_size, sizeof(uint32_t));
	}
}


void bpred_done()
{
	int core, thread;
	struct bpred_t *bpred;

	FOREACH_CORE FOREACH_THREAD {
		bpred = THREAD.bpred;
		free(bpred->btb);
		free(bpred->twolev_pht);
		free(bpred->ras);
		free(bpred);
	}
}


static uint32_t bpred_btb_index(uint32_t eip)
{
	return eip % (bpred_btb_sets * bpred_btb_ways);
}


uint32_t bpred_get_trace(int core, int thread, uint32_t eip, int count)
{
	struct bpred_t *bpred = THREAD.bpred;
	int i, taken;
	uint32_t trace = 0, bhr;

	bhr = bpred->twolev_bhr;
	for (i = 0; i < count; i++) {
		assert(bhr < (1 << bpred_bhr_size));
		taken = bpred->twolev_pht[bhr] > 1;
		trace = (trace << 1) | taken;
		bhr = ((bhr << 1) | taken) & ((1 << bpred_bhr_size) - 1);
	}
	return trace;
}


uint32_t bpred_lookup(struct uop_t *uop)
{
	int core = uop->core, thread = uop->thread;
	struct bpred_t *bpred = THREAD.bpred;
	struct btb_entry_t *entry;
	uint32_t dest;
	int taken;

	/* Save RAS index and BHR */
	uop->bpred_bhr = bpred->twolev_bhr;
	uop->bpred_ras_idx = bpred->ras_idx;

	/* Look up PHT */
	assert(bpred->twolev_bhr < (1 << bpred_bhr_size));
	taken = bpred->twolev_pht[bpred->twolev_bhr] > 1;
	bpred->twolev_bhr = ((bpred->twolev_bhr << 1) | taken) & ((1 << bpred_bhr_size) - 1);

	/* Look up BTB or RAS */
	dest = uop->eip + uop->size;
	entry = &bpred->btb[bpred_btb_index(uop->eip)];
	if (entry->source == uop->eip) {
		dest = entry->dest;
		if (entry->kind == btb_branch_kind_call) {
			bpred->ras[bpred->ras_idx] = uop->eip + entry->size;
			bpred->ras_idx = (bpred->ras_idx + 1) % bpred_ras_size;
		} else if (entry->kind == btb_branch_kind_ret) {
			bpred->ras_idx = (bpred->ras_idx + bpred_ras_size - 1) % bpred_ras_size;
			dest = bpred->ras[bpred->ras_idx];
		} else if (!taken)
			dest = uop->eip + uop->size;
	}

	/* Return target address */
	return dest;
}


void bpred_update(struct uop_t *uop)
{
	int core = uop->core, thread = uop->thread;
	struct bpred_t *bpred = THREAD.bpred;
	struct btb_entry_t *entry;
	char *ctr;

	/* Update PHT */
	assert((uop->flags & FCTRL));
	ctr = &bpred->twolev_pht[uop->bpred_bhr];
	if (uop->taken) {
		*ctr = MAX(*ctr + 1, 0);
	} else {
		*ctr = MIN(*ctr - 1, 3);
	}

	/* Update BTB */
	entry = &bpred->btb[bpred_btb_index(uop->eip)];
	entry->source = uop->eip;
	entry->dest = uop->neip;
	entry->size = uop->size;
	if ((uop->flags & FCALL))
		entry->kind = btb_branch_kind_call;
	else if ((uop->flags & FRET))
		entry->kind = btb_branch_kind_ret;
	else
		entry->kind = btb_branch_kind_branch;
}


void bpred_recover(struct uop_t *uop)
{
	int core = uop->core, thread = uop->thread;
	struct bpred_t *bpred = THREAD.bpred;
	bpred->twolev_bhr = uop->bpred_bhr;
	bpred->ras_idx = uop->bpred_ras_idx;
	assert((uop->flags & FCTRL));
	assert(uop->mispred);
}



