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

#include <cstring>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Misc.h"


class IniFile
{
	/* Case-insensitive string hash */
	struct KeyHash
	{
		size_t operator()(std::string s) const { Misc::StringToLower(s);
				return std::hash<std::string>()(s); }
	};

	/* Case-insensitive string compare */
	struct KeyCompare
	{
		bool operator()(const std::string& a, const std::string& b) const
				{ return !strcasecmp(a.c_str(), b.c_str()); }
	};

	/* File name */
	std::string path;
	
	/* Hash table containing present items. The keys are strings with the
	 * template "<section>" for sections and "<section>|<var>" for
	 * variables. The data are NULL for sections, and the variable value for
	 * variables. */
	std::unordered_map<std::string, std::string, KeyHash, KeyCompare> items;

	/* Hash table containing allowed items. The keys are strings
	 * "<section>|<variable>". */
	std::unordered_set<std::string, KeyHash, KeyCompare> allowed_items;

	/* Table of enforced items. Each element is a string with template
	 * "<section>|<variable>". */
	std::unordered_set<std::string, KeyHash, KeyCompare> enforced_items;

	/* Redundant list containing section names. This extra
	 * information is added to keep track of the order in which sections
	 * were loaded from a file or created by the user. Altering this order
	 * when saving the file could be annoying. */
	std::vector<std::string> sections;

	void ItemToSectionVar(std::string item, std::string& section,
			std::string& var);
	std::string SectionVarToItem(std::string section, std::string var);
	bool GetVarValue(std::string& s, std::string& var, std::string& value);
	bool InsertSection(std::string section);
	bool InsertVariable(std::string section, std::string var,
			std::string value);
public:

	IniFile();
	IniFile(std::string path);

	void Load(std::string path);
	void Save(std::string path);
	void Dump(std::ostream& os);

	/* Return true if a section or variable exists */
	bool Exists(std::string section);
	bool Exists(std::string section, std::string var);

	/* Remove a section or a variable, and return true if the section/
	 * variable was found in the file. */
	bool Remove(std::string section);
	bool Remove(std::string section, std::string var);

	unsigned int GetNumSections() { return sections.size(); }
	std::string GetSection(unsigned int index) { return index <
			sections.size() ? sections[index] : ""; }
	
	/* Add variables in a section; if section does not exists, it is created.
	 * If variable already exists, replace old value. */
	void WriteString(std::string section, std::string var, std::string value);
	void WriteInt(std::string section, std::string var, int value);
	void WriteInt64(std::string section, std::string var, long long value);
	void WriteBool(std::string section, std::string var, bool value);
	void WriteDouble(std::string section, std::string var, double value);
	void WriteEnum(std::string section, std::string var, int value, Misc::StringMap map);
	void WritePointer(std::string section, std::string var, void *value);

	/* Read variables from a section. If a section or variable does not exist, the
	 * default value in 'def' is returned. Variables read with IniFileReadXXX
	 * functions are added automatically to the list of allowed variables. */
	std::string ReadString(std::string section, std::string var, std::string def = "");
	int ReadInt(std::string section, std::string var, int def = 0);
	long long ReadInt64(std::string section, std::string var, long long def = 0);
	bool ReadBool(std::string section, std::string var, bool def = false);
	double ReadDouble(std::string section, std::string var, double def = 0.0);
	int ReadEnum(std::string section, std::string var, Misc::StringMap map, int def = 0);
	void *ReadPointer(std::string section, std::string var, void *def = NULL);

	/* Allowed/mandatory sections/variables */
	void Allow(std::string section);
	void Allow(std::string section, std::string var);
	void Enforce(std::string section);
	void Enforce(std::string section, std::string var);

	/* Check that all enforced variables and sections are present in the
	 * file, and that all variables/sections present in the file are
	 * allowed. */
	void Check();
};


#endif

