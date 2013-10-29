/*
 * Arg.cc
 *
 *  Created on: Oct 29, 2013
 *      Author: Chulian Zhang
 */

#include <lib/cpp/Misc.h>
#include "Arg.h"

using namespace Misc;

namespace frm2bin
{
StringMap arg_special_register_map =
{
	{ "SR_LaneId",	Fermi::InstSRegLaneld},
	{ "SR_VirtCfg",	Fermi::InstSRegVirtCfg},
	{ "SR_VirtId",	Fermi::InstSRegVirtId},
	{ "SR_PM0",	Fermi::InstSRegPM0},
	{ "SR_PM1",	Fermi::InstSRegPM1},
	{ "SR_PM2",	Fermi::InstSRegPM2},
	{ "SR_PM3",	Fermi::InstSRegPM3},
	{ "SR_PM4",	Fermi::InstSRegPM4},
	{ "SR_PM5",	Fermi::InstSRegPM5},
	{ "SR_PM6",	Fermi::InstSRegPM6},
	{ "SR_PM7",	Fermi::InstSRegPM7},
	{ "SR_PRIM_TYPE",	Fermi::InstSRegPrimType},
	{ "SR_INVOCATION_ID",	Fermi::InstSRegInvocationID},
	{ "SR_Y_DIRECTION",	Fermi::InstSRegYDirection},
	{ "SR_MACHINE_ID_0",	Fermi::InstSRegMachineID0},
	{ "SR_MACHINE_ID_1",	Fermi::InstSRegMachineID1},
	{ "SR_MACHINE_ID_2",	Fermi::InstSRegMachineID2},
	{ "SR_MACHINE_ID_3",	Fermi::InstSRegMachineID3},
	{ "SR_AFFINITY",	Fermi::InstSRegAffinity},
	{ "SR_Tid",	Fermi::InstSRegTid },
	{ "SR_Tid_X",	Fermi::InstSRegTidX },
	{ "SR_Tid_Y",	Fermi::InstSRegTidY },
	{ "SR_Tid_Z",	Fermi::InstSRegTidZ },
	{ "SR_CTAParam",	Fermi::InstSRegCTAParam},
	{ "SR_CTAid_X",	Fermi::InstSRegCTAidX},
	{ "SR_CTAid_Y",	Fermi::InstSRegCTAidY},
	{ "SR_CTAid_Z",	Fermi::InstSRegCTAidZ},
	{ "SR_NTid",	Fermi::InstSRegNTid},
	{ "SR_NTid_X",	Fermi::InstSRegNTid},
	{ "SR_NTid_Y",	Fermi::InstSRegNTid},
	{ "SR_NTid_Z",	Fermi::InstSRegNTid},
	{ "SR_GridParam",	Fermi::InstSRegGridParam},
	{ "SR_NCTAid_X",	Fermi::InstSRegNCTAidX},
	{ "SR_NCTAid_Y",	Fermi::InstSRegNCTAidY},
	{ "SR_NCTAid_Z",	Fermi::InstSRegNCTAidZ},
	{ "SR_SWinLo",	Fermi::InstSRegSWinLo},
	{ "SR_SWINSZ",	Fermi::InstSRegSWINSZ},
	{ "SR_SMemSz",	Fermi::InstSRegSMemSz},
	{ "SR_SMemBanks",	Fermi::InstSRegSMemBanks},
	{ "SR_LWinLo",	Fermi::InstSRegLWinLo},
	{ "SR_LWINSZ",	Fermi::InstSRegLWINSZ},
	{ "SR_LMemLoSz",	Fermi::InstSRegLMemLoSz},
	{ "SR_LMemHiOff",	Fermi::InstSRegLMemHiOff},
	{ "SR_EqMask",	Fermi::InstSRegEqMask},
	{ "SR_LtMask",	Fermi::InstSRegLtMask},
	{ "SR_LeMask",	Fermi::InstSRegLeMask},
	{ "SR_GtMask",	Fermi::InstSRegGtMask},
	{ "SR_GeMask",	Fermi::InstSRegGeMask},
	{ "SR_ClockLo",	Fermi::InstSRegClockLo},
	{ "SR_ClockHi",	Fermi::InstSRegClockHi},
	{0, 0}
};

StringMap arg_ccop_map =
{
	{ "F", arg_ccop_f},
	{ "LT", arg_ccop_lt},
	{ "EQ", arg_ccop_eq},
	{ "LE", arg_ccop_le},
	{ "GT", arg_ccop_gt},
	{ "NE", arg_ccop_ne},
	{ "GE", arg_ccop_ge},
	{ "NUM", arg_ccop_num},
	{ "NAN", arg_ccop_nan},
	{ "LTU", arg_ccop_ltu},
	{ "EQU", arg_ccop_equ},
	{ "LEU", arg_ccop_leu},
	{ "GTU", arg_ccop_gtu},
	{ "NEU", arg_ccop_neu},
	{ "GEU", arg_ccop_geu},
	{ "T", arg_ccop_t},
	{ "OFF", arg_ccop_off},
	{ "LO", arg_ccop_lo},
	{ "SFF", arg_ccop_sff},
	{ "LS", arg_ccop_ls},
	{ "HI", arg_ccop_hi},
	{ "SFT", arg_ccop_sft},
	{ "HS", arg_ccop_hs},
	{ "OFT", arg_ccop_oft},
	{ "CSM_TA", arg_ccop_csm_ta},
	{ "CSM_TR", arg_ccop_csm_tr},
	{ "CSM_MX", arg_ccop_csm_mx},
	{ "FCSM_TA", arg_ccop_fcsm_ta},
	{ "FCSM_TR", arg_ccop_fcsm_tr},
	{ "FCSM_MX", arg_ccop_fcsm_mx},
	{ "RLE", arg_ccop_rle},
	{ "RGT", arg_ccop_rgt},
	{0, 0}
};

StringMap mod_logic_type_map=
{
	{ "AND", logic_and},
	{ "OR", logic_or},
	{ "XOR", logic_xor},
	{0, 0}
};

StringMap mod_data_width_map=
{
	{ "U16", u16},
	{ "S16", s16},
	{ "U32", u32},
	{ "S32", s32},
	{ "U64", u64},
	{ "S64", u64},
	{0, 0}
};

StringMap mod_comp_map=
{
	{ "LT", lt},
	{ "EQ", eq},
	{ "LE", le},
	{ "GT", gt},
	{ "NE", ne},
	{ "GE", ge},
	{ "NUM", num},
	{ "NAN", nan},
	{ "LTU", ltu},
	{ "EQU", equ},
	{ "LEU", leu},
	{ "GTU", gtu},
	{ "GEU", geu},
	{ "NEU", neu},
	{0, 0}
};

} /* namespace frm2bin */
