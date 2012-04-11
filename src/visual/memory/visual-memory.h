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

struct visual_mem_system_t
{
	struct hash_table_t *mod_table;
	struct hash_table_t *net_table;
	struct hash_table_t *access_table;
	struct list_t *mod_level_list;
};

void visual_mem_system_init(void);
void visual_mem_system_done(void);




/*
 * Memory Module Access
 */

struct visual_mod_access_t
{
	char *name;
	char *state;

	unsigned int address;

	long long creation_cycle;
	long long state_update_cycle;
};

struct visual_mod_access_t *visual_mod_access_create(char *name, unsigned int address);
void visual_mod_access_free(struct visual_mod_access_t *access);

void visual_mod_access_set_state(struct visual_mod_access_t *access, char *state);

void visual_mod_access_read_checkpoint(struct visual_mod_access_t *access, FILE *f);
void visual_mod_access_write_checkpoint(struct visual_mod_access_t *access, FILE *f);

void visual_mod_access_get_name_short(char *access_name, char *buf, int size);
void visual_mod_access_get_name_long(char *access_name, char *buf, int size);
void visual_mod_access_get_desc(char *access_name, char *buf, int size);




/*
 * Memory Module
 */

#define VISUAL_MOD_DIR_ENTRY_SHARERS_SIZE(mod) (((mod)->num_sharers + 7) / 8)
#define VISUAL_MOD_DIR_ENTRY_SIZE(mod) (sizeof(struct visual_mod_dir_entry_t) + \
	VISUAL_MOD_DIR_ENTRY_SHARERS_SIZE((mod)))

struct visual_mod_dir_entry_t
{
	int owner;
	int num_sharers;

	/* Bit map of sharers (last field in variable-size structure) */
	unsigned char sharers[0];
};

struct visual_mod_block_t
{
	struct visual_mod_t *mod;

	int set;
	int way;
	int state;
	unsigned int tag;

	struct linked_list_t *access_list;

	struct visual_mod_dir_entry_t *dir_entries;
};

struct visual_mod_t
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

	struct visual_net_t *high_net;
	struct visual_net_t *low_net;

	struct visual_mod_block_t *blocks;

	struct hash_table_t *access_table;
};

struct visual_mod_t *visual_mod_create(struct trace_line_t *trace_line);
void visual_mod_free(struct visual_mod_t *mod);

void visual_mod_add_access(struct visual_mod_t *mod, int set, int way,
	struct visual_mod_access_t *access);
struct visual_mod_access_t *visual_mod_find_access(struct visual_mod_t *mod,
	int set, int way, char *access_name);
struct visual_mod_access_t *visual_mod_remove_access(struct visual_mod_t *mod,
	int set, int way, char *access_name);
struct linked_list_t *visual_mod_get_access_list(struct visual_mod_t *mod,
	int set, int way);

void visual_mod_block_set(struct visual_mod_t *mod, int set, int way,
	unsigned int tag, char *state);
int visual_mod_block_get_num_sharers(struct visual_mod_t *mod, int set, int way);

void visual_mod_read_checkpoint(struct visual_mod_t *mod, FILE *f);
void visual_mod_write_checkpoint(struct visual_mod_t *mod, FILE *f);

struct visual_mod_dir_entry_t *visual_mod_dir_entry_get(struct visual_mod_t *mod,
	int set, int way, int sub_block);
void visual_mod_dir_entry_set_sharer(struct visual_mod_t *mod,
	int x, int y, int z, int sharer);
void visual_mod_dir_entry_clear_sharer(struct visual_mod_t *mod,
	int x, int y, int z, int sharer);
void visual_mod_dir_entry_clear_all_sharers(struct visual_mod_t *mod,
	int x, int y, int z);
int visual_mod_dir_entry_is_sharer(struct visual_mod_t *mod,
	int x, int y, int z, int sharer);
void visual_mod_dir_entry_set_owner(struct visual_mod_t *mod,
	int x, int y, int z, int owner);

void visual_mod_dir_entry_read_checkpoint(struct visual_mod_t *mod, int x, int y, int z, FILE *f);
void visual_mod_dir_entry_write_checkpoint(struct visual_mod_t *mod, int x, int y, int z, FILE *f);




/*
 * Network
 */

struct visual_net_t
{
	char *name;

	struct list_t *node_list;
};

struct visual_net_t *visual_net_create(struct trace_line_t *trace_line);
void visual_net_free(struct visual_net_t *net);

void visual_net_attach_mod(struct visual_net_t *net, struct visual_mod_t *mod, int node_index);
struct visual_mod_t *visual_net_get_mod(struct visual_net_t *net, int node_index);




/*
 * Visual Memory System Widget
 */

struct visual_mem_system_widget_t;

struct visual_mem_system_widget_t *visual_mem_system_widget_create(void);
void visual_mem_system_widget_free(struct visual_mem_system_widget_t *widget);

void visual_mem_system_widget_refresh(struct visual_mem_system_widget_t *widget);

GtkWidget *visual_mem_system_widget_get_widget(struct visual_mem_system_widget_t *widget);




/*
 * Visual Module Widget
 */

struct visual_mod_widget_t;

struct visual_mod_widget_t *visual_mod_widget_create(char *name);
void visual_mod_widget_free(struct visual_mod_widget_t *widget);

void visual_mod_widget_refresh(struct visual_mod_widget_t *visual_mod_widget);

GtkWidget *visual_mod_widget_get_widget(struct visual_mod_widget_t *widget);





/*
 * Global Objects
 */

/* State */
extern struct visual_mem_system_t *visual_mem_system;

/* Widgets */
extern struct visual_mem_system_widget_t *visual_mem_system_widget;


#endif
