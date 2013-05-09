/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-encode.h>
#include <lib/util/elf-format.h>
#include <lib/util/file.h>

#include "bin.h"
#include "inst.h"
#include "si2bin.h"
#include <arch/southern-islands/asm/bin-file.h>
#include "task.h"
#include "stream.h"


/*
 * Stream Object
 */

struct si2bin_stream_t *si2bin_stream_create(int size)
{
	struct si2bin_stream_t *stream;
	
	stream = xcalloc(1, sizeof(struct si2bin_stream_t));
	stream->buf = xcalloc(1, size);
	stream->size = size;
	
	return stream;
}


void si2bin_stream_free(struct si2bin_stream_t *stream)
{
	free(stream->buf);
	free(stream);
}


void si2bin_stream_add_inst(struct si2bin_stream_t *stream,
		struct si2bin_inst_t *inst)
{
	/* Generate code for instruction */
	si2bin_inst_gen(inst);

	/* Size of stream exceeded */
	if (stream->offset + inst->size > stream->size)
		si2bin_yyerror("output stream size exceeded");

	/* Add instruction */
	memcpy(stream->buf + stream->offset, inst->inst_bytes.bytes, inst->size);
	stream->offset += inst->size;
}


void si2bin_stream_dump(struct si2bin_stream_t *stream, FILE *f)
{
	/*struct elf_buffer_t buffer;

	buffer.ptr = stream->buf;
	buffer.size = stream->offset;
	si_disasm_buffer(&buffer, f);*/
	
	struct si2bin_bin_t *bin;
        bin = si2bin_bin_create();

        bin->file->header.e_machine = 0x7d;
        bin->file->header.e_version = 1;
        bin->file->header.e_ident[EI_OSABI] = 0x64;
        bin->file->header.e_ident[EI_ABIVERSION] = 1;

        struct si2bin_bin_entry_t *entry;
        entry = si2bin_bin_entry_create();
	

        entry->header.d_machine = 26;
        entry->header.d_type = 0; /* ???? */

        struct elf_enc_symbol_t *uav11;
        uav11 = elf_enc_symbol_create("uav11");
        uav11->symbol.st_shndx = 16;
        elf_enc_symbol_table_add(entry->symbol_table, uav11);

        struct elf_enc_symbol_t *uav10;
        uav10 = elf_enc_symbol_create("uav10");
        uav10->symbol.st_value = 1;
        uav10->symbol.st_shndx = 16;
        elf_enc_symbol_table_add(entry->symbol_table, uav10);

        struct elf_enc_symbol_t *uav12;
        uav12 = elf_enc_symbol_create("uav12");
        uav12->symbol.st_value = 2;
        uav12->symbol.st_shndx = 16;
        elf_enc_symbol_table_add(entry->symbol_table, uav12);

        struct elf_enc_symbol_t *cb1;
        cb1 = elf_enc_symbol_create("cb1");
        cb1->symbol.st_shndx = 10;
        elf_enc_symbol_table_add(entry->symbol_table, cb1);

	
	struct elf_enc_symbol_t *cb0;
        cb0 = elf_enc_symbol_create("cb0");
        cb0->symbol.st_value = 1;
        cb0->symbol.st_shndx = 10;
        elf_enc_symbol_table_add(entry->symbol_table, cb0);
	

	elf_enc_buffer_write(entry->text_section_buffer, stream->buf, stream->offset);
        
        void *ptr;

        ptr = xcalloc(1, 4736);
        elf_enc_buffer_write(entry->data_section_buffer, ptr, 4736);
        free(ptr);


	/* Create Notes */	
        struct si2bin_bin_note_t *note;
	char *note_ptr;
	note_ptr = 0;
	
	/* ELF_NOTE_ATI_INPUTS */
	note = si2bin_bin_note_create(2, 0, note_ptr);
        si2bin_bin_entry_add_note(entry, note);

	/*ELF_NOTE_ATI_OUTPUTS */
	note = si2bin_bin_note_create(3, 0, note_ptr);
	si2bin_bin_entry_add_note(entry, note);

	/*ELF_NOTE_ATI_UAV */
	note_ptr = xcalloc(1, 48);
	
	note_ptr[0] = 0xb;
	note_ptr[16] = 0xa;
	note_ptr[32] = 0xc;

	note_ptr[4] = 4;
	note_ptr[20] = 4;
	note_ptr[36] = 4;

	note_ptr[12] = 5;
	note_ptr[28] = 5;
	note_ptr[44] = 5;

	note = si2bin_bin_note_create(16, 48, note_ptr);
	si2bin_bin_entry_add_note(entry, note);
	free(note_ptr);

	/* ELF_NOTE_ATI_CONDOUT */
	note_ptr = xcalloc(1, 4);
	note = si2bin_bin_note_create(4, 4, note_ptr);
	si2bin_bin_entry_add_note(entry, note);
	free(note_ptr);

	/* ELF_NOTE_ATI_FLOAT32CONSTS */
	note = si2bin_bin_note_create(5, 0, note_ptr);
	si2bin_bin_entry_add_note(entry, note);
	
	/* ELF_NOTE_ATI_INT32CONSTS */
	note = si2bin_bin_note_create(6, 0, note_ptr);
	si2bin_bin_entry_add_note(entry, note);
	
	/* ELF_NOTE_ATI_BOOL32CONSTS */
	note = si2bin_bin_note_create(7, 0, note_ptr);
	si2bin_bin_entry_add_note(entry, note);
	
	/* ELF_NOTE_ATI_EARLYEXIT */
	note_ptr = xcalloc(1, 4);
	note = si2bin_bin_note_create(8, 4, note_ptr);
	si2bin_bin_entry_add_note(entry, note);
	free(note_ptr);

	/* ELF_NOTE_ATI_GLOBAL_BUFFERS */
	note = si2bin_bin_note_create(9, 0, note_ptr);
	si2bin_bin_entry_add_note(entry, note);

	/* ELF_NOTE_ATI_CONSTANT_BUFFERS */
	
	note_ptr = xcalloc(1, 16);
		
	note_ptr[0] = 1;
	note_ptr[4] = 3;
	note_ptr[12] = 0xf;

	note = si2bin_bin_note_create(10, 16, note_ptr);
	si2bin_bin_entry_add_note(entry, note);

	free(note_ptr);

	/* ELF_NOTE_ATI_INPUT_SAMPLERS */
	note = si2bin_bin_note_create(11, 0, note_ptr);
	si2bin_bin_entry_add_note(entry, note);
	
	/* ELF_NOTE_ATI_SCRATCH_BUFFERS */
	note_ptr = xcalloc(1, 4);
	note = si2bin_bin_note_create(13, 4, note_ptr);
	si2bin_bin_entry_add_note(entry, note);
	free(note_ptr);
	
	/* ELF_NOTE_ATI_PERSISTENT_BUFFERS */
	note = si2bin_bin_note_create(12, 0, note_ptr);
	si2bin_bin_entry_add_note(entry, note);
	
	/* ELF_NOTE_ATI_PROGINFO */
	note_ptr = xcalloc(1, 912);
	note = si2bin_bin_note_create(1, 912, note_ptr);
	si2bin_bin_entry_add_note(entry, note);
	free(note_ptr);

	/* ELF_NOTE_ATI_SUB_CONSTANT_BUFFERS */
	note = si2bin_bin_note_create(14, 0, note_ptr);
	si2bin_bin_entry_add_note(entry, note);
	
	/* ELF_NOTE_ATI_UAV_MAILBOX_SIZE */
	note_ptr = xcalloc(1, 4);
	note = si2bin_bin_note_create(15, 4, note_ptr);
	si2bin_bin_entry_add_note(entry, note);
	free(note_ptr);

	/* ELF_NOTE_ATI_UAV_OP_MASK */
	note_ptr = xcalloc(1, 128);
	
	note_ptr[1] = 0x1c;

	note = si2bin_bin_note_create(17, 4, note_ptr);
	si2bin_bin_entry_add_note(entry, note);
	free(note_ptr);
	


        si2bin_bin_add_entry(bin, entry);


        struct elf_enc_buffer_t *kernel_buffer;

        kernel_buffer = elf_enc_buffer_create();

        si2bin_bin_generate(bin, kernel_buffer);

        f = file_open_for_write("kernel");
	elf_enc_buffer_write_to_file(kernel_buffer, f);
	file_close(f);

        struct elf_enc_section_t *text_section;
        text_section = elf_enc_section_create(".text", kernel_buffer, kernel_buffer);
        text_section->header.sh_type = SHT_PROGBITS;

        /* Outer ELF */

        struct elf_enc_buffer_t *rodata_buffer;
        rodata_buffer = elf_enc_buffer_create();

	/* Need to create metadata here */
	
	/* Create Header */
	note_ptr = xcalloc(1, 32);
	note_ptr[20] = 1;
	elf_enc_buffer_write(rodata_buffer, ptr, 32);
	free(note_ptr);	


	struct elf_enc_section_t *rodata_section;
        rodata_section = elf_enc_section_create(".rodata", rodata_buffer, rodata_buffer);
        rodata_section->header.sh_type = SHT_PROGBITS;

        struct elf_enc_symbol_table_t *symbol_table;
        symbol_table = elf_enc_symbol_table_create(".symtab", ".strtab");

        struct elf_enc_symbol_t *metadata;
        metadata = elf_enc_symbol_create("__OpenCL_kernel_metadata");
        metadata->symbol.st_shndx = 4;
        metadata->symbol.st_size = 369;
        metadata->symbol.st_info = ELF32_ST_TYPE(STT_OBJECT);
        elf_enc_symbol_table_add(symbol_table, metadata);

        struct elf_enc_symbol_t *kernel;
        kernel = elf_enc_symbol_create("__OpenCL_kernel_kernel");
        kernel->symbol.st_shndx = 5;
        kernel->symbol.st_size = kernel_buffer->size;
        kernel->symbol.st_info =  ELF32_ST_TYPE(STT_FUNC);
        elf_enc_symbol_table_add(symbol_table, kernel);

        struct elf_enc_symbol_t *header;
        header = elf_enc_symbol_create("__OpenCL_kernel_header");
        header->symbol.st_shndx = 4;
        header->symbol.st_size = 32;
        header->symbol.st_value = 369;
        header->symbol.st_info = ELF32_ST_TYPE(STT_OBJECT);
        elf_enc_symbol_table_add(symbol_table, header);


        struct elf_enc_file_t *file;
        file = elf_enc_file_create();

        file->header.e_machine = 0x3fd;
        file->header.e_version = 1;

        elf_enc_file_add_symbol_table(file, symbol_table);

        elf_enc_file_add_buffer(file, rodata_buffer);
        elf_enc_file_add_section(file, rodata_section);

        elf_enc_file_add_buffer(file, kernel_buffer);
        elf_enc_file_add_section(file, text_section);

	struct elf_enc_buffer_t *bin_buffer;
        bin_buffer = elf_enc_buffer_create();

        elf_enc_file_generate(file, bin_buffer);
	
	f = file_open_for_write("kernel.bin");
	elf_enc_buffer_write_to_file(bin_buffer, f);
	file_close(f);

        si2bin_bin_free(bin);

        elf_enc_file_free(file);

        elf_enc_buffer_free(bin_buffer);


}



/*
 * Global
 */

struct si2bin_stream_t *si2bin_out_stream;

void si2bin_stream_init(void)
{
	si2bin_out_stream = si2bin_stream_create(1 << 20);  /* 2KB */
}


void si2bin_stream_done(void)
{
	si2bin_stream_dump(si2bin_out_stream, stdout);
	si2bin_stream_free(si2bin_out_stream);
}


