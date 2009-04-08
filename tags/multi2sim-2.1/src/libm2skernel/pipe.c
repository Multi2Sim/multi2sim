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

#include "m2skernel.h"



struct pipe_t *pipe_create(void)
{
	struct pipe_t *p;

	/* Create pipe and buffer */
	p = calloc(1, sizeof(struct pipe_t));
	p->buffer = buffer_create(0x100);

	/* Open a new real pipe. We won't use this real pipe,
	 * but we just open it to get two new file descriptors,
	 * and ensure they won't be used in future 'open' calls. */
	if (pipe(p->fd))
		fatal("pipe_create: cannot create pipe");

	return p;
}


void pipe_free(struct pipe_t *pipe)
{
	buffer_free(pipe->buffer);
	free(pipe);
}


/* Locate a file descriptor among pipes. If it is found, the pipe is located
 * in the head of the pipe list, and the function returns true. Otherwise,
 * the function returns false. */
static int pipemgr_locate_fd(int fd)
{
	struct pipe_t *pipe, *prev;

	/* Search it */
	prev = NULL;
	pipe = ke->pipemgr->pipe_list;
	while (pipe && pipe->fd[0] != fd && pipe->fd[1] != fd) {
		prev = pipe;
		pipe = pipe->next;
	}

	/* Not found */
	if (!pipe)
		return 0;
	
	/* Place pipe at the list head. */
	if (prev) {
		prev->next = pipe->next;
		pipe->next = ke->pipemgr->pipe_list;
		ke->pipemgr->pipe_list = pipe;
	}
	return 1;
}


void pipemgr_init(void)
{
	ke->pipemgr = calloc(1, sizeof(struct pipemgr_t));
}


void pipemgr_done(void)
{
	struct pipe_t *pipe, *next;

	/* Free list of pipes */
	pipe = ke->pipemgr->pipe_list;
	while (pipe) {
		next = pipe->next;
		pipe_free(pipe);
		pipe = next;
	}

	/* Free pipe manager */
	free(ke->pipemgr);
}


void pipe_pipe(int fd[2])
{
	struct pipe_t *pipe;

	/* Create pipe and insert into list */
	pipe = pipe_create();
	pipe->next = ke->pipemgr->pipe_list;
	ke->pipemgr->pipe_list = pipe;

	/* Return file descriptors */
	fd[0] = pipe->fd[0];
	fd[1] = pipe->fd[1];
}


void pipe_close(int fd)
{
	struct pipe_t *pipe;
	if (!pipemgr_locate_fd(fd))
		fatal("pipe_close: not a pipe");
	pipe = ke->pipemgr->pipe_list;
	if (pipe->fd[0] == fd)
		pipe->fd[0] = -1;
	if (pipe->fd[1] == fd)
		pipe->fd[1] = -1;
	
	/* Close the real file descriptor that we created. */
	close(fd);
	
	/* If both file descriptors are -1, free pipe */
	if (pipe->fd[0] == -1 && pipe->fd[1] == -1) {
		ke->pipemgr->pipe_list = pipe->next;
		pipe_free(pipe);
	}
}


int pipe_is_pipe(int fd)
{
	int err;
	err = pipemgr_locate_fd(fd);
	return err;
}


int pipe_write(int fd, void *buf, int size)
{
	if (!pipemgr_locate_fd(fd))
		fatal("pipe_write: pipe not found");
	buffer_write(ke->pipemgr->pipe_list->buffer, buf, size);
	return size;
}


int pipe_read(int fd, void *buf, int size)
{
	int count;
	if (!pipemgr_locate_fd(fd))
		fatal("pipe_read: pipe not found");
	count = buffer_read(ke->pipemgr->pipe_list->buffer, buf, size);
	if (!count)
		fatal("pipe_read: no pending bytes to read");
	return count;
}


int pipe_count(int fd)
{
	if (!pipemgr_locate_fd(fd))
		fatal("pipe_count: not a pipe");
	return buffer_count(ke->pipemgr->pipe_list->buffer);
}

