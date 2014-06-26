/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#ifndef HSA_ASM_BRIGENTRY_H
#define HSA_ASM_BRIGENTRY_H

#include <cstring>
#include <stdexcept>

#include <lib/cpp/Misc.h>
#include <lib/cpp/String.h>

#include "BrigFile.h"
#include "BrigDef.h"
#include "SymbolModifier.h"

namespace HSA
{
	
/// Pure virtual class representing a brig entry
class BrigEntry
{

protected:

	// Pointer to the first byte of the entry
	char *base;

	// Brig file that this entry belongs to 
	BrigFile *file;

	// Maps type to type strings
	static misc::StringMap type_to_str_map;

	// Convert profile to string to print out
	static const char *profile2str(int profile);
	
	// Convert profile to string
	static const char *machineModel2str(int machineModel);

	// Returns the align string
	static const char *align2str(unsigned char align);

	// Returns segments as string 
	// Segments are attributes such as global, readonly...
	static const char *seg2str(unsigned char seg);

	// Maps memory semantic to string
	static misc::StringMap sem_to_str_map;

	// Convert memory semantic 
	const char *sem2str(unsigned char modifier) const;

	// Dump value list
	static void dumpValueList(BrigDataOffset32_t data,
			BrigType16_t type, 
			uint32_t elementCount,
			BrigFile *file,
			std::ostream &os);

	// Dump Symbol Declaration
	static void dumpSymDecl(const BrigEntry *dir, std::ostream &os);
	
	// Dump Argument list
	// \return
	// 	pointer to the next directive to be processed
	static char *dumpArgs(char *arg, 
			unsigned short argCount,
			BrigFile *file,
			std::ostream &os);

	// Dump _str if str is not empty, otherwise do nothing
	static void dumpUnderscore(const std::string &str,
			std::ostream &os = std::cout)
	{
		if(str.length())
		{
			os << "_" << str;
		}
	}
	
	// Dumps a numberic value
	void dumpValue(char *value) const;
	
	// Dump the function or kernel body
	void dumpBody(int codeOffset,
			int nInst,
			char *next,
			bool isDecl, 
			std::ostream &os) const;

	// Dump nested directives inside a class. It will dump all the directive
	// should be dumped before the current code offset
	// \param dirPtr
	//	Pointer to the directive to be dumped next
	// \return
	// 	Pointer to the next directive to be dumped
	char *DumpRelatedDirectives(char *dirPtr,
			unsigned int offset, std::ostream &os) const;

	// Dump the tab for indentation
	static void dumpIndent(std::ostream &os);
	
public:

	/// Constructor
	BrigEntry(char *buf, BrigFile *file);

	/// Returns the pointer to the buffer of the entry
	char *getBuffer() const{return base;}

	/// Returns the pointer to the brig file this entry belongs to
	BrigFile *getFile() const{return file;}

	/// Converts type to type string
	static const char *type2str(int type);

	/// Returns the size of the entry, the size is paddint into 4 bytes
	/// String entry should have different implementation, since the size
	/// field in string entry is 4 bytes long
	virtual unsigned int getSize() const;

	/// Dump the entry in HEX
	virtual void dumpHex(std::ostream &os) const;

	/// Virtual function for dump assembly
	virtual void Dump(std::ostream &os) const;

	/// Returns the pointer to the next entry
	virtual char *next() const{return this->base + this->getSize();}

};

}  // namespace HSA

#endif
