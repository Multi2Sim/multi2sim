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

#include <assert.h>
#include <unistd.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "file.h"


/*
 * Private Functions
 */

struct arm_file_desc_t *arm_file_desc_create(enum arm_file_desc_kind_t kind,
	int guest_fd, int host_fd, int flags, char *path)
{
	struct arm_file_desc_t *desc;

	/* Initialize */
	desc = xcalloc(1, sizeof(struct arm_file_desc_t));
	desc->kind = kind;
	desc->guest_fd = guest_fd;
	desc->host_fd = host_fd;
	desc->flags = flags;

	/* Path */
	if (path)
		desc->path = xstrdup(path);

	/* Return */
	return desc;
}


static void arm_file_desc_free(struct arm_file_desc_t *desc)
{
	if (desc->path)
		free(desc->path);
	free(desc);
}




/*
 * Public Functions
 */

struct arm_file_desc_table_t *arm_file_desc_table_create(void)
{
	struct arm_file_desc_table_t *table;
	struct arm_file_desc_t *desc;

	/* Initialize */
	table = xcalloc(1, sizeof(struct arm_file_desc_table_t));
	table->arm_file_desc_list = list_create();

	/* Add stdin */
	desc = arm_file_desc_create(arm_file_desc_std, 0, 0, 0, NULL);
	list_add(table->arm_file_desc_list, desc);

	/* Add stdout */
	desc = arm_file_desc_create(arm_file_desc_std, 1, 1, 0, NULL);
	list_add(table->arm_file_desc_list, desc);

	/* Add stderr */
	desc = arm_file_desc_create(arm_file_desc_std, 2, 2, 0, NULL);
	list_add(table->arm_file_desc_list, desc);

	/* Return */
	return table;
}


void arm_file_desc_table_free(struct arm_file_desc_table_t *table)
{
	int i;
	struct arm_file_desc_t *desc;

	/* Check no more links */
	assert(!table->num_links);

	/* Free arm_file descriptors */
	for (i = 0; i < list_count(table->arm_file_desc_list); i++)
	{
		desc = list_get(table->arm_file_desc_list, i);
		if (desc)
			arm_file_desc_free(desc);
	}

	/* Free list and table */
	list_free(table->arm_file_desc_list);
	free(table);
}


struct arm_file_desc_table_t *arm_file_desc_table_link(struct arm_file_desc_table_t *fdt)
{
	fdt->num_links++;
	return fdt;
}


void arm_file_desc_table_unlink(struct arm_file_desc_table_t *fdt)
{
	assert(fdt->num_links >= 0);
	if (fdt->num_links)
		fdt->num_links--;
	else
		arm_file_desc_table_free(fdt);
}


void arm_file_desc_table_dump(struct arm_file_desc_table_t *table, FILE *f)
{
	struct arm_file_desc_t *desc;

	int i;
	int busy = 0;

	for (i = 0; i < list_count(table->arm_file_desc_list); i++)
	{
		arm_file_desc_table_entry_dump(table, i, f);
		desc = list_get(table->arm_file_desc_list, i);
		if (desc)
			busy++;
	}
	fprintf(f, "  %d table entries, %d busy\n", list_count(table->arm_file_desc_list), busy);
}

int arm_file_desc_table_get_host_fd(struct arm_file_desc_table_t *table, int guest_fd)
{
	struct arm_file_desc_t *desc;

	desc = list_get(table->arm_file_desc_list, guest_fd);
	return desc ? desc->host_fd : -1;
}


int arm_file_desc_table_get_guest_fd(struct arm_file_desc_table_t *table, int host_fd)
{
	struct arm_file_desc_t *desc;
	int i;

	for (i = 0; i < list_count(table->arm_file_desc_list); i++)
	{
		desc = list_get(table->arm_file_desc_list, i);
		if (desc->host_fd == host_fd)
			return i;
	}

	/* Not found */
	return -1;
}


void arm_file_desc_table_entry_dump(struct arm_file_desc_table_t *table, int index, FILE *f)
{
	struct arm_file_desc_t *desc;

	/* Invalid entry */
	if (index < 0 || index >= list_count(table->arm_file_desc_list))
		return;

	/* Dump entry */
	desc = list_get(table->arm_file_desc_list, index);
	if (desc)
	{
		assert(desc->guest_fd == index);
		fprintf(f, "    %2d  host_fd=%-2d  kind=%-2d  path='%s'\n",
			desc->guest_fd, desc->host_fd, desc->kind, desc->path);
	}
	else
	{
		fprintf(f, "    %2d  empty\n", index);
	}
}


struct arm_file_desc_t *arm_file_desc_table_entry_get(struct arm_file_desc_table_t *table, int index)
{
	return list_get(table->arm_file_desc_list, index);
}

struct arm_file_desc_t *arm_file_desc_table_entry_new(struct arm_file_desc_table_t *table,
	enum arm_file_desc_kind_t kind, int host_fd, char *path, int flags)
{
	return arm_file_desc_table_entry_new_guest_fd(table, kind, -1, host_fd, path, flags);
}

struct arm_file_desc_t *arm_file_desc_table_entry_new_guest_fd(struct arm_file_desc_table_t *table,
	enum arm_file_desc_kind_t kind, int guest_fd, int host_fd, char *path, int flags)
{
	struct arm_file_desc_t *desc;

	int i;

	/* Look for a free entry */
	for (i = 0; i < list_count(table->arm_file_desc_list) && guest_fd < 0; i++)
		if (!list_get(table->arm_file_desc_list, i))
			guest_fd = i;

	/* If no free entry was found, add new entry. */
	if (guest_fd < 0)
	{
		guest_fd = list_count(table->arm_file_desc_list);
		list_add(table->arm_file_desc_list, NULL);
	}

	/* Specified guest_fd may still be too large */
	for (i = list_count(table->arm_file_desc_list); i <= guest_fd; ++i) {
		list_add(table->arm_file_desc_list, NULL);
	}

	/* Create guest arm_file descriptor and return. */
	desc = arm_file_desc_create(kind, guest_fd, host_fd, flags, path);
	list_set(table->arm_file_desc_list, guest_fd, desc);

	/* Return */
	return desc;
}


void arm_file_desc_table_entry_free(struct arm_file_desc_table_t *table, int index)
{
	struct arm_file_desc_t *desc;

	/* Get arm_file descriptor. If it is empty or out of range, exit. */
	desc = list_get(table->arm_file_desc_list, index);
	if (!desc)
		return;

	/* If it is a virtual arm_file, delete the temporary host path. */
	if (desc->kind == arm_file_desc_virtual)
	{
		if (unlink(desc->path))
			warning("%s: temporary host virtual arm_file could not be deleted",
				desc->path);
	}

	/* Free arm_file descriptor and remove entry in table. */
	list_set(table->arm_file_desc_list, index, NULL);
	arm_file_desc_free(desc);
}

