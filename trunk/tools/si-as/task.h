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

#include <stdio.h>

extern struct list_t *task_list;

struct si_task_t
{
	long offset;
	char *ID;
};

/* Returns a pointer to an si_task_t object
 * initialized with task ID = 'ID' and offset 'offset' */
struct si_task_t *si_task_create(char *ID, long offset);

void si_task_free(struct si_task_t *task);
void si_task_dump(struct si_task_t *task, FILE *f);

/***********Task List Functions***********/

void si_task_list_init(void);
void si_task_list_add(struct si_task_t *task);
void si_task_list_done(void);
void si_task_list_dump(FILE *f);
