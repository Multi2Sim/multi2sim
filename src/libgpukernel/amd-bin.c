/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <gpukernel.h>
	
#define AMD_BIN_NOT_SUPPORTED(__var) \
	fatal("%s: value 0x%x not supported for parameter '" #__var "'", __FUNCTION__, (__var))
#define AMD_BIN_NOT_SUPPORTED_NEQ(__var, __val) \
	if ((__var) != (__val)) \
	fatal("%s: parameter '" #__var "' was expected to be 0x%x", __FUNCTION__, (__val))


/* Note header */
struct pt_note_header_t {
	Elf32_Word namesz;  /* Size of the name field. Must be 8 */
	Elf32_Word descsz;  /* Size of the data payload */
	Elf32_Word type;  /* Type of the payload */
	char name[8];  /* Note header string. Must be "ATI CAL" */
};


static struct string_map_t pt_note_type_map = {
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


struct pt_note_data_segment_desc_t {
	Elf32_Word offset;  /* Offsets in bytes to start of data */
	Elf32_Word size;  /* Size in bytes of data */
};


struct pt_note_constant_buffer_mask_t {
	Elf32_Word index;  /* Constant buffer identfier */
	Elf32_Word size;  /* Size in vec4f constants of the buffer */
};


struct pt_note_prog_info_entry_t {
	Elf32_Word address;  /* Device address */
	Elf32_Word value;  /* Value */
};


/* Read next note at the current position of the PT_NOTE segment */
static void amd_bin_read_note_header(struct amd_bin_t *amd_bin, struct amd_bin_enc_dict_entry_t *enc_dict_entry)
{
	struct elf_buffer_t *buffer;

	struct pt_note_header_t *header;
	void *desc;

	int count;
	char *note_type_str;

	/* Read note header */
	buffer = &enc_dict_entry->pt_note_buffer;
	header = elf_buffer_tell(buffer);
	count = elf_buffer_read(buffer, NULL, sizeof(struct pt_note_header_t));
	if (count < sizeof(struct pt_note_header_t))
		fatal("%s: error decoding note header", amd_bin->elf_file->path);
	
	/* Read note description */
	desc = elf_buffer_tell(buffer);
	count = elf_buffer_read(buffer, NULL, header->descsz);
	if (count < header->descsz)
		fatal("%s: error decoding note description", amd_bin->elf_file->path);

	/* Debug */
	note_type_str = map_value(&pt_note_type_map, header->type);
	elf_debug("  note: type=%d (%s), descsz=%d\n",
		header->type, note_type_str, header->descsz);
		
	/* Analyze note */
	switch (header->type) {

	case 1:  /* ELF_NOTE_ATI_PROGINFO */
	{
		int prog_info_count;
		struct pt_note_prog_info_entry_t *prog_info_entry;
		int i;

		/* Get number of entries */
		assert(header->descsz % sizeof(struct pt_note_prog_info_entry_t) == 0);
		prog_info_count = header->descsz / sizeof(struct pt_note_prog_info_entry_t);
		elf_debug("\tnote including device configuration unique to the program (%d entries)\n",
			prog_info_count);

		/* Decode entries */
		for (i = 0; i < prog_info_count; i++) {
			prog_info_entry = desc + i * sizeof(struct pt_note_prog_info_entry_t);
			elf_debug("\tprog_info_entry: addr=0x%x, value=0x%x: ",
				prog_info_entry->address, prog_info_entry->value);

			/* Analyze entry */
			switch (prog_info_entry->address) {

			case 0x80000080:  /* AMU_ABI_NUM_GPR_USED */
				enc_dict_entry->num_gpr_used = prog_info_entry->value;
				elf_debug("AMU_ABI_NUM_GPR_USED = %d\n", enc_dict_entry->num_gpr_used);
				break;

			case 0x80000082:  /* AMU_ABI_LDS_SIZE_USED */
				enc_dict_entry->lds_size_used = prog_info_entry->value;
				elf_debug("AMU_ABI_LDS_SIZE_USED = %d\n", enc_dict_entry->lds_size_used);
				break;

			case 0x80000084:  /* AMU_ABI_STACK_SIZE_USED */
				enc_dict_entry->stack_size_used = prog_info_entry->value;
				elf_debug("AMU_ABI_STACK_SIZE_USED = %d\n", enc_dict_entry->stack_size_used);
				break;

			default:
				elf_debug("?\n");
			}
		}
		break;
	}

	
	case 2:  /* ELF_NOTE_ATI_INPUTS */
	{
		/* FIXME: Analyze program inputs */
		if (header->descsz)
			warning("%s: pt_note '%s' with descsz != 0 ignored", note_type_str, __FUNCTION__);
		break;
	}


	case 3:  /* ELF_NOTE_ATI_OUTPUTS */
	{
		/* FIXME: Analyze program inputs */
		if (header->descsz)
			warning("%s: pt_note '%s' with descsz != 0 ignored", note_type_str, __FUNCTION__);
		break;
	}

	
	case 4:  /* ELF_NOTE_ATI_CONDOUT */
		break;
	
	case 5:  /* ELF_NOTE_ATI_FLOAT32CONSTS */
	case 6:  /* ELF_NOTE_ATI_INT32CONSTS */
	case 7:  /* ELF_NOTE_ATI_BOOL32CONSTS */
	{
		int data_segment_desc_count;
		struct pt_note_data_segment_desc_t *data_segment_desc;

		int j;

		/* Get number of entries */
		assert(header->descsz % sizeof(struct pt_note_data_segment_desc_t) == 0);
		data_segment_desc_count = header->descsz / sizeof(struct pt_note_data_segment_desc_t);
		elf_debug("\tnote including data for constant buffers (%d entries)\n",
				data_segment_desc_count);

		/* Decode entries */
		for (j = 0; j < data_segment_desc_count; j++) {
			data_segment_desc = desc + j * sizeof(struct pt_note_data_segment_desc_t);
			elf_debug("\tdata_segment_desc[%d]: offset=0x%x, size=%d\n",
				j, data_segment_desc->offset, data_segment_desc->size);

			/* Dump constants - FIXME*/
			/*for (k = 0; k < data_segment_desc->size; k += 4) {
				uint32_t c;
				float f;
				c = * (uint32_t *) (cal_abi->data_buffer + data_segment_desc->offset * 16 + k);
				f = * (float *) (cal_abi->data_buffer + data_segment_desc->offset * 16 + k);
				elf_debug("constant[%02d] = 0x%08x, %gf\n", k / 4, c, f);
			}*/
		}
		break;
	}

	
	case 8:  /* ELF_NOTE_ATI_EARLYEXIT */
	{
		Elf32_Word early_exit;

		/* Get 'early_exit' value */
		early_exit = header->descsz ? * (uint32_t *) desc : 0;
		elf_debug("\tearly_exit = %s\n", early_exit ? "TRUE" : "FALSE");
		break;
	}

	
	case 9:  /* ELF_NOTE_ATI_GLOBAL_BUFFERS */
	{
		Elf32_Word global_buffers;
		global_buffers = header->descsz ? * (uint32_t *) desc : 0;
		elf_debug("\tglobal_buffers = %s\n", global_buffers ? "TRUE" : "FALSE");
		break;
	}
	
	
	case 10:  /* ELF_NOTE_ATI_CONSTANT_BUFFERS */
	{
		int constant_buffer_count;
		struct pt_note_constant_buffer_mask_t *constant_buffer_mask;
		int i;

		/* Get number of entries */
		assert(header->descsz % sizeof(struct pt_note_constant_buffer_mask_t) == 0);
		constant_buffer_count = header->descsz / sizeof(struct pt_note_constant_buffer_mask_t);
		elf_debug("\tnote including number and size of constant buffers (%d entries)\n",
			constant_buffer_count);

		/* Decode entries */
		for (i = 0; i < constant_buffer_count; i++) {
			constant_buffer_mask = desc + i * sizeof(struct pt_note_constant_buffer_mask_t);
			elf_debug("\tconstant_buffer[%d].size = %d (vec4f constants)\n",
				constant_buffer_mask->index, constant_buffer_mask->size);
		}
		break;
	}

	
	case 11:  /* ELF_NOTE_ATI_INPUT_SAMPLERS */
		break;
	
	case 12:  /* ELF_NOTE_ATI_PERSISTENT_BUFFERS */
	{
		Elf32_Word persistent_buffers;
		persistent_buffers = header->descsz ? * (uint32_t *) desc : 0;
		elf_debug("\tpersistent_buffers = %s\n", persistent_buffers ? "TRUE" : "FALSE");
		break;
	}

	
	case 13:  /* ELF_NOTE_ATI_SCRATCH_BUFFERS */
	{
		Elf32_Word scratch_buffers;
		scratch_buffers = header->descsz ? * (uint32_t *) desc : 0;
		elf_debug("\tscratch_buffers = %s\n", scratch_buffers ? "TRUE" : "FALSE");
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
		elf_debug("\tunknown type\n");
	}
}


/* Decode notes in the PT_NOTE segment of the given encoding dictionary entry */
static void amd_bin_read_notes(struct amd_bin_t *amd_bin, struct amd_bin_enc_dict_entry_t *enc_dict_entry)
{
	struct elf_buffer_t *buffer;

	/* Get buffer and set position */
	buffer = &enc_dict_entry->pt_note_buffer;
	elf_buffer_seek(buffer, 0);

	/* Decode notes */
	elf_debug("Reading notes in PT_NOTE segment (enc. dict. for machine=0x%x)\n",
		enc_dict_entry->header->d_machine);
	while (buffer->pos < buffer->size)
		amd_bin_read_note_header(amd_bin, enc_dict_entry);
}


static void amd_bin_read_enc_dict(struct amd_bin_t *amd_bin)
{
	struct elf_file_t *elf_file;
	struct elf_buffer_t *buffer;
	Elf32_Ehdr *elf_header;

	struct elf_program_header_t *program_header;

	struct amd_bin_enc_dict_entry_t *enc_dict_entry;
	struct amd_bin_enc_dict_entry_header_t *enc_dict_entry_header;
	int enc_dict_entry_count;

	int i;

	/* ELF header */
	elf_file = amd_bin->elf_file;
	elf_header = elf_file->header;
	buffer = &elf_file->buffer;
	elf_debug("**\n** Parsing AMD Binary (Internal ELF file)\n** %s\n**\n\n", elf_file->path);
	AMD_BIN_NOT_SUPPORTED_NEQ(elf_header->e_ident[EI_CLASS], ELFCLASS32);
	AMD_BIN_NOT_SUPPORTED_NEQ(elf_header->e_ident[EI_DATA], ELFDATA2LSB);
	AMD_BIN_NOT_SUPPORTED_NEQ(elf_header->e_ident[EI_OSABI], 0x64);
	AMD_BIN_NOT_SUPPORTED_NEQ(elf_header->e_ident[EI_ABIVERSION], 1);
	AMD_BIN_NOT_SUPPORTED_NEQ(elf_header->e_type, ET_EXEC);
	AMD_BIN_NOT_SUPPORTED_NEQ(elf_header->e_machine, 0x7d);
	AMD_BIN_NOT_SUPPORTED_NEQ(elf_header->e_entry, 0);
	
	/* Look for encoding dictionary (program header with type 'PT_LOPROC+2') */
	program_header = NULL;
	for (i = 0; i < list_count(elf_file->program_header_list); i++) {
		program_header = list_get(elf_file->program_header_list, i);
		if (program_header->header->p_type == PT_LOPROC + 2)
			break;
	}
	if (i == list_count(elf_file->program_header_list) || !program_header)
		fatal("%s: no encoding dictionary", elf_file->path);
	elf_debug("Encoding dictionary found in program header %d\n", i);
	
	/* Parse encoding dictionary */
	AMD_BIN_NOT_SUPPORTED_NEQ(program_header->header->p_vaddr, 0);
	AMD_BIN_NOT_SUPPORTED_NEQ(program_header->header->p_paddr, 0);
	AMD_BIN_NOT_SUPPORTED_NEQ(program_header->header->p_memsz, 0);
	AMD_BIN_NOT_SUPPORTED_NEQ(program_header->header->p_flags, 0);
	AMD_BIN_NOT_SUPPORTED_NEQ(program_header->header->p_align, 0);
	assert(program_header->header->p_filesz % sizeof(struct amd_bin_enc_dict_entry_header_t) == 0);
	enc_dict_entry_count = program_header->header->p_filesz / sizeof(struct amd_bin_enc_dict_entry_header_t);
	elf_debug("  -> %d entries\n\n", enc_dict_entry_count);

	/* Read encoding dictionary entries */
	amd_bin->enc_dict = list_create();
	elf_buffer_seek(buffer, program_header->header->p_offset);
	for (i = 0; i < enc_dict_entry_count; i++) {
		
		/* Create entry */
		enc_dict_entry = calloc(1, sizeof(struct amd_bin_enc_dict_entry_t));
		enc_dict_entry->header = elf_buffer_tell(buffer);
		elf_buffer_read(buffer, NULL, sizeof(struct amd_bin_enc_dict_entry_header_t));
		list_add(amd_bin->enc_dict, enc_dict_entry);

		/* Store encoding dictionary entry for Evergreen (code 9) */
		if (enc_dict_entry->header->d_machine == 9)
			amd_bin->enc_dict_entry_evergreen = enc_dict_entry;
	}

	/* Debug */
	elf_debug("idx %-10s %-10s %-10s %-10s %-10s\n", "d_machine", "d_type",
		"d_offset", "d_size", "d_flags");
	for (i = 0; i < 80; i++)
		elf_debug("-");
	elf_debug("\n");
	for (i = 0; i < list_count(amd_bin->enc_dict); i++) {
		enc_dict_entry = list_get(amd_bin->enc_dict, i);
		enc_dict_entry_header = enc_dict_entry->header;
		elf_debug("%3d 0x%-8x 0x%-8x 0x%-8x %-10d 0x%-8x\n",
			i, enc_dict_entry_header->d_machine,
			enc_dict_entry_header->d_type,
			enc_dict_entry_header->d_offset,
			enc_dict_entry_header->d_size,
			enc_dict_entry_header->d_flags);
	}
	elf_debug("\n\n");
}


static void amd_bin_read_segments(struct amd_bin_t *amd_bin)
{
	struct elf_file_t *elf_file;

	struct amd_bin_enc_dict_entry_t *enc_dict_entry;
	struct elf_program_header_t *program_header;

	int i, j;

	elf_debug("Reading PT_NOTE and PT_LOAD segments:\n");
	elf_file = amd_bin->elf_file;
	for (i = 0; i < list_count(amd_bin->enc_dict); i++)
	{
		/* Get encoding dictionary entry */
		enc_dict_entry = list_get(amd_bin->enc_dict, i);
		for (j = 0; j < list_count(elf_file->program_header_list); j++)
		{
			/* Get program header. If not in encoding dictionary segment, skip. */
			program_header = list_get(elf_file->program_header_list, j);
			if (program_header->header->p_offset < enc_dict_entry->header->d_offset ||
				program_header->header->p_offset >= enc_dict_entry->header->d_offset +
				enc_dict_entry->header->d_size)
				continue;
			assert(program_header->header->p_offset + program_header->header->p_filesz <=
				enc_dict_entry->header->d_offset + enc_dict_entry->header->d_size);

			/* Segment PT_NOTE */
			if (program_header->header->p_type == PT_NOTE) {
				if (enc_dict_entry->pt_note_buffer.size)
					fatal("%s: more than one PT_NOTE for encoding dictionary entry", __FUNCTION__);
				enc_dict_entry->pt_note_buffer.ptr = elf_file->buffer.ptr + program_header->header->p_offset;
				enc_dict_entry->pt_note_buffer.size = program_header->header->p_filesz;
				enc_dict_entry->pt_note_buffer.pos = 0;
			}

			/* Segment PT_LOAD */
			if (program_header->header->p_type == PT_LOAD) {
				if (enc_dict_entry->pt_load_buffer.size)
					fatal("%s: more than one PT_LOAD for encoding dictionary entry", __FUNCTION__);
				enc_dict_entry->pt_load_buffer.ptr = elf_file->buffer.ptr + program_header->header->p_offset;
				enc_dict_entry->pt_load_buffer.size = program_header->header->p_filesz;
				enc_dict_entry->pt_load_buffer.pos = 0;
			}
		}

		/* Check that both PT_NOTE and PT_LOAD segments were found */
		if (!enc_dict_entry->pt_note_buffer.size)
			fatal("%s: no PT_NOTE segment found for encoding dictionary entry", __FUNCTION__);
		if (!enc_dict_entry->pt_load_buffer.size)
			fatal("%s: no PT_LOAD segment found for encoding dictionary entry", __FUNCTION__);
		elf_debug("  Dict. entry %d: PT_NOTE segment: offset=0x%x, size=%d\n", i,
			(int) (enc_dict_entry->pt_note_buffer.ptr - elf_file->buffer.ptr), enc_dict_entry->pt_note_buffer.size);
		elf_debug("  Dict. entry %d: PT_LOAD segment: offset=0x%x, size=%d\n", i,
			(int) (enc_dict_entry->pt_load_buffer.ptr - elf_file->buffer.ptr), enc_dict_entry->pt_load_buffer.size);
	}
}


static void amd_bin_read_sections(struct amd_bin_t *amd_bin)
{
	struct elf_file_t *elf_file;

	struct amd_bin_enc_dict_entry_t *enc_dict_entry;
	struct elf_section_t *section;

	int i, j;

	int pt_load_offset;
	int pt_load_size;

	elf_file = amd_bin->elf_file;
	for (i = 0; i < list_count(amd_bin->enc_dict); i++)
	{
		/* Get encoding dictionary entry */
		enc_dict_entry = list_get(amd_bin->enc_dict, i);
		pt_load_offset = enc_dict_entry->pt_load_buffer.ptr - elf_file->buffer.ptr;
		pt_load_size = enc_dict_entry->pt_load_buffer.size;
		for (j = 0; j < list_count(elf_file->section_list); j++)
		{
			/* Get section. If not in PT_LOAD segment, skip. */
			section = list_get(elf_file->section_list, j);
			if (section->header->sh_offset < pt_load_offset ||
				section->header->sh_offset >= pt_load_offset + pt_load_size)
				continue;
			assert(section->header->sh_offset + section->header->sh_size <=
				pt_load_offset + pt_load_size);

			/* Sections */
			if (!strcmp(section->name, ".text")) {
				if (enc_dict_entry->sec_text_buffer.size)
					fatal("%s: duplicated '.text' section", __FUNCTION__);
				enc_dict_entry->sec_text_buffer.ptr = elf_file->buffer.ptr + section->header->sh_offset;
				enc_dict_entry->sec_text_buffer.size = section->header->sh_size;
				enc_dict_entry->sec_text_buffer.pos = 0;
			} else if (!strcmp(section->name, ".data")) {
				if (enc_dict_entry->sec_data_buffer.size)
					fatal("%s: duplicated '.data' section", __FUNCTION__);
				enc_dict_entry->sec_data_buffer.ptr = elf_file->buffer.ptr + section->header->sh_offset;
				enc_dict_entry->sec_data_buffer.size = section->header->sh_size;
				enc_dict_entry->sec_data_buffer.pos = 0;
			} else if (!strcmp(section->name, ".symtab")) {
				if (enc_dict_entry->sec_symtab_buffer.size)
					fatal("%s: duplicated '.symtab' section", __FUNCTION__);
				enc_dict_entry->sec_symtab_buffer.ptr = elf_file->buffer.ptr + section->header->sh_offset;
				enc_dict_entry->sec_symtab_buffer.size = section->header->sh_size;
				enc_dict_entry->sec_symtab_buffer.pos = 0;
			} else if (!strcmp(section->name, ".strtab")) {
				if (enc_dict_entry->sec_strtab_buffer.size)
					fatal("%s: duplicated '.strtab' section", __FUNCTION__);
				enc_dict_entry->sec_strtab_buffer.ptr = elf_file->buffer.ptr + section->header->sh_offset;
				enc_dict_entry->sec_strtab_buffer.size = section->header->sh_size;
				enc_dict_entry->sec_strtab_buffer.pos = 0;
			} else {
				fatal("%s: not recognized section name: '%s'",
					__FUNCTION__, section->name);
			}
		}

		/* Check that all sections where read */
		if (!enc_dict_entry->sec_text_buffer.size
			|| !enc_dict_entry->sec_data_buffer.size
			|| !enc_dict_entry->sec_symtab_buffer.size
			|| !enc_dict_entry->sec_strtab_buffer.size)
			fatal("%s: some section was not found: .text .data .symtab .strtab",
				__FUNCTION__);
	}

	/* Finish */
	elf_debug("\n");
}





/*
 * Public functions
 */


struct amd_bin_t *amd_bin_create(void *ptr, int size, char *name)
{
	struct amd_bin_t *amd_bin;

	/* Create structure */
	amd_bin = calloc(1, sizeof(struct amd_bin_t));

	/* Read and parse ELF file */
	amd_bin->elf_file = elf_file_create_from_buffer(ptr, size, name);

	/* Read encoding dictionary.
	 * Check that an Evergreen dictionary entry is present */
	amd_bin_read_enc_dict(amd_bin);
	if (!amd_bin->enc_dict_entry_evergreen)
		fatal("%s: no encoding dictionary entry for Evergreen.\n"
			"\tThe OpenCL kernel binary that your application is trying to load does not\n"
			"\tcontain Evergreen assembly code. Please make sure that a Cypress device\n"
			"\tis selected when compiling the OpenCL kernel source. In some cases, even\n"
			"\ta proper selection of this architecture causes Evergreen assembly not to\n"
			"\tbe included if the APP SDK is not correctly installed when compiling your\n"
			"\town kernel sources.\n",
			amd_bin->elf_file->path);
	
	/* Read segments and sections */
	amd_bin_read_segments(amd_bin);
	amd_bin_read_sections(amd_bin);

	/* Read notes in PT_NOTE segment for Evergreen dictionary entry */
	amd_bin_read_notes(amd_bin, amd_bin->enc_dict_entry_evergreen);

	/* Return */
	return amd_bin;
}


void amd_bin_free(struct amd_bin_t *amd_bin)
{
	/* Free encoding dictionary */
	while (list_count(amd_bin->enc_dict))
		free(list_remove_at(amd_bin->enc_dict, 0));
	list_free(amd_bin->enc_dict);

	/* Free rest */
	elf_file_free(amd_bin->elf_file);
	free(amd_bin);
}

