/*
 * Multi2Sim
 * Copyright (C) 2014 Agamemnon Despopoulos (agdespopoulos@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "TimingParameters.h"


namespace dram
{


void TimingParameters::DefaultDDR3_1600()
{
	tRC = 49;
	tRRD = 5;
	tRP = 11;
	tRFC = 128;
	tCCD = 4;
	tRTRS = 1;
	tCWD = 5;
	tWTR = 6;
	tCAS = 11;
	tRCD = 11;
	tOST = 1;
	tRAS = 28;
	tWR = 12;
	tRTP = 6;
	tBURST = 4;
}


}  // namespace dram
