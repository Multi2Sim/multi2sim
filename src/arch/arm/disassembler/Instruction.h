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

#ifndef ARCH_ARM_DISASSEMBLER_INSTRUCTION_H
#define ARCH_ARM_DISASSEMBLER_INSTRUCTION_H

#include <iostream>

namespace ARM
{

// Forward declarations
class Disassembler;


/// ARM instruction
class Instruction
{
public:

	/// Shift operators
	enum ShiftOperator
	{
		ShiftOperatorLsl = 0,
		ShiftOperatorLsr,
		ShiftOperatorAsr,
		ShiftOperatorRor
	};


	/// Condition Fields
	enum ConditionCodes
	{
		ConditionCodesEQ = 0, // Equal
		ConditionCodesNE,	// Not Equal
		ConditionCodesCS,	// Unsigned higher
		ConditionCodesCC,	// Unsigned Lower
		ConditionCodesMI,	// Negative
		ConditionCodesPL,	// Positive or Zero
		ConditionCodesVS,	// Overflow
		ConditionCodesVC,	// No Overflow
		ConditionCodesHI,	// Unsigned Higher
		ConditionCodesLS,	// Unsigned Lower
		ConditionCodesGE,	// Greater or Equal
		ConditionCodesLT,	// Less Than
		ConditionCodesGT,	// Greater than
		ConditionCodesLE,	// Less than or equal
		ConditionCodesAL	// Always
	};


	/// User register
	enum UserRegisters
	{
		UserRegistersR0 = 0,
		UserRegistersR1,
		UserRegistersR2,
		UserRegistersR3,
		UserRegistersR4,
		UserRegistersR5,
		UserRegistersR6,
		UserRegistersR7,
		UserRegistersR8,
		UserRegistersR9,
		UserRegistersR10,
		UserRegistersR11,
		UserRegistersR12,
		UserRegistersR13,	// Stack Pointer sp
		UserRegistersR14,	// Link register lr
		UserRegistersR15	// Program Counter pc
	};


	/// PSR register
	enum PsrRegisters
	{
		PsrRegistersCPSR = 0,
		PsrRegistersSPSR
	};
	
	struct BytesDpr
	{
		unsigned int op2 	: 12;  // [11:0]
		unsigned int dst_reg 	: 4;   // [15:12]
		unsigned int op1_reg 	: 4;   // [19:16]
		unsigned int s_cond 	: 1;   // [20]
		unsigned int opc_dpr 	: 4;   // [24:21]
		unsigned int imm 	: 1;   // [25]
		unsigned int __reserved0: 2;   // [27:26]
		unsigned int cond 	: 4;   // [31:28]
	};


	struct BytesDprSat
	{
		unsigned int op2 	: 12;  // [11:0]
		unsigned int dst_reg 	: 4;   // [15:12]
		unsigned int op1_reg 	: 4;   // [19:16]
		unsigned int s_cond 	: 1;   // [20]
		unsigned int opc_dpr 	: 4;   // [24:21]
		unsigned int imm 	: 1;   // [25]
		unsigned int __reserved0: 2;   // [27:26]
		unsigned int cond 	: 4;   // [31:28]
	};


	struct BytesPsr
	{
		unsigned int op2 	: 12;  // [11:0]
		unsigned int dst_reg 	: 4;   // [15:12]
		unsigned int __reserved0: 6;   // [21:16]
		unsigned int psr_loc	: 1;   // [22]
		unsigned int __reserved1: 2;   // [24:23]
		unsigned int imm 	: 1;   // [25]
		unsigned int __reserved2: 2;   // [27:26]
		unsigned int cond 	: 4;   // [31:28]
	};


	struct BytesMult
	{
		unsigned int op0_rm 	: 4;  // [3:0]
		unsigned int __reserved0: 4;  // [7:4]
		unsigned int op1_rs 	: 4;  // [11:8]
		unsigned int op2_rn 	: 4;  // [15:12]
		unsigned int dst_rd 	: 4;  // [19:16]
		unsigned int s_cond 	: 1;  // [20]
		unsigned int m_acc 	: 1;  // [21]
		unsigned int __reserved1: 6;  // [27:22]
		unsigned int cond 	: 4;  // [31:28]
	};


	struct BytesMultLn
	{
		unsigned int op0_rm 	: 4;  // [3:0]
		unsigned int __reserved0: 4;  // [7:4]
		unsigned int op1_rs 	: 4;  // [11:8]
		unsigned int dst_lo 	: 4;  // [15:12]
		unsigned int dst_hi 	: 4;  // [19:16]
		unsigned int s_cond 	: 1;  // [20]
		unsigned int m_acc 	: 1;  // [21]
		unsigned int sign 	: 1;  // [22]
		unsigned int __reserved1: 5;  // [27:23]
		unsigned int cond 	: 4;  // [31:28]
	};


	struct BytesSnglDswp
	{
		unsigned int op0_rm 	: 4;  // [3:0]
		unsigned int __reserved0: 8;  // [11:4]
		unsigned int dst_rd 	: 4;  // [15:12]
		unsigned int base_rn 	: 4;  // [19:16]
		unsigned int __reserved1: 2;  // [21:20]
		unsigned int d_type 	: 1;  // [22]
		unsigned int __reserved2: 5;  // [27:23]
		unsigned int cond 	: 4;  // [31:28]
	};


	struct BytesBax
	{
		unsigned int op0_rn 	: 4;  // [3:0]
		unsigned int __reserved0: 24;  // [27:4]
		unsigned int cond 	: 4;  // [31:28]
	};


	struct BytesHfwrdTnsReg
	{
		unsigned int off_reg 	: 4;  // [3:0]
		unsigned int __reserved0: 1;  // [4]
		unsigned int sh_comb 	: 2;  // [6:5]
		unsigned int __reserved1: 5;  // [11:7]
		unsigned int dst_rd 	: 4;  // [15:12]
		unsigned int base_rn 	: 4;  // [19:16]
		unsigned int ld_st 	: 1;  // [20]
		unsigned int wb 	: 1;  // [21]
		unsigned int __reserved2: 1;  // [22]
		unsigned int up_dn 	: 1;  // [23]
		unsigned int idx_typ 	: 1;  // [24]
		unsigned int __reserved3: 3;  // [27:25]
		unsigned int cond 	: 4;  // [31:28]
	};


	struct BytesHfwrdTnsImm
	{
		unsigned int imm_off_lo : 4;  // [3:0]
		unsigned int __reserved0: 1;  // [4]
		unsigned int sh_comb 	: 2;  // [6:5]
		unsigned int __reserved1: 1;  // [7]
		unsigned int imm_off_hi : 4;  // [11:8]
		unsigned int dst_rd 	: 4;  // [15:12]
		unsigned int base_rn 	: 4;  // [19:16]
		unsigned int ld_st 	: 1;  // [20]
		unsigned int wb 	: 1;  // [21]
		unsigned int __reserved2: 1;  // [22]
		unsigned int up_dn 	: 1;  // [23]
		unsigned int idx_typ 	: 1;  // [24]
		unsigned int __reserved3: 3;  // [27:25]
		unsigned int cond 	: 4;  // [31:28]
	};


	struct BytesSdtr
	{
		unsigned int off 	: 12;  // [11:0]
		unsigned int src_dst_rd : 4;  // [15:12]
		unsigned int base_rn 	: 4;  // [19:16]
		unsigned int ld_st 	: 1;  // [20]
		unsigned int wb 	: 1;  // [21]
		unsigned int d_type 	: 1;  // [22]
		unsigned int up_dn 	: 1;  // [23]
		unsigned int idx_typ 	: 1;  // [24]
		unsigned int imm 	: 1;  // [25]
		unsigned int __reserved0: 2;  // [27:26]
		unsigned int cond 	: 4;  // [31:28]
	};


	struct BytesBdtr
	{
		unsigned int reg_lst 	: 16;  // [15:0]
		unsigned int base_rn 	: 4;  // [19:16]
		unsigned int ld_st 	: 1;  // [20]
		unsigned int wb 	: 1;  // [21]
		unsigned int psr_frc 	: 1;  // [22]
		unsigned int up_dn 	: 1;  // [23]
		unsigned int idx_typ 	: 1;  // [24]
		unsigned int __reserved0: 3;  // [27:25]
		unsigned int cond 	: 4;  // [31:28]
	};


	struct BytesBrnch
	{
		signed int off 		: 24;  // [23:0]
		unsigned int link 	: 1;  // [24]
		unsigned int __reserved0: 3;  // [27:25]
		unsigned int cond 	: 4;  // [31:28]
	};


	struct BytesCprDtr
	{
		unsigned int off 	: 8;  // [7:0]
		unsigned int cpr_num 	: 4;  // [11:8]
		unsigned int cpr_sr_dst : 4;  // [15:12]
		unsigned int base_rn 	: 4;  // [19:16]
		unsigned int ld_st 	: 1;  // [20]
		unsigned int wb 	: 1;  // [21]
		unsigned int tr_len 	: 1;  // [22]
		unsigned int up_dn 	: 1;  // [23]
		unsigned int idx_typ	: 1;  // [24]
		unsigned int __reserved0: 3;  // [27:25]
		unsigned int cond 	: 4;  // [31:28]
	};


	struct BytesCprDop
	{
		unsigned int cpr_op_rm 	: 4;  // [3:0]
		unsigned int __reserved0: 1;  // [4]
		unsigned int cpr_info 	: 3;  // [7:5]
		unsigned int cpr_num 	: 4;  // [11:8]
		unsigned int cpr_dst 	: 4;  // [15:12]
		unsigned int cpr_rn 	: 4;  // [19:16]
		unsigned int cpr_opc 	: 4;  // [23:20]
		unsigned int __reserved1: 4;  // [27:24]
		unsigned int cond 	: 4;  // [31:28]
	};


	struct BytesCprRtr
	{
		unsigned int cpr_op_rm 	: 4;  // [3:0]
		unsigned int __reserved0: 1;  // [4]
		unsigned int cpr_info 	: 3;  // [7:5]
		unsigned int cpr_num 	: 4;  // [11:8]
		unsigned int rd 	: 4;  // [15:12]
		unsigned int cpr_rn 	: 4;  // [19:16]
		unsigned int ld_st 	: 1;  // [20]
		unsigned int cpr_opm 	: 1;  // [23:21]
		unsigned int __reserved1: 2;  // [27:24]
		unsigned int cond 	: 4;  // [31:28]
	};


	struct BytesSwiSvc
	{
		unsigned int cmnt 	: 24;  // [23:0]
		unsigned int __reserved0: 4;   // [27:24]
		unsigned int cond 	: 4;   // [31:28]
	};


	struct BytesVfpMv
	{
		unsigned int immd8	: 8;  // [7:0]
		unsigned int __reserved0: 4;  // [11:8]
		unsigned int vd		: 4;  // [15:12]
		unsigned int vfp_rn	: 4;  // [19:16]
		unsigned int __reserved1: 1;  // [20]
		unsigned int w		: 1;  // [21]
		unsigned int d		: 1;  // [22]
		unsigned int u		: 1;  // [23]
		unsigned int p		: 1;  // [24]
		unsigned int __reserved2: 3;  // [27:25]
		unsigned int cond	: 4;  // [31:28]
	};


	struct BytesVfpStregTr
	{
		unsigned int __reserved0:12;  // [11:0]
		unsigned int vfp_rt	: 4;  // [15:12]
		unsigned int __reserved1:12;  // [27:16]
		unsigned int cond	: 4;  // [31:28]
	};


	/// Structure of Instruction Format (Thumb2-32bit)
	struct Thumb32BytesLdStMult
	{
		unsigned int reglist	: 16;	// [15:0]
		unsigned int rn		: 4;	// [19:16]
		unsigned int __reserved0: 1; 	// [20]
		unsigned int wback	: 1;	// [21]
		unsigned int __reserved1: 10; 	// [31:22]
	};


	struct Thumb32BytesPushPop
	{
		unsigned int reglist	: 16;	// [15:0]
		unsigned int __reserved0: 16; 	// [31:16]
	};


	struct Thumb32BytesLdStDouble
	{
		unsigned int immd8	: 8;	// [7:0]
		unsigned int rt2	: 4;	// [11:8]
		unsigned int rt		: 4; 	// [15:12]
		unsigned int rn		: 4; 	// [19:16]
		unsigned int __reserved0: 1; 	// [20]
		unsigned int wback	: 1;	// [21]
		unsigned int __reserved1: 1; 	// [22]
		unsigned int add_sub	: 1;	// [23]
		unsigned int index	: 1;	// [24]
		unsigned int __reserved2: 7; 	// [31:25]
	};


	struct Thumb32BytesTableBranch
	{
		unsigned int rm		: 4; 	// [3:0]
		unsigned int h		: 1; 	// [4]
		unsigned int __reserved0: 11; 	// [15:5]
		unsigned int rn		: 4; 	// [19:16]
		unsigned int __reserved1: 12; 	// [31:20]
	};


	struct Thumb32BytesDataProcShftreg
	{
		unsigned int rm		: 4; 	// [3:0]
		unsigned int type	: 2; 	// [5:4]
		unsigned int imm2	: 2; 	// [7:6]
		unsigned int rd		: 4; 	// [11:8]
		unsigned int imm3	: 3; 	// [14:12]
		unsigned int __reserved0: 1; 	// [15]
		unsigned int rn		: 4; 	// [19:16]
		unsigned int sign	: 1; 	// [20]
		unsigned int __reserved1: 11; 	// [31:21]
	};


	struct Thumb32BytesDataProcImmd
	{
		unsigned int immd8	: 8; 	// [7:0]
		unsigned int rd		: 4; 	// [11:8]
		unsigned int immd3	: 3; 	// [14:12]
		unsigned int __reserved0: 1; 	// [15]
		unsigned int rn		: 4; 	// [19:16]
		unsigned int sign	: 1; 	// [20]
		unsigned int __reserved1: 5; 	// [25:21]
		unsigned int i_flag	: 1; 	// [26]
		unsigned int __reserved2: 5; 	// [31:27]
	};


	struct Thumb32BytesBranch
	{
		unsigned int immd11	: 11; 	// [10:0]
		unsigned int j2		: 1; 	// [11]
		unsigned int __reserved0: 1; 	// [12]
		unsigned int j1		: 1; 	// [13]
		unsigned int __reserved1: 2; 	// [15:14]
		unsigned int immd6	: 6; 	// [21:16]
		unsigned int cond	: 4; 	// [25:22]
		unsigned int sign	: 1; 	// [26]
		unsigned int __reserved2: 5; 	// [31:27]
	};


	struct Thumb32BytesBranchLink
	{
		unsigned int immd11	: 11; 	// [10:0]
		unsigned int j2		: 1; 	// [11]
		unsigned int __reserved0: 1; 	// [12]
		unsigned int j1		: 1; 	// [13]
		unsigned int __reserved1: 2; 	// [15:14]
		unsigned int immd10	: 10; 	// [25:16]
		unsigned int sign	: 1; 	// [26]
		unsigned int __reserved2: 5; 	// [31:27]
	};


	struct Thumb32BytesLdStrReg
	{
		unsigned int rm		: 4;	// [3:0]
		unsigned int immd2	: 2;	// [5:4]
		unsigned int __reserved0: 6; 	// [11:6]
		unsigned int rd		: 4;	// [15:12]
		unsigned int rn		: 4;	// [19:16]
		unsigned int __reserved1: 12; 	// [31:20]
	};


	struct Thumb32BytesLdStrImm
	{
		unsigned int immd12	: 12;	// [11:0]
		unsigned int rd		: 4;	// [15:12]
		unsigned int rn		: 4;	// [19:16]
		unsigned int __reserved0: 3; 	// [22:20]
		unsigned int add	: 1;	// [23]
		unsigned int __reserved1: 8; 	// [31:24]
	};


	struct Thumb32BytesLdStrtImm
	{
		unsigned int immd8	: 8;	// [7:0]
		unsigned int __reserved0: 4; 	// [11:8]
		unsigned int rd		: 4;	// [15:12]
		unsigned int rn		: 4;	// [19:16]
		unsigned int __reserved1: 12; 	// [31:20]
	};


	struct Thumb32BytesDprocReg
	{
		unsigned int rm		: 4;	// [3:0]
		unsigned int rot	: 2;	// [5:4]
		unsigned int __reserved0: 2; 	// [7:6]
		unsigned int rd		: 4;	// [11:8]
		unsigned int __reserved1: 4; 	// [15:12]
		unsigned int rn		: 4;	// [19:16]
		unsigned int sign	: 1;	// [20]
		unsigned int __reserved2: 11; 	// [31:21]
	};


	struct Thumb32BytesMult
	{
		unsigned int rm		: 4;	// [3:0]
		unsigned int __reserved0: 4; 	// [7:4]
		unsigned int rd		: 4; 	// [11:8]
		unsigned int ra		: 4; 	// [15:12]
		unsigned int rn		: 4; 	// [19:16]
		unsigned int __reserved1: 12; 	// [31:20]
	};


	struct Thumb32BytesMultLong
	{
		unsigned int rm		: 4;	// [3:0]
		unsigned int __reserved0: 4; 	// [7:4]
		unsigned int rdhi	: 4; 	// [11:8]
		unsigned int rdlo	: 4; 	// [15:12]
		unsigned int rn		: 4; 	// [19:16]
		unsigned int __reserved1: 12; 	// [31:20]
	};


	struct Thumb32BytesBitField
	{
		unsigned int msb	: 5;	// [4:0]
		unsigned int __reserved0: 1; 	// [5]
		unsigned int immd2	: 2; 	// [7:6]
		unsigned int rd		: 4; 	// [11:8]
		unsigned int immd3	: 3; 	// [14:9]
		unsigned int __reserved1: 1; 	// [15]
		unsigned int rn		: 4; 	// [19:16]
		unsigned int __reserved2: 12; 	// [31:20]
	};


	/// Structure of typedef Instruction Format (Thumb2-16bit)
	struct Thumb16BytesMovshiftReg
	{
		unsigned int reg_rd	: 3;	// [2:0]
		unsigned int reg_rs	: 3;	// [5:3]
		unsigned int offset	: 5;	// [10:6]
		unsigned int op_int	: 2; 	// [12:11]
		unsigned int __reserved0: 3; 	// [15:13]

	};


	struct Thumb16BytesAddSub
	{
		unsigned int reg_rd	: 3;	// [2:0]
		unsigned int reg_rs	: 3;	// [5:3]
		unsigned int rn_imm	: 3;	// [8:6]
		unsigned int op_int	: 1;	// [9]
		unsigned int immd	: 1;	// [10]
		unsigned int __reserved0: 5; 	// [15:11]
	};


	struct Thumb16BytesImmdOprs
	{
		unsigned int offset8	: 8;	// [7:0]
		unsigned int reg_rd	: 3;	// [10:8]
		unsigned int op_int	: 2;	// [12:11]
		unsigned int __reserved0: 3; 	// [15:13]
	};


	struct Thumb16BytesCmpT2
	{
		unsigned int reg_rn	: 3;	// [2:0]
		unsigned int reg_rm	: 4;	// [6:3]
		unsigned int N		: 1;	// [7]
		unsigned int __reserved0: 8; 	// [15:8]
	};


	struct Thumb16BytesDpr
	{
		unsigned int reg_rd	: 3;	// [2:0]
		unsigned int reg_rs	: 3;	// [5:3]
		unsigned int op_int	: 4;	// [9:6]
		unsigned int __reserved0: 6; 	// [15:10]
	};


	struct Thumb16BytesHighRegOprs
	{
		unsigned int reg_rd	: 3;	// [2:0]
		unsigned int reg_rs	: 4;	// [6:3]
		unsigned int h1		: 1;	// [7]
		unsigned int op_int	: 2;	// [9:8]
		unsigned int __reserved0: 6; 	// [15:10]
	};


	struct Thumb16BytesPcLdr
	{
		unsigned int immd_8	: 8;	// [7:0]
		unsigned int reg_rd	: 3; 	// [10:8]
		unsigned int __reserved0: 5; 	// [15:11]
	};


	struct Thumb16BytesLdStrReg
	{

		unsigned int reg_rd	: 3; 	// [2:0]
		unsigned int reg_rb	: 3; 	// [5:3]
		unsigned int reg_ro	: 3; 	// [8:6]
		unsigned int __reserved0: 1; 	// [9]
		unsigned int byte_wrd	: 1; 	// [10]
		unsigned int ld_st	: 1; 	// [11]
		unsigned int __reserved1: 4; 	// [15:12]
	};


	struct Thumb16BytesLdStrExts
	{

		unsigned int reg_rd	: 3; 	// [2:0]
		unsigned int reg_rb	: 3; 	// [5:3]
		unsigned int reg_ro	: 3; 	// [8:6]
		unsigned int __reserved0: 1; 	// [9]
		unsigned int sign_ext	: 1; 	// [10]
		unsigned int h_flag	: 1; 	// [11]
		unsigned int __reserved1: 4; 	// [15:12]
	};


	struct Thumb16BytesLdStrImmd
	{
		unsigned int reg_rd	: 3; 	// [2:0]
		unsigned int reg_rb	: 3; 	// [5:3]
		unsigned int offset	: 5;	// [10:6]
		unsigned int ld_st	: 1; 	// [11]
		unsigned int byte_wrd	: 1; 	// [12]
		unsigned int __reserved0: 3; 	// [15:13]
	};


	struct Thumb16BytesLdStrHfwrd
	{
		unsigned int reg_rd	: 3; 	// [2:0]
		unsigned int reg_rb	: 3; 	// [5:3]
		unsigned int offset	: 5;	// [10:6]
		unsigned int ld_st	: 1; 	// [11]
		unsigned int __reserved0: 4; 	// [15:12]
	};


	struct Thumb16BytesLdStrSpImmd
	{
		unsigned int immd_8	: 8;	// [7:0]
		unsigned int reg_rd	: 3; 	// [10:8]
		unsigned int ld_st	: 1; 	// [11]
		unsigned int __reserved0: 4; 	// [15:12]
	};


	struct Thumb16BytesMiscAddsp
	{
		unsigned int immd_8	: 8;	// [7:0]
		unsigned int reg_rd	: 3; 	// [10:8]
		unsigned int __reserved0: 5; 	// [15:11]
	};


	struct Thumb16BytesMiscSubsp
	{
		unsigned int immd_8	: 7;	// [7:0]
		unsigned int __reserved0: 9; 	// [15:8]
	};


	struct Thumb16BytesMiscCbnz
	{
		unsigned int reg_rn	: 3;	// [2:0]
		unsigned int immd_5	: 5;	// [7:3]
		unsigned int __reserved0: 1; 	// [8]
		unsigned int i_ext	: 1;	// [9]
		unsigned int __reserved1: 1; 	// [10]
		unsigned int op_int	: 1;	// [11]
		unsigned int __reserved2: 4; 	// [15:12]
	};


	struct Thumb16BytesMiscExtnd
	{
		unsigned int reg_rd	: 3;	// [2:0]
		unsigned int reg_rm	: 3;	// [5:3]
		unsigned int __reserved0: 10; 	// [15:6]
	};


	struct Thumb16BytesMiscPushPop
	{
		unsigned int reg_list	: 8;	// [7:0]
		unsigned int m_ext	: 1;	// [8]
		unsigned int __reserved0: 7; 	// [15:9]
	};


	struct Thumb16BytesMiscRev
	{
		unsigned int reg_rd	: 3;	// [2:0]
		unsigned int reg_rm	: 3;	// [5:3]
		unsigned int __reserved0: 10; 	// [15:6]
	};


	struct Thumb16BytesIfThen
	{
		unsigned int mask	: 4;	// [3:0]
		unsigned int first_cond	: 4;	// [7:4]
		unsigned int __reserved0: 8;	// [15:8]
	};


	struct Thumb16BytesLdmStm
	{
		unsigned int reg_list	: 8;	// [7:0]
		unsigned int reg_rb	: 3;	// [10:8]
		unsigned int ld_st	: 1;	// [11]
		unsigned int __reserved0: 4;	// [15:12]
	};


	struct Thumb16BytesSvc
	{
		unsigned int value	: 8;	// [7:0]
		unsigned int __reserved0: 8;	// [15:8]
	};


	struct Thumb16BytesCondBr
	{
		unsigned int s_offset	: 8;	// [7:0]
		unsigned int cond	: 4;	// [11:8]
		unsigned int __reserved0: 4;	// [15:12]
	};


	struct Thumb16BytesBr
	{
		unsigned int immd11	: 11;	// [10:0]
		unsigned int __reserved0: 5;	// [15:11]
	};


	union Bytes
	{
		// Indicate the instruction bit width
		unsigned char bytes[4];
		unsigned int word;

		BytesDpr dpr;
		BytesDprSat dpr_sat;
		BytesPsr psr;
		BytesMult mult;
		BytesMultLn mult_ln;
		BytesSnglDswp sngl_dswp;
		BytesBax bax;
		BytesHfwrdTnsReg hfwrd_reg;
		BytesHfwrdTnsImm hfwrd_imm;
		BytesSdtr sdtr;
		BytesBdtr bdtr;
		BytesBrnch brnch;
		BytesCprDtr cpr_dtr;
		BytesCprDop cpr_dop;
		BytesCprRtr cpr_rtr;
		BytesSwiSvc swi_svc;
		BytesVfpMv vfp_mv;
		BytesVfpStregTr vfp_strreg_tr;
	};


	union Thumb32Bytes
	{
		// Indicate the instruction bit width
		unsigned char bytes[4];
		unsigned int word;

		Thumb32BytesLdStMult ld_st_mult;
		Thumb32BytesPushPop push_pop;
		Thumb32BytesLdStDouble ld_st_double;
		Thumb32BytesTableBranch table_branch;
		Thumb32BytesDataProcShftreg data_proc_shftreg;
		Thumb32BytesDataProcImmd data_proc_immd;
		Thumb32BytesBranch branch;
		Thumb32BytesBranchLink branch_link;
		Thumb32BytesLdStrReg ldstr_reg;
		Thumb32BytesLdStrImm ldstr_imm;
		Thumb32BytesLdStrtImm ldstrt_imm;
		Thumb32BytesDprocReg dproc_reg;
		Thumb32BytesMult mult;
		Thumb32BytesMultLong mult_long;
		Thumb32BytesBitField bit_field;
		Thumb16BytesMiscAddsp add_sp;
	};


	union Thumb16Bytes
	{
		// Indicate the instruction bit width
		unsigned char bytes[2];

		Thumb16BytesMovshiftReg movshift_reg_ins;
		Thumb16BytesAddSub addsub_ins;
		Thumb16BytesImmdOprs immd_oprs_ins;
		Thumb16BytesDpr dpr_ins;
		Thumb16BytesHighRegOprs high_oprs_ins;
		Thumb16BytesPcLdr pcldr_ins;
		Thumb16BytesLdStrReg ldstr_reg_ins;
		Thumb16BytesLdStrExts ldstr_exts_ins;
		Thumb16BytesLdStrImmd ldstr_immd_ins;
		Thumb16BytesLdStrHfwrd ldstr_hfwrd_ins;
		Thumb16BytesLdStrSpImmd sp_immd_ins;
		Thumb16BytesMiscAddsp addsp_ins;
		Thumb16BytesMiscSubsp sub_sp_ins;
		Thumb16BytesMiscCbnz cbnz_ins;
		Thumb16BytesMiscExtnd misc_extnd_ins;
		Thumb16BytesMiscPushPop push_pop_ins;
		Thumb16BytesMiscRev rev_ins;
		Thumb16BytesIfThen if_eq_ins;
		Thumb16BytesLdmStm ldm_stm_ins;
		Thumb16BytesSvc svc_ins;
		Thumb16BytesCondBr cond_br_ins;
		Thumb16BytesCmpT2 cmp_t2;
		Thumb16BytesBr br_ins;
	};


	enum Opcode
	{
		OpcodeInvalid = 0,
	#define DEFINST(_name, _fmt_str, _category, _arg1, _arg2) \
		Opcode##_name,
	#include "Instruction.def"
	#undef DEFINST
		// Max
		OpcodeCount
	};


	enum Thumb16Opcode
	{
		Thumb16OpcodeInvalid = 0,
	#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6) \
		Thumb16Opcode##_name,
	#include "InstructionThumb.def"
	#undef DEFINST
		// Max
		Thumb16OpcodeCount
	};


	enum Thumb32Opcode
	{
		Thumb32OpcodeInvalid = 0,
	#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6,_op7,_op8) \
		Thumb32Opcode##_name,
	#include "InstructionThumb32.def"
	#undef DEFINST
		// Max
		Thumb32OpcodeCount
	};


	enum Category
	{
		CatInvalid = 0,
		CategoryDprReg, 	 // Data Processing Register instructions
		CategoryDprImm,	 // Data Processing Immediate instructions
		CategoryDprSat, 	 // Data Processing saturation instructions
		CategoryPsr,   // Processor Status Register (PSR) instructions
		CategoryMult,   // Multiplication instructions
		CategoryMultSign,   // Multiplication instructions
		CategoryMultLn,   // Long Multiplication instructions
		CategoryMultLnSign,   // Long Multiplication instructions
		CategorySdswp,   // Single Data Swap instructions
		CategoryBax,   // Branch and Exchange instructions
		CategoryHfwrdReg,   // Halfword data transfer Register offset
		CategoryHfwrdImm,   // Halfword data transfer Immediate offset
		CategorySdtr,   // Single Data Transfer instructions
		CategoryBdtr,   // Block Data Transfer instructions
		CategoryBrnch,   // Branch instructions
		CategoryCprDtr,   // Coprocessor Data Transfer instructions
		CategoryCprDop,   // Coprocessor Data Operation instructions
		CategoryCprRtr,   // Coprocessor Register Transfer instructions
		CategorySwiSvc,   // Software Interrupt / SVC Angel trap instructions
		CategoryVfp,   // Vector Floating Point Instructions
		CategoryUndef,
		CategoryCount
	};


	enum Thumb16Category
	{
		Thumb16CategoryNone = 0,
		Thumb16CategoryMovshiftReg,	 // Move Shift Instructions Register Based
		Thumb16CategoryAddsub,		 // Addition and Subtraction Instructions
		Thumb16CategoryImmdOprs,		 // Operations with Immediate Operations
		Thumb16CategoryDprIns,		 // Data Processing Operations
		Thumb16CategoryHiRegOprs,	 // High Register Operations
		Thumb16CategoryPcLdr,		 // LDR operation over PC
		Thumb16CategoryLdstrReg,		 // Load Store with Register Offset
		Thumb16CategoryLdstrExts,	 // Sign Extended Load Store
		Thumb16CategoryLdstrImmd,	 // Load Store with Immediate Offset
		Thumb16CategoryLdstrHfwrd,	 // Load Store Half Word
		Thumb16CategoryLdstrSpImmd,	 // Load Store SP Related with Immediate Offset
		Thumb16CategoryIfThen,		 // If Then Block Instructions
		Thumb16CategoryLdmStm,		 // Load/Store Multiple Instructions
		Thumb16CategoryMiscAddspIns,	 // Miscellaneous Instructions ADD SP relative
		Thumb16CategoryMiscSvcIns,	 // Miscellaneous Instructions SVC Instructions
		Thumb16CategoryMiscBr,		 // Miscellaneous Instructions Conditional Branch
		Thumb16CategoryMiscUcbr,		 // Miscellaneous Instructions Conditional Branch
		Thumb16CategoryMiscRev,		 // Miscellaneous Reverse Instructions
		Thumb16CategoryMiscSubspIns,	 // Miscellaneous Instructions SUB SP relative
		Thumb16CategoryMiscPushPop,	 // Miscellaneous Instructions PUSH and POP
		Thumb16CategoryMiscCbnz,		 // Miscellaneous Instructions CB{N}Z
		Thumb16CategoryCmpT2,		 // Miscellaneous Instructions CB{N}Z
		Thumb16CategoryUndef,
		Thumb16CategoryCount
	};


	enum Thumb32Category
	{
		Thumb32CategoryNone = 0,
		Thumb32CategoryLdStMult,	// Load Store Multiple
		Thumb32CategoryLdStDouble,	// Load Store Double Exclusive
		Thumb32CategoryPushPop,		// Push Pop Multiple
		Thumb32CategoryTableBrnch,	// Table Branch Byte
		Thumb32CategoryDprShftreg,	// Data processing Shifted register
		Thumb32CategoryDprImm,		// Data processing immediate
		Thumb32CategoryDprBinImm,	// Data processing binary immediate
		Thumb32CategoryBranch,		// Branch
		Thumb32CategoryBranchLx,	// Branch with Link exchange
		Thumb32CategoryBranchCond,	// Branch Conditional
		Thumb32CategoryLdstrByte,	// Load Store Register Byte/Halfword
		Thumb32CategoryLdstrReg,	// Load Store Register
		Thumb32CategoryLdstrImmd,	// Load Store Immediate
		Thumb32CategoryDprReg,		// Data Processing Register
		Thumb32CategoryMult,		// Multiply
		Thumb32CategoryMultLong,	// Multiply Long
		Thumb32CategoryBitField,	// Multiply Long
		Thumb32CategoryMovImmd,		// Immediate Move
		Thumb32CategoryUndef,
		Thumb32CategoryCount
	};


	struct Info
	{
		Opcode inst;
		Category category;
		const char* name;
		const char* fmt_str;
		Opcode opcode;
		int size;
	};


	struct Thumb16Info
	{
		Thumb16Opcode inst_16;
		Thumb16Category cat16;
		const char* name;
		const char* fmt_str;
		Thumb16Opcode opcode;
		int size;
		Thumb16Info *next_table;
		int next_table_low;
		int next_table_high;
	};


	struct Thumb32Info
	{
		Thumb32Opcode inst_32;
		Thumb32Category cat32;
		const char* name;
		const char* fmt_str;
		Thumb32Opcode opcode;
		int size;
		Thumb32Info *next_table;
		int next_table_low;
		int next_table_high;
	};

private:

	// Disassembler
	Disassembler *disassembler; 

	// Instruction address
	unsigned int addr;

	// Instruction content
	Bytes dword;
	Thumb16Bytes dword_16;
	Thumb32Bytes dword_32;

	// Information after decoded
	Info *info;
	Thumb16Info *info_16;
	Thumb32Info *info_32;

public:

	/// constructor
	Instruction();

	/// FIXME comment
	unsigned int Rotl(unsigned int value, int shift);
	unsigned int Rotr(unsigned int value, int shift);

	/// FIXME comment
	void Amode2Disasm(std::ostream &os, Category cat);
	void Amode3Disasm(std::ostream &os, Category cat);

	/// Functions that dump the ARM instruction related information
	void DumpRd(std::ostream &os);
	void DumpRn(std::ostream &os);
	void DumpRm(std::ostream &os);
	void DumpRs(std::ostream &os);
	void DumpOp2(std::ostream &os);
	void DumpCond(std::ostream &os);
	void DumpRdlo(std::ostream &os);
	void DumpRdhi(std::ostream &os);
	void DumpPsr(std::ostream &os);
	void DumpOp2Psr(std::ostream &os);
	void DumpAMode3(std::ostream &os);
	void DumpAMode2(std::ostream &os);
	void DumpIdx(std::ostream &os);
	void DumpBaddr(std::ostream &os);
	void DumpRegs(std::ostream &os);
	void DumpImmd24(std::ostream &os);
	void DumpImmd16(std::ostream &os);
	void DumpCopr(std::ostream &os);
	void DumpAMode5(std::ostream &os);
	void DumpVfp1stm(std::ostream &os);
	void DumpVfp1ldm(std::ostream &os);
	void DumpVfpRegs(std::ostream &os);
	void DumpFreg(std::ostream &os);
	void DumpFp(std::ostream &os);
	void DumpRt(std::ostream &os);
	void DumpHex(std::ostream &os, unsigned int *inst_ptr, unsigned int inst_addr);
	void Dump(std::ostream &os);

	/// Functions that dump the Thumb 16 instruction related information
	void Thumb16DumpRD(std::ostream &os);
	void Thumb16DumpRM(std::ostream &os);
	void Thumb16DumpRN(std::ostream &os);
	void Thumb16DumpIMMD8(std::ostream &os);
	void Thumb16DumpIMMD3(std::ostream &os);
	void Thumb16DumpIMMD5(std::ostream &os);
	void Thumb16DumpCOND(std::ostream &os);
	void Thumb16DumpREGS(std::ostream &os);
	void Thumb16DumpItEqX(std::ostream &os);
	void Thumb16DumpHex(std::ostream &os, unsigned int *inst_ptr , unsigned int inst_addr);
	void Thumb16Dump(std::ostream &os);

	/// Functions that dump the Thumb 32 instruction related information
	void Thumb32DumpRD(std::ostream &os);
	void Thumb32DumpRN(std::ostream &os);
	void Thumb32DumpRM(std::ostream &os);
	void Thumb32DumpRT(std::ostream &os);
	void Thumb32DumpRT2(std::ostream &os);
	void Thumb32DumpRA(std::ostream &os);
	void Thumb32DumpRDLO(std::ostream &os);
	void Thumb32DumpRDHI(std::ostream &os);
	void Thumb32DumpS(std::ostream &os);
	void Thumb32dumpREGS(std::ostream &os);
	void Thumb32DumpSHFTREG(std::ostream &os);
	void Thumb32DumpIMM12(std::ostream &os);
	void Thumb32DumpIMMD12(std::ostream &os);
	void Thumb32DumpIMMD8(std::ostream &os);
	void Thumb32DumpIMM2(std::ostream &os);
	void Thumb32DumpCOND(std::ostream &os);
	void Thumb32DumpLSB(std::ostream &os);
	void Thumb32DumpWID(std::ostream &os);
	void Thumb32DumpIMMD16(std::ostream &os);
	void Thumb32DumpADDR(std::ostream &os);
	void Thumb32DumpHex(std::ostream &os, unsigned int *inst_ptr, unsigned int inst_addr);
	void Thumb32Dump(std::ostream &os);

	/// Populate the 'info' field of an instruction by decoding the instruction
	/// in 'buf'. The first 4 bytes of 'buf' are copied to field 'bytes'. The value
	/// given in 'addr' is the instruction's virtual address, used for branch
	/// decoding purposes.
	void Decode(unsigned int addr, const char *buf);
	void Thumb16Decode(const char *buf, unsigned int ip);
	void Thumb32Decode(const char *buf, unsigned int ip);
	void Thumb16InstTableDecode();
	void Thumb32InstTableDecode();

	///Getters
	unsigned int getAddress() { return addr; }
	Bytes *getBytes() { return &dword; }
	Thumb16Bytes *getThumb16Bytes() { return &dword_16; }
	Thumb32Bytes *getThumb32Bytes() { return &dword_32; }
	Opcode getOpcode() { return info ? info->opcode : OpcodeInvalid; }
	Thumb16Opcode getThumb16Opcode() { return info_16 ? info_16->opcode : Thumb16OpcodeInvalid; }
	Thumb32Opcode getThumb32Opcode() { return info_32 ? info_32->opcode : Thumb32OpcodeInvalid; }
	Info *getInstInfo() { return info; }
	Thumb16Info *getInstThumb16Info() { return info_16; }
	Thumb32Info *getInstThumb32Info() { return info_32; }

};

}  // namespace ARM

#endif // ARCH_ARM_DISASSEMBLER_ASM_INSTRUCTION_H
