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

extern struct hash_table_t *label_table;

struct si_label_t
{
	long offset;
	char *ID;
};

/* Returns a pointer to an si_label_table_t object
 * initialized with label ID = 'ID' and offset 'offset' */
struct si_label_t *si_label_create(char *ID, long offset);

void si_label_free(struct si_label_t *label);
void si_label_dump(struct si_label_t *label, FILE *f);

/***********Label Table Functions***********/

void si_label_table_init(void);
void si_label_table_done(void);
void si_label_table_dump(FILE *f);
/* Returns 0 if a failure occured, otherwise creates a label object and inserts it */
int si_label_table_insert(struct si_label_t *label);
/* Returns a pointer to an si_label_t struct corresponding to the key 'ID' */
struct si_label_t *si_label_table_get(char *ID);

