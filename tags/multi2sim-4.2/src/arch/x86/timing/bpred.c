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


#include <lib/mhandle/mhandle.h>
#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "bpred.h"
#include "thread.h"
#include "uop.h"




/*
 * Public functions
 */


void X86ThreadInitBranchPred(X86Thread *self)
{
	char name[MAX_STRING_SIZE];

	snprintf(name, sizeof name, "%s.bpred", self->name);
	self->bpred = x86_bpred_create(name);
}


void X86ThreadFreeBranchPred(X86Thread *self)
{
	x86_bpred_free(self->bpred);
}


/* Return prediction for an address (0=not taken, 1=taken) */
int X86ThreadLookupBranchPred(X86Thread *self, struct x86_uop_t *uop)
{
	struct x86_bpred_t *bpred = self->bpred;

	/* If branch predictor is accessed, a BTB hit must have occurred before, which
	 * provides information about the branch, i.e., target address and whether it
	 * is a call, ret, jump, or conditional branch. Thus, branches other than
	 * conditional ones are always predicted taken. */
	assert(uop->flags & X86_UINST_CTRL);
	if (uop->flags & X86_UINST_UNCOND)
	{
		uop->pred = 1;
		return 1;
	}

	/* An internal branch (string operations) is always predicted taken */
	if (uop->uinst->opcode == x86_uinst_ibranch)
	{
		uop->pred = 1;
		return 1;
	}

	/* Perfect predictor */
	if (x86_bpred_kind == x86_bpred_kind_perfect)
		uop->pred = uop->neip != uop->eip + uop->mop_size;
	
	/* Taken predictor */
	if (x86_bpred_kind == x86_bpred_kind_taken)
		uop->pred = 1;
	
	/* Not-taken predictor */
	if (x86_bpred_kind == x86_bpred_kind_nottaken)
		uop->pred = 0;
	
	/* Bimodal predictor */
	if (x86_bpred_kind == x86_bpred_kind_bimod || x86_bpred_kind == x86_bpred_kind_comb)
	{
		uop->bimod_index = uop->eip & (x86_bpred_bimod_size - 1);
		uop->bimod_pred = bpred->bimod[uop->bimod_index] > 1;
		uop->pred = uop->bimod_pred;
	}
	
	/* Two-level adaptive */
	if (x86_bpred_kind == x86_bpred_kind_twolevel || x86_bpred_kind == x86_bpred_kind_comb)
	{
		uop->twolevel_bht_index = uop->eip & (x86_bpred_twolevel_l1size - 1);
		uop->twolevel_pht_row = bpred->twolevel_bht[uop->twolevel_bht_index];
		assert(uop->twolevel_pht_row < x86_bpred_twolevel_l2height);
		uop->twolevel_pht_col = uop->eip & (x86_bpred_twolevel_l2size - 1);
		uop->twolevel_pred = bpred->twolevel_pht[uop->twolevel_pht_row *
			x86_bpred_twolevel_l2size + uop->twolevel_pht_col] > 1;
		uop->pred = uop->twolevel_pred;
	}

	/* Combined */
	if (x86_bpred_kind == x86_bpred_kind_comb)
	{
		uop->choice_index = uop->eip & (x86_bpred_choice_size - 1);
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
int X86ThreadLookupBranchPredMultiple(X86Thread *self, unsigned int eip, int count)
{
	struct x86_bpred_t *bpred = self->bpred;

	int i, pred, temp_pred;
	unsigned int bht_index, pht_col;
	unsigned int bhr;  /* branch history register = pht_row */

	/* First make a regular prediction. This updates the necessary fields in the
	 * uop for a later call to X86ThreadUpdateBranchPred, and makes the first prediction
	 * considering known characteristics of the primary branch. */
	assert(x86_bpred_kind == x86_bpred_kind_twolevel);
	bht_index = eip & (x86_bpred_twolevel_l1size - 1);
	bhr = bpred->twolevel_bht[bht_index];
	assert(bhr < x86_bpred_twolevel_l2height);
	pht_col = eip & (x86_bpred_twolevel_l2size - 1);
	pred = temp_pred = bpred->twolevel_pht[bhr * x86_bpred_twolevel_l2size + pht_col] > 1;

	/* Make the rest of predictions */
	for (i = 1; i < count; i++)
	{
		bhr = ((bhr << 1) | temp_pred) & (x86_bpred_twolevel_l2height - 1);
		temp_pred = bpred->twolevel_pht[bhr * x86_bpred_twolevel_l2size + pht_col] > 1;
		assert(!temp_pred || temp_pred == 1);
		pred |= temp_pred << i;
	}

	/* Return */
	return pred;
}


void X86ThreadUpdateBranchPred(X86Thread *self, struct x86_uop_t *uop)
{
	struct x86_bpred_t *bpred = self->bpred;
	int taken;
	char *pctr;  /* pointer to 2-bit counter */
	unsigned int *pbhr;  /* pointer to branch history register */

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
	if (x86_bpred_kind == x86_bpred_kind_perfect)
		return;
	if (uop->flags & X86_UINST_UNCOND)
		return;
	
	/* Bimodal predictor was used */
	if (x86_bpred_kind == x86_bpred_kind_bimod || 
		(x86_bpred_kind == x86_bpred_kind_comb && !uop->choice_pred))
	{
		pctr = &bpred->bimod[uop->bimod_index];
		*pctr = taken ? MIN(*pctr + 1, 3) : MAX(*pctr - 1, 0);
	}

	/* Two-level adaptive predictor was used */
	if (x86_bpred_kind == x86_bpred_kind_twolevel ||
		(x86_bpred_kind == x86_bpred_kind_comb && uop->choice_pred))
	{
		/* Shift entry in BHT (level 1), and append direction */
		pbhr = &bpred->twolevel_bht[uop->twolevel_bht_index];
		*pbhr = ((*pbhr << 1) | taken) & (x86_bpred_twolevel_l2height - 1);

		/* Update counter in PHT (level 2) as per direction */
		pctr = &bpred->twolevel_pht[uop->twolevel_pht_row *
			x86_bpred_twolevel_l2size + uop->twolevel_pht_col];
		*pctr = taken ? MIN(*pctr + 1, 3) : MAX(*pctr - 1, 0);
	}

	/* Choice predictor - update only if bimodal and two-level
	 * predictions differ. */
	if (x86_bpred_kind == x86_bpred_kind_comb && uop->bimod_pred != uop->twolevel_pred) {
		pctr = &bpred->choice[uop->choice_index];
		*pctr = uop->bimod_pred == taken ? MAX(*pctr - 1, 0) : MIN(*pctr + 1, 3);
	}
}


/* Lookup BTB. If it contains the uop address, return target. The BTB also contains
 * information about the type of branch, i.e., jump, call, ret, or conditional. If
 * instruction is call or ret, access RAS instead of BTB. */
unsigned int X86ThreadLookupBTB(X86Thread *self, struct x86_uop_t *uop)
{
	struct x86_bpred_t *bpred = self->bpred;
	struct x86_bpred_btb_entry_t *entry;
	unsigned int way, set, target = 0;
	int hit = 0;

	assert(uop->flags & X86_UINST_CTRL);

	/* Perfect branch predictor */
	if (x86_bpred_kind == x86_bpred_kind_perfect)
		return uop->neip;

	/* Internal branch (string operations) always predicted to jump to itself */
	if (uop->uinst->opcode == x86_uinst_ibranch)
		return uop->eip;

	/* Search address in BTB */
	set = uop->eip & (x86_bpred_btb_sets - 1);
	for (way = 0; way < x86_bpred_btb_assoc; way++)
	{
		entry = X86_BPRED_BTB_ENTRY(set, way);
		if (entry->source != uop->eip)
			continue;
		target = entry->target;
		hit = 1;
		break;
	}
	
	/* If there was a hit, we know whether branch is a call.
	 * In this case, push return address into RAS. To avoid
	 * updates at recovery, do it only for non-spec instructions. */
	if (hit && uop->uinst->opcode == x86_uinst_call && !uop->specmode)
	{
		bpred->ras[bpred->ras_index] = uop->eip + uop->mop_size;
		bpred->ras_index = (bpred->ras_index + 1) % x86_bpred_ras_size;
	}

	/* If there was a hit, we know whether branch is a ret. In this case,
	 * pop target from the RAS, and ignore target obtained from BTB. */
	if (hit && uop->uinst->opcode == x86_uinst_ret && !uop->specmode)
	{
		bpred->ras_index = (bpred->ras_index + x86_bpred_ras_size - 1) % x86_bpred_ras_size;
		target = bpred->ras[bpred->ras_index];
	}

	/* Return */
	return target;
}


/* Update BTB */
void X86ThreadUpdateBTB(X86Thread *self, struct x86_uop_t *uop)
{
	struct x86_bpred_t *bpred = self->bpred;
	struct x86_bpred_btb_entry_t *entry, *found = NULL;
	int way, set;

	/* No update for perfect branch predictor */
	if (x86_bpred_kind == x86_bpred_kind_perfect)
		return;
	
	/* Search address in BTB */
	set = uop->eip & (x86_bpred_btb_sets - 1);
	for (way = 0; way < x86_bpred_btb_assoc; way++)
	{
		entry = X86_BPRED_BTB_ENTRY(set, way);
		if (entry->source == uop->eip)
		{
			found = entry;
			break;
		}
	}
	
	/* If address was not found, evict LRU entry */
	if (!found)
	{
		for (way = 0; way < x86_bpred_btb_assoc; way++)
		{
			entry = X86_BPRED_BTB_ENTRY(set, way);
			entry->counter--;
			if (entry->counter < 0) {
				entry->counter = x86_bpred_btb_assoc - 1;
				entry->source = uop->eip;
				entry->target = uop->neip;
			}
		}
	}
	
	/* If address was found, update LRU counters and target */
	if (found)
	{
		for (way = 0; way < x86_bpred_btb_assoc; way++)
		{
			entry = X86_BPRED_BTB_ENTRY(set, way);
			if (entry->counter > found->counter)
				entry->counter--;
		}
		found->counter = x86_bpred_btb_assoc - 1;
		found->target = uop->neip;
	}
}


/* Find address of next branch after eip within current block.
 * This is useful for accessing the trace
 * cache. At that point, the uop is not ready to call X86ThreadLookupBTB, since
 * functional simulation has not happened yet. */
unsigned int X86ThreadGetNextBranch(X86Thread *self, unsigned int eip, unsigned int bsize)
{
	struct x86_bpred_t *bpred = self->bpred;
	struct x86_bpred_btb_entry_t *entry;
	unsigned int limit;
	int set, way;

	assert(!(bsize & (bsize - 1)));
	limit = (eip + bsize) & ~(bsize - 1);
	while (eip < limit)
	{
		set = eip & (x86_bpred_btb_sets - 1);
		for (way = 0; way < x86_bpred_btb_assoc; way++)
		{
			entry = X86_BPRED_BTB_ENTRY(set, way);
			if (entry->source == eip)
				return eip;
		}
		eip++;
	}
	return 0;
}




/*
 * Object 'x86_bpred_t'
 */

struct x86_bpred_t *x86_bpred_create(char *name)
{
	struct x86_bpred_t *bpred;

	int i;
	int j;

	/* Initialize */
	bpred = xcalloc(1, sizeof(struct x86_bpred_t));
	bpred->name = xstrdup(name);
	bpred->ras = xcalloc(x86_bpred_ras_size, sizeof(unsigned int));

	/* Bimodal predictor */
	if (x86_bpred_kind == x86_bpred_kind_bimod || x86_bpred_kind == x86_bpred_kind_comb)
	{
		bpred->bimod = xcalloc(x86_bpred_bimod_size, sizeof(char));
		for (i = 0; i < x86_bpred_bimod_size; i++)
			bpred->bimod[i] = 2;
	}

	/* Two-level adaptive branch predictor */
	if (x86_bpred_kind == x86_bpred_kind_twolevel || x86_bpred_kind == x86_bpred_kind_comb)
	{
		bpred->twolevel_bht = xcalloc(x86_bpred_twolevel_l1size, sizeof(unsigned int));
		bpred->twolevel_pht = xcalloc(x86_bpred_twolevel_l2size * x86_bpred_twolevel_l2height, sizeof(char));
		for (i = 0; i < x86_bpred_twolevel_l2size * x86_bpred_twolevel_l2height; i++)
			bpred->twolevel_pht[i] = 2;
	}

	/* Choice predictor */
	if (x86_bpred_kind == x86_bpred_kind_comb)
	{
		bpred->choice = xcalloc(x86_bpred_choice_size, sizeof(char));
		for (i = 0; i < x86_bpred_choice_size; i++)
			bpred->choice[i] = 2;
	}

	/* Allocate BTB and assign LRU counters */
	bpred->btb = xcalloc(x86_bpred_btb_sets * x86_bpred_btb_assoc, sizeof(struct x86_bpred_btb_entry_t));
	for (i = 0; i < x86_bpred_btb_sets; i++)
		for (j = 0; j < x86_bpred_btb_assoc; j++)
			X86_BPRED_BTB_ENTRY(i, j)->counter = j;

	/* Return */
	return bpred;
}


void x86_bpred_free(struct x86_bpred_t *bpred)
{
	/* Bimodal table */
	if (x86_bpred_kind == x86_bpred_kind_bimod || x86_bpred_kind == x86_bpred_kind_comb)
		free(bpred->bimod);

	/* Two-level adaptive predictor tables */
	if (x86_bpred_kind == x86_bpred_kind_twolevel || x86_bpred_kind == x86_bpred_kind_comb) {
		free(bpred->twolevel_bht);
		free(bpred->twolevel_pht);
	}

	/* Choice table */
	if (x86_bpred_kind == x86_bpred_kind_comb)
		free(bpred->choice);

	/* Free */
	free(bpred->name);
	free(bpred->btb);
	free(bpred->ras);
	free(bpred);
}



/*
 * Public
 */

char *x86_bpred_kind_map[] = { "Perfect", "Taken", "NotTaken", "Bimodal", "TwoLevel", "Combined" };
enum x86_bpred_kind_t x86_bpred_kind;
int x86_bpred_btb_sets;  /* Number of BTB sets */
int x86_bpred_btb_assoc;  /* Number of BTB ways */
int x86_bpred_ras_size;  /* Return address stack size */
int x86_bpred_bimod_size;  /* Number of entries for bimodal predictor */
int x86_bpred_choice_size;  /* Number of entries for choice predictor */

int x86_bpred_twolevel_l1size;  /* Two-level adaptive predictor: level-1 size */
int x86_bpred_twolevel_l2size;  /* Two-level adaptive predictor: level-2 size */
int x86_bpred_twolevel_hist_size;  /* Two-level adaptive predictor: level-2 history size */
int x86_bpred_twolevel_l2height;


void X86ReadBranchPredConfig(struct config_t *config)
{
	char *section;

	section = "BranchPredictor";

	x86_bpred_kind = config_read_enum(config, section, "Kind",
			x86_bpred_kind_twolevel, x86_bpred_kind_map, 6);
	x86_bpred_btb_sets = config_read_int(config, section, "BTB.Sets", 256);
	x86_bpred_btb_assoc = config_read_int(config, section, "BTB.Assoc", 4);
	x86_bpred_bimod_size = config_read_int(config, section, "Bimod.Size", 1024);
	x86_bpred_choice_size = config_read_int(config, section, "Choice.Size", 1024);
	x86_bpred_ras_size = config_read_int(config, section, "RAS.Size", 32);
	x86_bpred_twolevel_l1size = config_read_int(config, section, "TwoLevel.L1Size", 1);
	x86_bpred_twolevel_l2size = config_read_int(config, section, "TwoLevel.L2Size", 1024);
	x86_bpred_twolevel_hist_size = config_read_int(config, section, "TwoLevel.HistorySize", 8);

	/* Two-level branch predictor parameter */
	x86_bpred_twolevel_l2height = 1 << x86_bpred_twolevel_hist_size;

	/* Integrity */
	if (x86_bpred_bimod_size & (x86_bpred_bimod_size - 1))
		fatal("number of entries in bimodal precitor must be a power of 2");
	if (x86_bpred_choice_size & (x86_bpred_choice_size - 1))
		fatal("number of entries in choice predictor must be power of 2");
	if (x86_bpred_btb_sets & (x86_bpred_btb_sets - 1))
		fatal("number of BTB sets must be a power of 2");
	if (x86_bpred_btb_assoc & (x86_bpred_btb_assoc - 1))
		fatal("BTB associativity must be a power of 2");

	if (x86_bpred_twolevel_hist_size < 1 || x86_bpred_twolevel_hist_size > 30)
		fatal("predictor history size must be >=1 and <=30");
	if (x86_bpred_twolevel_l1size & (x86_bpred_twolevel_l1size - 1))
		fatal("two-level predictor sizes must be power of 2");
	if (x86_bpred_twolevel_l2size & (x86_bpred_twolevel_l2size - 1))
		fatal("two-level predictor sizes must be power of 2");
}
