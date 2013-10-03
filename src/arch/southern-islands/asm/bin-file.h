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
 * Southern Islands Binary File
 */

#define SI_ABI_MAX_USER_ELEMENTS 16

struct SIBinaryNoteDataSegmentDesc
{
	Elf32_Word offset;  /* Offsets in bytes to start of data */
	Elf32_Word size;  /* Size in bytes of data */
};


struct SIBinaryNoteConstantBufferMask
{
	Elf32_Word index;  /* Constant buffer identifier */
	Elf32_Word size;  /* Size in vec4f constants of the buffer */
};

struct SIBinaryNoteUAVEntry
{
	Elf32_Word id;  /* UAV number */
	Elf32_Word unknown1;  /* FIXME */
	Elf32_Word unknown2;  /* FIXME */
	Elf32_Word unknown3;  /* FIXME */
};


struct SIBinaryNoteProgInfoEntry
{
	Elf32_Word address;  /* Device address */
	Elf32_Word value;  /* Value */
};


/* Encoding dictionary entry header (as encoded in ELF file) */
struct SIBinaryDictionaryHeader
{
	Elf32_Word d_machine;
	Elf32_Word d_type;
	Elf32_Off d_offset;  /* Offset for encoding data (PT_NOTE + PT_LOAD segments) */
	Elf32_Word d_size;  /* Size of encoding data (PT_NOTE + PT_LOAD segments) */
	Elf32_Word d_flags;
};


/* Constats embedded in the '.data' section */
struct SIBinaryDictionaryConsts
{
	float float_consts[256][4];
	unsigned int int_consts[32][4];
	unsigned int bool_consts[32];
};

enum SIBinaryUserData
{
	SIBinaryUserDataResource,               // immediate resource descriptor
	SIBinaryUserDataSampler,                // immediate sampler descriptor
	SIBinaryUserDataConstBuffer,           // immediate const buffer descriptor
	SIBinaryUserDataVertexBuffer,          // immediate vertex buffer descriptor
	SIBinaryUserDataUAV,                    // immediate UAV descriptor
	SIBinaryUserDataALUFloatConst,        // immediate float const (scalar or vector)
	SIBinaryUserDataALUBool32Const,       // 32 immediate bools packed into a single UINT
	SIBinaryUserDataGDSCounterRange,      // immediate UINT with GDS address range for counters
	SIBinaryUserDataGDSMemoryRange,       // immediate UINT with GDS address range for storage
	SIBinaryUserDataGWSBase,               // immediate UINT with GWS resource base offset
	SIBinaryUserDataWorkItemRange,        // immediate HSAIL work item range
	SIBinaryUserDataWorkGroupRange,       // immediate HSAIL work group range
	SIBinaryUserDataDispatchId,            // immediate HSAIL dispatch ID
	SIBinaryUserDataScratchBuffer,         // immediate HSAIL scratch buffer descriptor
	SIBinaryUserDataHeapBuffer,            // immediate HSAIL heap buffer descriptor
	SIBinaryUserDataKernelArg,             // immediate HSAIL kernel argument
	SIBinaryUserDataContextBase,           // immediate HSAIL context base-address
	SIBinaryUserDataLDSEsgsSize,          // immediate LDS ESGS size used in on-chip GS
	SIBinaryUserDataPtrFetchShader,       // fetch shader subroutine pointer
	SIBinaryUserDataPtrResourceTable,         // flat/chunked resource table pointer
	SIBinaryUserDataConstBufferTable,     // flat/chunked const buffer table pointer
	SIBinaryUserDataInteralResourceTable,// flat/chunked internal resource table pointer
	SIBinaryUserDataSamplerTable,          // flat/chunked sampler table pointer
	SIBinaryUserDataUAVTable,              // flat/chunked UAV resource table pointer
	SIBinaryUserDataInternalGlobalTable,  // internal driver table pointer
	SIBinaryUserDataVertexBufferTable,    // flat/chunked vertex buffer table pointer
	SIBinaryUserDataSoBufferTable,        // flat/chunked stream-out buffer table pointer
	SIBinaryUserDataExtendedUserData,     // extended user data in video memory
	SIBinaryUserDataIndirectResource,      // pointer to resource indirection table
	SIBinaryUserDataInternalResource,// pointer to internal resource indirection table
	SIBinaryUserDataPtrIndirectUAV,           // pointer to UAV indirection table
	SIBinaryUserDataLast
};


/* User Element entry */
struct SIBinaryUserElement
{
	unsigned int dataClass;
	unsigned int apiSlot;
	unsigned int startUserReg;
	unsigned int userRegCount;
};

struct SIBinaryUserElement *si_binary_user_element_create();
void si_binary_user_element_free(struct SIBinaryUserElement *user_elem);


/* COMPUTE_PGM_RSRC2 */
struct SIBinaryComputePgmRsrc2
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

struct SIBinaryComputePgmRsrc2 *si_binary_compute_pgm_rsrc2_create();
void si_binary_compute_pgm_rsrc2_free(struct SIBinaryComputePgmRsrc2 *pgm_rsrc2);


/* Encoding dictionary entry */
struct SIBinaryDictionaryEntry
{
	/* Header (pointer to ELF buffer contents) */
	struct SIBinaryDictionaryHeader *header;

	/* Buffers containing PT_LOAD and PT_NOTE segments */
	struct elf_buffer_t pt_load_buffer;
	struct elf_buffer_t pt_note_buffer;

	/* Buffers containing sections */
	struct elf_buffer_t sec_text_buffer;
	struct elf_buffer_t sec_data_buffer;
	struct elf_buffer_t sec_symtab_buffer;
	struct elf_buffer_t sec_strtab_buffer;

	/* Constants extract from '.data' section */
	struct SIBinaryDictionaryConsts *consts;

	/* Info read from pt_notes */
	int num_vgpr_used;
	int num_sgpr_used;
	int lds_size_used;
	int stack_size_used;

	unsigned int userElementCount;
	struct SIBinaryUserElement userElements[16];

	struct SIBinaryComputePgmRsrc2 *compute_pgm_rsrc2;
};


/* Binary file */
struct SIBinary
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
	struct SIBinaryDictionaryEntry *enc_dict_entry_southern_islands;
};

struct SIBinary *si_binary_create(void *ptr, int size, char *name);
void si_binary_free(struct SIBinary *bin);




/*
 * Public
 */

extern struct str_map_t pt_note_type_map;
extern struct str_map_t si_bin_user_data_class;


#endif
