/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef LIB_CLASS_INI_FILE_H
#define LIB_CLASS_INI_FILE_H

#include "class.h"
#include "string.h"


/*
 * Class 'IniFile'
 */

CLASS_BEGIN(IniFile, Object)

	/* File name */
	String *path;
	
	/* Hash table containing present items. The keys are strings with the
	 * template "<section>" for sections and "<section>\n<var>" for
	 * variables. The data are NULL for sections, and the variable value for
	 * variables. */
	HashTable *item_table;

	/* Hash table containing allowed items. The keys are strings
	 * "<section>\n<variable>", and the data elements are always NULL. */
	HashTable *allowed_item_table;

	/* Table of enforced items. Each element is a string with template
	 * "<section>\n<variable>", and data elements are always NULL. */
	HashTable *enforced_item_table;

	/* Redundant linked list containing section names. This extra
	 * information is added to keep track of the order in which sections
	 * were loaded from a file or created by the user. Altering this order
	 * when saving the file could be annoying. Each element is an allocated
	 * string of type 'char *'. */
	List *section_list;

	/* Internal iterator for sections. */
	int section_index;

CLASS_END(IniFile)


void IniFileCreate(IniFile *self, char *path);
void IniFileDestroy(IniFile *self);

void IniFileDump(Object *self, FILE *f);

/* Load/save INI file */
void IniFileLoad(IniFile *self);
void IniFileSave(IniFile *self);

/* Return true if a section/variable exists. */
int IniFileSectionExists(IniFile *self, char *section);
int IniFileVariableExists(IniFile *self, char *section, char *var);

/* Remove a section/variable. These functions return 0 on success, and a
 * non-zero value if the section/variable does not exist. */
int IniFileRemoveSection(IniFile *self, char *section);
int IniFileRemoveVariable(IniFile *self, char *section, char *var);

/* Enumeration of sections */
char *IniFileFirstSection(IniFile *self);
char *IniFileNextSection(IniFile *self);
#define IniFileForEachSection(inifile, section) \
	for ((section) = IniFileFirstSection((inifile)); \
		(section); (section) = IniFileNextSection((inifile)))

/* Add variables in a section; if section does not exists, it is created.
 * If variable already exists, replace old value. */
void IniFileWriteString(IniFile *self, char *section, char *var, char *value);
void IniFileWriteInt(IniFile *self, char *section, char *var, int value);
void IniFileWriteInt64(IniFile *self, char *section, char *var, long long value);
void IniFileWriteBool(IniFile *self, char *section, char *var, int value);
void IniFileWriteDouble(IniFile *self, char *section, char *var, double value);
void IniFileWriteEnum(IniFile *self, char *section, char *var, int value, StringMap map);
void IniFileWritePointer(IniFile *self, char *section, char *var, void *value);

/* Read variables from a section. If a section or variable does not exist, the
 * default value in 'def' is returned. Variables read with IniFileReadXXX
 * functions are added automatically to the list of allowed variables. */
char *IniFileReadString(IniFile *self, char *section, char *var, char *def);
int IniFileReadInt(IniFile *self, char *section, char *var, int def);
long long IniFileReadInt64(IniFile *self, char *section, char *var, long long def);
int IniFileReadBool(IniFile *self, char *section, char *var, int def);
double IniFileReadDouble(IniFile *self, char *section, char *var, double def);
int IniFileReadEnum(IniFile *self, char *section, char *var, int def, StringMap map);
void *IniFileReadPointer(IniFile *self, char *section, char *var, void *def);

/* Definition or allowed/mandatory sections and variables. */
void IniFileAllowSection(IniFile *self, char *section);
void IniFileEnforceSection(IniFile *self, char *section);
void IniFileAllowVariable(IniFile *self, char *section, char *var);
void IniFileEnforceVariable(IniFile *self, char *section, char *var);

/* Check that all enforced variables and sections are present in the file, and
 * that all variables/sections present in the file are allowed. */
void IniFileCheck(IniFile *self);

#endif

