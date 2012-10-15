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

#ifndef VISUAL_EVERGREEN_INST_H
#define VISUAL_EVERGREEN_INST_H


enum vi_evg_inst_cat_t
{
	vi_evg_inst_cat_invalid = 0,
	vi_evg_inst_cat_cf,
	vi_evg_inst_cat_alu,
	vi_evg_inst_cat_tex
};

enum vi_evg_inst_stage_t
{
	vi_evg_inst_stage_invalid = 0,

	vi_evg_inst_stage_cf_fetch,
	vi_evg_inst_stage_cf_decode,
	vi_evg_inst_stage_cf_execute,
	vi_evg_inst_stage_cf_complete,

	vi_evg_inst_stage_alu_fetch,
	vi_evg_inst_stage_alu_decode,
	vi_evg_inst_stage_alu_read,
	vi_evg_inst_stage_alu_execute,
	vi_evg_inst_stage_alu_write,

	vi_evg_inst_stage_tex_fetch,
	vi_evg_inst_stage_tex_decode,
	vi_evg_inst_stage_tex_read,
	vi_evg_inst_stage_tex_write,

	vi_evg_inst_stage_count
};

struct vi_evg_inst_t
{
	char *name;

	char *asm_code;
	char *asm_code_x;
	char *asm_code_y;
	char *asm_code_z;
	char *asm_code_w;
	char *asm_code_t;

	long long id;

	int compute_unit_id;
	int work_group_id;
	int wavefront_id;

	enum vi_evg_inst_cat_t cat;
	enum vi_evg_inst_stage_t stage;
};

extern struct str_map_t vi_evg_inst_cat_map;
extern struct str_map_t vi_evg_inst_stage_map;
extern struct str_map_t vi_evg_inst_stage_name_map;
extern struct str_map_t vi_evg_inst_stage_color_map;

struct vi_evg_inst_t *vi_evg_inst_create(char *name, long long id,
	int compute_unit_id, int work_group_id, int wavefront_id,
	enum vi_evg_inst_cat_t cat, enum vi_evg_inst_stage_t stage,
	char *asm_code, char *asm_code_x, char *asm_code_y,
	char *asm_code_z, char *asm_code_w, char *asm_code_t);
void vi_evg_inst_free(struct vi_evg_inst_t *inst);

void vi_evg_inst_get_markup(struct vi_evg_inst_t *inst, char *buf, int size);

void vi_evg_inst_read_checkpoint(struct vi_evg_inst_t *inst, FILE *f);
void vi_evg_inst_write_checkpoint(struct vi_evg_inst_t *inst, FILE *f);


#endif

