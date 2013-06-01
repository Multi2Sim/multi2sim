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

	/* Free buffer */
	//elf_enc_buffer_free(entry->text_section);

	/* Free bin entry */
	free(entry);
}


void si2bin_inner_bin_entry_add_note(struct si2bin_inner_bin_entry_t *entry,
		struct si2bin_inner_bin_note_t *note)
{
	list_add(entry->note_list, note);
}


/*
 * Constant Buffer
 */

struct si2bin_inner_bin_constant_buffer_t *si2bin_inner_bin_constant_buffer_create(int buff_num)
{
	struct si2bin_inner_bin_constant_buffer_t *cb;

	/* Initialize */
	cb = xcalloc(1, sizeof(struct si2bin_inner_bin_constant_buffer_t));
	
	cb->buffer_number = buff_num;

	/* Return */
	return cb;
}

void si2bin_inner_bin_constant_buffer_free(
		struct si2bin_inner_bin_constant_buffer_t *cb)
{
	free(cb);
}


/*
 * UAV Table Pointer
 */

struct si2bin_inner_bin_uav_table_pointer_t *si2bin_inner_bin_uav_table_pointer_create()
{
 	struct si2bin_inner_bin_uav_table_pointer_t *uav_ptr;

	/* Initialize */
	uav_ptr = xcalloc(1, sizeof(struct si2bin_inner_bin_uav_table_pointer_t));

	/* Return */
	return uav_ptr;
}

void si2bin_inner_bin_uav_table_pointer_free(
		struct si2bin_inner_bin_uav_table_pointer_t * uav_ptr)
{
	free(uav_ptr);
}


/*
 * AMD Internal Binary Object
 */


struct si2bin_inner_bin_t *si2bin_inner_bin_create(char *name)
{
	struct si2bin_inner_bin_t *bin;
	struct elf_enc_buffer_t *buffer;
	struct elf_enc_segment_t *segment;
	struct si2bin_inner_bin_constant_buffer_t *cb;

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

	/* Set up constant buffer list and uav ptr */
	bin->cb_list = list_create();
	cb = si2bin_inner_bin_constant_buffer_create(0);
	list_add(bin->cb_list, cb);

	cb = si2bin_inner_bin_constant_buffer_create(1);
	list_add(bin->cb_list, cb);
	
	cb = si2bin_inner_bin_constant_buffer_create(2);
	list_add(bin->cb_list, cb);
	
	bin->uav_ptr = si2bin_inner_bin_uav_table_pointer_create();


	/* Return */
	return bin;
}


void si2bin_inner_bin_free(struct si2bin_inner_bin_t *bin)
{
	struct si2bin_inner_bin_constant_buffer_t *cb;
	int i;

	/* Free list elements and list */
	LIST_FOR_EACH(bin->entry_list, i)
		si2bin_inner_bin_entry_free(list_get(bin->entry_list ,i));
	list_free(bin->entry_list);

	/* Free elf_enc_file */
	elf_enc_file_free(bin->file);

	/* Free kernel name */
	free(bin->name);

	/* Free Constant Buffer List*/
	LIST_FOR_EACH(bin->cb_list, i)
	{
		cb = list_get(bin->cb_list, i);
		si2bin_inner_bin_constant_buffer_free(cb);
	}
	list_free(bin->cb_list);

	/* Free UAV Table Pointer */
	si2bin_inner_bin_uav_table_pointer_free(bin->uav_ptr);

	/* Free si2bin_inner_bin */
	free(bin);
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


void si2bin_inner_bin_create_file(struct elf_enc_buffer_t *text_section_buffer, 
	struct elf_enc_buffer_t *bin_buffer)
{
        char *ptr;
	
	struct si2bin_inner_bin_t *bin;
        struct si2bin_inner_bin_entry_t *entry;
        struct si2bin_inner_bin_note_t *note;
        
	struct elf_enc_symbol_t *uav11;
        struct elf_enc_symbol_t *uav10;
        struct elf_enc_symbol_t *uav12;
        struct elf_enc_symbol_t *cb1;
	struct elf_enc_symbol_t *cb0;

        struct elf_enc_symbol_table_t *symbol_table;
        
	struct elf_enc_symbol_t *metadata;
        struct elf_enc_symbol_t *kernel;
	struct elf_enc_symbol_t *header;
        
	struct elf_enc_buffer_t *kernel_buffer;
        struct elf_enc_buffer_t *rodata_buffer;

        struct elf_enc_section_t *text_section;
        struct elf_enc_section_t *rodata_section;

        struct elf_enc_file_t *file;

	FILE *f;
	

	bin = si2bin_inner_bin_create("kernel");

        bin->file->header.e_machine = 0x7d;
        bin->file->header.e_version = 1;
        bin->file->header.e_ident[EI_OSABI] = 0x64;
        bin->file->header.e_ident[EI_ABIVERSION] = 1;

        entry = si2bin_inner_bin_entry_create();


        entry->header.d_machine = 26;
        entry->header.d_type = 0; /* ???? */

        uav11 = elf_enc_symbol_create("uav11");
        uav11->symbol.st_shndx = 16;
        elf_enc_symbol_table_add(entry->symbol_table, uav11);

        uav10 = elf_enc_symbol_create("uav10");
        uav10->symbol.st_value = 1;
        uav10->symbol.st_shndx = 16;
        elf_enc_symbol_table_add(entry->symbol_table, uav10);

        uav12 = elf_enc_symbol_create("uav12");
        uav12->symbol.st_value = 2;
        uav12->symbol.st_shndx = 16;
        elf_enc_symbol_table_add(entry->symbol_table, uav12);

        cb1 = elf_enc_symbol_create("cb1");
        cb1->symbol.st_shndx = 10;
        elf_enc_symbol_table_add(entry->symbol_table, cb1);
	
        cb0 = elf_enc_symbol_create("cb0");
        cb0->symbol.st_value = 1;
        cb0->symbol.st_shndx = 10;
        elf_enc_symbol_table_add(entry->symbol_table, cb0);


        elf_enc_buffer_write(entry->text_section_buffer, text_section_buffer->ptr, text_section_buffer->offset);


        ptr = xcalloc(1, 4736);
        elf_enc_buffer_write(entry->data_section_buffer, ptr, 4736);
        free(ptr);


        /* Create Notes */
        ptr = 0;

        /* ELF_NOTE_ATI_INPUTS */
        note = si2bin_inner_bin_note_create(2, 0, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);

        /*ELF_NOTE_ATI_OUTPUTS */
        note = si2bin_inner_bin_note_create(3, 0, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);

        /*ELF_NOTE_ATI_UAV */
        ptr = xcalloc(1, 48);

        ptr[0] = 0xb;
        ptr[16] = 0xa;
        ptr[32] = 0xc;

        ptr[4] = 4;
        ptr[20] = 4;
        ptr[36] = 4;

        ptr[12] = 5;
        ptr[28] = 5;
        ptr[44] = 5;

        note = si2bin_inner_bin_note_create(16, 48, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);
	free(ptr);

	/* ELF_NOTE_ATI_CONDOUT */
        ptr = xcalloc(1, 4);
        note = si2bin_inner_bin_note_create(4, 4, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);
        free(ptr);

        /* ELF_NOTE_ATI_FLOAT32CONSTS */
        note = si2bin_inner_bin_note_create(5, 0, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);

        /* ELF_NOTE_ATI_INT32CONSTS */
        note = si2bin_inner_bin_note_create(6, 0, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);

        /* ELF_NOTE_ATI_BOOL32CONSTS */
        note = si2bin_inner_bin_note_create(7, 0, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);

        /* ELF_NOTE_ATI_EARLYEXIT */
        ptr = xcalloc(1, 4);
        note = si2bin_inner_bin_note_create(8, 4, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);
        free(ptr);

        /* ELF_NOTE_ATI_GLOBAL_BUFFERS */
        note = si2bin_inner_bin_note_create(9, 0, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);

        /* ELF_NOTE_ATI_CONSTANT_BUFFERS */

        ptr = xcalloc(1, 16);

        ptr[0] = 1;
        ptr[4] = 3;
        ptr[12] = 0xf;

        note = si2bin_inner_bin_note_create(10, 16, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);

        free(ptr);

        /* ELF_NOTE_ATI_INPUT_SAMPLERS */
        note = si2bin_inner_bin_note_create(11, 0, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);

	/* ELF_NOTE_ATI_SCRATCH_BUFFERS */
        ptr = xcalloc(1, 4);
        note = si2bin_inner_bin_note_create(13, 4, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);
        free(ptr);

        /* ELF_NOTE_ATI_PERSISTENT_BUFFERS */
        note = si2bin_inner_bin_note_create(12, 0, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);

        /* ELF_NOTE_ATI_PROGINFO */
        ptr = xcalloc(1, 912);
        note = si2bin_inner_bin_note_create(1, 912, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);
        free(ptr);

        /* ELF_NOTE_ATI_SUB_CONSTANT_BUFFERS */
        note = si2bin_inner_bin_note_create(14, 0, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);

        /* ELF_NOTE_ATI_UAV_MAILBOX_SIZE */
        ptr = xcalloc(1, 4);
        note = si2bin_inner_bin_note_create(15, 4, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);
        free(ptr);

        /* ELF_NOTE_ATI_UAV_OP_MASK */
        ptr = xcalloc(1, 128);

        ptr[1] = 0x1c;

        note = si2bin_inner_bin_note_create(17, 4, ptr);
        si2bin_inner_bin_entry_add_note(entry, note);
        free(ptr);



        si2bin_inner_bin_add_entry(bin, entry);

        kernel_buffer = elf_enc_buffer_create();

        si2bin_inner_bin_generate(bin, kernel_buffer);

	f = file_open_for_write("kernel");
	elf_enc_buffer_write_to_file(kernel_buffer, f);
	file_close(f);

        text_section = elf_enc_section_create(".text", kernel_buffer, kernel_buffer);
        text_section->header.sh_type = SHT_PROGBITS;

        /* Outer ELF */

        rodata_buffer = elf_enc_buffer_create();

        /* Need to create metadata here */

        /* Create Header */
        ptr = xcalloc(1, 32);
        ptr[20] = 1;
        elf_enc_buffer_write(rodata_buffer, ptr, 32);
        free(ptr);


        rodata_section = elf_enc_section_create(".rodata", rodata_buffer, rodata_buffer);
        rodata_section->header.sh_type = SHT_PROGBITS;

        symbol_table = elf_enc_symbol_table_create(".symtab", ".strtab");

        metadata = elf_enc_symbol_create("__OpenCL_kernel_metadata");
        metadata->symbol.st_shndx = 4;
        metadata->symbol.st_size = 0;
        metadata->symbol.st_info = ELF32_ST_TYPE(STT_OBJECT);
        elf_enc_symbol_table_add(symbol_table, metadata);

        kernel = elf_enc_symbol_create("__OpenCL_kernel_kernel");
        kernel->symbol.st_shndx = 5;
        kernel->symbol.st_size = kernel_buffer->size;
        kernel->symbol.st_info =  ELF32_ST_TYPE(STT_FUNC);
        elf_enc_symbol_table_add(symbol_table, kernel);
	
        header = elf_enc_symbol_create("__OpenCL_kernel_header");
        header->symbol.st_shndx = 4;
        header->symbol.st_size = 32;
        header->symbol.st_value = metadata->symbol.st_size;
        header->symbol.st_info = ELF32_ST_TYPE(STT_OBJECT);
        elf_enc_symbol_table_add(symbol_table, header);


        file = elf_enc_file_create();

        file->header.e_machine = 0x3fd;
        file->header.e_version = 1;

        elf_enc_file_add_symbol_table(file, symbol_table);

        elf_enc_file_add_buffer(file, rodata_buffer);
        elf_enc_file_add_section(file, rodata_section);

        elf_enc_file_add_buffer(file, kernel_buffer);
        elf_enc_file_add_section(file, text_section);

        elf_enc_file_generate(file, bin_buffer);

	//f = file_open_for_write("kernel.bin");
        //elf_enc_buffer_write_to_file(bin_buffer, f);
	//file_close(f);

        si2bin_inner_bin_free(bin);

        elf_enc_file_free(file);

        //elf_enc_buffer_free(bin_buffer);

}
