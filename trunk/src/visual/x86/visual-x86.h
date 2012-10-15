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


#ifndef VISUAL_X86_H
#define VISUAL_X86_H

#include <visual-common.h>



/*
 * Context
 */

#define VI_X86_CONTEXT_EMPTY   ((void *) 1)

struct vi_x86_context_t
{
	char *name;
	int id;

	/* Map */
	int core_id;
	int thread_id;
	int parent_id;

	long long creation_cycle;
};

struct vi_x86_context_t *vi_x86_context_create(char *name, int id, int parent_id);
void vi_x86_context_free(struct vi_x86_context_t *context);

void vi_x86_context_get_name_short(char *context_name, char *buf, int size);
void vi_x86_context_get_desc(char *context_name, char *buf, int size);

void vi_x86_context_read_checkpoint(struct vi_x86_context_t *context, FILE *f);
void vi_x86_context_write_checkpoint(struct vi_x86_context_t *context, FILE *f);




/*
 * Instruction
 */

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




/*
 * Core
 */

struct vi_x86_core_t
{
	char *name;

	/* Hash table of contexts.
	 * Only names are stored. The elements in the table are all
	 * 'VI_X86_CONTEXT_EMPTY'. */
	struct hash_table_t *context_table;

	/* Hash table of instructions.
	 * Each element is of type 'struct vi_x86_inst_t'. */
	struct hash_table_t *inst_table;

	/* Number of instructions */
	long long num_insts;
};

struct vi_x86_core_t *vi_x86_core_create(char *name);
void vi_x86_core_free(struct vi_x86_core_t *core);

void vi_x86_core_read_checkpoint(struct vi_x86_core_t *core, FILE *f);
void vi_x86_core_write_checkpoint(struct vi_x86_core_t *core, FILE *f);




/*
 * CPU
 */

struct vi_x86_cpu_t
{
	struct list_t *core_list;

	/* Hash table of contexts.
	 * Elements are of type 'struct vi_x86_context_t' */
	struct hash_table_t *context_table;
};


extern struct vi_x86_cpu_t *vi_x86_cpu;

void vi_x86_cpu_init(void);
void vi_x86_cpu_done(void);




/*
 * Timing Diagram
 */

struct vi_x86_time_dia_t;

struct vi_x86_time_dia_t *vi_x86_time_dia_create(struct vi_x86_core_t *core);
void vi_x86_time_dia_free(struct vi_x86_time_dia_t *time_dia);

GtkWidget *vi_x86_time_dia_get_widget(struct vi_x86_time_dia_t *time_dia);

void vi_x86_time_dia_refresh(struct vi_x86_time_dia_t *time_dia);
void vi_x86_time_dia_go_to_cycle(struct vi_x86_time_dia_t *time_dia, long long cycle);




/*
 * Panel
 */

struct vi_x86_panel_t *vi_x86_panel_create(void);
void vi_x86_panel_free(struct vi_x86_panel_t *panel);

void vi_x86_panel_refresh(struct vi_x86_panel_t *panel);

GtkWidget *vi_x86_panel_get_widget(struct vi_x86_panel_t *panel);




#endif
