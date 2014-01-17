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
		delete(item);
		return 1;
	}
	
	/* Set new value */
	HashTableInsert(self->item_table, asObject(item), asObject(value));
	delete(item);
	return 0;
}





/*
 * Class 'IniFile'
 */

void IniFileCreate(IniFile *self, char *path)
{
	self->path = new(String, path);
	self->section_index = -1;

	self->item_table = new(HashTable);
	HashTableSetCaseSensitive(self->item_table, 0);

	self->allowed_item_table = new(HashTable);
	HashTableSetCaseSensitive(self->allowed_item_table, 0);

	self->enforced_item_table = new(HashTable);
	HashTableSetCaseSensitive(self->enforced_item_table, 0);

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

	HashTableDeleteObjects(self->enforced_item_table);
	delete(self->enforced_item_table);

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


int IniFileSectionExists(IniFile *self, char *section)
{
	String section_str;

	new_static(&section_str, String, section);
	StringSingleSpaces(&section_str);
	HashTableGet(self->item_table, asObject(&section_str));
	delete_static(&section_str);
	return !self->item_table->error;
}


int IniFileVariableExists(IniFile *self, char *section, char *var)
{
	String *item;
	String section_str;
	String var_str;

	new_static(&section_str, String, section);
	new_static(&var_str, String, var);

	item = IniFileSectionVariableToItem(&section_str, &var_str);
	HashTableGet(self->item_table, asObject(item));

	delete_static(&section_str);
	delete_static(&var_str);
	delete(item);

	return !self->item_table->error;
}


int IniFileRemoveSection(IniFile *self, char *section)
{
	List *item_list;

	String *item;
	String *value;
	String *tmp_section;

	int length;

	/* Create list of items to remove */
	item_list = new(List);
	length = strlen(section);
	HashTableForEach(self->item_table, item, String)
		if (!strncasecmp(item->text, section, length)
				&& item->text[length] == '\n')
			ListAdd(item_list, asObject(item));
	
	/* Section not found */
	if (!item_list->count)
	{
		delete(item_list);
		return 1;
	}

	/* Remove items */
	ListHead(item_list);
	while (item_list->count)
	{
		item = asString(ListRemove(item_list));
		value = asString(HashTableRemove(self->item_table, asObject(item)));
		if (value)
			delete(value);
	}

	/* Remove it from the list of sections */
	ListForEach(self->section_list, tmp_section, String)
	{
		if (!strcasecmp(tmp_section->text, section))
		{
			ListRemove(self->section_list);
			delete(tmp_section);
			break;
		}
	}

	/* Success */
	delete(item_list);
	return 0;
}


int IniFileRemoveVariable(IniFile *self, char *section, char *var)
{
	String section_str;
	String var_str;

	String *item;
	String *value;

	new_static(&section_str, String, section);
	new_static(&var_str, String, var);
	item = IniFileSectionVariableToItem(&section_str, &var_str);

	value = asString(HashTableRemove(self->item_table, asObject(item)));
	if (value)
		delete(value);

	delete(item);
	delete_static(&section_str);
	delete_static(&var_str);

	return !!self->item_table->error;
}


char *IniFileFirstSection(IniFile *self)
{
	/* No section */
	if (!self->section_list->count)
	{
		self->section_index = -1;
		return NULL;
	}

	/* Return first section */
	self->section_index = 0;
	ListHead(self->section_list);
	return asString(ListGet(self->section_list))->text;
}


char *IniFileNextSection(IniFile *self)
{
	/* Invalid iterator */
	if (self->section_index < 0)
		return NULL;

	/* Increment */
	self->section_index++;

	/* Past the end */
	if (self->section_index >= self->section_list->count)
	{
		self->section_index = -1;
		return NULL;
	}

	/* Return current section */
	ListGoto(self->section_list, self->section_index);
	return asString(ListGet(self->section_list))->text;
}


void IniFileWriteString(IniFile *self, char *section, char *var, char *value)
{
	String section_str;
	String var_str;
	String value_str;

	String *item;

	/* Create item, section, and variable strings */
	new_static(&section_str, String, section);
	new_static(&var_str, String, var);
	new_static(&value_str, String, value);
	item = IniFileSectionVariableToItem(&section_str, &var_str);
	
	/* Insert section and variable into table of allowed items */
	HashTableInsertString(self->allowed_item_table, section, NULL);
	HashTableInsert(self->allowed_item_table, asObject(item), NULL);

	/* Write value */
	IniFileInsertSection(self, &section_str);
	IniFileInsertVariable(self, &section_str, &var_str, &value_str);

	/* Free strings */
	delete_static(&section_str);
	delete_static(&var_str);
	delete_static(&value_str);
	delete(item);
}


void IniFileWriteInt(IniFile *self, char *section, char *var, int value)
{
	char s[100];
	
	snprintf(s, sizeof s, "%d", value);
	IniFileWriteString(self, section, var, s);
}


void IniFileWriteInt64(IniFile *self, char *section, char *var, long long value)
{
	char s[100];
	
	snprintf(s, sizeof s, "%lld", value);
	IniFileWriteString(self, section, var, s);
}


void IniFileWriteBool(IniFile *self, char *section, char *var, int value)
{
	char s[100];
	
	snprintf(s, sizeof s, "%s", value ? "True" : "False");
	IniFileWriteString(self, section, var, s);
}


void IniFileWriteDouble(IniFile *self, char *section, char *var, double value)
{
	char s[100];
	
	snprintf(s, sizeof s, "%f", value);
	IniFileWriteString(self, section, var, s);
}


void IniFileWriteEnum(IniFile *self, char *section, char *var, int value, StringMap map)
{
	char s[100];
	
	snprintf(s, sizeof s, "%s", StringMapValue(map, value));
	IniFileWriteString(self, section, var, s);
}


void IniFileWritePointer(IniFile *self, char *section, char *var, void *value)
{
	char s[100];
	
	snprintf(s, sizeof s, "%p", value);
	IniFileWriteString(self, section, var, s);
}


char *IniFileReadString(IniFile *self, char *section, char *var, char *def)
{
	String section_str;
	String var_str;

	String *item;
	String *value;

	/* Create item, section, and variable strings */
	new_static(&section_str, String, section);
	new_static(&var_str, String, var);
	item = IniFileSectionVariableToItem(&section_str, &var_str);
	
	/* Insert section and variable into table of allowed items */
	HashTableInsertString(self->allowed_item_table, section, NULL);
	HashTableInsert(self->allowed_item_table, asObject(item), NULL);

	/* Read value */
	value = asString(HashTableGet(self->item_table, asObject(item)));

	/* Free strings */
	delete_static(&section_str);
	delete_static(&var_str);
	delete(item);

	/* Return value */
	return value ? value->text : def;
}


int IniFileReadInt(IniFile *self, char *section, char *var, int def)
{
	String s;

	char *text;

	int value;
	int err;

	/* Read value */
	text = IniFileReadString(self, section, var, NULL);
	if (!text)
		return def;

	/* Convert */
	new_static(&s, String, text);
	value = StringToInt(&s, &err);
	delete_static(&s);
	if (err)
		fatal("%s: section [%s], variable %s, value '%s': %s\n",
				self->path->text, section, var, text,
				StringGetErrorString(err));

	/* Return */
	return value;
}


long long IniFileReadInt64(IniFile *self, char *section, char *var, long long def)
{
	String s;

	char *text;

	int value;
	int err;

	/* Read value */
	text = IniFileReadString(self, section, var, NULL);
	if (!text)
		return def;

	/* Convert */
	new_static(&s, String, text);
	value = StringToInt64(&s, &err);
	delete_static(&s);
	if (err)
		fatal("%s: section [%s], variable %s, value '%s': %s\n",
				self->path->text, section, var, text,
				StringGetErrorString(err));

	/* Return */
	return value;
}


int IniFileReadBool(IniFile *self, char *section, char *var, int def)
{
	char *text;

	/* Read variable */
	text = IniFileReadString(self, section, var, NULL);
	if (!text)
		return def;

	/* True */
	if (!strcasecmp(text, "t") || !strcasecmp(text, "True")
			|| !strcasecmp(text, "On"))
		return 1;
	
	/* False */
	if (!strcasecmp(text, "f") || !strcasecmp(text, "False")
			|| !strcasecmp(text, "Off"))
		return 0;

	/* Invalid value */
	fatal("%s: section [%s], variable '%s', invalid value '%s'\n"
			"\tPossible values are {t|True|On|f|False|Off}\n",
			self->path->text, section, var, text);
	return 0;
}


double IniFileReadDouble(IniFile *self, char *section, char *var, double def)
{
	char *text;
	double value;

	/* Read value */
	text = IniFileReadString(self, section, var, NULL);
	if (!text)
		return def;
	
	/* Convert */
	sscanf(text, "%lf", &value);
	return value;
}


int IniFileReadEnum(IniFile *self, char *section, char *var, int def, StringMap map)
{
	char *text;

	int value;
	int err;

	/* Read value */
	text = IniFileReadString(self, section, var, NULL);
	if (!text)
		return def;
	
	/* Convert */
	value = StringMapStringCaseErr(map, text, &err);
	if (!err)
		return value;

	/* Error, show options */
	fatal("%s: section [%s], variable '%s', invalid value '%s'\n"
			"\tPossible values are %s",
			self->path->text, section, var, text,
			StringMapGetValues(map)->text);
	return 0;
}


void *IniFileReadPointer(IniFile *self, char *section, char *var, void *def)
{
	char *text;
	void *pointer;

	/* Read value */
	text = IniFileReadString(self, section, var, NULL);
	if (!text)
		return def;
	
	/* Convert */
	sscanf(text, "%p", &pointer);
	return pointer;
}


void IniFileAllowSection(IniFile *self, char *section)
{
	HashTableInsertString(self->allowed_item_table, section, NULL);
}


void IniFileEnforceSection(IniFile *self, char *section)
{
	HashTableInsertString(self->allowed_item_table, section, NULL);
	HashTableInsertString(self->enforced_item_table, section, NULL);
}


void IniFileAllowVariable(IniFile *self, char *section, char *var)
{
	String section_str;
	String var_str;
	String *item;

	/* Create item */
	new_static(&section_str, String, section);
	new_static(&var_str, String, var);
	item = IniFileSectionVariableToItem(&section_str, &var_str);

	/* Insert it */
	HashTableInsertString(self->allowed_item_table, section, NULL);
	HashTableInsert(self->allowed_item_table, asObject(item), NULL);

	/* Free */
	delete_static(&section_str);
	delete_static(&var_str);
	delete(item);
}


void IniFileEnforceVariable(IniFile *self, char *section, char *var)
{
	String section_str;
	String var_str;
	String *item;

	/* Create item */
	new_static(&section_str, String, section);
	new_static(&var_str, String, var);
	item = IniFileSectionVariableToItem(&section_str, &var_str);

	/* Insert it */
	HashTableInsertString(self->allowed_item_table, section, NULL);
	HashTableInsertString(self->enforced_item_table, section, NULL);
	HashTableInsert(self->allowed_item_table, asObject(item), NULL);
	HashTableInsert(self->enforced_item_table, asObject(item), NULL);

	/* Free */
	delete_static(&section_str);
	delete_static(&var_str);
	delete(item);
} 


void IniFileCheck(IniFile *self)
{
	String *item;
	String *section;
	String *var;

	/* Check that all mandatory items are present */
	HashTableForEach(self->enforced_item_table, item, String)
	{
		/* Item is present */
		HashTableGet(self->item_table, asObject(item));
		if (!self->item_table->error)
			continue;

		/* Item not present */
		IniFileItemToSectionVariable(item, &section, &var);
		if (var)
			fatal("%s: section [%s], variable '%s' missing",
				self->path->text, section->text, var->text);
		else
			fatal("%s: section [%s] missing",
				self->path->text, section->text);
	}

	/* Check that all present items are allowed */
	HashTableForEach(self->item_table, item, String)
	{
		/* Item is present */
		HashTableGet(self->allowed_item_table, asObject(item));
		if (!self->allowed_item_table->error)
			continue;

		/* Item not present */
		IniFileItemToSectionVariable(item, &section, &var);
		if (var)
			fatal("%s: section [%s], invalid variable '%s'",
				self->path->text, section->text, var->text);
		else
			fatal("%s: invalid section [%s]",
				self->path->text, section->text);
	}
}

