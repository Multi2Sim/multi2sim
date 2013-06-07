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

#include <stdio.h>

#include <arch/southern-islands/asm/arg.h>
#include <arch/southern-islands/asm/bin-file.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-encode.h>
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "metadata.h"
#include "outer-bin.h"

#define NUM_PROG_INFO_ELEM 114

struct si2bin_outer_bin_t *si2bin_outer_bin_create(void)
{
	struct si2bin_outer_bin_t *outer_bin;

	/* Initialize */
	outer_bin = xcalloc(1, sizeof(struct si2bin_outer_bin_t));

	outer_bin->file = elf_enc_file_create();

	outer_bin->inner_bin_list = list_create();

	outer_bin->metadata_list = list_create();
	
	return outer_bin;

}

void si2bin_outer_bin_free(struct si2bin_outer_bin_t *outer_bin)
{
	int i;

	elf_enc_file_free(outer_bin->file);
	
	LIST_FOR_EACH(outer_bin->inner_bin_list, i)
	{
		si2bin_inner_bin_free(list_get(outer_bin->inner_bin_list, i));
	}
	list_free(outer_bin->inner_bin_list);

	LIST_FOR_EACH(outer_bin->metadata_list, i)
	{
		si2bin_metadata_free(list_get(outer_bin->metadata_list, i));
	}
	list_free(outer_bin->metadata_list);
	
	free(outer_bin);
}

void si2bin_outer_bin_add(struct si2bin_outer_bin_t *outer_bin,
		struct si2bin_inner_bin_t *inner_bin, struct si2bin_metadata_t *metadata)
{
	list_add(outer_bin->inner_bin_list, inner_bin);
	list_add(outer_bin->metadata_list, metadata);
}

void si2bin_outer_bin_generate(struct si2bin_outer_bin_t *outer_bin,
		struct elf_enc_buffer_t *buffer)
{
	struct si2bin_inner_bin_t *inner_bin;
	struct si2bin_inner_bin_entry_t *entry;
	struct si2bin_inner_bin_constant_buffer_t *cb;
	struct si2bin_inner_bin_note_t *note;

	struct elf_enc_section_t *text_section;
	struct elf_enc_section_t *rodata_section;
	struct elf_enc_symbol_table_t *symbol_table;
	struct elf_enc_buffer_t *rodata_buffer;
	struct elf_enc_buffer_t *text_buffer;
	struct elf_enc_buffer_t *kernel_buffer;
	struct elf_enc_symbol_t *header_symbol;
	struct elf_enc_symbol_t *metadata_symbol;
	struct elf_enc_symbol_t *kernel_symbol;
	struct elf_enc_symbol_t *uav_symbol;
	struct elf_enc_symbol_t *cb_symbol;

	struct si2bin_metadata_t *metadata;
	struct si_arg_t *arg;
	struct pt_note_prog_info_entry_t prog_info[NUM_PROG_INFO_ELEM];

	char line[MAX_LONG_STRING_SIZE];
	char *data_type;
	char *scope;
	char *access_type;
	char *reflection;
	char *ptr;

	int i;
	int j;
	int k;
	int offset;
	int skip_cb;
	int uav[20];
	int rodata_size;
	int buff_num_offset;

	
	FILE *f;
	

	rodata_size = 0;

	/* Create Text Section */
	text_buffer = elf_enc_buffer_create();
        text_section = elf_enc_section_create(".text", text_buffer, text_buffer);
        text_section->header.sh_type = SHT_PROGBITS;
	
	/* Create .symtab section and .strtab section */
        symbol_table = elf_enc_symbol_table_create(".symtab", ".strtab");
	
	/* Create .rodata section */
	rodata_buffer = elf_enc_buffer_create();
 	rodata_section = elf_enc_section_create(".rodata", rodata_buffer, rodata_buffer);
        rodata_section->header.sh_type = SHT_PROGBITS;
	


	LIST_FOR_EACH(outer_bin->inner_bin_list, i)
	{
		offset = 0;
		skip_cb = 0;

		for(j = 0; j < 20; j++)
			uav[j] = 0;

		kernel_buffer = elf_enc_buffer_create();


		/* Initial Inner ELF settings */

		inner_bin = list_get(outer_bin->inner_bin_list, i);
		metadata = list_get(outer_bin->metadata_list, i);
		entry = list_get(inner_bin->entry_list, 0);

		inner_bin->file->header.e_machine = 0x7d;
		inner_bin->file->header.e_version = 1;
		inner_bin->file->header.e_ident[EI_OSABI] = 0x64;
		inner_bin->file->header.e_ident[EI_ABIVERSION] = 1;

		entry->header.d_machine = 26;
		entry->header.d_type = 0; /* ???? */


		/* Metadata -> .rodata section of Outer ELF */

		/* Print kernel name */
		snprintf(line, sizeof line, ";ARGSTART:__OpenCL_%s_kernel\n", inner_bin->name);
		elf_enc_buffer_write(rodata_buffer, line, strlen(line));

		/* Version */
		snprintf(line, sizeof line, ";version:3:1:104\n");
		elf_enc_buffer_write(rodata_buffer, line, strlen(line));

		/* Device */
		snprintf(line, sizeof line, ";device:tahiti\n");
		elf_enc_buffer_write(rodata_buffer, line, strlen(line));

		/* Unique ID */
		snprintf(line, sizeof line, ";uniqueid:%d\n", metadata->uniqueid);
		elf_enc_buffer_write(rodata_buffer, line, strlen(line));

		/* Memory - uavprivate */
		snprintf(line, sizeof line, ";memory:uavprivate:%d\n", metadata->uavprivate);
		elf_enc_buffer_write(rodata_buffer, line, strlen(line));

		/* Memory - hwregion */
		snprintf(line, sizeof line, ";memory:hwregion:%d\n", metadata->hwregion);
		elf_enc_buffer_write(rodata_buffer, line, strlen(line));

		/* Memory - hwlocal */
		snprintf(line, sizeof line, ";memory:hwlocal:%d\n", metadata->hwlocal);
		elf_enc_buffer_write(rodata_buffer, line, strlen(line));


		LIST_FOR_EACH(metadata->arg_list, j)
		{
			arg = list_get(metadata->arg_list, j);


			switch (arg->type)
			{
				case si_arg_pointer:

					data_type = str_map_value(&si_arg_data_type_map, arg->pointer.data_type);
					scope = str_map_value(&si_arg_scope_map, arg->pointer.scope);
					access_type = str_map_value(&si_arg_access_type_map, arg->pointer.access_type);

					snprintf(line, sizeof line, ";pointer:%s%d:%s:1:%d:%d:%s:%d:%d:%s:0:0\n", arg->name, j, 
							data_type, arg->pointer.constant_buffer_num, arg->pointer.constant_offset,
							scope, arg->pointer.buffer_num, 4, access_type);

					if (((arg->pointer.constant_offset - offset) < 16) && j && !(arg->pointer.constant_offset % 16)) 
						fatal("16 byte alignment not maintained in arg: %d", j);

					offset += arg->pointer.constant_offset;

					if (arg->pointer.scope == si_arg_uav && !(uav[arg->pointer.buffer_num]))
					{
						uav[arg->pointer.buffer_num] = 1;
					}

					elf_enc_buffer_write(rodata_buffer, line, strlen(line));

					break;

				case si_arg_value:

					data_type = str_map_value(&si_arg_data_type_map, arg->value.data_type);

					snprintf(line, sizeof line, ";value:%s%d:%s:%d:%d:%d\n", arg->name, j, 
							data_type, arg->value.num_elems, arg->value.constant_buffer_num, 
							arg->value.constant_offset);

					if (((arg->value.constant_offset - offset) < 16) && j && !(arg->value.constant_offset % 16)) 
						fatal("16 byte alignment not maintained in arg %d", j);

					offset += arg->value.constant_offset;

					elf_enc_buffer_write(rodata_buffer, line, strlen(line));


					break;

				default:
					fatal("Unrecognized argument type: arg %d", j);
			}
		}

		/* Function ID */
		snprintf(line, sizeof line, ";function:1:%d\n", metadata->uniqueid + 2);
		elf_enc_buffer_write(rodata_buffer, line, strlen(line));

		/* Private ID */
		snprintf(line, sizeof line, ";privateid:%d\n", 8);
		elf_enc_buffer_write(rodata_buffer, line, strlen(line));

		/* Reflections */
		LIST_FOR_EACH(metadata->arg_list, j)
		{
			arg = list_get(metadata->arg_list, j);


			switch (arg->type)
			{
				case si_arg_pointer:

					reflection = str_map_value(&si_arg_reflection_map, arg->pointer.data_type);

					snprintf(line, sizeof line, ";reflection:%d:%s*\n", j, reflection);
					elf_enc_buffer_write(rodata_buffer, line, strlen(line));

					break;

				case si_arg_value:

					reflection = str_map_value(&si_arg_reflection_map, arg->value.data_type);

					snprintf(line, sizeof line, ";reflection:%s\n", reflection);
					elf_enc_buffer_write(rodata_buffer, line, strlen(line));

					break;

				default:
					fatal("Unrecognized argument type: arg %d", j);
			}

		}								

		/* ARGEND */
		snprintf(line, sizeof line, ";ARGEND:__OpenCL_%s_kernel\n", inner_bin->name);
		elf_enc_buffer_write(rodata_buffer, line, strlen(line));


		snprintf(line, sizeof line + 1, "__OpenCL_%s_metadata", inner_bin->name);

		metadata_symbol = elf_enc_symbol_create(line);
		metadata_symbol->symbol.st_shndx = 4;
		metadata_symbol->symbol.st_size = rodata_buffer->offset - rodata_size;
		metadata_symbol->symbol.st_value = rodata_size;
		metadata_symbol->symbol.st_info = ELF32_ST_TYPE(STT_OBJECT);
		elf_enc_symbol_table_add(symbol_table, metadata_symbol);


		rodata_size = rodata_buffer->offset;



		/* Header ->  .rodata section */

		snprintf(line, sizeof line + 1, "__OpenCL_%s_header", inner_bin->name);

		header_symbol = elf_enc_symbol_create(line);
		header_symbol->symbol.st_shndx = 4;
		header_symbol->symbol.st_size = 32;
		header_symbol->symbol.st_value = rodata_size;
		header_symbol->symbol.st_info = ELF32_ST_TYPE(STT_OBJECT);
		elf_enc_symbol_table_add(symbol_table, header_symbol);

		ptr = xcalloc(1, 32);
		ptr[20] = 1;
		elf_enc_buffer_write(rodata_buffer, ptr, 32);
		free(ptr);


		/* Kernel -> .text section */
	        
		/* Create Notes */
		ptr = 0;

		/* ELF_NOTE_ATI_INPUTS */
		note = si2bin_inner_bin_note_create(2, 0, ptr);
		si2bin_inner_bin_entry_add_note(entry, note);

		/* ELF_NOTE_ATI_OUTPUTS */
		note = si2bin_inner_bin_note_create(3, 0, ptr);
		si2bin_inner_bin_entry_add_note(entry, note);


		buff_num_offset = 0;

		for (k = 0; k < 20; k++)
		{
			if (uav[k])
				buff_num_offset++;
		}


		/* ELF_NOTE_ATI_UAV */
		ptr = xcalloc(1, (16 * buff_num_offset));
		
		buff_num_offset = 0;
		
		/* UAV Symbols */
		for (k = 0; k < 20; k++)
		{
			if (!uav[k])
				continue;
		
			snprintf(line, sizeof line + 1, "uav%d", k);
			uav_symbol = elf_enc_symbol_create(line);
			uav_symbol->symbol.st_value = buff_num_offset;
			uav_symbol->symbol.st_shndx = 16;
			elf_enc_symbol_table_add(entry->symbol_table, uav_symbol);

			ptr[buff_num_offset * 16] = k;
			ptr[buff_num_offset * 16 + 4] = 4;
			ptr[buff_num_offset * 16 + 12] = 5;

			buff_num_offset++;
		}
	
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


		skip_cb = 0;

		for (k = 0; k < 3; k++)
		{
			cb = list_get(inner_bin->cb_list, k);
			
			if (!cb->start_reg && !cb->end_reg)
				continue;

			skip_cb++;
		}

		ptr = xcalloc(1, (8 * skip_cb));
		
		skip_cb = 0;

		/* CB Symbols */
		for (k = 2; k >= 0; k--)
		{
			cb = list_get(inner_bin->cb_list, k);

			if (!cb->start_reg && !cb->end_reg)
			{
				skip_cb++;
				continue;
			}

			snprintf(line, sizeof line + 1, "cb%d", cb->buffer_number);

			cb_symbol = elf_enc_symbol_create(line);
			cb_symbol->symbol.st_value = list_count(inner_bin->cb_list) - skip_cb - k - 1;
			cb_symbol->symbol.st_shndx = 10;
			elf_enc_symbol_table_add(entry->symbol_table, cb_symbol);

			ptr[(list_count(inner_bin->cb_list) - skip_cb - k - 1) * 8] = cb->buffer_number;
			
			if (cb->buffer_number == 0)
				ptr[12] = 0xf;

			if (cb->buffer_number == 1)
				ptr[4] = 3;

		}


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
		
		prog_info[0].address = 0x80001000;
		prog_info[0].value = 3;
		prog_info[1].address = 0x80001001;
		prog_info[1].value = 0x17;
		prog_info[2].address = 0x80001002;
		prog_info[2].value = 0;
		prog_info[3].address = 0x80001003;
		prog_info[3].value = 2;
		prog_info[4].address = 0x80001004;
		prog_info[4].value = 2;
		prog_info[5].address = 0x80001005;
		prog_info[5].value = 2;
		prog_info[6].address = 0x80001006;
		prog_info[6].value = 0;
		prog_info[7].address = 0x80001007;
		prog_info[7].value = 4;
		prog_info[8].address = 0x80001008;
		prog_info[8].value = 4;
		prog_info[9].address = 0x80001009;
		prog_info[9].value = 2;
		prog_info[10].address = 0x8000100a;
		prog_info[10].value = 1;
		prog_info[11].address = 0x8000100b;
		prog_info[11].value = 8;
		prog_info[12].address = 0x8000100c;
		prog_info[12].value = 4;
		prog_info[13].address = 0x8000100d;
		prog_info[13].value = 0;
		prog_info[14].address = 0x8000100e;
		prog_info[14].value = 0;
		prog_info[15].address = 0x8000100f;
		prog_info[15].value = 0;
		prog_info[16].address = 0x80001010;
		prog_info[16].value = 0;
		prog_info[17].address = 0x80001011;
		prog_info[17].value = 0;
		prog_info[18].address = 0x80001012;
		prog_info[18].value = 0;
		prog_info[19].address = 0x80001013;
		prog_info[19].value = 0;
		prog_info[20].address = 0x80001014;
		prog_info[20].value = 0;
		prog_info[21].address = 0x80001015;
		prog_info[21].value = 0;
		prog_info[22].address = 0x80001016;
		prog_info[22].value = 0;
		prog_info[23].address = 0x80001017;
		prog_info[23].value = 0;
		prog_info[24].address = 0x80001018;
		prog_info[24].value = 0;
		prog_info[25].address = 0x80001019;
		prog_info[25].value = 0;
		prog_info[26].address = 0x8000101a;
		prog_info[26].value = 0;
		prog_info[27].address = 0x8000101b;
		prog_info[27].value = 0;
		prog_info[28].address = 0x8000101c;
		prog_info[28].value = 0;
		prog_info[29].address = 0x8000101d;
		prog_info[29].value = 0;
		prog_info[30].address = 0x8000101e;
		prog_info[30].value = 0;
		prog_info[31].address = 0x8000101f;
		prog_info[31].value = 0;
		prog_info[32].address = 0x80001020;
		prog_info[32].value = 0;
		prog_info[33].address = 0x80001021;
		prog_info[33].value = 0;
		prog_info[34].address = 0x80001022;
		prog_info[34].value = 0;
		prog_info[35].address = 0x80001023;
		prog_info[35].value = 0;
		prog_info[36].address = 0x80001024;
		prog_info[36].value = 0;
		prog_info[37].address = 0x80001025;
		prog_info[37].value = 0;
		prog_info[38].address = 0x80001026;
		prog_info[38].value = 0;
		prog_info[39].address = 0x80001027;
		prog_info[39].value = 0;
		prog_info[40].address = 0x80001028;
		prog_info[40].value = 0;
		prog_info[41].address = 0x80001029;
		prog_info[41].value = 0;
		prog_info[42].address = 0x8000102a;
		prog_info[42].value = 0;
		prog_info[43].address = 0x8000102b;
		prog_info[43].value = 0;
		prog_info[44].address = 0x8000102c;
		prog_info[44].value = 0;
		prog_info[45].address = 0x8000102d;
		prog_info[45].value = 0;
		prog_info[46].address = 0x8000102e;
		prog_info[46].value = 0;
		prog_info[47].address = 0x8000102f;
		prog_info[47].value = 0;
		prog_info[48].address = 0x80001030;
		prog_info[48].value = 0;
		prog_info[49].address = 0x80001031;
		prog_info[49].value = 0;
		prog_info[50].address = 0x80001032;
		prog_info[50].value = 0;
		prog_info[51].address = 0x80001033;
		prog_info[51].value = 0;
		prog_info[52].address = 0x80001034;
		prog_info[52].value = 0;
		prog_info[53].address = 0x80001035;
		prog_info[53].value = 0;
		prog_info[54].address = 0x80001036;
		prog_info[54].value = 0;
		prog_info[55].address = 0x80001037;
		prog_info[55].value = 0;
		prog_info[56].address = 0x80001038;
		prog_info[56].value = 0;
		prog_info[57].address = 0x80001039;
		prog_info[57].value = 0;
		prog_info[58].address = 0x8000103a;
		prog_info[58].value = 0;
		prog_info[59].address = 0x8000103c;
		prog_info[59].value = 0;
		prog_info[60].address = 0x8000103d;
		prog_info[60].value = 0;
		prog_info[61].address = 0x8000103e;
		prog_info[61].value = 0;
		prog_info[62].address = 0x8000103f;
		prog_info[62].value = 0;
		prog_info[63].address = 0x80001040;
		prog_info[63].value = 0;
		prog_info[64].address = 0x80001041;
		prog_info[64].value = metadata->num_vgprs;
		prog_info[65].address = 0x80001042;
		prog_info[65].value = metadata->num_sgprs;;
		prog_info[66].address = 0x80001863;
		prog_info[66].value = 0x66;
		prog_info[67].address = 0x80001864;
		prog_info[67].value = 0x100;
		prog_info[68].address = 0x80001043;
		prog_info[68].value = 0xc0;
		prog_info[69].address = 0x80001044;
		prog_info[69].value = 0;
		prog_info[70].address = 0x80001045;
		prog_info[70].value = 0;
		prog_info[71].address = 0x00002e13;
		prog_info[71].value = 0x98;
		prog_info[72].address = 0x8000001c;
		prog_info[72].value = 0x100;
		prog_info[73].address = 0x8000001d;
		prog_info[73].value = 0;
		prog_info[74].address = 0x8000001e;
		prog_info[74].value = 0;
		prog_info[75].address = 0x80001841;
		prog_info[75].value = 0;
		prog_info[76].address = 0x8000001f;
		prog_info[76].value = 0x1c00;
		prog_info[77].address = 0x80001843;
		prog_info[77].value = 0x1c00;
		prog_info[78].address = 0x80001844;
		prog_info[78].value = 0;
		prog_info[79].address = 0x80001845;
		prog_info[79].value = 0;
		prog_info[80].address = 0x80001846;
		prog_info[80].value = 0;
		prog_info[81].address = 0x80001847;
		prog_info[81].value = 0;
		prog_info[82].address = 0x80001848;
		prog_info[82].value = 0;
		prog_info[83].address = 0x80001849;
		prog_info[83].value = 0;
		prog_info[84].address = 0x8000184a;
		prog_info[84].value = 0;
		prog_info[85].address = 0x8000184b;
		prog_info[85].value = 0;
		prog_info[86].address = 0x8000184c;
		prog_info[86].value = 0;
		prog_info[87].address = 0x8000184d;
		prog_info[87].value = 0;
		prog_info[88].address = 0x8000184e;
		prog_info[88].value = 0;
		prog_info[89].address = 0x8000184f;
		prog_info[89].value = 0;
		prog_info[90].address = 0x80001850;
		prog_info[90].value = 0;
		prog_info[91].address = 0x80001851;
		prog_info[91].value = 0;
		prog_info[92].address = 0x80001852;
		prog_info[92].value = 0;
		prog_info[93].address = 0x80001853;
		prog_info[93].value = 0;
		prog_info[94].address = 0x80001854;
		prog_info[94].value = 0;
		prog_info[95].address = 0x80001855;
		prog_info[95].value = 0;
		prog_info[96].address = 0x80001856;
		prog_info[96].value = 0;
		prog_info[97].address = 0x80001857;
		prog_info[97].value = 0;
		prog_info[98].address = 0x80001858;
		prog_info[98].value = 0;
		prog_info[99].address = 0x80001859;
		prog_info[99].value = 0;
		prog_info[100].address = 0x8000185a;
		prog_info[100].value = 0;
		prog_info[101].address = 0x8000185b;
		prog_info[101].value = 0;
		prog_info[102].address = 0x8000185c;
		prog_info[102].value = 0;
		prog_info[103].address = 0x8000185d;
		prog_info[103].value = 0;
		prog_info[104].address = 0x8000185e;
		prog_info[104].value = 0;
		prog_info[105].address = 0x8000185f;
		prog_info[105].value = 0;
		prog_info[106].address = 0x80001860;
		prog_info[106].value = 0;
		prog_info[107].address = 0x80001861;
		prog_info[107].value = 0;
		prog_info[108].address = 0x80001862;
		prog_info[108].value = 0;
		prog_info[109].address = 0x8000000a;
		prog_info[109].value = 1;
		prog_info[110].address = 0x80000078;
		prog_info[110].value = 0x40;
		prog_info[111].address = 0x80000081;
		prog_info[111].value = 0x8000;
		prog_info[112].address = 0x80000082;
		prog_info[112].value = 0;


		note = si2bin_inner_bin_note_create(1, 912, prog_info);
		si2bin_inner_bin_entry_add_note(entry, note);

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

		/* Data Section - Not supported yet (section is empty right now) */
		ptr = xcalloc(1, 4736);
		elf_enc_buffer_write(entry->data_section_buffer, ptr, 4736);
		free(ptr);


		/* Generate Inner Bin File */
		si2bin_inner_bin_generate(inner_bin, kernel_buffer);
		
		
		f = file_open_for_write("kernel");
		elf_enc_buffer_write_to_file(kernel_buffer, f);
		file_close(f);



		snprintf(line, sizeof line + 1, "__OpenCL_%s_kernel", inner_bin->name);

		kernel_symbol = elf_enc_symbol_create(line);
		kernel_symbol->symbol.st_shndx = 5;
		kernel_symbol->symbol.st_size = kernel_buffer->offset;
		kernel_symbol->symbol.st_value = text_buffer->offset;
		kernel_symbol->symbol.st_info =  ELF32_ST_TYPE(STT_FUNC);
		elf_enc_symbol_table_add(symbol_table, kernel_symbol);


		elf_enc_buffer_write(text_buffer, kernel_buffer->ptr, kernel_buffer->offset);


		elf_enc_buffer_free(kernel_buffer);

	}
	

        outer_bin->file->header.e_machine = 0x3fd;
        outer_bin->file->header.e_version = 1;

        elf_enc_file_add_symbol_table(outer_bin->file, symbol_table);

        elf_enc_file_add_buffer(outer_bin->file, rodata_buffer);
        elf_enc_file_add_section(outer_bin->file, rodata_section);

        elf_enc_file_add_buffer(outer_bin->file, text_buffer);
        elf_enc_file_add_section(outer_bin->file, text_section);

        elf_enc_file_generate(outer_bin->file, buffer);


}

