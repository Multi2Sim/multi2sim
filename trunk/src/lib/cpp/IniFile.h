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

#include <cassert>
#include <cstring>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Debug.h"
#include "String.h"


namespace misc
{


class IniFile
{
	// Case-insensitive string hash
	struct KeyHash
	{
		size_t operator()(std::string s) const
		{
			misc::StringToLower(s);
			return std::hash<std::string>()(s);
		}
	};

	// Case-insensitive string compare
	struct KeyCompare
	{
		bool operator()(const std::string& a, const std::string& b) const
		{
			return !strcasecmp(a.c_str(), b.c_str());
		}
	};

	// Inifile debugger
	static Debug debug;

	// File name
	std::string path;
	
	// Hash table containing present items. The keys are strings with the
	// template "<section>" for sections and "<section>|<var>" for
	// variables. The data are NULL for sections, and the variable value for
	// variables.
	std::unordered_map<std::string, std::string, KeyHash, KeyCompare> items;

	// Hash table containing allowed items. The keys are strings
	// "<section>|<variable>".
	std::unordered_set<std::string, KeyHash, KeyCompare> allowed_items;

	// Table of enforced items. Each element is a string with template
	// "<section>|<variable>".
	std::unordered_set<std::string, KeyHash, KeyCompare> enforced_items;

	// Redundant list containing section names. This extra
	// information is added to keep track of the order in which sections
	// were loaded from a file or created by the user. Altering this order
	// when saving the file could be annoying.
	std::vector<std::string> sections;

	/// Extract the section and variable fields from an item. The item has
	/// the format "<section>|<variable>" or "<section>". In the latter case,
	/// output argument \a var is an empty string.
	static void ItemToSectionVar(const std::string &item,
			std::string &section, std::string &var);
	
	/// Create a string "<section>|<var>" from separate strings. String
	/// "<section>" is created if \a var is an empty string. Remove any
	/// spaces on the left or right of both the section and variable names.
	static std::string SectionVarToItem(const std::string &section,
			const std::string &var);

	/// Given a string in the format "<var>=<value>", return a string
	/// containing the variable and another string containing the value. If
	/// the input string format is invalid, the function returns false.
	static bool getVarValue(const std::string &s, std::string &var,
			std::string &value);

	// Parse the INI file from an input stream
	void Parse(std::istream *f);

	bool InsertSection(std::string section);

	bool InsertVariable(std::string section, std::string var,
			std::string value);
public:

	/// Create an empty INI file
	IniFile();

	/// Create an INI file and load its content from the file system
	IniFile(const std::string &path);

	/// If the content of the INI file was loaded from the file system,
	/// return the file name. Otherwise, return an empty string.
	const std::string &getPath() const { return path; }

	/// Load the INI file content from the file system
	void Load(const std::string &path);

	/// Load the INI file from string
	void LoadFromString(const std::string &string);

	/// Store the INI file into the file system
	void Save(const std::string &path) const;

	/// Dump the content of the INI file into an output stream
	void Dump(std::ostream &os = std::cout) const;

	/// Alternative syntax to dump file into output stream
	friend std::ostream &operator<<(std::ostream &os,
			const IniFile &ini_file)
	{
		ini_file.Dump(os);
		return os;
	}

	/// Return \a true if the section exists
	bool Exists(const std::string &section) const;

	/// Return \a true if a variable exists
	bool Exists(const std::string &section, const std::string &var) const;

	/// Remove a section, returning \a false if the section was not found
	bool Remove(const std::string &section);

	/// Remove a variable, returning \a false if it wasn't found
	bool Remove(const std::string &section, const std::string &var);

	/// Return the number of sections
	int getNumSections() const { return sections.size(); }

	/// Return the name of the section at position \a index. The result is
	/// undefined if \a index is an invalid position.
	const std::string &getSection(int index) const
	{
		assert(index >= 0 && index < (int) sections.size());
		return sections[index];
	}

	/// Return an iterator to the first section in the file.
	std::vector<std::string>::iterator sections_begin()
	{
		return sections.begin();
	}

	/// Return a past-the-end iterator to the list of sections in the INI
	/// file.
	std::vector<std::string>::iterator sections_end()
	{
		return sections.end();
	}
	
	/// Add a new variable to a section. If the section doesn't exist, it
	/// will be created. Sections and variables added with \a WriteXXX()
	/// calls are automatically added to the list of allowed sections and
	/// variables.
	void WriteString(const std::string &section, const std::string &var,
			const std::string &value);

	/// Write an integer variable
	void WriteInt(const std::string &section, const std::string &var,
			int value);

	/// Write a 64-bit integer variable
	void WriteInt64(const std::string &section, const std::string &var,
			long long value);

	/// Write a boolean value, stored as a string <tt>True|False</tt>
	void WriteBool(const std::string &section, const std::string &var,
			bool value);

	/// Write a variable of type \a double
	void WriteDouble(const std::string &section, const std::string &var,
			double value);

	/// Write a value that is member of the enumeration represented in \a
	/// map.
	void WriteEnum(const std::string &section, const std::string &var,
			int value, misc::StringMap &map);

	/// Write a variable of type pointer
	void WritePointer(const std::string &section, const std::string &var,
			void *value);

	/// Read a string variable from the INI file. If the variable is not
	/// found, return the default value specified in \a def.
	std::string ReadString(const std::string &section,
			const std::string &var,
			const std::string &def = "");
	
	/// Read an integer variable
	int ReadInt(const std::string &section, const std::string &var,
			int def = 0);

	/// Read a 64-bit integer variable
	long long ReadInt64(const std::string &section, const std::string &var,
			long long def = 0);

	/// Read a boolean variable. String values accepted are
	/// <tt>t|True|On|f|False|Off</tt> and all their combinations of lower
	/// and upper case
	bool ReadBool(const std::string &section, const std::string &var,
			bool def = false);

	/// Read a variable of type \a double.
	double ReadDouble(const std::string &section, const std::string &var,
			double def = 0.0);

	/// Read a string representing a constant integer value, as specified in
	/// the string \a map. If the string does not map to any value, or the
	/// variable was not present, return the default value specified in \a
	/// def.
	int ReadEnum(const std::string &section, const std::string &var,
			misc::StringMap &map, int def = 0);

	/// Read a pointer from a file, represented as an hexadecimal number,
	/// with the optional <tt>0x</tt> prefix.
	void *ReadPointer(const std::string &section, const std::string &var,
			void *def = nullptr);

	/// Allow the presence of a section in the file. A section is implicitly
	/// allowed with any call to ReadString() or similar, or any call to
	/// WriteString() or similar, using that section.
	void Allow(const std::string &section);

	/// Allow the presence of a variable in the file. A variable is
	/// implicitly allowed every time it is read or written with calls to
	/// ReadString(), WriteString(), and similar.
	void Allow(const std::string &section, const std::string &var);

	/// Enforce the presence of a section in the file. If a mandatory
	/// section is not present when the programmer invokes Check(), a fatal
	/// message is produced.
	void Enforce(const std::string &section);

	/// Enforce the presence of a variable in the file. If a mandatory
	/// variable is not present when the programmer invoked Check(), a fatal
	/// message is produced.
	void Enforce(const std::string &section, const std::string &var);

	/// Check that all enforced variables and sections are present in the
	/// file, and that all variables/sections present in the file are
	/// allowed.
	void Check() const;

	/// Check that all enforced variables are present in the given section,
	/// and that all variables present in the section are allowed.
	void Check(const std::string &section) const;

	/// Activate debug information for the Inifile parser.
	///
	/// \param path
	///	Path to dump debug information. Strings \c stdout and \c stderr
	///	are special values referring to the standard output and standard
	///	error output, respectively.
	static void setDebugPath(const std::string &path)
	{
		debug.setPath(path);
		debug.setPrefix("[IniFile]");
	}
};


}  // namespace misc

#endif

