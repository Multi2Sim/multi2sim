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
 

static const int GPR_COUNT = 32;


namespace mips{
class Regs
{
	unsigned int  gpr[GPR_COUNT]; // General Purpose registers
	unsigned int pc; //program counter
    unsigned int hi;  // mult register for [63:32]
    unsigned int lo;  // mult register for [31:0]
public:
    /// Constructor Destructor
    Regs();
    ~Regs();
    
    void reset();
    
    ///read one word from GPR, n is the register number
    ///if n is great than 31, exit program
    unsigned int read_r(int n);
    
    ///write one word to GPR, n is the register number, value
    ///if n is great than 31, exit program
    void write_r(int n, unsigned int value);
    
    ///read from PC
    unsigned int read_PC();
    
    ///write pc with value
    void write_PC(unsigned int value);
    
    ///read from HI register
    unsigned int read_HI();
    
    ///write HI register with value 
    void write_HI(unsigned int value);
    
    ///read from low register
    unsigned int read_LO();
    
    ///write Lo register with value
    void write_LO(unsigned int value); 
    
    ///show all registers
  	//int show_all_registers(int n, char *output_buffer); 
};

}

