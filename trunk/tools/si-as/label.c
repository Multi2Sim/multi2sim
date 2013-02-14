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

#include <string.h>

#include <lib/util/debug.h>
#include <lib/util/hash-table.h>

#include "label.h"

struct hash_table_t *label_table;

struct si_label_t *si_label_create(char *id, long offset)
{
	struct si_label_t *label;
	
	/* Allocate */
	label = calloc(1, sizeof(struct si_label_t));
	if (!label)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Initialize the label's offset and ID */
	label->offset = offset;
	label->ID = strdup(id);
	
	/* Return */
	return label;

}

void si_label_free(struct si_label_t *label)
{
	free(label->ID);
	free(label);
}

void si_label_dump(struct si_label_t *label, FILE *f)
{
	fprintf(f, "\tLabel ID: %s\n", label->ID);
	fprintf(f, "\tLabel Offset: %lu\n", label->offset);
	fprintf(f, "\n");
}

/***********Label Table Functions***********/

void si_label_table_init(void)
{
	/* Initialize to size of 5, and make the table
	 * case-sensitive.							  */
	label_table = hash_table_create(5, 1);
}

int si_label_table_insert(struct si_label_t *label)
{
	return hash_table_insert(label_table, label->ID, label);
}

struct si_label_t *si_label_table_get(char *ID)
{
	return hash_table_get(label_table, ID);
}

void si_label_table_done(void)
{
	char *key;
	struct si_label_t *label;

	/* Free all labels */
	for (key = hash_table_find_first(label_table, (void **) &label);
			key; key = hash_table_find_next(label_table, (void **) &label))
		si_label_free(label);
	
	/* Free label table */
	hash_table_free(label_table);
}

void si_label_table_dump(FILE *f)
{
	char *key;
	struct si_label_t *label;

	for (key = hash_table_find_first(label_table, (void **) &label);
			key; key = hash_table_find_next(label_table, (void **) &label))
		si_label_dump(label, f);
}

