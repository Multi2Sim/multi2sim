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

#include <assert.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include <arch/southern-islands/asm/bin-file.h>

#define SI_BIN_FILE_NOT_SUPPORTED(__var) \
	fatal("%s: value 0x%x not supported for parameter '" #__var "'", __FUNCTION__, (__var))
#define SI_BIN_FILE_NOT_SUPPORTED_NEQ(__var, __val) \
	if ((__var) != (__val)) \
		fatal("%s: parameter '" #__var "' was expected to be 0x%x", __FUNCTION__, (__val))


/* Note header */
struct pt_note_header_t
{
	Elf32_Word namesz;  /* Size of the name field. Must be 8 */
	Elf32_Word descsz;  /* Size of the data payload */
	Elf32_Word type;  /* Type of the payload */
	char name[8];  /* Note header string. Must be "ATI CAL" */
};

struct str_map_t si_bin_user_data_class = 
{
	32, {
		{ "IMM_RESOURCE",                      IMM_RESOURCE },
		{ "IMM_SAMPLER",                       IMM_SAMPLER},
		{ "IMM_CONST_BUFFER",                  IMM_CONST_BUFFER },
		{ "IMM_VERTEX_BUFFER",                 IMM_VERTEX_BUFFER },
		{ "IMM_UAV",                           IMM_UAV },
		{ "IMM_ALU_FLOAT_CONST",               IMM_ALU_FLOAT_CONST},
		{ "IMM_ALU_BOOL32_CONST",              IMM_ALU_BOOL32_CONST },
		{ "IMM_GDS_COUNTER_RANGE",             IMM_GDS_COUNTER_RANGE },
		{ "IMM_GDS_MEMORY_RANGE",              IMM_GDS_MEMORY_RANGE },
		{ "IMM_GWS_BASE",                      IMM_GWS_BASE },
		{ "IMM_WORK_ITEM_RANGE",               IMM_WORK_ITEM_RANGE },
		{ "IMM_WORK_GROUP_RANGE",              IMM_WORK_GROUP_RANGE },
		{ "IMM_DISPATCH_ID",                   IMM_DISPATCH_ID },
		{ "IMM_SCRATCH_BUFFER",                IMM_SCRATCH_BUFFER },
		{ "IMM_HEAP_BUFFER",                   IMM_HEAP_BUFFER },
		{ "IMM_KERNEL_ARG",                    IMM_KERNEL_ARG },
		{ "IMM_CONTEXT_BASE",                  IMM_CONTEXT_BASE},
		{ "IMM_LDS_ESGS_SIZE",                 IMM_LDS_ESGS_SIZE },
		{ "SUB_PTR_FETCH_SHADER",              SUB_PTR_FETCH_SHADER },
		{ "PTR_RESOURCE_TABLE",                PTR_RESOURCE_TABLE },
		{ "PTR_CONST_BUFFER_TABLE",            PTR_CONST_BUFFER_TABLE },
		{ "PTR_INTERNAL_RESOURCE_TABLE",       PTR_INTERNAL_RESOURCE_TABLE },
		{ "PTR_SAMPLER_TABLE",                 PTR_SAMPLER_TABLE },
		{ "PTR_UAV_TABLE",                     PTR_UAV_TABLE },
		{ "PTR_INTERNAL_GLOBAL_TABLE",         PTR_INTERNAL_GLOBAL_TABLE },
		{ "PTR_VERTEX_BUFFER_TABLE",           PTR_VERTEX_BUFFER_TABLE },
		{ "PTR_SO_BUFFER_TABLE",               PTR_SO_BUFFER_TABLE },
		{ "PTR_EXTENDED_USER_DATA",            PTR_EXTENDED_USER_DATA },
		{ "PTR_INDIRECT_RESOURCE",             PTR_INDIRECT_RESOURCE },
		{ "PTR_INDIRECT_INTERNAL_RESOURCE",    PTR_INDIRECT_INTERNAL_RESOURCE },
		{ "PTR_INDIRECT_UAV",                  PTR_INDIRECT_UAV },
		{ "E_SC_USER_DATA_CLASS_LAST",         E_SC_USER_DATA_CLASS_LAST },
	}
};


static struct str_map_t enc_dict_machine_map =
{
	20, {
		{ "R600",	0 },
		{ "RV610",	1 },
		{ "RV630",	2 },
		{ "RV670",	3 },
		{ "R700",	4 },
		{ "RV770",	5 },
		{ "RV710",	6 },
		{ "RV730",	7 },
		{ "Cypress",	8 },
		{ "Juniper",	9 },
		{ "Redwood",	10 },
		{ "Cedar",	11 },
		{ "Sumo",	12 },
		{ "Supersumo",	13 },
		{ "Wrestler",	14 },
		{ "Cayman",	15 },
		{ "Kauai",	16 },
		{ "Barts",	17 },
		{ "Turks",	18 },
		{ "Caicos",	19 }
	}
};


struct str_map_t pt_note_type_map = {
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


static struct str_map_t prog_info_entry_map = {
	268, {
		{ "mmSQ_DYN_GPR_CNTL_PS_FLUSH_REQ",          0x00002363 },
		{ "mmSQ_GPR_RESOURCE_MGMT_1",                0x00002301 },
		{ "mmSQ_GPR_RESOURCE_MGMT_3__EG",            0x00002303 },
		{ "mmSQ_THREAD_RESOURCE_MGMT__EG",           0x00002306 },
		{ "mmSQ_THREAD_RESOURCE_MGMT_2__EG",         0x00002307 },
		{ "COMPUTE_PGM_RSRC2",                       0x00002e13 },
		{ "mmSPI_THREAD_GROUPING",                   0x0000a1b2 },
		{ "mmSPI_COMPUTE_INPUT_CNTL",                0x0000a1ba },
		{ "mmSPI_GPR_MGMT",                          0x0000a1be },
		{ "mmSPI_WAVE_MGMT_1",                       0x0000a1c1 },
		{ "mmSPI_WAVE_MGMT_2",                       0x0000a1c2 },
		{ "mmSQ_LDS_ALLOC",                          0x0000a23a },
		{ "mmSQ_PGM_START_LS",                       0x0000a234 },
		{ "mmSQ_PGM_RESOURCES_LS",                   0x0000a235 },
		{ "mmSQ_PGM_RESOURCES_2_LS",                 0x0000a236 },
		{ "AMU_ABI_PS_INPUT_COUNT",                  0x80000000 },
		{ "AMU_ABI_PS_INPUT_SPARSE",                 0x80000001 },
		{ "AMU_ABI_CS_MAX_SCRATCH_REGS",             0x80000002 },
		{ "AMU_ABI_CS_NUM_SHARED_GPR_USER",          0x80000003 },
		{ "AMU_ABI_CS_NUM_SHARED_GPR_TOTAL",         0x80000004 },
    		{ "AMU_ABI_ECS_SETUP_MODE",                  0x80000005 },
		{ "AMU_ABI_NUM_THREAD_PER_GROUP",            0x80000006 },
		{ "AMU_ABI_TOTAL_NUM_THREAD_GROUP",          0x80000007 },
		{ "AMU_ABI_MAX_WAVEFRONT_PER_SIMD",          0x80000009 },
		{ "AMU_ABI_NUM_WAVEFRONT_PER_SIMD",          0x8000000a },
		{ "AMU_ABI_IS_MAX_NUM_WAVE_PER_SIMD",        0x8000000b },
		{ "AMU_ABI_SET_BUFFER_FOR_NUM_GROUP",        0x8000000c },
		{ "AMU_ABI_VS_OUTPUT_COUNT",                 0x8000000d },
		{ "AMU_ABI_VS_OUTPUT_SPARSE",                0x8000000e },
		{ "AMU_ABI_VS_INPUT_COUNT",                  0x8000000f },
		{ "AMU_ABI_SET_BUFFER_FOR_UAV_RET_BUFFER0",  0x80000010 },
		{ "AMU_ABI_SET_BUFFER_FOR_UAV_RET_BUFFER1",  0x80000011 },
		{ "AMU_ABI_SET_BUFFER_FOR_UAV_RET_BUFFER2",  0x80000012 },
		{ "AMU_ABI_SET_BUFFER_FOR_UAV_RET_BUFFER3",  0x80000013 },
		{ "AMU_ABI_SET_BUFFER_FOR_UAV_RET_BUFFER4",  0x80000014 },
		{ "AMU_ABI_SET_BUFFER_FOR_UAV_RET_BUFFER5",  0x80000015 },
		{ "AMU_ABI_SET_BUFFER_FOR_UAV_RET_BUFFER6",  0x80000016 },
		{ "AMU_ABI_SET_BUFFER_FOR_UAV_RET_BUFFER7",  0x80000017 },
		{ "AMU_ABI_SET_BUFFER_FOR_UAV_RET_BUFFER8",  0x80000018 },
		{ "AMU_ABI_SET_BUFFER_FOR_UAV_RET_BUFFER9",  0x80000019 },
		{ "AMU_ABI_SET_BUFFER_FOR_UAV_RET_BUFFER10", 0x8000001a },
		{ "AMU_ABI_SET_BUFFER_FOR_UAV_RET_BUFFER11", 0x8000001b },
		{ "AMU_ABI_NUM_THREAD_PER_GROUP_X",          0x8000001c },
		{ "AMU_ABI_NUM_THREAD_PER_GROUP_Y",          0x8000001d },
		{ "AMU_ABI_NUM_THREAD_PER_GROUP_Z",          0x8000001e },
		{ "AMU_ABI_NUM_THREAD_PER_GROUP_X",          0x8000001c },
		{ "AMU_ABI_NUM_THREAD_PER_GROUP_Y",          0x8000001d },
		{ "AMU_ABI_NUM_THREAD_PER_GROUP_Z",          0x8000001e },
		{ "AMU_ABI_TOTAL_NUM_THREAD_GROUP",          0x80000007 },
		{ "AMU_ABI_NUM_WAVEFRONT_PER_SIMD",          0x8000000a },
		{ "AMU_ABI_IS_MAX_NUM_WAVE_PER_SIMD",        0x8000000b },
		{ "AMU_ABI_SET_BUFFER_FOR_NUM_GROUP",        0x8000000c },
		{ "AMU_ABI_RAT_OP_IS_USED",                  0x8000001f },
		{ "AMU_ABI_RAT_ATOMIC_OP_IS_USED",           0x80000020 },
		{ "AMU_ABI_GLOBAL_RETURN_BUFFER",            0x80000021 },
		{ "AMU_ABI_NUM_GLOBAL_UAV",                  0x80000022 },
		{ "AMU_ABI_NUM_GLOBAL_RETURN_BUFFER",        0x80000023 },
		{ "AMU_ABI_GLOBAL_RETURN_BUFFER_SIZE",       0x80000024 },
		{ "AMU_ABI_GLOBAL_RETURN_BUFFER_SIZE_SHORT", 0x80000025 },
		{ "AMU_ABI_GLOBAL_RETURN_BUFFER_SIZE_BYTE",  0x80000026 },
		{ "AMU_ABI_EXTENDED_CACHING",                0x80000027 },
		{ "AMU_ABI_OFFSET_TO_UAV0",                  0x80000030 },
		{ "AMU_ABI_OFFSET_TO_UAV1",                  0x80000031 },
		{ "AMU_ABI_OFFSET_TO_UAV2",                  0x80000032 },
		{ "AMU_ABI_OFFSET_TO_UAV3",                  0x80000033 },
		{ "AMU_ABI_OFFSET_TO_UAV4",                  0x80000034 },
		{ "AMU_ABI_OFFSET_TO_UAV5",                  0x80000035 },
		{ "AMU_ABI_OFFSET_TO_UAV6",                  0x80000036 },
		{ "AMU_ABI_OFFSET_TO_UAV7",                  0x80000037 },
		{ "AMU_ABI_OFFSET_TO_UAV8",                  0x80000038 },
		{ "AMU_ABI_OFFSET_TO_UAV9",                  0x80000039 },
		{ "AMU_ABI_OFFSET_TO_UAV10",                 0x8000003a },
		{ "AMU_ABI_OFFSET_TO_UAV11",                 0x8000003b },
		{ "AMU_ABI_UNCACHED_FETCH_CONST_UAV0",       0x80000040 },
		{ "AMU_ABI_UNCACHED_FETCH_CONST_UAV1",       0x80000041 },
		{ "AMU_ABI_UNCACHED_FETCH_CONST_UAV2",       0x80000042 },
		{ "AMU_ABI_UNCACHED_FETCH_CONST_UAV3",       0x80000043 },
		{ "AMU_ABI_UNCACHED_FETCH_CONST_UAV4",       0x80000044 },
		{ "AMU_ABI_UNCACHED_FETCH_CONST_UAV5",       0x80000045 },
		{ "AMU_ABI_UNCACHED_FETCH_CONST_UAV6",       0x80000046 },
		{ "AMU_ABI_UNCACHED_FETCH_CONST_UAV7",       0x80000047 },
		{ "AMU_ABI_UNCACHED_FETCH_CONST_UAV8",       0x80000048 },
		{ "AMU_ABI_UNCACHED_FETCH_CONST_UAV9",       0x80000049 },
		{ "AMU_ABI_UNCACHED_FETCH_CONST_UAV10",      0x8000004a },
		{ "AMU_ABI_UNCACHED_FETCH_CONST_UAV11",      0x8000004b },
		{ "AMU_ABI_CACHED_FETCH_CONST_UAV0",         0x80000050 },
		{ "AMU_ABI_CACHED_FETCH_CONST_UAV1",         0x80000051 },
		{ "AMU_ABI_CACHED_FETCH_CONST_UAV2",         0x80000052 },
		{ "AMU_ABI_CACHED_FETCH_CONST_UAV3",         0x80000053 },
		{ "AMU_ABI_CACHED_FETCH_CONST_UAV4",         0x80000054 },
		{ "AMU_ABI_CACHED_FETCH_CONST_UAV5",         0x80000055 },
		{ "AMU_ABI_CACHED_FETCH_CONST_UAV6",         0x80000056 },
		{ "AMU_ABI_CACHED_FETCH_CONST_UAV7",         0x80000057 },
		{ "AMU_ABI_CACHED_FETCH_CONST_UAV8",         0x80000058 },
		{ "AMU_ABI_CACHED_FETCH_CONST_UAV9",         0x80000059 },
		{ "AMU_ABI_CACHED_FETCH_CONST_UAV10",        0x8000005a },
		{ "AMU_ABI_CACHED_FETCH_CONST_UAV11",        0x8000005b },
		{ "AMU_ABI_GLOBAL_RETURN_FETCH_CONST0",      0x80000060 },
		{ "AMU_ABI_GLOBAL_RETURN_FETCH_CONST1",      0x80000061 },
		{ "AMU_ABI_GLOBAL_RETURN_FETCH_CONST2",      0x80000062 },
		{ "AMU_ABI_GLOBAL_RETURN_FETCH_CONST3",      0x80000063 },
		{ "AMU_ABI_GLOBAL_RETURN_FETCH_CONST4",      0x80000064 },
		{ "AMU_ABI_GLOBAL_RETURN_FETCH_CONST5",      0x80000065 },
		{ "AMU_ABI_GLOBAL_RETURN_FETCH_CONST6",      0x80000066 },
		{ "AMU_ABI_GLOBAL_RETURN_FETCH_CONST7",      0x80000067 },
		{ "AMU_ABI_GLOBAL_RETURN_BUFFER_TYPE0",      0x80000070 },
		{ "AMU_ABI_GLOBAL_RETURN_BUFFER_TYPE1",      0x80000071 },
		{ "AMU_ABI_GLOBAL_RETURN_BUFFER_TYPE2",      0x80000072 },
		{ "AMU_ABI_GLOBAL_RETURN_BUFFER_TYPE3",      0x80000073 },
		{ "AMU_ABI_GLOBAL_RETURN_BUFFER_TYPE4",      0x80000074 },
		{ "AMU_ABI_GLOBAL_RETURN_BUFFER_TYPE5",      0x80000075 },
		{ "AMU_ABI_GLOBAL_RETURN_BUFFER_TYPE6",      0x80000076 },
		{ "AMU_ABI_GLOBAL_RETURN_BUFFER_TYPE7",      0x80000077 },
		{ "AMU_ABI_WAVEFRONT_SIZE",                  0x80000078 },
		{ "AMU_ABI_NUM_GPR_AVAIL",                   0x80000079 },
		{ "AMU_ABI_NUM_GPR_USED",                    0x80000080 },
		{ "AMU_ABI_LDS_SIZE_AVAIL",                  0x80000081 },
		{ "AMU_ABI_LDS_SIZE_USED",                   0x80000082 },
		{ "AMU_ABI_STACK_SIZE_AVAIL",                0x80000083 },
		{ "AMU_ABI_STACK_SIZE_USED",                 0x80000084 },
		{ "AMU_ABI_CACHED_FETCH_CONST64_UAV0",       0x80000090 },
		{ "AMU_ABI_CACHED_FETCH_CONST64_UAV1",       0x80000091 },
		{ "AMU_ABI_CACHED_FETCH_CONST64_UAV2",       0x80000092 },
		{ "AMU_ABI_CACHED_FETCH_CONST64_UAV3",       0x80000093 },
		{ "AMU_ABI_CACHED_FETCH_CONST64_UAV4",       0x80000094 },
		{ "AMU_ABI_CACHED_FETCH_CONST64_UAV5",       0x80000095 },
		{ "AMU_ABI_CACHED_FETCH_CONST64_UAV6",       0x80000096 },
		{ "AMU_ABI_CACHED_FETCH_CONST64_UAV7",       0x80000097 },
		{ "AMU_ABI_CACHED_FETCH_CONST64_UAV8",       0x80000098 },
		{ "AMU_ABI_CACHED_FETCH_CONST64_UAV9",       0x80000099 },
		{ "AMU_ABI_CACHED_FETCH_CONST64_UAV10",      0x8000009a },
		{ "AMU_ABI_CACHED_FETCH_CONST64_UAV11",      0x8000009b },
		{ "AMU_ABI_CACHED_FETCH_CONST128_UAV0",      0x800000a0 },
		{ "AMU_ABI_CACHED_FETCH_CONST128_UAV1",      0x800000a1 },
		{ "AMU_ABI_CACHED_FETCH_CONST128_UAV2",      0x800000a2 },
		{ "AMU_ABI_CACHED_FETCH_CONST128_UAV3",      0x800000a3 },
		{ "AMU_ABI_CACHED_FETCH_CONST128_UAV4",      0x800000a4 },
		{ "AMU_ABI_CACHED_FETCH_CONST128_UAV5",      0x800000a5 },
		{ "AMU_ABI_CACHED_FETCH_CONST128_UAV6",      0x800000a6 },
		{ "AMU_ABI_CACHED_FETCH_CONST128_UAV7",      0x800000a7 },
		{ "AMU_ABI_CACHED_FETCH_CONST128_UAV8",      0x800000a8 },
		{ "AMU_ABI_CACHED_FETCH_CONST128_UAV9",      0x800000a9 },
		{ "AMU_ABI_CACHED_FETCH_CONST128_UAV10",     0x800000aa },
		{ "AMU_ABI_CACHED_FETCH_CONST128_UAV11",     0x800000ab },

		/* Southern Islands Related Fields */

		{ "AMU_ABI_USER_ELEMENT_COUNT",              0x80001000 },
		{ "AMU_ABI_USER_ELEMENTS_0_DWORD0",          0x80001001 },
		{ "AMU_ABI_USER_ELEMENTS_0_DWORD1",          0x80001002 },
		{ "AMU_ABI_USER_ELEMENTS_0_DWORD2",          0x80001003 },
		{ "AMU_ABI_USER_ELEMENTS_0_DWORD3",          0x80001004 },
		{ "AMU_ABI_USER_ELEMENTS_1_DWORD0",          0x80001005 },
		{ "AMU_ABI_USER_ELEMENTS_1_DWORD1",          0x80001006 },
		{ "AMU_ABI_USER_ELEMENTS_1_DWORD2",          0x80001007 },
		{ "AMU_ABI_USER_ELEMENTS_1_DWORD3",          0x80001008 },
		{ "AMU_ABI_USER_ELEMENTS_2_DWORD0",          0x80001009 },
		{ "AMU_ABI_USER_ELEMENTS_2_DWORD1",          0x8000100a },
		{ "AMU_ABI_USER_ELEMENTS_2_DWORD2",          0x8000100b },
		{ "AMU_ABI_USER_ELEMENTS_2_DWORD3",          0x8000100c },
		{ "AMU_ABI_USER_ELEMENTS_3_DWORD0",          0x8000100d },
		{ "AMU_ABI_USER_ELEMENTS_3_DWORD1",          0x8000100e },
		{ "AMU_ABI_USER_ELEMENTS_3_DWORD2",          0x8000100f },
		{ "AMU_ABI_USER_ELEMENTS_3_DWORD3",          0x80001010 },
		{ "AMU_ABI_USER_ELEMENTS_4_DWORD0",          0x80001011 },
		{ "AMU_ABI_USER_ELEMENTS_4_DWORD1",          0x80001012 },
		{ "AMU_ABI_USER_ELEMENTS_4_DWORD2",          0x80001013 },
		{ "AMU_ABI_USER_ELEMENTS_4_DWORD3",          0x80001014 },
		{ "AMU_ABI_USER_ELEMENTS_5_DWORD0",          0x80001015 },
		{ "AMU_ABI_USER_ELEMENTS_5_DWORD1",          0x80001016 },
		{ "AMU_ABI_USER_ELEMENTS_5_DWORD2",          0x80001017 },
		{ "AMU_ABI_USER_ELEMENTS_5_DWORD3",          0x80001018 },
		{ "AMU_ABI_USER_ELEMENTS_6_DWORD0",          0x80001019 },
		{ "AMU_ABI_USER_ELEMENTS_6_DWORD1",          0x8000101a },
		{ "AMU_ABI_USER_ELEMENTS_6_DWORD2",          0x8000101b },
		{ "AMU_ABI_USER_ELEMENTS_6_DWORD3",          0x8000101c },
		{ "AMU_ABI_USER_ELEMENTS_7_DWORD0",          0x8000101d },
		{ "AMU_ABI_USER_ELEMENTS_7_DWORD1",          0x8000101e },
		{ "AMU_ABI_USER_ELEMENTS_7_DWORD2",          0x8000101f },
		{ "AMU_ABI_USER_ELEMENTS_7_DWORD3",          0x80001020 },
		{ "AMU_ABI_USER_ELEMENTS_8_DWORD0",          0x80001021 },
		{ "AMU_ABI_USER_ELEMENTS_8_DWORD1",          0x80001022 },
		{ "AMU_ABI_USER_ELEMENTS_8_DWORD2",          0x80001023 },
		{ "AMU_ABI_USER_ELEMENTS_8_DWORD3",          0x80001024 },
		{ "AMU_ABI_USER_ELEMENTS_9_DWORD0",          0x80001025 },
		{ "AMU_ABI_USER_ELEMENTS_9_DWORD1",          0x80001026 },
		{ "AMU_ABI_USER_ELEMENTS_9_DWORD2",          0x80001027 },
		{ "AMU_ABI_USER_ELEMENTS_9_DWORD3",          0x80001028 },
		{ "AMU_ABI_USER_ELEMENTS_10_DWORD0",         0x80001029 },
		{ "AMU_ABI_USER_ELEMENTS_10_DWORD1",         0x8000102a },
		{ "AMU_ABI_USER_ELEMENTS_10_DWORD2",         0x8000102b },
		{ "AMU_ABI_USER_ELEMENTS_10_DWORD3",         0x8000102c },
		{ "AMU_ABI_USER_ELEMENTS_11_DWORD0",         0x8000102d },
		{ "AMU_ABI_USER_ELEMENTS_11_DWORD1",         0x8000102e },
		{ "AMU_ABI_USER_ELEMENTS_11_DWORD2",         0x8000102f },
		{ "AMU_ABI_USER_ELEMENTS_11_DWORD3",         0x80001030 },
		{ "AMU_ABI_USER_ELEMENTS_12_DWORD0",         0x80001031 },
		{ "AMU_ABI_USER_ELEMENTS_12_DWORD1",         0x80001032 },
		{ "AMU_ABI_USER_ELEMENTS_12_DWORD2",         0x80001033 },
		{ "AMU_ABI_USER_ELEMENTS_12_DWORD3",         0x80001034 },
		{ "AMU_ABI_USER_ELEMENTS_13_DWORD0",         0x80001035 },
		{ "AMU_ABI_USER_ELEMENTS_13_DWORD1",         0x80001036 },
		{ "AMU_ABI_USER_ELEMENTS_13_DWORD2",         0x80001037 },
		{ "AMU_ABI_USER_ELEMENTS_13_DWORD3",         0x80001038 },
		{ "AMU_ABI_USER_ELEMENTS_14_DWORD0",         0x80001039 },
		{ "AMU_ABI_USER_ELEMENTS_14_DWORD1",         0x8000103a },
		{ "AMU_ABI_USER_ELEMENTS_14_DWORD2",         0x8000103b },
		{ "AMU_ABI_USER_ELEMENTS_14_DWORD3",         0x8000103c },
		{ "AMU_ABI_USER_ELEMENTS_15_DWORD0",         0x8000103d },
		{ "AMU_ABI_USER_ELEMENTS_15_DWORD1",         0x8000103e },
		{ "AMU_ABI_USER_ELEMENTS_15_DWORD2",         0x8000103f },
		{ "AMU_ABI_USER_ELEMENTS_15_DWORD3",         0x80001040 },
		{ "AMU_ABI_SI_NUM_VGPRS",                    0x80001041 },
		{ "AMU_ABI_SI_NUM_SGPRS",                    0x80001042 },
		{ "AMU_ABI_SI_FLOAT_MODE",                   0x80001043 },
		{ "AMU_ABI_SI_IEEE_MODE",                    0x80001044 },
		{ "AMU_ABI_SI_SCRATCH_SIZE",                 0x80001045 },
		{ "AMU_ABI_POS_FLOAT_LOCATION",              0x80001046 },
		{ "AMU_ABI_EXPORT_PATCH_CODE_SIZE",          0x80001047 },
		{ "AMU_ABI_EXPORT_PATCH_OFFSET_0",           0x80001048 },
		{ "AMU_ABI_EXPORT_PATCH_OFFSET_1",           0x80001049 },
		{ "AMU_ABI_EXPORT_PATCH_OFFSET_2",           0x8000104a },
		{ "AMU_ABI_EXPORT_PATCH_OFFSET_3",           0x8000104b },
		{ "AMU_ABI_EXPORT_PATCH_OFFSET_4",           0x8000104c },
		{ "AMU_ABI_EXPORT_PATCH_OFFSET_5",           0x8000104d },
		{ "AMU_ABI_EXPORT_PATCH_OFFSET_6",           0x8000104e },
		{ "AMU_ABI_EXPORT_PATCH_OFFSET_7",           0x8000104f },
		/* From 0x80001100 to 0x8000123f, there are 320 entries
		 * of the format: 
		 * AMU_ABI_EXPORT_PATCH_CODE_EXPORT0_FMT0_DWORD0
		 * AMU_ABI_EXPORT_PATCH_CODE_EXPORT0_FMT0_DWORD1
		 * AMU_ABI_EXPORT_PATCH_CODE_EXPORT0_FMT0_DWORD2
		 * AMU_ABI_EXPORT_PATCH_CODE_EXPORT0_FMT0_DWORD3
		 * AMU_ABI_EXPORT_PATCH_CODE_EXPORT0_FMT1_DWORD0
		 * ...
		 * AMU_ABI_EXPORT_PATCH_CODE_EXPORT7_FMT9_DWORD3
		 */
		{ "AMU_ABI_VS_IN_SEMANTICS_0",                0x80001800 },
		/*", There are 32 input semantics */
		{ "AMU_ABI_VS_IN_SEMANTICS_31",               0x8000181f },
		{ "AMU_ABI_VS_OUT_SEMANTICS_0",               0x80001820 },
		/*", There are 32 output semantics */
		{ "AMU_ABI_VS_OUT_SEMANTICS_31",              0x8000183f },
		{ "AMU_ABI_DX9_TEX_CUBE_MASK",                0x80001840 },
		{ "AMU_ABI_ORDERED_APPEND_ENABLE",            0x80001841 },
		{ "AMU_ABI_SI_VGPR_COMP_CNT",                 0x80001842 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_0",              0x80001843 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_1",              0x80001844 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_2",              0x80001845 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_3",              0x80001846 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_4",              0x80001847 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_5",              0x80001848 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_6",              0x80001849 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_7",              0x8000184a },
		{ "AMU_ABI_UAV_RESOURCE_MASK_8",              0x8000184b },
		{ "AMU_ABI_UAV_RESOURCE_MASK_9",              0x8000184c },
		{ "AMU_ABI_UAV_RESOURCE_MASK_10",             0x8000184d },
		{ "AMU_ABI_UAV_RESOURCE_MASK_11",             0x8000184e },
		{ "AMU_ABI_UAV_RESOURCE_MASK_12",             0x8000184f },
		{ "AMU_ABI_UAV_RESOURCE_MASK_13",             0x80001850 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_14",             0x80001851 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_15",             0x80001852 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_16",             0x80001853 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_17",             0x80001854 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_18",             0x80001855 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_19",             0x80001856 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_20",             0x80001857 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_21",             0x80001858 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_22",             0x80001859 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_23",             0x8000185a },
		{ "AMU_ABI_UAV_RESOURCE_MASK_24",             0x8000185b },
		{ "AMU_ABI_UAV_RESOURCE_MASK_25",             0x8000185c },
		{ "AMU_ABI_UAV_RESOURCE_MASK_26",             0x8000185d },
		{ "AMU_ABI_UAV_RESOURCE_MASK_27",             0x8000185e },
		{ "AMU_ABI_UAV_RESOURCE_MASK_28",             0x8000185f },
		{ "AMU_ABI_UAV_RESOURCE_MASK_29",             0x80001860 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_30",             0x80001861 },
		{ "AMU_ABI_UAV_RESOURCE_MASK_31",             0x80001862 },
		{ "AMU_ABI_SI_NUM_SGPRS_AVAIL",               0x80001863 },
		{ "AMU_ABI_SI_NUM_VGPRS_AVAIL",               0x80001864 }
	}
};







/* Read next note at the current position of the PT_NOTE segment */
static void si_bin_file_read_note_header(struct si_bin_file_t *bin_file, struct si_bin_enc_dict_entry_t *enc_dict_entry)
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
		fatal("%s: error decoding note header", bin_file->elf_file->path);
	
	/* Read note description (payload) */
	desc = elf_buffer_tell(buffer);
	count = elf_buffer_read(buffer, NULL, header->descsz);
	if (count < header->descsz)
		fatal("%s: error decoding note description", bin_file->elf_file->path);

	/* Debug */
	note_type_str = str_map_value(&pt_note_type_map, header->type);
	elf_debug("  note: type=%d (%s), descsz=%d\n",
		header->type, note_type_str, header->descsz);
		
	/* Analyze note */
	switch (header->type)
	{

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
		for (i = 0; i < prog_info_count; i++)
		{
			prog_info_entry = desc + i * sizeof(struct pt_note_prog_info_entry_t);
			elf_debug("\tprog_info_entry: addr=0x%x (%s), value=%u\n",
				prog_info_entry->address, str_map_value(&prog_info_entry_map,
				prog_info_entry->address), prog_info_entry->value);

			/* Analyze entry */
			switch (prog_info_entry->address)
			{

			case 0x00002e13:  /* COMPUTE_PGM_RSRC2 */
			enc_dict_entry->compute_pgm_rsrc2 = 
				(struct si_bin_compute_pgm_rsrc2_t*)&prog_info_entry->value;
			break;

			case 0x80000082:  /* AMU_ABI_LDS_SIZE_USED */
				enc_dict_entry->lds_size_used = prog_info_entry->value;
				break;

			case 0x80000084:  /* AMU_ABI_STACK_SIZE_USED */
				enc_dict_entry->stack_size_used = prog_info_entry->value;
				break;

			case 0x80001000:  /* AMU_ABI_USER_ELEMENT_COUNT */
				enc_dict_entry->userElementCount = prog_info_entry->value;
				i++;

				/* Analyze user elements */
				for(int j = 0; j < 4 * enc_dict_entry->userElementCount; j++)
				{
					prog_info_entry = desc + i * sizeof(struct pt_note_prog_info_entry_t);
					elf_debug("\tprog_info_entry: addr=0x%x (%s), value=%u\n",
							prog_info_entry->address, str_map_value(&prog_info_entry_map,
							prog_info_entry->address), prog_info_entry->value);
					switch(j % 4)
					{
					case 0:
						enc_dict_entry->userElements[j / 4].dataClass = prog_info_entry->value;
						break;
					case 1:
						enc_dict_entry->userElements[j / 4].apiSlot = prog_info_entry->value;
						break;
					case 2:
						enc_dict_entry->userElements[j / 4].startUserReg = prog_info_entry->value;
						break;
					case 3:
						enc_dict_entry->userElements[j / 4].userRegCount = prog_info_entry->value;
						break;
					}

					i++;
				}
				break;

			case 0x80001041:  /* NUM VGPRS */
				enc_dict_entry->num_vgpr_used = prog_info_entry->value;
				break;

			case 0x80001042:  /* NUM SGPRS */
				enc_dict_entry->num_sgpr_used = prog_info_entry->value;
				break;

			}


		}
		break;
	}

	
	case 2:  /* ELF_NOTE_ATI_INPUTS */
	{
		/* FIXME: Analyze program inputs */
		if (header->descsz) 
			warning("%s: pt_note '%s' with descsz != 0 ignored (desc value = 0x%x)",
				note_type_str, __FUNCTION__, * (unsigned int *) desc);

		break;
	}


	case 3:  /* ELF_NOTE_ATI_OUTPUTS */
	{
		/* FIXME: Analyze program inputs */
		if (header->descsz) 
			warning("%s: pt_note '%s' with descsz != 0 ignored (desc value = 0x%x)",
				note_type_str, __FUNCTION__, * (unsigned int *) desc);

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

		struct si_bin_enc_dict_entry_consts_t *consts;
		char const_value[MAX_STRING_SIZE];

		int j;

		/* Get number of entries */
		consts = enc_dict_entry->consts;
		assert(header->descsz % sizeof(struct pt_note_data_segment_desc_t) == 0);
		data_segment_desc_count = header->descsz / sizeof(struct pt_note_data_segment_desc_t);
		elf_debug("\tnote including data for constant buffers (%d entries)\n",
				data_segment_desc_count);

		/* Decode entries */
		for (j = 0; j < data_segment_desc_count; j++)
		{
			data_segment_desc = desc + j * sizeof(struct pt_note_data_segment_desc_t);
			if (header->type == 5)
				snprintf(const_value, sizeof(const_value), "{%g,%g,%g,%g}",
					consts->float_consts[data_segment_desc->offset][0],
					consts->float_consts[data_segment_desc->offset][1],
					consts->float_consts[data_segment_desc->offset][2],
					consts->float_consts[data_segment_desc->offset][3]);
			else if (header->type == 6)
				snprintf(const_value, sizeof(const_value), "{%u,%u,%u,%u}",
					consts->int_consts[data_segment_desc->offset][0],
					consts->int_consts[data_segment_desc->offset][1],
					consts->int_consts[data_segment_desc->offset][2],
					consts->int_consts[data_segment_desc->offset][3]);
			else
				snprintf(const_value, sizeof(const_value), "%d",
					consts->bool_consts[data_segment_desc->offset]);
			elf_debug("\tdata_segment_desc[%d]: offset=0x%x, size=%d, value=%s\n",
				j, data_segment_desc->offset, data_segment_desc->size, const_value);
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
		assert(header->descsz % 
			sizeof(struct pt_note_constant_buffer_mask_t) == 0);
		constant_buffer_count = header->descsz / 
			sizeof(struct pt_note_constant_buffer_mask_t);
		elf_debug("\tnote including number and size of constant "
			"buffers (%d entries)\n", constant_buffer_count);

		/* Decode entries */
		for (i = 0; i < constant_buffer_count; i++) 
		{
			constant_buffer_mask = desc + i * 
				sizeof(struct pt_note_constant_buffer_mask_t);
			elf_debug("\tconstant_buffer[%d].size = %d "
				"(vec4f constants)\n", 
				constant_buffer_mask->index, 
				constant_buffer_mask->size);
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
	{	
		int uav_entry_count;
		struct pt_note_uav_entry_t *uav_entry;
		int i;

		assert(header->descsz % sizeof(struct pt_note_uav_entry_t) == 0);
		uav_entry_count = header->descsz / sizeof(struct pt_note_uav_entry_t);
		elf_debug("\tnote (%d entries)\n", uav_entry_count);

		/* Decode entries */
		for (i = 0; i < uav_entry_count; i++) {
			uav_entry = desc + i * sizeof(struct pt_note_uav_entry_t);
			elf_debug("\tuav_entry[%d].uav = %d [%d, %d, %d]\n", i, uav_entry->id,
				uav_entry->unknown1, uav_entry->unknown2, uav_entry->unknown3);
		}
		break;
	}	


	case 17:  /* ELF_NOTE_ATI_UAV_OP_MASK */
		break;

	default:
		elf_debug("\tunknown type\n");
	}
}


/* Decode notes in the PT_NOTE segment of the given encoding dictionary entry */
static void si_bin_file_read_notes(struct si_bin_file_t *bin_file, struct si_bin_enc_dict_entry_t *enc_dict_entry)
{
	struct elf_buffer_t *buffer;

	/* Get buffer and set position */
	buffer = &enc_dict_entry->pt_note_buffer;
	elf_buffer_seek(buffer, 0);

	/* Decode notes */
	elf_debug("Reading notes in PT_NOTE segment (enc. dict. for machine=0x%x)\n",
		enc_dict_entry->header->d_machine);
	while (buffer->pos < buffer->size)
		si_bin_file_read_note_header(bin_file, enc_dict_entry);
	elf_debug("\n\n\n");
}


static void si_bin_file_read_enc_dict(struct si_bin_file_t *bin_file)
{
	struct elf_file_t *elf_file;
	struct elf_buffer_t *buffer;
	Elf32_Ehdr *elf_header;

	struct elf_program_header_t *program_header;

	struct si_bin_enc_dict_entry_t *enc_dict_entry;
	struct si_bin_enc_dict_entry_header_t *enc_dict_entry_header;
	int enc_dict_entry_count;

	int i;

	/* ELF header */
	elf_file = bin_file->elf_file;
	elf_header = elf_file->header;
	buffer = &elf_file->buffer;
	elf_debug("**\n** Parsing AMD Binary (Internal ELF file)\n** %s\n**\n\n", elf_file->path);
	SI_BIN_FILE_NOT_SUPPORTED_NEQ(elf_header->e_ident[EI_CLASS], ELFCLASS32);
	SI_BIN_FILE_NOT_SUPPORTED_NEQ(elf_header->e_ident[EI_DATA], ELFDATA2LSB);
	SI_BIN_FILE_NOT_SUPPORTED_NEQ(elf_header->e_ident[EI_OSABI], 0x64);
	SI_BIN_FILE_NOT_SUPPORTED_NEQ(elf_header->e_ident[EI_ABIVERSION], 1);
	SI_BIN_FILE_NOT_SUPPORTED_NEQ(elf_header->e_type, ET_EXEC);
	SI_BIN_FILE_NOT_SUPPORTED_NEQ(elf_header->e_machine, 0x7d);
	SI_BIN_FILE_NOT_SUPPORTED_NEQ(elf_header->e_entry, 0);
	
	/* Look for encoding dictionary (program header with type 'PT_LOPROC+2') */
	program_header = NULL;
	for (i = 0; i < list_count(elf_file->program_header_list); i++)
	{
		program_header = list_get(elf_file->program_header_list, i);
		if (program_header->header->p_type == PT_LOPROC + 2)
			break;
	}
	if (i == list_count(elf_file->program_header_list) || !program_header)
		fatal("%s: no encoding dictionary", elf_file->path);
	elf_debug("Encoding dictionary found in program header %d\n", i);
	
	/* Parse encoding dictionary */
	SI_BIN_FILE_NOT_SUPPORTED_NEQ(program_header->header->p_vaddr, 0);
	SI_BIN_FILE_NOT_SUPPORTED_NEQ(program_header->header->p_paddr, 0);
	SI_BIN_FILE_NOT_SUPPORTED_NEQ(program_header->header->p_memsz, 0);
	SI_BIN_FILE_NOT_SUPPORTED_NEQ(program_header->header->p_flags, 0);
	SI_BIN_FILE_NOT_SUPPORTED_NEQ(program_header->header->p_align, 0);
	assert(program_header->header->p_filesz % sizeof(struct si_bin_enc_dict_entry_header_t) == 0);
	enc_dict_entry_count = program_header->header->p_filesz / sizeof(struct si_bin_enc_dict_entry_header_t);
	elf_debug("  -> %d entries\n\n", enc_dict_entry_count);

	/* Read encoding dictionary entries */
	bin_file->enc_dict = list_create();
	elf_buffer_seek(buffer, program_header->header->p_offset);
	for (i = 0; i < enc_dict_entry_count; i++)
	{
		/* Create entry */
		enc_dict_entry = xcalloc(1, sizeof(struct si_bin_enc_dict_entry_t));
		enc_dict_entry->header = elf_buffer_tell(buffer);
		elf_buffer_read(buffer, NULL, sizeof(struct si_bin_enc_dict_entry_header_t));
		list_add(bin_file->enc_dict, enc_dict_entry);

		/* Store encoding dictionary entry for Southern Islands.
		 * Apparently the valid code changes by driver version */
		if (enc_dict_entry->header->d_machine == 9)
		{
			/* Driver XXX */
			elf_debug("machine = %d (tahiti or pitcairn)\n", 
				enc_dict_entry->header->d_machine);
			bin_file->enc_dict_entry_southern_islands = 
				enc_dict_entry;
		}
		else if (enc_dict_entry->header->d_machine == 25)
		{
			/* This entry is always present but doesn't seem
			 * useful information.  We should probably figure
			 * out what is stored here. */
			elf_debug("machine = 25 (skip this entry)\n");
		}
		else if (enc_dict_entry->header->d_machine == 26)
		{
			/* Driver XXX */
			elf_debug("machine = %d (tahiti or pitcairn)\n", 
				enc_dict_entry->header->d_machine);
			bin_file->enc_dict_entry_southern_islands = 
				enc_dict_entry;
		}
		else if (enc_dict_entry->header->d_machine == 27)
		{
			/* Driver 12.4 */
			elf_debug("machine = %d (tahiti or pitcairn)\n", 
				enc_dict_entry->header->d_machine);
			bin_file->enc_dict_entry_southern_islands = 
				enc_dict_entry;
		}
		else if (enc_dict_entry->header->d_machine == 28)
		{
			elf_debug("machine = %d (capeverde)\n", 
				enc_dict_entry->header->d_machine);
			bin_file->enc_dict_entry_southern_islands = 
				enc_dict_entry;
		}
		else
	 	{
			fatal("%s: unknown machine number (%d)\n", __FUNCTION__, 
				enc_dict_entry->header->d_machine);
		}
	}

	/* Debug */
	elf_debug("idx %-15s %-10s %-10s %-10s %-10s\n", "d_machine", "d_type",
		"d_offset", "d_size", "d_flags");
	for (i = 0; i < 80; i++)
		elf_debug("-");
	elf_debug("\n");
	for (i = 0; i < list_count(bin_file->enc_dict); i++)
	{
		char machine_str[MAX_STRING_SIZE];

		enc_dict_entry = list_get(bin_file->enc_dict, i);
		enc_dict_entry_header = enc_dict_entry->header;
		snprintf(machine_str, sizeof(machine_str), "%d (%s)",
			enc_dict_entry_header->d_machine, str_map_value(&enc_dict_machine_map,
			enc_dict_entry_header->d_machine - 1));
		elf_debug("%3d %-15s 0x%-8x 0x%-8x %-10d 0x%-8x\n",
			i, machine_str,
			enc_dict_entry_header->d_type,
			enc_dict_entry_header->d_offset,
			enc_dict_entry_header->d_size,
			enc_dict_entry_header->d_flags);
	}
	elf_debug("\n\n");
}


static void si_bin_file_read_segments(struct si_bin_file_t *bin_file)
{
	struct elf_file_t *elf_file;

	struct si_bin_enc_dict_entry_t *enc_dict_entry;
	struct elf_program_header_t *program_header;

	int i, j;

	elf_debug("Reading PT_NOTE and PT_LOAD segments:\n");
	elf_file = bin_file->elf_file;
	for (i = 0; i < list_count(bin_file->enc_dict); i++)
	{
		/* Get encoding dictionary entry */
		enc_dict_entry = list_get(bin_file->enc_dict, i);
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
			if (program_header->header->p_type == PT_NOTE)
			{
				if (enc_dict_entry->pt_note_buffer.size)
					fatal("%s: more than one PT_NOTE for encoding dictionary entry", __FUNCTION__);
				enc_dict_entry->pt_note_buffer.ptr = elf_file->buffer.ptr + program_header->header->p_offset;
				enc_dict_entry->pt_note_buffer.size = program_header->header->p_filesz;
				enc_dict_entry->pt_note_buffer.pos = 0;
			}

			/* Segment PT_LOAD */
			if (program_header->header->p_type == PT_LOAD)
			{
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


static void si_bin_file_read_sections(struct si_bin_file_t *bin_file)
{
	struct elf_file_t *elf_file;
	struct elf_buffer_t *buffer;

	struct si_bin_enc_dict_entry_t *enc_dict_entry;
	struct elf_section_t *section;

	int i, j;

	int pt_load_offset;
	int pt_load_size;

	elf_file = bin_file->elf_file;
	for (i = 0; i < list_count(bin_file->enc_dict); i++)
	{
		/* Get encoding dictionary entry */
		enc_dict_entry = list_get(bin_file->enc_dict, i);
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
			if (!strcmp(section->name, ".text"))
			{
				if (enc_dict_entry->sec_text_buffer.size)
					fatal("%s: duplicated '.text' section", __FUNCTION__);
				enc_dict_entry->sec_text_buffer.ptr = elf_file->buffer.ptr + section->header->sh_offset;
				enc_dict_entry->sec_text_buffer.size = section->header->sh_size;
				enc_dict_entry->sec_text_buffer.pos = 0;
			}
			else if (!strcmp(section->name, ".data"))
			{
				buffer = &enc_dict_entry->sec_data_buffer;
				if (buffer->size)
					fatal("%s: duplicated '.data' section", __FUNCTION__);
				buffer->ptr = elf_file->buffer.ptr + section->header->sh_offset;
				buffer->size = section->header->sh_size;
				buffer->pos = 0;
				if (buffer->size != 4736)
					fatal("%s: '.data' section expected to be 4736 bytes", __FUNCTION__);
				enc_dict_entry->consts = buffer->ptr;
			}
			else if (!strcmp(section->name, ".symtab"))
			{
				if (enc_dict_entry->sec_symtab_buffer.size)
					fatal("%s: duplicated '.symtab' section", __FUNCTION__);
				enc_dict_entry->sec_symtab_buffer.ptr = elf_file->buffer.ptr + section->header->sh_offset;
				enc_dict_entry->sec_symtab_buffer.size = section->header->sh_size;
				enc_dict_entry->sec_symtab_buffer.pos = 0;
			}
			else if (!strcmp(section->name, ".strtab"))
			{
				if (enc_dict_entry->sec_strtab_buffer.size)
					fatal("%s: duplicated '.strtab' section", __FUNCTION__);
				enc_dict_entry->sec_strtab_buffer.ptr = elf_file->buffer.ptr + section->header->sh_offset;
				enc_dict_entry->sec_strtab_buffer.size = section->header->sh_size;
				enc_dict_entry->sec_strtab_buffer.pos = 0;
			}
			else
			{
				fatal("%s: not recognized section name: '%s'",
					__FUNCTION__, section->name);
			}
		}

		/* Check that all sections where read */
		if (!enc_dict_entry->sec_text_buffer.size
			|| !enc_dict_entry->sec_data_buffer.size
			|| !enc_dict_entry->sec_symtab_buffer.size
			|| !enc_dict_entry->sec_strtab_buffer.size)
		{
			fatal("%s: some section was not found: .text .data"
				" .symtab .strtab", __FUNCTION__);
		}
	}

	/* Finish */
	elf_debug("\n");
}





/*
 * Public functions
 */


struct si_bin_file_t *si_bin_file_create(void *ptr, int size, char *name)
{
	struct si_bin_file_t *bin_file;

	/* Create structure */
	bin_file = xcalloc(1, sizeof(struct si_bin_file_t));

	/* Read and parse ELF file */
	bin_file->elf_file = elf_file_create_from_buffer(ptr, size, name);

	/* Read encoding dictionary.
	 * Check that a Southern Islands dictionary entry is present */
	si_bin_file_read_enc_dict(bin_file);
	if (!bin_file->enc_dict_entry_southern_islands)
		fatal(
	"%s: no encoding dictionary entry for Southern Islands.\n"
	"\tThe OpenCL kernel binary that your application is trying to load\n"
	"\tdoes not contain Southern Islands assembly code. Please make\n" 
	"\tsure that a Tahiti device is selected when compiling the OpenCL\n"
	"\tkernel source. In some cases, even a proper selection of this\n"
	"\tarchitecture causes Southern Islands assembly not to be included\n"
	"\tif the APP SDK is not correctly installed when compiling your\n"
	"\town kernel sources.\n", 
			bin_file->elf_file->path);
	
	/* Read segments and sections */
	si_bin_file_read_segments(bin_file);
	si_bin_file_read_sections(bin_file);

	/* Read notes in PT_NOTE segment for Southern Islands 
	 * dictionary entry */
	si_bin_file_read_notes(bin_file, bin_file->
		enc_dict_entry_southern_islands);

	/* Return */
	return bin_file;
}


void si_bin_file_free(struct si_bin_file_t *bin_file)
{
	/* Free encoding dictionary */
	while (list_count(bin_file->enc_dict))
	{
		free(list_remove_at(bin_file->enc_dict, 0));
	}
	list_free(bin_file->enc_dict);

	/* Free rest */
	elf_file_free(bin_file->elf_file);
	free(bin_file);
}

struct si_bin_enc_user_element_t *si_bin_enc_user_element_create()
{
	struct si_bin_enc_user_element_t *user_elem;

	/* Initialize */
	user_elem = xcalloc(1, sizeof(struct si_bin_enc_user_element_t));
	
	/* Return */
	return user_elem;
}

void si_bin_enc_user_element_free(struct si_bin_enc_user_element_t *user_elem)
{
	free(user_elem);
}

struct si_bin_compute_pgm_rsrc2_t *si_bin_compute_pgm_rsrc2_create()
{
	struct si_bin_compute_pgm_rsrc2_t *pgm_rsrc2;

	/* Initialize */
	pgm_rsrc2 = xcalloc(1, sizeof(struct si_bin_compute_pgm_rsrc2_t));
	
	/* Return */
	return pgm_rsrc2;
}

void si_bin_compute_pgm_rsrc2_free(struct si_bin_compute_pgm_rsrc2_t *pgm_rsrc2)
{
	free(pgm_rsrc2);
}

