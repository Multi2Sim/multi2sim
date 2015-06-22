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

#ifndef ARCH_ARM_EMU_FILE_H
#define ARCH_ARM_EMU_FILE_H

#include <stdio.h>


enum arm_file_desc_kind_t
{
	arm_file_desc_invalid = 0,
	arm_file_desc_regular,  /* Regular arm_file */
	arm_file_desc_std,  /* Standard input or output */
	arm_file_desc_pipe,  /* A pipe */
	arm_file_desc_virtual,  /* A virtual arm_file with artificial contents */
	arm_file_desc_gpu,  /* GPU device */
	arm_file_desc_socket  /* Network socket */
};

struct arm_file_desc_t
{
	enum arm_file_desc_kind_t kind;  /* File type */
	int guest_fd;  /* Guest arm_file descriptor id */
	int host_fd;  /* Equivalent open host arm_file */
	int flags;  /* O_xxx flags */
	char *path;  /* Associated path if applicable */
};


/* File descriptor table */
struct arm_file_desc_table_t
{
	/* Number of extra contexts sharing table */
	int num_links;

	/* List of descriptors */
	struct list_t *arm_file_desc_list;
};


struct arm_file_desc_table_t *arm_file_desc_table_create(void);
void arm_file_desc_table_free(struct arm_file_desc_table_t *table);

struct arm_file_desc_table_t *arm_file_desc_table_link(struct arm_file_desc_table_t *table);
void arm_file_desc_table_unlink(struct arm_file_desc_table_t *table);

void arm_file_desc_table_dump(struct arm_file_desc_table_t *table, FILE *f);

struct arm_file_desc_t *arm_file_desc_table_entry_get(struct arm_file_desc_table_t *table, int index);
struct arm_file_desc_t *arm_file_desc_table_entry_new(struct arm_file_desc_table_t *table,
	enum arm_file_desc_kind_t kind, int host_fd, char *path, int flags);
struct arm_file_desc_t *arm_file_desc_table_entry_new_guest_fd(struct arm_file_desc_table_t *table,
        enum arm_file_desc_kind_t kind, int guest_fd, int host_fd, char *path, int flags);
void arm_file_desc_table_entry_free(struct arm_file_desc_table_t *table, int index);
void arm_file_desc_table_entry_dump(struct arm_file_desc_table_t *table, int index, FILE *f);

int arm_file_desc_table_get_host_fd(struct arm_file_desc_table_t *table, int guest_fd);
int arm_file_desc_table_get_guest_fd(struct arm_file_desc_table_t *table, int host_fd);


#endif

