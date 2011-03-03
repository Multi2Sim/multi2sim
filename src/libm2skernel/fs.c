/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include <m2skernel.h>

struct fdt_t *fdt_create()
{
	struct fdt_t *fdt;
	struct fd_t *fd0, *fd1, *fd2;

	/* Create file descriptor table and file descriptor list */
	fdt = calloc(1, sizeof(struct fdt_t));
	fdt->fd_list = list_create(3);

	/* Add stdin */
	fd0 = calloc(1, sizeof(struct fd_t));
	fd0->kind = fd_kind_std;
	fd0->guest_fd = 0;
	fd0->host_fd = 0;
	list_add(fdt->fd_list, fd0);

	/* Add stdout */
	fd1 = calloc(1, sizeof(struct fd_t));
	fd1->kind = fd_kind_std;
	fd1->guest_fd = 1;
	fd1->host_fd = 1;
	list_add(fdt->fd_list, fd1);

	/* Add stderr */
	fd2 = calloc(1, sizeof(struct fd_t));
	fd2->kind = fd_kind_std;
	fd2->guest_fd = 2;
	fd2->host_fd = 2;
	list_add(fdt->fd_list, fd2);

	return fdt;
}


void fdt_free(struct fdt_t *fdt)
{
	int i;
	struct fd_t *fd;

	/* Free file descriptors */
	for (i = 0; i < list_count(fdt->fd_list); i++) {
		fd = list_get(fdt->fd_list, i);
		if (fd)
			free(fd);
	}

	/* Free list and fdt */
	list_free(fdt->fd_list);
	free(fdt);
}


void fdt_dump(struct fdt_t *fdt, FILE *f)
{
	int i, busy = 0;
	struct fd_t *fd;

	for (i = 0; i < list_count(fdt->fd_list); i++) {
		fdt_entry_dump(fdt, i, f);
		fd = list_get(fdt->fd_list, i);
		if (fd)
			busy++;
	}
	fprintf(f, "  %d table entries, %d busy\n", list_count(fdt->fd_list), busy);
}


int fdt_get_host_fd(struct fdt_t *fdt, int guest_fd)
{
	struct fd_t *fd;

	fd = list_get(fdt->fd_list, guest_fd);
	return fd ? fd->host_fd : -1;
}


int fdt_get_guest_fd(struct fdt_t *fdt, int host_fd)
{
	struct fd_t *fd;
	int i;

	for (i = 0; i < list_count(fdt->fd_list); i++) {
		fd = list_get(fdt->fd_list, i);
		if (fd->host_fd == host_fd)
			return i;
	}
	return -1;
}


void fdt_entry_dump(struct fdt_t *fdt, int index, FILE *f)
{
	struct fd_t *fd;
	if (index < 0 || index >= list_count(fdt->fd_list))
		return;
	fd = list_get(fdt->fd_list, index);
	if (!fd)
		fprintf(f, "    %2d  empty\n", index);
	else {
		assert(fd->guest_fd == index);
		fprintf(f, "    %2d  host_fd=%-2d  kind=%-2d  path='%s'\n",
			fd->guest_fd, fd->host_fd, fd->kind, fd->path);
	}
}


struct fd_t *fdt_entry_get(struct fdt_t *fdt, int index)
{
	return list_get(fdt->fd_list, index);
}


struct fd_t *fdt_entry_new(struct fdt_t *fdt, enum fd_kind_enum kind,
	int host_fd, char *path, int flags)
{
	int i, guest_fd;
	struct fd_t *fd;

	/* Look for an existing null entry */
	guest_fd = -1;
	for (i = 0; i < list_count(fdt->fd_list) && guest_fd < 0; i++)
		if (!list_get(fdt->fd_list, i))
			guest_fd = i;
	
	/* If no free entry was found, add new entry. */
	if (guest_fd < 0) {
		guest_fd = list_count(fdt->fd_list);
		list_add(fdt->fd_list, NULL);
	}

	/* Create guest file descriptor and return. */
	fd = calloc(1, sizeof(struct fd_t));
	fd->guest_fd = guest_fd;
	fd->host_fd = host_fd;
	fd->kind = kind;
	fd->flags = flags;
	strncpy(fd->path, path, MAX_PATH_SIZE);
	list_set(fdt->fd_list, guest_fd, fd);

	/* Return */
	return fd;
}


void fdt_entry_free(struct fdt_t *fdt, int index)
{
	struct fd_t *fd;

	/* Get file descriptor. If it is empty or out of range, exit. */
	fd = list_get(fdt->fd_list, index);
	if (!fd)
		return;
	
	/* If it is a virtual file, delete the temporary host path. */
	if (fd->kind == fd_kind_virtual) {
		if (unlink(fd->path))
			warning("%s: temporary host virtual file could not be deleted",
				fd->path);
	}
	
	/* Free file descriptor and remove entry in table. */
	free(fd);
	list_set(fdt->fd_list, index, NULL);
}

