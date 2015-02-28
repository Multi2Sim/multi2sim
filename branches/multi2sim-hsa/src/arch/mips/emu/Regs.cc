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

#include <lib/cpp/Misc.h>

#include "Regs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace MIPS
{
Regs::Regs() : pc(0), hi(0), lo(0)
{
	for(int i=0; i<32; i++)
		{
			gpr[i]=0;
		}
//		pc=hi=lo=0;

}


Regs::~Regs()
{
}

// Read one word from GPRs, n is the register number.
// If the n is greater than 31, exit program
unsigned int Regs::getGPR(int n)
{
		  return gpr[n];
}

// Read from HI register
unsigned int Regs::getHI()
{
		  return hi;
}

// Write HI register with value
void Regs::setHI(unsigned int value)
{
		  hi=value;
}

// Write LO register with value
void Regs::setLO(unsigned int value)
{
		  lo=value;
}

/*
int Regs::show_all_registers(int n, char *output_buffer)
{
	int m=strlen(output_buffer);
	if(n==16)
	{
		for(int i=0; i<GPR_COUNT; i++)
		{
			m+=sprintf(output_buffer+m,"%4s%-2d %#-10x", "r",i, read_r(i));
			if((i+1)%4==0)m+=sprintf(output_buffer+m, "%c",'\n');

		}
		m+=sprintf(output_buffer+m,"%6s %#-10x", "PC", read_PC());
		m+=sprintf(output_buffer+m, "%6s %#-10x", "HI", read_HI());
		m+=sprintf(output_buffer+m, "%6s %#-10x\n", "LO",read_LO());

	}
	else
	{
		char buffer[40];
		for(int i=0; i<GPR_COUNT; i++)
		{
			m+=sprintf(output_buffer+m, "%4s%-2d %-8s", "r",i,(read_r(i), buffer, n));
			if((i+1)%4==0)m+=sprintf(output_buffer+m, "%c",'\n');

		}

		m+=sprintf(output_buffer+m, "%6s %-8s", "PC",(read_PC(), buffer, n));
		m+=sprintf(output_buffer+m, "%6s %-8s",  "HI",(read_HI(), buffer, n));
		m+=sprintf(output_buffer+m, "%6s %-8s\n", "LO",(read_LO(), buffer, n));

	}
	return 0;
}
*/

}// namespace mips
