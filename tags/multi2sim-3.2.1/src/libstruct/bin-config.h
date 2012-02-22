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

#ifndef BIN_CONFIG_H
#define BIN_CONFIG_H

/* Error codes generated by functions. */
enum bin_config_error_t
{
	BIN_CONFIG_ERR_OK = 0,
	BIN_CONFIG_ERR_NOT_FOUND,
	BIN_CONFIG_ERR_PARENT,
	BIN_CONFIG_ERR_DATA,
	BIN_CONFIG_ERR_DUPLICATE,
	BIN_CONFIG_ERR_IO,
	BIN_CONFIG_ERR_FORMAT
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

	/* Data */
	void *data;
	int size;

	/* If TRUE, the 'data' pointer contains a newly allocated memory
	 * that needs to be freed. Otherwise, it is a pointer allocated
	 * and handled by the caller. */
	int dup_data;
};


/* Binary configuration file */
struct bin_config_t
{
	/* Associated file name */
	char *file_name;

	/* Error code updated by each call 'bin_config_xxx' except
	 * creation and destruction calls. The possible values are
	 * enumerated in 'enum bin_config_error_t'. */
	int error_code;

	/* List of elements */
	struct hash_table_t *elem_list;
};


/* Creation and destruction.
 * The creation function associates a file path to the configuration
 * file. If there is not enough virtual memory, the function
 * aborts the program. */
struct bin_config_t *bin_config_create(char *file_name);
void bin_config_free(struct bin_config_t *bin_config);


/** Load configuration file from associated file.
 *
 * @param bin_config
 *	Configuration file object.
 *
 * @return
 *	The function returns a non-zero value if the operation succeeds, and 0
 *	otherwise. The error code is updated to one of the following values:
 *
 *	BIN_CONFIG_ERR_OK
 *		No error.
 *	BIN_CONFIG_ERR_IO
 *		I/O error accessing file.
 *	BIN_CONFIG_ERR_FORMAT
 *		Bad format of configuration file.
 */
int bin_config_load(struct bin_config_t *bin_config);


/** Save configuration file into its associated file.
 *
 * @param bin_config
 *	Configuration file object.
 *
 * @return
 *	The function returns a non-zero value if the operation succeeds, and 0
 *	otherwise. The error code is updated to one of the following values:
 *
 *	BIN_CONFIG_ERR_OK
 *		No error.
 *	BIN_CONFIG_ERR_IO
 *		I/O error accessing file.
 */
int bin_config_save(struct bin_config_t *bin_config);


/** Dump configuration in a human-readable format.
 *
 * @param bin_config
 *	Configuration file object.
 * @param f
 * 	File to dump information (e.g., stdout).
 *
 * @return
 *	No value is returned.
 *	The error code is set to BIN_CONFIG_ERR_OK.
 */
void bin_config_dump(struct bin_config_t *bin_config, FILE *f);


/** Clear configuration contents.
 *
 * @param bin_config
 *	Configuration file object.
 *
 * @return
 *	No value is returned.
 *	The error code is set to BIN_CONFIG_ERR_OK.
 */
void bin_config_clear(struct bin_config_t *bin_config);


/** Add a variable to the configuration file.
 *
 * @param bin_config
 *	Configuration file object.
 * @param parent_elem
 *	Parent element to add the variable to. If NULL, the variable will be
 *	added to the highest level of the hierarchy. This argument should be the
 *	element returned by a previous call to 'bin_config_add'.
 * @param var
 *	Variable to add. If the variable already exists, the function will fail.
 *	The variable name will be internally duplicated, so the caller can
 *	safely rewrite it without affecting the configuration file.
 * @param data
 *	Data associated with the variable (can be NULL). The data will be
 *	duplicated internally, so an external change will not affect the internal
 *	contents.
 * @param size
 *	Size of the data.
 *
 * @return
 *	The function returns a pointer to the created element. This pointer can
 *	be used in future calls to create child elements. In case of error, the
 *	function returns NULL. The error code is updated with one of the
 *	following values:
 *
 *	BIN_CONFIG_ERR_OK
 *		No error.
 *	BIN_CONFIG_ERR_DUPLICATE
 *		A variable with the same name already exists.
 *	BIN_CONFIG_ERR_DATA
 *		Argument 'size' is greater than 0, but 'data' is NULL.
 *	BIN_CONFIG_ERR_PARENT
 *		Element 'parent_elem' does not belong to 'bin_config'.
 */
struct bin_config_elem_t *bin_config_add(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem, char *var, void *data, int size);


/** Add a variable to the configuration file with no contents duplication.
 *
 * @param bin_config
 *	Configuration file object.
 * @param parent_elem
 *	Parent element to add the variable to. If NULL, the variable will be
 *	added to the highest level of the hierarchy. This argument should be the
 *	element returned by a previous call to 'bin_config_add'.
 * @param var
 *	Variable to add. If the variable already exists, the function will fail.
 * @param data:
 *	Data associated with the variable (can be NULL). The data will not be
 *	duplicated in this case. The caller needs to guarantee that the pointed
 *	value remains valid as long as the 'bin_config' object exists.
 * @param size
 *	Size of the data.
 *
 * @return
 *	The function returns a pointer to the created element. This pointer can
 *	be used in future calls to create child elements. In case of error, the
 *	function returns NULL. The error code is updated with one of the
 *	following values:
 *
 *	BIN_CONFIG_ERR_OK
 *		No error.
 *	BIN_CONFIG_ERR_DUPLICATE
 *		A variable with the same name already exists.
 *	BIN_CONFIG_ERR_DATA
 *		Argument 'size' is greater than 0, but 'data' is NULL.
 *	BIN_CONFIG_ERR_PARENT
 *		Element 'parent_elem' does not belong to 'bin_config'.
 */
struct bin_config_elem_t *bin_config_add_no_dup(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem, char *var, void *data, int size);


/** Remove a variable and all its children from configuration file.
 *
 * @param bin_config
 *	Configuration file object.
 * @param parent_elem
 *	Element where to search for the variable. If NULL, the variable is
 *	searched in the highest level of the hierarchy.
 * @param var
 *	Name of the variable.
 *
 * @return
 *	The function returns non-0 on success.
 *	If there was an error, the function returns 0 and the error code is
 *	updated accordingly to any of the following values:
 *	BIN_CONFIG_ERR_OK
 *		No error.
 *	BIN_CONFIG_ERR_NOT_FOUND
 *		Variable was not found.
 *	BIN_CONFIG_ERR_PARENT
 *		Element 'parent_elem' does not belong to 'bin_config'.
 */
int bin_config_remove(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem, char *var);


/** Get the data associated with a variable.
 *
 * @param bin_config
 *	Configuration file object.
 * @param parent_elem
 *	Element where to search for the variable. If NULL, the variable is
 *	searched in the highest level of the hierarchy.
 * @param var
 *	Name of the variable.
 * @param data_ptr
 *	If not NULL, pointer to a variable where the data associated with the
 *	variable should be returned.
 * @param size_ptr
 *	If not NULL, pointer to a variable where the size of the data associated
 *	with the variable should be returned.
 *
 * @return
 *	On success, the function returns the element associated with the
 *	requested variable. Fields 'data' and 'size' of the returned object are
 *	the same as those returned in 'data_ptr' and 'size_ptr'. On failure, the
 *	function returns NULL. After this call, the error code is set to one of
 *	the following values:
 *
 *	BIN_CONFIG_ERR_OK
 *		No error.
 *	BIN_CONFIG_ERR_NOT_FOUND
 *		Variable was not found.
 *	BIN_CONFIG_ERR_PARENT
 *		Element 'parent_elem' does not belong to 'bin_config'.
 */	
struct bin_config_elem_t *bin_config_get(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem, char *var,
	void **data_ptr, int *size_ptr);


/** Start an enumeration of variables.
 * This call should be followed by a sequence of 'bin_config_find_next'.
 *
 * @param bin_config
 *	Configuration file object.
 * @param parent_elem
 *	Parent element to look for child variables. If NULL, the enumeration
 *	will start in the highest level of the hierarchy.
 * @param var_ptr
 *	If not NULL, the first variable name in the enumeration will be stored
 *	at the location pointed to by 'var_ptr'.
 * @param data_ptr
 *	If not NULL, pointer to a variable where the data associated with the
 *	first child element should be returned.
 * @param size_ptr
 *	If not NULL, pointer to a variable where the size of the data associated
 *	with the first child element should be returned.
 *
 * @return
 *	On success, the function returns the first element found in the list.
 *	Fields 'data' and 'size' of the returned object are the same as those
 *	returned in 'data_ptr' and 'size_ptr'. If there is no element in the
 *	list, the function returns NULL.
 *	The error code will be set to one of the following values:
 *
 *	BIN_CONFIG_ERR_OK
 *		No error.
 *	BIN_CONFIG_ERR_NOT_FOUND
 *		No element found in the list.
 *	BIN_CONFIG_ERR_PARENT
 *		Element 'parent_elem' does not belong to 'bin_config'.
 */
struct bin_config_elem_t *bin_config_find_first(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem,
	char **var_ptr, void **data_ptr, int *size_ptr);


/** Continue an enumeration of variables.
 *
 * @param bin_config
 *	Configuration file object.
 * @param parent_elem
 *	Parent element to look for child variables. If NULL, the enumeration
 *	will start in the highest level of the hierarchy.
 * @param var_ptr
 *	If not NULL, the first variable name in the enumeration will be stored
 *	at the location pointed to by 'var_ptr'.
 * @param data_ptr
 *	If not NULL, pointer to a variable where the data associated with the
 *	first child element should be returned.
 * @param size_ptr
 *	If not NULL, pointer to a variable where the size of the data associated
 *	with the first child element should be returned.
 *
 * @return
 *	On success, the function returns the next element found in the list, and
 *	variables pointed to by 'var_ptr', 'data_ptr', and 'size_ptr' are set
 *	accordingly. If no more elements were found in the list, the function
 *	returns NULL.
 *	This call must be preceded by a call to 'bin_config_find_first'. The
 *	return value is undefined if any call to other 'bin_config_xxx' function
 *	was performed between 'bin_config_find_first' and subsequent calls to
 *	'bin_config_find_next'.
 *	The error code is set to one of the following values:
 *
 *	BIN_CONFIG_ERR_OK
 *		No error.
 *	BIN_CONFIG_ERR_NOT_FOUND
 *		No more elements found in the list.
 *	BIN_CONFIG_ERR_PARENT
 *		Element 'parent_elem' does not belong to 'bin_config'.
 */
struct bin_config_elem_t *bin_config_find_next(struct bin_config_t *bin_config,
	struct bin_config_elem_t *parent_elem,
	char **var_ptr, void **data_ptr, int *size_ptr);

#endif

