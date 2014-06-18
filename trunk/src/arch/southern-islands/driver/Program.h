
/*
 *  Multi2Sim
 *  Copyright (C) 2014  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef ARCH_SI_PROGRAM_H
#define ARCH_SI_PROGRAM_H

#include <lib/cpp/ELFReader.h>

#include "Driver.h"

namespace SI
{

/// Program Class
class Program
{
	int id;

	// ELF binary
	std::unique_ptr<ELFReader::File> elf_file;

public:
	Program(int id);

	/// Load ELF binary into program object
	///
	/// \param buf
	/// Buffer containing OpenCL ELF program binary
	/// \param size
	///	Size of buffer
	void SetBinary(const char *buf, unsigned int size);

	/// Get the symbol in the Program ELF file by symbol name
	///
	/// \param name
	///	Name of the symbol
	ELFReader::Symbol *getSymbol(const std::string &name) const {
		return elf_file->getSymbol(name); };

	/// Get the id of the program
	int getId() const { return id; }
};


} // namespace SI

#endif /* ARCH_SI_PROGRAM_H */
