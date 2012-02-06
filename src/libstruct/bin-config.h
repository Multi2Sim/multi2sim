/*
 *  Libstruct
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

#ifndef ELF_FORMAT_H
#define ELF_FORMAT_H

#include <bin-config.h>

enum bin_config_error_t
{
	ERR_BIN_CONFIG_OK = 0,
	ERR_BIN_CONFIG_NOT_FOUND,
	ERR_BIN_CONFIG_DUPLICATE,
	ERR_BIN_CONFIG_FORMAT
};


/* One element in binary configuration file. */
struct bin_config_elem_t
{
	/* Configuration file where it belongs. */
	struct bin_config_t *bin_config;

	/* Parent element - NULL if no upper level */
	struct bin_config_elem_t *parent_elem;

	/* List of child elements - NULL if no children */
	struct hash_table_t *child_elem_list;

	/* Size of 'data' field */
	int size;

	/* Data contained in element. This field must be the last
	 * in the structure. The total size allocated for this structure
	 * will depend on the size allocated for the data. */
	unsigned char data[0];
};


/* Binary configuration file */
struct bin_config_t
{
	/* Associated file name */
	char *file_name;

	/* List of elements */
	struct hash_table_t *elem_list;
};


/* Creation and destruction */
struct bin_config_t *bin_config_create(char *file_name);
void bin_config_free(struct bin_config_t *bin_config);

/* Load configuration file from associated file */
int bin_config_load(struct bin_config_t *bin_config);
int bin_config_save(struct bin_config_t *bin_config);


/* Add a variable to the configuration file.
 * Arguments:
 *   bin_config: Configuration file object.
 *   parent_elem: Parent element to add the variable to. If NULL, the variable
 *     will be added to the highest level of the hierarchy. This argument should
 *     be the element returned by a previous call to 'bin_config_add'.
 *   var: Variable to add. If the variable already exists, the function will fail.
 *   data: Data associated with the variable (can be NULL). This data will be
 *     duplicated internally, so an external change will not affect the internal
 *     contents.
 *   size: Size of the data.
 * Return value:
 *   The function returns a pointer to the created element, or NULL in case of
 *   error. The returned pointer can be used in future calls to create child elements.
 * Update to 'bin_config->error_code':
 *   ERR_BIN_CONFIG_OK: no error.
 *   ERR_BIN_CONFIG_DUPLICATE: valiable with the same name already exists.
 */
struct bin_config_elem_t *bin_config_add(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem, char *var, void *data, int size);


/* Remove element associated with a variable.
 * Arguments:
 *   bin_config: Configuration file object.
 *   parent_elem: Element where to search for the variable. If NULL, the variable
 *     is searched for in the highest level of the hierarchy.
 *   var: Name of the variable.
 * Return value:
 *   The function returns 0.
 * The variable is searched as a child of 'parent_elem', or in the highest level
 * list of elements if 'parent_elem' is NULL.
 * The function returns an error code of type ERR_BIN_CONFIG_XXX. */
int bin_config_remove(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem, char *var);

/* Get the data associated with an variable, given the variable name and its parent
 * element.
 * The function returns the element associated with the requested variable, or NULL
 * if the variable does not exist. The 'data' and 'size' fields of the returned
 * element are the same as those returned in the 'data' and 'size' arguments of
 * the function. */
struct bin_config_elem_t *bin_config_get(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem, char *var,
	void **data, int *size);

#endif

