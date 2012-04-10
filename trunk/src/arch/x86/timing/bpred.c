/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <cpuarch.h>

#define BTB_ENTRY(SET, WAY) (&bpred->btb[(SET) * bpred_btb_assoc + (WAY)])

/* BTB Entry */
struct btb_entry_t {
	uint32_t source;  /* eip */
	uint32_t target;  /* neip */
	int counter;  /* LRU counter */
};


/* Branch Predictor Structure */
struct bpred_t {
	
	/* RAS */
	uint32_t *ras;
	int ras_idx;
	
	/* BTB - array of bpred_btb_sets*bpred_btb_assoc entries of
	 * type btb_entry_t. */
	struct btb_entry_t *btb;
	
	/* bimod - array of bimodal counters indexed by PC
	 *   0,1 - Branch not taken.
	 *   2,3 - Branch taken. */
	char *bimod;

	/* Two-level adaptive branch predictor. It contains a
	 * BHT (branch history table) and PHT (pattern history table). */
	uint32_t *twolevel_bht;  /* array of level1_size branch history registers */
	char *twolevel_pht;  /* array of level2_size*2^hist_size 2-bit counters */
	
	/* choice - array of bimodal counters indexed by PC
	 *   0,1 - Use bimodal predictor.
	 *   2,3 - Use two-level adaptive predictor */
	char *choice;

	/* Stats */
	char name[20];
	uint64_t accesses;
	uint64_t hits;
};


char *bpred_kind_map[] = { "Perfect", "Taken", "NotTaken", "Bimodal", "TwoLevel", "Combined" };
enum bpred_kind_t bpred_kind;
int bpred_btb_sets;  /* Number of BTB sets */
int bpred_btb_assoc;  /* Number of BTB ways */
int bpred_ras_size;  /* Return address stack size */
int bpred_bimod_size;  /* Number of entries for bimodal predictor */
int bpred_choice_size;  /* Number of entries for choice predictor */

int bpred_twolevel_l1size;  /* Two-level adaptive predictor: level-1 size */
int bpred_twolevel_l2size;  /* Two-level adaptive predictor: level-2 size */
int bpred_twolevel_hist_size;  /* Two-level adaptive predictor: level-2 history size */
static int bpred_twolevel_l2height;




/*
 * Public functions
 */


void bpred_init()
{
	int core, thread;

	/* Two-level bpred parameter */
	bpred_twolevel_l2height = 1 << bpred_twolevel_hist_size;
	
	/* Integrity */
	if (bpred_bimod_size & (bpred_bimod_size - 1))
		fatal("number of entries in bimodal precitor must be a power of 2");
	if (bpred_choice_size & (bpred_choice_size - 1))
		fatal("number of entries in choice predictor must be power of 2");
	if (bpred_btb_sets & (bpred_btb_sets - 1))
		fatal("number of BTB sets must be a power of 2");
	if (bpred_btb_assoc & (bpred_btb_assoc - 1))
		fatal("BTB associativity must be a power of 2");
	
	if (bpred_twolevel_hist_size < 1 || bpred_twolevel_hist_size > 30)
		fatal("predictor history size must be >=1 and <=30");
	if (bpred_twolevel_l1size & (bpred_twolevel_l1size - 1))
		fatal("two-level predictor sizes must be power of 2");
	if (bpred_twolevel_l2size & (bpred_twolevel_l2size - 1))
		fatal("two-level predictor sizes must be power of 2");
	
	/* Initialization */
	FOREACH_CORE FOREACH_THREAD {
		THREAD.bpred = bpred_create();
		sprintf(THREAD.bpred->name, "c%dt%d.bpred", core, thread);
	}
}


void bpred_done()
{
	int core, thread;
	FOREACH_CORE FOREACH_THREAD
		bpred_free(THREAD.bpred);
}


struct bpred_t *bpred_create()
{
	struct bpred_t *bpred;
	int i, j;

	/* Create bpred */
	bpred = calloc(1, sizeof(struct bpred_t));
	strcpy(bpred->name, "bpred");
	bpred->ras = calloc(bpred_ras_size, sizeof(uint32_t));

	/* Bimodal predictor */
	if (bpred_kind == bpred_kind_bimod || bpred_kind == bpred_kind_comb) {
		bpred->bimod = calloc(bpred_bimod_size, sizeof(char));
		for (i = 0; i < bpred_bimod_size; i++)
			bpred->bimod[i] = 2;
	}

	/* Two-level adaptive branch predictor */
	if (bpred_kind == bpred_kind_twolevel || bpred_kind == bpred_kind_comb) {
		bpred->twolevel_bht = calloc(bpred_twolevel_l1size, sizeof(uint32_t));
		bpred->twolevel_pht = calloc(bpred_twolevel_l2size * bpred_twolevel_l2height, sizeof(char));
		for (i = 0; i < bpred_twolevel_l2size * bpred_twolevel_l2height; i++)
			bpred->twolevel_pht[i] = 2;
	}
	
	/* Choice predictor */
	if (bpred_kind == bpred_kind_comb) {
		bpred->choice = calloc(bpred_choice_size, sizeof(char));
		for (i = 0; i < bpred_choice_size; i++)
			bpred->choice[i] = 2;
	}

	/* Allocate BTB and assign lru counters */
	bpred->btb = calloc(bpred_btb_sets * bpred_btb_assoc, sizeof(struct btb_entry_t));
	for (i = 0; i < bpred_btb_sets; i++)
		for (j = 0; j < bpred_btb_assoc; j++)
			BTB_ENTRY(i, j)->counter = j;
	
	/* Return */
	return bpred;
}


void bpred_free(struct bpred_t *bpred)
{
	/* Bimodal table */
	if (bpred_kind == bpred_kind_bimod || bpred_kind == bpred_kind_comb)
		free(bpred->bimod);

	/* Two-level adaptive predictor tables */
	if (bpred_kind == bpred_kind_twolevel || bpred_kind == bpred_kind_comb) {
		free(bpred->twolevel_bht);
		free(bpred->twolevel_pht);
	}

	/* Choice table */
	if (bpred_kind == bpred_kind_comb)
		free(bpred->choice);
	
	/* Free */
	free(bpred->btb);
	free(bpred->ras);
	free(bpred);
}


/* Return prediction for an address (0=not taken, 1=taken) */
int bpred_lookup(struct bpred_t *bpred, struct uop_t *uop)
{
	/* If branch predictor is accessed, a BTB hit must have occurred before, which
	 * provides information about the branch, i.e., target address and whether it
	 * is a call, ret, jump, or conditional branch. Thus, branches other than
	 * conditional ones are always predicted taken. */
	assert(uop->flags & X86_UINST_CTRL);
	if (uop->flags | X86_UINST_UNCOND) {
		uop->pred = 1;
		return 1;
	}

	/* An internal branch (string operations) is always predicted taken */
	if (uop->uinst->opcode == x86_uinst_ibranch) {
		uop->pred = 1;
		return 1;
	}

	/* Perfect predictor */
	if (bpred_kind == bpred_kind_perfect)
		uop->pred = uop->neip != uop->eip + uop->mop_size;
	
	/* Taken predictor */
	if (bpred_kind == bpred_kind_taken)
		uop->pred = 1;
	
	/* Not-taken predictor */
	if (bpred_kind == bpred_kind_nottaken)
		uop->pred = 0;
	
	/* Bimodal predictor */
	if (bpred_kind == bpred_kind_bimod || bpred_kind == bpred_kind_comb) {
		uop->bimod_index = uop->eip & (bpred_bimod_size - 1);
		uop->bimod_pred = bpred->bimod[uop->bimod_index] > 1;
		uop->pred = uop->bimod_pred;
	}
	
	/* Two-level adaptive */
	if (bpred_kind == bpred_kind_twolevel || bpred_kind == bpred_kind_comb) {
		uop->twolevel_bht_index = uop->eip & (bpred_twolevel_l1size - 1);
		uop->twolevel_pht_row = bpred->twolevel_bht[uop->twolevel_bht_index];
		assert(uop->twolevel_pht_row < bpred_twolevel_l2height);
		uop->twolevel_pht_col = uop->eip & (bpred_twolevel_l2size - 1);
		uop->twolevel_pred = bpred->twolevel_pht[uop->twolevel_pht_row *
			bpred_twolevel_l2size + uop->twolevel_pht_col] > 1;
		uop->pred = uop->twolevel_pred;
	}

	/* Combined */
	if (bpred_kind == bpred_kind_comb) {
		uop->choice_index = uop->eip & (bpred_choice_size - 1);
		uop->choice_pred = bpred->choice[uop->choice_index] > 1;
		uop->pred = uop->choice_pred ? uop->twolevel_pred : uop->bimod_pred;
	}

	/* Return prediction */
	assert(!uop->pred || uop->pred == 1);
	return uop->pred;
}


/* Return multiple predictions for an address. This can only be done for two-level
 * adaptive predictors, since they use global history. The prediction of the
 * primary branch is stored in the least significant bit (bit 0), whereas the prediction
 * of the last branch is stored in bit 'count-1'. */
/* FIXME - debug */
int bpred_lookup_multiple(struct bpred_t *bpred, uint32_t eip, int count)
{
	int i, pred, temp_pred;
	uint32_t bht_index, pht_col;
	uint32_t bhr;  /* branch history register = pht_row */

	/* First make a regular prediction. This updates the necessary fields in the
	 * uop for a later call to bpred_update, and makes the first prediction
	 * considering known characteristics of the primary branch. */
	assert(bpred_kind == bpred_kind_twolevel);
	bht_index = eip & (bpred_twolevel_l1size - 1);
	bhr = bpred->twolevel_bht[bht_index];
	assert(bhr < bpred_twolevel_l2height);
	pht_col = eip & (bpred_twolevel_l2size - 1);
	pred = temp_pred = bpred->twolevel_pht[bhr * bpred_twolevel_l2size + pht_col] > 1;

	/* Make the rest of predictions */
	for (i = 1; i < count; i++) {
		bhr = ((bhr << 1) | temp_pred) & (bpred_twolevel_l2height - 1);
		temp_pred = bpred->twolevel_pht[bhr * bpred_twolevel_l2size + pht_col] > 1;
		assert(!temp_pred || temp_pred == 1);
		pred |= temp_pred << i;
	}

	/* Return */
	return pred;
}


void bpred_update(struct bpred_t *bpred, struct uop_t *uop)
{
	int taken;
	char *pctr;  /* pointer to 2-bit counter */
	uint32_t *pbhr;  /* pointer to branch history register */

	assert(!uop->specmode);
	assert(uop->flags & X86_UINST_CTRL);
	taken = uop->neip != uop->eip + uop->mop_size;

	/* Stats */
	bpred->accesses++;
	if (uop->neip == uop->pred_neip)
		bpred->hits++;
	
	/* Update predictors. This is only done for conditional branches. Thus,
	 * exit now if instruction is a call, ret, or jmp.
	 * No update is performed in a perfect branch predictor either. */
	if (bpred_kind == bpred_kind_perfect)
		return;
	if (uop->flags & X86_UINST_UNCOND)
		return;
	
	/* Bimodal predictor was used */
	if (bpred_kind == bpred_kind_bimod || 
		(bpred_kind == bpred_kind_comb && !uop->choice_pred))
	{
		pctr = &bpred->bimod[uop->bimod_index];
		*pctr = taken ? MIN(*pctr + 1, 3) : MAX(*pctr - 1, 0);
	}

	/* Two-level adaptive predictor was used */
	if (bpred_kind == bpred_kind_twolevel ||
		(bpred_kind == bpred_kind_comb && uop->choice_pred))
	{
		/* Shift entry in BHT (level 1), and append direction */
		pbhr = &bpred->twolevel_bht[uop->twolevel_bht_index];
		*pbhr = ((*pbhr << 1) | taken) & (bpred_twolevel_l2height - 1);

		/* Update counter in PHT (level 2) as per direction */
		pctr = &bpred->twolevel_pht[uop->twolevel_pht_row *
			bpred_twolevel_l2size + uop->twolevel_pht_col];
		*pctr = taken ? MIN(*pctr + 1, 3) : MAX(*pctr - 1, 0);
	}

	/* Choice predictor - update only if bimodal and two-level
	 * predictions differ. */
	if (bpred_kind == bpred_kind_comb && uop->bimod_pred != uop->twolevel_pred) {
		pctr = &bpred->choice[uop->choice_index];
		*pctr = uop->bimod_pred == taken ? MAX(*pctr - 1, 0) : MIN(*pctr + 1, 3);
	}
}


/* Lookup BTB. If it contains the uop address, return target. The BTB also contains
 * information about the type of branch, i.e., jump, call, ret, or conditional. If
 * instruction is call or ret, access RAS instead of BTB. */
uint32_t bpred_btb_lookup(struct bpred_t *bpred, struct uop_t *uop)
{
	struct btb_entry_t *entry;
	uint32_t way, set, target = 0;
	int hit = 0;

	assert(uop->flags & X86_UINST_CTRL);

	/* Perfect branch predictor */
	if (bpred_kind == bpred_kind_perfect)
		return uop->neip;

	/* Internal branch (string operations) always predicted to jump to itself */
	if (uop->uinst->opcode == x86_uinst_ibranch)
		return uop->eip;

	/* Search address in BTB */
	set = uop->eip & (bpred_btb_sets - 1);
	for (way = 0; way < bpred_btb_assoc; way++) {
		entry = BTB_ENTRY(set, way);
		if (entry->source != uop->eip)
			continue;
		target = entry->target;
		hit = 1;
		break;
	}
	
	/* If there was a hit, we know whether branch is a call.
	 * In this case, push return address into RAS. To avoid
	 * updates at recovery, do it only for non-spec instructions. */
	if (hit && uop->uinst->opcode == x86_uinst_call && !uop->specmode) {
		bpred->ras[bpred->ras_idx] = uop->eip + uop->mop_size;
		bpred->ras_idx = (bpred->ras_idx + 1) % bpred_ras_size;
	}

	/* If there was a hit, we know whether branch is a ret. In this case,
	 * pop target from the RAS, and ignore target obtained from BTB. */
	if (hit && uop->uinst->opcode == x86_uinst_ret && !uop->specmode) {
		bpred->ras_idx = (bpred->ras_idx + bpred_ras_size - 1) % bpred_ras_size;
		target = bpred->ras[bpred->ras_idx];
	}

	/* Return */
	return target;
}


/* Update BTB */
void bpred_btb_update(struct bpred_t *bpred, struct uop_t *uop)
{
	struct btb_entry_t *entry, *found = NULL;
	int way, set;

	/* No update for perfect branch predictor */
	if (bpred_kind == bpred_kind_perfect)
		return;
	
	/* Search address in BTB */
	set = uop->eip & (bpred_btb_sets - 1);
	for (way = 0; way < bpred_btb_assoc; way++) {
		entry = BTB_ENTRY(set, way);
		if (entry->source == uop->eip) {
			found = entry;
			break;
		}
	}
	
	/* If address was not found, evict LRU entry */
	if (!found) {
		for (way = 0; way < bpred_btb_assoc; way++) {
			entry = BTB_ENTRY(set, way);
			entry->counter--;
			if (entry->counter < 0) {
				entry->counter = bpred_btb_assoc - 1;
				entry->source = uop->eip;
				entry->target = uop->neip;
			}
		}
	}
	
	/* If address was found, update LRU counters and target */
	if (found) {
		for (way = 0; way < bpred_btb_assoc; way++) {
			entry = BTB_ENTRY(set, way);
			if (entry->counter > found->counter)
				entry->counter--;
		}
		found->counter = bpred_btb_assoc - 1;
		found->target = uop->neip;
	}
}


/* Find address of next branch after eip within current block.
 * This is useful for accessing the trace
 * cache. At that point, the uop is not ready to call bpred_btb_lookup, since
 * functional simulation has not happened yet. */
uint32_t bpred_btb_next_branch(struct bpred_t *bpred, uint32_t eip, uint32_t bsize)
{
	struct btb_entry_t *entry;
	uint32_t limit;
	int set, way;

	assert(!(bsize & (bsize - 1)));
	limit = (eip + bsize) & ~(bsize - 1);
	while (eip < limit) {
		set = eip & (bpred_btb_sets - 1);
		for (way = 0; way < bpred_btb_assoc; way++) {
			entry = BTB_ENTRY(set, way);
			if (entry->source == eip)
				return eip;
		}
		eip++;
	}
	return 0;
}

