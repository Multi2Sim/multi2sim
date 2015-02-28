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
 
#ifndef ARCH_MIPS_EMU_REGS_H
#define ARCH_MIPS_EMU_REGS_H

#include <lib/cpp/Misc.h>

namespace MIPS
{

class Regs
{
	unsigned int  gpr[32]; // General Purpose registers

	// Floating point registers used are either single or double precision
	union
	{
		float SinglePrecicion[32]; // Single precision FP registers
		double DoublePrecision[16]; // Double precision FP registers
	} fpr;

	unsigned int coprocessor0_gpr[32]; // Co-processor 0 registers
	unsigned int pc; //program counter
    unsigned int hi; // mult register for [63:32]
    unsigned int lo; // mult register for [31:0]

    struct {
    	unsigned int FIR;
    	unsigned int FCSR;
    }regs_FPCR; // Float Point Control Register

public:
    /// Constructor Destructor
    Regs();
    ~Regs();
    
    /// Read one word from GPR, n is the register number If n is great than 31,
    /// exit program
    unsigned int getGPR(int n);

    /// Set the value of a general purpose register.
    ///
    /// \param n
    ///      Register index. Must be a value between 0 and 31.
    ///
    /// \param value
    ///      New value for the register.
    void setGPR(int n, unsigned int value){ gpr[n] = value; }
    
    /// Get float pointer register FIR
    unsigned int getFIR() { return regs_FPCR.FIR; }

    /// Set float pointer register FCSR
    /// \param value
    ///      New value for the register.
    void setFIR(unsigned int value) { regs_FPCR.FIR = value; }

    /// Get float pointer register FCSR
    unsigned int getFCSR() { return regs_FPCR.FCSR; }

    /// Set float pointer register FCSR
    /// \param value
    ///      New value for the register.
    void setFCSR(unsigned int value) { regs_FPCR.FCSR = value; }

    /// Set the value of a co-processor 0 register
    void setCoprocessor0GPR(int n, unsigned int value)
    {
    	coprocessor0_gpr[n] = value;
    }

    /// Get the value of a co-processor 0 register
    unsigned int getCoprocessor0GPR(int n)
    {
    	return coprocessor0_gpr[n];
    }

    /// Set the value of a single precision floating point register
    void setSinglePrecisionFPR(int n, float value) { fpr.SinglePrecicion[n] = value; }

    /// Set the value of a double precision floating point register
    void setDoublePrecisionFPR(int n, float value) { fpr.DoublePrecision[n] = value; }

    /// Get the value of a single precision floating point register
    float getSinglePrecisionFPR(int n) { return fpr.SinglePrecicion[n]; }

    /// Get the value of a double precision floating point register
    double getDoublePrecisionFPR(int n) { return fpr.DoublePrecision[n]; }

    /// Write pc with value
    void setPC(unsigned int value) { pc = value; }
    
    /// Increment value of register \c pc by \a value bytes.
    void incPC(int value) { pc += value; }

    /// Decrement value of register \c pc by \a value bytes
    void decPC(unsigned int value) { pc -= value; }

    /// Read from Program Counter
    unsigned int getPC(){ return pc; }

    /// Set Register29 stack pointer to value
    void setSP(int value) { gpr[29] = value; }

    /// Return the value of the stack pointer register ($31)
    unsigned int getSP() { return gpr[29]; }

    /// Read from HI register
    unsigned int getHI();
    
    /// Write HI register with value
    void setHI(unsigned int value);
    
    /// Read from low register
    unsigned int getLO(){ return lo; }
    
    /// Write Lo register with value
    void setLO(unsigned int value); 
    
    ///show all registers
  	//int show_all_registers(int n, char *output_buffer); 
};

}

#endif
