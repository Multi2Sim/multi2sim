#ifndef HSA_ASM_BRIGENTRY_H
#define HSA_ASM_BRIGENTRY_H

#include <cstring>
#include "lib/cpp/Misc.h"
#include "lib/cpp/String.h"

#include "BrigFile.h"
#include "BrigDef.h"
#include "SymbolModifier.h"

namespace HSA{
/// Pure virtual class representing a brig entry
class BrigEntry
{
protected: 
	// Pointer to the first byte of the entry
	char *base;
	// Brig file that this entry belongs to 
	BrigFile *file;

	// Converts type to type string
	static const char *type2str(int type);

	// Convert profile to string to print out
	static const char *profile2str(int profile);
	
	// Convert profile to string
	static const char *machineModel2str(int machineModel);

	// Returns the align string
	static const char *align2str(unsigned char align);

	// Returns segments as string 
	// Segments are attributes such as global, readonly...
	static const char *seg2str(unsigned char seg);

	// Maps memory semantic to string
	static misc::StringMap sem_to_str_map;

	// Convert memory semantic 
	const char *sem2str(unsigned char modifier) const;

	// Dump value list
	static void dumpValueList(
			BrigDataOffset32_t data,
			BrigType16_t type, 
			uint32_t elementCount,
			BrigFile *file,
			std::ostream &os
		);

	// Dump Symbol Declaration
	static void dumpSymDecl(
			const BrigEntry *dir,
			std::ostream &os
		);
	
	// Dump Argument list
	// Returns a pointer to the next directive to be processed
	static char *dumpArgs(
			char *arg, 
			unsigned short argCount,
			BrigFile *file,
			std::ostream &os
		);

	// Dump _str if str is not empty, otherwise do nothing
	void dump_(const char *str, std::ostream &os = std::cout) const{
		if(strlen(str)){
			os << "_" << str;
		}
	}
	
	// Dumps a numberic value
	void dumpValue(char *value) const;
	
	// Dump the function or kernel body
	void dumpBody(
			int codeOffset,
			int nInst,
			char *next,
			bool isDecl, 
			std::ostream &os
		) const;

	// Dump the tab for indentation
	static void dumpIndent(std::ostream &os);

	static void DumpBrigTypeNone(void *data, uint32_t count, std::ostream &os);	// 0	//.mnemo=""
	static void DumpBrigTypeU8(void *data, uint32_t count, std::ostream &os);		// 1	//.ctype=uint8_t  // unsigned integer 8 bits
	static void DumpBrigTypeU16(void *data, uint32_t count, std::ostream &os);		// 2	//.ctype=uint16_t // unsigned integer 16 bits
	static void DumpBrigTypeU32(void *data, uint32_t count, std::ostream &os);		// 3	//.ctype=uint32_t // unsigned integer 32 bits
	static void DumpBrigTypeU64(void *data, uint32_t count, std::ostream &os);		// 4	//.ctype=uint64_t // unsigned integer 64 bits
	static void DumpBrigTypeS8(void *data, uint32_t count, std::ostream &os);		// 5	//.ctype=int8_t   // signed integer 8 bits
	static void DumpBrigTypeS16(void *data, uint32_t count, std::ostream &os);		// 6	//.ctype=int16_t  // signed integer 16 bits
	static void DumpBrigTypeS32(void *data, uint32_t count, std::ostream &os);		// 7	//.ctype=int32_t  // signed integer 32 bits
	static void DumpBrigTypeS64(void *data, uint32_t count, std::ostream &os);		// 8	//.ctype=int64_t  // signed integer 64 bits
	static void DumpBrigTypeF16(void *data, uint32_t count, std::ostream &os);		// 9	//.ctype=f16_t    // floating-point 16 bits
	static void DumpBrigTypeF32(void *data, uint32_t count, std::ostream &os);		// 10	//.ctype=float    // floating-point 32 bits
	static void DumpBrigTypeF64(void *data, uint32_t count, std::ostream &os);		// 11	//.ctype=double   // floating-point 64 bits
	static void DumpBrigTypeB1(void *data, uint32_t count, std::ostream &os);		// 12	//.ctype=bool     // uninterpreted bit string of length 1 bit
	static void DumpBrigTypeB8(void *data, uint32_t count, std::ostream &os);		// 13	//.ctype=uint8_t  // uninterpreted bit string of length 8 bits
	static void DumpBrigTypeB16(void *data, uint32_t count, std::ostream &os);		// 14	//.ctype=uint16_t // uninterpreted bit string of length 16 bits
	static void DumpBrigTypeB32(void *data, uint32_t count, std::ostream &os);		// 15	//.ctype=uint32_t // uninterpreted bit string of length 32 bits
	static void DumpBrigTypeB64(void *data, uint32_t count, std::ostream &os);		// 16	//.ctype=uint64_t // uninterpreted bit string of length 64 bits
	static void DumpBrigTypeB128(void *data, uint32_t count, std::ostream &os);	// 17	//.ctype=b128_t   // uninterpreted bit string of length 128 bits
	static void DumpBrigTypeSamp(void *data, uint32_t count, std::ostream &os);	// 18	//.mnemo=samp  // sampler object
	static void DumpBrigTypeRoimg(void *data, uint32_t count, std::ostream &os);	// 19	//.mnemo=roimg // read-only image object
	static void DumpBrigTypeRwimg(void *data, uint32_t count, std::ostream &os);	// 20	//.mnemo=rwimg // read/write image object
	static void DumpBrigTypeFbar(void *data, uint32_t count, std::ostream &os);	// 21
	static void DumpBrigTypeU8X4(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_U8  | BRIG_TYPE_PACK_32   //.ctype=uint8_t  // four bytes unsigned
	static void DumpBrigTypeU8X8(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_U8  | BRIG_TYPE_PACK_64,   //.ctype=uint8_t  // eight bytes unsigned
	static void DumpBrigTypeU8X16(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_U8  | BRIG_TYPE_PACK_128,  //.ctype=uint8_t  // 16 bytes unsigned
	static void DumpBrigTypeU16X2(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_U16 | BRIG_TYPE_PACK_32,   //.ctype=uint16_t // two short unsigned integers
	static void DumpBrigTypeU16X4(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_U16 | BRIG_TYPE_PACK_64,   //.ctype=uint16_t // four short unsigned integers
	static void DumpBrigTypeU16X8(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_U16 | BRIG_TYPE_PACK_128,  //.ctype=uint16_t // eight short unsigned integer
	static void DumpBrigTypeU32X2(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_U32 | BRIG_TYPE_PACK_64,   //.ctype=uint32_t // two unsigned integers
	static void DumpBrigTypeU32X4(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_U32 | BRIG_TYPE_PACK_128,  //.ctype=uint32_t // four unsigned integers
	static void DumpBrigTypeU64X2(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_U64 | BRIG_TYPE_PACK_128,  //.ctype=uint64_t // two 64-bit unsigned integers
	static void DumpBrigTypeS8X4(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_S8  | BRIG_TYPE_PACK_32,   //.ctype=int8_t   // four bytes signed
	static void DumpBrigTypeS8X8(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_S8  | BRIG_TYPE_PACK_64,   //.ctype=int8_t   // eight bytes signed
	static void DumpBrigTypeS8X16(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_S8  | BRIG_TYPE_PACK_128,  //.ctype=int8_t   // 16 bytes signed
	static void DumpBrigTypeS16X2(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_S16 | BRIG_TYPE_PACK_32,   //.ctype=int16_t  // two short signed integers
	static void DumpBrigTypeS16X4(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_S16 | BRIG_TYPE_PACK_64,   //.ctype=int16_t  // four short signed integers
	static void DumpBrigTypeS16X8(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_S16 | BRIG_TYPE_PACK_128,  //.ctype=int16_t  // eight short signed integers
	static void DumpBrigTypeS32X2(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_S32 | BRIG_TYPE_PACK_64,   //.ctype=int32_t  // two signed integers
	static void DumpBrigTypeS32X4(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_S32 | BRIG_TYPE_PACK_128,  //.ctype=int32_t  // four signed integers
	static void DumpBrigTypeS64X2(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_S64 | BRIG_TYPE_PACK_128,  //.ctype=int64_t  // two 64-bit signed integers
	static void DumpBrigTypeF16X2(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_F16 | BRIG_TYPE_PACK_32,   //.ctype=f16_t    // two half-floats
	static void DumpBrigTypeF16X4(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_F16 | BRIG_TYPE_PACK_64,   //.ctype=f16_t    // four half-floats
	static void DumpBrigTypeF16X8(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_F16 | BRIG_TYPE_PACK_128,  //.ctype=f16_t    // eight half-floats
	static void DumpBrigTypeF32X2(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_F32 | BRIG_TYPE_PACK_64,   //.ctype=float    // two floats
	static void DumpBrigTypeF32X4(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_F32 | BRIG_TYPE_PACK_128,  //.ctype=float    // four floats
	static void DumpBrigTypeF64X2(void *data, uint32_t count, std::ostream &os);	// BRIG_TYPE_F64 | BRIG_TYPE_PACK_128   //.ctype=double   // two doubles

public:
	/// Constructor
	BrigEntry(char *buf, BrigFile *file);

	/// Returns the pointer to the buffer of the entry
	char *getBuffer() const{return base;}

	/// Returns the size of the entry, the size is paddint into 4 bytes
	/// String entry should have different implementation, since the size
	/// field in string entry is 4 bytes long
	virtual unsigned int getSize() const;

	/// Dump the entry in HEX
	virtual void dumpHex(std::ostream &os) const;

	/// Virtual function for dump assembly
	virtual void Dump(std::ostream &os) const;

	/// Returns the pointer to the next entry
	/// FIX: This function is not safe since it may return a pointer out of current Brig Section
	virtual char *next() const{return this->base + this->getSize();}

};
}

#endif
