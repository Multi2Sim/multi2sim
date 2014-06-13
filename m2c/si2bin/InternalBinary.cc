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


#include <memory>

#include <arch/southern-islands/asm/Binary.h>
#include <lib/cpp/ELFWriter.h>
#include <lib/cpp/Misc.h>

#include "InternalBinary.h"

namespace si2bin
{

/*
 * Note Object
 */

InnerBinNote::InnerBinNote(InnerBinEntry *entry, InnerBinNoteType type, 
		unsigned int size, void *payload)
{
	this->entry = entry;
	this->type = type;
	this->size = size;
	this->payload = std::unique_ptr<void>(payload);
}

/*void si2bin_inner_bin_note_dump(ELFWriterBuffer *buffer, FILE *fp)
{
	int offset;
	int descsz;
	int n;

	const char *note_type_str;

	ELFWriterBuffer *payload;

	descsz = 0;
	offset = 0;
	n = 0;

	while(offset < buffer->size)
	{
		fprintf(fp, "Note %d", n);
		fprintf(fp, "\n Name Size: %d", *((int *)(buffer->ptr + offset)));
		offset += 4;

		fprintf(fp, "\n Description Size: %d", *((int *)(buffer->ptr + offset)));
		descsz = *((int *)(buffer->ptr + offset));
		offset += 4;
		
		note_type_str = StringMapValueWrap(si_binary_note_map,
				*((int *)(buffer->ptr + offset)));
		fprintf(fp, "\n Type: %s", note_type_str);
		offset += 4;

		fprintf(fp, "\n Name: %s\n", (char *)(buffer->ptr + offset));
		offset += 8;

		payload = new(ELFWriterBuffer);
		ELFWriterBufferWrite(payload, buffer->ptr + offset, descsz);
		ELFWriterBufferDump(asObject(payload), fp);
		fprintf(fp, "\n\n");
		delete(payload);
		
		offset += descsz;

		n++;
	}

}
*/


/*
 * Object representing an Encoding Dictionary Entry
 */

InnerBinEntry::InnerBinEntry(InnerBin *bin)
{
	ELFWriter::Segment *note_segment;
	ELFWriter::Segment *load_segment;

	this->bin = bin;
	
	/* Make note list buffer for note segment */
	note_buffer = this->bin->writer.newBuffer();
	
	/* Crete Note and Load Segments */
	note_segment = this->bin->writer.newSegment("Note Segment", note_buffer,
			note_buffer);
	note_segment->setType(PT_NOTE);

	/* Text Section Initialization */
	text_section_buffer = this->bin->writer.newBuffer();
	text_section = this->bin->writer.newSection(".text", text_section_buffer, 
			text_section_buffer);
	text_section->setType(SHT_PROGBITS);

	/* Data Section Initialization */
	data_section_buffer = this->bin->writer.newBuffer();
	data_section = this->bin->writer.newSection(".data", data_section_buffer, 
			data_section_buffer);
	data_section->setType(SHT_PROGBITS);
	
	/* Symbol Table Initialization */
	symbol_table = this->bin->writer.newSymbolTable(".symtab", ".strtab");

		
	load_segment = this->bin->writer.newSegment("Load Segment", text_section_buffer,
			symbol_table->getStringTableBuffer());
	load_segment->setType(PT_LOAD);

	
}

void InnerBinEntry::newNote(InnerBinNoteType type, unsigned int size, 
		void *payload)
{
	note_list.push_back(std::unique_ptr<InnerBinNote>(new InnerBinNote(this, 
			type, size, payload)));

}




/*
 * AMD Internal Binary Object
 */


InnerBin::InnerBin(const std::string &name)
{
	ELFWriter::Buffer *buffer;
	ELFWriter::Segment *segment;

	this->name = name;

	/* Create buffer and segment for encoding dictionary */
	buffer = writer.newBuffer();
	
	segment = writer.newSegment("Encoding Dictionary", buffer, buffer);

	segment->setType(PT_LOPROC + 2);

}

SI::BinaryUserElement *InnerBin::newUserElement(unsigned int index, unsigned int dataClass, unsigned int apiSlot,
		unsigned int startUserReg, unsigned int userRegCount)
{
	unsigned int count = user_element_list.size();
	SI::BinaryUserElement *user_elem = new SI::BinaryUserElement();
	user_elem->dataClass = dataClass;
	user_elem->apiSlot = apiSlot;
	user_elem->startUserReg = startUserReg;
	user_elem->userRegCount = userRegCount;

	if (count == index)
	{
		user_element_list.push_back(
				std::unique_ptr<SI::BinaryUserElement>(user_elem));
	}
	else if (count > index)
	{
		if (user_element_list.at(index).get())
		{
			delete user_elem;
			misc::fatal("userElement[%d] defined twice", index);
		}
		
		user_element_list.at(index) = 
				std::unique_ptr<SI::BinaryUserElement>(user_elem);
	}
	else if (count < index)
	{
		while (count < index)
		{
			user_element_list.emplace_back(nullptr);
			count = user_element_list.size();
		}

		user_element_list.at(index) =
				std::unique_ptr<SI::BinaryUserElement>(user_elem);
	}

	return user_element_list.back().get();

}

InnerBinEntry *InnerBin::newEntry()
{
	entry_list.push_back(std::unique_ptr<InnerBinEntry>(new InnerBinEntry(this)));
	return entry_list.back().get();
}

void InnerBin::Generate(std::ostream& os)
{
	int i;
	int namesz;
	int start;
	int end;
	int buf_offset;
	int phtab_size;
	unsigned int tmp;

	std::string name;

	ELFWriter::Buffer *enc_dict;
	ELFWriter::Buffer *buffer;


	namesz = 8;
	name = "ATI CAL";

	enc_dict = writer.getBuffer(2);

	phtab_size = sizeof(Elf32_Phdr) * writer.getSegmentCount();

	for (auto &entry : entry_list)
	{
		for (auto &note : entry->note_list)
		{	
			/* Write name, size, type, etc. to buffer */
			entry->note_buffer->Write((char*)(&namesz), 4);
			tmp = note->getSize();
			entry->note_buffer->Write((char*)(&tmp), 4);
			tmp = note->getType();
			entry->note_buffer->Write((char*)(&tmp), 4);
			entry->note_buffer->Write(name.c_str(), 8);
			entry->note_buffer->Write((char*)(note->getPayload()), 
					note->getSize());
		}

		start = entry->note_buffer->getIndex();
		end = entry->symbol_table->getStringTableBuffer()->getIndex();
		
		entry->getSymbolTable()->Generate();

		/* Calculate offset and type for enc_dict */
		for(i = start; i <= end; i++)
		{
			buffer = writer.getBuffer(i);
			entry->setSize(entry->getSize() + buffer->getSize());
		}
		
		buf_offset = 0;
		
		for(i = 0; i < start; i++)
		{	
			buffer = writer.getBuffer(i);
			buf_offset += buffer->getSize();
		}

		entry->setOffset(sizeof(Elf32_Ehdr) + phtab_size +
			sizeof(SI::BinaryDictHeader) * entry_list.size()
			+ buf_offset);


		/* Write information to enc_dict */
		enc_dict->Write((char*)(entry->getHeader()),
				sizeof(SI::BinaryDictHeader));

	}

	
	/* Write elf_enc_file to buffer */
	writer.Generate(os);
	
}

} /* namespace si2bin */

