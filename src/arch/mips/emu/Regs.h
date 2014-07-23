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

namespace MIPS
{

class Regs
{
	unsigned int  gpr[32]; // General Purpose registers
	unsigned int pc; //program counter
    unsigned int hi;  // mult register for [63:32]
    unsigned int lo;  // mult register for [31:0]
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
    void setGPR(int n, unsigned int value);
    
    /// Write pc with value
    void setPC(unsigned int value){ pc = value; }
    
    /// Increment value of register \c eip by \a value bytes.
    void incPC(int value) { pc += value; }

    /// Read from Program Counter
    unsigned int getPC(){ return pc; }

    /// Set Register31 stack pointer to value
    void setSP(int value) { gpr[31] = value; }

    /// Return the value of the stack pointer register ($31)
    unsigned int getSP() { return gpr[31]; }

    /// Read from HI register
    unsigned int getHI();
    
    /// Write HI register with value
    void setHI(unsigned int value);
    
    /// Read from low register
    unsigned int getLO();
    
    /// Write Lo register with value
    void setLO(unsigned int value); 
    
    ///show all registers
  	//int show_all_registers(int n, char *output_buffer); 
};

}

#endif
