/*
 *  Multi2Sim
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


#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <debug.h>
#include <hash-table.h>
#include <bin-config.h>

/*
 * Private functions
 */

struct bin_config_elem_t *bin_config_elem_create(struct bin_config_t *bin_config,
	void *data, int size)
{
	struct bin_config_elem_t *elem;

	/* Create */
	elem = calloc(1, sizeof(struct bin_config_elem_t) + size);
	if (!elem)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Initialize */
	elem->bin_config = bin_config;
	elem->size = size;
	memcpy(elem->data, data, size);
	
	/* Return */
	return elem;
}


void bin_config_elem_free(struct bin_config_elem_t *elem)
{
	free(elem);
}




/*
 * Public Functions
 */

struct bin_config_t *bin_config_create(char *file_name)
{
	struct bin_config_t *bin_config;

	/* Create */
	bin_config = calloc(1, sizeof(struct bin_config_t));
	if (!bin_config)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Initialize fields */
	bin_config->file_name = strdup(file_name);
	if (!bin_config->file_name)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Return */
	return bin_config;
}


void bin_config_free(struct bin_config_t *bin_config)
{
	free(bin_config->file_name);
	free(bin_config);
}


struct bin_config_elem_t *bin_config_add(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem, char *var, void *data, int size)
{
	struct bin_config_elem_t *elem;
	struct hash_table_t *child_elem_list;

	/* Create element */
	elem = bin_config_elem_create(bin_config, data, size);

	/* Add to parent's list of elements */
	if (parent_elem)
	{
		if (parent_elem->bin_config != bin_config)
			fatal("%s: parent element not in config", __FUNCTION__);
		elem->parent_elem = parent_elem;
		child_elem_list = elem->child_elem_list;
	}
	else
	{
		child_elem_list = bin_config->elem_list;
	}
	
	/* Add to main/parent's list of elements */
	if (!hash_table_insert(child_elem_list, var, elem))
		fatal("%s: duplicated element", __FUNCTION__);
	
	/* Return created element */
	return elem;
}

