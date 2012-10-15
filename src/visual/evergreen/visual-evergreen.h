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


#ifndef VISUAL_EVERGREEN_H
#define VISUAL_EVERGREEN_H


#include <visual-common.h>



/*
 * Work-Group
 */

struct vi_evg_work_group_t
{
	char *name;

	int id;

	int work_item_id_first;
	int work_item_count;

	int wavefront_id_first;
	int wavefront_count;
};

struct vi_evg_work_group_t *vi_evg_work_group_create(char *name, int id, int work_item_id_first,
	int work_item_count, int wavefront_id_first, int wavefront_count);
void vi_evg_work_group_free(struct vi_evg_work_group_t *work_group);

void vi_evg_work_group_get_name_short(char *work_group_name, char *buf, int size);
void vi_evg_work_group_get_desc(char *work_group_name, char *buf, int size);

void vi_evg_work_group_read_checkpoint(struct vi_evg_work_group_t *work_group, FILE *f);
void vi_evg_work_group_write_checkpoint(struct vi_evg_work_group_t *work_group, FILE *f);




/*
 * Instruction
 */

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




/*
 * Compute Unit
 */

struct vi_evg_compute_unit_t
{
	char *name;

	struct hash_table_t *work_group_table;
	struct hash_table_t *inst_table;

	/* Number of instructions created */
	long long num_insts;
};

struct vi_evg_compute_unit_t *vi_evg_compute_unit_create(char *name);
void vi_evg_compute_unit_free(struct vi_evg_compute_unit_t *compute_unit);

void vi_evg_compute_unit_read_checkpoint(struct vi_evg_compute_unit_t *compute_unit, FILE *f);
void vi_evg_compute_unit_write_checkpoint(struct vi_evg_compute_unit_t *compute_unit, FILE *f);



/*
 * GPU
 */

struct vi_evg_gpu_t
{
	struct list_t *compute_unit_list;
};


extern struct vi_evg_gpu_t *vi_evg_gpu;

void vi_evg_gpu_init(void);
void vi_evg_gpu_done(void);




/*
 * Panel
 */

struct vi_evg_panel_t;

struct vi_evg_panel_t *vi_evg_panel_create(void);
void vi_evg_panel_free(struct vi_evg_panel_t *panel);

void vi_evg_panel_refresh(struct vi_evg_panel_t *panel);

GtkWidget *vi_evg_panel_get_widget(struct vi_evg_panel_t *panel);




/*
 * Time Diagram
 */

struct vi_evg_time_dia_t;

struct vi_evg_time_dia_t *vi_evg_time_dia_create(struct vi_evg_compute_unit_t *compute_unit);
void vi_evg_time_dia_free(struct vi_evg_time_dia_t *time_dia);

GtkWidget *vi_evg_time_dia_get_widget(struct vi_evg_time_dia_t *time_dia);

void vi_evg_time_dia_go_to_cycle(struct vi_evg_time_dia_t *time_dia, long long cycle);
void vi_evg_time_dia_refresh(struct vi_evg_time_dia_t *time_dia);




#endif
