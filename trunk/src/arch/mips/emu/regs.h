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

#ifndef ARCH_MIPS_EMU_REGS_H
#define ARCH_MIPS_EMU_REGS_H

///* Mips Modes of Operation */
//#define MIPS_MODE_USER 0x10
//#define MIPS_MODE_UNDEF 0x1b
//#define MIPS_MODE_SYS 0x1f
//#define MIPS_MODE_SVC 0x13
//#define MIPS_MODE_IRQ 0x12
//#define MIPS_MODE_FIQ 0x11
//#define MIPS_MODE_ABT 0x17

/* register file */
#define GPR_COUNT	32
#define FPR_COUNT	32

/* Register File access */
#define MIPS_COP0_GET(X) 			ctx->regs->regs_cop0[X]

#define MIPS_FPR_S_GET(X)			ctx->regs->regs_F.s[X]
#define MIPS_FPR_S_SET(X, V)		ctx->regs->regs_F.s[X] = (V)

#define MIPS_FPR_D_GET(X)			ctx->regs->regs_F.d[X]
#define MIPS_FPR_D_SET(X, V)		ctx->regs->regs_F.d[X] = (V)

#define MIPS_REG_HI 				ctx->regs->regs_HI
#define MIPS_REG_LO 				ctx->regs->regs_LO
#define MIPS_REG_C_FPC_FCSR 		ctx->regs->regs_C.FCSR
#define MIPS_REG_C_FPC_FIR 			ctx->regs->regs_C.FIR

/* Instruction fields */
#define RS 				MIPSInstWrapGetBytes(ctx->inst)->standard.rs
#define RD 				MIPSInstWrapGetBytes(ctx->inst)->standard.rd
#define SA 				MIPSInstWrapGetBytes(ctx->inst)->standard.sa
#define OFFSET 			MIPSInstWrapGetBytes(ctx->inst)->offset_imm.offset
#define RT				MIPSInstWrapGetBytes(ctx->inst)->standard.rt
#define IMM				MIPSInstWrapGetBytes(ctx->inst)->offset_imm.offset
#define	FT				MIPSInstWrapGetBytes(ctx->inst)->standard.rt
#define	FS				MIPSInstWrapGetBytes(ctx->inst)->standard.rd
#define	FD				MIPSInstWrapGetBytes(ctx->inst)->standard.sa
#define	TARGET			MIPSInstWrapGetBytes(ctx->inst)->target.target

extern char *gpr_name[];
extern char *fpr_name[];

class mips_regs_t {
    private:
        class mips_regs_impl {
            unsigned int regs_R[GPR_COUNT]; /* General Purpose registers */

            union {
                float	s[FPR_COUNT];		/* single precision fp regs */
                double	d[FPR_COUNT / 2];	/* double precision fp regs */
            } regs_F;

            struct {
                unsigned int FIR;
                unsigned int FCSR;
            } regs_C;

            unsigned int regs_cop0[GPR_COUNT]; /* Coprocessor 0 registers */
            unsigned int pc;
            unsigned int regs_HI;
            unsigned int regs_LO;
        } __attribute__((packed));

        mips_regs_impl* m_inst;

        void copy_from(const mips_regs_t& b);

    public:
        mips_regs_t();
        ~mips_regs_t();

        void initialize();
        void destroy();

        mips_regs_t(const mips_regs_t& b);
        mips_regs_t operator=(const mips_regs_t& b);
};

#endif
