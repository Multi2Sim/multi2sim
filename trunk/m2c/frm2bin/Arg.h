/*
 * Arg.h
 *
 *  Created on: Oct 28, 2013
 *      Author: Chulian Zhang
 */

#ifndef FRM2BIN_ARG_H
#define FRM2BIN_ARG_H

#ifdef __cplusplus

#include <arch/fermi/asm/Inst.h>
#include <lib/cpp/Misc.h>
#include <iostream>
#include <string>
#include <cstring>
#include "Token.h"

namespace frm2bin
{

/* Forward declarations */

//extern Misc::StringMap arg_type_map;
extern misc::StringMap ArgCcopMap;
extern misc::StringMap ArgSpecialRegisterMap;
extern misc::StringMap ModDataWidthMap;
extern misc::StringMap ModCompMap;
extern misc::StringMap ModLogicTypeMap;

class Token;


enum ArgType
{
	arg_invalid = 0,

	arg_scalar_register,
	arg_special_register,
	arg_predicate_register,
	arg_zero_register,
	arg_literal,
	/*frm_arg_label,*/
	arg_maddr,
	arg_const_maddr,
	arg_shared_maddr,
	arg_glob_maddr,
	arg_pt,
	arg_ccop,
};

/* NOTE: modify string map 'frm_arg_special_register_map' in asm.c together
 * with this enumeration. */
enum ArgSpecialRegisterType
{
	/* Do we still need the invalid ?? */
	//frm_arg_special_register_invalid = 0,

	/* More to be added */
	arg_special_register_LaneId = 0,
	arg_special_register_VirtCfg = 2,
	arg_special_register_VirtId,
	arg_special_register_PM0,
	arg_special_register_PM1,
	arg_special_register_PM2,
	arg_special_register_PM3,
	arg_special_register_PM4,
	arg_special_register_PM5,
	arg_special_register_PM6,
	arg_special_register_PM7,
	arg_special_register_PRIM_TYPE = 16,
	arg_special_register_INVOCATION_ID,
	arg_special_register_Y_DIRECTION,
	arg_special_register_MACHINE_ID_0 = 24,
	arg_special_register_MACHINE_ID_1,
	arg_special_register_MACHINE_ID_2,
	arg_special_register_MACHINE_ID_3,
	arg_special_register_AFFINITY,
	arg_special_register_Tid = 32,
	arg_special_register_Tid_X,
	arg_special_register_Tid_Y,
	arg_special_register_Tid_Z,
	arg_special_register_CTAParam,
	arg_special_register_CTAid_X,
	arg_special_register_CTAid_Y,
	arg_special_register_CTAid_Z,
	arg_special_register_NTid,
	arg_special_register_NTid_X,
	arg_special_register_NTid_Y,
	arg_special_register_NTid_Z,
	arg_special_register_GridParam,
	arg_special_register_NCTAid_X,
	arg_special_register_NCTAid_Y,
	arg_special_register_NCTAid_Z,
	arg_special_register_SWinLo,
	arg_special_register_SWINSZ,
	arg_special_register_SMemSz,
	arg_special_register_SMemBanks,
	arg_special_register_LWinLo,
	arg_special_register_LWINSZ,
	arg_special_register_LMemLoSz,
	arg_special_register_LMemHiOff,
	arg_special_register_EqMask,
	arg_special_register_LtMask,
	arg_special_register_LeMask,
	arg_special_register_GtMask,
	arg_special_register_GeMask,
	arg_special_register_ClockLo = 80,
	arg_special_register_ClockHi,
};

enum ArgCcopType
{
	arg_ccop_invalid = 0,
	arg_ccop_f,
	arg_ccop_lt,
	arg_ccop_eq,
	arg_ccop_le,
	arg_ccop_gt,
	arg_ccop_ne,
	arg_ccop_ge,
	arg_ccop_num,
	arg_ccop_nan,
	arg_ccop_ltu,
	arg_ccop_equ,
	arg_ccop_leu,
	arg_ccop_gtu,
	arg_ccop_neu,
	arg_ccop_geu,
	arg_ccop_t,
	arg_ccop_off,
	arg_ccop_lo,
	arg_ccop_sff,
	arg_ccop_ls,
	arg_ccop_hi,
	arg_ccop_sft,
	arg_ccop_hs,
	arg_ccop_oft,
	arg_ccop_csm_ta,
	arg_ccop_csm_tr,
	arg_ccop_csm_mx,
	arg_ccop_fcsm_ta,
	arg_ccop_fcsm_tr,
	arg_ccop_fcsm_mx,
	arg_ccop_rle,
	arg_ccop_rgt,
};

/* logic type */
enum ModLogicType
{
	modlogic_invalid = 0,
	modlogic_and,
	modlogic_or,
	modlogic_xor,
};

/* mod data width */
enum ModDataWidthType
{
	width_invalid = 0,
	width_u16,
	width_s16,
	width_u32,
	width_s32,
	width_u64,
	width_s64,
};

/* mod comparison type */
enum ModCompType
{
	modcomp_invalid = 0,
	modcomp_lt,
	modcomp_eq,
	modcomp_le,
	modcomp_gt,
	modcomp_ne,
	modcomp_ge,
	modcomp_num,
	modcomp_nan,
	modcomp_ltu,
	modcomp_equ,
	modcomp_leu,
	modcomp_gtu,
	modcomp_geu,
	modcomp_neu,
};

enum Mod0BType
{
	mod0b_invalid = 0,
	mod0b_u8,
	mod0b_s8,
	mod0b_u16,
	mod0b_s16,
	mod0b_64,
	mod0b_128,
};

enum Mod0BCopType
{
	mod0bcop_invalid = 0,
	mod0bcop_ca,
	mod0bcop_cg,
	mod0bcop_lu,
	mod0bcop_cv,
};

enum ModGen0Src1DtypeType
{
	modgen0src1dtype_invalid = 0,
	modgen0src1dtype_f16,
	modgen0src1dtype_f32,
	modgen0src1dtype_f64,
};

enum Mod0ARedarvType
{
	mod0aredarv_invalid = 0,
	mod0aredarv_red,
	mod0aredarv_arv,
};

enum Mod0AOpType
{
	mod0aop_invalid = 0,
	mod0aop_popc,
	mod0aop_and,
	mod0aop_or,

};

enum Mod0DFtzfmzType
{
	mod0dftzfmz_invalid = 0,
	mod0dftzfmz_ftz,
	mod0dftzfmz_fmz,
};

enum ModGen0Mod1BRndType
{
	modgen0mod1brnd_invalid = 0,
	modgen0mod1brnd_rn,
	modgen0mod1brnd_rm,
	modgen0mod1brnd_rp,
	modgen0mod1brnd_rz,
};

enum ModOffsMod1AOpType
{
	modoffsmod1aop_invalid = 0,
	modoffsmod1aop_fma64,
	modoffsmod1aop_fma32,
	modoffsmod1aop_xlu,
	modoffsmod1aop_alu,
	modoffsmod1aop_agu,
	modoffsmod1aop_su,
	modoffsmod1aop_fu,
	modoffsmod1aop_fmul,
};


class Arg
{
protected:
	/* Argument type. It determines the sub-class of the actual instance of
	 * type 'Arg'. */
	ArgType type;

	bool abs;  /* Absolute value */
	bool neg;  /* Negation */
public:
	/* Constructor */
	Arg(ArgType type) : type(type), abs(false), neg(false) { }

	/* Getters */
	ArgType getType() { return type; }
	bool getAbs() { return abs; }
	bool getNeg() { return neg; }

	/* Dump operand (pure virtual function) */
	//virtual void Dump(std::ostream &os) = 0;
	//friend std::ostream &operator<<(std::ostream &os, Arg &arg) {
	//	arg.Dump(os);
	//	return os;
	//}
};

class ArgLiteral : public Arg
{
	int value;
public:
	ArgLiteral(int value) : value(value) {if (value < 0) neg = true; }

	/* getter and setter */
	int getValue() { return value; }
	void setValue(int value) { this->value = value; }

};

class ArgLiteralFloat : public Arg
{
	float value;
public:
	ArgLiteralFloat(float value) : value(value) {if (value < 0) neg = true; }

	/* getter and setter */
	float getValue() { return value; }
	void setValue(float value) { this->value = value; }

};

class ArgScalarRegister : public Arg
{
	/* register number */
	int id;
public:
	ArgScalarRegister(int id) : id(id) { }

	/* getter and setter */
	int getId() { return id; }
	void setId(int id) { this->id = id; }
};

class ArgSpecialRegister : public Arg
{
	Fermi::InstSReg reg;
public:
	ArgSpecialRegister(const std::string &name) {reg =
		(Fermi::InstSReg)StringMapString(ArgSpecialRegisterMap, name);}

	Fermi::InstSReg getReg() { return reg; }
};

class ArgPredicateRegister : public Arg
{
	/* register number */
	int reg;
public:
	ArgPredicateRegister(const std::string &name) {reg = atoi(name.c_str() + 1);}

	int getPredicateReg() { return reg; }
};

class ArgConstMaddr : public Arg
{
	/* constant memory bank index */
	unsigned int bank_idx;
	/* offset within the bank */
	unsigned int offset;
public:
	ArgConstMaddr(int bank_idx, int offset, bool negative) :
		bank_idx(bank_idx), offset(offset), neg(negative) { }

	/* getter and setter */
	int getBankIdx() { return bank_idx; }
	int getOffest() { return offset; }
	bool getNegative() {return neg; }
	void setBankIdx(int bank_idx) { this->bank_idx = bank_idx; }
	void setOffset(int offset) { this->offset = offset; }
	void setNegative(bool neg) {this->neg = neg; }
};

class ArgSharedMaddr : public Arg
{
	/* shared memory bank index */
	unsigned int bank_idx;
	/* offset within the bank */
	unsigned int offset;
public:
	ArgSharedMaddr(int bank_idx, int offset) :
		bank_idx(bank_idx), offset(offset) { }

	/* getter and setter */
	int getBankIdx() { return bank_idx; }
	int getOffest() { return offset; }
	bool getNegative() {return neg; }
	void setBankIdx(int bank_idx) { this->bank_idx = bank_idx; }
	void setOffset(int offset) { this->offset = offset; }
	void setNegative(bool neg) {this->neg = neg; }
};

class ArgGlobMaddr : public Arg
{
	/* register number */
	unsigned int reg_idx;
	/* offset */
	unsigned int offset;
public:
	ArgGlobMaddr(int reg_idx, int offset) :
		reg_idx(reg_idx), offset(offset) { }

	/* getter and setter */
	int getRegIdx() { return reg_idx; }
	int getOffest() { return offset; }
	bool getNegative() {return neg; }
	void setRegIdx(int reg_idx) { this->reg_idx = reg_idx; }
	void setOffset(int offset) { this->offset = offset; }
	void setNegative(bool neg) {this->neg = neg; }
};

/* predicate argument */
class ArgPt : public Arg
{
	/* whether it's pt or not*/
	int idx;
public:
	ArgPt(const std::string &name) {if (!name.compare("pt")) idx = 7; else idx = 0; }

	/* getter and setter */
	int getPt() { return idx; }
	void setPt(int idx) { this->idx = idx; }
};

class ArgCcop : public Arg
{
	/* whether it's ccop or not*/
	int op;
public:
	ArgCcop(const std::string &name) {op = StringMapString(ArgCcopMap, name.substr(3, std::string::npos)) }

	/* getter and setter */
	int getOp() { return op; }
	void setOp(int idx) { this->op = op; }
};

class Mod
{
protected:
	TokenType type;
public:
	/* Constructor */
	Mod(TokenType type) : type(type) { }

	Mod(const std::string &name);

	/* Getters */
	TokenType getType() {return type; }
};

class ModDataWidth : public Mod
{
	ModDataWidthType dataWidth;
public:
	ModDataWidth(const std::string &name);

	/* Getter */
	ModDataWidth getDataWidth(void) {return dataWidth; }

};

/* modifier specifically for IMAD instruction */
class ModIMAD : public Mod
{
	/*TODO: This needs to be modified later */
	int val;
public:
	ModIMAD(int val): val(val) {}

	/* Getter */
	int getVal(void) {return val; }
};

class Mod0B : public Mod
{
	Mod0BType type;
public:
	Mod0B(const std::string &name);

	/* Getter */
	Mod0BType getType(void) {return type; }

};

class Mod0BCop : public Mod
{
	Mod0BCopType type;
public:
	Mod0BCop(const std::string &name);

	/* Getter */
	Mod0BCopType getType(void) {return type; }
};

class ModLogic : public Mod
{
	ModLogicType type;
public:
	ModLogic(const std::string &name);

	/* Getter */
	ModLogicType getType(void) {return type; }
};

class ModComp : public Mod
{
	ModCompType type;
public:
	ModComp(const std::string &name);

	/* Getter */
	ModCompType getType(void) {return type; }
};

class ModBrev : public Mod
{
	bool exist;
public:
	ModBrev(const std::string &name);

	/* Getter */
	bool getExist(void) {return exist; }
};

class ModGen0Src1Dtype : public Mod
{
	ModGen0Src1DtypeType type;
public:
	ModGen0Src1Dtype(const std::string &name);

	/* Getter */
	ModGen0Src1DtypeType getType(void) {return type; }
};

class ModGen0DstCc : public Mod
{
	bool exist;
public:
	ModGen0DstCc(const std::string &name);

	/* Getter */
	bool getExist(void) {return exist; }
};

class ModTgtU : public Mod
{
	bool exist;
public:
	ModTgtU(const std::string &name);

	/* Getter */
	bool getExist(void) {return exist; }
};

class ModTgtLmt : public Mod
{
	bool exist;
public:
	ModTgtLmt(const std::string &name);

	/* Getter */
	bool getExist(void) {return exist; }
};

class Mod0AW : public Mod
{
	bool exist;
public:
	Mod0AW(const std::string &name);

	/* Getter */
	bool getExist(void) {return exist; }
};

class Mod0ARedarv : public Mod
{
	Mod0ARedarvType type;
public:
	Mod0ARedarv(const std::string &name);

	/* Getter */
	Mod0ARedarvType getType(void) {return type; }
};

class Mod0AOp : public Mod
{
	Mod0AOpType type;
public:
	Mod0AOp(const std::string &name);

	/* Getter */
	Mod0AOpType getType(void) {return type; }
};

class Mod0CS : public Mod
{
	bool exist;
public:
	Mod0CS(const std::string &name);

	/* Getter */
	bool getExist(void) {return exist; }
};

class Mod0DFtzfmz : public Mod
{
	Mod0DFtzfmzType type;
public:
	Mod0DFtzfmz(const std::string &name);

	/* Getter */
	Mod0DFtzfmzType getType(void) {return type; }
};

class ModGen0Mod1BRnd : public Mod
{
	ModGen0Mod1BRndType type;
public:
	ModGen0Mod1BRnd(const std::string &name);

	/* Getter */
	ModGen0Mod1BRndType getType(void) {return type; }
};

class Mod0DSat : public Mod
{
	bool exist;
public:
	Mod0DSat(const std::string &name);

	/* Getter */
	bool getExist(void) {return exist; }
};

class Mod0DX : public Mod
{
	bool exist;
public:
	Mod0DX(const std::string &name);

	/* Getter */
	bool getExist(void) {return exist; }
};

class Mod1ATrig : public Mod
{
	bool exist;
public:
	Mod1ATrig(const std::string &name);

	/* Getter */
	bool getExist(void) {return exist; }
};

class ModOffsMod1AOp : public Mod
{
	ModOffsMod1AOpType type;
public:
	ModOffsMod1AOp(const std::string &name);

	/* Getter */
	ModOffsMod1AOpType getType(void) {return type; }
};


} /* namespace frm2bin */

#endif  /* __cplusplus */


/*
 * C Wrapper
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct frm_symbol_t;


enum frm_arg_type_t
{
	frm_arg_invalid = 0,

	frm_arg_scalar_register,
	frm_arg_special_register,
	frm_arg_predicate_register,
	frm_arg_zero_register,
	frm_arg_literal,
	/*frm_arg_label,*/
	frm_arg_maddr,
	frm_arg_const_maddr,
	frm_arg_shared_maddr,
	frm_arg_glob_maddr,
	frm_arg_pt,
	frm_arg_ccop,
};

/* NOTE: modify string map 'frm_arg_special_register_map' in asm.c together
 * with this enumeration. */
enum frm_arg_special_register_type_t
{
	/* Do we still need the invalid ?? */
	//frm_arg_special_register_invalid = 0,

	/* More to be added */
	frm_arg_special_register_LaneId = 0,
	frm_arg_special_register_VirtCfg = 2,
	frm_arg_special_register_VirtId,
	frm_arg_special_register_PM0,
	frm_arg_special_register_PM1,
	frm_arg_special_register_PM2,
	frm_arg_special_register_PM3,
	frm_arg_special_register_PM4,
	frm_arg_special_register_PM5,
	frm_arg_special_register_PM6,
	frm_arg_special_register_PM7,
	frm_arg_special_register_PRIM_TYPE = 16,
	frm_arg_special_register_INVOCATION_ID,
	frm_arg_special_register_Y_DIRECTION,
	frm_arg_special_register_MACHINE_ID_0 = 24,
	frm_arg_special_register_MACHINE_ID_1,
	frm_arg_special_register_MACHINE_ID_2,
	frm_arg_special_register_MACHINE_ID_3,
	frm_arg_special_register_AFFINITY,
	frm_arg_special_register_Tid = 32,
	frm_arg_special_register_Tid_X,
	frm_arg_special_register_Tid_Y,
	frm_arg_special_register_Tid_Z,
	frm_arg_special_register_CTAParam,
	frm_arg_special_register_CTAid_X,
	frm_arg_special_register_CTAid_Y,
	frm_arg_special_register_CTAid_Z,
	frm_arg_special_register_NTid,
	frm_arg_special_register_NTid_X,
	frm_arg_special_register_NTid_Y,
	frm_arg_special_register_NTid_Z,
	frm_arg_special_register_GridParam,
	frm_arg_special_register_NCTAid_X,
	frm_arg_special_register_NCTAid_Y,
	frm_arg_special_register_NCTAid_Z,
	frm_arg_special_register_SWinLo,
	frm_arg_special_register_SWINSZ,
	frm_arg_special_register_SMemSz,
	frm_arg_special_register_SMemBanks,
	frm_arg_special_register_LWinLo,
	frm_arg_special_register_LWINSZ,
	frm_arg_special_register_LMemLoSz,
	frm_arg_special_register_LMemHiOff,
	frm_arg_special_register_EqMask,
	frm_arg_special_register_LtMask,
	frm_arg_special_register_LeMask,
	frm_arg_special_register_GtMask,
	frm_arg_special_register_GeMask,
	frm_arg_special_register_ClockLo = 80,
	frm_arg_special_register_ClockHi,

};

enum frm_arg_ccop_type_t
{
	frm_arg_ccop_f = 0,
	frm_arg_ccop_lt,
	frm_arg_ccop_eq,
	frm_arg_ccop_le,
	frm_arg_ccop_gt,
	frm_arg_ccop_ne,
	frm_arg_ccop_ge,
	frm_arg_ccop_num,
	frm_arg_ccop_nan,
	frm_arg_ccop_ltu,
	frm_arg_ccop_equ,
	frm_arg_ccop_leu,
	frm_arg_ccop_gtu,
	frm_arg_ccop_neu,
	frm_arg_ccop_geu,
	frm_arg_ccop_t,
	frm_arg_ccop_off,
	frm_arg_ccop_lo,
	frm_arg_ccop_sff,
	frm_arg_ccop_ls,
	frm_arg_ccop_hi,
	frm_arg_ccop_sft,
	frm_arg_ccop_hs,
	frm_arg_ccop_oft,
	frm_arg_ccop_csm_ta,
	frm_arg_ccop_csm_tr,
	frm_arg_ccop_csm_mx,
	frm_arg_ccop_fcsm_ta,
	frm_arg_ccop_fcsm_tr,
	frm_arg_ccop_fcsm_mx,
	frm_arg_ccop_rle,
	frm_arg_ccop_rgt,
};

/* logic type */
enum frm_mod_logic_type_t
{
	logic_and,
	logic_or,
	logic_xor,
};

/* mod data width */
enum frm_mod_data_width_t
{
	u16,
	s16,
	u32,
	s32,
	u64,
	s64,
};

/* mod comparison type */
enum frm_mod_comp_t
{
	frm_lt,
	frm_eq,
	frm_le,
	frm_gt,
	frm_ne,
	frm_ge,
	frm_num,
	frm_nan,
	frm_ltu,
	frm_equ,
	frm_leu,
	frm_gtu,
	frm_geu,
	frm_neu,
};

/* type def for modifier */
struct frm_mod_t
{
	enum frm_token_type_t type;
	union
	{
		enum frm_mod_data_width_t data_width;
		enum frm_mod_logic_type_t logic;
		enum frm_mod_comp_t comparison;
		int brev;	/* bit reverse; 0-false, 1-true */
		int dst_cc;	/* dst condition code? 0-false, 1-ture */
		int tgt_u;	/* tgt_u , 0-false, 1-true */
		int tgt_lmt;	/* tgt_lmt, 0-false, 1-ture */
		int mod0_A_w;	/* 0-default, 1-w */
		int mod0_A_redarv;	/* 0-red, 1-arv */
		int mod0_A_op;	/* 0-popc, 1-and, 2-or, 3-invalid */
		int mod0_B_cop;	/* 0-ca, 1-cg, 2-lu, 3-cv */
		int mod0_B_type;	/* 0-u8, 1-s8, 2-u16, 3-s16, 4-default, 5-64, 6-128 */
		int mod0_C_s;	/* 0-default, 1-s */
		int mod0_D_sat;	/* 0-default, 1-sat */
		int mod0_D_x;	/* 0-default, 1-x */
		int mod0_D_ftzfmz;	/* 0-default, 1-ftz, 2-fmz, 3-invalid */
		int gen0_mod1_B_rnd;	/* 0-default, 1-rm, 2-rp, 3-rz */
		int gen0_src1_dtype;	/* 0-default, 1-f16, 2-f32, 3-f64 */
		int offs_mod1_A_trig;	/* 0-default, 1-trig */
		int offs_mod1_A_op;	/* 0-default, fma64, fma32, xlu, alu, agu, su, fu, fmul */
		int IMAD_mod;	/* for both IMAD_mod1 and IMAD_mod2, 0-u32, 1-s32 */
	} value;
};

struct frm_arg_t
{
	enum frm_arg_type_t type;
	int abs;  /* Absolute value */
	int neg;  /* Negation */

	union
	{

		struct
		{
			unsigned int id;
		} scalar_register;

		struct
		{
			int val;
		} literal;

		struct
		{
			float val;
		} literal_float;

//		struct
//		{
			/* Sub-argument of type 'vector', 'scalar', 'literal',
			 * or 'literal_float'. */
//			struct frm_arg_t *soffset;

			/* Sub-argument of type 'maddr_qual'
			 * (memory address qualifier) */
//			struct frm_arg_t *qual;

//			char *data_format;
//			char *num_format;
//		} maddr;

//		struct
//		{
//			int offen;
//			int idxen;
//			int offset;
//		} maddr_qual;

		struct
		{
			unsigned int bank_idx;
			/* offset within the bank */
			unsigned int offset;
		} const_maddr;

		struct
		{
			unsigned int bank_idx;
			/* offset within the bank */
			unsigned int offset;
		} shared_maddr;

		struct
		{
			/* [reg + offset] */
			/* index of the register used */
			unsigned int reg_idx;
			unsigned int offset;
		} glob_maddr;

		struct
		{
			/* whether it's pt or not*/
			unsigned int idx;
		} pt;

		struct
		{
			int op;
		} ccop;

		struct
		{
			enum frm_arg_special_register_type_t type;
		} special_register;

		struct
		{
			/* P1: 1, !P1: -1*/
			int id;
		} predicate_register;

//		struct
//		{
//			struct frm_symbol_t *symbol;
//		} label;

	} value;
};



struct frm_arg_t *frm_arg_create(void);
void frm_arg_free(struct frm_arg_t *inst_arg);

struct frm_arg_t *frm_arg_create_literal(int value);
struct frm_arg_t *frm_arg_create_scalar_register(char* name);
struct frm_arg_t *frm_arg_create_special_register(char *name);
struct frm_arg_t *frm_arg_create_predicate_register(char *name);
struct frm_arg_t *frm_arg_create_zero_register(char *name);
struct frm_arg_t *frm_arg_create_const_maddr(int bank_idx, int offset, int negative);
struct frm_arg_t *frm_arg_create_shared_maddr(int bank_idx, int offset);
struct frm_arg_t *frm_arg_create_glob_maddr(int reg_idx, int offset);
struct frm_arg_t *frm_arg_create_glob_maddr_reg(int reg_idx);
//struct frm_arg_t *frm_arg_create_glob_maddr_offset(int offset);
struct frm_arg_t *frm_arg_create_pt(char* name);
struct frm_arg_t *frm_arg_create_ccop(char* name);
struct frm_arg_t *frm_arg_create_maddr(struct frm_arg_t *soffset,
	struct frm_arg_t *qual, char *data_format, char *num_format);
/*
struct frm_arg_t *frm_arg_create_label(struct frm_symbol_t *symbol);
*/

int frm_arg_encode_operand(struct frm_arg_t *arg);

void frm_arg_dump(struct frm_arg_t *inst_arg, FILE *f);
void frm_mod_free(struct frm_mod_t *mod);

struct frm_mod_t *frm_mod_create_data_width(char *mod_name);
struct frm_mod_t *frm_mod_create_IMAD_mod(char *mod_name);
struct frm_mod_t *frm_mod_create_logic(char *mod_name);
struct frm_mod_t *frm_mod_create_comparison(char *mod_name);
struct frm_mod_t *frm_mod_create_brev(char *mod_name);
struct frm_mod_t *frm_mod_create_gen0_dst_cc(char *mod_name);
struct frm_mod_t *frm_mod_create_tgt_u(char *mod_name);
struct frm_mod_t *frm_mod_create_tgt_lmt(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_A_w(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_A_redarv(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_A_op(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_B_cop(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_B_type(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_C_s(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_D_sat(char* mod_name);
struct frm_mod_t *frm_mod_create_mod0_D_x(char *mod_name);
struct frm_mod_t *frm_mod_create_mod0_D_ftzfmz(char* mod_name);
struct frm_mod_t *frm_mod_create_gen0_src1_dtype(char* mod_name);
struct frm_mod_t *frm_mod_create_gen0_mod1_B_rnd(char* mod_name);
struct frm_mod_t *frm_mod_create_offs_mod1_A_trig(char* mod_name);
struct frm_mod_t *frm_mod_create_offs_mod1_A_op(char* mod_name);
struct frm_mod_t *frm_mod_create_with_name(char *name);



#ifdef __cplusplus
}
#endif

#endif /* FRM2BIN_ARG_H */
