/*
 *  Libstruct
 *  Copyright (C) 2007  Rafael Ubal Tena (ubal@gap.upv.es)
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mhandle.h>
#include "config.h"
#include "hash.h"

#define BUFSIZE		1000
#define SECTIONS_SIZE	20
#define KEYS_SIZE	20


/* Data type in 'sections' is of type 'struct hashtable_t';
 * Data type of 'sections' elements is of type 'char *'; */
struct config_t {
	char *filename;
	struct hashtable_t *sections;

	/* Hash table containing allowed/mandatory sections/keys.
	 * The keys are strings representing section/key names. In case a string
	 * represents a key, it is stored as "<section_name>\n<key_name>"
	 * The valuese are NULL. */
	struct hashtable_t *allowed_items;
	struct hashtable_t *enforced_items;
};




/*
 * Private Functions
 */

/* Free 'keys' hash table */
static void free_keys(struct hashtable_t *keys)
{
	char *key, *value;
	
	/* Free all values */
	key = hashtable_find_first(keys, (void **) &value);
	while (key) {
		free(value);
		key = hashtable_find_next(keys, (void **) &value);
	}
	
	/* Free keys hash table */
	hashtable_free(keys);
}


/* Free 'sections' hash table */
static void free_sections(struct hashtable_t *sections)
{
	char *section;
	struct hashtable_t *keys;
	
	/* Free all 'keys' hash tables */
	section = hashtable_find_first(sections, (void **) &keys);
	while (section) {
		free_keys(keys);
		section = hashtable_find_next(sections, (void **) &keys);
	}
	
	/* Free sections hash table */
	hashtable_free(sections);
}



/* Create new section or return an existing one.
 * Return value: ptr to keys hash table */
static struct hashtable_t *new_section(struct hashtable_t *sections, char *section)
{
	struct hashtable_t *keys;
	
	/* If section exists, return existing one */
	keys = (struct hashtable_t *) hashtable_get(sections, section);
	if (keys)
		return keys;
	
	/* Create new section */
	keys = hashtable_create(KEYS_SIZE, 0);
	hashtable_insert(sections, section, keys);
	return keys;
}


/* Create a new key or change the value of an existing one */
static void new_key(struct hashtable_t *keys, char *key, char *value)
{
	char *ovalue, *nvalue;
	
	/* If key already exists, free old value and set new one */
	ovalue = (char *) hashtable_get(keys, key);
	if (ovalue) {
		free(ovalue);
		nvalue = strdup(value);
		hashtable_set(keys, key, nvalue);
		return;
	}
	
	/* Insert new value */
	nvalue = strdup(value);
	hashtable_insert(keys, key, nvalue);
}


/* Delete spaces and final \n */
static char *trim(char *s)
{
	int len = strlen(s);
	while (*s == ' ' || *s == '\t')
		s++, len--;
	while (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\n')
		s[--len] = 0;
	return s;
}


/* Break line in the form 'key=value' */
static void break_line(char *s, char **key, char **value)
{
	char *equal;
	
	/* If no equal sign, error */
	equal = index(s, '=');
	if (!equal) {
		*key = *value = NULL;
		return;
	}
	
	/* Compute 'key' and 'value' */
	*equal = 0;
	*key = trim(s);
	*value = trim(equal + 1);
}




/*
 * Public Functions
 */

/* Creation and destruction */
struct config_t *config_create(char *filename)
{
	struct config_t *cfg;
	
	/* Config object */
	cfg = calloc(1, sizeof(struct config_t));
	if (!cfg)
		return NULL;
	
	/* Sections hash table & file name*/
	cfg->sections = hashtable_create(SECTIONS_SIZE, 0);
	cfg->filename = strdup(filename);
	if (!cfg->filename || !cfg->sections)
		return NULL;
	
	/* Other hash tables */
	cfg->allowed_items = hashtable_create(SECTIONS_SIZE, 0);
	cfg->enforced_items = hashtable_create(SECTIONS_SIZE, 0);
	if (!cfg->allowed_items || !cfg->enforced_items)
		return NULL;
	
	/* Return created object */
	return cfg;
}


void config_free(struct config_t *cfg)
{
	hashtable_free(cfg->allowed_items);
	hashtable_free(cfg->enforced_items);
	free_sections(cfg->sections);
	free(cfg->filename);
	free(cfg);
}


int config_load(struct config_t *cfg)
{
	FILE *f;
	char buf[BUFSIZE], *line, *key, *value;
	struct hashtable_t *keys = NULL;
	
	/* free old data and create new sections hash table */
	free_sections(cfg->sections);
	cfg->sections = hashtable_create(SECTIONS_SIZE, 0);
	
	/* try to open file for reading */
	f = fopen(cfg->filename, "rt");
	if (!f)
		return 0;
	
	/* read lines */
	while (!feof(f)) {
	
		/* read a line */
		line = fgets(buf, BUFSIZE, f);
		if (!line)
			break;
		line = trim(line);
		
		/* is it a new section? */
		if (*line == '[' && line[strlen(line) - 1] == ']') {
			line++;
			line[strlen(line) - 1] = 0;
			line = trim(line);
			keys = new_section(cfg->sections, line);
			continue;
		}
		
		/* if no section active or wrong entry, ignore entry */
		break_line(line, &key, &value);
		if (!keys || !key)
			continue;
		
		/* add key */
		new_key(keys, key, value);
	}
	
	/* close file */
	fclose(f);
	return 1;
}


int config_save(struct config_t *cfg)
{
	FILE *f;
	char *section, *key, *value;
	struct hashtable_t *keys;
	
	/* Try to open file for writing */
	f = fopen(cfg->filename, "wt");
	if (!f)
		return 0;
	
	/* Dump sections */
	section = hashtable_find_first(cfg->sections, (void **) &keys);
	while (section) {
	
		/* Print section header */
		fprintf(f, "[%s]\n", section);
		
		/* Print all keys in section */
		key = hashtable_find_first(keys, (void **) &value);
		while (key) {
			fprintf(f, "%s=%s\n", key, value);
			key = hashtable_find_next(keys, (void **) &value);
		}
	
		/* Next section */
		section = hashtable_find_next(cfg->sections, (void **) &keys);
		fprintf(f, "\n");
	}
	
	/* close file */
	fclose(f);
	return 1;
}


int config_section_exists(struct config_t *cfg, char *section)
{
	return hashtable_get(cfg->sections, section) != NULL;
}


int config_key_exists(struct config_t *cfg, char *section, char *key)
{
	struct hashtable_t *keys;
	
	/* Search section */
	keys = (struct hashtable_t *) hashtable_get(cfg->sections, section);
	if (!keys)
		return 0;
	
	/* Search key */
	return hashtable_get(keys, key) != NULL;
}


char *config_section_first(struct config_t *cfg)
{
	return hashtable_find_first(cfg->sections, NULL);
}


char *config_section_next(struct config_t *cfg)
{
	return hashtable_find_next(cfg->sections, NULL);
}


int config_section_remove(struct config_t *cfg, char *section)
{
	struct hashtable_t *keys;
	
	/* Search section */
	keys = (struct hashtable_t *) hashtable_remove(cfg->sections, section);
	if (!keys)
		return 0;
	
	/* Free keys */
	free_keys(keys);
	return 1;
}


int config_key_remove(struct config_t *cfg, char *section, char *key)
{
	struct hashtable_t *keys;
	char *value;
	
	/* Search section */
	keys = (struct hashtable_t *) hashtable_get(cfg->sections, section);
	if (!keys)
		return 0;
	
	/* Remove key */
	value = (char *) hashtable_remove(keys, key);
	if (!value)
		return 0;
	
	/* Free removed value */
	free(value);
	return 1;
}


void config_write_string(struct config_t *cfg, char *section, char *key, char *value)
{
	struct hashtable_t *keys;
	keys = new_section(cfg->sections, section);
	new_key(keys, key, value);
}


void config_write_int(struct config_t *cfg, char *section, char *key, int value)
{
	char s[BUFSIZE];
	sprintf(s, "%d", value);
	config_write_string(cfg, section, key, s);
}


void config_write_bool(struct config_t *cfg, char *section, char *key, int value)
{
	char s[BUFSIZE];
	strcpy(s, value ? "t" : "f");
	config_write_string(cfg, section, key, s);
}


void config_write_double(struct config_t *cfg, char *section, char *key, double value)
{
	char s[BUFSIZE];
	sprintf(s, "%f", value);
	config_write_string(cfg, section, key, s);
}


void config_write_ptr(struct config_t *cfg, char *section, char *key, void *value)
{
	char s[BUFSIZE];
	sprintf(s, "%p", value);
	config_write_string(cfg, section, key, s);
}


char *config_read_string(struct config_t *cfg, char *section, char *key, char *def)
{
	struct hashtable_t *keys;
	char *value;
	
	/* Search section */
	keys = (struct hashtable_t *) hashtable_get(cfg->sections, section);
	if (!keys)
		return def;
	
	/* Search key */
	value = (char *) hashtable_get(keys, key);
	if (!value)
		return def;
	return value;
}


int config_read_int(struct config_t *cfg, char *section, char *key, int def)
{
	char *result;
	result = config_read_string(cfg, section, key, NULL);
	return result ? atoi(result) : def;
}


int config_read_bool(struct config_t *cfg, char *section, char *key, int def)
{
	char *result;
	result = config_read_string(cfg, section, key, NULL);
	if (!result)
		return def;
	if (!strcmp(result, "t") || !strcmp(result, "true") ||
		!strcmp(result, "true") || !strcmp(result, "True") ||
		!strcmp(result, "TRUE"))
		return 1;
	return 0;
}


double config_read_double(struct config_t *cfg, char *section, char *key, double def)
{
	char *result;
	double d;
	result = config_read_string(cfg, section, key, NULL);
	if (!result)
		return def;
	sscanf(result, "%lf", &d);
	return d;
}


void *config_read_ptr(struct config_t *cfg, char *section, char *key, void *def)
{
	char *result;
	void *ptr;
	result = config_read_string(cfg, section, key, NULL);
	if (!result)
		return def;
	sscanf(result, "%p", &ptr);
	return ptr;
}


void config_section_allow(struct config_t *cfg, char *section)
{
	char section_copy[BUFSIZE];
	char *section_trimmed;

	strcpy(section_copy, section);
	section_trimmed = trim(section_copy);
	hashtable_insert(cfg->allowed_items, section_trimmed, (void *) 1);
}


void config_section_enforce(struct config_t *cfg, char *section)
{
	char section_copy[BUFSIZE];
	char *section_trimmed;

	strcpy(section_copy, section);
	section_trimmed = trim(section_copy);
	hashtable_insert(cfg->enforced_items, section_trimmed, (void *) 1);
}


void config_key_allow(struct config_t *cfg, char *section, char *key)
{
	char section_copy[BUFSIZE], key_copy[BUFSIZE], value[BUFSIZE];
	char *section_trimmed, *key_trimmed;

	strcpy(section_copy, section);
	strcpy(key_copy, key);
	section_trimmed = trim(section_copy);
	key_trimmed = trim(key_copy);
	snprintf(value, BUFSIZE, "%s\n%s", section_trimmed, key_trimmed);
	hashtable_insert(cfg->allowed_items, value, (void *) 1);
}


void config_key_enforce(struct config_t *cfg, char *section, char *key)
{
	char section_copy[BUFSIZE], key_copy[BUFSIZE], value[BUFSIZE];
	char *section_trimmed, *key_trimmed;

	strcpy(section_copy, section);
	strcpy(key_copy, key);
	section_trimmed = trim(section_copy);
	key_trimmed = trim(key_copy);
	snprintf(value, BUFSIZE, "%s\n%s", section_trimmed, key_trimmed);
	hashtable_insert(cfg->enforced_items, value, (void *) 1);
}


static void split_section_key(char *str, char *section, char *key)
{
	char *section_brk;

	strcpy(section, str);
	section_brk = index(section, '\n');
	if (section_brk) {
		*section_brk = '\0';
		strcpy(key, section_brk + 1);
	} else
		*key = '\0';
}


void config_check(struct config_t *cfg)
{
	char item_str[BUFSIZE], section_str[BUFSIZE], key_str[BUFSIZE];
	struct hashtable_t *keys;
	char *item, *section, *key;

	/* Go through mandatory items and check they are present */
	item = hashtable_find_first(cfg->enforced_items, NULL);
	while (item) {

		/* Find value */
		split_section_key(item, section_str, key_str);
		keys = (struct hashtable_t *) hashtable_get(cfg->sections, section_str);
		if (!keys) {
			fprintf(stderr, "%s: section '[ %s ]' not found in configuration file\n",
				cfg->filename, section_str);
			exit(1);
		}
	
		/* Search key */
		if (key_str[0] && !hashtable_get(keys, key_str)) {
			fprintf(stderr, "%s: section '[ %s ]': variable '%s' is missing in the configuration file\n",
				cfg->filename, section_str, key_str);
			exit(1);
		}

		/* Next element */
		item = hashtable_find_next(cfg->enforced_items, NULL);
	}
	
	/* Go through all present sections/keys and check they are present in the
	 * set of allowed/enforced items. */
	section = hashtable_find_first(cfg->sections, (void **) &keys);
	while (section) {
	
		/* Check that section name is allowed */
		if (!hashtable_get(cfg->enforced_items, section) && !hashtable_get(cfg->allowed_items, section)) {
			fprintf(stderr, "%s: section '[ %s ]' is not valid in the configuration file\n",
				cfg->filename, section);
			exit(1);
		}
		
		/* Print all keys in section */
		key = hashtable_find_first(keys, NULL);
		while (key) {

			/* Check that key name is allowed */
			snprintf(item_str, BUFSIZE, "%s\n%s", section, key);
			if (!hashtable_get(cfg->enforced_items, item_str) && !hashtable_get(cfg->allowed_items, item_str)) {
				fprintf(stderr, "%s: section '[ %s ]': variable '%s' is not valid in configuration file\n",
					cfg->filename, section, key);
				exit(1);
			}

			/* Next key */
			key = hashtable_find_next(keys, NULL);
		}
	
		/* Next section */
		section = hashtable_find_next(cfg->sections, (void **) &keys);
	}
}


void config_section_check(struct config_t *cfg, char *section)
{
}

