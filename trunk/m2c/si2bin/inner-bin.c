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

#include <elf.h>

#include <arch/southern-islands/asm/bin-file.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-encode.h>
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "inner-bin.h"



/*
 * Note Object
 */

struct si2bin_inner_bin_note_t *si2bin_inner_bin_note_create(unsigned int type,
		unsigned int size, void *payload)
{
	struct si2bin_inner_bin_note_t *note;

	/* Initialize */
	note = xcalloc(1, sizeof(struct si2bin_inner_bin_note_t));
	note->type = type;
	note->size = size;

	/* Copy payload */
	if (size)
	{
		note->payload = xcalloc(1, size);
		memcpy(note->payload, payload, size);
	}

	/* Return */
	return note;
}


void si2bin_inner_bin_note_free(struct si2bin_inner_bin_note_t *note)
{
	if (note->size)
		free(note->payload);
	free(note);
}


void si2bin_inner_bin_note_dump(struct elf_enc_buffer_t *buffer, FILE *fp)
{
	int offset;
	int descsz;
	int n;

	char *note_type_str;

	struct elf_enc_buffer_t *payload;

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
		
		note_type_str = str_map_value(&pt_note_type_map, *((int *)(buffer->ptr + offset)));
		fprintf(fp, "\n Type: %s", note_type_str);
		offset += 4;

		fprintf(fp, "\n Name: %s\n", (char *)(buffer->ptr + offset));
		offset += 8;

		payload = elf_enc_buffer_create();
		elf_enc_buffer_write(payload, buffer->ptr + offset, descsz);
		elf_enc_buffer_dump(payload, fp);
		fprintf(fp, "\n\n");
		elf_enc_buffer_free(payload);
		
		offset += descsz;

		n++;
	}

}




/*
 * Object representing an Encoding Dictionary Entry
 */


struct si2bin_inner_bin_entry_t *si2bin_inner_bin_entry_create(void)
{
	struct si2bin_inner_bin_entry_t *entry;

	/* Initialize */
	entry = xcalloc(1, sizeof(struct si2bin_inner_bin_entry_t));

	/* Text Section Initialization */
	entry->text_section_buffer = elf_enc_buffer_create();
	entry->text_section = elf_enc_section_create(".text", entry->text_section_buffer, 
			entry->text_section_buffer);
	entry->text_section->header.sh_type = SHT_PROGBITS;

	/* Data Section Initialization */
	entry->data_section_buffer = elf_enc_buffer_create();
	entry->data_section = elf_enc_section_create(".data", entry->data_section_buffer, 
			entry->data_section_buffer);
	entry->data_section->header.sh_type = SHT_PROGBITS;
	
	/* Symbol Table Initialization */
	entry->symbol_table = elf_enc_symbol_table_create(".symtab", ".strtab");

	/* Make note list and buffer for note segment */
	entry->note_list = list_create();
	entry->note_buffer = elf_enc_buffer_create();

	
	/* Return */
	return entry;
}


void si2bin_inner_bin_entry_free(struct si2bin_inner_bin_entry_t *entry)
{
	int i;

	/* Free list element and list */
	LIST_FOR_EACH(entry->note_list, i)
		si2bin_inner_bin_note_free(list_get(entry->note_list, i));
	list_free(entry->note_list);

	/* Free bin entry */
	free(entry);
}


void si2bin_inner_bin_entry_add_note(struct si2bin_inner_bin_entry_t *entry,
		struct si2bin_inner_bin_note_t *note)
{
	list_add(entry->note_list, note);
}




/*
 * AMD Internal Binary Object
 */


struct si2bin_inner_bin_t *si2bin_inner_bin_create(char *name)
{
	struct si2bin_inner_bin_t *bin;
	struct elf_enc_buffer_t *buffer;
	struct elf_enc_segment_t *segment;

	/* Initialize */
	bin = xcalloc(1, sizeof(struct si2bin_inner_bin_t));
	bin->file = elf_enc_file_create();
	bin->entry_list = list_create();
	
	/* Create buffer and segment for encoding dictionary */
	buffer = elf_enc_buffer_create();
	elf_enc_file_add_buffer(bin->file, buffer);
	
	segment = elf_enc_segment_create("Encoding Dictionary", buffer, buffer);
	elf_enc_file_add_segment(bin->file, segment);

	segment->header.p_type = PT_LOPROC + 2;

	/* Save kernel name */
	bin->name = xstrdup(name);

	/* Set up user element list and program resource */
	bin->pgm_rsrc2 = si_bin_compute_pgm_rsrc2_create();
	bin->user_element_list = list_create();

	/* Return */
	return bin;
}


void si2bin_inner_bin_free(struct si2bin_inner_bin_t *bin)
{
	struct si_bin_enc_user_element_t *user_elem;
	int i;

	/* Free list elements and list */
	LIST_FOR_EACH(bin->entry_list, i)
		si2bin_inner_bin_entry_free(list_get(bin->entry_list ,i));
	list_free(bin->entry_list);

	/* Free elf_enc_file */
	elf_enc_file_free(bin->file);

	/* Free kernel name */
	free(bin->name);

	/* Free User Element List*/
	LIST_FOR_EACH(bin->user_element_list, i)
	{
		user_elem = list_get(bin->user_element_list, i);
		si_bin_enc_user_element_free(user_elem);
	}
	list_free(bin->user_element_list);

	/* Free Program Resource */
	si_bin_compute_pgm_rsrc2_free(bin->pgm_rsrc2);

	/* Free si2bin_inner_bin */
	free(bin);
}

void si2bin_inner_bin_add_user_element(struct si2bin_inner_bin_t *bin, 
		struct si_bin_enc_user_element_t *user_elem, int index)
{
	int count;
	
	
	count = list_count(bin->user_element_list);

	if (count == index)
	{
		list_add(bin->user_element_list, user_elem);
	}
	else if (count > index)
	{
		if (list_get(bin->user_element_list, index))
			fatal("userElement[%d] defined twice", index);

		list_remove_at(bin->user_element_list, index);
		list_insert(bin->user_element_list, index, user_elem);
	}
	else if (count < index)
	{
		while (count < index)
		{
			list_add(bin->user_element_list, NULL);
			count = list_count(bin->user_element_list);
		}

		list_add(bin->user_element_list, user_elem);
	}

}

void si2bin_inner_bin_add_entry(struct si2bin_inner_bin_t *bin,
		struct si2bin_inner_bin_entry_t *entry)
{
	struct elf_enc_segment_t *note_segment;
	struct elf_enc_segment_t *load_segment;
	
	/* Add entry */
	list_add(bin->entry_list, entry);
	
	/* Add note_buffer and create note segment */
	elf_enc_file_add_buffer(bin->file, entry->note_buffer);

	note_segment = elf_enc_segment_create("Note Segment", entry->note_buffer,
			entry->note_buffer);
	elf_enc_file_add_segment(bin->file, note_segment);

	note_segment->header.p_type = PT_NOTE;

	/* Add text section and text buffer section to elf_enc_file */
	elf_enc_file_add_buffer(bin->file, entry->text_section_buffer);
	elf_enc_file_add_section(bin->file, entry->text_section);

	/* Add data section and data buffer section to elf_enc_file */
	elf_enc_file_add_buffer(bin->file, entry->data_section_buffer);
	elf_enc_file_add_section(bin->file, entry->data_section);
	
	/* Add symbol table section and text buffer section to elf_enc_file */
	elf_enc_file_add_symbol_table(bin->file, entry->symbol_table);

	/* Create load segment */
	load_segment = elf_enc_segment_create("Load Segment", entry->text_section_buffer,
			entry->symbol_table->string_table_buffer);
	elf_enc_file_add_segment(bin->file, load_segment);

	load_segment->header.p_type = PT_LOAD;


}


void si2bin_inner_bin_generate(struct si2bin_inner_bin_t *bin, struct elf_enc_buffer_t *bin_buffer)
{
	int i;
	int namesz;
	int start;
	int end;
	int buf_offset;
	int phtab_size;
	char *name;
	struct si2bin_inner_bin_note_t *note;
	struct si2bin_inner_bin_entry_t *entry;
	struct elf_enc_buffer_t *enc_dict;
	struct elf_enc_buffer_t *buffer;


	namesz = 8;
	name = "ATI CAL";

	enc_dict = list_get(bin->file->buffer_list, 2);

	phtab_size = sizeof(Elf32_Phdr) * list_count(bin->file->segment_list);

	LIST_FOR_EACH(bin->entry_list, i)
	{
		entry = list_get(bin->entry_list, i);

		LIST_FOR_EACH(entry->note_list, i)
		{	

			/* Write name, size, type, etc. to buffer */
			note = list_get(entry->note_list, i);
			elf_enc_buffer_write(entry->note_buffer, &namesz, 4);
			elf_enc_buffer_write(entry->note_buffer, &note->size, 4);
			elf_enc_buffer_write(entry->note_buffer, &note->type, 4);
			elf_enc_buffer_write(entry->note_buffer, name, 8);
			elf_enc_buffer_write(entry->note_buffer, note->payload,
					note->size);
		}

		start = entry->note_buffer->index;
		end = entry->symbol_table->string_table_buffer->index;
		
		/* Calculate offset and type for enc_dict */
		for(i = start; i <= end; i++)
		{
			buffer = list_get(bin->file->buffer_list, i);
			entry->header.d_size += buffer->size;
		}
		
		buf_offset = 0;
		
		for(i = 0; i < start; i++)
		{	
			buffer = list_get(bin->file->buffer_list, i);
			buf_offset += buffer->size;
		}

		entry->header.d_offset = sizeof(Elf32_Ehdr) + phtab_size +
			sizeof(struct si2bin_inner_bin_entry_header_t) *
			list_count(bin->entry_list) + buf_offset;


		/* Write information to enc_dict */
		elf_enc_buffer_write(enc_dict, &entry->header,
				sizeof(struct si2bin_inner_bin_entry_header_t));

	}

	

	
	/* Write elf_enc_file to buffer */
	elf_enc_file_generate(bin->file, bin_buffer);
	
}

