/*
 *  Libstruct
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mhandle.h>
#include "config.h"
#include "hash.h"

#define BUFSIZE		1000
#define SECTIONS_SIZE	20
#define KEYS_SIZE	20


/*
 * TYPES
 */

/* main object;
 * data in 'sections' are of type 'struct hashtable_t';
 * data of 'sections' elements are of type 'char *'; */
struct config_t {
	char	*filename;
	struct	hashtable_t *sections;
};




/*
 * PRIVATE FUNCTIONS
 */

/* free 'keys' hash table */
static void free_keys(struct hashtable_t *keys)
{
	char *key, *value;
	
	/* free all values */
	key = hashtable_find_first(keys, (void **) &value);
	while (key) {
		free(value);
		key = hashtable_find_next(keys, (void **) &value);
	}
	
	/* free keys hash table */
	hashtable_free(keys);
}


/* free 'sections' hash table */
static void free_sections(struct hashtable_t *sections)
{
	char *section;
	struct hashtable_t *keys;
	
	/* free all 'keys' hash tables */
	section = hashtable_find_first(sections, (void **) &keys);
	while (section) {
		free_keys(keys);
		section = hashtable_find_next(sections, (void **) &keys);
	}
	
	/* free sections hash table */
	hashtable_free(sections);
}



/* create new section or return an existing one;
 * return value: ptr to keys hash table */
static struct hashtable_t *new_section(struct hashtable_t *sections, char *section)
{
	struct hashtable_t *keys;
	
	/* if section exists, return existing one */
	keys = (struct hashtable_t *) hashtable_get(sections, section);
	if (keys)
		return keys;
	
	/* create new section */
	keys = hashtable_create(KEYS_SIZE, 0);
	hashtable_insert(sections, section, keys);
	return keys;
}


/* create a new key or change the value of an existing one */
static void new_key(struct hashtable_t *keys, char *key, char *value)
{
	char *ovalue, *nvalue;
	
	/* if key already exists, free old value and set new one */
	ovalue = (char *) hashtable_get(keys, key);
	if (ovalue) {
		free(ovalue);
		nvalue = strdup(value);
		hashtable_set(keys, key, nvalue);
		return;
	}
	
	/* insert new value */
	nvalue = strdup(value);
	hashtable_insert(keys, key, nvalue);
}


/* delete spaces and final \n */
static char *trim(char *s)
{
	int len = strlen(s);
	while (*s == ' ' || *s == '\t')
		s++, len--;
	while (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\n')
		s[--len] = 0;
	return s;
}


/* break line in the form 'key=value' */
static void break_line(char *s, char **key, char **value)
{
	char *equal;
	
	/* if no equal sign, error */
	equal = index(s, '=');
	if (!equal) {
		*key = *value = NULL;
		return;
	}
	
	/* compute 'key' and 'value' */
	*equal = 0;
	*key = trim(s);
	*value = trim(equal + 1);
}




/*
 * PUBLIC FUNCTIONS
 */

/* creation and destruction */
struct config_t *config_create(char *filename)
{
	struct config_t *cfg;
	
	/* config object */
	cfg = malloc(sizeof(struct config_t));
	if (!cfg)
		return NULL;
	
	/* sections hash table & file name*/
	cfg->sections = hashtable_create(SECTIONS_SIZE, 0);
	cfg->filename = strdup(filename);
	if (!cfg->filename || !cfg->sections)
		return NULL;
	
	/* return created object */
	return cfg;
}


void config_free(struct config_t *cfg)
{
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
	
	/* try to open file for writing */
	f = fopen(cfg->filename, "wt");
	if (!f)
		return 0;
	
	/* dump sections */
	section = hashtable_find_first(cfg->sections, (void **) &keys);
	while (section) {
	
		/* print section header */
		fprintf(f, "[%s]\n", section);
		
		/* print all keys in section */
		key = hashtable_find_first(keys, (void **) &value);
		while (key) {
			fprintf(f, "%s=%s\n", key, value);
			key = hashtable_find_next(keys, (void **) &value);
		}
	
		/* next section */
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
	
	/* search section */
	keys = (struct hashtable_t *) hashtable_get(cfg->sections, section);
	if (!keys)
		return 0;
	
	/* search key */
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
	
	/* search section */
	keys = (struct hashtable_t *) hashtable_remove(cfg->sections, section);
	if (!keys)
		return 0;
	
	/* free keys */
	free_keys(keys);
	return 1;
}


int config_key_remove(struct config_t *cfg, char *section, char *key)
{
	struct hashtable_t *keys;
	char *value;
	
	/* search section */
	keys = (struct hashtable_t *) hashtable_get(cfg->sections, section);
	if (!keys)
		return 0;
	
	/* remove key */
	value = (char *) hashtable_remove(keys, key);
	if (!value)
		return 0;
	
	/* free removed value */
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
	
	/* search section */
	keys = (struct hashtable_t *) hashtable_get(cfg->sections, section);
	if (!keys)
		return def;
	
	/* search key */
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

