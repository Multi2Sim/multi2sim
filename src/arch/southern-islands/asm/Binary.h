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

#ifndef ARCH_SOUTHERN_ISLANDS_ASM_BINARY_H
#define ARCH_SOUTHERN_ISLANDS_ASM_BINARY_H

#define SI_BINARY_MAX_USER_ELEMENTS 16


#ifdef __cplusplus

#include <lib/cpp/ELFReader.h>
#include <lib/cpp/Misc.h>



namespace SI
{

extern Misc::StringMap binary_machine_map;
extern Misc::StringMap binary_note_map;
extern Misc::StringMap binary_prog_info_map;


struct BinaryNoteDataSegmentDesc
{
	Elf32_Word offset;  /* Offsets in bytes to start of data */
	Elf32_Word size;  /* Size in bytes of data */
};


struct BinaryNoteConstantBufferMask
{
	Elf32_Word index;  /* Constant buffer identifier */
	Elf32_Word size;  /* Size in vec4f constants of the buffer */
};

struct BinaryNoteUAVEntry
{
	Elf32_Word id;  /* UAV number */
	Elf32_Word unknown1;  /* FIXME */
	Elf32_Word unknown2;  /* FIXME */
	Elf32_Word unknown3;  /* FIXME */
};


struct BinaryNoteProgInfoEntry
{
	Elf32_Word address;  /* Device address */
	Elf32_Word value;  /* Value */
};


/* Encoding dictionary entry header (as encoded in ELF file) */
struct BinaryDictHeader
{
	Elf32_Word d_machine;
	Elf32_Word d_type;
	Elf32_Off d_offset;  /* Offset for encoding data (PT_NOTE + PT_LOAD segments) */
	Elf32_Word d_size;  /* Size of encoding data (PT_NOTE + PT_LOAD segments) */
	Elf32_Word d_flags;
};


/* Constats embedded in the '.data' section */
struct BinaryDictConsts
{
	float float_consts[256][4];
	unsigned int int_consts[32][4];
	unsigned int bool_consts[32];
};

extern Misc::StringMap binary_user_data_map;
enum BinaryUserData
{
	BinaryUserDataResource,               // immediate resource descriptor
	BinaryUserDataSampler,                // immediate sampler descriptor
	BinaryUserDataConstBuffer,           // immediate const buffer descriptor
	BinaryUserDataVertexBuffer,          // immediate vertex buffer descriptor
	BinaryUserDataUAV,                    // immediate UAV descriptor
	BinaryUserDataALUFloatConst,        // immediate float const (scalar or vector)
	BinaryUserDataALUBool32Const,       // 32 immediate bools packed into a single UINT
	BinaryUserDataGDSCounterRange,      // immediate UINT with GDS address range for counters
	BinaryUserDataGDSMemoryRange,       // immediate UINT with GDS address range for storage
	BinaryUserDataGWSBase,               // immediate UINT with GWS resource base offset
	BinaryUserDataWorkItemRange,        // immediate HSAIL work item range
	BinaryUserDataWorkGroupRange,       // immediate HSAIL work group range
	BinaryUserDataDispatchId,            // immediate HSAIL dispatch ID
	BinaryUserDataScratchBuffer,         // immediate HSAIL scratch buffer descriptor
	BinaryUserDataHeapBuffer,            // immediate HSAIL heap buffer descriptor
	BinaryUserDataKernelArg,             // immediate HSAIL kernel argument
	BinaryUserDataContextBase,           // immediate HSAIL context base-address
	BinaryUserDataLDSEsgsSize,          // immediate LDS ESGS size used in on-chip GS
	BinaryUserDataPtrFetchShader,       // fetch shader subroutine pointer
	BinaryUserDataPtrResourceTable,         // flat/chunked resource table pointer
	BinaryUserDataConstBufferTable,     // flat/chunked const buffer table pointer
	BinaryUserDataInteralResourceTable,// flat/chunked internal resource table pointer
	BinaryUserDataSamplerTable,          // flat/chunked sampler table pointer
	BinaryUserDataUAVTable,              // flat/chunked UAV resource table pointer
	BinaryUserDataInternalGlobalTable,  // internal driver table pointer
	BinaryUserDataVertexBufferTable,    // flat/chunked vertex buffer table pointer
	BinaryUserDataSoBufferTable,        // flat/chunked stream-out buffer table pointer
	BinaryUserDataExtendedUserData,     // extended user data in video memory
	BinaryUserDataIndirectResource,      // pointer to resource indirection table
	BinaryUserDataInternalResource,// pointer to internal resource indirection table
	BinaryUserDataPtrIndirectUAV,           // pointer to UAV indirection table
	BinaryUserDataLast
};


/* User Element entry */
struct BinaryUserElement
{
	unsigned int dataClass;
	unsigned int apiSlot;
	unsigned int startUserReg;
	unsigned int userRegCount;
};


/* COMPUTE_PGM_RSRC2 */
struct BinaryComputePgmRsrc2
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

/* Encoding dictionary entry */
struct BinaryDictEntry
{
	/* Header (pointer to ELF buffer contents) */
	BinaryDictHeader *header;

	/* Streams containing PT_LOAD and PT_NOTE segments */
	ELFReader::ProgramHeader *pt_load_segment;
	ELFReader::ProgramHeader *pt_note_segment;

	/* Streams containing sections */
	ELFReader::Section *text_section;
	ELFReader::Section *data_section;
	ELFReader::Section *symtab_section;
	ELFReader::Section *strtab_section;

	/* Constants extract from '.data' section */
	BinaryDictConsts *consts;

	/* Info read from pt_notes */
	int num_vgpr;
	int num_sgpr;
	int lds_size;
	int stack_size;

	unsigned int num_user_elements;
	BinaryUserElement user_elements[16];

	BinaryComputePgmRsrc2 *compute_pgm_rsrc2;
};


/* Binary file */
class Binary : ELFReader::File
{
	/* Encoding dictionary. Each element of the dictionary contains the
	 * binary for a different architecture (Evergreen, x86, etc.) */
	std::vector<BinaryDictEntry*> dict;

	/* Encoding dictionary entry containing the Southern Islands kernel.
	 * This is a member of the 'dict' vector. */
	BinaryDictEntry *si_dict_entry;

	/* Binary file name */
	std::string name;

	void ReadNote(BinaryDictEntry *dict_entry, std::istringstream& ss,
			char *buffer);
	void ReadNotes(BinaryDictEntry *dict_entry);
	void ReadDictionary();
	void ReadSegments();
	void ReadSections();
public:
	static Misc::Debug debug;

	Binary(const char *buffer, unsigned int size, std::string name);
	~Binary();

	BinaryDictEntry *GetSIDictEntry() { return si_dict_entry; }
};





}  /* namespace SI */


#endif  /* __cplusplus */



/*
 * C Wrapper
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <lib/util/elf-format.h>
#include <lib/util/list.h>


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

extern struct StringMapWrap *si_binary_user_data_map;
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


extern struct StringMapWrap *si_binary_machine_map;
extern struct StringMapWrap *si_binary_note_map;
extern struct StringMapWrap *si_binary_prog_info_map;

/* Binary dictionary entry */
struct SIBinaryDictEntry;
int SIBinaryDictEntryGetNumVgpr(struct SIBinaryDictEntry *self);
int SIBinaryDictEntryGetNumSgpr(struct SIBinaryDictEntry *self);
int SIBinaryDictEntryGetLDSSize(struct SIBinaryDictEntry *self);
int SIBinaryDictEntryGetStackSize(struct SIBinaryDictEntry *self);
unsigned int SIBinaryDictEntryGetNumUserElements(struct SIBinaryDictEntry *self);
struct SIBinaryUserElement *SIBinaryDictEntryGetUserElements(struct SIBinaryDictEntry *self);
struct SIBinaryComputePgmRsrc2 *SIBinaryDictEntryGetComputePgmRsrc2(struct SIBinaryDictEntry *self);
char *SIBinaryDictEntryGetTextBuffer(struct SIBinaryDictEntry *self);
unsigned int SIBinaryDictEntryGetTextSize(struct SIBinaryDictEntry *self);

/* Binary file */
struct SIBinary;
struct SIBinary *SIBinaryCreate(const char *buffer, unsigned int size, char *name);
void SIBinaryFree(struct SIBinary *bin);
void SIBinarySetDebugFile(const char *path);
struct SIBinaryDictEntry *SIBinaryGetSIDictEntry(struct SIBinary *bin);


#ifdef __cplusplus
}
#endif


#endif  /* ARCH_SOUTHERN_ISLANDS_ASM_BINARY_H */
