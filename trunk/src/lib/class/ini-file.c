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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>

#include "hash-table.h"
#include "ini-file.h"
#include "list.h"


static char *ini_file_err_format =
	"\tA syntax error was detected while parsing a configuration INI file.\n"
	"\tThese files are formed of sections in brackets (e.g. '[ SectionName ]')\n"
	"\tfollowed by pairs 'VariableName = Value'. Comments preceded with ';'\n"
	"\tor '#' characters can be used, as well as blank lines. Please verify\n"
	"\tthe integrity of your input file and retry.\n";


/* Return a section and variable name from a string "<section>\n<var>" or
 * "<section>". In the latter case, the variable name is returned as NULL. It is
 * the responsibility of the caller to delete() the returned strings. */
void IniFileItemToSectionVariable(String *item, String **section_ptr,
		String **var_ptr)
{
	List *tokens;

	/* Extract tokens */
	tokens = StringTokenize(item, "\n");
	if (tokens->count != 1 && tokens->count != 2)
		panic("%s: invalid number of tokens", __FUNCTION__);

	/* Save tokens */
	assert(section_ptr);
	assert(var_ptr);
	ListHead(tokens);
	*section_ptr = asString(ListRemove(tokens));
	*var_ptr = asString(ListRemove(tokens));
	delete(tokens);
}


/* Create string "<section>\n<var>" from separate strings.
 * String "<section>" is created if 'var' is NULL or an empty string.
 * Remove any spaces on the left or right of both the section and variable
 * names. */
String *IniFileSectionVariableToItem(String *section, String *var)
{
	String *item;

	/* Trim section and variable strings */
	StringSingleSpaces(section);
	if (var)
		StringTrim(var, "\t ");

	/* Check section name */
	if (!section->length)
		panic("%s: invalid section name", __FUNCTION__);

	/* Create return string */
	item = new(String, section->text);
	if (var && var->length)
		StringConcat(item, "\n%s", var->text);
	
	/* Return */
	return item;
}


/* Given a string in the format "<var>=<value>", return a string containing the
 * variable and another string containing the value. If the input string format
 * is invalid, a non-zero error code is returned. On success, the function
 * returns 0. */
static int IniFileGetVarValue(String *var_value, String **var, String **value)
{
	int pos;

	/* Check syntax */
	if (StringCount(var_value, "=") != 1)
		return 1;

	/* Split string */
	assert(var);
	assert(value);
	pos = StringIndex(var_value, "=");
	*var = StringSubStr(var_value, 0, pos);
	*value = StringSubStr(var_value, pos + 1, var_value->length);
	StringTrim(*var, "\n\r\t ");
	StringTrim(*value, "\n\r\t ");
	return 0;
}


/* Insert a new section into the 'item_table' hash table. If the section already
 * exists, return a non-zero error code. Return 0 on success. */
static int IniFileInsertSection(IniFile *self, String *section)
{
	HashTableInsert(self->item_table, asObject(section), NULL);
	if (!self->item_table->error)
		ListAdd(self->section_list, asObject(new(String,
				section->text)));

	return self->item_table->error;
}


/* Insert a variable and its value into the 'item_table' hash table. The string
 * in 'value' is duplicated and stored as the value.
 * If 'var' already existed, the previous value is freed, and replaced by the
 * new value, returning non-zero. If 'var' did not exist, return 0. */
static int IniFileInsertVariable(IniFile *self, String *section,
		String *var, String *value)
{
	String *item;
	String *old_value;

	/* Combine section and variable */
	item = IniFileSectionVariableToItem(section, var);

	/* Duplicate value */
	value = new(String, value->text);

	/* Remove previous value if existing */
	old_value = asString(HashTableGet(self->item_table, asObject(item)));
	if (old_value)
	{
		delete(old_value);
		HashTableSet(self->item_table, asObject(item), asObject(value));
		return 1;
	}
	
	/* Set new value */
	HashTableInsert(self->item_table, asObject(item), asObject(value));
	return 0;
}





/*
 * Class 'IniFile'
 */

void IniFileCreate(IniFile *self, char *path)
{
	self->path = new(String, path);

	self->item_table = new(HashTable);
	HashTableSetCaseSensitive(self->item_table, 0);

	self->allowed_item_table = new(HashTable);
	HashTableSetCaseSensitive(self->allowed_item_table, 0);

	self->enforced_item_list = new(List);
	self->section_list = new(List);

	/* Virtual functions */
	asObject(self)->Dump = IniFileDump;
}


void IniFileDestroy(IniFile *self)
{
	delete(self->path);

	HashTableDeleteObjects(self->item_table);
	delete(self->item_table);

	HashTableDeleteObjects(self->allowed_item_table);
	delete(self->allowed_item_table);

	ListDeleteObjects(self->enforced_item_list);
	delete(self->enforced_item_list);

	ListDeleteObjects(self->section_list);
	delete(self->section_list);
}


void IniFileDump(Object *self, FILE *f)
{
	IniFile *ini_file = asIniFile(self);

	String *tmp_section;
	String *section;
	String *item;
	String *var;
	String *value;

	ListForEach(ini_file->section_list, section, String)
	{
		fprintf(f, "[ %s ]\n", section->text);
		HashTableForEach(ini_file->item_table, item, String)
		{
			if (!strncasecmp(item->text, section->text, section->length)
					&& item->text[section->length] == '\n')
			{
				value = asString(HashTableGet(ini_file->item_table,
						asObject(item)));
				IniFileItemToSectionVariable(item, &tmp_section, &var);
				fprintf(f, "%s = '%s'\n", var->text, value->text);
				delete(tmp_section);
				delete(var);
			}
		}
		fprintf(f, "\n");
	}
}


void IniFileLoad(IniFile *self)
{
	FILE *f;

	String *line;
	String *section;
	String *var;
	String *value;

	int line_num;
	int err;
	
	/* Try to open file for reading */
	f = fopen(self->path->text, "rt");
	if (!f)
		fatal("%s: cannot open INI file",
				self->path->text);
	
	/* Read lines */
	section = new(String, "");
	line = new(String, "");
	line_num = 0;
	for (;;)
	{
		/* Read a line */
		line_num++;
		err = StringRead(line, f);
		if (err)
			break;

		/* Comment or blank line */
		StringTrim(line, "\n\t\r ");
		if (!line->length || line->text[0] == ';' || line->text[0] == '#')
			continue;
		
		/* New "[ <section> ]" entry */
		if (line->text[0] == '[' && line->text[line->length - 1] == ']')
		{
			/* Get section name */
			delete(section);
			section = StringSubStr(line, 1, line->length - 2);
			StringSingleSpaces(section);

			/* Insert section */
			err = IniFileInsertSection(self, section);
			if (err)
				fatal("%s: line %d: duplicated section '%s'.\n%s",
					self->path->text, line_num,
					section->text, ini_file_err_format);

			/* Done for this line */
			continue;
		}

		/* Check that there is an active section */
		if (!section->length)
			fatal("%s: line %d: section name expected.\n%s",
				self->path->text, line_num, ini_file_err_format);
		
		/* New "<var> = <value>" entry. */
		err = IniFileGetVarValue(line, &var, &value);
		if (err)
			fatal("%s: line %d: invalid format.\n%s",
				self->path->text, line_num, ini_file_err_format);

		/* New variable */
		err = IniFileInsertVariable(self, section, var, value);
		if (err)
			fatal("%s: line %d: duplicated variable '%s'.\n%s",
				self->path->text, line_num, var->text,
				ini_file_err_format);

		/* Free */
		delete(var);
		delete(value);
	}
	
	/* End */
	delete(line);
	delete(section);
	fclose(f);
}


void IniFileSave(IniFile *self)
{
	FILE *f;

	/* Try to open file for writing */
	f = fopen(self->path->text, "wt");
	if (!f)
		fatal("%s: cannot save configuration file",
				self->path->text);
	
	/* Dump */
	IniFileDump(asObject(self), f);

	/* Close */
	fclose(f);
	
}


int IniFileSectionExsists(IniFile *self, char *section)
{
	return 0;
}


int IniFileVariableExists(IniFile *self, char *section, char *var)
{
	return 0;
}


int IniFileRemoveSection(IniFile *self, char *section)
{
	return 0;
}


int IniFileRemoveVariable(IniFile *self, char *section, char *var)
{
	return 0;
}


char *IniFileFirstSection(IniFile *self)
{
	return 0;
}


char *IniFileNextSection(IniFile *self)
{
	return 0;
}


void IniFileWriteString(IniFile *self, char *section, char *var, char *value)
{
}


void IniFileWriteInt(IniFile *self, char *section, char *var, int value)
{
}


void IniFileWriteInt64(IniFile *self, char *section, char *var, long long value)
{
}


void IniFileWriteBool(IniFile *self, char *section, char *var, int value)
{
}


void IniFileWriteDouble(IniFile *self, char *section, char *var, double value)
{
}


void IniFileWriteEnum(IniFile *self, char *section, char *var, int value, StringMap map)
{
}


void IniFileWritePointer(IniFile *self, char *section, char *var, void *value)
{
}


char *IniFileReadString(IniFile *self, char *section, char *var, char *def)
{
	return NULL;
}


int IniFileReadInt(IniFile *self, char *section, char *var, int def)
{
	return 0;
}


long long IniFileReadInt64(IniFile *self, char *section, char *var, long long def)
{
	return 0;
}


int IniFileReadBool(IniFile *self, char *section, char *var, int def)
{
	return 0;
}


double IniFileReadDouble(IniFile *self, char *section, char *var, double def)
{
	return 0;
}


int IniFileReadEnum(IniFile *self, char *section, char *var, int def, StringMap map)
{
	return 0;
}


void *IniFileReadPointer(IniFile *self, char *section, char *var, void *def)
{
	return NULL;
}


void IniFileAllowSection(IniFile *self, char *section)
{
}


void IniFileEnforceSection(IniFile *self, char *section)
{
}


void IniFileAllowVariable(IniFile *self, char *section, char *var)
{
}


void IniFileEnforceVariable(IniFile *self, char *section, char *var)
{
}


void IniFileCheck(IniFile *self)
{
}


void IniFileCheckSection(IniFile *self, char *section)
{
}

