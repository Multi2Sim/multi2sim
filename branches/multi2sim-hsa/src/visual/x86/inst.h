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

#ifndef VISUAL_X86_INST_H
#define VISUAL_X86_INST_H


enum vi_x86_inst_stage_t
{
	vi_x86_inst_stage_invalid = 0,

	vi_x86_inst_stage_fetch,
	vi_x86_inst_stage_decode,
	vi_x86_inst_stage_dispatch,
	vi_x86_inst_stage_issue,
	vi_x86_inst_stage_writeback,
	vi_x86_inst_stage_commit,
	vi_x86_inst_stage_squash,

	vi_x86_inst_stage_count
};

extern struct str_map_t vi_x86_inst_stage_map;
extern struct str_map_t vi_x86_inst_stage_name_map;
extern struct str_map_t vi_x86_inst_stage_color_map;

struct vi_x86_inst_t
{
	long long id;

	int spec_mode;

	char *name;

	char *asm_code;
	char *asm_micro_code;

	enum vi_x86_inst_stage_t stage;
};

struct vi_x86_inst_t *vi_x86_inst_create(long long id, char *name,
	char *asm_code, char *asm_micro_code, int spec_mode,
	enum vi_x86_inst_stage_t stage);
void vi_x86_inst_free(struct vi_x86_inst_t *inst);

void vi_x86_inst_read_checkpoint(struct vi_x86_inst_t *inst, FILE *f);
void vi_x86_inst_write_checkpoint(struct vi_x86_inst_t *inst, FILE *f);


#endif

