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

#include <m2s-glut.h>

#include <assert.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>


static struct list_t *x86_glut_window_list;


void x86_glut_window_init(void)
{
	/* Initialize window list */
	x86_glut_window_list = list_create();
	list_add(x86_glut_window_list, NULL);
}


void x86_glut_window_done(void)
{
	struct x86_glut_window_t *window;

	int guest_id;

	/* Clear window list */
	LIST_FOR_EACH(x86_glut_window_list, guest_id)
	{
		window = list_get(x86_glut_window_list, guest_id);
		if (window)
			x86_glut_window_free(window);
	}
	list_free(x86_glut_window_list);
}


struct x86_glut_window_t *x86_glut_window_create(char *title)
{
	struct x86_glut_window_t *window;

	/* Allocate */
	window = calloc(1, sizeof(struct x86_glut_window_t));
	if (!window)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	window->guest_id = list_count(x86_glut_window_list);
	window->title = str_set(NULL, title);
	list_add(x86_glut_window_list, window);
	
	/* Return */
	return window;
}


void x86_glut_window_free(struct x86_glut_window_t *window)
{
	/* Remove from window list */
	assert(list_get(x86_glut_window_list, window->guest_id) == window);
	list_set(x86_glut_window_list, window->guest_id, NULL);

	/* Free */
	str_free(window->title);
	free(window);
}


struct x86_glut_window_t *x86_glut_window_get_by_guest_id(int guest_id)
{
	return list_get(x86_glut_window_list, guest_id);
}


struct x86_glut_window_t *x86_glut_window_get_by_host_id(int host_id)
{
	struct x86_glut_window_t *window;

	int guest_id;

	LIST_FOR_EACH(x86_glut_window_list, guest_id)
	{
		window = list_get(x86_glut_window_list, guest_id);
		if (window && window->host_id == host_id)
			return window;
	}

	/* Not found */
	return NULL;
}

