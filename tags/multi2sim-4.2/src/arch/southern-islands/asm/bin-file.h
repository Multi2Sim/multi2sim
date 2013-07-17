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

#ifndef ARCH_SOUTHERN_ISLANDS_ASM_BIN_FILE_H
#define ARCH_SOUTHERN_ISLANDS_ASM_BIN_FILE_H

#include <lib/util/elf-format.h>


/*
 * Global Variables
 */
extern struct str_map_t pt_note_type_map;
extern struct str_map_t si_bin_user_data_class;

/*
 * Southern Islands Binary File
 */

#define SI_ABI_MAX_USER_ELEMENTS 16

struct pt_note_data_segment_desc_t
{
	Elf32_Word offset;  /* Offsets in bytes to start of data */
	Elf32_Word size;  /* Size in bytes of data */
};


struct pt_note_constant_buffer_mask_t
{
	Elf32_Word index;  /* Constant buffer identifier */
	Elf32_Word size;  /* Size in vec4f constants of the buffer */
};

struct pt_note_uav_entry_t
{
	Elf32_Word id;  /* UAV number */
	Elf32_Word unknown1;  /* FIXME */
	Elf32_Word unknown2;  /* FIXME */
	Elf32_Word unknown3;  /* FIXME */
};


struct pt_note_prog_info_entry_t
{
	Elf32_Word address;  /* Device address */
	Elf32_Word value;  /* Value */
};


/* Encoding dictionary entry header (as encoded in ELF file) */
struct si_bin_enc_dict_entry_header_t
{
	Elf32_Word d_machine;
	Elf32_Word d_type;
	Elf32_Off d_offset;  /* Offset for encoding data (PT_NOTE + PT_LOAD segments) */
	Elf32_Word d_size;  /* Size of encoding data (PT_NOTE + PT_LOAD segments) */
	Elf32_Word d_flags;
};


/* Constats embedded in the '.data' section */
struct si_bin_enc_dict_entry_consts_t
{
	float float_consts[256][4];
	unsigned int int_consts[32][4];
	unsigned int bool_consts[32];
};

typedef enum _E_SC_USER_DATA_CLASS
{
	IMM_RESOURCE,               // immediate resource descriptor
	IMM_SAMPLER,                // immediate sampler descriptor
	IMM_CONST_BUFFER,           // immediate const buffer descriptor
	IMM_VERTEX_BUFFER,          // immediate vertex buffer descriptor
	IMM_UAV,                    // immediate UAV descriptor
	IMM_ALU_FLOAT_CONST,        // immediate float const (scalar or vector)
	IMM_ALU_BOOL32_CONST,       // 32 immediate bools packed into a single UINT
	IMM_GDS_COUNTER_RANGE,      // immediate UINT with GDS address range for counters
	IMM_GDS_MEMORY_RANGE,       // immediate UINT with GDS address range for storage
	IMM_GWS_BASE,               // immediate UINT with GWS resource base offset
	IMM_WORK_ITEM_RANGE,        // immediate HSAIL work item range
	IMM_WORK_GROUP_RANGE,       // immediate HSAIL work group range
	IMM_DISPATCH_ID,            // immediate HSAIL dispatch ID
	IMM_SCRATCH_BUFFER,         // immediate HSAIL scratch buffer descriptor
	IMM_HEAP_BUFFER,            // immediate HSAIL heap buffer descriptor
	IMM_KERNEL_ARG,             // immediate HSAIL kernel argument
	IMM_CONTEXT_BASE,           // immediate HSAIL context base-address
	IMM_LDS_ESGS_SIZE,          // immediate LDS ESGS size used in on-chip GS
	SUB_PTR_FETCH_SHADER,       // fetch shader subroutine pointer
	PTR_RESOURCE_TABLE,         // flat/chunked resource table pointer
	PTR_CONST_BUFFER_TABLE,     // flat/chunked const buffer table pointer
	PTR_INTERNAL_RESOURCE_TABLE,// flat/chunked internal resource table pointer
	PTR_SAMPLER_TABLE,          // flat/chunked sampler table pointer
	PTR_UAV_TABLE,              // flat/chunked UAV resource table pointer
	PTR_INTERNAL_GLOBAL_TABLE,  // internal driver table pointer
	PTR_VERTEX_BUFFER_TABLE,    // flat/chunked vertex buffer table pointer
	PTR_SO_BUFFER_TABLE,        // flat/chunked stream-out buffer table pointer
	PTR_EXTENDED_USER_DATA,     // extended user data in video memory
	PTR_INDIRECT_RESOURCE,      // pointer to resource indirection table
	PTR_INDIRECT_INTERNAL_RESOURCE,// pointer to internal resource indirection table
	PTR_INDIRECT_UAV,           // pointer to UAV indirection table
	E_SC_USER_DATA_CLASS_LAST
} E_SC_USER_DATA_CLASS;


/* User Element entry */
struct si_bin_enc_user_element_t
{
	unsigned int dataClass;
	unsigned int apiSlot;
	unsigned int startUserReg;
	unsigned int userRegCount;
};

struct si_bin_enc_user_element_t *si_bin_enc_user_element_create();
void si_bin_enc_user_element_free(struct si_bin_enc_user_element_t *user_elem);


/* COMPUTE_PGM_RSRC2 */
struct si_bin_compute_pgm_rsrc2_t
{
	unsigned int scrach_en 		: 1;
	unsigned int user_sgpr 		: 5;
	unsigned int trap_present 	: 1;
	unsigned int tgid_x_en 		: 1;
	unsigned int tgid_y_en 		: 1;
	unsigned int tgid_z_en 		: 1;
	unsigned int tg_size_en 	: 1;
	unsigned int tidig_comp_cnt 	: 2;
	unsigned int excp_en_msb 	: 2;
	unsigned int lds_size 		: 9;
	unsigned int excp_en 		: 7;
	unsigned int 			: 1;
};

struct si_bin_compute_pgm_rsrc2_t *si_bin_compute_pgm_rsrc2_create();
void si_bin_compute_pgm_rsrc2_free(struct si_bin_compute_pgm_rsrc2_t *pgm_rsrc2);


/* Encoding dictionary entry */
struct si_bin_enc_dict_entry_t
{
	/* Header (pointer to ELF buffer contents) */
	struct si_bin_enc_dict_entry_header_t *header;

	/* Buffers containing PT_LOAD and PT_NOTE segments */
	struct elf_buffer_t pt_load_buffer;
	struct elf_buffer_t pt_note_buffer;

	/* Buffers containing sections */
	struct elf_buffer_t sec_text_buffer;
	struct elf_buffer_t sec_data_buffer;
	struct elf_buffer_t sec_symtab_buffer;
	struct elf_buffer_t sec_strtab_buffer;

	/* Constants extract from '.data' section */
	struct si_bin_enc_dict_entry_consts_t *consts;

	/* Info read from pt_notes */
	int num_vgpr_used;
	int num_sgpr_used;
	int lds_size_used;
	int stack_size_used;

	unsigned int userElementCount;
	struct si_bin_enc_user_element_t userElements[16];

	struct si_bin_compute_pgm_rsrc2_t *compute_pgm_rsrc2;
};


/* Binary file */
struct si_bin_file_t
{
	/* Associated ELF file */
	struct elf_file_t *elf_file;

	/* Encoding dictionary.
	 * Elements are of type 'struct si_bin_enc_dict_entry_t'
	 * Each element of the dictionary contains the binary for a different architecture
	 * (Evergreen, x86, etc.) */
	struct list_t *enc_dict;

	/* Encoding dictionary entry containing the Southern Islands kernel.
	 * This is a member of the 'enc_dict' list. */
	struct si_bin_enc_dict_entry_t *enc_dict_entry_southern_islands;
};

struct si_bin_file_t *si_bin_file_create(void *ptr, int size, char *name);
void si_bin_file_free(struct si_bin_file_t *bin);

#endif
