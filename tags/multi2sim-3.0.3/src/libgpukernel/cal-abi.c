/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal (ubal@gap.upv.es)
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

#include <gpukernel-local.h>


#define CAL_ABI_NOT_SUPPORTED(__var) \
	fatal("kernel binary parser: value 0x%x not supported for parameter '" #__var "'", (__var))
#define CAL_ABI_NOT_SUPPORTED_NEQ(__var, __val) \
	if ((__var) != (__val)) \
	fatal("kernel binary parser: parameter '" #__var "' was expected to be 0x%x", (__val))


typedef struct {
	Elf32_Word d_machine;
	Elf32_Word d_type;
	Elf32_Off d_offset;  /* Offset for encoding data (PT_NOTE + PT_LOAD segments) */
	Elf32_Word d_size;  /* Size of encoding data (PT_NOTE + PT_LOAD segments) */
	Elf32_Word d_flags;
} CALEncodingDictionaryEntry;


typedef struct {
	Elf32_Word namesz;  /* Size of the name field. Must be 8 */
	Elf32_Word descsz;  /* Size of the data payload */
	Elf32_Word type;  /* Type of the payload */
	char name[8];  /* Note header string. Must be "ATI CAL" */
} CALNoteHeader;


struct string_map_t pt_note_type_map = {
	17, {
		{ "ELF_NOTE_ATI_PROGINFO", 1 },
		{ "ELF_NOTE_ATI_INPUTS", 2 },
		{ "ELF_NOTE_ATI_OUTPUTS", 3 },
		{ "ELF_NOTE_ATI_CONDOUT", 4 },
		{ "ELF_NOTE_ATI_FLOAT32CONSTS", 5 },
		{ "ELF_NOTE_ATI_INT32CONSTS", 6 },
		{ "ELF_NOTE_ATI_BOOL32CONSTS", 7 },
		{ "ELF_NOTE_ATI_EARLYEXIT", 8 },
		{ "ELF_NOTE_ATI_GLOBAL_BUFFERS", 9 },
		{ "ELF_NOTE_ATI_CONSTANT_BUFFERS", 10 },
		{ "ELF_NOTE_ATI_INPUT_SAMPLERS", 11 },
		{ "ELF_NOTE_ATI_PERSISTENT_BUFFERS", 12 },
		{ "ELF_NOTE_ATI_SCRATCH_BUFFERS", 13 },
		{ "ELF_NOTE_ATI_SUB_CONSTANT_BUFFERS", 14 },
		{ "ELF_NOTE_ATI_UAV_MAILBOX_SIZE", 15 },
		{ "ELF_NOTE_ATI_UAV", 16 },
		{ "ELF_NOTE_ATI_UAV_OP_MASK", 17 }
	}
};


typedef struct {
	Elf32_Word offset;  /* Offsets in bytes to start of data */
	Elf32_Word size;  /* Size in bytes of data */
} CALDataSegmentDesc;


typedef struct {
	Elf32_Word index;  /* Constant buffer identfier */
	Elf32_Word size;  /* Size in vec4f constants of the buffer */
} CALConstantBufferMask;


typedef struct {
	Elf32_Word address;  /* Device address */
	Elf32_Word value;  /* Value */
} CALProgramInfoEntry;


void cal_abi_analyze_note_header(struct cal_abi_t *cal_abi, void *pt_note_ptr)
{
	char *pt_note_type;
	CALNoteHeader *pt_note_header = pt_note_ptr;

	debug_tab(opencl_debug_category, 4);
	pt_note_type = map_value(&pt_note_type_map, pt_note_header->type);
	opencl_debug("pt_note: type=%d (%s), descsz=%d\n", pt_note_header->type,
		pt_note_type, pt_note_header->descsz);
		
	/* Analyze note */
	switch (pt_note_header->type) {

	case 1:  /* ELF_NOTE_ATI_PROGINFO */
	{
		int prog_info_count;
		CALProgramInfoEntry *prog_info_entry;
		int i;

		/* Get number of entries */
		prog_info_count = pt_note_header->descsz / sizeof(CALProgramInfoEntry);
		opencl_debug("Note including device configuration unique to the program (%d entries)\n",
			prog_info_count);

		/* Decode entries */
		for (i = 0; i < prog_info_count; i++) {
			prog_info_entry = pt_note_ptr + sizeof(CALNoteHeader) + i * sizeof(CALProgramInfoEntry);
			opencl_debug("  prog_info_entry: address=0x%x, value=0x%x\n",
				prog_info_entry->address, prog_info_entry->value);
		}
		break;
	}

	
	case 2:  /* ELF_NOTE_ATI_INPUTS */
	{
		/* FIXME: Analyze program inputs */
		if (pt_note_header->descsz)
			warning("%s: pt_note '%s' with descsz != 0 ignored", pt_note_type, __FUNCTION__);
		break;
	}


	case 3:  /* ELF_NOTE_ATI_OUTPUTS */
	{
		/* FIXME: Analyze program inputs */
		if (pt_note_header->descsz)
			warning("%s: pt_note '%s' with descsz != 0 ignored", pt_note_type, __FUNCTION__);
		break;
	}

	
	case 4:  /* ELF_NOTE_ATI_CONDOUT */
		break;
	
	case 5:  /* ELF_NOTE_ATI_FLOAT32CONSTS */
	case 6:  /* ELF_NOTE_ATI_INT32CONSTS */
	case 7:  /* ELF_NOTE_ATI_BOOL32CONSTS */
	{
		int data_segment_desc_count;
		CALDataSegmentDesc *data_segment_desc;
		int j, k;
		uint32_t c;
		float f;

		/* Get number of entries */
		data_segment_desc_count = pt_note_header->descsz / sizeof(CALDataSegmentDesc);
		opencl_debug("Note including data for constant buffers (%d entries)\n",
				data_segment_desc_count);

		/* Decode entries */
		for (j = 0; j < data_segment_desc_count; j++) {
			data_segment_desc = pt_note_ptr + sizeof(CALNoteHeader) + j * sizeof(CALDataSegmentDesc);
			opencl_debug("data_segment_desc[%d]:\n", j);
			debug_tab_inc(opencl_debug_category, 2);
			opencl_debug("offset  = 0x%x\n", data_segment_desc->offset);
			opencl_debug("size    = %u\n", data_segment_desc->size);

			/* Dump constants */
			debug_tab_inc(opencl_debug_category, 2);
			for (k = 0; k < data_segment_desc->size; k += 4) {
				c = * (uint32_t *) (cal_abi->data_buffer + data_segment_desc->offset);
				f = * (float *) (cal_abi->data_buffer + data_segment_desc->offset);
				opencl_debug("constant[%02d] = 0x%08x, %gf\n", k / 4, c, f);
			}
			debug_tab_dec(opencl_debug_category, 2);
			debug_tab_dec(opencl_debug_category, 2);
		}
		break;
	}

	
	case 8:  /* ELF_NOTE_ATI_EARLYEXIT */
	{
		Elf32_Word early_exit;

		/* Get 'early_exit' value */
		early_exit = pt_note_header->descsz ? * (unsigned int *) (pt_note_ptr + sizeof(CALNoteHeader)) : 0;
		opencl_debug("  early_exit = %s\n", early_exit ? "TRUE" : "FALSE");
		break;
	}

	
	case 9:  /* ELF_NOTE_ATI_GLOBAL_BUFFERS */
	{
		Elf32_Word global_buffers;
		global_buffers = pt_note_header->descsz ? * (unsigned int *) (pt_note_ptr + sizeof(CALNoteHeader)) : 0;
		opencl_debug("  global_buffers = %s\n", global_buffers ? "TRUE" : "FALSE");
		break;
	}
	
	
	case 10:  /* ELF_NOTE_ATI_CONSTANT_BUFFERS */
	{
		int constant_buffer_count;
		CALConstantBufferMask *constant_buffer_mask;
		int i;

		/* Get number of entries */
		constant_buffer_count = pt_note_header->descsz / sizeof(CALConstantBufferMask);
		debug_tab_inc(opencl_debug_category, 2);
		opencl_debug("Note including number and size of constant buffers (%d entries)\n",
			constant_buffer_count);

		/* Decode entries */
		for (i = 0; i < constant_buffer_count; i++) {
			constant_buffer_mask = pt_note_ptr + sizeof(CALNoteHeader) + i * sizeof(CALConstantBufferMask);
			opencl_debug("constant_buffer[%d].size = %d (vec4f constants)\n",
				constant_buffer_mask->index, constant_buffer_mask->size);
		}
		debug_tab_dec(opencl_debug_category, 2);
		break;
	}

	
	case 11:  /* ELF_NOTE_ATI_INPUT_SAMPLERS */
		break;
	
	case 12:  /* ELF_NOTE_ATI_PERSISTENT_BUFFERS */
	{
		Elf32_Word persistent_buffers;
		persistent_buffers = pt_note_header->descsz ? * (unsigned int *) (pt_note_ptr + sizeof(CALNoteHeader)) : 0;
		opencl_debug("  persistent_buffers = %s\n", persistent_buffers ? "TRUE" : "FALSE");
		break;
	}

	
	case 13:  /* ELF_NOTE_ATI_SCRATCH_BUFFERS */
	{
		Elf32_Word scratch_buffers;
		scratch_buffers = pt_note_header->descsz ? * (unsigned int *) (pt_note_ptr + sizeof(CALNoteHeader)) : 0;
		opencl_debug("  scratch_buffers = %s\n", scratch_buffers ? "TRUE" : "FALSE");
		break;
	}

	
	case 14:  /* ELF_NOTE_ATI_SUB_CONSTANT_BUFFERS */
		break;
	
	case 15:  /* ELF_NOTE_ATI_UAV_MAILBOX_SIZE */
		break;
	
	case 16:  /* ELF_NOTE_ATI_UAV */
		break;
	
	case 17:  /* ELF_NOTE_ATI_UAV_OP_MASK */
		break;

	default:
		opencl_debug("unknown type\n");
	}

	/* New line */
	opencl_debug("\n");
}


/* Given an encoding dictionary entry, allocate the corresponding PT_NOTE and PT_LOAD segments
 * into the 'cal_abi' object pointers. */
void cal_abi_read_segments(struct cal_abi_t *cal_abi, CALEncodingDictionaryEntry *enc_dict_entry)
{
	struct elf_file_t *elf = cal_abi->elf;
	int phidx;  /* Program header index */
	Elf32_Phdr *phdr;

	assert(!cal_abi->pt_note_buffer);
	assert(!cal_abi->pt_load_buffer);
	for (phidx = 0; phidx < elf->ehdr.e_phnum; phidx++) {
		
		/* Segment referred by program header must fall in range delimited by encoding dictionary entry */
		phdr = &elf->phdr[phidx];
		if (!IN_RANGE(phdr->p_offset, enc_dict_entry->d_offset, enc_dict_entry->d_offset + enc_dict_entry->d_size - 1))
			continue;

		/* Segment PT_NOTE */
		if (phdr->p_type == PT_NOTE) {
			if (cal_abi->pt_note_buffer)
				fatal("%s: more than one PT_NOTE segment for encoding dictionary entry", __FUNCTION__);
			cal_abi->pt_note_phdr = phdr;
			cal_abi->pt_note_buffer = elf_read_buffer(elf, phdr->p_offset, phdr->p_filesz);
		}

		/* Segment PT_LOAD */
		if (phdr->p_type == PT_LOAD) {
			if (cal_abi->pt_load_buffer)
				fatal("%s: more than one PT_LOAD segment for encoding dictionary entry", __FUNCTION__);
			cal_abi->pt_load_phdr = phdr;
			cal_abi->pt_load_buffer = elf_read_buffer(elf, phdr->p_offset, phdr->p_filesz);
		}
	}

	/* Check that both PT_NOTE and PT_LOAD segments were found */
	if (!cal_abi->pt_note_buffer)
		fatal("%s: no PT_NOTE segment found for encoding dictionary entry", __FUNCTION__);
	if (!cal_abi->pt_load_buffer)
		fatal("%s: no PT_LOAD segment found for encoding dictionary entry", __FUNCTION__);
}


/* Read the PT_LOAD segment and assign pointers to the different sections. */
void cal_abi_read_pt_load_sections(struct cal_abi_t *cal_abi)
{
	struct elf_file_t *elf = cal_abi->elf;
	char *section_name;
	void *section_buffer;
	Elf32_Shdr *shdr;
	int i;

	assert(!cal_abi->text_buffer);
	assert(!cal_abi->data_buffer);
	assert(!cal_abi->symtab_buffer);
	assert(!cal_abi->strtab_buffer);

	/* Find sections */
	for (i = 0; i < elf_section_count(elf); i++) {
		
		/* Section must be in segment */
		shdr = &elf->shdr[i];
		elf_section_info(elf, i, &section_name, NULL, NULL, NULL);
		if (!IN_RANGE(shdr->sh_offset, cal_abi->pt_load_phdr->p_offset, cal_abi->pt_load_phdr->p_offset
			+ cal_abi->pt_load_phdr->p_filesz - 1))
			continue;

		/* Locate section within 'pt_load_buffer'. The relative offset is equal to the absolute section offset
		 * minus the PT_LOAD segment offset. */
		section_buffer = cal_abi->pt_load_buffer + shdr->sh_offset - cal_abi->pt_load_phdr->p_offset;

		/* Which section is this */
		if (!strcmp(section_name, ".text")) {
			if (cal_abi->text_buffer)
				fatal("%s: duplicated '.text' section", __FUNCTION__);
			cal_abi->text_shdr = shdr;
			cal_abi->text_buffer = section_buffer;
		} else if (!strcmp(section_name, ".data")) {
			if (cal_abi->data_buffer)
				fatal("%s: duplicated '.data' section", __FUNCTION__);
			cal_abi->data_shdr = shdr;
			cal_abi->data_buffer = section_buffer;
		} else if (!strcmp(section_name, ".symtab")) {
			if (cal_abi->symtab_buffer)
				fatal("%s: duplicated '.symtab' section", __FUNCTION__);
			cal_abi->symtab_shdr = shdr;
			cal_abi->symtab_buffer = section_buffer;
		} else if (!strcmp(section_name, ".strtab")) {
			if (cal_abi->strtab_buffer)
				fatal("%s: duplicated '.strtab' section", __FUNCTION__);
			cal_abi->strtab_shdr = shdr;
			cal_abi->strtab_buffer = section_buffer;
		} else
			fatal("%s: not recognized section name: '%s'", __FUNCTION__, section_name);
	}

	/* Check that all sections were found */
	if (!cal_abi->text_buffer || !cal_abi->data_buffer || !cal_abi->symtab_buffer || !cal_abi->strtab_buffer)
		fatal("%s: some section was not found: .text .data .symtab .strtab", __FUNCTION__);
}


void cal_abi_parse_elf(struct cal_abi_t *cal_abi, char *file_name)
{
	/* Encoding dictionary entries */
	CALEncodingDictionaryEntry *enc_dict_entries, *enc_dict_entry;

	/* Individual pt_notes in the PT_NOTE segment */
	void *pt_note_ptr;
	CALNoteHeader *pt_note_header;

	int i;
	struct elf_file_t *elf;

	/* Open ELF file */
	strncpy(cal_abi->file_name, file_name, MAX_PATH_SIZE);
	opencl_debug("CAL ABI analyzer: parsing file '%s'\n", cal_abi->file_name);
	cal_abi->elf = elf = elf_open(cal_abi->file_name);
	
	/* ELF header */
	CAL_ABI_NOT_SUPPORTED_NEQ(elf->ehdr.e_ident[EI_CLASS], ELFCLASS32);
	CAL_ABI_NOT_SUPPORTED_NEQ(elf->ehdr.e_ident[EI_DATA], ELFDATA2LSB);
	CAL_ABI_NOT_SUPPORTED_NEQ(elf->ehdr.e_ident[EI_OSABI], 0x64);
	CAL_ABI_NOT_SUPPORTED_NEQ(elf->ehdr.e_ident[EI_ABIVERSION], 1);
	CAL_ABI_NOT_SUPPORTED_NEQ(elf->ehdr.e_type, ET_EXEC);
	CAL_ABI_NOT_SUPPORTED_NEQ(elf->ehdr.e_machine, 0x7d);
	CAL_ABI_NOT_SUPPORTED_NEQ(elf->ehdr.e_entry, 0);

	/* Look for encoding dictionary */
	opencl_debug("  Parsing encoding dictionary\n");
	for (cal_abi->enc_dict_phdr_idx = 0;
		cal_abi->enc_dict_phdr_idx < elf->ehdr.e_phnum && elf->phdr[cal_abi->enc_dict_phdr_idx].p_type != PT_LOPROC + 2;
		cal_abi->enc_dict_phdr_idx++);
	if (cal_abi->enc_dict_phdr_idx == elf->ehdr.e_phnum)
		CAL_ABI_NOT_SUPPORTED(cal_abi->enc_dict_phdr_idx);
	cal_abi->enc_dict_phdr = &elf->phdr[cal_abi->enc_dict_phdr_idx];

	/* Parse encoding dictionary */
	CAL_ABI_NOT_SUPPORTED_NEQ(cal_abi->enc_dict_phdr->p_vaddr, 0);
	CAL_ABI_NOT_SUPPORTED_NEQ(cal_abi->enc_dict_phdr->p_paddr, 0);
	CAL_ABI_NOT_SUPPORTED_NEQ(cal_abi->enc_dict_phdr->p_memsz, 0);
	CAL_ABI_NOT_SUPPORTED_NEQ(cal_abi->enc_dict_phdr->p_flags, 0);
	CAL_ABI_NOT_SUPPORTED_NEQ(cal_abi->enc_dict_phdr->p_align, 0);
	cal_abi->enc_dict_entry_count = cal_abi->enc_dict_phdr->p_filesz / sizeof(CALEncodingDictionaryEntry);
	opencl_debug("%d entries\n", cal_abi->enc_dict_entry_count);
	
	/* Read encoding dictionary entry and locate that entry with 'd_machine'=0x9 */
	enc_dict_entries = elf_read_buffer(elf, cal_abi->enc_dict_phdr->p_offset, cal_abi->enc_dict_phdr->p_filesz);
	cal_abi->enc_dict_entry_idx = -1;
	for (i = 0; i < cal_abi->enc_dict_entry_count; i++) {
		
		/* Decode entry */
		enc_dict_entry = &enc_dict_entries[i];
		debug_tab(opencl_debug_category, 2);
		opencl_debug("Encoding dictionary entry %d:\n", i);
		debug_tab(opencl_debug_category, 4);
		opencl_debug("d_machine = 0x%x\n", enc_dict_entry->d_machine);
		opencl_debug("d_type    = 0x%x\n", enc_dict_entry->d_type);
		opencl_debug("d_offset  = 0x%x\n", enc_dict_entry->d_offset);
		opencl_debug("d_size    = 0x%x\n", enc_dict_entry->d_size);
		opencl_debug("d_flags   = 0x%x\n", enc_dict_entry->d_flags);

		/* Check if 'd_machine' entry is 0x9 */
		if (enc_dict_entry->d_machine == 0x9) {
			if (cal_abi->enc_dict_entry_idx >= 0)
				fatal("%s: two encoding dictionary entries with 'd_machine' = 0x9", __FUNCTION__);
			cal_abi->enc_dict_entry_idx = i;
		}
	}

	/* Check that encoding dictionary entry with 'd_machine'=0x9 was found */
	if (cal_abi->enc_dict_entry_idx < 0)
		fatal("%s: no encoding dictionary entry with 'd_machine'=0x9 found", __FUNCTION__);
	enc_dict_entry = &enc_dict_entries[cal_abi->enc_dict_entry_idx];
	debug_tab(opencl_debug_category, 2);
	opencl_debug("\n");
	opencl_debug("Encoding dictionary entry selected for loading: %d\n", cal_abi->enc_dict_entry_idx);

	/* Load PT_NOTE and PT_LOAD segments for selected encoding dictionary entry.
	 * Also get pointers to sections within PT_LOAD segment */
	cal_abi_read_segments(cal_abi, enc_dict_entry);
	cal_abi_read_pt_load_sections(cal_abi);

	/* Decode notes in PT_NOTE segment */
	pt_note_ptr = cal_abi->pt_note_buffer;
	while (pt_note_ptr < cal_abi->pt_note_buffer + cal_abi->pt_note_phdr->p_filesz) {
		cal_abi_analyze_note_header(cal_abi, pt_note_ptr);
		pt_note_header = pt_note_ptr;
		pt_note_ptr += sizeof(CALNoteHeader) + pt_note_header->descsz;
	}

	/* Check that the encoding dictionary entry with d_machine=0x9 was found */
	if (!cal_abi->text_buffer)
		fatal("%s: no entry with d_machine=0x9 found", __FUNCTION__);

	/* Free structures */
	elf_free_buffer(enc_dict_entries);
	debug_tab(opencl_debug_category, 0);
}


/* Create CAL ABI object */
struct cal_abi_t *cal_abi_create()
{
	struct cal_abi_t *cal_abi;

	cal_abi = calloc(1, sizeof(struct cal_abi_t));
	return cal_abi;
}


void cal_abi_free(struct cal_abi_t *cal_abi)
{
	if (cal_abi->elf) {
		elf_free_buffer(cal_abi->pt_note_buffer);
		elf_free_buffer(cal_abi->pt_load_buffer);
		elf_close(cal_abi->elf);
	}
	free(cal_abi);
}

