#include "BrigImmed.h"
#include <iomanip>
#include "BrigDef.h"
#include "BrigEntry.h"

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
	{BRIG_TYPE_B128, &BrigImmed::dumpImmedB128},
	{BRIG_TYPE_SAMP, &BrigImmed::dumpImmedSAMP},
	{BRIG_TYPE_ROIMG, &BrigImmed::dumpImmedROIMG},
	{BRIG_TYPE_RWIMG, &BrigImmed::dumpImmedRWIMG},
	{BRIG_TYPE_FBAR, &BrigImmed::dumpImmedFBAR},
	{BRIG_TYPE_U8  | BRIG_TYPE_PACK_32 , &BrigImmed::dumpImmedU8X4},
	{BRIG_TYPE_U8  | BRIG_TYPE_PACK_64 , &BrigImmed::dumpImmedU8X8},
	{BRIG_TYPE_U8  | BRIG_TYPE_PACK_128, &BrigImmed::dumpImmedU8X16},
	{BRIG_TYPE_U16 | BRIG_TYPE_PACK_32 , &BrigImmed::dumpImmedU16X2},
	{BRIG_TYPE_U16 | BRIG_TYPE_PACK_64 , &BrigImmed::dumpImmedU16X4},
	{BRIG_TYPE_U16 | BRIG_TYPE_PACK_128, &BrigImmed::dumpImmedU16X8},
	{BRIG_TYPE_U32 | BRIG_TYPE_PACK_64 , &BrigImmed::dumpImmedU32X2},
	{BRIG_TYPE_U32 | BRIG_TYPE_PACK_128, &BrigImmed::dumpImmedU32X4},
	{BRIG_TYPE_U64 | BRIG_TYPE_PACK_128, &BrigImmed::dumpImmedU64X2},
	{BRIG_TYPE_S8  | BRIG_TYPE_PACK_32 , &BrigImmed::dumpImmedS8X4},
	{BRIG_TYPE_S8  | BRIG_TYPE_PACK_64 , &BrigImmed::dumpImmedS8X8},
	{BRIG_TYPE_S8  | BRIG_TYPE_PACK_128, &BrigImmed::dumpImmedS8X16},
	{BRIG_TYPE_S16 | BRIG_TYPE_PACK_32 , &BrigImmed::dumpImmedS16X2},
	{BRIG_TYPE_S16 | BRIG_TYPE_PACK_64 , &BrigImmed::dumpImmedS16X4},
	{BRIG_TYPE_S16 | BRIG_TYPE_PACK_128, &BrigImmed::dumpImmedS16X8},
	{BRIG_TYPE_S32 | BRIG_TYPE_PACK_64 , &BrigImmed::dumpImmedS32X2},
	{BRIG_TYPE_S32 | BRIG_TYPE_PACK_128, &BrigImmed::dumpImmedS32X4},
	{BRIG_TYPE_S64 | BRIG_TYPE_PACK_128, &BrigImmed::dumpImmedS64X2},
	{BRIG_TYPE_F16 | BRIG_TYPE_PACK_32 , &BrigImmed::dumpImmedF16X2},
	{BRIG_TYPE_F16 | BRIG_TYPE_PACK_64 , &BrigImmed::dumpImmedF16X4},
	{BRIG_TYPE_F16 | BRIG_TYPE_PACK_128, &BrigImmed::dumpImmedF16X8},
	{BRIG_TYPE_F32 | BRIG_TYPE_PACK_64 , &BrigImmed::dumpImmedF32X2},
	{BRIG_TYPE_F32 | BRIG_TYPE_PACK_128, &BrigImmed::dumpImmedF32X4},
	{BRIG_TYPE_F64 | BRIG_TYPE_PACK_128, &BrigImmed::dumpImmedF64X2}
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
	{BRIG_TYPE_FBAR, 	0},
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

int BrigImmed::getSize(){
	return type_to_size_map[type]; 	
}

unsigned char* BrigImmed::dumpImmedNONE(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	dumpImmedUnsupported(os);
	return ptr;
}
unsigned char* BrigImmed::dumpImmedU8(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned char *value = (unsigned char *)ptr;
	os << std::dec << (int)*value;
	return ptr + 1;
}
unsigned char* BrigImmed::dumpImmedU16(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned short *value = (unsigned short *)ptr;
	os << std::dec << *value;
	return ptr + 2;
}
unsigned char* BrigImmed::dumpImmedU32(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int *value = (unsigned int *)ptr;
	os << std::dec << *value;
	return ptr + 4;
}
unsigned char* BrigImmed::dumpImmedU64(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	uint64_t *value = (uint64_t *)ptr;
	os << std::dec << *value;
	return ptr + 8;
}
unsigned char* BrigImmed::dumpImmedS8(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = (unsigned char *)this->ptr;
	char *value = (char *)ptr;
	os << std::dec << (int)*value;
	return ptr + 1;
}
unsigned char* BrigImmed::dumpImmedS16(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = (unsigned char *)this->ptr;
	short *value = (short *)ptr;
	os << std::dec << *value;
	return ptr + 2;
}
unsigned char* BrigImmed::dumpImmedS32(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = (unsigned char *)this->ptr;
	int *value = (int *)ptr;
	os << std::dec << *value;
	return ptr + 4;
}
unsigned char* BrigImmed::dumpImmedS64(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = (unsigned char *)this->ptr;
	long long *value = (long long *)ptr;
	os << std::dec << *value;
	return ptr + 8;
}
unsigned char* BrigImmed::dumpImmedF16(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	os << "0H";
	for(int i=0; i<2; i++)
	{
		unsigned char *value = &ptr[1-i];
		std::cout << std::setfill('0') << std::setw(2) << 
			std::hex << (int)*value;
		std::cout << std::dec;
	}
	return ptr + 2;
}
unsigned char* BrigImmed::dumpImmedF32(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	os << "0F";
	for(int i=0; i<4; i++)
	{
		unsigned char *value = &ptr[3-i];
		std::cout << std::setfill('0') << std::setw(2) << 
			std::hex << (int)*value;
		std::cout << std::dec;
	}
	return ptr + 4;
}
unsigned char* BrigImmed::dumpImmedF64(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	os << "0D";
	for(int i=0; i<8; i++)
	{
		unsigned char *value = (unsigned char*) &ptr[7-i];
		std::cout << std::setfill('0') << std::setw(2) << 
			std::hex << (int)*value;
		std::cout << std::dec;
	}
	return ptr + 8;
}
unsigned char* BrigImmed::dumpImmedB1(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	if(!ptr) ptr = this->ptr;
	if(*ptr)
	{
		os << '1';
	}
	else
	{
		os << '0';
	}
	return ptr + 1;
}
unsigned char* BrigImmed::dumpImmedB8(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	if(!ptr) ptr = this->ptr;
	dumpImmedU8(nullptr, os);
	return ptr + 1;
}
unsigned char* BrigImmed::dumpImmedB16(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	if(!ptr) ptr = this->ptr;
	dumpImmedU16(nullptr, os);
	return ptr + 2;
}
unsigned char* BrigImmed::dumpImmedB32(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	if(!ptr) ptr = this->ptr;
	dumpImmedU32(nullptr, os);
	return ptr + 4;
}
unsigned char* BrigImmed::dumpImmedB64(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	if(!ptr) ptr = this->ptr;
	dumpImmedU64(nullptr, os);
	return ptr + 8;
}
unsigned char* BrigImmed::dumpImmedB128(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	if(!ptr) ptr = this->ptr;
	dumpImmedU64(nullptr, os);
	return ptr + 16;
}
unsigned char* BrigImmed::dumpImmedSAMP(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	if(!ptr) ptr = this->ptr;
	dumpImmedUnsupported(os);
	return ptr;
}
unsigned char* BrigImmed::dumpImmedROIMG(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	if(!ptr) ptr = this->ptr;
	dumpImmedUnsupported(os);
	return ptr;
}
unsigned char* BrigImmed::dumpImmedRWIMG(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	if(!ptr) ptr = this->ptr;
	dumpImmedUnsupported(os);
	return ptr;
}
unsigned char* BrigImmed::dumpImmedFBAR(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	if(!ptr) ptr = this->ptr;
	dumpImmedUnsupported(os);
	return ptr;
}
unsigned char* BrigImmed::dumpImmedU8X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 1;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU8(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedU8X8(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 1;
	unsigned int pack = 8;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU8(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedU8X16(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 1;
	unsigned int pack = 16;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU8(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedU16X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{	
	if(!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedU16X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{	
	if(!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedU16X8(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{	
	if(!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 8;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedU32X2(unsigned char *ptr = nullptr, std::ostream &os = std::cout) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 4;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU32(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedU32X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{	
	if(!ptr) ptr = this->ptr;
	unsigned int size = 4;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU32(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedU64X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{	
	if(!ptr) ptr = this->ptr;
	unsigned int size = 8;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_u" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedU64(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedS8X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 1;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS8(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedS8X8(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{	
	if(!ptr) ptr = this->ptr;
	unsigned int size = 1;
	unsigned int pack = 8;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS8(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedS8X16(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{	
	if(!ptr) ptr = this->ptr;
	unsigned int size = 1;
	unsigned int pack = 16;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS8(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedS16X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedS16X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedS16X8(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 8;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedS32X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 4;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS32(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedS32X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 4;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS32(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedS64X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 8;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_s" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedS64(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedF16X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedF16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedF16X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedF16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedF16X8(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 2;
	unsigned int pack = 8;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedF16(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedF32X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 4;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedF32(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedF32X4(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 4;
	unsigned int pack = 4;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedF32(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}
unsigned char* BrigImmed::dumpImmedF64X2(
		unsigned char *ptr = nullptr, 
		std::ostream &os = std::cout
	) const
{
	if(!ptr) ptr = this->ptr;
	unsigned int size = 8;
	unsigned int pack = 2;
	unsigned int lastIndex = size * pack - size;
	unsigned char *value = (unsigned char*) &ptr[lastIndex];
	os << "_f" << size*8 << "x" << pack << "(";
	for(unsigned int i=0; i<pack; i++)
	{
		if(i > 0) os << ",";
		dumpImmedF64(value, os);
		value -= size;
	}
	os << ")";
	return ptr + size*pack;
}

void BrigImmed::dumpImmedUnsupported(std::ostream &os = std::cout) const
{
	os << "<immed type: " << BrigEntry::type2str(this->type) 
		<< " >";
}


}