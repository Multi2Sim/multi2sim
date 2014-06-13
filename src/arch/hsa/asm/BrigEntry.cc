#include "BrigEntry.h"
#include "BrigStrEntry.h"
#include "BrigDirEntry.h"
#include "BrigInstEntry.h"
#include "lib/cpp/Misc.h"
#include "lib/cpp/String.h"
#include "Asm.h"

namespace HSA{
const char *BrigEntry::type2str(int type)
{
	switch(type)
	{
		case 0:	return "";
		case 1: return "u8";
		case 2: return "u16";
		case 3: return "u32";
		case 4: return "u64";
		case 5: return "s8";
		case 6: return "s16";
		case 7: return "s32";
		case 8: return "s64";
		case 9: return "f16";
		case 10: return "f32";
		case 11: return "f64";
		case 12: return "b1";
		case 13: return "b8";
		case 14: return "b16";
		case 15: return "b32";
		case 16: return "b64";
		case 17: return "b128";
		case 18: return "samp";
		case 19: return "roimg";
		case 20: return "rwimg";
		case 21: return "fbar";

		case BRIG_TYPE_U8|BRIG_TYPE_PACK_32: return "u8x4";
		case BRIG_TYPE_U8|BRIG_TYPE_PACK_64: return "u8x8";
		case BRIG_TYPE_U8|BRIG_TYPE_PACK_128: return "u8x16";
		case BRIG_TYPE_U16|BRIG_TYPE_PACK_32: return "u16x2";
		case BRIG_TYPE_U16|BRIG_TYPE_PACK_64: return "u16x4";
		case BRIG_TYPE_U16|BRIG_TYPE_PACK_128: return "u16x8";
		case BRIG_TYPE_U32|BRIG_TYPE_PACK_64: return "u32x2";
		case BRIG_TYPE_U32|BRIG_TYPE_PACK_128: return "u32x4";
		case BRIG_TYPE_U64|BRIG_TYPE_PACK_128: return "u64x2";
		case BRIG_TYPE_S8|BRIG_TYPE_PACK_32: return "s8x4";
		case BRIG_TYPE_S8|BRIG_TYPE_PACK_64: return "s8x8";
		case BRIG_TYPE_S8|BRIG_TYPE_PACK_128: return "s8x16";
		case BRIG_TYPE_S16|BRIG_TYPE_PACK_32: return "s16x2";
		case BRIG_TYPE_S16|BRIG_TYPE_PACK_64: return "s16x4";
		case BRIG_TYPE_S16|BRIG_TYPE_PACK_128: return "s16x8";
		case BRIG_TYPE_S32|BRIG_TYPE_PACK_64: return "s32x2";
		case BRIG_TYPE_S32|BRIG_TYPE_PACK_128: return "s32x4";
		case BRIG_TYPE_S64|BRIG_TYPE_PACK_128: return "s64x2";
		case BRIG_TYPE_F16|BRIG_TYPE_PACK_32: return "f16x2";
		case BRIG_TYPE_F16|BRIG_TYPE_PACK_64: return "f16x4";
		case BRIG_TYPE_F16|BRIG_TYPE_PACK_128: return "f16x8";
		case BRIG_TYPE_F32|BRIG_TYPE_PACK_64: return "f32x2";
		case BRIG_TYPE_F32|BRIG_TYPE_PACK_128: return "f32x4";
		case BRIG_TYPE_F64|BRIG_TYPE_PACK_128: return "f64x2";
		default: 
			misc::warning("Unsupported type!");
			return "";
	}
	return "";
}

const char *BrigEntry::profile2str(int profile)
{
	if(profile == 1)
	{
		return "$full";
	}
	else if(profile == 0)
	{
		return "$base";
	}
	return "Invalid profile";
}

const char *BrigEntry::machineModel2str(int machineModel)
{
	if(machineModel == 1)
	{
		return "$large";
	}
	else if(machineModel == 0)
	{
		return "$small";
	}
	return "Invalid machine model";
}

const char *BrigEntry::align2str(unsigned char align)
{
	std::stringstream ss;
	if(align > 1)
	{
		ss << "align " << (unsigned)align << ' ';	
	}
	return ss.str().c_str();
}


const char *BrigEntry::seg2str(unsigned char seg)
{
	switch(seg)
	{
		case 0:	return "";	
		case 1: return "";
		case 2: return "global";
		case 3: return "readonly";
		case 4: return "kernarg";
		case 5: return "group";
		case 6: return "private";
		case 7: return "spill";
		case 8: return "arg";
		default: misc::warning("Unsupported segment!");
	}
	return "";
}

misc::StringMap BrigEntry::sem_to_str_map = 
{
	{"", 0},
	{"", 1},
	{"acq", 2},
	{"rel", 3},
	{"ar", 4},
	{"part_acq", 5},
	{"part_rel", 6},
	{"part_ar", 7}
};

const char *BrigEntry::sem2str(unsigned char modifier) const
{
	unsigned char sem = modifier & BRIG_MEMORY_SEMANTIC;
	return sem_to_str_map.MapValue(sem);
}

void BrigEntry::dumpValueList(
		BrigDataOffset32_t data,
		BrigType16_t type,
		uint32_t elementCount,
		BrigFile *file,
		std::ostream &os = std::cout
	)
{
	BrigSection *stringSection = file->getBrigSection(BrigSectionString);
	char *temp = (char *)stringSection->getBuffer();
	temp += data;
	switch(type)
	{
		case 0:	DumpBrigTypeNone(temp, elementCount, os); break;
		case 1: DumpBrigTypeU8(temp, elementCount, os); break;
		case 2: DumpBrigTypeU16(temp, elementCount, os); break;
		case 3: DumpBrigTypeU32(temp, elementCount, os); break;
		case 4: DumpBrigTypeU64(temp, elementCount, os); break;
		case 5: DumpBrigTypeS8(temp, elementCount, os); break;
		case 6: DumpBrigTypeS16(temp, elementCount, os); break;
		case 7: DumpBrigTypeS32(temp, elementCount, os); break;
		case 8: DumpBrigTypeS64(temp, elementCount, os); break;
		case 9: DumpBrigTypeF16(temp, elementCount, os); break;
		case 10: DumpBrigTypeF32(temp, elementCount, os); break;
		case 11: DumpBrigTypeF64(temp, elementCount, os); break;
		case 12: DumpBrigTypeB1(temp, elementCount, os); break;
		case 13: DumpBrigTypeB8(temp, elementCount, os); break;
		case 14: DumpBrigTypeB16(temp, elementCount, os); break;
		case 15: DumpBrigTypeB32(temp, elementCount, os); break;
		case 16: DumpBrigTypeB64(temp, elementCount, os); break;
		case 17: DumpBrigTypeB128(temp, elementCount, os); break;
		case 18: DumpBrigTypeSamp(temp, elementCount, os); break;
		case 19: DumpBrigTypeRoimg(temp, elementCount, os); break;
		case 20: DumpBrigTypeRwimg(temp, elementCount, os); break;
		case 21: DumpBrigTypeFbar(temp, elementCount, os); break;

		case BRIG_TYPE_U8|BRIG_TYPE_PACK_32: DumpBrigTypeU8X4(temp, elementCount, os); break;
		case BRIG_TYPE_U8|BRIG_TYPE_PACK_64: DumpBrigTypeU8X8(temp, elementCount, os); break;
		case BRIG_TYPE_U8|BRIG_TYPE_PACK_128: DumpBrigTypeU8X16(temp, elementCount, os); break;
		case BRIG_TYPE_U16|BRIG_TYPE_PACK_32: DumpBrigTypeU16X2(temp, elementCount, os); break;
		case BRIG_TYPE_U16|BRIG_TYPE_PACK_64: DumpBrigTypeU16X4(temp, elementCount, os); break;
		case BRIG_TYPE_U16|BRIG_TYPE_PACK_128: DumpBrigTypeU16X8(temp, elementCount, os); break;
		case BRIG_TYPE_U32|BRIG_TYPE_PACK_64: DumpBrigTypeU32X2(temp, elementCount, os); break;
		case BRIG_TYPE_U32|BRIG_TYPE_PACK_128: DumpBrigTypeU32X4(temp, elementCount, os); break;
		case BRIG_TYPE_U64|BRIG_TYPE_PACK_128: DumpBrigTypeU64X2(temp, elementCount, os); break;
		case BRIG_TYPE_S8|BRIG_TYPE_PACK_32: DumpBrigTypeS8X4(temp, elementCount, os); break;
		case BRIG_TYPE_S8|BRIG_TYPE_PACK_64: DumpBrigTypeS8X8(temp, elementCount, os); break;
		case BRIG_TYPE_S8|BRIG_TYPE_PACK_128: DumpBrigTypeS8X16(temp, elementCount, os); break;
		case BRIG_TYPE_S16|BRIG_TYPE_PACK_32: DumpBrigTypeS16X2(temp, elementCount, os); break;
		case BRIG_TYPE_S16|BRIG_TYPE_PACK_64: DumpBrigTypeS16X4(temp, elementCount, os); break;
		case BRIG_TYPE_S16|BRIG_TYPE_PACK_128: DumpBrigTypeS16X8(temp, elementCount, os); break;
		case BRIG_TYPE_S32|BRIG_TYPE_PACK_64: DumpBrigTypeS32X2(temp, elementCount, os); break;
		case BRIG_TYPE_S32|BRIG_TYPE_PACK_128: DumpBrigTypeS32X4(temp, elementCount, os); break;
		case BRIG_TYPE_S64|BRIG_TYPE_PACK_128: DumpBrigTypeS64X2(temp, elementCount, os); break;
		case BRIG_TYPE_F16|BRIG_TYPE_PACK_32: DumpBrigTypeF16X2(temp, elementCount, os); break;
		case BRIG_TYPE_F16|BRIG_TYPE_PACK_64: DumpBrigTypeF16X4(temp, elementCount, os); break;
		case BRIG_TYPE_F16|BRIG_TYPE_PACK_128: DumpBrigTypeF16X8(temp, elementCount, os); break;
		case BRIG_TYPE_F32|BRIG_TYPE_PACK_64: DumpBrigTypeF32X2(temp, elementCount, os); break;
		case BRIG_TYPE_F32|BRIG_TYPE_PACK_128: DumpBrigTypeF32X4(temp, elementCount, os); break;
		case BRIG_TYPE_F64|BRIG_TYPE_PACK_128: DumpBrigTypeF64X2(temp, elementCount, os); break;
		default:
			misc::warning("Unsupported type!"); break;
	}
}

void BrigEntry::DumpBrigTypeNone(void *data, uint32_t count, std::ostream &os = std::cout)
{
	misc::warning("Unknown dump format for type: %s", "None");
}

void BrigEntry::DumpBrigTypeU8(void *data, uint32_t count, std::ostream &os = std::cout)
{
	uint8_t *ptr = (uint8_t *)data;
	if(count == 1)
		os << *ptr << ";\n";
	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << *ptr << "};\n";
	}
}

void BrigEntry::DumpBrigTypeU16(void *data, uint32_t count, std::ostream &os = std::cout)
{
	uint16_t *ptr = (uint16_t *)data;
	if(count == 1)
		os << *ptr << ";\n";
	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << *ptr << "};\n";
	}
}

void BrigEntry::DumpBrigTypeU32(void *data, uint32_t count, std::ostream &os = std::cout)
{
	uint32_t *ptr = (uint32_t *)data;
	if(count == 1)
		os << *ptr << ";\n";
	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << *ptr << "};\n";
	}
}

void BrigEntry::DumpBrigTypeU64(void *data, uint32_t count, std::ostream &os = std::cout)
{
	uint64_t *ptr = (uint64_t *)data;
	if(count == 1)
		os << *ptr << ";\n";
	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << *ptr << "};\n";
	}
}

void BrigEntry::DumpBrigTypeS8(void *data, uint32_t count, std::ostream &os = std::cout)
{
	int8_t *ptr = (int8_t *)data;
	if(count == 1)
		os << *ptr << ";\n";
	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << *ptr << "};\n";
	}
}

void BrigEntry::DumpBrigTypeS16(void *data, uint32_t count, std::ostream &os = std::cout)
{
	int16_t *ptr = (int16_t *)data;
	if(count == 1)
		os << *ptr << ";\n";
	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << *ptr << "};\n";
	}
}

void BrigEntry::DumpBrigTypeS32(void *data, uint32_t count, std::ostream &os = std::cout)
{
	int32_t *ptr = (int32_t *)data;
	if(count == 1)
		os << *ptr << ";\n";
	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << *ptr << "};\n";
	}
}

void BrigEntry::DumpBrigTypeS64(void *data, uint32_t count, std::ostream &os = std::cout)
{
	int64_t *ptr = (int64_t *)data;
	if(count == 1)
		os << *ptr << ";\n";
	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << *ptr << "};\n";
	}
}

void BrigEntry::DumpBrigTypeF16(void *data, uint32_t count, std::ostream &os = std::cout)
{
	misc::warning("type %s not supported by standard c", "half-precision");
}

void BrigEntry::DumpBrigTypeF32(void *data, uint32_t count, std::ostream &os = std::cout)
{
	float *ptr = (float *)data;
	if(count == 1)
		os << *ptr << ";\n";
	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << *ptr << "};\n";
	}
}

void BrigEntry::DumpBrigTypeF64(void *data, uint32_t count, std::ostream &os = std::cout)
{
	double *ptr = (double *)data;
	if(count == 1)
		os << *ptr << ";\n";
	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << *ptr << "};\n";
	}
}

void BrigEntry::DumpBrigTypeB1(void *data, uint32_t count, std::ostream &os = std::cout)
{
	misc::warning("Unknown dump format for type: %s", "B1");
	os << "\n";
}

void BrigEntry::DumpBrigTypeB8(void *data, uint32_t count, std::ostream &os = std::cout)
{
	misc::warning("Unknown dump format for type: %s", "B8");
	os << "\n";
}

void BrigEntry::DumpBrigTypeB16(void *data, uint32_t count, std::ostream &os = std::cout)
{
	misc::warning("Unknown dump format for type: %s", "B16");
	os << "\n";
}

void BrigEntry::DumpBrigTypeB32(void *data, uint32_t count, std::ostream &os = std::cout)
{
	misc::warning("Unknown dump format for type: %s", "B32");
	os << "\n";
}

void BrigEntry::DumpBrigTypeB64(void *data, uint32_t count, std::ostream &os = std::cout)
{
	misc::warning("Unknown dump format for type: %s", "B64");
	os << "\n";
}

void BrigEntry::DumpBrigTypeB128(void *data, uint32_t count, std::ostream &os = std::cout)
{
	misc::warning("Unknown dump format for type: %s", "B128");
	os << "\n";
}

void BrigEntry::DumpBrigTypeSamp(void *data, uint32_t count, std::ostream &os = std::cout)
{
	misc::warning("Unknown dump format for type: %s", "Samp");
	os << "\n";
}

void BrigEntry::DumpBrigTypeRoimg(void *data, uint32_t count, std::ostream &os = std::cout)
{
	misc::warning("Unknown dump format for type: %s", "Roimg");
	os << "\n";
}

void BrigEntry::DumpBrigTypeRwimg(void *data, uint32_t count, std::ostream &os = std::cout)
{
	misc::warning("Unknown dump format for type: %s", "Rwimg");
	os << "\n";
}

void BrigEntry::DumpBrigTypeFbar(void *data, uint32_t count, std::ostream &os = std::cout)
{
	misc::warning("Unknown dump format for type: %s", "Fbar");
	os << "\n";
}

void BrigEntry::DumpBrigTypeU8X4(void *data, uint32_t count, std::ostream &os = std::cout)
{
	uint8_t *ptr = (uint8_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 4; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 4; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 4; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeU8X8(void *data, uint32_t count, std::ostream &os = std::cout)
{
	uint8_t *ptr = (uint8_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 8; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 8; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 8; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeU8X16(void *data, uint32_t count, std::ostream &os = std::cout)
{
	uint8_t *ptr = (uint8_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 16; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 16; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 16; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeU16X2(void *data, uint32_t count, std::ostream &os = std::cout)
{
	uint16_t *ptr = (uint16_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 2; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 2; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 2; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeU16X4(void *data, uint32_t count, std::ostream &os = std::cout)
{
	uint16_t *ptr = (uint16_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 4; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 4; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 4; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeU16X8(void *data, uint32_t count, std::ostream &os = std::cout)
{
	uint16_t *ptr = (uint16_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 8; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 8; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 8; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeU32X2(void *data, uint32_t count, std::ostream &os = std::cout)
{
	uint32_t *ptr = (uint32_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 2; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 2; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 2; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeU32X4(void *data, uint32_t count, std::ostream &os = std::cout)
{
	uint32_t *ptr = (uint32_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 4; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 4; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 4; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeU64X2(void *data, uint32_t count, std::ostream &os = std::cout)
{
	uint64_t *ptr = (uint64_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 2; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 2; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 2; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeS8X4(void *data, uint32_t count, std::ostream &os = std::cout)
{
	int8_t *ptr = (int8_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 4; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 4; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 4; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeS8X8(void *data, uint32_t count, std::ostream &os = std::cout)
{
	int8_t *ptr = (int8_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 8; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 8; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 8; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeS8X16(void *data, uint32_t count, std::ostream &os = std::cout)
{
	int8_t *ptr = (int8_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 16; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 16; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 16; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeS16X2(void *data, uint32_t count, std::ostream &os = std::cout)
{
	int16_t *ptr = (int16_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 2; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 2; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 2; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeS16X4(void *data, uint32_t count, std::ostream &os = std::cout)
{
	int16_t *ptr = (int16_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 4; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 4; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 4; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeS16X8(void *data, uint32_t count, std::ostream &os = std::cout)
{
	int16_t *ptr = (int16_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 8; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 8; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 8; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeS32X2(void *data, uint32_t count, std::ostream &os = std::cout)
{
	int32_t *ptr = (int32_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 2; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 2; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 2; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeS32X4(void *data, uint32_t count, std::ostream &os = std::cout)
{
	int32_t *ptr = (int32_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 4; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 4; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 4; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeS64X2(void *data, uint32_t count, std::ostream &os = std::cout)
{
	int64_t *ptr = (int64_t *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 2; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 2; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 2; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeF16X2(void *data, uint32_t count, std::ostream &os = std::cout)
{
	misc::warning("type %s not supported by standard c", "half-precision");
	os << "\n";
}

void BrigEntry::DumpBrigTypeF16X4(void *data, uint32_t count, std::ostream &os = std::cout)
{
	misc::warning("type %s not supported by standard c", "half-precision");
	os << "\n";
}

void BrigEntry::DumpBrigTypeF16X8(void *data, uint32_t count, std::ostream &os = std::cout)
{
	misc::warning("type %s not supported by standard c", "half-precision");
	os << "\n";
}

void BrigEntry::DumpBrigTypeF32X2(void *data, uint32_t count, std::ostream &os = std::cout)
{
	float *ptr = (float *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 2; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 2; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 2; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeF32X4(void *data, uint32_t count, std::ostream &os = std::cout)
{
	float *ptr = (float *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 4; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 4; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 4; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}

void BrigEntry::DumpBrigTypeF64X2(void *data, uint32_t count, std::ostream &os = std::cout)
{
	double *ptr = (double *)data;
	if(count == 1)
	{
		os << "(";
		for(int i = 1; i < 2; i++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ");\n";
	}

	else if(count != 0)
	{
		os << "{ ";
		for(unsigned i = 0; i < count - 1; i++)
		{
			os << "(";
			for(int j = 1; j < 2; j++)
			{
				os << *ptr << ", ";
				ptr++;
			}
			os << ptr << "), ";
		}
		os << "(";
		for(int j = 1; j < 2; j++)
		{
			os << *ptr << ", ";
			ptr++;
		}
		os << ptr << ")};\n";
	}
}


void BrigEntry::dumpValue(char *value) const
{
}

void BrigEntry::dumpSymDecl(
		const BrigEntry *dir,
		std::ostream &os = std::cout
	)
{
	struct BrigDirectiveSymbol *sym = (struct BrigDirectiveSymbol *)dir->base;
	SymbolModifier modifier(sym->modifier.allBits);
	// extern, static
	os << modifier.getLinkageStr();
	// const
	os << modifier.getConstStr();
	// align
	os << BrigEntry::align2str(sym->align);
	// segment
	os << BrigEntry::seg2str(sym->segment);
	// type
	os << '_' << BrigEntry::type2str(sym->type);
	// name
	os << ' ' << BrigStrEntry::GetStringByOffset(dir->file, sym->name);
	uint64_t dim = (uint64_t(sym->dimHi) << 32) | uint64_t(sym->dimLo);
	if(
		modifier.isFlexArray() ||
		( dim == 0 && modifier.isArray() && modifier.isDeclaration() )
	){
		os << "[]";
	}
	else if(modifier.isArray())
	{
		os << "[" << dim << "]";
	}
}

char *BrigEntry::dumpArgs(
		char *arg, 
		unsigned short argCount,
		BrigFile * file,
		std::ostream &os = std::cout
	)
{
	os << "(";	
	char *next = arg;
	if(argCount == 1)
	{
		BrigDirEntry dir(next, file);
		dumpSymDecl(&dir, os);
		next = dir.nextTop();
	}
	else if(argCount >1)
	{
		Asm *as = Asm::getInstance();
		as->indent++;
		for(int i  = 0; i < argCount; i++)
		{
			os << "\n";
			dumpIndent(os);
			BrigDirEntry dir(next, file);
			dumpSymDecl(&dir, os);
			next = dir.nextTop();
			if(i != argCount - 1)
				os << ",";
		}
		as->indent--;
	}	
	os << ")";
	return next;
}

void BrigEntry::dumpBody(
		int codeOffset,
		int nInst,
		char *next,
		bool isDecl, 
		std::ostream &os = std::cout
	) const
{
	if(!isDecl)
	{
		Asm *as = Asm::getInstance();
		as->indent++;
		os << "\n{\n";
		BrigSection *bsc = this->file->getBrigSection(BrigSectionCode);
		char *bsc_buffer = (char *)bsc->getBuffer();
		char *bufPtr = bsc_buffer + codeOffset;
		struct BrigDirectiveBase *dirPtr = (struct BrigDirectiveBase *)next;
		for(int i=0; i<nInst; i++)
		{	
			while(dirPtr && dirPtr->code <= (BrigCodeOffset32_t)(bufPtr - bsc_buffer))
			{
				BrigDirEntry dir((char *)dirPtr, this->file);
				dir.Dump(os);
				dirPtr = (struct BrigDirectiveBase *)dir.nextTop();
			}
			//printf("Inst %d/%d", i, nInst);
			BrigInstEntry inst(bufPtr, this->file);
			dumpIndent(os);
			inst.Dump(os);
			bufPtr = inst.next();
		}
		as->indent--;
		os << "}";
	}	
	os << ";\n";
}

void BrigEntry::dumpIndent(std::ostream &os = std::cout)
{
	Asm *as = Asm::getInstance();
	for(int i = 0; i < as->indent; i++)
		os << "\t";
}

BrigEntry::BrigEntry(char *buf, BrigFile* file)
{
	this->base = buf;
	this->file = file;
}

unsigned int BrigEntry::getSize() const
{
	uint16_t *size = (uint16_t *) base;
	// Considering the padding
	*size = ((*size) + 3 ) / 4 * 4;
	return *size;
}

void BrigEntry::dumpHex(std::ostream &os = std::cout) const
{
	unsigned char *buf = (unsigned char *)base;
	for(unsigned int i=0; i<this->getSize(); i++)
	{
		os << misc::fmt("%02x", buf[i]);
		if((i+1) % 4 == 0) os<<misc::fmt(" ");
	}
	os << "\n";
}

void BrigEntry::Dump(std::ostream &os = std::cout) const {};

	
}
