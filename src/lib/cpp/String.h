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

#ifndef LIB_CPP_STRING_H
#define LIB_CPP_STRING_H

#include <cassert>
#include <string>
#include <vector>

#include "Misc.h"


namespace misc
{


/// Possible error codes returned by string-related functions
enum StringError
{
	StringErrorOK = 0,
	StringErrorBase,
	StringErrorFormat,
	StringErrorRange
};

/// Return a string associated with an error code of type StringError
const char *StringErrorToString(StringError error);

/// Return a string obtained by formating the input in argument \a fmt, which
/// follows the standard \c printf formatting rules. The number and type of
/// arguments following \a fmt depend on the special characters used in the
/// format string itself. This is an exception of a function that does not
/// have the \c StringXXX prefix, due to its very frequent use.
std::string fmt(const char *fmt_str, ...)
		__attribute__ ((format(printf, 1, 2)));

/// Return \c true if the character given in \a c is present in string \a set.
inline bool StringHasChar(const std::string &set, char c) {
	return set.find(c) != std::string::npos;
}

/// Remove any character in \a set on the left of string \a s. If argument \a
/// set is not given, newline, tab, and space characters are removed from the
/// left of the string.
void StringTrimLeft(std::string &s, const std::string &set = " \t\n\r");

/// Remove any character in \a set on the right of string \a s. If argument \a
/// set is not given, newline, tab, and space characters are removed from the
/// right of the string.
void StringTrimRight(std::string &s, const std::string &set = " \t\n\r");

/// Remove any character in \a set from both left and right ends of string \a s.
/// If argument \a set is not given, newline, tab, and space characters are
/// trimmed from both sides of the string.
void StringTrim(std::string &s, const std::string &set = " \t\n\r");

/// Remove consecutive occurrences of characters in \a set from string \a s and
/// replace them by one single space each time. If argument \a set is not given,
/// newline, tab, and space characters are replaced by one single space.
void StringSingleSpaces(std::string &s, const std::string &set =  " \t\n\r");

/// Convert string to lower case
void StringToLower(std::string &s);

/// Convert string to upper case
void StringToUpper(std::string &s);

/// Return \c true if string \a s starts with \a prefix.
bool StringPrefix(const std::string &s, const std::string &prefix);

/// Return \c true if string \a s ends with \a suffix.
bool StringSuffix(const std::string &s, const std::string &suffix);

/// Split a string in tokens and place them in a vector of strings passed by
/// reference in argument \a token. Optionally, argument \a set can specify the
/// characters considered as tokens separators. If \a set is not given, newline,
/// space, and tab characters are considered as token separators.
void StringTokenize(const std::string &s, std::vector<std::string> &tokens,
		const std::string &set = " \t\n\r");

/// Convert \a digit in base \a base into an integer. The digit must be a
/// character between 0-9 or a-z (lower and upper case are valid). On success,
/// the functions returns the value represented by the digit. The following
/// errors can be returned in optional argument \a error:
///
/// - \c StringErrorOK - No error.
/// - \c StringErrorBase - The base is not valid (2, 8, 10, or 16).
/// - \c StringErrorFormat - The digit is equal or greater than \a base, or the
///   digit is an invalid value.
int StringDigitToInt(char digit, int base, StringError &error);
int StringDigitToInt(char digit, int base);

/// Convert a string into an integer, accepting the following modifiers.
/// If conversion fails due to wrong formatting of the string, an error code is
/// returned in optional argument \a error.
///
/// - Possible string prefixes are:
///   - Prefix \c 0x - Use base 16 for conversion.
///   - Prefix \c 0 - Use base 8 for conversion.
/// - String suffixes:
///   - Suffix \c k - Multiply by 1024.
///   - Suffix \c K - Multiply by 1000.
///   - Suffix \c m - Multiply by 1024*1024.
///   - Suffix \c M - Multiply by 1000*1000.
///   - Suffix \c g - Multiply by 1024*1024*1024.
///   - Suffix \c G - Multiply by 1000*1000*1000.
///
/// The following errors can occur during the conversion, returned in optional
/// argument \a error:
///
/// - \c StringErrorOK - No error.
/// - \c StringErrorFormat - Invalid character found.
/// - \c StringErrorRange - The number represented in the string exceeds the
///   range of the integer format used to capture it.
int StringToInt(const std::string &s, StringError &error);
int StringToInt(const std::string &s);

/// Convert a string to a 64-bit signed integer. The same set of prefixes and
/// suffixes can be used as in StringToInt() to specify radix, sign, or factors.
long long StringToInt64(const std::string &s, StringError &error);
long long StringToInt64(const std::string &s);

/// Organize the string in \a text with a nice layout, removing redundant spaces
/// and adjusting it to a fixed text width. This function is ideal to format
/// error messages, or help messages provided to the user.
///
/// Argument \a indent specifies the number of spaces to include in the
/// beginning of each line, starting at the second line. Argument \a
/// first_indent indicates the number of spaces on the left of the first line.
/// Argument \a width specifies the maximum number of characters in one line,
/// including indentation spaces.
std::string StringParagraph(const std::string &text,
		int indent = 0, int first_indent = 0,
		int width = 80);

/// Return a string with a human-readable representation of a binary buffer.
/// \param buffer Pointer to the binary buffer.
/// \param size Number of bytes available in the buffer.
/// \param truncate Maximum number of bytes from the original buffer to dump
///	into the string, or 0 for no max. This argument is optional.
std::string StringBinaryBuffer(char *buffer, int size, int truncate = 0);


/// Class representing a string map: a set of strings mapped to integer values.
/// Typically, string maps are global variables declared as follows:
///
/// \code
///	StringMap my_map = {
///		{ "element1", 1 },
///		{ "second_element", 2 },
///		{ "last item", 3 }
///	};
/// \endcode
///
class StringMap
{
	struct Item
	{
		const char *text;
		int value;
		
		Item(const char *text, int value) :
				text(text), value(value) { }
	};

	std::vector<Item> items;
public:

	/// Constructor
	StringMap(std::initializer_list<Item> items) : items(items) {
		// Make sure that map is not null-terminated, since this used
		// to be the way string maps were declared in the past
#ifndef NDEBUG
		assert(items.size() > 0);
		if (!this->items[items.size() - 1].text)
			panic("string map %s is null-terminated. This is a\n"
					"deprecated form of declaring string\n"
					"maps. Please remove its last element.",
					toString().c_str());
#endif
	}

	/// Dump the strings in the string map, in the same format as function
	/// toString() does.
	void Dump(std::ostream &os = std::cout) const;

	/// Operator \c << invoking function Dump() on an output stream.
	friend std::ostream &operator<<(std::ostream &os, const StringMap &map) {
		map.Dump(os);
		return os;
	}

	/// Return a string with all strings present in a string map, set off
	/// in brackets and separated by commas. For example, given the
	/// following bitmap:
	///
	/// \code
	///	StringMap my_map = {
	///		{ "Fetch", 1 },
	///		{ "Decode", 2 },
	///		{ "Issue", 3 },
	///		{ "Writeback", 4 }
	///	};
	/// \endcode
	///
	/// a call to <tt>my_map.toString()</tt> returns string
	/// <tt>{Fetch,Decode,Issue,Writeback}</tt>.
	///
	std::string toString() const;

	/// Obtain the string associated with \a value in the string map. If
	/// value is not present, an empty string (<tt>""</tt>) is returned, and
	/// optional argument \a error is set to \c true. If the value is found
	/// in the string map, optional argument \a error is set to \c false.
	const char *MapValue(int value, bool &error) const;

	/// Overloaded version of MapValue() that omits the \a error argument.
	const char *MapValue(int value) const {
		bool error;
		return MapValue(value, error);
	}

	/// Obtain the string associated with \a value in the string map. If
	/// value is not present, an empty string (<tt>""</tt>) is returned, and
	/// optional argument \a error is set to \c true. If the value is found
	/// in the string map, optional argument \a error is set to \c false.
	int MapString(const std::string &s, bool &error) const;

	/// Overloaded version of MapString() that omits the \a error argument.
	int MapString(const std::string &s) const {
		bool error;
		return MapString(s, error);
	}

	/// Same as MapString(), but case insensitive.
	int MapStringCase(const std::string &s, bool &error) const;

	/// Overloaded version of MapStringCase() that omits the \a error
	/// argument.
	int MapStringCase(const std::string &s) const {
		bool error;
		return MapStringCase(s, error);
	}

	/// Construct a string containing the set of flags in bitmap \a flags,
	/// as specified in the string map. The string map is assumed to contain
	/// only integer values that are powers of 2 (only one bit is set). For
	/// example, given the following bitmap:
	///
	/// \code
	///	StringMap my_flags = {
	///		{ "Read", 1 },
	///		{ "Write", 2 },
	///		{ "Modify", 4 },
	///		{ "Execute", 8 }
	///	};
	/// \endcode
	///
	/// a call to <tt>my_flags.MapFlags(my_flags, 1|4|8)</tt> returns string
	/// <tt>{Read|Modify|Execute}</tt>.
	///
	std::string MapFlags(unsigned flags) const;
};


} // namespace misc

#endif

