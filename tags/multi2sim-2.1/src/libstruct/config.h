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

#ifndef CONFIG_H
#define CONFIG_H

struct config_t;

/* Creation and destruction */
struct config_t *config_create(char *filename);
void config_free(struct config_t *cfg);

/* Load and save configuration;
 * Return value: non-0=ok, 0=file access failure */
int config_load(struct config_t *cfg);
int config_save(struct config_t *cfg);

/* Ask for section of key existence */
int config_section_exists(struct config_t *cfg, char *section);
int config_key_exists(struct config_t *cfg, char *section, char *key);

/* Enumerate sections */
char *config_section_first(struct config_t *cfg);
char *config_section_next(struct config_t *cfg);

/* Remove a key/section;
 * Return value: non-0=ok, 0=key/section does not exist */
int config_section_remove(struct config_t *cfg, char *section);
int config_key_remove(struct config_t *cfg, char *section, char *key);

/* Add keys in a section; if section does not exists, it is created;
 * If key already exists, replace old value;
 * String values are strdup'ped, so they can be modified in user program */
void config_write_string(struct config_t *cfg, char *section, char *key, char *value);
void config_write_int(struct config_t *cfg, char *section, char *key, int value);
void config_write_bool(struct config_t *cfg, char *section, char *key, int value);
void config_write_double(struct config_t *cfg, char *section, char *key, double value);
void config_write_ptr(struct config_t *cfg, char *section, char *key, void *value);

/* Read keys from a section; if section or key do not exist,
 * the default value if returned */
char *config_read_string(struct config_t *cfg, char *section, char *key, char *def);
int config_read_int(struct config_t *cfg, char *section, char *key, int def);
int config_read_bool(struct config_t *cfg, char *section, char *key, int def);
double config_read_double(struct config_t *cfg, char *section, char *key, double def);
void *config_read_ptr(struct config_t *cfg, char *section, char *key, void *def);

#endif

