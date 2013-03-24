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

#ifndef TOOLS_SI_AS_FORMAL_ARG_H
#define TOOLS_SI_AS_FORMAL_ARG_H

enum si_token_type_t
{
	si_token_invalid = 0,

	si_token_64_sdst,
	si_token_64_ssrc0,
	si_token_64_ssrc1,
	si_token_64_src0,
	si_token_64_src1,
	si_token_64_src2,
	si_token_64_svdst,
	si_token_64_vdst,
	si_token_label,
	si_token_mt_maddr,
	si_token_mt_series_vdata,
	si_token_offset,
	si_token_sdst,
	si_token_series_sbase,
	si_token_series_sdst,
	si_token_series_srsrc,
	si_token_simm16,
	si_token_smrd_sdst,
	si_token_src0,
	si_token_src1,
	si_token_src2,
	si_token_ssrc0,
	si_token_ssrc1,
	si_token_vaddr,
	si_token_vcc,
	si_token_vdst,
	si_token_vop3_64_svdst,
	si_token_vop3_src0,
	si_token_vop3_src1,
	si_token_vop3_src2,
	si_token_vop3_vdst,
	si_token_vsrc0,
	si_token_vsrc1,
	si_token_wait_cnt,

	si_token_count
};

extern struct str_map_t si_token_map;

struct si_token_t
{
	enum si_token_type_t type;
};

struct si_token_t *si_token_create(enum si_token_type_t type);
void si_token_free(struct si_token_t *token);

struct si_arg_t;
int si_token_is_arg_allowed(struct si_token_t *token, struct si_arg_t *arg);

#endif

