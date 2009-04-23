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


/* Parameters */
static enum bpred_kind_enum {
	bpred_kind_perfect = 0,
	bpred_kind_taken,
	bpred_kind_nottaken,
	bpred_kind_comb
} bpred_kind = bpred_kind_comb;


static char *bpred_btb = "256:4";
static uint32_t bpred_btb_sets;
static uint32_t bpred_btb_assoc;
static uint32_t bpred_bimod_size = 4096;
static uint32_t bpred_gshare_size = 4096;
static uint32_t bpred_choice_size = 4096;
static uint32_t bpred_ras_size = 256;


/* Push return address to the corresponding RAS */
static void bpred_ras_push(struct bpred_t *bpred, struct uop_t *uop)
{
	bpred->ras[bpred->ras_idx] = uop->eip + uop->size;
	bpred->ras_idx = (bpred->ras_idx + 1) % bpred_ras_size;
}


/* Get a return address from the corresponding RAS */
static uint32_t bpred_ras_pop(struct bpred_t *bpred, struct uop_t *uop)
{
	bpred->ras_idx = (bpred->ras_idx + bpred_ras_size - 1) % bpred_ras_size;
	return bpred->ras[bpred->ras_idx];
}


/* Lookup BTB. If it contains a valid address, return it.
 * Otherwise, return a destination address corresponding to a not taken branch. */
static uint32_t bpred_btb_lookup(struct bpred_t *bpred, struct uop_t *uop)
{
	uint32_t way, set;
	
	/* Search source address in BTB */
	set = uop->eip & (bpred_btb_sets - 1);
	for (way = 0; way < bpred_btb_assoc; way++)
		if (bpred->btb[set][way].source == uop->eip)
			return bpred->btb[set][way].dest;
	
	/* If not found, return address of next instruction */
	return uop->eip + uop->size;
}


/* Update BTB */
static void bpred_btb_update(struct bpred_t *bpred, struct uop_t *uop)
{
	int way, set;
	int found_way = -1;
	
	/* Search source address in BTB */
	set = uop->eip & (bpred_btb_sets - 1);
	for (way = 0; way < bpred_btb_assoc; way++) {
		if (bpred->btb[set][way].source == uop->eip)
			found_way = way;
	}
	
	/* If address was not found, evict LRU entry */
	if (found_way < 0) {
		for (way = 0; way < bpred_btb_assoc; way++) {
			bpred->btb[set][way].counter--;
			if (bpred->btb[set][way].counter < 0) {
				bpred->btb[set][way].counter = bpred_btb_assoc - 1;
				bpred->btb[set][way].source = uop->eip;
				bpred->btb[set][way].dest = uop->neip;
			}
		}
	}
	
	/* If address was found, update LRU counters */
	if (found_way >= 0) {
		for (way = 0; way < bpred_btb_assoc; way++)
			if (bpred->btb[set][way].counter > bpred->btb[set][found_way].counter)
				bpred->btb[set][way].counter--;
		bpred->btb[set][found_way].counter = bpred_btb_assoc - 1;
	}
}


void bpred_reg_options()
{
	static char *bpred_kind_map[] = { "perfect", "taken", "nottaken", "comb" };
	opt_reg_enum("-bpred", "branch predictor kind {perfect|taken|nottaken|comb}",
		(int *) &bpred_kind, bpred_kind_map, 4);
	opt_reg_string("-bpred:btb", "branch predictor btb configuration (<sets>:<assoc>)", &bpred_btb);
	opt_reg_uint32("-bpred:ras", "return address stack size", &bpred_ras_size);
	opt_reg_uint32("-bpred:bimod", "BHT size for bimodal component", &bpred_bimod_size);
	opt_reg_uint32("-bpred:gshare", "BHT for gshare component", &bpred_gshare_size);
	opt_reg_uint32("-bpred:choice", "BHT size for choice predictor", &bpred_choice_size);
}


void bpred_init()
{
	int core, thread;
	
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
	
	/* Integrity */
	if (bpred_bimod_size & (bpred_bimod_size - 1))
		fatal("bpred:bimod must be power of 2");
	if (bpred_gshare_size & (bpred_gshare_size - 1))
		fatal("bpred:gshare must be power of 2");
	if (bpred_choice_size & (bpred_choice_size - 1))
		fatal("bpred:choice must be power of 2");
	if (sscanf(bpred_btb, "%d:%d", &bpred_btb_sets, &bpred_btb_assoc) != 2)
		fatal("invalid bpred:btb format");
	if (bpred_btb_sets & (bpred_btb_sets - 1))
		fatal("number of btb sets must be power of 2");
	if (bpred_btb_assoc & (bpred_btb_assoc - 1))
		fatal("btb associativity must be power of 2");
	
	/* Create bpred */
	bpred = calloc(1, sizeof(struct bpred_t));
	bpred->ras = calloc(bpred_ras_size, sizeof(uint32_t));
	bpred->bimod = calloc(bpred_bimod_size, sizeof(char));
	bpred->gshare = calloc(bpred_gshare_size, sizeof(char));
	bpred->choice = calloc(bpred_choice_size, sizeof(char));
	
	/* BTB */
	bpred->btb = calloc(bpred_btb_sets, sizeof(struct btb_entry_t *));
	for (i = 0; i < bpred_btb_sets; i++) {
		bpred->btb[i] = calloc(bpred_btb_assoc, sizeof(struct btb_entry_t));
		for (j = 0; j < bpred_btb_assoc; j++)
			bpred->btb[i][j].counter = j;	/* Assign LRU counters */
	}
	
	/* Initialize tables */
	for (i = 0; i < bpred_bimod_size; i++)
		bpred->bimod[i] = 2;
	for (i = 0; i < bpred_gshare_size; i++)
		bpred->gshare[i] = 2;
	for (i = 0; i < bpred_choice_size; i++)
		bpred->choice[i] = 2;
	return bpred;
}


void bpred_free(struct bpred_t *bpred)
{
	int i;

	/* Print stats */
	fprintf(stderr, "%s.accesses  %lld  # Accesses while in non speculative mode\n",
		bpred->name, (long long) bpred->accesses);
	fprintf(stderr, "%s.hits  %lld  # Correct branch predictions\n",
		bpred->name, (long long) bpred->hits);
	fprintf(stderr, "%s.acc  %.4f  # Prediction accuracy\n",
		bpred->name, bpred->accesses ? (double) bpred->hits / bpred->accesses : 0.0);

	/* Bree BTB */
	for (i = 0; i < bpred_btb_sets; i++)
		free(bpred->btb[i]);
	free(bpred->btb);
	
	/* Bree bpred */
	free(bpred->ras);
	free(bpred->bimod);
	free(bpred->gshare);
	free(bpred->choice);
	free(bpred);
}


uint32_t bpred_lookup(struct bpred_t *bpred, struct uop_t *uop)
{
	/* If instruction is not a branch */
	if (!(uop->flags & FCTRL))
		return uop->neip;
	
	/* If our predictor kind is 'perfect', return the correct address */
	if (bpred_kind == bpred_kind_perfect)
		return uop->neip;
	
	/* If instruction is not speculative, we can think of using the RAS.
	 * If it is a call, push to RAS; if it is a return, pop from RAS. */
	if (!uop->specmode) {
		if (uop->flags & FCALL)
			bpred_ras_push(bpred, uop);
		if (uop->flags & FRET)
			return bpred_ras_pop(bpred, uop);
	}
	
	
	/* Actions depending on predictor kind */
	switch (bpred_kind) {
	
		case bpred_kind_nottaken:
			return uop->eip + uop->size;
	
		case bpred_kind_taken:
			return bpred_btb_lookup(bpred, uop);
	
		case bpred_kind_comb:
		{
			int taken;

			uop->bimod_idx = uop->eip & (bpred_bimod_size - 1);
			uop->bimod_taken = bpred->bimod[uop->bimod_idx] > 1;
			
			uop->gshare_idx = uop->eip ^ bpred->gshare_bhr;
			uop->gshare_idx &= bpred_gshare_size - 1;
			uop->gshare_taken = bpred->gshare[uop->gshare_idx] > 1;
			bpred->gshare_bhr = (bpred->gshare_bhr << 1) |
				(uop->gshare_taken ? 1 : 0);

			uop->choice_idx = uop->eip & (bpred_choice_size - 1);
			uop->choice_value = bpred->choice[uop->choice_idx];

			taken = uop->choice_value < 2 ? uop->bimod_taken :
				uop->gshare_taken;
			
			return taken ? bpred_btb_lookup(bpred, uop)
				: uop->eip + uop->size;
		}
		
		default:
			panic("unknown predictor");
			return 0;
	}
}


void bpred_update(struct bpred_t *bpred, struct uop_t *uop)
{
	/* Update BTB except if instr is a RET */
	assert(!uop->specmode);
	assert(uop->flags & FCTRL);
	if (!(uop->flags & FRET))
		bpred_btb_update(bpred, uop);
	
	/* Update bimodal predictor only if inst is neither CALL nor RET */
	if (bpred_kind == bpred_kind_comb &&
		!(uop->flags & FCALL) &&
		!(uop->flags & FRET))
	{

		int taken = uop->neip != uop->eip + uop->size;

		/* Update bimodal predictor */
		bpred->bimod[uop->bimod_idx] = taken ?
			MIN(bpred->bimod[uop->bimod_idx] + 1, 3) :
			MAX(bpred->bimod[uop->bimod_idx] - 1, 0);

		/* Update gshare predictor */
		bpred->gshare[uop->gshare_idx] = taken ?
			MIN(bpred->gshare[uop->gshare_idx] + 1, 3) :
			MAX(bpred->gshare[uop->gshare_idx] - 1, 0);

		/* Update choice predictor */
		if (uop->bimod_taken != uop->gshare_taken) {
			bpred->choice[uop->choice_idx] =
				uop->bimod_taken == taken ?
				MAX(bpred->choice[uop->choice_idx] - 1, 0) :
				MIN(bpred->choice[uop->choice_idx] + 1, 3);
		}
	}

	/* Stats */
	bpred->accesses++;
	if (uop->neip == uop->pred_neip)
		bpred->hits++;
}
