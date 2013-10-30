/*
 * Arg.cc
 *
 *  Created on: Oct 29, 2013
 *      Author: Chulian Zhang
 */

#include <lib/cpp/Misc.h>
#include "Arg.h"

using namespace Misc;

extern StringMap TokenTypeMap;

namespace frm2bin
{
StringMap ArgSpecialRegisterMap =
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

StringMap ArgCcopMap =
{
	{ "invalid", ArgCcopType::arg_ccop_invalid},
	{ "F", ArgCcopType::arg_ccop_f},
	{ "LT", ArgCcopType::arg_ccop_lt},
	{ "EQ", ArgCcopType::arg_ccop_eq},
	{ "LE", ArgCcopType::arg_ccop_le},
	{ "GT", ArgCcopType::arg_ccop_gt},
	{ "NE", ArgCcopType::arg_ccop_ne},
	{ "GE", ArgCcopType::arg_ccop_ge},
	{ "NUM", ArgCcopType::arg_ccop_num},
	{ "NAN", ArgCcopType::arg_ccop_nan},
	{ "LTU", ArgCcopType::arg_ccop_ltu},
	{ "EQU", ArgCcopType::arg_ccop_equ},
	{ "LEU", ArgCcopType::arg_ccop_leu},
	{ "GTU", ArgCcopType::arg_ccop_gtu},
	{ "NEU", ArgCcopType::arg_ccop_neu},
	{ "GEU", ArgCcopType::arg_ccop_geu},
	{ "T", ArgCcopType::arg_ccop_t},
	{ "OFF", ArgCcopType::arg_ccop_off},
	{ "LO", ArgCcopType::arg_ccop_lo},
	{ "SFF", ArgCcopType::arg_ccop_sff},
	{ "LS", ArgCcopType::arg_ccop_ls},
	{ "HI", ArgCcopType::arg_ccop_hi},
	{ "SFT", ArgCcopType::arg_ccop_sft},
	{ "HS", ArgCcopType::arg_ccop_hs},
	{ "OFT", ArgCcopType::arg_ccop_oft},
	{ "CSM_TA", ArgCcopType::arg_ccop_csm_ta},
	{ "CSM_TR", ArgCcopType::arg_ccop_csm_tr},
	{ "CSM_MX", ArgCcopType::arg_ccop_csm_mx},
	{ "FCSM_TA", ArgCcopType::arg_ccop_fcsm_ta},
	{ "FCSM_TR", ArgCcopType::arg_ccop_fcsm_tr},
	{ "FCSM_MX", ArgCcopType::arg_ccop_fcsm_mx},
	{ "RLE", ArgCcopType::arg_ccop_rle},
	{ "RGT", ArgCcopType::arg_ccop_rgt},
	{0, 0}
};

StringMap ModLogicTypeMap=
{
	{ "invalid", ModLogicType::logic_invalid},
	{ "AND", ModLogicType::logic_and},
	{ "OR", ModLogicType::logic_or},
	{ "XOR", ModLogicType::logic_xor},
	{0, 0}
};

StringMap ModDataWidthMap=
{
	{ "invalid", ModDataWidthType::width_invalid},
	{ "U16", ModDataWidthType::width_u16},
	{ "S16", ModDataWidthType::width_s16},
	{ "U32", ModDataWidthType::width_u32},
	{ "S32", ModDataWidthType::width_s32},
	{ "U64", ModDataWidthType::width_u64},
	{ "S64", ModDataWidthType::width_u64},
	{0, 0}
};

StringMap ModCompMap=
{
	{ "invalid", ModCompType::comp_invalid},
	{ "LT", ModCompType::comp_lt},
	{ "EQ", ModCompType::comp_eq},
	{ "LE", ModCompType::comp_le},
	{ "GT", ModCompType::comp_gt},
	{ "NE", ModCompType::comp_ne},
	{ "GE", ModCompType::comp_ge},
	{ "NUM", ModCompType::comp_num},
	{ "NAN", ModCompType::comp_nan},
	{ "LTU", ModCompType::comp_ltu},
	{ "EQU", ModCompType::comp_equ},
	{ "LEU", ModCompType::comp_leu},
	{ "GTU", ModCompType::comp_gtu},
	{ "GEU", ModCompType::comp_geu},
	{ "NEU", ModCompType::comp_neu},
	{0, 0}
};

StringMap Mod0BTypeMap=
{
	{ "invalid", Mod0BType::mod0b_invalid},
	{ "U8", Mod0BType::mod0b_u8},
	{ "S8", Mod0BType::mod0b_s8},
	{ "U16", Mod0BType::mod0b_u16},
	{ "S16", Mod0BType::mod0b_s16},
	{ "64", Mod0BType::mod0b_64},
	{ "128", Mod0BType::mod0b_128},
};

Mod::Mod(const std::string &name)
{
	/* add % back to the modifier string */
	std::string tmpStr = "%";

	tmpStr.append(name);

	this->type = StringMapString(TokenTypeMap, tmpStr);

	if (this->type == TokenType::token_invalid)
	{
		std::cout<<"invalid modifier type! ["<<tmpStr<<"]"<<std::endl;
		exit(1);
	}
}

ModDataWidth::ModDataWidth(const std::string &name)
{
	this->dataWidth = Misc::StringMapString(ModDataWidthMap, name);

	if (this->dataWidth == ModDataWidthType::width_invalid)
	{
		std::cout<<"invalid ModDataWidth! ["<<name<<"]"<<std::endl;
		exit(1);
	}
}

Mod0B::Mod0B(const std::string &name)
{
	this->type = Misc::StringMapString(Mod0BTypeMap, name);

	if (this->type == Mod0BType::mod0b_invalid)
	{
		std::cout<<"invalid Mod0B Type! ["<<name<<"]"<<std::endl;
		exit(1);
	}
}

} /* namespace frm2bin */
