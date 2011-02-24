#!/usr/bin/python

# Copyright (C) 2009 Rafael Ubal Tena
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

import sys
import re


# Global variables

ProgramName = "IniFile 1.0"
FileName = ""  # Name of the file
Modif = False  # Set to true if there was some change
SectionList = []  # Elements = [ SimpleSectionName, SectionName, KeyList ]


def Warning(msg):
	sys.stderr.write("Warning: %s\n" % msg)


def error(msg):
	sys.stderr.write("Error: %s\n" % msg)
	sys.exit(1)


def ReadFile():

	# Flush the list [ SimpleSectionName, SectionName, KeyList ]
	# into SectionList. Those unformatted lines at the end of KeyList
	# are added as sections.
	def FlushSection():
		section_index = len(SectionList)
		while len(KeyList) > 0:
			Key = KeyList[-1]
			[ SimpleKeyName, KeyName, Value ] = Key
			if SimpleKeyName != "":
				break
			Section = [ "", KeyName, [] ]
			SectionList.insert(section_index, Section)
			del(KeyList[-1])
		Section = [ SimpleSectionName, SectionName, KeyList ]
		SectionList.insert(section_index, Section)
	
	# Open file
	try:
		f = open(FileName, "r")
	except:
		return
	
	# Initialize
	global SectionList
	global SectionRe
	global KeyRe
	SimpleSectionName = ""
	SectionName = ""
	KeyList = []
	
	# Read
	for Line in f:
		
		# Remove final '\n'
		if Line != "" and Line[-1] == "\n":
			Line = Line[:-1]
		
		# New section
		SectionMatch = SectionRe.match(Line)
		if SectionMatch:
			if SectionName != "":
				FlushSection()
			SimpleSectionName = SectionMatch.group(1).strip().lower()
			SectionName = Line
			KeyList = []
			continue

		# New key
		KeyMatch = KeyRe.match(Line)
		if KeyMatch:
			SimpleKeyName = KeyMatch.group(1).strip().lower()
			KeyName = Line
			Value = KeyMatch.group(2).strip()
			Key = [ SimpleKeyName, KeyName, Value ]
			KeyList.append(Key)
			continue

		# Comment or unformatted line
		if SectionName == "":
			Section = [ "", Line, [] ]
			SectionList.append(Section)
		else:
			Key = [ "", Line, "" ]
			KeyList.append(Key)
	
	# Store last section and close file
	if SectionName != "":
		FlushSection()
	f.close()


def WriteFile():
	
	# Open file
	try:
		f = open(FileName, "w")
	except:
		error("file '%s' cannot be written" % (FileName))
	
	# Write
	for Section in SectionList:
		[ SimpleSectionName, SectionName, KeyList ] = Section
		f.write("%s\n" % (SectionName))
		for Key in KeyList:
			[ SimpleKeyName, KeyName, Value ] = Key
			f.write("%s\n" % (KeyName))
	
	# Close file
	f.close()


def DumpSection(Section):
	[ SimpleSectionName, SectionName, KeyList ] = Section
	if SimpleSectionName == "":
		print ">>> %s" % SectionName
	else:
		print "Section '%s' ('%s'):" % \
			(SectionName, SimpleSectionName)
		for Key in KeyList:
			[ SimpleKeyName, KeyName, Value ] = Key
			if SimpleKeyName == "":
				print "   >>> %s" % KeyName
			else:
				print "   Key '%s' ('%s' = '%s')" % \
					(KeyName, SimpleKeyName, Value)


# Return the index within SectionList of a given section. If it is not
# present, return -1
def FindSection(Name):
	Name = Name.strip().lower()
	index = 0
	for Section in SectionList:
		[ SimpleSectionName, SectionName, KeyList ] = Section
		if SimpleSectionName == Name:
			return index
		index = index + 1
	return -1


# Return the index within KeyList. If not present, return -1
def FindKey(KeyList, Name):
	Name = Name.strip().lower()
	index = 0
	for Key in KeyList:
		[ SimpleKeyName, KeyName, Value ] = Key
		if SimpleKeyName == Name:
			return index
		index = index + 1
	return -1


# Create a new section at the end of the list and return its index.
def CreateSection():

	# Add a blank line if necessary
	if len(SectionList) > 0:
		Section = SectionList[-1]
		[ SimpleSectionName, SectionName, KeyList ] = Section
		SectionName = SectionName.strip()
		if SectionName != "":
			Section = [ "", "", [] ]
			SectionList.append(Section)
	
	# Add section
	Section = [ "", "", [] ]
	SectionList.append(Section)
	return len(SectionList) - 1


# Create a new key in section with index 'section_index' and return its index.
# The key will be created just after the last present key. */
def CreateKey(section_index):
	
	# Find position
	Section = SectionList[section_index]
	[ SimpleSectionName, SectionName, KeyList ] = Section
	index = -1
	for index in range(len(KeyList) - 1, -1, -1):
		Key = KeyList[index]
		[ SimpleKeyName, KeyName, Value ] = Key
		if SimpleKeyName != "":
			index = index + 1
			break
	index = max(index, 0)

	# Insert and return index
	Key = [ "", "", "" ]
	KeyList.insert(index, Key)
	Section = [ SimpleSectionName, SectionName, KeyList ]
	SectionList[section_index] = Section
	return index


def ProcessCommandDump(Args):
	if len(Args) == 0:
		for Section in SectionList:
			DumpSection(Section)
	
	elif len(Args) == 1:
		index = FindSection(Args[0])
		if index < 0:
			error("section '%s' not found" % Args[0])
		DumpSection(SectionList[index])
	else:
		error("command 'dump': wrong syntax")


def ProcessCommandRead(Args):
	
	# Syntax
	if len(Args) != 2 and len(Args) != 3:
		error("command 'read': wrong syntax")
	
	# Default return value
	Def = ""
	if len(Args) == 3:
		Def = Args[2]
	
	# Find section
	index = FindSection(Args[0])
	if index < 0:
		print Def
		return
	Section = SectionList[index]
	[ SimpleSectionName, SectionName, KeyList ] = Section

	# Find key
	index = FindKey(KeyList, Args[1])
	if index < 0:
		print Def
		return
	Key = KeyList[index]
	[ SimpleKeyName, KeyName, Value ] = Key
	print Value


def ProcessCommandWrite(Args):
	
	# Syntax
	global Modif
	if len(Args) != 3:
		error("command 'write': wrong syntax")
	Args[0] = Args[0].strip()
	Args[1] = Args[1].strip()
	Args[2] = Args[2].strip()
	if len(Args[0]) == 0:
		error("section name not valid: '%s'" % (Args[0]))
	if len(Args[1]) == 0:
		error("key name not valid: '%s'" % (Args[1]))

	# Find section or create new one
	section_index = FindSection(Args[0])
	if section_index < 0:
		section_index = CreateSection()
		SimpleSectionName = Args[0].lower()
		SectionName = "[ " + Args[0] + " ]"
		KeyList = []
		Section = [ SimpleSectionName, SectionName, KeyList ]
		SectionList[section_index] = Section
	else:
		Section = SectionList[section_index]
		[ SimpleSectionName, SectionName, KeyList ] = Section
	
	# Find key or create new one
	key_index = FindKey(KeyList, Args[1])
	if key_index < 0:
		key_index = CreateKey(section_index)
		Section = SectionList[section_index]
		[ SimpleSectionName, SectionName, KeyList ] = Section
	SimpleKeyName = Args[1].lower()
	KeyName = Args[1] + " = " + Args[2]
	Value = Args[2]
	Key = [ SimpleKeyName, KeyName, Value ]
	KeyList[key_index] = Key
	Section = [ SimpleSectionName, SectionName, KeyList ]
	SectionList[section_index] = Section
	
	# File modified
	Modif = True


def ProcessCommandRemove(Args):
	
	# Syntax
	global Modif
	if len(Args) != 1 and len(Args) != 2:
		error("command 'remove': wrong syntax")
	
	# Find section
	section_index = FindSection(Args[0])
	if section_index < 0:
		return
	Section = SectionList[section_index]
	[ SimpleSectionName, SectionName, KeyList ] = Section
	
	# Remove a key
	if len(Args) == 2:
		key_index = FindKey(KeyList, Args[1])
		if key_index < 0:
			return
		del(KeyList[key_index])
		Section = [ SimpleSectionName, SectionName, KeyList ]
		SectionList[section_index] = Section
		Modif = True
		return
	
	# Remove the whole section and following blank lines
	del(SectionList[section_index])
	while section_index < len(SectionList):
		Section = SectionList[section_index]
		[ SimpleSectionName, SectionName, KeyList ] = Section
		SectionName = SectionName.strip()
		if SectionName != "":
			break
		del(SectionList[section_index])
	Modif = True


def ProcessCommandExists(Args):
	
	# Syntax
	if len(Args) != 1 and len(Args) != 2:
		error("command 'exists': wrong syntax")
	Args[0] = Args[0].strip()
	if len(Args) == 2:
		Args[1] = Args[1].strip()
	
	# Find section
	section_index = FindSection(Args[0])
	if section_index < 0:
		print 0
		return
	if len(Args) == 1:
		print 1
		return
	Section = SectionList[section_index]
	[ SimpleSectionName, SectionName, KeyList ] = Section
	
	# Find key
	key_index = FindKey(KeyList, Args[1])
	if key_index < 0:
		print 0
		return
	print 1


def ProcessCommandList(Args):
	
	# Syntax
	if len(Args) != 0 and len(Args) != 1:
		error("command 'list': wrong syntax")
	
	# List sections
	if len(Args) == 0:
		for Section in SectionList:
			[ SimpleSectionName, SectionName, KeyList ] = Section
			if SimpleSectionName != "":
				SectionMatch = SectionRe.match(SectionName)
				print SectionMatch.group(1)
		return
	
	# List keys
	section_index = FindSection(Args[0])
	if section_index < 0:
		return
	Section = SectionList[section_index]
	[ SimpleSectionName, SectionName, KeyList ] = Section
	for Key in KeyList:
		[ SimpleKeyName, KeyName, Value ] = Key
		if SimpleKeyName != "":
			KeyMatch = KeyRe.match(KeyName)
			print KeyMatch.group(1)


def ParseString(s):
	
	def FlushString(temp, end_char):
		temp = temp.strip()
		if len(temp) > 0 or end_char == "\"":
			array.append(temp)
	
	array = []
	temp = ""
	end_char = " "
	while len(s):
		c = s[0]
		s = s[1:]
		if c == end_char:
			FlushString(temp, end_char)
			temp = ""
			end_char = " "
		elif c == "\"":
			end_char = "\""
		else:
			temp = temp + c
	FlushString(temp, end_char)
	return array



def ProcessCommandRun(Args):
	
	# Syntax
	if len(Args) != 1:
		error("command 'run': wrong syntax")
	
	# Open script
	try:
		f = open(Args[0], "r")
	except:
		error("cannot open script '%s'" % (Args[0]))
	
	# Read it
	for Line in f:
		if Line != "" and Line[-1] == "\n":
			Line = Line[:-1]
		Line = Line.strip()
		Args = ParseString(Line)
		if len(Args) > 1:
			ProcessCommand(Args[0], Args[1:])
	f.close()


def ProcessCommand(Command, Args):
	Command = Command.strip().lower()

	if Command == "dump":
		ProcessCommandDump(Args)
	
	elif Command == "read":
		ProcessCommandRead(Args)
	
	elif Command == "write":
		ProcessCommandWrite(Args)
	
	elif Command == "remove":
		ProcessCommandRemove(Args)
	
	elif Command == "exists":
		ProcessCommandExists(Args)
	
	elif Command == "list":
		ProcessCommandList(Args)
	
	elif Command == "run":
		ProcessCommandRun(Args)
	
	else:
		error("invalid command: %s" % Command)



# Main program

Syntax = ProgramName + """
Edit/read configuration files with sections and keys.

Syntax: inifile <file> <cmd> [<args>]
       <file>       The configuration file.
       <cmd>        The command to perform {read|write|exists|list}.
       <args>       Arguments depending on the command.

The following is a list of possible commands with their associated options.
  
  * Command 'dump'. Arguments: [<section>]
       Dump the contents of the file or of a section if <section> is
       given.

  * Command 'read'. Arguments: <section> <key> [<def>]
       Read the key <key> at section <section>. If <def> is given and the
       section or key to read does not exist, <def> is returned as default
       value. If <key> does not exist and <def> is not given, no error is
       shown and an empty output is given.

  * Command 'write'. Arguments: <section> <key> <val>
       Write the pair '<key> = <val>' in section <section>. If the section
       does not exist, it will be created.

  * Command 'remove'. Arguments: <section> [<key>]
       Remove a whole section or just the key within a section if <key>
       is given. If the section/key is not found, no error is given.

  * Command 'exists'. Arguments: <section> [<key>]
       Check if a section (or a key if <key> is given) exists. Return 1
       in case it does, or 0 otherwise.

  * Command 'list'. Arguments: [<section>]
       If no argument is given, list all sections within the file.
       If <section> is given, list all keys within the section.

  * Command 'run'. Arguments: <script>
       Run a sequence of commands from a script.

"""

if len(sys.argv) < 3:
	sys.stderr.write(Syntax)
	sys.exit(1)

SectionRe = re.compile("^ *\[ *(.*) *\] *$")
KeyRe = re.compile("^ *(.*) *= *(.*) *$")

FileName = sys.argv[1]
Command = sys.argv[2]
Args = sys.argv[3:]

ReadFile()
ProcessCommand(Command, Args)
if Modif:
	WriteFile()

