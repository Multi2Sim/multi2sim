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
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <debug.h>
#include <hash-table.h>
#include <bin-config.h>


/*
 * Private functions
 */


static void bin_config_free_elem_list(struct hash_table_t *elem_list);


static struct bin_config_elem_t *bin_config_elem_create(struct bin_config_t *bin_config,
	void *data, int size, int dup_data)
{
	struct bin_config_elem_t *elem;

	/* Create */
	elem = calloc(1, sizeof(struct bin_config_elem_t));
	if (!elem)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Initialize */
	elem->bin_config = bin_config;
	elem->size = size;
	elem->dup_data = dup_data;
	if (dup_data)
	{
		elem->data = malloc(size);
		if (!elem->data)
			fatal("%s: out of memory", __FUNCTION__);
		memcpy(elem->data, data, size);
	}
	else
	{
		elem->data = data;
	}
	
	/* Return */
	return elem;
}


static void bin_config_elem_free(struct bin_config_elem_t *elem)
{
	/* Free list of child elements */
	if (elem->child_elem_list)
		bin_config_free_elem_list(elem->child_elem_list);

	/* Free element */
	if (elem->dup_data && elem->data)
		free(elem->data);
	free(elem);
}


static struct bin_config_elem_t *bin_config_add_dup_data(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem, char *var, void *data, int size,
	int dup_data)
{
	struct bin_config_elem_t *elem;
	struct hash_table_t *child_elem_list;

	/* Check parent element */
	if (parent_elem && parent_elem->bin_config != bin_config)
	{
		bin_config->error_code = BIN_CONFIG_ERR_PARENT;
		return NULL;
	}

	/* Check valid data pointer */
	if (size && !data)
	{
		bin_config->error_code = BIN_CONFIG_ERR_DATA;
		return NULL;
	}

	/* Get list of child elements.
	 * Lazy creation of child element lists happens here. */
	if (parent_elem)
	{
		if (!parent_elem->child_elem_list)
			parent_elem->child_elem_list = hash_table_create(1, 0);
		child_elem_list = parent_elem->child_elem_list;
	}
	else
	{
		if (!bin_config->elem_list)
			bin_config->elem_list = hash_table_create(1, 0);
		child_elem_list = bin_config->elem_list;
	}

	/* Create element */
	elem = bin_config_elem_create(bin_config, data, size, dup_data);
	elem->parent_elem = parent_elem;

	/* Add to list of child elements */
	if (!hash_table_insert(child_elem_list, var, elem))
	{
		bin_config_elem_free(elem);
		bin_config->error_code = BIN_CONFIG_ERR_DUPLICATE;
		return NULL;
	}

	/* Return created element */
	bin_config->error_code = BIN_CONFIG_ERR_OK;
	return elem;
}


static void bin_config_free_elem_list(struct hash_table_t *elem_list)
{
	struct bin_config_elem_t *elem;

	assert(elem_list);
	for (hash_table_find_first(elem_list, (void **) &elem); elem;
		hash_table_find_next(elem_list, (void **) &elem))
	{
		bin_config_elem_free(elem);
	}
	hash_table_free(elem_list);
}


static void bin_config_dump_elem_list(struct hash_table_t *elem_list, int level, FILE *f)
{
	struct bin_config_elem_t *elem;
	char *var;
	char *comma;
	int i;

	/* Empty table */
	if (!elem_list)
		return;

	/* Print each element */
	for (var = hash_table_find_first(elem_list, (void **) &elem); var;
		var = hash_table_find_next(elem_list, (void **) &elem))
	{
		/* Print tabs */
		comma = "";
		for (i = 0; i < level; i++)
			fprintf(f, "\t");

		/* Variable name */
		if (elem->size)
			fprintf(f, "%s = ", var);
		else
			fprintf(f, "[%s]", var);

		/* Integer dump */
		switch (elem->size)
		{
		case 1:
			fprintf(f, "%s%d, 0x%x", comma, * (uint8_t *) elem->data,
				* (uint8_t *) elem->data);
			comma = ", ";
			break;

		case 2:
			fprintf(f, "%s%d, 0x%x", comma, * (uint16_t *) elem->data,
				* (uint16_t *) elem->data);
			comma = ", ";
			break;

		case 4:
			fprintf(f, "%s%d, 0x%x, %.2g", comma, * (uint32_t *) elem->data,
				* (uint32_t *) elem->data, * (float *) elem->data);
			comma = ", ";
			break;

		case 8:
			fprintf(f, "%s%d, 0x%x, %.2g", comma, * (uint16_t *) elem->data,
				* (uint16_t *) elem->data, * (double *) elem->data);
			comma = ", ";
			break;
		}

		/* String dump */
		if (elem->size)
		{
			fprintf(f, "%s\"", comma);
			for (i = 0; i < elem->size; i++)
			{
				unsigned char c = * (unsigned char *) (elem->data + i);
				fprintf(f, "%c", c > 31 && c < 255 ? c : '.');
			}
			fprintf(f, "\"");
		}

		/* New line */
		fprintf(f, "\n");

		/* Print children */
		if (elem->child_elem_list)
			bin_config_dump_elem_list(elem->child_elem_list, level + 1, f);
	}
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
	if (bin_config->elem_list)
		bin_config_free_elem_list(bin_config->elem_list);
	free(bin_config->file_name);
	free(bin_config);
}


struct bin_config_elem_t *bin_config_add(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem, char *var, void *data, int size)
{
	return bin_config_add_dup_data(bin_config, parent_elem,
		var, data, size, 1);
}


struct bin_config_elem_t *bin_config_add_no_dup(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem, char *var, void *data, int size)
{
	return bin_config_add_dup_data(bin_config, parent_elem,
		var, data, size, 0);
}


int bin_config_remove(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem, char *var)
{
	struct hash_table_t *child_elem_list;
	struct bin_config_elem_t *elem;

	/* Check parent element */
	if (parent_elem && parent_elem->bin_config != bin_config)
	{
		bin_config->error_code = BIN_CONFIG_ERR_PARENT;
		return 0;
	}

	/* Get list of child elements */
	child_elem_list = parent_elem ? parent_elem->child_elem_list
		: bin_config->elem_list;
	if (!child_elem_list)
	{
		bin_config->error_code = BIN_CONFIG_ERR_NOT_FOUND;
		return 0;
	}

	/* Remove element */
	elem = hash_table_remove(child_elem_list, var);
	if (!elem)
	{
		bin_config->error_code = BIN_CONFIG_ERR_NOT_FOUND;
		return 0;
	}

	/* Free element */
	bin_config_elem_free(elem);

	/* Success */
	bin_config->error_code = BIN_CONFIG_ERR_OK;
	return 1;
}


struct bin_config_elem_t *bin_config_get(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem, char *var,
	void **data_ptr, int *size_ptr)
{
	struct hash_table_t *child_elem_list;
	struct bin_config_elem_t *elem;

	/* Check parent element */
	if (parent_elem && parent_elem->bin_config != bin_config)
	{
		bin_config->error_code = BIN_CONFIG_ERR_PARENT;
		return 0;
	}

	/* Get list of child elements */
	child_elem_list = parent_elem ? parent_elem->child_elem_list
		: bin_config->elem_list;
	if (!child_elem_list)
	{
		bin_config->error_code = BIN_CONFIG_ERR_NOT_FOUND;
		return 0;
	}

	/* Remove element */
	elem = hash_table_get(child_elem_list, var);
	if (!elem)
	{
		bin_config->error_code = BIN_CONFIG_ERR_NOT_FOUND;
		return 0;
	}

	/* Return data and size */
	if (data_ptr)
		*data_ptr = elem->data;
	if (size_ptr)
		*size_ptr = elem->size;

	/* Return element */
	bin_config->error_code = BIN_CONFIG_ERR_OK;
	return elem;
}


void bin_config_dump(struct bin_config_t *bin_config, FILE *f)
{
	/* Dump list of elements */
	bin_config_dump_elem_list(bin_config->elem_list, 0, f);

	/* Success */
	bin_config->error_code = BIN_CONFIG_ERR_OK;
}
