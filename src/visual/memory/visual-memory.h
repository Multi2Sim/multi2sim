/*
 *  Multi2Sim Tools
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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


#ifndef VISUAL_MEMORY_H
#define VISUAL_MEMORY_H

#include <visual-common.h>



/*
 * Memory System
 */

struct vi_mem_system_t
{
	struct hash_table_t *mod_table;
	struct hash_table_t *net_table;
	struct hash_table_t *access_table;
	struct list_t *mod_level_list;
};


extern struct vi_mem_system_t *vi_mem_system;


void vi_mem_system_init(void);
void vi_mem_system_done(void);




/*
 * Memory Module Access
 */

struct vi_mod_access_t
{
	char *name;
	char *state;

	unsigned int address;

	long long creation_cycle;
	long long state_update_cycle;
};

struct vi_mod_access_t *vi_mod_access_create(char *name, unsigned int address);
void vi_mod_access_free(struct vi_mod_access_t *access);

void vi_mod_access_set_state(struct vi_mod_access_t *access, char *state);

void vi_mod_access_read_checkpoint(struct vi_mod_access_t *access, FILE *f);
void vi_mod_access_write_checkpoint(struct vi_mod_access_t *access, FILE *f);

void vi_mod_access_get_name_short(char *access_name, char *buf, int size);
void vi_mod_access_get_name_long(char *access_name, char *buf, int size);
void vi_mod_access_get_desc(char *access_name, char *buf, int size);




/*
 * Memory Module
 */

#define VI_MOD_DIR_ENTRY_SHARERS_SIZE(mod) (((mod)->num_sharers + 7) / 8)
#define VI_MOD_DIR_ENTRY_SIZE(mod) (sizeof(struct vi_mod_dir_entry_t) + \
	VI_MOD_DIR_ENTRY_SHARERS_SIZE((mod)))

struct vi_mod_dir_entry_t
{
	int owner;
	int num_sharers;

	/* Bit map of sharers (last field in variable-size structure) */
	unsigned char sharers[0];
};

struct vi_mod_block_t
{
	struct vi_mod_t *mod;

	int set;
	int way;
	int state;
	unsigned int tag;

	struct linked_list_t *access_list;

	struct vi_mod_dir_entry_t *dir_entries;
};

struct vi_mod_t
{
	char *name;

	int num_sets;
	int assoc;
	int block_size;
	int sub_block_size;
	int num_sub_blocks;
	int num_sharers;
	int level;

	int high_net_node_index;
	int low_net_node_index;

	struct vi_net_t *high_net;
	struct vi_net_t *low_net;

	struct vi_mod_block_t *blocks;

	struct hash_table_t *access_table;
};

struct vi_mod_t *vi_mod_create(struct vi_trace_line_t *trace_line);
void vi_mod_free(struct vi_mod_t *mod);

void vi_mod_add_access(struct vi_mod_t *mod, int set, int way,
	struct vi_mod_access_t *access);
struct vi_mod_access_t *vi_mod_find_access(struct vi_mod_t *mod,
	int set, int way, char *access_name);
struct vi_mod_access_t *vi_mod_remove_access(struct vi_mod_t *mod,
	int set, int way, char *access_name);
struct linked_list_t *vi_mod_get_access_list(struct vi_mod_t *mod,
	int set, int way);

void vi_mod_block_set(struct vi_mod_t *mod, int set, int way,
	unsigned int tag, char *state);
int vi_mod_block_get_num_sharers(struct vi_mod_t *mod, int set, int way);

void vi_mod_read_checkpoint(struct vi_mod_t *mod, FILE *f);
void vi_mod_write_checkpoint(struct vi_mod_t *mod, FILE *f);

struct vi_mod_dir_entry_t *vi_mod_dir_entry_get(struct vi_mod_t *mod,
	int set, int way, int sub_block);
void vi_mod_dir_entry_set_sharer(struct vi_mod_t *mod,
	int x, int y, int z, int sharer);
void vi_mod_dir_entry_clear_sharer(struct vi_mod_t *mod,
	int x, int y, int z, int sharer);
void vi_mod_dir_entry_clear_all_sharers(struct vi_mod_t *mod,
	int x, int y, int z);
int vi_mod_dir_entry_is_sharer(struct vi_mod_t *mod,
	int x, int y, int z, int sharer);
void vi_mod_dir_entry_set_owner(struct vi_mod_t *mod,
	int x, int y, int z, int owner);

void vi_mod_dir_entry_read_checkpoint(struct vi_mod_t *mod, int x, int y, int z, FILE *f);
void vi_mod_dir_entry_write_checkpoint(struct vi_mod_t *mod, int x, int y, int z, FILE *f);




/*
 * Network
 */

struct vi_net_t
{
	char *name;

	struct list_t *node_list;
};

struct vi_net_t *vi_net_create(struct vi_trace_line_t *trace_line);
void vi_net_free(struct vi_net_t *net);

void vi_net_attach_mod(struct vi_net_t *net, struct vi_mod_t *mod, int node_index);
struct vi_mod_t *vi_net_get_mod(struct vi_net_t *net, int node_index);




/*
 * Visual Memory System Panel
 */

struct vi_mem_panel_t;

struct vi_mem_panel_t *vi_mem_panel_create(void);
void vi_mem_panel_free(struct vi_mem_panel_t *panel);

void vi_mem_panel_refresh(struct vi_mem_panel_t *panel);

GtkWidget *vi_mem_panel_get_widget(struct vi_mem_panel_t *panel);




/*
 * Visual Module Widget
 */

struct vi_mod_widget_t;

struct vi_mod_widget_t *vi_mod_widget_create(char *name);
void vi_mod_widget_free(struct vi_mod_widget_t *widget);

void vi_mod_widget_refresh(struct vi_mod_widget_t *visual_mod_widget);

GtkWidget *vi_mod_widget_get_widget(struct vi_mod_widget_t *widget);



#endif
