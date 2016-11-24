/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#include "AsmService.h"

namespace HSA
{

misc::StringMap AsmService::opcode_to_str_map = 
{
#define DEFINST(name, opcode, opstr ) \
	{opstr, opcode},	
#include "Instruction.def"
#undef DEFINST

	{"invalid", 65535}
};


misc::StringMap AsmService::type_to_str_map = 
{
	{"", BRIG_TYPE_NONE},
	{"u8", BRIG_TYPE_U8},
	{"u16", BRIG_TYPE_U16},
	{"u32", BRIG_TYPE_U32},
	{"u64", BRIG_TYPE_U64},
	{"s8", BRIG_TYPE_S8},
	{"s16", BRIG_TYPE_S16},
	{"s32", BRIG_TYPE_S32},
	{"s64", BRIG_TYPE_S64},
	{"f16", BRIG_TYPE_F16},
	{"f32", BRIG_TYPE_F32},
	{"f64", BRIG_TYPE_F64},
	{"b1", BRIG_TYPE_B1},
	{"b8", BRIG_TYPE_B8},
	{"b16", BRIG_TYPE_B16},
	{"b32", BRIG_TYPE_B32},
	{"b64", BRIG_TYPE_B64},
	{"b128", BRIG_TYPE_B128},
	{"samp", BRIG_TYPE_SAMP},
	{"roimg", BRIG_TYPE_ROIMG},
	{"woimg", BRIG_TYPE_WOIMG},
	{"rwimg", BRIG_TYPE_RWIMG},
	{"sig32", BRIG_TYPE_SIG32},
	{"sig64", BRIG_TYPE_SIG64},
	{"u8x4", BRIG_TYPE_U8X4},
	{"u8x8", BRIG_TYPE_U8X8},
	{"u8x16", BRIG_TYPE_U8X16},
	{"u16x2", BRIG_TYPE_U16X2},
	{"u16x4", BRIG_TYPE_U16X4},
	{"u16x8", BRIG_TYPE_U16X8},
	{"u32x2", BRIG_TYPE_U32X2},
	{"u32x4", BRIG_TYPE_U32X4},
	{"u64x2", BRIG_TYPE_U64X2},
	{"s8x4", BRIG_TYPE_S8X4},
	{"s8x8", BRIG_TYPE_S8X8},
	{"s8x16", BRIG_TYPE_S8X16},
	{"s16x2", BRIG_TYPE_S16X2},
	{"s16x4", BRIG_TYPE_S16X4},
	{"s16x8", BRIG_TYPE_S16X8},
	{"s32x2", BRIG_TYPE_S32X2},
	{"s32x4", BRIG_TYPE_S32X4},
	{"s64x2", BRIG_TYPE_S64X2},
	{"f16x2", BRIG_TYPE_F16X2},
	{"f16x4", BRIG_TYPE_F16X4},
	{"f16x8", BRIG_TYPE_F16X8},
	{"f32x2", BRIG_TYPE_F32X2},
	{"f32x4", BRIG_TYPE_F32X4},
	{"f64x2", BRIG_TYPE_F64X2},

	{"u8", BRIG_TYPE_U8_ARRAY},
	{"u16", BRIG_TYPE_U16_ARRAY},
	{"u32", BRIG_TYPE_U32_ARRAY},
	{"u64", BRIG_TYPE_U64_ARRAY},
	{"s8", BRIG_TYPE_S8_ARRAY},
	{"s16", BRIG_TYPE_S16_ARRAY},
	{"s32", BRIG_TYPE_S32_ARRAY},
	{"s64", BRIG_TYPE_S64_ARRAY},
	{"f16", BRIG_TYPE_F16_ARRAY},
	{"f32", BRIG_TYPE_F32_ARRAY},
	{"f64", BRIG_TYPE_F64_ARRAY},
	{"b8", BRIG_TYPE_B8_ARRAY},
	{"b16", BRIG_TYPE_B16_ARRAY},
	{"b32", BRIG_TYPE_B32_ARRAY},
	{"b64", BRIG_TYPE_B64_ARRAY},
	{"b128", BRIG_TYPE_B128_ARRAY},
	{"samp", BRIG_TYPE_SAMP_ARRAY},
	{"roimg", BRIG_TYPE_ROIMG_ARRAY},
	{"woimg", BRIG_TYPE_WOIMG_ARRAY},
	{"rwimg", BRIG_TYPE_RWIMG_ARRAY},
	{"sig32", BRIG_TYPE_SIG32_ARRAY},
	{"sig64", BRIG_TYPE_SIG64_ARRAY},
	{"u8x4", BRIG_TYPE_U8X4_ARRAY},
	{"u8x8", BRIG_TYPE_U8X8_ARRAY},
	{"u8x16", BRIG_TYPE_U8X16_ARRAY},
	{"u16x2", BRIG_TYPE_U16X2_ARRAY},
	{"u16x4", BRIG_TYPE_U16X4_ARRAY},
	{"u16x8", BRIG_TYPE_U16X8_ARRAY},
	{"u32x2", BRIG_TYPE_U32X2_ARRAY},
	{"u32x4", BRIG_TYPE_U32X4_ARRAY},
	{"u64x2", BRIG_TYPE_U64X2_ARRAY},
	{"s8x4", BRIG_TYPE_S8X4_ARRAY},
	{"s8x8", BRIG_TYPE_S8X8_ARRAY},
	{"s8x16", BRIG_TYPE_S8X16_ARRAY},
	{"s16x2", BRIG_TYPE_S16X2_ARRAY},
	{"s16x4", BRIG_TYPE_S16X4_ARRAY},
	{"s16x8", BRIG_TYPE_S16X8_ARRAY},
	{"s32x2", BRIG_TYPE_S32X2_ARRAY},
	{"s32x4", BRIG_TYPE_S32X4_ARRAY},
	{"s64x2", BRIG_TYPE_S64X2_ARRAY},
	{"f16x2", BRIG_TYPE_F16X2_ARRAY},
	{"f16x4", BRIG_TYPE_F16X4_ARRAY},
	{"f16x8", BRIG_TYPE_F16X8_ARRAY},
	{"f32x2", BRIG_TYPE_F32X2_ARRAY},
	{"f32x4", BRIG_TYPE_F32X4_ARRAY},
	{"f64x2", BRIG_TYPE_F64X2_ARRAY}

};


std::map<int, unsigned> AsmService::type_to_size_map =
{
	{BRIG_TYPE_NONE, 0},
	{BRIG_TYPE_U8, 1},
	{BRIG_TYPE_U16, 2},
	{BRIG_TYPE_U32, 4},
	{BRIG_TYPE_U64, 8},
	{BRIG_TYPE_S8, 1},
	{BRIG_TYPE_S16, 2},
	{BRIG_TYPE_S32, 4},
	{BRIG_TYPE_S64, 8},
	{BRIG_TYPE_F16, 2},
	{BRIG_TYPE_F32, 4},
	{BRIG_TYPE_F64, 8},
	{BRIG_TYPE_B1, 1},
	{BRIG_TYPE_B8, 1},
	{BRIG_TYPE_B16, 2},
	{BRIG_TYPE_B32, 4},
	{BRIG_TYPE_B64, 8},
	{BRIG_TYPE_B128, 16},
	{BRIG_TYPE_SAMP, 0},
	{BRIG_TYPE_ROIMG, 0},
	{BRIG_TYPE_WOIMG, 0},
	{BRIG_TYPE_RWIMG, 0},
	{BRIG_TYPE_SIG32, 4},
	{BRIG_TYPE_SIG64, 8},

	{BRIG_TYPE_U8X4, 4},
	{BRIG_TYPE_U8X8, 8},
	{BRIG_TYPE_U8X16, 16},
	{BRIG_TYPE_U16X2, 4},
	{BRIG_TYPE_U16X4, 8},
	{BRIG_TYPE_U16X8, 16},
	{BRIG_TYPE_U32X2, 8},
	{BRIG_TYPE_U32X4, 16},
	{BRIG_TYPE_U64X2, 16},
	{BRIG_TYPE_S8X4, 4},
	{BRIG_TYPE_S8X8, 8},
	{BRIG_TYPE_S8X16, 16},
	{BRIG_TYPE_S16X2, 4},
	{BRIG_TYPE_S16X4, 8},
	{BRIG_TYPE_S16X8, 16},
	{BRIG_TYPE_S32X2, 8},
	{BRIG_TYPE_S32X4, 16},
	{BRIG_TYPE_S64X2, 16},
	{BRIG_TYPE_F16X2, 4},
	{BRIG_TYPE_F16X4, 8},
	{BRIG_TYPE_F16X8, 16},
	{BRIG_TYPE_F32X2, 8},
	{BRIG_TYPE_F32X4, 16},
	{BRIG_TYPE_F64X2, 16},

	{BRIG_TYPE_U8_ARRAY, 1},
	{BRIG_TYPE_U16_ARRAY, 2},
	{BRIG_TYPE_U32_ARRAY, 4},
	{BRIG_TYPE_U64_ARRAY, 8},
	{BRIG_TYPE_S8_ARRAY, 1},
	{BRIG_TYPE_S16_ARRAY, 2},
	{BRIG_TYPE_S32_ARRAY, 4},
	{BRIG_TYPE_S64_ARRAY, 8},
	{BRIG_TYPE_F16_ARRAY, 2},
	{BRIG_TYPE_F32_ARRAY, 4},
	{BRIG_TYPE_F64_ARRAY, 8},
	{BRIG_TYPE_B8_ARRAY, 1},
	{BRIG_TYPE_B16_ARRAY, 2},
	{BRIG_TYPE_B32_ARRAY, 4},
	{BRIG_TYPE_B64_ARRAY, 8},
	{BRIG_TYPE_B128_ARRAY, 16},
	{BRIG_TYPE_SAMP_ARRAY, 0},
	{BRIG_TYPE_ROIMG_ARRAY, 0},
	{BRIG_TYPE_WOIMG_ARRAY, 0},
	{BRIG_TYPE_RWIMG_ARRAY, 0},
	{BRIG_TYPE_SIG32_ARRAY, 4},
	{BRIG_TYPE_SIG64_ARRAY, 8},
	{BRIG_TYPE_U8X4_ARRAY, 4},
	{BRIG_TYPE_U8X8_ARRAY, 8},
	{BRIG_TYPE_U8X16_ARRAY, 16},
	{BRIG_TYPE_U16X2_ARRAY, 4},
	{BRIG_TYPE_U16X4_ARRAY, 8},
	{BRIG_TYPE_U16X8_ARRAY, 16},
	{BRIG_TYPE_U32X2_ARRAY, 8},
	{BRIG_TYPE_U32X4_ARRAY, 16},
	{BRIG_TYPE_U64X2_ARRAY, 16},
	{BRIG_TYPE_S8X4_ARRAY, 4},
	{BRIG_TYPE_S8X8_ARRAY, 8},
	{BRIG_TYPE_S8X16_ARRAY, 16},
	{BRIG_TYPE_S16X2_ARRAY, 4},
	{BRIG_TYPE_S16X4_ARRAY, 8},
	{BRIG_TYPE_S16X8_ARRAY, 16},
	{BRIG_TYPE_S32X2_ARRAY, 8},
	{BRIG_TYPE_S32X4_ARRAY, 16},
	{BRIG_TYPE_S64X2_ARRAY, 16},
	{BRIG_TYPE_F16X2_ARRAY, 4},
	{BRIG_TYPE_F16X4_ARRAY, 8},
	{BRIG_TYPE_F16X8_ARRAY, 16},
	{BRIG_TYPE_F32X2_ARRAY, 8},
	{BRIG_TYPE_F32X4_ARRAY, 16},
	{BRIG_TYPE_F64X2_ARRAY, 16}
};


misc::StringMap AsmService::linkage_to_str_map = 
{
	{"", BRIG_LINKAGE_NONE},
	{"prog", BRIG_LINKAGE_PROGRAM},
	{"", BRIG_LINKAGE_MODULE},
	{"", BRIG_LINKAGE_FUNCTION},
	{"", BRIG_LINKAGE_ARG}
};


misc::StringMap AsmService::segment_to_str_map = 
{
	{"", BRIG_SEGMENT_NONE},
	{"", BRIG_SEGMENT_FLAT},
	{"global", BRIG_SEGMENT_GLOBAL},
	{"readonly", BRIG_SEGMENT_READONLY},
	{"kernarg", BRIG_SEGMENT_KERNARG},
	{"group", BRIG_SEGMENT_GROUP},
	{"private", BRIG_SEGMENT_PRIVATE},
	{"spill", BRIG_SEGMENT_SPILL},
	{"arg", BRIG_SEGMENT_ARG},
};


misc::StringMap AsmService::align_to_str_map = 
{
	{"", BRIG_ALIGNMENT_1},
	{"align(2)", BRIG_ALIGNMENT_2},
	{"align(4)", BRIG_ALIGNMENT_4},	
	{"align(8)", BRIG_ALIGNMENT_8},	
	{"align(16)", BRIG_ALIGNMENT_16},	
	{"align(32)", BRIG_ALIGNMENT_32},
	{"align(64)", BRIG_ALIGNMENT_64},
	{"align(128)", BRIG_ALIGNMENT_128},
	{"align(256)", BRIG_ALIGNMENT_256}				
};


misc::StringMap AsmService::register_kind_to_str_map = 
{
	{"$c", BRIG_REGISTER_KIND_CONTROL},
	{"$s", BRIG_REGISTER_KIND_SINGLE},
	{"$d", BRIG_REGISTER_KIND_DOUBLE},
	{"$q", BRIG_REGISTER_KIND_QUAD},
};


misc::StringMap AsmService::width_to_str_map = 
{
	{"", BRIG_WIDTH_NONE}, 
	{"width(1)", BRIG_WIDTH_1}, 
	{"width(2)", BRIG_WIDTH_2}, 
	{"width(4)", BRIG_WIDTH_4}, 
	{"width(8)", BRIG_WIDTH_8}, 
	{"width(16)", BRIG_WIDTH_16}, 
	{"width(32)", BRIG_WIDTH_32}, 
	{"width(64)", BRIG_WIDTH_64}, 
	{"width(128)", BRIG_WIDTH_128}, 
	{"width(256)", BRIG_WIDTH_256}, 
	{"width(512)", BRIG_WIDTH_512}, 
	{"width(1024)", BRIG_WIDTH_1024}, 
	{"width(2048)", BRIG_WIDTH_2048}, 
	{"width(4096)", BRIG_WIDTH_4096}, 
	{"width(8192)", BRIG_WIDTH_8192}, 
	{"width(16384)", BRIG_WIDTH_16384}, 
	{"width(32768)", BRIG_WIDTH_32768}, 
	{"width(65536)", BRIG_WIDTH_65536}, 
	{"width(131072)", BRIG_WIDTH_131072}, 
	{"width(262144)", BRIG_WIDTH_262144}, 
	{"width(524288)", BRIG_WIDTH_524288}, 
	{"width(1048576)", BRIG_WIDTH_1048576}, 
	{"width(2097152)", BRIG_WIDTH_2097152}, 
	{"width(4194304)", BRIG_WIDTH_4194304}, 
	{"width(8388608)", BRIG_WIDTH_8388608}, 
	{"width(16777216)", BRIG_WIDTH_16777216}, 
	{"width(33554432)", BRIG_WIDTH_33554432}, 
	{"width(67108864)", BRIG_WIDTH_67108864}, 
	{"width(134217728)", BRIG_WIDTH_134217728}, 
	{"width(268435456)", BRIG_WIDTH_268435456}, 
	{"width(536870912)", BRIG_WIDTH_536870912}, 
	{"width(1073741824)", BRIG_WIDTH_1073741824}, 
	{"width(2147483648)", BRIG_WIDTH_2147483648}, 
	{"width(WAVESIZE)", BRIG_WIDTH_WAVESIZE}, 
	{"width(all)", BRIG_WIDTH_ALL}, 
};


misc::StringMap AsmService::compare_operation_to_str_map = 
{
	{"eq", BRIG_COMPARE_EQ}, 
	{"ne", BRIG_COMPARE_NE}, 
	{"lt", BRIG_COMPARE_LT}, 
	{"le", BRIG_COMPARE_LE}, 
	{"gt", BRIG_COMPARE_GT}, 
	{"ge", BRIG_COMPARE_GE}, 
	{"equ", BRIG_COMPARE_EQU}, 
	{"neu", BRIG_COMPARE_NEU}, 
	{"ltu", BRIG_COMPARE_LTU}, 
	{"leu", BRIG_COMPARE_LEU}, 
	{"gtu", BRIG_COMPARE_GTU}, 
	{"geu", BRIG_COMPARE_GEU}, 
	{"num", BRIG_COMPARE_NUM}, 
	{"nan", BRIG_COMPARE_NAN}, 
	{"seq", BRIG_COMPARE_SEQ}, 
	{"sne", BRIG_COMPARE_SNE}, 
	{"slt", BRIG_COMPARE_SLT}, 
	{"sle", BRIG_COMPARE_SLE}, 
	{"sgt", BRIG_COMPARE_SGT}, 
	{"sge", BRIG_COMPARE_SGE}, 
	{"sgeu", BRIG_COMPARE_SGEU}, 
	{"sequ", BRIG_COMPARE_SEQU}, 
	{"sneu", BRIG_COMPARE_SNEU}, 
	{"sltu", BRIG_COMPARE_SLTU}, 
	{"sleu", BRIG_COMPARE_SLEU}, 
	{"snum", BRIG_COMPARE_SNUM}, 
	{"snan", BRIG_COMPARE_SNAN}, 
	{"sgtu", BRIG_COMPARE_SGTU}
};


misc::StringMap AsmService::rounding_to_str_map = 
{
	{"", BRIG_ROUND_NONE}, 
	{"", BRIG_ROUND_FLOAT_DEFAULT}, 
	{"near", BRIG_ROUND_FLOAT_NEAR_EVEN}, 
	{"zero", BRIG_ROUND_FLOAT_ZERO}, 
	{"up", BRIG_ROUND_FLOAT_PLUS_INFINITY}, 
	{"down", BRIG_ROUND_FLOAT_MINUS_INFINITY}, 
	{"neari", BRIG_ROUND_INTEGER_NEAR_EVEN}, 
	{"zeroi", BRIG_ROUND_INTEGER_ZERO}, 
	{"upi", BRIG_ROUND_INTEGER_PLUS_INFINITY}, 
	{"downi", BRIG_ROUND_INTEGER_MINUS_INFINITY}, 
	{"neari_sat", BRIG_ROUND_INTEGER_NEAR_EVEN_SAT}, 
	{"zeroi_sat", BRIG_ROUND_INTEGER_ZERO_SAT}, 
	{"upi_sat", BRIG_ROUND_INTEGER_PLUS_INFINITY_SAT}, 
	{"downi_sat", BRIG_ROUND_INTEGER_MINUS_INFINITY_SAT}, 
	{"sneari", BRIG_ROUND_INTEGER_SIGNALING_NEAR_EVEN}, 
	{"szeroi", BRIG_ROUND_INTEGER_SIGNALING_ZERO}, 
	{"supi", BRIG_ROUND_INTEGER_SIGNALING_PLUS_INFINITY}, 
	{"sdowni", BRIG_ROUND_INTEGER_SIGNALING_MINUS_INFINITY}, 
	{"sneari_sat", BRIG_ROUND_INTEGER_SIGNALING_NEAR_EVEN_SAT}, 
	{"szeroi_sat", BRIG_ROUND_INTEGER_SIGNALING_ZERO_SAT}, 
	{"supi_sat", BRIG_ROUND_INTEGER_SIGNALING_PLUS_INFINITY_SAT}, 
	{"sdowni_sat", BRIG_ROUND_INTEGER_SIGNALING_MINUS_INFINITY_SAT}
};


misc::StringMap AsmService::pack_to_str_map = 
{
	{"", BRIG_PACK_NONE}, 
	{"pp", BRIG_PACK_PP}, 
	{"ps", BRIG_PACK_PS}, 
	{"sp", BRIG_PACK_SP}, 
	{"ss", BRIG_PACK_SS}, 
	{"s", BRIG_PACK_S}, 
	{"p", BRIG_PACK_P}, 
	{"pp_sat", BRIG_PACK_PPSAT}, 
	{"ps_sat", BRIG_PACK_PSSAT}, 
	{"sp_sat", BRIG_PACK_SPSAT}, 
	{"ss_sat", BRIG_PACK_SSSAT}, 
	{"s_sat", BRIG_PACK_SSAT}, 
	{"p_sat", BRIG_PACK_PSAT}
};


misc::StringMap AsmService::kind_to_str_map = 
{
	{"NONE", BRIG_KIND_NONE},
	{"DIRECTIVE_ARG_BLOCK_END", BRIG_KIND_DIRECTIVE_ARG_BLOCK_END}, 
	{"DIRECTIVE_ARG_BLOCK_START", BRIG_KIND_DIRECTIVE_ARG_BLOCK_START}, 
	{"DIRECTIVE_COMMENT", BRIG_KIND_DIRECTIVE_COMMENT}, 
	{"DIRECTIVE_CONTROL", BRIG_KIND_DIRECTIVE_CONTROL}, 
	{"DIRECTIVE_EXTENSION", BRIG_KIND_DIRECTIVE_EXTENSION}, 
	{"DIRECTIVE_FBARRIER", BRIG_KIND_DIRECTIVE_FBARRIER}, 
	{"DIRECTIVE_FUNCTION", BRIG_KIND_DIRECTIVE_FUNCTION}, 
	{"DIRECTIVE_INDIRECT_FUNCTION", BRIG_KIND_DIRECTIVE_INDIRECT_FUNCTION}, 
	{"DIRECTIVE_KERNEL", BRIG_KIND_DIRECTIVE_KERNEL}, 
	{"DIRECTIVE_LABEL", BRIG_KIND_DIRECTIVE_LABEL}, 
	{"DIRECTIVE_LOC", BRIG_KIND_DIRECTIVE_LOC}, 
	{"DIRECTIVE_MODULE", BRIG_KIND_DIRECTIVE_MODULE}, 
	{"DIRECTIVE_PRAGMA", BRIG_KIND_DIRECTIVE_PRAGMA}, 
	{"DIRECTIVE_SIGNATURE", BRIG_KIND_DIRECTIVE_SIGNATURE}, 
	{"DIRECTIVE_VARIABLE", BRIG_KIND_DIRECTIVE_VARIABLE}, 

	{"INST_ADDR", BRIG_KIND_INST_ADDR}, 
	{"INST_ATOMIC", BRIG_KIND_INST_ATOMIC}, 
	{"INST_BASIC", BRIG_KIND_INST_BASIC}, 
	{"INST_BR", BRIG_KIND_INST_BR}, 
	{"INST_CMP", BRIG_KIND_INST_CMP}, 
	{"INST_CVT", BRIG_KIND_INST_CVT}, 
	{"INST_IMAGE", BRIG_KIND_INST_IMAGE}, 
	{"INST_LANE", BRIG_KIND_INST_LANE}, 
	{"INST_MEM", BRIG_KIND_INST_MEM}, 
	{"INST_MEM_FENCE", BRIG_KIND_INST_MEM_FENCE}, 
	{"INST_MOD", BRIG_KIND_INST_MOD}, 
	{"INST_QUERY_IMAGE", BRIG_KIND_INST_QUERY_IMAGE}, 
	{"INST_QUERY_SAMPLER", BRIG_KIND_INST_QUERY_SAMPLER}, 
	{"INST_QUEUE", BRIG_KIND_INST_QUEUE}, 
	{"INST_SEG", BRIG_KIND_INST_SEG}, 
	{"INST_SEG_CVT", BRIG_KIND_INST_SEG_CVT}, 
	{"INST_SIGNAL", BRIG_KIND_INST_SIGNAL}, 
	{"INST_SOURCE_TYPE", BRIG_KIND_INST_SOURCE_TYPE}, 

	{"OPERAND_ADDRESS", BRIG_KIND_OPERAND_ADDRESS}, 
	{"OPERAND_ALIGN", BRIG_KIND_OPERAND_ALIGN}, 
	{"OPERAND_CODE_LIST", BRIG_KIND_OPERAND_CODE_LIST}, 
	{"OPERAND_CODE_REF", BRIG_KIND_OPERAND_CODE_REF}, 
	{"OPERAND_CODE_CONSTANT_BYTES", BRIG_KIND_OPERAND_CONSTANT_BYTES}, 
	{"OPERAND_CODE_CONSTANT_IMAGE", BRIG_KIND_OPERAND_CONSTANT_IMAGE}, 
	{"OPERAND_CODE_CONSTANT_OPERAND_LIST", 
			BRIG_KIND_OPERAND_CONSTANT_OPERAND_LIST}, 
	{"OPERAND_CODE_CONSTANT_SAMPLER", BRIG_KIND_OPERAND_CONSTANT_SAMPLER}, 
	{"OPERAND_OPERAND_LIST", BRIG_KIND_OPERAND_OPERAND_LIST}, 
	{"OPERAND_REGISTER", BRIG_KIND_OPERAND_REGISTER}, 
	{"OPERAND_STRING", BRIG_KIND_OPERAND_STRING}, 
	{"OPERAND_WAVESIZE", BRIG_KIND_OPERAND_WAVESIZE}
};


misc::StringMap AsmService::atomic_operation_to_str_map = 
{
	{"add", BRIG_ATOMIC_ADD},
	{"and", BRIG_ATOMIC_AND},
	{"cas", BRIG_ATOMIC_CAS},
	{"exch", BRIG_ATOMIC_EXCH},
	{"ld", BRIG_ATOMIC_LD},
	{"max", BRIG_ATOMIC_MAX},
	{"min", BRIG_ATOMIC_MIN},
	{"or", BRIG_ATOMIC_OR},
	{"st", BRIG_ATOMIC_ST},
	{"sub", BRIG_ATOMIC_SUB},
	{"wrapdec", BRIG_ATOMIC_WRAPDEC},
	{"wrapinc", BRIG_ATOMIC_WRAPINC},
	{"xor", BRIG_ATOMIC_XOR},
	{"wait_eq", BRIG_ATOMIC_WAIT_EQ},
	{"wait_ne", BRIG_ATOMIC_WAIT_NE},
	{"wait_lt", BRIG_ATOMIC_WAIT_LT},
	{"wait_gte", BRIG_ATOMIC_WAIT_GTE},
	{"waittimeout_eq", BRIG_ATOMIC_WAITTIMEOUT_EQ},
	{"waittimeout_ne", BRIG_ATOMIC_WAITTIMEOUT_NE},
	{"waittimeout_lt", BRIG_ATOMIC_WAITTIMEOUT_LT},
	{"waittimeout_gte", BRIG_ATOMIC_WAITTIMEOUT_GTE}
};


misc::StringMap AsmService::memory_order_to_str_map = 
{
	{"", BRIG_MEMORY_ORDER_NONE},
	{"rlx", BRIG_MEMORY_ORDER_RELAXED},
	{"scacq", BRIG_MEMORY_ORDER_SC_ACQUIRE},
	{"screl", BRIG_MEMORY_ORDER_SC_RELEASE},
	{"scar", BRIG_MEMORY_ORDER_SC_ACQUIRE_RELEASE}
};


misc::StringMap AsmService::memory_scope_to_str_map = 
{
	{"", BRIG_MEMORY_SCOPE_NONE},
	{"wi", BRIG_MEMORY_SCOPE_WORKITEM},
	{"wave", BRIG_MEMORY_SCOPE_WAVEFRONT},
	{"wg", BRIG_MEMORY_SCOPE_WORKGROUP},
	{"agent", BRIG_MEMORY_SCOPE_AGENT},
	{"system", BRIG_MEMORY_SCOPE_SYSTEM}
};


misc::StringMap AsmService::image_geometry_to_str_map = 
{
	{"1d", BRIG_GEOMETRY_1D},
	{"2d", BRIG_GEOMETRY_2D},
	{"3d", BRIG_GEOMETRY_3D},
	{"1da", BRIG_GEOMETRY_1DA},
	{"1db", BRIG_GEOMETRY_1DB},
	{"2da", BRIG_GEOMETRY_2DA},
	{"2ddepth", BRIG_GEOMETRY_2DDEPTH},
	{"2dadepth", BRIG_GEOMETRY_2DADEPTH},
};


misc::StringMap AsmService::image_query_to_str_map = 
{
	{"width", BRIG_IMAGE_QUERY_WIDTH},
	{"height", BRIG_IMAGE_QUERY_HEIGHT},
	{"depth", BRIG_IMAGE_QUERY_DEPTH},
	{"array", BRIG_IMAGE_QUERY_ARRAY},
	{"channelorder", BRIG_IMAGE_QUERY_CHANNELORDER},
	{"channeltype", BRIG_IMAGE_QUERY_CHANNELTYPE}
};


bool AsmService::isInteger(BrigType type) 
{
	if (type == BRIG_TYPE_U8 || 
			type == BRIG_TYPE_U16 || 
			type == BRIG_TYPE_U32 ||
			type == BRIG_TYPE_U64 || 
			type == BRIG_TYPE_S8 ||
			type == BRIG_TYPE_S16 ||
			type == BRIG_TYPE_S32 ||
			type == BRIG_TYPE_S64)
		return true;
	return false;
}


bool AsmService::isFloat(BrigType type) 
{
	if (type == BRIG_TYPE_F16 || 
			type == BRIG_TYPE_F32 ||
			type == BRIG_TYPE_F64)
		return true;
	return false;
}


bool AsmService::isPacked(BrigType type)
{
	if ((type & BRIG_TYPE_PACK_32) ||
			(type & BRIG_TYPE_PACK_64) ||
			(type & BRIG_TYPE_PACK_128))
		return true;
	return false;
}


bool AsmService::isArray(BrigType type)
{
	if (type & BRIG_TYPE_ARRAY)
		return true;
	return false;
}


std::string AsmService::OpcodeToString(BrigOpcode opcode)
{
	return opcode_to_str_map.MapValue(opcode);
}


std::string AsmService::TypeToString(BrigType type)
{
	return type_to_str_map.MapValue(type);
}


std::string AsmService::ProfileToString(BrigProfile profile)
{
	switch(profile)
	{
	case BRIG_PROFILE_BASE:
		return "$base";
	case BRIG_PROFILE_FULL:
		return "$full";
	default:
		throw misc::Panic("Unrecognized profile.");
	}

	return "";
}


std::string AsmService::MachineModelToString(BrigMachineModel machine)
{
	switch(machine)
	{
	case BRIG_MACHINE_LARGE:
		return "$large";
	case BRIG_MACHINE_SMALL:
		return "$small";
	default:
		throw misc::Panic("Unrecognized machine model.");
	}
	
	return "";
}


std::string AsmService::DeclToString(bool decl)
{
	if (decl)
		return "decl";
	return "";
}


std::string AsmService::LinkageToString(unsigned char linkage)
{
	return linkage_to_str_map.MapValue(linkage);	
}


std::string AsmService::AllocationToString(BrigAllocation allocation, 
			BrigSegment segment)
{
	if (allocation == BRIG_ALLOCATION_AGENT && 
			segment != BRIG_SEGMENT_READONLY)
		return "alloc(agent)";
	return "";
}


std::string AsmService::SegmentToString(unsigned char segment)
{
	return segment_to_str_map.MapValue(segment);
}


std::string AsmService::AlignToString(BrigAlignment align)
{
	return align_to_str_map.MapValue(align);
}


std::string AsmService::ConstToString(bool isConst)
{
	if (isConst)
	{
		return "const";
	}
	return "";
}


std::string AsmService::RegisterKindToString(BrigRegisterKind register_kind)
{
	return register_kind_to_str_map.MapValue(register_kind);
}


std::string AsmService::RegisterToString(BrigRegisterKind register_kind,
			unsigned short register_number)
{
	return misc::fmt("%s%d",
			AsmService::RegisterKindToString(register_kind).c_str(),
			register_number);
}


std::string AsmService::WidthToString(BrigWidth width)
{
	return width_to_str_map.MapValue(width);
}


std::string AsmService::CompareOperationToString(BrigCompareOperation compare)
{
	return compare_operation_to_str_map.MapValue(compare);	
}


std::string AsmService::AluModifierToString(BrigAluModifier modifier)
{
	if (modifier.allBits & BRIG_ALU_FTZ)
	{
		return "ftz";	
	}
	return "";
}


std::string AsmService::RoundingToString(BrigRound rounding)
{
	return rounding_to_str_map.MapValue(rounding);
}


std::string AsmService::PackToString(BrigPack pack)
{
	return pack_to_str_map.MapValue(pack);
}


std::string AsmService::VectorModifierToString(unsigned vectorModifier)
{
	switch(vectorModifier)
	{
	case 0:
		return "";
	case 2:
		return "v2";
	case 3:
		return "v3";
	case 4:
		return "v4";
	default:
		throw misc::Panic(misc::fmt("Invalid vector modifier v%d", 
				vectorModifier));
	}
}


std::string AsmService::KindToString(BrigKind kind)
{
	return kind_to_str_map.MapValue(kind);
}


std::string AsmService::NoNullToString(bool is_no_null)
{
	return is_no_null ? "nonull" : "";
}


std::string AsmService::EquivClassToString(unsigned char equiv_class)
{
	return misc::fmt("equiv(%d)", equiv_class);
}


std::string AsmService::AtomicOperationToString(BrigAtomicOperation atomic_operation)
{
	return atomic_operation_to_str_map.MapValue(atomic_operation);
}


std::string AsmService::MemoryOrderToString(BrigMemoryOrder memory_order)
{
	return memory_order_to_str_map.MapValue(memory_order);
}


std::string AsmService::MemoryScopeToString(BrigMemoryScope memory_scope)
{
	return memory_scope_to_str_map.MapValue(memory_scope);
}


std::string AsmService::ImageGeometryToString(BrigImageGeometry geometry)
{
	return image_geometry_to_str_map.MapValue(geometry);
}


std::string AsmService::ImageQueryToString(BrigImageQuery query)
{
	return image_query_to_str_map.MapValue(query);
}


std::string AsmService::SamplerQueryToString(BrigSamplerQuery query)
{
	switch(query)
	{
	case BRIG_SAMPLER_QUERY_ADDRESSING: return "addressing";
	case BRIG_SAMPLER_QUERY_COORD: return "coord";
	case BRIG_SAMPLER_QUERY_FILTER: return "filter";
	}
	return "";
}


unsigned AsmService::TypeToSize(BrigType type)
{
	return type_to_size_map.at(type);
}

unsigned AsmService::getSizeInByteByRegisterName(const std::string &name)
{
	if (name[1] == 'c')
		return 1;
	else if (name[1] == 's')
		return 4;
	else if (name[1] == 'd')
		return 8;
	else if(name[1] == 'q')
		return 16;
	else
		throw misc::Panic(misc::fmt("Unknown register name %s\n",
				name.c_str()));
}


void AsmService::DumpUnderscore(const std::string &string, 
		std::ostream &os = std::cout)
{
	if (string != "")
		os << "_" << string;
}


void AsmService::DumpStringLiteral(const std::string &string, 
		std::ostream &os = std::cout)
{
	os << '"';	

	// Traverse all charators
	const char *str_data = string.c_str();
	for (unsigned int i = 0; i < string.length(); i++)
	{
		unsigned char c = str_data[i];
		switch(c)
		{
		case '\a': os << "\\a"; break;
		case '\b': os << "\\b"; break;
		case '\f': os << "\\f"; break;
		case '\n': os << "\\n"; break;
		case '\r': os << "\\r"; break;
		case '\t': os << "\\t"; break;
		case '\"': os << "\\\""; break;
		case '\\': os << "\\\\"; break;
		default:
			if (c  >= 32 && c < 127)
			{
				os << c;
			}
			else
			{
				os << misc::fmt("\\x%02x", c);
			}
			break;
		}
	}

	os << '"';
}


void AsmService::DumpWithSpace(const std::string &string, 
		std::ostream &os = std::cout)
{
	if (string != "")
		os << string << " ";
}

}  // namespace HSA


