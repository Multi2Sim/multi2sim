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


/* #include <arch/fermi/asm/bin-file.h> */
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

struct frm2bin_inner_bin_note_t *frm2bin_inner_bin_note_create(unsigned int
	type, unsigned int size, void *payload)
{
	struct frm2bin_inner_bin_note_t *note;

	/* Initialize */
	note = xcalloc(1, sizeof(struct frm2bin_inner_bin_note_t));
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


void frm2bin_inner_bin_note_free(struct frm2bin_inner_bin_note_t *note)
{
	if (note->size)
		free(note->payload);
	free(note);
}


//void frm2bin_inner_bin_note_dump(struct elf_enc_buffer_t *buffer, FILE *fp)
//{
//	int offset;
//	int descsz;
//	int n;
//
//	char *note_type_str;
//
//	struct elf_enc_buffer_t *payload;
//
//	descsz = 0;
//	offset = 0;
//	n = 0;
//
//	while(offset < buffer->size)
//	{
//		fprintf(fp, "Note %d", n);
//		fprintf(fp, "\n Name Size: %d", *((int *)(buffer->ptr + offset)));
//		offset += 4;
//
//		fprintf(fp, "\n Description Size: %d", *((int *)(buffer->ptr + offset)));
//		descsz = *((int *)(buffer->ptr + offset));
//		offset += 4;
//
//		note_type_str = str_map_value(&pt_note_type_map, *((int *)(buffer->ptr + offset)));
//		fprintf(fp, "\n Type: %s", note_type_str);
//		offset += 4;
//
//		fprintf(fp, "\n Name: %s\n", (char *)(buffer->ptr + offset));
//		offset += 8;
//
//		payload = elf_enc_buffer_create();
//		elf_enc_buffer_write(payload, buffer->ptr + offset, descsz);
//		elf_enc_buffer_dump(payload, fp);
//		fprintf(fp, "\n\n");
//		elf_enc_buffer_free(payload);
//
//		offset += descsz;
//
//		n++;
//	}
//
//}




/*
 * Object representing an Encoding Dictionary Entry
 */


struct frm2bin_inner_bin_entry_t *frm2bin_inner_bin_entry_create(void)
{
	struct frm2bin_inner_bin_entry_t *entry;

	/* Initialize */
	entry = xcalloc(1, sizeof(struct frm2bin_inner_bin_entry_t));

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
	
	/* .nv.info section initialization */
	entry->nv_info_section_buffer = elf_enc_buffer_create();
	entry->nv_info_section = elf_enc_section_create(".nv.info", entry->nv_info_section_buffer,
		entry->nv_info_section_buffer);
	entry->nv_info_section->header.sh_type = SHT_LOPROC;

	/* .nv.constant0 section initialization */
	entry->nv_constant0_section_buffer = elf_enc_buffer_create();
	entry->nv_constant0_section = elf_enc_section_create(".nv.constant0", entry->nv_constant0_section_buffer,
		entry->nv_constant0_section_buffer);
	entry->nv_constant0_section->header.sh_type = SHT_PROGBITS;

	/* .nv.constant0 section initialization */
	entry->nv_constant16_section_buffer = elf_enc_buffer_create();
	entry->nv_constant16_section = elf_enc_section_create(".nv.constant16", entry->nv_constant16_section_buffer,
		entry->nv_constant16_section_buffer);
	entry->nv_constant16_section->header.sh_type = SHT_PROGBITS;

	/* .nv.shared section initialization */
	entry->nv_shared_section_buffer = elf_enc_buffer_create();
	entry->nv_shared_section = elf_enc_section_create(".nv.shared", entry->nv_shared_section_buffer,
		entry->nv_shared_section_buffer);
	entry->nv_shared_section->header.sh_type = SHT_NOBITS;


	/* Symbol Table Initialization */
	entry->symbol_table = elf_enc_symbol_table_create(".symtab", ".strtab");

	/* Make note list and buffer for note segment */
	//entry->note_list = list_create();
	//entry->note_buffer = elf_enc_buffer_create();

	
	/* Return */
	return entry;
}


void frm2bin_inner_bin_entry_free(struct frm2bin_inner_bin_entry_t *entry)
{
	//int i;

	/* Free list element and list */
	//LIST_FOR_EACH(entry->note_list, i)
	//	frm2bin_inner_bin_note_free(list_get(entry->note_list, i));
	//list_free(entry->note_list);

	/* Free buffer */
	//elf_enc_buffer_free(entry->text_section);

	/* Free bin entry */
	free(entry);
}


void frm2bin_inner_bin_entry_add_note(struct frm2bin_inner_bin_entry_t *entry,
	struct frm2bin_inner_bin_note_t *note)
{
	/* do nothing. Fermi doesn't have note */
	list_add(entry->note_list, note);
}




/* 
 * AMD Internal Binary Object
 */


struct frm2bin_inner_bin_t *frm2bin_inner_bin_create(char *name)
{
	struct frm2bin_inner_bin_t *bin;
	struct elf_enc_buffer_t *buffer;
	struct elf_enc_segment_t *segment;

	/* Initialize */
	bin = xcalloc(1, sizeof(struct frm2bin_inner_bin_t));
	bin->file = elf_enc_file_create();
	bin->entry_list = list_create();

	/* Initialize the argument total size to 0 */
	bin->arg_totalSize = 0;

	/* Create buffer and segment for encoding dictionary */
	buffer = elf_enc_buffer_create();
	elf_enc_file_add_buffer(bin->file, buffer);

	segment =
		elf_enc_segment_create("Encoding Dictionary", buffer, buffer);
	elf_enc_file_add_segment(bin->file, segment);

	segment->header.p_type = PT_LOPROC + 2;

	/* Save kernel name */
	bin->name = xstrdup(name);

	/* Set up user element list and program resource */
	/* bin->pgm_rsrc2 = frm_bin_compute_pgm_rsrc2_create(); */
	bin->user_element_list = list_create();

	/* Return */
	return bin;
}


void frm2bin_inner_bin_free(struct frm2bin_inner_bin_t *bin)
{
	/* struct frm_bin_enc_user_element_t *user_elem; */
	int i;

	/* Free list elements and list */
	LIST_FOR_EACH(bin->entry_list, i)
		frm2bin_inner_bin_entry_free(list_get(bin->entry_list, i));
	list_free(bin->entry_list);

	/* Free elf_enc_file */
	elf_enc_file_free(bin->file);

	/* Free kernel name */
	free(bin->name);

	/* Free User Element List */
	LIST_FOR_EACH(bin->user_element_list, i)
	{
		/* user_elem = list_get(bin->user_element_list, i); */
		/* frm_bin_enc_user_element_free(user_elem); */
	}
	list_free(bin->user_element_list);

	/* Free Program Resource */
	/* frm_bin_compute_pgm_rsrc2_free(bin->pgm_rsrc2); */

	/* Free frm2bin_inner_bin */
	free(bin);
}

void frm2bin_inner_bin_add_user_element(struct frm2bin_inner_bin_t *bin,
	struct frm_bin_enc_user_element_t *user_elem, int index)
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

void frm2bin_inner_bin_add_entry(struct frm2bin_inner_bin_t *bin,
	struct frm2bin_inner_bin_entry_t *entry)
{
	//struct elf_enc_segment_t *note_segment;
	//struct elf_enc_segment_t *load_segment;
	
	/* Add entry */
	list_add(bin->entry_list, entry);

	/* Add note_buffer and create note segment */
	//elf_enc_file_add_buffer(bin->file, entry->note_buffer);
        //
	//note_segment = elf_enc_segment_create("Note Segment", entry->note_buffer,
	//		entry->note_buffer);
	//elf_enc_file_add_segment(bin->file, note_segment);
        //
	//note_segment->header.p_type = PT_NOTE;


	/* Add text section and text buffer section to elf_enc_file */
	elf_enc_file_add_buffer(bin->file, entry->text_section_buffer);
	elf_enc_file_add_section(bin->file, entry->text_section);

	/* Add data section and data buffer section to elf_enc_file */
	elf_enc_file_add_buffer(bin->file, entry->data_section_buffer);
	elf_enc_file_add_section(bin->file, entry->data_section);

	/* Add constant0 section and constant0 buffer section to elf_enc_file */
	elf_enc_file_add_buffer(bin->file, entry->nv_constant0_section_buffer);
	elf_enc_file_add_section(bin->file, entry->nv_constant0_section);

	/* Add constant0 section and constant0 buffer section to elf_enc_file */
	elf_enc_file_add_buffer(bin->file, entry->nv_constant16_section_buffer);
	elf_enc_file_add_section(bin->file, entry->nv_constant16_section);


	/* Add info section and info buffer section to elf_enc_file */
	elf_enc_file_add_buffer(bin->file, entry->nv_info_section_buffer);
	elf_enc_file_add_section(bin->file, entry->nv_info_section);

	/* Add shared section and shared buffer section to elf_enc_file */
	elf_enc_file_add_buffer(bin->file, entry->nv_shared_section_buffer);
	elf_enc_file_add_section(bin->file, entry->nv_shared_section);

	/* Add local section and local buffer section to elf_enc_file */
	//elf_enc_file_add_buffer(bin->file, entry->nv_local_section_buffer);
	//elf_enc_file_add_section(bin->file, entry->nv_local_section);

	/* Add symbol table section and text buffer section to elf_enc_file */
	elf_enc_file_add_symbol_table(bin->file, entry->symbol_table);

	/* Create load segment */
	//load_segment = elf_enc_segment_create("Load Segment", entry->text_section_buffer,
	//		entry->symbol_table->string_table_buffer);
	//elf_enc_file_add_segment(bin->file, load_segment);
        //
	//load_segment->header.p_type = PT_LOAD;


}


void frm2bin_inner_bin_generate(struct frm2bin_inner_bin_t *bin,
	struct elf_enc_buffer_t *bin_buffer)
{
	int i;
	unsigned int *tmp_buf;

//	int namesz;
//	int start;
//	int end;
//	int buf_offset;
//	int phtab_size;
//
//	char *name;
//
//	struct frm2bin_inner_bin_note_t *note;
	struct frm2bin_inner_bin_entry_t *entry;
//	struct elf_enc_buffer_t *enc_dict;
//	struct elf_enc_buffer_t *buffer;
//
//
//	namesz = 8;
//	name = "ATI CAL";
//
//	enc_dict = list_get(bin->file->buffer_list, 2);
//
//	phtab_size = sizeof(Elf32_Phdr) * list_count(bin->file->segment_list);

	//struct elf_enc_section_t *section;
	//struct elf_enc_buffer_t *buffer;

	LIST_FOR_EACH(bin->entry_list, i)
	{
		entry = list_get(bin->entry_list, i);

		/* generate the constant0 section */

		/* create an array with all 0s */
		tmp_buf = (unsigned int *) xmalloc(0x20 + bin->arg_totalSize);
		memset(tmp_buf, 0, (0x20 + bin->arg_totalSize));

		/* fill constant0 section will 0s */
		elf_enc_buffer_write(entry->nv_constant0_section_buffer,
			tmp_buf, (0x20 + bin->arg_totalSize));

		free(tmp_buf);

		/* generate the .nv.info.kernName section */
		if (bin->arg_totalSize == 0)
		{
			/* no parameter */
			tmp_buf = (unsigned int *) xmalloc(12);
			*tmp_buf++ = 0x00080a04;

			/* FIXME: symbol index of constant0 section */
			*tmp_buf++ = entry->nv_constant0_section->index;
			*tmp_buf = 0x200000;
		}
		else
		{

		}

		/* doesn't know how to populate local, shared and constant16
		 * section yet */


//		LIST_FOR_EACH(entry->note_list, i)
//		{
//
//			/* Write name, size, type, etc. to buffer */
//			note = list_get(entry->note_list, i);
//			elf_enc_buffer_write(entry->note_buffer, &namesz, 4);
//			elf_enc_buffer_write(entry->note_buffer, &note->size, 4);
//			elf_enc_buffer_write(entry->note_buffer, &note->type, 4);
//			elf_enc_buffer_write(entry->note_buffer, name, 8);
//			elf_enc_buffer_write(entry->note_buffer, note->payload,
//					note->size);
//		}
//
//		start = entry->note_buffer->index;
//		end = entry->symbol_table->string_table_buffer->index;
//
//		/* Calculate offset and type for enc_dict */
//		for (i = start; i <= end; i++)
//		{
//			buffer = list_get(bin->file->buffer_list, i);
//			entry->header.d_size += buffer->size;
//		}
//
//		buf_offset = 0;
//
//		for (i = 0; i < start; i++)
//		{
//			buffer = list_get(bin->file->buffer_list, i);
//			buf_offset += buffer->size;
//		}
//
//		entry->header.d_offset = sizeof(Elf32_Ehdr) + phtab_size +
//			sizeof(struct frm2bin_inner_bin_entry_header_t) *
//			list_count(bin->entry_list) + buf_offset;
//
//
//		/* Write information to enc_dict */
//		elf_enc_buffer_write(enc_dict, &entry->header,
//			sizeof(struct frm2bin_inner_bin_entry_header_t));

	}

	



	/* Write elf_enc_file to buffer */
	elf_enc_file_generate(bin->file, bin_buffer);

	/* write kernel buffer to bin buffer */
	//elf_enc_buffer_write();

}


void frm2bin_inner_bin_create_file(struct elf_enc_buffer_t
	*text_section_buffer, struct elf_enc_buffer_t *bin_buffer)
{
	char *ptr;

	struct frm2bin_inner_bin_t *bin;
	struct frm2bin_inner_bin_entry_t *entry;
	struct frm2bin_inner_bin_note_t *note;
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
	

	bin = frm2bin_inner_bin_create("kernel");

	bin->file->header.e_machine = 0x7d;
	bin->file->header.e_version = 1;
	bin->file->header.e_ident[EI_OSABI] = 0x64;
	bin->file->header.e_ident[EI_ABIVERSION] = 1;

	entry = frm2bin_inner_bin_entry_create();


	entry->header.d_machine = 26;
	entry->header.d_type = 0;	/* ???? */

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


	elf_enc_buffer_write(entry->text_section_buffer,
		text_section_buffer->ptr, text_section_buffer->offset);


	ptr = xcalloc(1, 4736);
	elf_enc_buffer_write(entry->data_section_buffer, ptr, 4736);
	free(ptr);


	/* Create Notes */
	ptr = 0;

	/* ELF_NOTE_ATI_INPUTS */
	note = frm2bin_inner_bin_note_create(2, 0, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);

	/* ELF_NOTE_ATI_OUTPUTS */
	note = frm2bin_inner_bin_note_create(3, 0, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);

	/* ELF_NOTE_ATI_UAV */
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

	note = frm2bin_inner_bin_note_create(16, 48, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);
	free(ptr);

	/* ELF_NOTE_ATI_CONDOUT */
	ptr = xcalloc(1, 4);
	note = frm2bin_inner_bin_note_create(4, 4, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);
	free(ptr);

	/* ELF_NOTE_ATI_FLOAT32CONSTS */
	note = frm2bin_inner_bin_note_create(5, 0, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);

	/* ELF_NOTE_ATI_INT32CONSTS */
	note = frm2bin_inner_bin_note_create(6, 0, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);

	/* ELF_NOTE_ATI_BOOL32CONSTS */
	note = frm2bin_inner_bin_note_create(7, 0, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);

	/* ELF_NOTE_ATI_EARLYEXIT */
	ptr = xcalloc(1, 4);
	note = frm2bin_inner_bin_note_create(8, 4, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);
	free(ptr);

	/* ELF_NOTE_ATI_GLOBAL_BUFFERS */
	note = frm2bin_inner_bin_note_create(9, 0, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);

	/* ELF_NOTE_ATI_CONSTANT_BUFFERS */

	ptr = xcalloc(1, 16);

	ptr[0] = 1;
	ptr[4] = 3;
	ptr[12] = 0xf;

	note = frm2bin_inner_bin_note_create(10, 16, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);

	free(ptr);

	/* ELF_NOTE_ATI_INPUT_SAMPLERS */
	note = frm2bin_inner_bin_note_create(11, 0, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);

	/* ELF_NOTE_ATI_SCRATCH_BUFFERS */
	ptr = xcalloc(1, 4);
	note = frm2bin_inner_bin_note_create(13, 4, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);
	free(ptr);

	/* ELF_NOTE_ATI_PERSISTENT_BUFFERS */
	note = frm2bin_inner_bin_note_create(12, 0, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);

	/* ELF_NOTE_ATI_PROGINFO */
	ptr = xcalloc(1, 912);
	note = frm2bin_inner_bin_note_create(1, 912, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);
	free(ptr);

	/* ELF_NOTE_ATI_SUB_CONSTANT_BUFFERS */
	note = frm2bin_inner_bin_note_create(14, 0, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);

	/* ELF_NOTE_ATI_UAV_MAILBOX_SIZE */
	ptr = xcalloc(1, 4);
	note = frm2bin_inner_bin_note_create(15, 4, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);
	free(ptr);

	/* ELF_NOTE_ATI_UAV_OP_MASK */
	ptr = xcalloc(1, 128);

	ptr[1] = 0x1c;

	note = frm2bin_inner_bin_note_create(17, 4, ptr);
	frm2bin_inner_bin_entry_add_note(entry, note);
	free(ptr);



	frm2bin_inner_bin_add_entry(bin, entry);

	kernel_buffer = elf_enc_buffer_create();

	frm2bin_inner_bin_generate(bin, kernel_buffer);

	f = file_open_for_write("kernel");
	elf_enc_buffer_write_to_file(kernel_buffer, f);
	file_close(f);

	text_section =
		elf_enc_section_create(".text", kernel_buffer, kernel_buffer);
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
	kernel->symbol.st_info = ELF32_ST_TYPE(STT_FUNC);
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

	/* f = file_open_for_write("kernel.bin"); */
	/* elf_enc_buffer_write_to_file(bin_buffer, f); */
	/* file_close(f); */

	frm2bin_inner_bin_free(bin);

	elf_enc_file_free(file);

	/* elf_enc_buffer_free(bin_buffer); */

}
