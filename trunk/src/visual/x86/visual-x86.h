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


#ifndef VISUAL_X86_H
#define VISUAL_X86_H

#include <visual-common.h>


/*
 * X86 Panel
 */

struct vi_x86_panel_t *vi_x86_panel_create(void);
void vi_x86_panel_free(struct vi_x86_panel_t *panel);

void vi_x86_panel_refresh(struct vi_x86_panel_t *panel);

GtkWidget *vi_x86_panel_get_widget(struct vi_x86_panel_t *panel);




/*
 * Context
 */

struct vi_x86_context_t
{
	char *name;
	int id;

	/* Map */
	int core_id;
	int thread_id;
};

struct vi_x86_context_t *vi_x86_context_create(char *name, int id);
void vi_x86_context_free(struct vi_x86_context_t *context);

void vi_x86_context_get_name_short(char *context_name, char *buf, int size);
void vi_x86_context_get_desc(char *context_name, char *buf, int size);




/*
 * Core
 */

struct vi_x86_core_t
{
	char *name;

	struct hash_table_t *context_table;
};

struct vi_x86_core_t *vi_x86_core_create(char *name);
void vi_x86_core_free(struct vi_x86_core_t *core);




/*
 * CPU
 */

struct vi_x86_cpu_t
{
	struct list_t *core_list;
	struct hash_table_t *context_table;
};


extern struct vi_x86_cpu_t *vi_x86_cpu;

void vi_x86_cpu_init(void);
void vi_x86_cpu_done(void);


#endif
