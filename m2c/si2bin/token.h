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

#ifndef M2C_SI2BIN_TOKEN_H
#define M2C_SI2BIN_TOKEN_H

enum si2bin_token_type_t
{
	si2bin_token_invalid = 0,

	si2bin_token_64_sdst,
	si2bin_token_64_ssrc0,
	si2bin_token_64_ssrc1,
	si2bin_token_64_src0,
	si2bin_token_64_src1,
	si2bin_token_64_src2,
	si2bin_token_64_svdst,
	si2bin_token_64_vdst,
	si2bin_token_label,
	si2bin_token_mt_maddr,
	si2bin_token_mt_series_vdata,
	si2bin_token_offset,
	si2bin_token_sdst,
	si2bin_token_series_sbase,
	si2bin_token_series_sdst,
	si2bin_token_series_srsrc,
	si2bin_token_simm16,
	si2bin_token_smrd_sdst,
	si2bin_token_src0,
	si2bin_token_src1,
	si2bin_token_src2,
	si2bin_token_ssrc0,
	si2bin_token_ssrc1,
	si2bin_token_vaddr,
	si2bin_token_vcc,
	si2bin_token_vdst,
	si2bin_token_svdst,
	si2bin_token_vop3_64_svdst,
	si2bin_token_vop3_src0,
	si2bin_token_vop3_src1,
	si2bin_token_vop3_src2,
	si2bin_token_vop3_64_src0,
	si2bin_token_vop3_64_src1,
	si2bin_token_vop3_64_src2,
	si2bin_token_vop3_64_sdst,
	si2bin_token_vop3_vdst,
	si2bin_token_vop3_64_vdst,
	si2bin_token_vsrc0,
	si2bin_token_vsrc1,
	si2bin_token_wait_cnt,
	si2bin_token_addr,
	si2bin_token_data0,
	si2bin_token_ds_vdst,

	si2bin_token_count
};

extern struct str_map_t si2bin_token_map;

struct si2bin_token_t
{
	enum si2bin_token_type_t type;
};

struct si2bin_token_t *si2bin_token_create(enum si2bin_token_type_t type);
void si2bin_token_free(struct si2bin_token_t *token);

struct si2bin_arg_t;
int si2bin_token_is_arg_allowed(struct si2bin_token_t *token, struct si2bin_arg_t *arg);

#endif

