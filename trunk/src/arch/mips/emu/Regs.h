/*
 *  Multi2Sim
 *  Copyright (C) 2012  Sida Gu (gu.sid@husky.neu.edu)
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
#define GPR_COUNT 32
#define FPR_COUNT 32

/* Instruction fields */
#define RS              MIPSInstWrapGetBytes(ctx->inst)->standard.rs
#define RD              MIPSInstWrapGetBytes(ctx->inst)->standard.rd
#define SA              MIPSInstWrapGetBytes(ctx->inst)->standard.sa
#define OFFSET              MIPSInstWrapGetBytes(ctx->inst)->offset_imm.offset
#define RT              MIPSInstWrapGetBytes(ctx->inst)->standard.rt
#define IMM             MIPSInstWrapGetBytes(ctx->inst)->offset_imm.offset
#define FT              MIPSInstWrapGetBytes(ctx->inst)->standard.rt
#define FS              MIPSInstWrapGetBytes(ctx->inst)->standard.rd
#define FD              MIPSInstWrapGetBytes(ctx->inst)->standard.sa
#define TARGET              MIPSInstWrapGetBytes(ctx->inst)->target.target

namespace mips{
    class Regs
    {
        private:
            unsigned int regs_R[32]; // General Purpose registers
            float s[FPR_COUNT]; // single precision fp regs
            double d[FPR_COUNT/2]; // double precision fp regs
        public:
            Regs();
            ~Regs();
            unsigned int getGPR(int i);
    } 

}

