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

extern Misc::StringMap arg_type_map;
extern Misc::StringMap arg_ccop_map;
extern Misc::StringMap arg_special_register_map;

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
	arg_ccop_f = 0,
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
	logic_and,
	logic_or,
	logic_xor,
};

/* mod data width */
enum MmodDataWidth
{
	u16,
	s16,
	u32,
	s32,
	u64,
	s64,
};

/* mod comparison type */
enum ModComp
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

//class ArgSpecialRegister : public Arg
//{
//	/* register number */
//	int id;
//public:
//	ArgSpecialRegister(int id) : id(id) { }
//
//	/* getter and setter */
//	int getId() { return id; }
//	void setId(int id) { this->id = id; }
//};

class ArgSpecialRegister : public Arg
{
	Fermi::InstSReg reg;
public:
	ArgSpecialRegister(const std::string &name) {reg =
		(Fermi::InstSReg)StringMapString(arg_special_register_map, name);}

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

/* predicate argument */
class ArgCcop : public Arg
{
	/* whether it's ccop or not*/
	int op;
public:
	ArgCcop(const std::string &name) {op = StringMapString(arg_ccop_map, name.substr(3, std::string::npos)) }

	/* getter and setter */
	int getOp() { return op; }
	void setOp(int idx) { this->op = op; }
};


} /* namespace frm2bin */

#endif  /* __cplusplus */


/*
 * C Wrapper
 */

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* FRM2BIN_ARG_H */
