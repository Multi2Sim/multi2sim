/*
 * Arg.cc
 *
 *  Created on: Oct 29, 2013
 *      Author: Chulian Zhang
 */

#include <lib/cpp/Misc.h>
#include "Arg.h"
#include "Token.h"

using namespace misc;

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
	{ "SR_ClockHi",	Fermi::InstSRegClockHi}
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
	{ "RGT", ArgCcopType::arg_ccop_rgt}
};

StringMap ModLogicTypeMap=
{
	{ "invalid", ModLogicType::logic_invalid},
	{ "AND", ModLogicType::logic_and},
	{ "OR", ModLogicType::logic_or},
	{ "XOR", ModLogicType::logic_xor}
};

StringMap ModDataWidthMap=
{
	{ "invalid", ModDataWidthType::width_invalid},
	{ "U16", ModDataWidthType::width_u16},
	{ "S16", ModDataWidthType::width_s16},
	{ "U32", ModDataWidthType::width_u32},
	{ "S32", ModDataWidthType::width_s32},
	{ "U64", ModDataWidthType::width_u64},
	{ "S64", ModDataWidthType::width_u64}
};

StringMap ModCompMap=
{
	{ "invalid", ModCompType::modcomp_invalid},
	{ "LT", ModCompType::modcomp_lt},
	{ "EQ", ModCompType::modcomp_eq},
	{ "LE", ModCompType::modcomp_le},
	{ "GT", ModCompType::modcomp_gt},
	{ "NE", ModCompType::modcomp_ne},
	{ "GE", ModCompType::modcomp_ge},
	{ "NUM", ModCompType::modcomp_num},
	{ "NAN", ModCompType::modcomp_nan},
	{ "LTU", ModCompType::modcomp_ltu},
	{ "EQU", ModCompType::modcomp_equ},
	{ "LEU", ModCompType::modcomp_leu},
	{ "GTU", ModCompType::modcomp_gtu},
	{ "GEU", ModCompType::modcomp_geu},
	{ "NEU", ModCompType::modcomp_neu}
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

StringMap Mod0BCopTypeMap=
{
	{ "invalid", Mod0BCopType::mod0bcop_invalid},
	{ "CA", Mod0BCopType::mod0bcop_ca},
	{ "CG", Mod0BCopType::mod0bcop_cg},
	{ "LU", Mod0BCopType::mod0bcop_lu},
	{ "CV", Mod0BCopType::mod0bcop_cv},
};

StringMap ModLogicMap=
{
	{ "invalid", ModLogicType::modlogic_invalid},
	{ "CA", ModLogicType::modlogic_and},
	{ "CG", ModLogicType::modlogic_or},
	{ "LU", ModLogicType::modlogic_xor},

};

StringMap ModGen0Src1DtypeMap=
{
	{ "invalid", ModGen0Src1DtypeType::modgen0src1dtype_invalid},
	{ "F16", ModGen0Src1DtypeType::modgen0src1dtype_f16},
	{ "F32", ModGen0Src1DtypeType::modgen0src1dtype_f32},
	{ "F64", ModGen0Src1DtypeType::modgen0src1dtype_f64},
};

StringMap Mod0ARedarvTypeMap=
{
	{ "invalid", Mod0ARedarvType::mod0aredarv_invalid},
	{ "RED", Mod0ARedarvType::mod0aredarv_red},
	{ "ARV", Mod0ARedarvType::mod0aredarv_arv},
};

StringMap Mod0AOpTypeMap=
{
	{ "invalid", Mod0AOpType::mod0aop_invalid},
	{ "AND", Mod0AOpType::mod0aop_and},
	{ "OR", Mod0AOpType::mod0aop_or},
};

StringMap Mod0DFtzfmzTypeMap=
{
	{ "invalid", Mod0DFtzfmzType::mod0dftzfmz_invalid},
	{ "FTZ", Mod0DFtzfmzType::mod0dftzfmz_ftz},
	{ "FMZ", Mod0DFtzfmzType::mod0dftzfmz_fmz},
};

StringMap ModGen0Mod1BRndTypeMap=
{
	{ "invalid", ModGen0Mod1BRndType::modgen0mod1brnd_invalid},
	{ "FTZ", ModGen0Mod1BRndType::modgen0mod1brnd_rn},
	{ "FMZ", ModGen0Mod1BRndType::modgen0mod1brnd_rm},
	{ "FMZ", ModGen0Mod1BRndType::modgen0mod1brnd_rp},
	{ "FMZ", ModGen0Mod1BRndType::modgen0mod1brnd_rz},
};

StringMap ModOffsMod1AOpTypeMap=
{
	{ "invalid", ModOffsMod1AOpType::modoffsmod1aop_invalid},
	{ "FMA64", ModOffsMod1AOpType::modoffsmod1aop_fma64},
	{ "FMA32", ModOffsMod1AOpType::modoffsmod1aop_fma32},
	{ "XLU", ModOffsMod1AOpType::modoffsmod1aop_xlu},
	{ "ALU", ModOffsMod1AOpType::modoffsmod1aop_alu},
	{ "AGU", ModOffsMod1AOpType::modoffsmod1aop_agu},
	{ "SU", ModOffsMod1AOpType::modoffsmod1aop_su},
	{ "FU", ModOffsMod1AOpType::modoffsmod1aop_fu},
	{ "FMUL", ModOffsMod1AOpType::modoffsmod1aop_fmul},
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
	this->dataWidth = misc::StringMapString(ModDataWidthMap, name);

	if (this->dataWidth == ModDataWidthType::width_invalid)
	{
		std::cout<<"invalid ModDataWidth! ["<<name<<"]"<<std::endl;
		exit(1);
	}
}

Mod0B::Mod0B(const std::string &name)
{
	this->type = misc::StringMapString(Mod0BTypeMap, name);

	if (this->type == Mod0BType::mod0b_invalid)
	{
		std::cout<<"invalid Mod0B Type! ["<<name<<"]"<<std::endl;
		exit(1);
	}
}

Mod0BCop::Mod0BCop(const std::string &name)
{
	this->type = misc::StringMapString(Mod0BCopTypeMap, name);

	if (this->type == Mod0BCopType::mod0bcop_invalid)
	{
		std::cout<<"invalid Mod0BCop Type! ["<<name<<"]"<<std::endl;
		exit(1);
	}
}

ModLogic::ModLogic(const std::string &name)
{
	this->type = misc::StringMapString(ModLogicMap, name);

	if (this->type == ModLogicType::modlogic_invalid)
	{
		std::cout<<"invalid ModLogic Type! ["<<name<<"]"<<std::endl;
		exit(1);
	}
}

ModComp::ModComp(const std::string &name)
{
	this->type = misc::StringMapString(ModCompMap, name);

	if (this->type == ModCompType::modcomp_invalid)
	{
		std::cout<<"invalid ModComp Type! ["<<name<<"]"<<std::endl;
		exit(1);
	}
}

ModBrev::ModBrev(const std::string &name)
{
	if (!name.compare("BREV"))
		this->exist = true;
	else this->exist = false;
}

ModGen0Src1Dtype::ModGen0Src1Dtype(const std::string &name)
{
	this->type = misc::StringMapString(ModGen0Src1DtypeMap, name);

	if (this->type == ModGen0Src1DtypeType::modgen0src1dtype_invalid)
	{
		std::cout<<"invalid ModGen0Src1Dtype Type! ["<<name<<"]"
				<<std::endl;
		exit(1);
	}
}

ModGen0DstCc::ModGen0DstCc(const std::string &name)
{
	if (!name.compare("CC"))
		this->exist = true;
	else this->exist = false;
}

ModTgtU::ModTgtU(const std::string &name)
{
	if (!name.compare("U"))
		this->exist = true;
	else this->exist = false;
}

ModTgtLmt::ModTgtLmt(const std::string &name)
{
	if (!name.compare("LMT"))
		this->exist = true;
	else this->exist = false;
}

Mod0AW::Mod0AW(const std::string &name)
{
	if (!name.compare("W"))
		this->exist = true;
	else this->exist = false;
}

Mod0ARedarv::Mod0ARedarv(const std::string &name)
{
	this->type = misc::StringMapString(Mod0ARedarvTypeMap, name);

	if (this->type == Mod0ARedarvType::mod0aredarv_invalid)
	{
		std::cout<<"invalid Mod0ARedarv Type! ["<<name<<"]"<<std::endl;
		exit(1);
	}
}

Mod0AOp::Mod0AOp(const std::string &name)
{
	this->type = misc::StringMapString(Mod0AOpTypeMap, name);

	if (this->type == Mod0AOpType::mod0aop_invalid)
	{
		std::cout<<"invalid Mod0AOp Type! ["<<name<<"]"<<std::endl;
		exit(1);
	}
}

Mod0CS::Mod0CS(const std::string &name)
{
	if (!name.compare("S"))
		this->exist = true;
	else this->exist = false;
}

Mod0DFtzfmz::Mod0DFtzfmz(const std::string &name)
{
	this->type = misc::StringMapString(Mod0DFtzfmzTypeMap, name);

	if (this->type == Mod0DFtzfmzType::mod0dftzfmz_invalid)
	{
		std::cout<<"invalid Mod0DFtzfmz Type! ["<<name<<"]"<<std::endl;
		exit(1);
	}
}

ModGen0Mod1BRnd::ModGen0Mod1BRnd(const std::string &name)
{
	this->type = misc::StringMapString(ModGen0Mod1BRndTypeMap, name);

	if (this->type == ModGen0Mod1BRndType::modgen0mod1brnd_invalid)
	{
		std::cout<<"invalid ModGen0Mod1BRnd Type! ["<<name<<"]"<<std::endl;
		exit(1);
	}
}

Mod0DSat::Mod0DSat(const std::string &name)
{
	if (!name.compare("SAT"))
		this->exist = true;
	else this->exist = false;
}

Mod0DX::Mod0DX(const std::string &name)
{
	if (!name.compare("X"))
		this->exist = true;
	else this->exist = false;
}

Mod1ATrig::Mod1ATrig(const std::string &name)
{
	if (!name.compare("TRIG"))
		this->exist = true;
	else this->exist = false;
}

ModOffsMod1AOp::ModOffsMod1AOp(const std::string &name)
{
	this->type = misc::StringMapString(ModOffsMod1AOpTypeMap, name);

	if (this->type == ModOffsMod1AOpType::modoffsmod1aop_invalid)
	{
		std::cout<<"invalid ModOffsMod1AOp Type! ["<<name<<"]"<<std::endl;
		exit(1);
	}
}


} /* namespace frm2bin */
