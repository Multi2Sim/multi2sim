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
#include <vector>
#include <memory>

#include <lib/cpp/ELFWriter.h>

#include <src/arch/southern-islands/asm/Binary.h>

namespace si2bin
{

/* Forward Declarations */
class InnerBinEntry;
class InnerBin;


/*
 * Note in the PT_NOTE segment
 */

enum InnerBinNoteType
{
	InnerBinNoteTypeInvalid = 0,
	
	InnerBinNoteTypeProgInfo,
	InnerBinNoteTypeInputs,
	InnerBinNoteTypeOutputs,
	InnerBinNoteTypeCondOut,
	InnerBinNoteTypeFloat32Consts,
	InnerBinNoteTypeInt32Consts,
	InnerBinNoteTypeBool32Consts,
	InnerBinNoteTypeEarlyExit,
	InnerBinNoteTypeGlobalBuffers,
	InnerBinNoteTypeConstantBuffers,
	InnerBinNoteTypeInputSamplers,
	InnerBinNoteTypePersistentBuffers,
	InnerBinNoteTypeScratchBuffers,
	InnerBinNoteTypeSubConstantBuffers,
	InnerBinNoteTypeUAVMailboxSize,
	InnerBinNoteTypeUAV,
	InnerBinNoteTypeUAVOPMask
};

class InnerBinNote
{
	friend class InnerBinEntry;

	InnerBinEntry *entry;

	InnerBinNoteType type;
	unsigned int size;
	std::unique_ptr<void> payload;
	
	/* Constructor */
	InnerBinNote(InnerBinEntry *entry, InnerBinNoteType type, unsigned int size, 
			void *payload);

public:
		
	//InnerBinNoteDump();

	/* Getters */
	unsigned int getType() { return type; }
	unsigned int getSize() { return size; }
	void *getPayload() { return payload.get(); }
};

class InnerBinEntry
{
	
	friend class InnerBin;

	InnerBin *bin;
	
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
	ELFWriter::Buffer *text_section_buffer;  /* Public field */
	ELFWriter::Section *text_section;  /* Private field */

	/* Section .data associated with this encoding dictionary entry. The
	 * user can fill it out by adding data into the buffer. */
	ELFWriter::Buffer *data_section_buffer;  /* Public field */
	ELFWriter::Section *data_section;  /* Private field */

	/* Symbol table associated with the encoding dictionary entry. It is
	 * initialized internally, and the user can just add new symbols
	 * using function 'elf_enc_symbol_table_add'. */
	ELFWriter::SymbolTable *symbol_table;  /* Public field */

	/* List of notes. Each element is of type 'si2bin_inner_bin_note_t'.
	 * Private field. */
	std::list<std::unique_ptr<InnerBinNote>> note_list;
	ELFWriter::Buffer *note_buffer;


public:
	
	/* Constructor */
	InnerBinEntry(InnerBin *bin);


	/* Getters */
	SI::BinaryDictHeader *getHeader() { return &header; }
	ELFWriter::Buffer *getTextSectionBuffer() { return text_section_buffer; }
	ELFWriter::Buffer *getDataSectionBuffer() { return data_section_buffer; }
	ELFWriter::SymbolTable *getSymbolTable() { return symbol_table; };
	unsigned int getSize() { return header.d_size; }

	void setSize(unsigned int size) { header.d_size = size; }
	void setOffset(unsigned int offset) { header.d_offset = offset; }
	void setType(unsigned int type) { header.d_type = type; }
	void setMachine(unsigned int machine) { header.d_machine = machine; }

	void newNote(InnerBinNoteType type, unsigned int size, void *payload);
};




/*
 * AMD Internal ELF
 */


class InnerBin
{
	friend class OuterBin;

	std::string name;

	/* Program Resource */
	SI::BinaryComputePgmRsrc2 pgm_rsrc2;

	/* Number of SGPRS and VGPRS */
	int num_sgprs;
	int num_vgprs;

	std::vector<std::unique_ptr<SI::BinaryUserElement>> user_element_list;

	/* FloatMode */
	int FloatMode;

	/*IeeeMode */
	int IeeeMode;


	std::vector<std::unique_ptr<InnerBinEntry>> entry_list;

	/* Constructor */
	InnerBin(const std::string &name);


public:
	
	/* Make ELF Writer Public so New Buffer, Section, and Segments
	 * easily be made
	 */
	ELFWriter::File writer;
	
	/* Getters */
	std::string getName() { return name; }
	SI::BinaryComputePgmRsrc2 *getPgmRsrc2() { return &pgm_rsrc2; }
	int getNumSgpr() { return num_sgprs; }
	int getNumVgpr() { return num_vgprs; }
	int getFloatMode() { return FloatMode; }
	int getIeeeMode() { return IeeeMode; }
	InnerBinEntry *getEntry(unsigned int index) { return index < entry_list.size() ?
			entry_list[index].get() : nullptr; }
	SI::BinaryUserElement *getUserElement(unsigned int index) 
			{ return index < user_element_list.size() ? 
			user_element_list[index].get() : nullptr; }
	unsigned int getUserElementCount() { return user_element_list.size(); }
	
	/* Setters */
	void setPgmRsrc2(SI::BinaryComputePgmRsrc2 &pgm_rsrc2) { this->pgm_rsrc2 = pgm_rsrc2; }
	void setNumSgpr(int num_sgprs) { this->num_sgprs = num_sgprs; }
	void setNumVgpr(int num_vgprs) { this->num_vgprs = num_vgprs; }
	void setFloatMode(int FloatMode) { this->FloatMode = FloatMode; }
	void setIeeeMode(int IeeMode) { this->IeeeMode = IeeeMode; }


	void Generate(std::ostream& os);

	SI::BinaryUserElement *newUserElement(unsigned int index, unsigned int dataClass, 
			unsigned int apiSlot, unsigned int startUserReg, 
			unsigned int userRegCount);
	InnerBinEntry *newEntry();

};



} /* namespace si2bin */ 

#endif

