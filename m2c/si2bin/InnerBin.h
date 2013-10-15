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

#ifndef M2C_SI2BIN_INNER_BIN_H
#define M2C_SI2BIN_INNER_BIN_H

#include <elf.h>
#include <stdio.h>
//#include <memory>
#include <vector>

#include <lib/cpp/ELFWriter>

#include <src/arch/southern-islands/asm/Binary.h>

namespace si2bin
{

/*
 * Note in the PT_NOTE segment
 */

class InnerBinNote
{
	unsigned int type;
	unsigned int size;
	void *payload;

public:
	/* Constructor */
	InnerBinNote(unsigned int type, unsigned size, void *payload);
	
	//InnerBinNoteDump();

	/* Getters */
	unsigned int GetType() { return type; }
	unsigned int GetSize() { return size; }


class InnerBinEntry
{
	/* Public fields:
	 *	- d_machine
	 * Private fields:
	 *	- d_type
	 *	- d_offset - offset for encoding data (PT_NOTE + PT_LOAD segments)
	 *	- d_size - size of encoding data (PT_NOTE + PT_LOAD segments)
	 *	- d_flags */
	SI::BinaryDictHeader header;

	/* This will form the .text section containing the final ISA. The user
	 * is responsible for dumping instructions in this buffer. The buffer is
	 * created and freed internally, however. */
	ELFWriter::Buffer text_section_buffer;  /* Public field */
	ELFWriter::Section text_section;  /* Private field */

	/* Section .data associated with this encoding dictionary entry. The
	 * user can fill it out by adding data into the buffer. */
	ELFWriter::Buffer data_section_buffer;  /* Public field */
	ELFWriter::Section data_section;  /* Private field */

	/* Symbol table associated with the encoding dictionary entry. It is
	 * initialized internally, and the user can just add new symbols
	 * using function 'elf_enc_symbol_table_add'. */
	ELFWriter::SymbolTable *symbol_table;  /* Public field */

	/* List of notes. Each element is of type 'si2bin_inner_bin_note_t'.
	 * Private field. */
	//vector<unique_ptr<InnerBinNote>> note_list;
	ELFWriter::Buffer *note_buffer;


public:
	
	/* Constructor */
	InnerBinEntry();


	/* Getters */
	SI::BinaryDictHeader *GetHeader() { return &header; }
	ELFWriter::Buffer *GetTextSectionBuffer { return &text_section_buffer; }
	ELFWriter::Buffer *GetDataSectionBuffer { return &data_section_buffer; }
	ELFWriter::SymbolTable *GetSymbolTable { return &symbol_table; };

	//InnerBinEntryAddNote(InnerBinNote *note);
};




/*
 * AMD Internal ELF
 */


class InnerBin
{
	std::string name;

	/* Program Resource */
	SI::BinaryComputePgmRsrc2 *pgm_rsrc2;

	/* RAT_OP */
	int rat_op;

	/* Number of SGPRS and VGPRS */
	int num_sgprs;
	int num_vgprs;

	//vector<unique_ptr<BinaryUserElement>> user_element_list;

	/* FloatMode */
	int FloatMode;

	/*IeeeMode */
	int IeeeMode;

	/* ELF file created internally.
	 * Private field. */
	ELFWriter::File *writer;

	//vector<unique_ptr<InnerBinEntry>> entry_list;

	
public:
	
	/* Constructor */
	InnerBin(std::string);

	/* Getters */
	int GetRatOp() { return rat_op; }
	int GetNumSgpr { return num_sgprs; }
	int GetNumVgpr { return num_vgprs; }
	int GetFloatMode { return FloatMode; }
	int GetIeeeMode { return IeeeMode; }

	/* Setters */
	void SetRatOp(int rat_op) { this->rat_op = rat_op; }
	void SetNumSgpr(int num_sgprs) { this->num_sgprs = num_sgprs; }
	void SetNumVgpr(int num_vgprs) { this->num_vgprs = num_vgprs; }
	void SetFloatMode(int FloatMode) { this->FloatMode = FloatMode; }
	void SetIeeeMode(int IeeMode) { this->IeeeMode = IeeeMode; }


	void Generate(ELFWriter::Buffer *bin_buffer);

	//void AddUserElement(SI::BinaryUserElement *user_elem, int index);
	//void AddEntry(InnerBinEntry *entry);

};



} /* namespace si2bin */ 

#endif

