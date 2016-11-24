/*
 *  Multi2Sim
 *  Copyright (C) 2014  Yifan Sun (yifansun@coe.neu.edu)
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

#include <iomanip>

#include "Brig.h"
#include "AsmService.h"
#include "BrigImmed.h"

namespace HSA
{

std::map<int, BrigImmed::DumpImmedFn> 
BrigImmed::dump_immed_fn = 
{
	{BRIG_TYPE_NONE, &BrigImmed::dumpImmedNONE},
	{BRIG_TYPE_U8, &BrigImmed::dumpImmedU8},
	{BRIG_TYPE_U16, &BrigImmed::dumpImmedU16},
	{BRIG_TYPE_U32, &BrigImmed::dumpImmedU32},
	{BRIG_TYPE_U64, &BrigImmed::dumpImmedU64},
	{BRIG_TYPE_S8, &BrigImmed::dumpImmedS8},
	{BRIG_TYPE_S16, &BrigImmed::dumpImmedS16},
	{BRIG_TYPE_S32, &BrigImmed::dumpImmedS32},
	{BRIG_TYPE_S64, &BrigImmed::dumpImmedS64},
	{BRIG_TYPE_F16, &BrigImmed::dumpImmedF16},
	{BRIG_TYPE_F32, &BrigImmed::dumpImmedF32},
	{BRIG_TYPE_F64, &BrigImmed::dumpImmedF64},
	{BRIG_TYPE_B1, &BrigImmed::dumpImmedB1},
	{BRIG_TYPE_B8, &BrigImmed::dumpImmedB8},
	{BRIG_TYPE_B16, &BrigImmed::dumpImmedB16},
	{BRIG_TYPE_B32, &BrigImmed::dumpImmedB32},
	{BRIG_TYPE_B64, &BrigImmed::dumpImmedB64},
	{BRIG_TYPE_B128, &BrigImmed::dumpImmedU8X16},
	{BRIG_TYPE_SAMP, &BrigImmed::dumpImmedSAMP},
	{BRIG_TYPE_ROIMG, &BrigImmed::dumpImmedROIMG},
	{BRIG_TYPE_RWIMG, &BrigImmed::dumpImmedRWIMG},
	{BRIG_TYPE_SIG32, &BrigImmed::dumpImmedSIG32},
	{BRIG_TYPE_SIG64, &BrigImmed::dumpImmedSIG64},
	{BRIG_TYPE_U8X4, &BrigImmed::dumpImmedU8X4},
	{BRIG_TYPE_U8X8, &BrigImmed::dumpImmedU8X8},
	{BRIG_TYPE_U8X16, &BrigImmed::dumpImmedU8X16},
	{BRIG_TYPE_U16X2, &BrigImmed::dumpImmedU16X2},
	{BRIG_TYPE_U16X4, &BrigImmed::dumpImmedU16X4},
	{BRIG_TYPE_U16X8, &BrigImmed::dumpImmedU16X8},
	{BRIG_TYPE_U32X2, &BrigImmed::dumpImmedU32X2},
	{BRIG_TYPE_U32X4, &BrigImmed::dumpImmedU32X4},
	{BRIG_TYPE_U64X2, &BrigImmed::dumpImmedU64X2},
	{BRIG_TYPE_S8X4, &BrigImmed::dumpImmedS8X4},
	{BRIG_TYPE_S8X8, &BrigImmed::dumpImmedS8X8},
	{BRIG_TYPE_S8X16, &BrigImmed::dumpImmedS8X16},
	{BRIG_TYPE_S16X2, &BrigImmed::dumpImmedS16X2},
	{BRIG_TYPE_S16X4, &BrigImmed::dumpImmedS16X4},
	{BRIG_TYPE_S16X8, &BrigImmed::dumpImmedS16X8},
	{BRIG_TYPE_S32X2 , &BrigImmed::dumpImmedS32X2},
	{BRIG_TYPE_S32X4, &BrigImmed::dumpImmedS32X4},
	{BRIG_TYPE_S64X2, &BrigImmed::dumpImmedS64X2},
	{BRIG_TYPE_F16X2 , &BrigImmed::dumpImmedF16X2},
	{BRIG_TYPE_F16X4 , &BrigImmed::dumpImmedF16X4},
	{BRIG_TYPE_F16X8, &BrigImmed::dumpImmedF16X8},
	{BRIG_TYPE_F32X2, &BrigImmed::dumpImmedF32X2},
	{BRIG_TYPE_F32X4, &BrigImmed::dumpImmedF32X4},
	{BRIG_TYPE_F64X2, &BrigImmed::dumpImmedF64X2}
};


std::map<int, int> BrigImmed::type_to_size_map = 
{
	{BRIG_TYPE_NONE, 	0},
	{BRIG_TYPE_U8, 		1},
	{BRIG_TYPE_U16, 	2},
	{BRIG_TYPE_U32, 	4},
	{BRIG_TYPE_U64, 	8},
	{BRIG_TYPE_S8, 		1},
	{BRIG_TYPE_S16, 	2},
	{BRIG_TYPE_S32, 	4},
	{BRIG_TYPE_S64, 	8},
	{BRIG_TYPE_F16, 	2},
	{BRIG_TYPE_F32, 	4},
	{BRIG_TYPE_F64, 	8},
	{BRIG_TYPE_B1, 		1},
	{BRIG_TYPE_B8, 		1},
	{BRIG_TYPE_B16, 	2},
	{BRIG_TYPE_B32, 	4},
	{BRIG_TYPE_B64, 	8},
	{BRIG_TYPE_B128, 	16},
	{BRIG_TYPE_SAMP, 	0},
	{BRIG_TYPE_ROIMG, 	0},
	{BRIG_TYPE_RWIMG, 	0},
	{BRIG_TYPE_SIG32, 	4},
	{BRIG_TYPE_SIG64, 	8},
	{BRIG_TYPE_U8  | BRIG_TYPE_PACK_32 , 4},
	{BRIG_TYPE_U8  | BRIG_TYPE_PACK_64 , 8},
	{BRIG_TYPE_U8  | BRIG_TYPE_PACK_128, 16},
	{BRIG_TYPE_U16 | BRIG_TYPE_PACK_32 , 4},
	{BRIG_TYPE_U16 | BRIG_TYPE_PACK_64 , 8},
	{BRIG_TYPE_U16 | BRIG_TYPE_PACK_128, 16}, 
	{BRIG_TYPE_U32 | BRIG_TYPE_PACK_64 , 8},
	{BRIG_TYPE_U32 | BRIG_TYPE_PACK_128, 16},
	{BRIG_TYPE_U64 | BRIG_TYPE_PACK_128, 18},
	{BRIG_TYPE_S8  | BRIG_TYPE_PACK_32 , 2},
	{BRIG_TYPE_S8  | BRIG_TYPE_PACK_64 , 8},
	{BRIG_TYPE_S8  | BRIG_TYPE_PACK_128, 16},
	{BRIG_TYPE_S16 | BRIG_TYPE_PACK_32 , 4},
	{BRIG_TYPE_S16 | BRIG_TYPE_PACK_64 , 8},
	{BRIG_TYPE_S16 | BRIG_TYPE_PACK_128, 16},
	{BRIG_TYPE_S32 | BRIG_TYPE_PACK_64 , 8},
	{BRIG_TYPE_S32 | BRIG_TYPE_PACK_128, 16},
	{BRIG_TYPE_S64 | BRIG_TYPE_PACK_128, 16},
	{BRIG_TYPE_F16 | BRIG_TYPE_PACK_32 , 4},
	{BRIG_TYPE_F16 | BRIG_TYPE_PACK_64 , 8},
	{BRIG_TYPE_F16 | BRIG_TYPE_PACK_128, 16},
	{BRIG_TYPE_F32 | BRIG_TYPE_PACK_64 , 8},
	{BRIG_TYPE_F32 | BRIG_TYPE_PACK_128, 16},
	{BRIG_TYPE_F64 | BRIG_TYPE_PACK_128, 16}
};


int BrigImmed::getSize()
{
	return type_to_size_map[type]; 	
}


const unsigned char *BrigImmed::dumpImmedNONE(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	dumpImmedUnsupported(os);
	return ptr;
}


const unsigned char *BrigImmed::dumpImmedU8(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned char *value = (unsigned char *)ptr;
	os << misc::fmt("%u", *value);
	return ptr + 1;
}


const unsigned char *BrigImmed::dumpImmedU16(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned short *value = (unsigned short *)ptr;
	os << misc::fmt("%u", *value);
	return ptr + 2;
}


const unsigned char *BrigImmed::dumpImmedU32(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int *value = (unsigned int *)ptr;
	os << misc::fmt("%u", *value);
	return ptr + 4;
}


const unsigned char * BrigImmed::dumpImmedU64(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned long long *value = (unsigned long long *)ptr;
	os << misc::fmt("%llu", *value);
	return ptr + 8;
}


const unsigned char *BrigImmed::dumpImmedSIG32(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int *value = (unsigned int *)ptr;
	os << misc::fmt("sig32(%u)", *value);
	return ptr + 4;
}


const unsigned char * BrigImmed::dumpImmedSIG64(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned long long *value = (unsigned long long *)ptr;
	os << misc::fmt("sig64(%llu)", *value);
	return ptr + 8;
}



const unsigned char * BrigImmed::dumpImmedS8(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = (unsigned char *)this->ptr;
	char *value = (char *)ptr;
	os << misc::fmt("%d", *value);
	return ptr + 1;
}


const unsigned char * BrigImmed::dumpImmedS16(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = (unsigned char *)this->ptr;
	short *value = (short *)ptr;
	os << misc::fmt("%d", *value);
	return ptr + 2;
}


const unsigned char * BrigImmed::dumpImmedS32(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = (unsigned char *)this->ptr;
	int *value = (int *)ptr;
	os << misc::fmt("%d", *value);
	return ptr + 4;
}


const unsigned char * BrigImmed::dumpImmedS64(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = (unsigned char *)this->ptr;
	long long *value = (long long *)ptr;
	os << misc::fmt("%lld", *value);
	return ptr + 8;
}


const unsigned char * BrigImmed::dumpImmedF16(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	os << "0H";
	for (int i=0; i<2; i++)
	{
		const unsigned char *value = &ptr[1-i];
		os << misc::fmt("%02x", (int)*value);
	}
	return ptr + 2;
}


const unsigned char * BrigImmed::dumpImmedF32(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	os << "0F";
	for (int i=0; i<4; i++)
	{
		const unsigned char *value = &ptr[3-i];
		os << misc::fmt("%02x", (int)*value);
	}
	return ptr + 4;
}


const unsigned char * BrigImmed::dumpImmedF64(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	os << "0D";
	for (int i=0; i<8; i++)
	{
		const unsigned char *value = (const unsigned char *) &ptr[7-i];
		os << misc::fmt("%02x", (int)*value);
	}
	return ptr + 8;
}


const unsigned char * BrigImmed::dumpImmedB1(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	if (*ptr)
	{
		os << '1';
	}
	else
	{
		os << '0';
	}
	return ptr + 1;
}


const unsigned char * BrigImmed::dumpImmedB8(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	dumpImmedU8(nullptr, os);
	return ptr + 1;
}


const unsigned char * BrigImmed::dumpImmedB16(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	dumpImmedU16(nullptr, os);
	return ptr + 2;
}


const unsigned char * BrigImmed::dumpImmedB32(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	dumpImmedU32(nullptr, os);
	return ptr + 4;
}


const unsigned char * BrigImmed::dumpImmedB64(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	dumpImmedU64(nullptr, os);
	return ptr + 8;
}


const unsigned char * BrigImmed::dumpImmedB128(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	dumpImmedU64(nullptr, os);
	return ptr + 16;
}


const unsigned char * BrigImmed::dumpImmedSAMP(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	dumpImmedUnsupported(os);
	return ptr;
}


const unsigned char * BrigImmed::dumpImmedROIMG(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	dumpImmedUnsupported(os);
	return ptr;
}


const unsigned char * BrigImmed::dumpImmedRWIMG(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	dumpImmedUnsupported(os);
	return ptr;
}


const unsigned char * BrigImmed::dumpImmedU8X4(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 1;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedU8(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedU8X8(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 1;
	unsigned int pack = 8;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedU8(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedU8X16(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 1;
	unsigned int pack = 16;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "u" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedU8(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedU16X2(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{	
	if (!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedU16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedU16X4(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{	
	if (!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedU16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedU16X8(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{	
	if (!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 8;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedU16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedU32X2(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 4;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedU32(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedU32X4(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{	
	if (!ptr) ptr = this->ptr;
	unsigned int size = 4;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedU32(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedU64X2(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{	
	if (!ptr) ptr = this->ptr;
	unsigned int size = 8;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedU64(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedS8X4(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 1;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedS8(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedS8X8(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{	
	if (!ptr) ptr = this->ptr;
	unsigned int size = 1;
	unsigned int pack = 8;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedS8(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedS8X16(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{	
	if (!ptr) ptr = this->ptr;
	unsigned int size = 1;
	unsigned int pack = 16;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedS8(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedS16X2(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedS16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedS16X4(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedS16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedS16X8(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 8;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedS16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedS32X2(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 4;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedS32(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedS32X4(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 4;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedS32(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedS64X2(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 8;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedS64(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedF16X2(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedF16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedF16X4(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedF16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedF16X8(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 8;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedF16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedF32X2(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 4;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedF32(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedF32X4(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 4;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedF32(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}


const unsigned char * BrigImmed::dumpImmedF64X2(const unsigned char *ptr = nullptr,
		std::ostream &os = std::cout) const
{
	if (!ptr) ptr = this->ptr;
	unsigned int size = 8;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	const unsigned char *value = (const unsigned char *) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for (unsigned int i=0; i<pack; i++)
	{
		if (i > 0) os << ",";
		dumpImmedF64(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}

void BrigImmed::dumpImmedUnsupported(std::ostream &os = std::cout) const
{
	os << "<immed type: " << AsmService::TypeToString(this->type) 
		<< " >";
}


}
