/*
 *  Libstruct
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

#include <lib/mhandle/mhandle.h>

#include "config.h"
#include "debug.h"
#include "hash-table.h"
#include "linked-list.h"
#include "string.h"


#define HASH_TABLE_SIZE		100

#define ITEM_ALLOWED  ((void *) 1)
#define ITEM_MANDATORY  ((void *) 2)


/* Error message */
static char *config_err_format =
	"\tA syntax error was detected while parsing a configuration INI file.\n"
	"\tThese files are formed of sections in brackets (e.g. '[ SectionName ]')\n"
	"\tfollowed by pairs 'VariableName = Value'. Comments preceded with ';'\n"
	"\tor '#' characters can be used, as well as blank lines. Please verify\n"
	"\tthe integrity of your input file and retry.\n";


/* Structure representing a configuration file.
 * The keys in the hash tables are strings representing section/variable names.
 *    If a key is the name of a section, it is represented as the name of the section.
 *    If a key is the name of a variable, it is represented as "<section>\n<var>"
 *
 */
struct config_t
{
	/* Text file name containing configuration */
	char *file_name;
	
	/* Hash table containing present elements
 	 * The values for sections are (void *) 1, while the values for variables are the
	 * actual value represented for that variable in the config file. */
	struct hash_table_t *items;

	/* Hash table containing allowed items.
	 * The keys are strings "<section>\n<variable>".
	 * The values are SECTION_VARIABLE_ALLOWED/SECTION_VARIABLE_MANDATORY */
	struct hash_table_t *allowed_items;

	/* Redundant linked list containing section names. This extra
	 * information is added to keep track of the order in which sections
	 * were loaded from a file or created by the user. Altering this order
	 * when saving the file could be annoying. Each element is an allocated
	 * string of type 'char *'. */
	struct linked_list_t *section_list;
};




/*
 * Private Functions
 */

/* Return a section and variable name from a string "<section>\n<var>" or "<section>". In the
 * latter case, the variable name is returned as an empty string. */
static void get_section_var_from_item(char *item, char *section, int section_size,
	char *var, int var_size)
{
	char *section_brk;

	snprintf(section, section_size, "%s", item);
	section_brk = index(section, '\n');
	if (section_brk)
	{
		*section_brk = '\0';
		snprintf(var, var_size, "%s", section_brk + 1);
	}
	else
	{
		assert(var_size);
		*var = '\0';
	}
}


/* Create string "<section>\n<var>" from separate strings.
 * String "<section>" is created if 'var' is NULL or an empty string.
 * Remove any spaces on the left or right of both the section and variable names. */
static void get_item_from_section_var(char *section, char *var, char *item, int size)
{
	char var_trim[MAX_LONG_STRING_SIZE];
	char section_trim[MAX_LONG_STRING_SIZE];

	assert(section && *section);
	str_single_spaces(section_trim, sizeof section_trim, section);
	if (var && *var)
	{
		str_trim(var_trim, sizeof var_trim, var);
		snprintf(item, size, "%s\n%s", section_trim, var_trim);
	}
	else
	{
		snprintf(item, size, "%s", section_trim);
	}
}


/* Return a variable name and its value from a string "<var>=<value>".
 * Both the returned variable and the value are trimmed.
 * If the input string format is wrong, a non-zero value is returned.
 * Zero is returned on success. */
static int get_var_value_from_item(char *item, char *var, int var_size,
	char *value, int value_size)
{
	char var_str[MAX_LONG_STRING_SIZE];
	char value_str[MAX_LONG_STRING_SIZE];

	char *equal_ptr;
	int equal_pos;

	equal_ptr = index(item, '=');
	if (!equal_ptr)
	{
		*var = '\0';
		*value = '\0';
		return 1;
	}

	/* Equal sign found, split string */
	equal_pos = equal_ptr - item;
	strncpy(var_str, item, equal_pos);
	var_str[equal_pos] = '\0';
	strncpy(value_str, equal_ptr + 1, strlen(item) - equal_pos - 1);
	value_str[strlen(item) - equal_pos - 1] = '\0';

	/* Return trimmed strings */
	str_trim(var, var_size, var_str);
	str_trim(value, value_size, value_str);
	return 0;
}


/* Return non-zero if the section already exists */
static int config_insert_section(struct config_t *config, char *section)
{
	char section_trim[MAX_LONG_STRING_SIZE];
	int ret;

	str_single_spaces(section_trim, sizeof section_trim, section);
	ret = hash_table_insert(config->items, section_trim, (void *) 1);
	if (ret)
		linked_list_add(config->section_list, xstrdup(section_trim));

	return !ret;
}


/* Return non-zero if the variable already exists */
static int config_insert_var(struct config_t *config, char *section, char *var, char *value)
{
	char item[MAX_LONG_STRING_SIZE];
	char value_trim[MAX_LONG_STRING_SIZE];
	char *ovalue, *nvalue;

	/* Combine section and variable */
	get_item_from_section_var(section, var, item, sizeof item);

	/* Allocate new value */
	str_trim(value_trim, sizeof value_trim, value);
	nvalue = xstrdup(value_trim);

	/* Free previous value if variable existed */
	ovalue = hash_table_get(config->items, item);
	if (ovalue)
	{
		free(ovalue);
		hash_table_set(config->items, item, nvalue);
		return 1;
	}

	/* Insert new value */
	hash_table_insert(config->items, item, nvalue);
	return 0;
}




/*
 * Public Functions
 */

/* Creation and destruction */
struct config_t *config_create(char *filename)
{
	struct config_t *config;
	
	/* Initialize */
	config = xcalloc(1, sizeof(struct config_t));
	config->file_name = xstrdup(filename);
	config->items = hash_table_create(HASH_TABLE_SIZE, 0);
	config->allowed_items = hash_table_create(HASH_TABLE_SIZE, 0);
	config->section_list = linked_list_create();

	/* Return */
	return config;
}


void config_free(struct config_t *config)
{
	char *item;
	void *value;
	char section[MAX_LONG_STRING_SIZE];
	char var[MAX_LONG_STRING_SIZE];

	/* Free variable values */
	HASH_TABLE_FOR_EACH(config->items, item, value)
	{
		get_section_var_from_item(item, section, sizeof section,
			var, sizeof var);
		if (var[0])
		{
			free(value);
			continue;
		}
		assert(value == ITEM_ALLOWED || value == ITEM_MANDATORY);
	}

	/* Free section list */
	LINKED_LIST_FOR_EACH(config->section_list)
		str_free(linked_list_get(config->section_list));
	linked_list_free(config->section_list);

	/* Free rest */
	free(config->file_name);
	hash_table_free(config->items);
	hash_table_free(config->allowed_items);
	free(config);
}


/* Get the file name */
char *config_get_file_name(struct config_t *config)
{
	return config->file_name ? config->file_name : "";
}


void config_load(struct config_t *config)
{
	FILE *f;

	char line[MAX_LONG_STRING_SIZE];
	char line_trim[MAX_LONG_STRING_SIZE];
	char *line_ptr;

	char section[MAX_LONG_STRING_SIZE];
	char var[MAX_LONG_STRING_SIZE];
	char value[MAX_LONG_STRING_SIZE];

	int line_num;
	int length;
	int err;
	
	/* Try to open file for reading */
	f = fopen(config->file_name, "rt");
	if (!f)
		fatal("%s: cannot open configuration file", config->file_name);
	
	/* Read lines */
	section[0] = '\0';
	line_num = 0;
	while (!feof(f))
	{
		/* Read a line */
		line_num++;
		line_ptr = fgets(line, sizeof line, f);
		if (!line_ptr)
			break;

		/* Trim line */
		str_trim(line_trim, sizeof line_trim, line);

		/* Comment or blank line */
		if (!line_trim[0] || line_trim[0] == ';' || line_trim[0] == '#')
			continue;
		
		/* New "[ <section> ]" entry */
		length = strlen(line_trim);
		if (line_trim[0] == '[' && line_trim[length - 1] == ']')
		{
			/* Get section name */
			line_trim[0] = ' ';
			line_trim[length - 1] = ' ';
			str_single_spaces(section, sizeof section, line_trim);

			/* Insert section */
			err = config_insert_section(config, section);
			if (err)
				fatal("%s: line %d: duplicated section '%s'.\n%s",
					config->file_name, line_num, section, config_err_format);

			/* Done for this line */
			continue;
		}

		/* Check that there is an active section */
		if (!section[0])
			fatal("%s: line %d: section name expected.\n%s",
				config->file_name, line_num, config_err_format);
		
		/* New "<var> = <value>" entry. */
		err = get_var_value_from_item(line_trim, var, sizeof var, value, sizeof value);
		if (err)
			fatal("%s: line %d: invalid format.\n%s",
				config->file_name, line_num, config_err_format);

		/* New variable */
		err = config_insert_var(config, section, var, value);
		if (err)
			fatal("%s: line %d: duplicated variable '%s'.\n%s",
				config->file_name, line_num, var, config_err_format);
	}
	
	/* Close file */
	fclose(f);
}


void config_save(struct config_t *config)
{
	char *section;
	char *item, *value;
	FILE *f;
	
	/* Try to open file for writing */
	f = fopen(config->file_name, "wt");
	if (!f)
		fatal("%s: cannot save configuration file", config->file_name);
	
	/* Dump all variables for each section */
	LINKED_LIST_FOR_EACH(config->section_list)
	{
		char section_buf[MAX_LONG_STRING_SIZE];
		char var_buf[MAX_LONG_STRING_SIZE];

		section = linked_list_get(config->section_list);
		fprintf(f, "[ %s ]\n", section);

		HASH_TABLE_FOR_EACH(config->items, item, value)
		{
			get_section_var_from_item(item, section_buf, sizeof section_buf,
				var_buf, sizeof var_buf);
			if (var_buf[0] && !strcmp(section_buf, section))
				fprintf(f, "%s = %s\n", var_buf, value);
		}

		fprintf(f, "\n");
	}

	/* close file */
	fclose(f);
}


int config_section_exists(struct config_t *config, char *section)
{
	char section_trim[MAX_LONG_STRING_SIZE];

	str_single_spaces(section_trim, sizeof section_trim, section);
	return hash_table_get(config->items, section_trim) != NULL;
}


int config_var_exists(struct config_t *config, char *section, char *var)
{
	char item[MAX_LONG_STRING_SIZE];

	get_item_from_section_var(section, var, item, sizeof item);
	return hash_table_get(config->items, item) != NULL;
}


int config_section_remove(struct config_t *config, char *section)
{
	fprintf(stderr, "%s: not implemented\n", __FUNCTION__);
	return 1;
}


int config_key_remove(struct config_t *config, char *section, char *key)
{
	fprintf(stderr, "%s: not implemented\n", __FUNCTION__);
	return 1;
}




/*
 * Enumeration of sections
 */

char *config_section_first(struct config_t *config)
{
	linked_list_head(config->section_list);
	return linked_list_get(config->section_list);
}


char *config_section_next(struct config_t *config)
{
	linked_list_next(config->section_list);
	return linked_list_get(config->section_list);
}




/*
 * Writing to configuration file
 */

void config_write_string(struct config_t *config, char *section, char *var, char *value)
{
	char item[MAX_LONG_STRING_SIZE];

	/* Add section and variable to the set of allowed items, as long as
	 * it is not added already as a mandatory item. */
	get_item_from_section_var(section, var, item, sizeof item);
	if (!hash_table_get(config->allowed_items, section))
		hash_table_insert(config->allowed_items, section, ITEM_ALLOWED);
	if (!hash_table_get(config->allowed_items, item))
		hash_table_insert(config->allowed_items, item, ITEM_ALLOWED);
	
	/* Write value */
	config_insert_section(config, section);
	config_insert_var(config, section, var, value);
}


void config_write_int(struct config_t *config, char *section, char *var, int value)
{
	char value_str[MAX_LONG_STRING_SIZE];

	sprintf(value_str, "%d", value);
	config_write_string(config, section, var, value_str);
}


void config_write_llint(struct config_t *config, char *section, char *var, long long value)
{
	char value_str[MAX_LONG_STRING_SIZE];

	sprintf(value_str, "%lld", value);
	config_write_string(config, section, var, value_str);
}


void config_write_bool(struct config_t *config, char *section, char *var, int value)
{
	char value_str[MAX_LONG_STRING_SIZE];

	strcpy(value_str, value ? "True" : "False");
	config_write_string(config, section, var, value_str);
}


void config_write_double(struct config_t *config, char *section, char *var, double value)
{
	char value_str[MAX_LONG_STRING_SIZE];

	sprintf(value_str, "%g", value);
	config_write_string(config, section, var, value_str);
}


void config_write_ptr(struct config_t *config, char *section, char *var, void *value)
{
	char value_str[MAX_LONG_STRING_SIZE];

	sprintf(value_str, "%p", value);
	config_write_string(config, section, var, value_str);
}




/*
 * Reading from configuration file
 */

char *config_read_string(struct config_t *config, char *section, char *var, char *def)
{
	char item[MAX_LONG_STRING_SIZE];
	char *value;

	/* Add section and variable to the set of allowed items, as long as
	 * it is not added already as a mandatory item. */
	get_item_from_section_var(section, var, item, sizeof item);
	if (!hash_table_get(config->allowed_items, section))
		hash_table_insert(config->allowed_items, section, ITEM_ALLOWED);
	if (!hash_table_get(config->allowed_items, item))
		hash_table_insert(config->allowed_items, item, ITEM_ALLOWED);
	
	/* Read value */
	value = hash_table_get(config->items, item);
	return value ? value : def;
}


int config_read_int(struct config_t *config, char *section, char *var, int def)
{
	char *result;

	int value;
	int err;

	/* Read value */
	result = config_read_string(config, section, var, NULL);
	if (!result)
		return def;

	/* Convert */
	value = str_to_int(result, &err);
	if (err)
		fatal("%s: Section [ %s ], variable %s = '%s': %s\n",
			config_get_file_name(config), section, var, result, str_error(err));

	/* Return */
	return value;
}


long long config_read_llint(struct config_t *config, char *section, char *var, long long def)
{
	char *result;
	long long value;
	int err;

	/* Read value */
	result = config_read_string(config, section, var, NULL);
	if (!result)
		return def;
	
	/* Convert */
	value = str_to_llint(result, &err);
	if (err)
		fatal("%s: Section [ %s ], variable %s = '%s': %s\n",
			config_get_file_name(config), section, var, result, str_error(err));

	/* Return */
	return value;
}


int config_read_bool(struct config_t *config, char *section, char *var, int def)
{
	char *result;

	/* Read variable */
	result = config_read_string(config, section, var, NULL);
	if (!result)
		return def;

	/* True */
	if (!strcasecmp(result, "t") || !strcasecmp(result, "True")
		|| !strcasecmp(result, "On"))
		return 1;
	
	/* False */
	if (!strcasecmp(result, "f") || !strcasecmp(result, "False")
		|| !strcasecmp(result, "Off"))
		return 0;

	/* Invalid value */
	fatal("%s: Section [ %s ]: Invalid value for '%s'\n"
		"\tPossible values are {t|True|On|f|False|Off}\n",
		config_get_file_name(config), section, var);
	return 0;
}


double config_read_double(struct config_t *config, char *section, char *var, double def)
{
	char *result;
	double d;

	result = config_read_string(config, section, var, NULL);
	if (!result)
		return def;
	sscanf(result, "%lf", &d);
	return d;
}


static void enumerate_map(char **map, int map_count)
{
	int i;
	char *comma = "";

	fprintf(stderr, "Possible allowed values are { ");
	for (i = 0; i < map_count; i++)
	{
		fprintf(stderr, "%s%s", comma, map[i]);
		comma = ", ";
	}
	fprintf(stderr, " }\n");
}


int config_read_enum(struct config_t *config, char *section, char *var, int def, char **map, int map_count)
{
	char *result;
	int i;

	result = config_read_string(config, section, var, NULL);
	if (!result)
		return def;
	
	/* Translate */
	for (i = 0; i < map_count; i++)
		if (!strcasecmp(map[i], result))
			return i;
	
	/* No match found with map */
	fprintf(stderr, "%s: section '[ %s ]': variable '%s': invalid value ('%s')\n",
		config->file_name, section, var, result);
	enumerate_map(map, map_count);
	exit(1);
}


void *config_read_ptr(struct config_t *config, char *section, char *var, void *def)
{
	char *result;
	void *ptr;
	result = config_read_string(config, section, var, NULL);
	if (!result)
		return def;
	sscanf(result, "%p", &ptr);
	return ptr;
}




/*
 * Configuration file format
 */

/* Insert a section (and variable) into the hash table of allowed sections (variables).
 * If the item was there, update it with the new allowed/mandatory property.
 * Field 'property' should be ITEM_ALLOWED/ITEM_MANDATORY. */
static void allowed_items_insert(struct config_t *config, char *section, char *var, void *property)
{
	char item[MAX_LONG_STRING_SIZE];

	get_item_from_section_var(section, var, item, sizeof item);
	if (hash_table_get(config->allowed_items, item))
		hash_table_set(config->allowed_items, item, property);
	hash_table_insert(config->allowed_items, item, property);
}


/* Return true if an item is allowed (or mandatory).
 * Argument 'var' can be NULL or an empty string to refer to a section. */
static int item_is_allowed(struct config_t *config, char *section, char *var)
{
	char item[MAX_LONG_STRING_SIZE];

	get_item_from_section_var(section, var, item, sizeof item);
	return hash_table_get(config->allowed_items, item) != NULL;
}


/* Return true if an item is present in the configuration file.
 * Argument 'var' can be NULL or an empty string to refer to a section. */
static int item_is_present(struct config_t *config, char *section, char *var)
{
	char item[MAX_LONG_STRING_SIZE];

	get_item_from_section_var(section, var, item, sizeof item);
	return hash_table_get(config->items, item) != NULL;
}


void config_section_allow(struct config_t *config, char *section)
{
	allowed_items_insert(config, section, NULL, ITEM_ALLOWED);
}


void config_section_enforce(struct config_t *config, char *section)
{
	allowed_items_insert(config, section, NULL, ITEM_MANDATORY);
}


void config_var_allow(struct config_t *config, char *section, char *var)
{
	allowed_items_insert(config, section, var, ITEM_ALLOWED);
}


void config_var_enforce(struct config_t *config, char *section, char *var)
{
	allowed_items_insert(config, section, var, ITEM_MANDATORY);
}


void config_check(struct config_t *config)
{
	char *item;
	void *property;

	char section[MAX_LONG_STRING_SIZE];
	char var[MAX_LONG_STRING_SIZE];

	/* Go through mandatory items and check they are present */
	for (item = hash_table_find_first(config->allowed_items, &property);
		item; item = hash_table_find_next(config->allowed_items, &property))
	{
		
		/* If this is an allowed (not mandatory) item, continue */
		if (property == ITEM_ALLOWED)
			continue;

		/* Item must be in the configuration file */
		get_section_var_from_item(item, section, sizeof section,
			var, sizeof var);
		if (!item_is_present(config, section, NULL))
			fatal("%s: section [ %s ] missing",
				config->file_name, section);
		if (!item_is_present(config, section, var))
			fatal("%s: section [ %s ]: missing mandatory variable '%s'",
				config->file_name, section, var);
	}
	
	/* Go through all present sections/keys and check they are present in the
	 * set of allowed/mandatory items. */
	for (item = hash_table_find_first(config->items, NULL);
		item; item = hash_table_find_next(config->items, NULL))
	{
		/* Check if it is allowed */
		get_section_var_from_item(item, section, sizeof section,
			var, sizeof var);
		if (item_is_allowed(config, section, var))
			continue;

		/* It is not, error */
		if (!var[0])
			fatal("%s: invalid section [ %s ]",
				config->file_name, section);
		else
			fatal("%s: section [ %s ]: invalid variable '%s'",
				config->file_name, section, var);
	}
}


void config_section_check(struct config_t *config, char *section_ref)
{
	char *item;
	void *property;

	char section[MAX_LONG_STRING_SIZE];
	char var[MAX_LONG_STRING_SIZE];

	/* Go through mandatory items and check they are present */
	for (item = hash_table_find_first(config->allowed_items, &property);
		item; item = hash_table_find_next(config->allowed_items, &property))
	{
		
		/* If this is an allowed (not mandatory) item, continue */
		if (property == ITEM_ALLOWED)
			continue;

		/* Check that item is associated with section */
		get_section_var_from_item(item, section, sizeof section,
			var, sizeof var);
		if (strcasecmp(section, section_ref))
			continue;

		/* Check that variable is present */
		if (!item_is_present(config, section, var))
			fatal("%s: section [ %s ]: missing mandatory variable '%s'",
				config->file_name, section, var);
	}
	
	/* Go through all present sections/keys and check they are present in the
	 * set of allowed/mandatory items. */
	for (item = hash_table_find_first(config->items, NULL);
		item; item = hash_table_find_next(config->items, NULL))
	{
		/* Check it this is the section we're interested in */
		get_section_var_from_item(item, section, sizeof section,
			var, sizeof var);
		if (strcasecmp(section, section_ref))
			continue;

		/* Check if it is allowed */
		if (!item_is_allowed(config, section, var))
			fatal("%s: section [ %s ]: invalid variable '%s'",
				config->file_name, section, var);
	}
}

