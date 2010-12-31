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
	5, {
		{ "ELF_NOTE_ATI_INPUTS", 2 },
		{ "ELF_NOTE_ATI_OUTPUTS", 4 },
		{ "ELF_NOTE_ATI_FLOATCONSTS", 5 },
		{ "ELF_NOTE_ATI_INTCONSTS", 6 },
		{ "ELF_NOTE_ATI_EARLYEXIT", 7 }
	}
};


typedef struct {
	Elf32_Word offset;  /* Offsets in bytes to start of data */
	Elf32_Word size;  /* Size in bytes of data */
} CALDataSegmentDesc;


void cal_abi_dump_note_header(CALNoteHeader *pt_note_header)
{
	debug_tab(opencl_debug_category, 6);
	opencl_debug("pt_note.namesz = 0x%x\n", pt_note_header->namesz);
	opencl_debug("pt_note.descsz = 0x%x\n", pt_note_header->descsz);
	opencl_debug("pt_note.type = 0x%x (%s)\n", pt_note_header->type, map_value(&pt_note_type_map, pt_note_header->type));
	opencl_debug("pt_note.name = '%s'\n", pt_note_header->name);
}


/* Given an encoding dictionary entry, find the corresponding PT_NOTE and PT_LOAD segments.
 * Return a pointer to the associated program header, and an allocated copy of the segment contents. */
void cal_abi_read_segments(struct elf_file_t *elf,
	CALEncodingDictionaryEntry *enc_dict_entry,
	Elf32_Phdr **pt_note_phdr, void **pt_note_buffer,
	Elf32_Phdr **pt_load_phdr, void **pt_load_buffer)
{
	int phidx;  /* Program header index */
	Elf32_Phdr *phdr;

	*pt_note_buffer = NULL;
	*pt_load_buffer = NULL;
	for (phidx = 0; phidx < elf->ehdr.e_phnum; phidx++) {
		
		/* Segment referred by program header must fall in range delimited by encoding dictionary entry */
		phdr = &elf->phdr[phidx];
		if (!IN_RANGE(phdr->p_offset, enc_dict_entry->d_offset, enc_dict_entry->d_offset + enc_dict_entry->d_size - 1))
			continue;

		/* Segment PT_NOTE */
		if (phdr->p_type == PT_NOTE) {
			if (*pt_note_buffer)
				fatal("%s: more than one PT_NOTE segment for encoding dictionary entry", __FUNCTION__);
			*pt_note_phdr = phdr;
			*pt_note_buffer = elf_read_buffer(elf, phdr->p_offset, phdr->p_filesz);
		}

		/* Segment PT_LOAD */
		if (phdr->p_type == PT_LOAD) {
			if (*pt_load_buffer)
				fatal("%s: more than one PT_LOAD segment for encoding dictionary entry", __FUNCTION__);
			*pt_load_phdr = phdr;
			*pt_load_buffer = elf_read_buffer(elf, phdr->p_offset, phdr->p_filesz);
		}
	}

	/* Check that both PT_NOTE and PT_LOAD segments were found */
	if (!*pt_note_buffer)
		fatal("%s: no PT_NOTE segment found for encoding dictionary entry", __FUNCTION__);
	if (!*pt_load_buffer)
		fatal("%s: no PT_LOAD segment found for encoding dictionary entry", __FUNCTION__);
}


/* Given a program header representing a PT_LOAD segment, and the buffer containing the
 * segment contents, return pointers to the '.text', '.data', '.symtab', and '.strtab'
 * sections within that segment. */
void cal_abi_read_segment_sections(struct elf_file_t *elf, Elf32_Phdr *pt_load_phdr, void *pt_load_buffer,
	Elf32_Shdr **text_shdr, void **text_buffer,
	Elf32_Shdr **data_shdr, void **data_buffer,
	Elf32_Shdr **symtab_shdr, void **symtab_buffer,
	Elf32_Shdr **strtab_shdr, void **strtab_buffer)
{
	char *section_name;
	void *section_buffer;
	Elf32_Shdr *shdr;
	int i;

	*text_buffer = NULL;
	*data_buffer = NULL;
	*symtab_buffer = NULL;
	*strtab_buffer = NULL;

	/* Find sections */
	for (i = 0; i < elf_section_count(elf); i++) {
		
		/* Section must be in segment */
		shdr = &elf->shdr[i];
		elf_section_info(elf, i, &section_name, NULL, NULL, NULL);
		if (!IN_RANGE(shdr->sh_offset, pt_load_phdr->p_offset, pt_load_phdr->p_offset + pt_load_phdr->p_filesz - 1))
			continue;

		/* Locate section within 'pt_load_buffer'. The relative offset is equal to the absolute section offset
		 * minus the PT_LOAD segment offset. */
		section_buffer = pt_load_buffer + shdr->sh_offset - pt_load_phdr->p_offset;

		/* Which section is this */
		if (!strcmp(section_name, ".text")) {
			if (*text_buffer)
				fatal("%s: duplicated '.text' section", __FUNCTION__);
			*text_shdr = shdr;
			*text_buffer = section_buffer;
		} else if (!strcmp(section_name, ".data")) {
			if (*data_buffer)
				fatal("%s: duplicated '.data' section", __FUNCTION__);
			*data_shdr = shdr;
			*data_buffer = section_buffer;
		} else if (!strcmp(section_name, ".symtab")) {
			if (*symtab_buffer)
				fatal("%s: duplicated '.symtab' section", __FUNCTION__);
			*symtab_shdr = shdr;
			*symtab_buffer = section_buffer;
		} else if (!strcmp(section_name, ".strtab")) {
			if (*strtab_buffer)
				fatal("%s: duplicated '.strtab' section", __FUNCTION__);
			*strtab_shdr = shdr;
			*strtab_buffer = section_buffer;
		} else
			fatal("%s: not recognized section name: '%s'", __FUNCTION__, section_name);
	}

	/* Check that all sections were found */
	if (!*text_buffer || !*data_buffer || !*symtab_buffer || !*strtab_buffer)
		fatal("%s: some section was not found: .text .data .symtab .strtab", __FUNCTION__);
}


void cal_abi_parse_elf(char *file_name)
{
	struct elf_file_t *elf;

	int enc_dict_idx;  /* Index for encoding dictionary in program header table */
	Elf32_Phdr *enc_dict_phdr;
	CALEncodingDictionaryEntry *enc_dict_entries, *enc_dict_entry;
	int enc_dict_entry_count;

	/* PT_NOTE segment */
	Elf32_Phdr *pt_note_phdr;
	void *pt_note_buffer;

	/* For individual pt_notes */
	void *pt_note_ptr;
	CALNoteHeader *pt_note_header;

	/* PT_LOAD segment */
	Elf32_Phdr *pt_load_phdr;
	void *pt_load_buffer;

	/* Sections within PT_LOAD segment */
	Elf32_Shdr *text_shdr, *data_shdr, *symtab_shdr, *strtab_shdr;
	void *text_buffer, *data_buffer, *symtab_buffer, *strtab_buffer;
	
	int i;

	/* Analyze ELF */
	opencl_debug("CAL ABI analyzer: parsing file '%s'\n", file_name);
	elf = elf_open(file_name);
	
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
	for (enc_dict_idx = 0; enc_dict_idx < elf->ehdr.e_phnum && elf->phdr[enc_dict_idx].p_type != PT_LOPROC + 2; enc_dict_idx++);
	if (enc_dict_idx == elf->ehdr.e_phnum)
		CAL_ABI_NOT_SUPPORTED(enc_dict_idx);
	enc_dict_phdr = &elf->phdr[enc_dict_idx];

	/* Parse encoding dictionary */
	CAL_ABI_NOT_SUPPORTED_NEQ(enc_dict_phdr->p_vaddr, 0);
	CAL_ABI_NOT_SUPPORTED_NEQ(enc_dict_phdr->p_paddr, 0);
	CAL_ABI_NOT_SUPPORTED_NEQ(enc_dict_phdr->p_memsz, 0);
	CAL_ABI_NOT_SUPPORTED_NEQ(enc_dict_phdr->p_flags, 0);
	CAL_ABI_NOT_SUPPORTED_NEQ(enc_dict_phdr->p_align, 0);
	enc_dict_entry_count = enc_dict_phdr->p_filesz / sizeof(CALEncodingDictionaryEntry);
	opencl_debug("%d entries\n", enc_dict_entry_count);
	
	/* Read entries */
	enc_dict_entries = elf_read_buffer(elf, enc_dict_phdr->p_offset, enc_dict_phdr->p_filesz);
	for (i = 0; i < enc_dict_entry_count; i++) {
		
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

		/* Load PT_NOTE and PT_LOAD segments for encoding dictionary entry */
		cal_abi_read_segments(elf, enc_dict_entry, &pt_note_phdr, &pt_note_buffer,
			&pt_load_phdr, &pt_load_buffer);

		/* Get sections within PT_LOAD segment */
		cal_abi_read_segment_sections(elf, pt_load_phdr, pt_load_buffer,
			&text_shdr, &text_buffer, &data_shdr, &data_buffer,
			&symtab_shdr, &symtab_buffer, &strtab_shdr, &strtab_buffer);

		/* Decode notes */
		pt_note_ptr = pt_note_buffer;
		while (pt_note_ptr < pt_note_buffer + pt_note_phdr->p_filesz) {
			
			/* Get note */
			pt_note_header = pt_note_ptr;
			cal_abi_dump_note_header(pt_note_header);

			/* Analyze note */
			switch (pt_note_header->type) {
			
			case 6:  /* ELF_NOTE_ATI_INTCONSTS */
			{
				int data_segment_desc_count;
				CALDataSegmentDesc *data_segment_desc;
				int j, k;
				uint32_t c;

				/* Get number of entries */
				data_segment_desc_count = pt_note_header->descsz / sizeof(CALDataSegmentDesc);
				opencl_debug("Note including integer constants for constant buffers (%d entries)\n",
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
						c = * (uint32_t *) (data_buffer + data_segment_desc->offset);
						opencl_debug("constant[%02d] = 0x%08x\n", k / 4, c);
					}
					debug_tab_dec(opencl_debug_category, 2);
					debug_tab_dec(opencl_debug_category, 2);
				}
				break;
			}

			default:
				opencl_debug("unknown type\n");
			}

			/* Next note */
			opencl_debug("\n");
			pt_note_ptr += sizeof(CALNoteHeader) + pt_note_header->descsz;
		}

		/* Free PT_NOTE and PT_LOAD buffers */
		elf_free_buffer(pt_note_buffer);
		elf_free_buffer(pt_load_buffer);
	}

	/* Free structures */
	elf_free_buffer(enc_dict_entries);

	/* Close */
	debug_tab(opencl_debug_category, 0);
	elf_close(elf);
	exit(1);//////
}

