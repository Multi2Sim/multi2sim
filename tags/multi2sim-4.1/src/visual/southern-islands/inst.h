/*
 *  Multi2Sim Tools
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


#ifndef VISUAL_SOUTHERN_ISLANDS_INST_H
#define VISUAL_SOUTHERN_ISLANDS_INST_H


enum vi_si_inst_stage_t
{
	vi_si_inst_stage_invalid = 0,

	vi_si_inst_stage_stalled,

	vi_si_inst_stage_fetch,
	vi_si_inst_stage_issue,

	vi_si_inst_stage_branch_decode,
	vi_si_inst_stage_branch_read,
	vi_si_inst_stage_branch_execute,
	vi_si_inst_stage_branch_writeback,

	vi_si_inst_stage_scalar_decode,
	vi_si_inst_stage_scalar_read,
	vi_si_inst_stage_scalar_execute,
	vi_si_inst_stage_scalar_memory,
	vi_si_inst_stage_scalar_writeback,

	vi_si_inst_stage_simd_decode,
	vi_si_inst_stage_simd_execute,

	vi_si_inst_stage_mem_decode,
	vi_si_inst_stage_mem_read,
	vi_si_inst_stage_mem_memory,
	vi_si_inst_stage_mem_writeback,

	vi_si_inst_stage_lds_decode,
	vi_si_inst_stage_lds_read,
	vi_si_inst_stage_lds_memory,
	vi_si_inst_stage_lds_writeback,

	vi_si_inst_stage_count
};

struct vi_si_inst_t
{
	char *name;

	char *asm_code;

	long long id;

	int compute_unit_id;
	int wfp_id; /* Wavefront pool ID */
	int work_group_id;
	int wavefront_id;
	long long int uop_id_in_wavefront;

	enum vi_si_inst_stage_t stage;
};

extern struct str_map_t vi_si_inst_stage_map;
extern struct str_map_t vi_si_inst_stage_name_map;
extern struct str_map_t vi_si_inst_stage_color_map;

struct vi_si_inst_t *vi_si_inst_create(char *name, long long id,
	int compute_unit_id, int wfp_id, int work_group_id, 
	int wavefront_id, enum vi_si_inst_stage_t stage, 
	long long int uop_id_in_wavefront, char *asm_code);
void vi_si_inst_free(struct vi_si_inst_t *inst);

void vi_si_inst_get_markup(struct vi_si_inst_t *inst, char *buf, int size);

void vi_si_inst_read_checkpoint(struct vi_si_inst_t *inst, FILE *f);
void vi_si_inst_write_checkpoint(struct vi_si_inst_t *inst, FILE *f);


#endif

