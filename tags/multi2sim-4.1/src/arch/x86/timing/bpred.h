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


/* Forward types */
struct x86_uop_t;


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


void x86_bpred_init(void);
void x86_bpred_done(void);

struct x86_bpred_t *x86_bpred_create(char *name);
void x86_bpred_free(struct x86_bpred_t *bpred);
int x86_bpred_lookup(struct x86_bpred_t *bpred, struct x86_uop_t *uop);
int x86_bpred_lookup_multiple(struct x86_bpred_t *bpred, unsigned int eip, int count);
void x86_bpred_update(struct x86_bpred_t *bpred, struct x86_uop_t *uop);

unsigned int x86_bpred_btb_lookup(struct x86_bpred_t *bpred, struct x86_uop_t *uop);
void x86_bpred_btb_update(struct x86_bpred_t *bpred, struct x86_uop_t *uop);
unsigned int x86_bpred_btb_next_branch(struct x86_bpred_t *bpred, unsigned int eip, unsigned int bsize);


#endif

