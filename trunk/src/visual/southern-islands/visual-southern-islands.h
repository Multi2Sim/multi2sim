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


#ifndef VISUAL_SOUTHERN_ISLANDS_H
#define VISUAL_SOUTHERN_ISLANDS_H


#include <visual-common.h>



/*
 * Work-Group
 */

struct vi_si_work_group_t
{
	char *name;

	int id;

	int work_item_id_first;
	int work_item_count;

	int wavefront_id_first;
	int wavefront_count;
};

struct vi_si_work_group_t *vi_si_work_group_create(char *name, int id, int work_item_id_first,
	int work_item_count, int wavefront_id_first, int wavefront_count);
void vi_si_work_group_free(struct vi_si_work_group_t *work_group);

void vi_si_work_group_get_name_short(char *work_group_name, char *buf, int size);
void vi_si_work_group_get_desc(char *work_group_name, char *buf, int size);

void vi_si_work_group_read_checkpoint(struct vi_si_work_group_t *work_group, FILE *f);
void vi_si_work_group_write_checkpoint(struct vi_si_work_group_t *work_group, FILE *f);




/*
 * Instruction
 */

enum vi_si_inst_stage_t
{
	vi_si_inst_stage_invalid = 0,

	vi_si_inst_stage_stalled,

	vi_si_inst_stage_fetch,
	vi_si_inst_stage_decode,

	vi_si_inst_stage_branch_read,
	vi_si_inst_stage_branch_execute,
	vi_si_inst_stage_branch_writeback,

	vi_si_inst_stage_scalar_read,
	vi_si_inst_stage_scalar_alu,
	vi_si_inst_stage_scalar_memory,
	vi_si_inst_stage_scalar_writeback,

	vi_si_inst_stage_simd_read,
	vi_si_inst_stage_simd_execute,
	vi_si_inst_stage_simd_writeback,

	vi_si_inst_stage_mem_read,
	vi_si_inst_stage_mem_execute,
	vi_si_inst_stage_mem_writeback,

	vi_si_inst_stage_lds_read,
	vi_si_inst_stage_lds_execute,
	vi_si_inst_stage_lds_writeback,

	vi_si_inst_stage_count
};

struct vi_si_inst_t
{
	char *name;

	char *asm_code;

	long long id;

	int compute_unit_id;
	int work_group_id;
	int wavefront_id;

	enum vi_si_inst_stage_t stage;
};

extern struct string_map_t vi_si_inst_stage_map;
extern struct string_map_t vi_si_inst_stage_name_map;
extern struct string_map_t vi_si_inst_stage_color_map;

struct vi_si_inst_t *vi_si_inst_create(char *name, long long id,
	int compute_unit_id, int work_group_id, int wavefront_id,
	enum vi_si_inst_stage_t stage, char *asm_code);
void vi_si_inst_free(struct vi_si_inst_t *inst);

void vi_si_inst_get_markup(struct vi_si_inst_t *inst, char *buf, int size);

void vi_si_inst_read_checkpoint(struct vi_si_inst_t *inst, FILE *f);
void vi_si_inst_write_checkpoint(struct vi_si_inst_t *inst, FILE *f);




/*
 * Compute Unit
 */

struct vi_si_compute_unit_t
{
	char *name;

	struct hash_table_t *work_group_table;
	struct hash_table_t *inst_table;

	/* Number of instructions created */
	long long num_insts;
};

struct vi_si_compute_unit_t *vi_si_compute_unit_create(char *name);
void vi_si_compute_unit_free(struct vi_si_compute_unit_t *compute_unit);

void vi_si_compute_unit_read_checkpoint(struct vi_si_compute_unit_t *compute_unit, FILE *f);
void vi_si_compute_unit_write_checkpoint(struct vi_si_compute_unit_t *compute_unit, FILE *f);



/*
 * GPU
 */

struct vi_si_gpu_t
{
	struct list_t *compute_unit_list;
};


extern struct vi_si_gpu_t *vi_si_gpu;

void vi_si_gpu_init(void);
void vi_si_gpu_done(void);




/*
 * Panel
 */

struct vi_si_panel_t;

struct vi_si_panel_t *vi_si_panel_create(void);
void vi_si_panel_free(struct vi_si_panel_t *panel);

void vi_si_panel_refresh(struct vi_si_panel_t *panel);

GtkWidget *vi_si_panel_get_widget(struct vi_si_panel_t *panel);




/*
 * Time Diagram
 */

struct vi_si_time_dia_t;

struct vi_si_time_dia_t *vi_si_time_dia_create(struct vi_si_compute_unit_t *compute_unit);
void vi_si_time_dia_free(struct vi_si_time_dia_t *time_dia);

GtkWidget *vi_si_time_dia_get_widget(struct vi_si_time_dia_t *time_dia);

void vi_si_time_dia_go_to_cycle(struct vi_si_time_dia_t *time_dia, long long cycle);
void vi_si_time_dia_refresh(struct vi_si_time_dia_t *time_dia);




#endif
