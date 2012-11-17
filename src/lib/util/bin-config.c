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


#include <stdint.h>
#include <zlib.h>
#include <assert.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/hash-table.h>

#include "bin-config.h"


/*
 * Private functions
 */


static struct bin_config_elem_t *bin_config_elem_create(struct bin_config_t *bin_config,
	void *data, int size, int dup_data)
{
	struct bin_config_elem_t *elem;

	/* Initialize */
	elem = xcalloc(1, sizeof(struct bin_config_elem_t));
	elem->bin_config = bin_config;
	elem->size = size;
	elem->data = data;
	elem->dup_data = dup_data;
	
	/* Return */
	return elem;
}


static void bin_config_elem_free(struct bin_config_elem_t *elem)
{
	struct bin_config_elem_t *child_elem;

	/* Free list of child elements */
	if (elem->child_elem_list)
	{
		for (hash_table_find_first(elem->child_elem_list, (void **) &child_elem);
			child_elem;
			hash_table_find_next(elem->child_elem_list, (void **) &child_elem))
		{
			bin_config_elem_free(child_elem);
		}
		hash_table_free(elem->child_elem_list);
	}

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
			fprintf(f, "%s%lld, 0x%llx, %.2g", comma, * (long long *) elem->data,
				* (long long *) elem->data, * (double *) elem->data);
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

	/* Initialize fields */
	bin_config = xcalloc(1, sizeof(struct bin_config_t));
	bin_config->file_name = xstrdup(file_name);
	
	/* Return */
	return bin_config;
}


void bin_config_free(struct bin_config_t *bin_config)
{
	/* Free elements */
	if (bin_config->elem_list)
	{
		bin_config_clear(bin_config);
		hash_table_free(bin_config->elem_list);
	}
	
	/* Free configuration object */
	free(bin_config->file_name);
	free(bin_config);
}


struct bin_config_elem_t *bin_config_add(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem, char *var, void *data, int size)
{
	void *data_copy;

	/* Create duplicate of data */
	if (data && size > 0)
	{
		/* Copy data */
		data_copy = xmalloc(size);
		memcpy(data_copy, data, size);
	}
	else
	{
		data_copy = NULL;
	}

	/* Add variable */
	return bin_config_add_dup_data(bin_config, parent_elem,
		var, data_copy, size, 1);
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


static void bin_config_elem_list_save(struct hash_table_t *elem_list, gzFile f)
{
	struct bin_config_elem_t *elem;
	int num_elem;

	char *var;
	int var_len;

	/* Empty list */
	assert(sizeof(int) == 4);
	if (!elem_list)
	{
		num_elem = 0;
		gzwrite(f, &num_elem, 4);
		return;
	}

	/* Number of elements */
	num_elem = hash_table_count(elem_list);
	gzwrite(f, &num_elem, 4);

	/* List of elements */
	for (var = hash_table_find_first(elem_list, (void **) &elem); elem;
		var = hash_table_find_next(elem_list, (void **) &elem))
	{
		/* Variable name */
		var_len = strlen(var);
		gzwrite(f, &var_len, 4);
		gzwrite(f, var, var_len);

		/* Variable value */
		gzwrite(f, &elem->size, 4);
		if (elem->data)
			gzwrite(f, elem->data, elem->size);

		/* Child elements */
		bin_config_elem_list_save(elem->child_elem_list, f);

	}
}


int bin_config_save(struct bin_config_t *bin_config)
{
	gzFile f;

	/* Open file */
	f = gzopen(bin_config->file_name, "wb");
	if (!f)
	{
		bin_config->error_code = BIN_CONFIG_ERR_IO;
		return 0;
	}

	/* Store list of elements */
	bin_config_elem_list_save(bin_config->elem_list, f);

	/* Close */
	gzclose(f);
	bin_config->error_code = BIN_CONFIG_ERR_OK;
	return 1;
}


static struct hash_table_t *bin_config_load_elem_list(struct bin_config_t *bin_config,
	gzFile f)
{
	struct hash_table_t *elem_list;
	struct bin_config_elem_t *elem;

	int num_elem;
	int i;

	/* Get number of elements */
	num_elem = 0;
	gzread(f, &num_elem, 4);
	if (!num_elem)
		return NULL;
	
	/* Create list */
	elem_list = hash_table_create(num_elem, 0);

	/* Read elements */
	for (i = 0; i < num_elem; i++)
	{
		char *var;
		int var_len;

		int size;
		void *data;

		/* Read variable */
		var_len = 0;
		gzread(f, &var_len, 4);
		var = xcalloc(1, var_len + 1);
		gzread(f, var, var_len);

		/* Read data */
		size = 0;
		gzread(f, &size, 4);
		data = xmalloc(size);
		gzread(f, data, size);

		/* Create and add element */
		elem = bin_config_elem_create(bin_config, data, size, 1);
		hash_table_insert(elem_list, var, elem);

		/* Read child elements */
		elem->child_elem_list = bin_config_load_elem_list(bin_config, f);

		/* Free variable */
		free(var);
	}

	/* Return list */
	return elem_list;
}


int bin_config_load(struct bin_config_t *bin_config)
{
	gzFile f;

	/* Open file */
	f = gzopen(bin_config->file_name, "rb");
	if (!f)
	{
		bin_config->error_code = BIN_CONFIG_ERR_IO;
		return 0;
	}

	/* Clear configuration file */
	bin_config_clear(bin_config);
	if (bin_config->elem_list)
	{
		hash_table_free(bin_config->elem_list);
		bin_config->elem_list = NULL;
	}

	/* Load list of elements */
	bin_config->elem_list = bin_config_load_elem_list(bin_config, f);

	/* Close */
	gzclose(f);
	bin_config->error_code = BIN_CONFIG_ERR_OK;
	return 1;
}


void bin_config_dump(struct bin_config_t *bin_config, FILE *f)
{
	/* Dump list of elements */
	bin_config_dump_elem_list(bin_config->elem_list, 0, f);

	/* Success */
	bin_config->error_code = BIN_CONFIG_ERR_OK;
}


void bin_config_clear(struct bin_config_t *bin_config)
{
	struct hash_table_t *elem_list;
	struct bin_config_elem_t *elem;
	char *var;

	/* Get element list */
	elem_list = bin_config->elem_list;
	if (!elem_list)
		return;
	
	/* Empty list */
	while ((var = hash_table_find_first(elem_list, (void **) &elem)))
	{
		bin_config_elem_free(elem);
		hash_table_remove(elem_list, var);
	}

	/* Success */
	bin_config->error_code = BIN_CONFIG_ERR_OK;
}


struct bin_config_elem_t *bin_config_find_first(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem,
	char **var_ptr, void **data_ptr, int *size_ptr)
{
	struct hash_table_t *elem_list;
	struct bin_config_elem_t *elem;
	char *var;

	/* Reset return values */
	if (var_ptr)
		*var_ptr = NULL;
	if (data_ptr)
		*data_ptr = NULL;
	if (size_ptr)
		*size_ptr = 0;

	/* Check parent element */
	if (parent_elem && parent_elem->bin_config != bin_config)
	{
		bin_config->error_code = BIN_CONFIG_ERR_PARENT;
		return NULL;
	}

	/* Get list of elements */
	elem_list = parent_elem ? parent_elem->child_elem_list
		: bin_config->elem_list;
	if (!elem_list)
	{
		bin_config->error_code = BIN_CONFIG_ERR_NOT_FOUND;
		return NULL;
	}

	/* Get element */
	var = hash_table_find_first(elem_list, (void **) &elem);
	if (!var)
	{
		bin_config->error_code = BIN_CONFIG_ERR_NOT_FOUND;
		return NULL;
	}

	/* Return element */
	assert(elem);
	if (var_ptr)
		*var_ptr = var;
	if (data_ptr)
		*data_ptr = elem->data;
	if (size_ptr)
		*size_ptr = elem->size;
	bin_config->error_code = BIN_CONFIG_ERR_OK;
	return elem;
}


struct bin_config_elem_t *bin_config_find_next(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem,
	char **var_ptr, void **data_ptr, int *size_ptr)
{
	struct hash_table_t *elem_list;
	struct bin_config_elem_t *elem;
	char *var;

	/* Reset return values */
	if (var_ptr)
		*var_ptr = NULL;
	if (data_ptr)
		*data_ptr = NULL;
	if (size_ptr)
		*size_ptr = 0;

	/* Check parent element */
	if (parent_elem && parent_elem->bin_config != bin_config)
	{
		bin_config->error_code = BIN_CONFIG_ERR_PARENT;
		return NULL;
	}

	/* Get list of elements */
	elem_list = parent_elem ? parent_elem->child_elem_list
		: bin_config->elem_list;
	if (!elem_list)
	{
		bin_config->error_code = BIN_CONFIG_ERR_NOT_FOUND;
		return NULL;
	}

	/* Get element */
	var = hash_table_find_next(elem_list, (void **) &elem);
	if (!var)
	{
		bin_config->error_code = BIN_CONFIG_ERR_NOT_FOUND;
		return NULL;
	}

	/* Return element */
	assert(elem);
	if (var_ptr)
		*var_ptr = var;
	if (data_ptr)
		*data_ptr = elem->data;
	if (size_ptr)
		*size_ptr = elem->size;
	bin_config->error_code = BIN_CONFIG_ERR_OK;
	return elem;
}

