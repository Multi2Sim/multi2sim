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
#include "data.h"
#include "si2bin.h"


#define NUM_PROG_INFO_ELEM 114
#define MAX_UAV_NUM 25
#define MAX_CB_NUM 25

struct str_map_t si2bin_outer_bin_device_map = 
{
	4,
	{
		{"invalid", si2bin_outer_bin_invalid},
		{"capeverde", si2bin_outer_bin_cape_verde},
		{"pitcairn", si2bin_outer_bin_pitcairn},
		{"tahiti", si2bin_outer_bin_tahiti},
	}

};

struct si2bin_outer_bin_t *si2bin_outer_bin_create(void)
{
	struct si2bin_outer_bin_t *outer_bin;

	/* Initialize */
	outer_bin = xcalloc(1, sizeof(struct si2bin_outer_bin_t));

	/* Create Lists */
	outer_bin->file = elf_enc_file_create();
	outer_bin->data_list = list_create();
	outer_bin->inner_bin_list = list_create();
	outer_bin->metadata_list = list_create();

	/* Return */
	return outer_bin;

}

void si2bin_outer_bin_free(struct si2bin_outer_bin_t *outer_bin)
{
	int i;

	elf_enc_file_free(outer_bin->file);

	LIST_FOR_EACH(outer_bin->data_list, i)
	{
		si2bin_data_free(list_get(outer_bin->data_list, i));
	}
	list_free(outer_bin->data_list);

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

void si2bin_outer_bin_add_data(struct si2bin_outer_bin_t *outer_bin,
		struct si2bin_data_t *data)
{
	list_add(outer_bin->data_list, data);
}

void si2bin_outer_bin_add(struct si2bin_outer_bin_t *outer_bin,
		struct si2bin_inner_bin_t *inner_bin,
		struct si2bin_metadata_t *metadata)
{
	list_add(outer_bin->inner_bin_list, inner_bin);
	list_add(outer_bin->metadata_list, metadata);
}

void si2bin_outer_bin_generate(struct si2bin_outer_bin_t *outer_bin,
		struct elf_enc_buffer_t *buffer)
{
	struct si2bin_inner_bin_t *inner_bin;
	struct si2bin_inner_bin_entry_t *entry;
	struct si2bin_inner_bin_note_t *note;

	struct elf_enc_section_t *text_section;
	struct elf_enc_section_t *rodata_section;
	struct elf_enc_symbol_table_t *symbol_table;
	struct elf_enc_buffer_t *rodata_buffer;
	struct elf_enc_buffer_t *text_buffer;
	struct elf_enc_buffer_t *kernel_buffer;
	struct elf_enc_symbol_t *global_symbol;
	struct elf_enc_symbol_t *header_symbol;
	struct elf_enc_symbol_t *metadata_symbol;
	struct elf_enc_symbol_t *kernel_symbol;
	struct elf_enc_symbol_t *uav_symbol;
	struct elf_enc_symbol_t *cb_symbol;

	struct si2bin_metadata_t *metadata;
	struct si_arg_t *arg;
	struct si_bin_enc_user_element_t *user_elem;
	struct pt_note_prog_info_entry_t prog_info[NUM_PROG_INFO_ELEM];
	struct si2bin_data_t *data;

	char line[MAX_LONG_STRING_SIZE];
	char *data_type;
	char *scope;
	char *access_type;
	char *reflection;
	char *ptr;
	unsigned char byte;

	int i;
	int j;
	int k;
	int offset;
	int uav[MAX_UAV_NUM];
	int cb[MAX_CB_NUM];
	int rodata_size;
	int buff_num_offset;
	int buff_size;
	int data_size;
	int glob_size;
	int imm_cb_found;
	int ptr_cb_table_found;
		

	/* Set machine type */
	outer_bin->device = str_map_string_case(&si2bin_outer_bin_device_map, si2bin_machine_name);

	if(outer_bin->device == si2bin_outer_bin_invalid)
	{
		fatal("Invalid machine type");
	}


	/* Create Text Section */
	text_buffer = elf_enc_buffer_create();
	text_section = elf_enc_section_create(".text", text_buffer, text_buffer);
	text_section->header.sh_type = SHT_PROGBITS;
	text_section->header.sh_flags = SHF_EXECINSTR | SHF_ALLOC;

	/* Create .symtab section and .strtab section */
	symbol_table = elf_enc_symbol_table_create(".symtab", ".strtab");

	/* Create .rodata section */
	rodata_buffer = elf_enc_buffer_create();
	rodata_section = elf_enc_section_create(".rodata", rodata_buffer, rodata_buffer);
	rodata_section->header.sh_type = SHT_PROGBITS;
	rodata_section->header.sh_flags = SHF_ALLOC;

	rodata_size = 0;
	byte = 0;
	
	/* Check if global symbol is needed */
	if (list_count(outer_bin->data_list) != 0)
	{
		LIST_FOR_EACH(outer_bin->data_list, i)
		{
			data = list_get(outer_bin->data_list, i);
			switch (data->data_type)
			{
				case si2bin_data_invalid:
					fatal("Type for .data element %d is not set", i);
					break;

				case si2bin_data_int:
					elf_enc_buffer_write(rodata_buffer, 
						&data->int_value, 
						sizeof(int));
					break;
							
				case si2bin_data_short:
					elf_enc_buffer_write(rodata_buffer, 
						&data->short_value, 
						sizeof(short));
					break;

				case si2bin_data_float:
					elf_enc_buffer_write(rodata_buffer, 
						&data->float_value, 
						sizeof(float));
					break;
				
				case si2bin_data_word:
					elf_enc_buffer_write(rodata_buffer, 
						&data->word_value, 
						sizeof(unsigned int));
					break;
				
				case si2bin_data_half:
					elf_enc_buffer_write(rodata_buffer, 
						&data->half_value, 
						sizeof(unsigned short));
					break;
				
				case si2bin_data_byte:
					elf_enc_buffer_write(rodata_buffer, 
						&data->byte_value, 
						sizeof(unsigned char));
					break;
			}
		}
		
		/* Global section has to have a size that is a multiple of 16 */
		while ((rodata_buffer->offset % 16) != 0)
		{
			elf_enc_buffer_write(rodata_buffer, &byte,
				sizeof(unsigned char));
		}
		
		
		/* Get number of elements to be used later in cb2 */
		glob_size = rodata_buffer->offset / 16;
					
		/* Add global symbol correspoding to data elements */
		snprintf(line, sizeof line, "__OpenCL_%d_global", 2);

		global_symbol = elf_enc_symbol_create(line);
		global_symbol->symbol.st_shndx = 4;
		global_symbol->symbol.st_size = rodata_buffer->offset - rodata_size;
		global_symbol->symbol.st_value = rodata_size;
		global_symbol->symbol.st_info = ELF32_ST_TYPE(STT_OBJECT);
		elf_enc_symbol_table_add(symbol_table, global_symbol);

		rodata_size = rodata_buffer->offset;
	}

	LIST_FOR_EACH(outer_bin->inner_bin_list, i)
	{
		/* Intialize values at each iteration */
		offset = 0;
		ptr_cb_table_found = 0;
		imm_cb_found = 0;

		for (j = 0; j < MAX_UAV_NUM; j++)
			uav[j] = 0;

		for (j = 0; j < MAX_CB_NUM; j++)
			cb[j] = 0;

		kernel_buffer = elf_enc_buffer_create();


		/* Initial Inner ELF settings */

		inner_bin = list_get(outer_bin->inner_bin_list, i);
		metadata = list_get(outer_bin->metadata_list, i);
		entry = list_get(inner_bin->entry_list, 0);

		inner_bin->file->header.e_machine = 0x7d;
		inner_bin->file->header.e_version = 1;
		inner_bin->file->header.e_ident[EI_OSABI] = 0x64;
		inner_bin->file->header.e_ident[EI_ABIVERSION] = 1;

		entry->header.d_type = 4;	/* ???? */

		/* d_machine values based on model */
		switch (outer_bin->device)
		{
			case si2bin_outer_bin_cape_verde:
    				entry->header.d_machine = 28;
				break;

			case si2bin_outer_bin_pitcairn:
				entry->header.d_machine = 27;
				break;

			case si2bin_outer_bin_tahiti:
				entry->header.d_machine = 26;
				break;

			default:
				fatal("%s: unrecognized device type", __FUNCTION__);
		}

		/* Metadata -> .rodata section of Outer ELF */

		/* Print kernel name */
		snprintf(line, sizeof line, ";ARGSTART:__OpenCL_%s_kernel\n", inner_bin->name);
		elf_enc_buffer_write(rodata_buffer, line, strlen(line));

		/* Version */
		snprintf(line, sizeof line, ";version:3:1:104\n");
		elf_enc_buffer_write(rodata_buffer, line, strlen(line));

		/* Device */
		snprintf(line, sizeof line, ";device:%s\n", str_map_value(&si2bin_outer_bin_device_map, outer_bin->device));
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

					data_type =
						str_map_value(&si_arg_data_type_map,
						arg->pointer.data_type);
					data_size =
						si_arg_get_data_size(arg->pointer.
						data_type) * arg->pointer.num_elems;
					scope = str_map_value(&si_arg_scope_map,
						arg->pointer.scope);
					access_type =
						str_map_value(&si_arg_access_type_map,
						arg->pointer.access_type);

					snprintf(line, sizeof line,
						";pointer:%s:%s:1:%d:%d:%s:%d:%d:%s:0:0\n",
						arg->name, data_type,
						arg->pointer.constant_buffer_num,
						arg->pointer.constant_offset, scope,
						arg->pointer.buffer_num, data_size,
						access_type);

					
					/* Check for 16 byte alignment */
					if (((arg->pointer.constant_offset - offset) < 16) && j) 
						fatal("16 byte alignment not maintained in argument: %s - Expected offset of %d or higher", 
							arg->name, offset + 16);

					offset = arg->pointer.constant_offset;
					
					/* Mark which uav's are being used */
					if (arg->pointer.scope == si_arg_uav
						&& !(uav[arg->pointer.buffer_num]))
					{
						uav[arg->pointer.buffer_num] = 1;
					}

					elf_enc_buffer_write(rodata_buffer, line, strlen(line));
					
					/* Include const_arg line only if pointer is marked with "const" */
					if (arg->constarg)
					{
						snprintf(line, sizeof line, ";constarg:%d:%s\n", j, arg->name);
						elf_enc_buffer_write(rodata_buffer, line, strlen(line));
					}

					break;

				case si_arg_value:

					data_type = 
						str_map_value(&si_arg_data_type_map, 
						arg->value.data_type);

					snprintf(line, sizeof line, 
						";value:%s:%s:%d:%d:%d\n", arg->name, data_type, 
						arg->value.num_elems, arg->value.constant_buffer_num, 
						arg->value.constant_offset);

					/* Check for 16 byte alignment */
					if (((arg->value.constant_offset - offset) < 16) && j) 
						fatal("16 byte alignment not maintained in argument: %s - Expected offset of %d or higher",
							arg->name, offset + 16);

					offset = arg->value.constant_offset;

					elf_enc_buffer_write(rodata_buffer, line, strlen(line));
					
					/* Include const_arg line only if pointer is marked with "const" */
					if (arg->constarg)
					{
						snprintf(line, sizeof line, 
							";constarg:%d:%s\n",
							j, arg->name);
						
						elf_enc_buffer_write(rodata_buffer, line, strlen(line));
					}


					break;

				default:
					fatal("Unrecognized argument type: arg %d", j);
			}
		}
		
		/* Data Required */
		if (list_count(outer_bin->data_list) > 0)
		{
			snprintf(line, sizeof line, ";memory:datareqd\n");
			elf_enc_buffer_write(rodata_buffer, line, strlen(line));
		}

		/* Function ID */
		snprintf(line, sizeof line, 
			";function:1:%d\n", metadata->uniqueid + 3);
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

					reflection = str_map_value(&si_arg_reflection_map, 
						arg->pointer.data_type);
					
					if (arg->pointer.num_elems == 1)
					{
						snprintf(line, sizeof line, 
							";reflection:%d:%s*\n",j, reflection);
					}
					else if (arg->pointer.num_elems > 1)
					{
						snprintf(line, sizeof line, 
							";reflection:%d:%s%d*\n", j, reflection,
							arg->pointer.num_elems);
					}
					else
					{
						fatal("Invalid number of elements in argument: %s", arg->name);
					}

					elf_enc_buffer_write(rodata_buffer, line, strlen(line));

					break;

				case si_arg_value:

					reflection = 
						str_map_value(&si_arg_reflection_map,
						arg->value.data_type);
					
					if (arg->value.num_elems == 1)
					{
						snprintf(line, sizeof line, 
							";reflection:%d:%s\n", j, reflection);
					}
					else if (arg->value.num_elems > 1)
					{
						snprintf(line, sizeof line, 
							";reflection:%d:%s%d\n", j, reflection,
							arg->value.num_elems);
					}
					else
					{
						fatal("Invalid number of elements in argument: %s", arg->name);
					}

					elf_enc_buffer_write(rodata_buffer, line, strlen(line));

					break;

				default:
					fatal("Unrecognized argument type: arg %d", j);
			}

		}								

		/* ARGEND */
		snprintf(line, sizeof line, ";ARGEND:__OpenCL_%s_kernel\n",
			inner_bin->name);
		elf_enc_buffer_write(rodata_buffer, line, strlen(line));

		/* Create metadata symbol and store it */
		snprintf(line, sizeof line, "__OpenCL_%s_metadata", 
			inner_bin->name);

		metadata_symbol = elf_enc_symbol_create(line);
		metadata_symbol->symbol.st_shndx = 4;
		metadata_symbol->symbol.st_size = rodata_buffer->offset - rodata_size;
		metadata_symbol->symbol.st_value = rodata_size;
		metadata_symbol->symbol.st_info = ELF32_ST_TYPE(STT_OBJECT);
		elf_enc_symbol_table_add(symbol_table, metadata_symbol);

		/* Increment rodata size */
		rodata_size = rodata_buffer->offset;



		/* Create header symbol and store it */
		snprintf(line, sizeof line, "__OpenCL_%s_header", 
			inner_bin->name);

		header_symbol = elf_enc_symbol_create(line);
		header_symbol->symbol.st_shndx = 4;
		header_symbol->symbol.st_size = 32;
		header_symbol->symbol.st_value = rodata_size;
		header_symbol->symbol.st_info = ELF32_ST_TYPE(STT_OBJECT);
		elf_enc_symbol_table_add(symbol_table, header_symbol);

		/* Create header - Header is not always set the way it is here but
		 * changing it does not seem to affect the program
		 */

		ptr = xcalloc(1, 32);
		ptr[20] = 1;
		elf_enc_buffer_write(rodata_buffer, ptr, 32);
		free(ptr);

		rodata_size = rodata_buffer->offset;


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

		for (k = 0; k < MAX_UAV_NUM; k++)
		{
			if (uav[k])
				buff_num_offset++;
		}


		/* ELF_NOTE_ATI_UAV */
		buff_size = 16 * buff_num_offset;
		ptr = xcalloc(1, buff_size);
		
		buff_num_offset = 0;
		
		/* UAV Symbols */
		for (k = 0; k < MAX_UAV_NUM; k++)
		{
			if (!uav[k])
				continue;
		
			snprintf(line, sizeof line, "uav%d", k);
			uav_symbol = elf_enc_symbol_create(line);
			uav_symbol->symbol.st_value = buff_num_offset;
			uav_symbol->symbol.st_shndx = 16;
			elf_enc_symbol_table_add(entry->symbol_table, uav_symbol);

			ptr[buff_num_offset * 16] = k;
			ptr[buff_num_offset * 16 + 4] = 4;
			ptr[buff_num_offset * 16 + 12] = 5;

			buff_num_offset++;
		}
	
		note = si2bin_inner_bin_note_create(16, buff_size, ptr);
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
		
		/* Check for non-contiguous entries in user element list*/
		k = 0;
		LIST_FOR_EACH(inner_bin->user_element_list, k)
		{
			if(!(list_get(inner_bin->user_element_list, k)))
				fatal("userElement[%d] missing", k);
		}
		

		buff_num_offset = 0;

		for (k = 0; k < list_count(inner_bin->user_element_list); k++)
		{
			user_elem = list_get(inner_bin->user_element_list, k);
			
			if (user_elem->dataClass == IMM_CONST_BUFFER)
			{
				/* Cannot have both IMM_CONST_BUFFER and PTR_CONST_BUFFER_TABLE */
				if (ptr_cb_table_found)
					fatal("Cannot have both IMM_CONST_BUFFER and PTR_CONST_BUFFER_TABLE");	
				
				cb[user_elem->apiSlot] = 1;

				buff_num_offset++;
				imm_cb_found = 1;
			}
			else if (user_elem->dataClass == PTR_CONST_BUFFER_TABLE)
			{
				/* Cannot have both IMM_CONST_BUFFER and PTR_CONST_BUFFER_TABLE */
				if (imm_cb_found)
					fatal("Cannot have both IMM_CONST_BUFFER and PTR_CONST_BUFFER_TABLE");	
			
				/* If PTR_CONST_BUFFER_TABLE is found, manually set cb's */
				cb[0] = 1;
				
				if (list_count(metadata->arg_list) > 0)
					cb[1] = 1;
				
				if (list_count(outer_bin->data_list) > 0)
					cb[2] = 1;

				buff_num_offset = cb[0] + cb[1] + cb[2];
				ptr_cb_table_found = 1;
			}
		}

		/* Check if data values have been added without using constant buffer 2 */
		if ( (!cb[2]) &&  (list_count(outer_bin->data_list) > 0) )
			fatal("Data values have been added but constant buffer 2 has not been specified");
		
		
		buff_size = 8 * buff_num_offset;
		ptr = xcalloc(1, (buff_size));
		
		/* CB Symbols */
		for (k = 0; k < MAX_CB_NUM; k++)
		{
			if (cb[k])
			{
				snprintf(line, sizeof line, "cb%d", k);

				cb_symbol = elf_enc_symbol_create(line);
				cb_symbol->symbol.st_value = buff_num_offset - 1;
				cb_symbol->symbol.st_shndx = 10;
				elf_enc_symbol_table_add(entry->symbol_table, cb_symbol);
				
			
				ptr[(buff_num_offset - 1) * 8] = k;
			
				if (k == 0)
					ptr[(buff_num_offset - 1) * 8 + 4] = 0xf;

				if (k == 1)
					ptr[(buff_num_offset - 1) * 8 + 4] = 
						list_count(metadata->arg_list);
				
				if (k == 2)
				{
					if (list_count(outer_bin->data_list) == 0)
						fatal("%s: Constant Buffer 2 is used but nothing has been added to a global symbol",
							 __FUNCTION__);

					ptr[(buff_num_offset - 1) * 8 + 4] = glob_size;
				}

				buff_num_offset--;
			}

		}
		
		note = si2bin_inner_bin_note_create(10, buff_size, ptr);
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
		
		/* AMU_ABI_USER_ELEMENT_COUNT */
		prog_info[0].address = 0x80001000;
		prog_info[0].value = list_count(inner_bin->user_element_list);
		
		/* AMU_ABI_USER_ELEMENTS (16 maximum) */
		for (k = 0; k < 16; k++)
		{
			prog_info[(k * 4) + 1].address = 0x80001000 + (k * 4) + 1;
			prog_info[(k * 4) + 2].address = 0x80001000 + (k * 4) + 2;
			prog_info[(k * 4) + 3].address = 0x80001000 + (k * 4) + 3;
			prog_info[(k * 4) + 4].address = 0x80001000 + (k * 4) + 4;
			
			if ((list_count(inner_bin->user_element_list) - 1) >= k)
			{
				user_elem = list_get(inner_bin->user_element_list, k);

				prog_info[(k * 4) + 1].value = user_elem->dataClass;
				prog_info[(k * 4) + 2].value = user_elem->apiSlot;
				prog_info[(k * 4) + 3].value = user_elem->startUserReg;
				prog_info[(k * 4) + 4].value = user_elem->userRegCount;
		
			}
			else
			{
				prog_info[(k * 4) + 1].value = 0;
				prog_info[(k * 4) + 2].value = 0;
				prog_info[(k * 4) + 3].value = 0;
				prog_info[(k * 4) + 4].value = 0;
			}
		}
				
		
	
		/* AMU_ABI_SI_NUM_VGPRS */
		prog_info[65].address = 0x80001041;
		prog_info[65].value = inner_bin->num_vgprs;
		
		/* AMU_ABI_SI_NUM_SGPRS */
		prog_info[66].address = 0x80001042;	
		prog_info[66].value = inner_bin->num_sgprs;;
		
		/* AMU_ABI_SI_NUM_SGPRS_AVAIL */
		prog_info[67].address = 0x80001863;
		prog_info[67].value = 0x66;

		/* AMU_ABI_SI_NUM_VGPRS_ AVAIL */
		prog_info[68].address = 0x80001864;
		prog_info[68].value = 0x100;

		/* AMU_ABI_SI_FLOAT_MODE */
		prog_info[69].address = 0x80001043;
		prog_info[69].value = inner_bin->FloatMode;

		/* AU_ABI_SI_IEEE_MODE */
		prog_info[70].address = 0x80001044;
		prog_info[70].value = inner_bin->IeeeMode;

		prog_info[71].address = 0x80001045;
		prog_info[71].value = 0;
		
		/* COMPUTE_PGM_RSRC2 */
		prog_info[72].address = 0x00002e13;
		prog_info[72].value = *((int*)inner_bin->pgm_rsrc2);


		/* AMU_ABI_NUM_THREAD_PER_GROUP_X */
		prog_info[73].address = 0x8000001c;
		prog_info[73].value = 0x100;

		prog_info[74].address = 0x8000001d;
		prog_info[74].value = 0;
		prog_info[75].address = 0x8000001e;
		prog_info[75].value = 0;
		prog_info[76].address = 0x80001841;
		prog_info[76].value = 0;
		
		/* AMU_ABI_RAT_OP_IS_USED */
		prog_info[77].address = 0x8000001f;
		prog_info[77].value = inner_bin->rat_op;

		/* AMU_ABI_UAV_RESOURCE_MASK_0 */
		prog_info[78].address = 0x80001843;
		prog_info[78].value = inner_bin->rat_op;
		
		prog_info[79].address = 0x80001844;
		prog_info[79].value = 0;
		prog_info[80].address = 0x80001845;
		prog_info[80].value = 0;
		prog_info[81].address = 0x80001846;
		prog_info[81].value = 0;
		prog_info[82].address = 0x80001847;
		prog_info[82].value = 0;
		prog_info[83].address = 0x80001848;
		prog_info[83].value = 0;
		prog_info[84].address = 0x80001849;
		prog_info[84].value = 0;
		prog_info[85].address = 0x8000184a;
		prog_info[85].value = 0;
		prog_info[86].address = 0x8000184b;
		prog_info[86].value = 0;
		prog_info[87].address = 0x8000184c;
		prog_info[87].value = 0;
		prog_info[88].address = 0x8000184d;
		prog_info[88].value = 0;
		prog_info[89].address = 0x8000184e;
		prog_info[89].value = 0;
		prog_info[90].address = 0x8000184f;
		prog_info[90].value = 0;
		prog_info[91].address = 0x80001850;
		prog_info[91].value = 0;
		prog_info[92].address = 0x80001851;
		prog_info[92].value = 0;
		prog_info[93].address = 0x80001852;
		prog_info[93].value = 0;
		prog_info[94].address = 0x80001853;
		prog_info[94].value = 0;
		prog_info[95].address = 0x80001854;
		prog_info[95].value = 0;
		prog_info[96].address = 0x80001855;
		prog_info[96].value = 0;
		prog_info[97].address = 0x80001856;
		prog_info[97].value = 0;
		prog_info[98].address = 0x80001857;
		prog_info[98].value = 0;
		prog_info[99].address = 0x80001858;
		prog_info[99].value = 0;
		prog_info[100].address = 0x80001859;
		prog_info[100].value = 0;
		prog_info[101].address = 0x8000185a;
		prog_info[101].value = 0;
		prog_info[102].address = 0x8000185b;
		prog_info[102].value = 0;
		prog_info[103].address = 0x8000185c;
		prog_info[103].value = 0;
		prog_info[104].address = 0x8000185d;
		prog_info[104].value = 0;
		prog_info[105].address = 0x8000185e;
		prog_info[105].value = 0;
		prog_info[106].address = 0x8000185f;
		prog_info[106].value = 0;
		prog_info[107].address = 0x80001860;
		prog_info[107].value = 0;
		prog_info[108].address = 0x80001861;
		prog_info[108].value = 0;
		prog_info[109].address = 0x80001862;
		prog_info[109].value = 0;

		/* AMU_ABI_NUM_WAVEFRONT_PER_SIMD */
		prog_info[110].address = 0x8000000a;
		prog_info[110].value = 1;
		
		/* AMU_ABI_WAVEFRONT_SIZE - Set to 64*/
		prog_info[111].address = 0x80000078;
		prog_info[111].value = 0x40;

		/* AMU_ABI_LDS_SIZE_AVAIL */
		prog_info[112].address = 0x80000081;
		prog_info[112].value = 0x8000;

		/* AMU_ABI_LDS_SIZE_USED */
		prog_info[113].address = 0x80000082;
		prog_info[113].value = inner_bin->pgm_rsrc2->lds_size * 256;

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
		
		*((int*)ptr) = inner_bin->rat_op;

		note = si2bin_inner_bin_note_create(17, 128, ptr);
		si2bin_inner_bin_entry_add_note(entry, note);
		free(ptr);

		/* Data Section - Not supported yet (section is empty right now) */
		ptr = xcalloc(1, 4736);
		elf_enc_buffer_write(entry->data_section_buffer, ptr, 4736);
		free(ptr);


		/* Generate Inner Bin File */
		si2bin_inner_bin_generate(inner_bin, kernel_buffer);
			
		/* Output Inner ELF */
		/* FILE *f;
		snprintf(line, sizeof line, "%s_kernel", inner_bin->name);
		f = file_open_for_write(line);
		elf_enc_buffer_write_to_file(kernel_buffer, f);
		file_close(f); */

	
		/* Create kernel symbol and add it to the symbol table */
		snprintf(line, sizeof line, "__OpenCL_%s_kernel", inner_bin->name);

		kernel_symbol = elf_enc_symbol_create(line);
		kernel_symbol->symbol.st_shndx = 5;
		kernel_symbol->symbol.st_size = kernel_buffer->offset;
		kernel_symbol->symbol.st_value = text_buffer->offset;
		kernel_symbol->symbol.st_info =  ELF32_ST_TYPE(STT_FUNC);
		elf_enc_symbol_table_add(symbol_table, kernel_symbol);


		elf_enc_buffer_write(text_buffer, kernel_buffer->ptr,
			kernel_buffer->offset);


		elf_enc_buffer_free(kernel_buffer);

	}
	
	/* Set e_machine value on outer elf */
	switch (outer_bin->device)
	{
		case si2bin_outer_bin_cape_verde:
			outer_bin->file->header.e_machine = 0x3ff;
			break;

		case si2bin_outer_bin_pitcairn:
			outer_bin->file->header.e_machine = 0x3fe;
			break;

		case si2bin_outer_bin_tahiti:
			outer_bin->file->header.e_machine = 0x3fd;
			break;

		default:
			fatal("%s: unrecognized device type", __FUNCTION__);
	}
        
	outer_bin->file->header.e_version = 1;

	/* Add symbol table to outer elf */
        elf_enc_file_add_symbol_table(outer_bin->file, symbol_table);
	
	/* Create and rodata section and add it to outer elf */
        elf_enc_file_add_buffer(outer_bin->file, rodata_buffer);
        elf_enc_file_add_section(outer_bin->file, rodata_section);

	/* Create text section and add it to outer elf */
        elf_enc_file_add_buffer(outer_bin->file, text_buffer);
        elf_enc_file_add_section(outer_bin->file, text_section);

	/* Generate final binary */
        elf_enc_file_generate(outer_bin->file, buffer);
}

