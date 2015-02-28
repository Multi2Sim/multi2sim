/* 
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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


#include <sstream>
#include <memory>
#include <string>
#include <vector>

#include <arch/southern-islands/asm/Arg.h>
#include <arch/southern-islands/asm/Binary.h>
#include <lib/cpp/ELFWriter.h>
#include <lib/cpp/Misc.h>

#include "Metadata.h"
#include "InternalBinary.h"
#include "Binary.h"
#include "Context.h"

namespace si2bin
{

/* Global Variables */
const unsigned int NUM_PROG_INFO_ELEM = 114;
const unsigned int MAX_UAV_NUM = 25;
const unsigned int MAX_CB_NUM = 25;
const unsigned int NUM_NOTES = 17;


misc::StringMap OuterBinDeviceMap = 
{
	{"invalid", OuterBinInvalid },
	{"capeverde", OuterBinCapeVerde },
	{"pitcairn", OuterBinPitcairn },
	{"tahiti", OuterBinTahiti }
};


void OuterBin::Generate(std::ostream& os)
{
	InnerBin *inner_bin;
	InnerBinEntry *entry;

	ELFWriter::Section *text_section;
	ELFWriter::Section *rodata_section;
	ELFWriter::SymbolTable *symbol_table;
	ELFWriter::SymbolTable *inner_symbol_table;
	ELFWriter::Buffer *rodata_buffer;
	ELFWriter::Buffer *text_buffer;
	ELFWriter::Symbol *global_symbol;
	ELFWriter::Symbol *header_symbol;
	ELFWriter::Symbol *metadata_symbol;
	ELFWriter::Symbol *kernel_symbol;
	ELFWriter::Symbol *uav_symbol;
	ELFWriter::Symbol *cb_symbol;

	Metadata *metadata;
	SI::BinaryUserElement *user_elem;
	//SI::BinaryNoteProgInfoEntry prog_info[NUM_PROG_INFO_ELEM];

	std::string line;
	std::string data_type;
	std::string scope;
	std::string access_type;
	std::string kernel_str;
	char *ptr;
	char *note_ptr[NUM_NOTES];
	char byte;

	unsigned int i;
	unsigned int j;
	unsigned int k;
	unsigned int kernel_size;
	unsigned int pos;
	unsigned index;
	unsigned int offset;
	int uav[MAX_UAV_NUM];
	int cb[MAX_CB_NUM];
	int rodata_size;
	int buff_num_offset;
	int buff_size;
	int glob_size;
	int imm_cb_found;
	int ptr_cb_table_found;
	int rat_op;


	/* Set machine type */
	setDevice(static_cast<OuterBinDevice> 
			(OuterBinDeviceMap.MapStringCase(MachineName)));

	if (getDevice() == OuterBinInvalid)
	{
		misc::fatal("Invalid machine type");
	}

	
	/* Create .symtab section and .strtab section */
	symbol_table = writer.newSymbolTable(".symtab", ".strtab");

	/* Create .rodata section */
	rodata_buffer = writer.newBuffer();
	rodata_section = writer.newSection(".rodata", rodata_buffer, rodata_buffer);
	rodata_section->setType(SHT_PROGBITS);
	
	/* Create Text Section */
	text_buffer = writer.newBuffer();
	text_section = writer.newSection(".text", text_buffer, text_buffer);
	text_section->setType(SHT_PROGBITS);
	text_section->setFlags(SHF_EXECINSTR | SHF_ALLOC);
	rodata_section->setFlags(SHF_ALLOC);

	rodata_size = 0;
	byte = 0;
	
	/* Write data list to rodata buffer */
	if (data_list.size())
	{
		for (auto &data : data_list)
			data->Write(rodata_buffer);

		/* Global section has to have a size that is a multiple of 16 */
		while ((rodata_buffer->getSize() % 16) != 0)
		{
			rodata_buffer->Write(&byte, sizeof(unsigned char));
		}


		/* Get number of elements to be used later in cb2 */
		glob_size = rodata_buffer->getSize() / 16;

		/* Add global symbol correspoding to data elements */
		line = "__OpenCL_" + std::to_string(2) + "_global";

		global_symbol = symbol_table->newSymbol(line);
		global_symbol->setShndx(4);
		global_symbol->setSize(rodata_buffer->getSize() - rodata_size);
		global_symbol->setValue(rodata_size);
		global_symbol->setInfo(ELF32_ST_TYPE(STT_OBJECT));

		rodata_size = rodata_buffer->getSize();

	}

	for(i = 0; i < inner_bin_list.size(); i++)
	{
		/* Intialize values at each iteration */
		offset = 0;
		ptr_cb_table_found = 0;
		imm_cb_found = 0;
		rat_op = 0;

		for (j = 0; j < MAX_UAV_NUM; j++)
			uav[j] = 0;

		for (j = 0; j < MAX_CB_NUM; j++)
			cb[j] = 0;


		/* Initial Inner ELF settings */

		inner_bin = getInnerBin(i);
		metadata = getMetadata(i);
		entry = inner_bin->getEntry(0);
		inner_symbol_table = entry->getSymbolTable();

		inner_bin->writer.setMachine(0x7d);
		inner_bin->writer.setVersion(1);
		inner_bin->writer.setIdent(EI_OSABI, 0x64);
		inner_bin->writer.setIdent(EI_ABIVERSION, 1);

		entry->setType(4);	/* ???? */

		/* d_machine values based on model */
		switch (getDevice())
		{
			case OuterBinCapeVerde:
    				entry->setMachine(28);
				break;

			case OuterBinPitcairn:
				entry->setMachine(27);
				break;

			case OuterBinTahiti:
				entry->setMachine(26);
				break;

			default:
				misc::fatal("%s: unrecognized device type", __FUNCTION__);
		}

		/* Metadata -> .rodata section of Outer ELF */

		/* Print kernel name */
		line = ";ARGSTART:__OpenCL_" + inner_bin->getName() + "_kernel\n";
		rodata_buffer->Write(line.c_str(), line.size());

		/* Version */
		line = ";version:3:1:104\n";
		rodata_buffer->Write(line.c_str(), line.size());

		/* Device */
		std::string device_str(OuterBinDeviceMap.MapValue(getDevice()));
		line = ";device:" + 
				device_str
				+ "\n";
		rodata_buffer->Write(line.c_str(), line.size());

		/* Unique ID */
		line = ";uniqueid:" + std::to_string(metadata->getUniqueId()) + "\n";
		rodata_buffer->Write(line.c_str(), line.size());

		/* Memory - uavprivate */
		line = ";memory:uavprivate:" + std::to_string(metadata->getUAVPrivate())
				+ "\n";
		rodata_buffer->Write(line.c_str(), line.size());

		/* Memory - hwregion */
		line = ";memory:hwregion:" + std::to_string(metadata->getHWRegion()) 
				+ "\n";
		rodata_buffer->Write(line.c_str(), line.size());

		/* Memory - hwlocal */
		line = ";memory:hwlocal:" + std::to_string(metadata->getHWLocal()) + "\n";
		rodata_buffer->Write(line.c_str(), line.size());

		index = 0;
		for (auto &arg : metadata->getArgList())
		{
			arg->WriteInfo(rodata_buffer, index, offset, uav);
			index++;
		}
		
		/* Data Required */
		if (getDataCount() > 0)
		{
			line = ";memory:datareqd\n";
			rodata_buffer->Write(line.c_str(), line.size());
		}

		/* Function ID */
		line = ";function:1:" + std::to_string(metadata->getUniqueId() + 3) + "\n";
		rodata_buffer->Write(line.c_str(), line.size());

		/* Private ID */
		line = ";privateid:" + std::to_string(8) + "\n";
		rodata_buffer->Write(line.c_str(), line.size());

		/* Reflections */
		index = 0;
		for (auto &arg : metadata->getArgList())
		{
			arg->WriteReflection(rodata_buffer, index);
			index++;
		}

		/* ARGEND */
		line = ";ARGEND:__OpenCL_" + inner_bin->getName() + "_kernel\n";
		rodata_buffer->Write(line.c_str(), line.size());

		/* Create metadata symbol and store it */
		line = "__OpenCL_" + inner_bin->getName() + "_metadata", 

		metadata_symbol = symbol_table->newSymbol(line);
		metadata_symbol->setShndx(4);
		metadata_symbol->setSize(rodata_buffer->getSize() - rodata_size);
		metadata_symbol->setValue(rodata_size);
		metadata_symbol->setInfo(ELF32_ST_TYPE(STT_OBJECT));

		/* Increment rodata size */
		rodata_size = rodata_buffer->getSize();



		/* Create header symbol and store it */
		line = "__OpenCL_" + inner_bin->getName() + "_header", 

		header_symbol = symbol_table->newSymbol(line);
		header_symbol->setShndx(4);
		header_symbol->setSize(32);
		header_symbol->setValue(rodata_size);
		header_symbol->setInfo(ELF32_ST_TYPE(STT_OBJECT));

		/* Create header - Header is not always set the way it is here but
		 * changing it does not seem to affect the program
		 */

		ptr = new char[32]();
		ptr[20] = 1;
		rodata_buffer->Write(ptr, 32);
		delete[] ptr;

		rodata_size = rodata_buffer->getSize();


		/* Kernel -> .text section */
	        
		/* Create Notes */

		/* ELF_NOTE_ATI_INPUTS */
		note_ptr[0] = NULL;
		entry->newNote(InnerBinNoteTypeInputs, 0, note_ptr[0]);

		/* ELF_NOTE_ATI_OUTPUTS */
		note_ptr[1] = NULL;
		entry->newNote(InnerBinNoteTypeOutputs, 0, note_ptr[1]);


		buff_num_offset = 0;

		for (k = 0; k < MAX_UAV_NUM; k++)
		{
			if (uav[k])
			{
				rat_op |= (1 << k);
				buff_num_offset++;
			}
		}


		/* ELF_NOTE_ATI_UAV */
		if (buff_num_offset)
		{

			buff_size = 16 * buff_num_offset;
			note_ptr[2] = new char[buff_size]();

			buff_num_offset = 0;

			/* UAV Symbols */
			for (k = 0; k < MAX_UAV_NUM; k++)
			{
				if (!uav[k])
					continue;

				line = "uav" + std::to_string(k);
				uav_symbol = inner_symbol_table->newSymbol(line);
				uav_symbol->setValue(buff_num_offset);
				uav_symbol->setShndx(16);

				note_ptr[2][buff_num_offset * 16] = k;
				note_ptr[2][buff_num_offset * 16 + 4] = 4;
				note_ptr[2][buff_num_offset * 16 + 12] = 5;

				buff_num_offset++;
			}

			entry->newNote(InnerBinNoteTypeUAV, buff_size, note_ptr[2]);

		}

		/* ELF_NOTE_ATI_CONDOUT */
		note_ptr[3] = new char[4]();
		entry->newNote(InnerBinNoteTypeCondOut, 4, note_ptr[3]);


		/* ELF_NOTE_ATI_FLOAT32CONSTS */
		note_ptr[4] = NULL;
		entry->newNote(InnerBinNoteTypeFloat32Consts, 0, note_ptr[4]);

		/* ELF_NOTE_ATI_INT32CONSTS */
		note_ptr[5] = NULL;
		entry->newNote(InnerBinNoteTypeInt32Consts, 0, note_ptr[5]);

		/* ELF_NOTE_ATI_BOOL32CONSTS */
		note_ptr[6] = NULL;
		entry->newNote(InnerBinNoteTypeBool32Consts, 0, note_ptr[6]);

		/* ELF_NOTE_ATI_EARLYEXIT */
		note_ptr[7] = new char[4]();
		entry->newNote(InnerBinNoteTypeEarlyExit, 4, note_ptr[7]);


		/* ELF_NOTE_ATI_GLOBAL_BUFFERS */
		note_ptr[8] = NULL;
		entry->newNote(InnerBinNoteTypeGlobalBuffers, 0, note_ptr[8]);

	
		/* ELF_NOTE_ATI_CONSTANT_BUFFERS */
		
		/* Check for non-contiguous entries in user element list*/
		for (k = 0; k < inner_bin->getUserElementCount(); k++)
		{
			if(!(inner_bin->getUserElement(k)))
				misc::fatal("userElement[%d] missing", k);
		}
		

		buff_num_offset = 0;

		for (k = 0; k < inner_bin->getUserElementCount(); k++)
		{
			user_elem = inner_bin->getUserElement(k);
			
			if (user_elem->dataClass == SI::BinaryUserDataConstBuffer)
			{
				/* Cannot have both IMM_CONST_BUFFER and PTR_CONST_BUFFER_TABLE */
				if (ptr_cb_table_found)
					misc::fatal("Cannot have both IMM_CONST_BUFFER and PTR_CONST_BUFFER_TABLE");	
				
				cb[user_elem->apiSlot] = 1;

				buff_num_offset++;
				imm_cb_found = 1;
			}
			else if (user_elem->dataClass == SI::BinaryUserDataConstBufferTable)
			{
				/* Cannot have both IMM_CONST_BUFFER and PTR_CONST_BUFFER_TABLE */
				if (imm_cb_found)
					misc::fatal("Cannot have both IMM_CONST_BUFFER and PTR_CONST_BUFFER_TABLE");	
			
				/* If PTR_CONST_BUFFER_TABLE is found, manually set cb's */
				cb[0] = 1;
				
				if (metadata->getArgCount() > 0)
					cb[1] = 1;
				
				if (getDataCount() > 0)
					cb[2] = 1;

				buff_num_offset = cb[0] + cb[1] + cb[2];
				ptr_cb_table_found = 1;
			}
		}

		/* Check if data values have been added without using constant buffer 2 */
		if ( (!cb[2]) && (getDataCount() > 0) )
			misc::fatal("Data values have been added but constant buffer 2 has not been specified");
		
		
		buff_size = 8 * buff_num_offset;
		note_ptr[9] = new char[buff_size]();
		
		/* CB Symbols */
		for (k = 0; k < MAX_CB_NUM; k++)
		{
			if (cb[k])
			{
				line = "cb" + std::to_string(k);

				cb_symbol = inner_symbol_table->newSymbol(line);
				cb_symbol->setValue(buff_num_offset - 1);
				cb_symbol->setShndx(10);
			
				note_ptr[9][(buff_num_offset - 1) * 8] = k;
			
				if (k == 0)
					note_ptr[9][(buff_num_offset - 1) * 8 + 4] = 0xf;

				if (k == 1)
					note_ptr[9][(buff_num_offset - 1) * 8 + 4] = 
						metadata->getArgCount();
				
				if (k == 2)
				{
					if (getDataCount() == 0)
						misc::fatal("%s: Constant Buffer 2 is used but nothing has been added to a global symbol",
							 __FUNCTION__);

					note_ptr[9][(buff_num_offset - 1) * 8 + 4] = glob_size;
				}

				buff_num_offset--;
			}

		}
		
		entry->newNote(InnerBinNoteTypeConstantBuffers, buff_size, note_ptr[9]);


		/* ELF_NOTE_ATI_INPUT_SAMPLERS */
		note_ptr[10] = NULL;
		entry->newNote(InnerBinNoteTypeInputSamplers, 0, note_ptr[10]);

		/* ELF_NOTE_ATI_SCRATCH_BUFFERS */
		note_ptr[11] = new char[4]();
		entry->newNote(InnerBinNoteTypeScratchBuffers, 4, note_ptr[11]);


		/* ELF_NOTE_ATI_PERSISTENT_BUFFERS */
		note_ptr[12] = NULL;
		entry->newNote(InnerBinNoteTypePersistentBuffers, 0, note_ptr[12]);

		/* ELF_NOTE_ATI_PROGINFO */
		
		SI::BinaryNoteProgInfoEntry *prog_info = 
			new SI::BinaryNoteProgInfoEntry[NUM_PROG_INFO_ELEM]();
		
		/* AMU_ABI_USER_ELEMENT_COUNT */
		prog_info[0].address = 0x80001000;
		prog_info[0].value = inner_bin->getUserElementCount();
		
		/* AMU_ABI_USER_ELEMENTS (16 maximum) */
		for (k = 0; k < 16; k++)
		{
			prog_info[(k * 4) + 1].address = 0x80001000 + (k * 4) + 1;
			prog_info[(k * 4) + 2].address = 0x80001000 + (k * 4) + 2;
			prog_info[(k * 4) + 3].address = 0x80001000 + (k * 4) + 3;
			prog_info[(k * 4) + 4].address = 0x80001000 + (k * 4) + 4;
			
			if ((inner_bin->getUserElementCount() - 1) >= k)
			{
				user_elem = inner_bin->getUserElement(k);

				prog_info[(k * 4) + 1].value = user_elem->dataClass;
				prog_info[(k * 4) + 2].value = user_elem->apiSlot;
				prog_info[(k * 4) + 3].value = user_elem->startUserReg;
				prog_info[(k * 4) + 4].value = user_elem->userRegCount;
		
			}
			else
			{
				prog_info[(k * 4) + 1].value = 0;
				prog_info[(k * 4) + 2].value = 0;
				prog_info[(k * 4) + 3].value = 0;
				prog_info[(k * 4) + 4].value = 0;
			}
		}
				
		
	
		/* AMU_ABI_SI_NUM_VGPRS */
		prog_info[65].address = 0x80001041;
		prog_info[65].value = inner_bin->getNumVgpr();
		
		/* AMU_ABI_SI_NUM_SGPRS */
		prog_info[66].address = 0x80001042;	
		prog_info[66].value = inner_bin->getNumSgpr();
		
		/* AMU_ABI_SI_NUM_SGPRS_AVAIL */
		prog_info[67].address = 0x80001863;
		prog_info[67].value = 0x66;

		/* AMU_ABI_SI_NUM_VGPRS_ AVAIL */
		prog_info[68].address = 0x80001864;
		prog_info[68].value = 0x100;

		/* AMU_ABI_SI_FLOAT_MODE */
		prog_info[69].address = 0x80001043;
		prog_info[69].value = inner_bin->getFloatMode();

		/* AU_ABI_SI_IEEE_MODE */
		prog_info[70].address = 0x80001044;
		prog_info[70].value = inner_bin->getIeeeMode();

		prog_info[71].address = 0x80001045;
		prog_info[71].value = 0;
		
		/* COMPUTE_PGM_RSRC2 */
		prog_info[72].address = 0x00002e13;
		prog_info[72].value = *((int*)inner_bin->getPgmRsrc2());


		/* AMU_ABI_NUM_THREAD_PER_GROUP_X */
		prog_info[73].address = 0x8000001c;
		prog_info[73].value = 0x100;

		prog_info[74].address = 0x8000001d;
		prog_info[74].value = 0;
		prog_info[75].address = 0x8000001e;
		prog_info[75].value = 0;
		prog_info[76].address = 0x80001841;
		prog_info[76].value = 0;
		
		/* AMU_ABI_RAT_OP_IS_USED */
		prog_info[77].address = 0x8000001f;
		prog_info[77].value = rat_op;

		/* AMU_ABI_UAV_RESOURCE_MASK_0 */
		prog_info[78].address = 0x80001843;
		prog_info[78].value = rat_op;
		
		prog_info[79].address = 0x80001844;
		prog_info[79].value = 0;
		prog_info[80].address = 0x80001845;
		prog_info[80].value = 0;
		prog_info[81].address = 0x80001846;
		prog_info[81].value = 0;
		prog_info[82].address = 0x80001847;
		prog_info[82].value = 0;
		prog_info[83].address = 0x80001848;
		prog_info[83].value = 0;
		prog_info[84].address = 0x80001849;
		prog_info[84].value = 0;
		prog_info[85].address = 0x8000184a;
		prog_info[85].value = 0;
		prog_info[86].address = 0x8000184b;
		prog_info[86].value = 0;
		prog_info[87].address = 0x8000184c;
		prog_info[87].value = 0;
		prog_info[88].address = 0x8000184d;
		prog_info[88].value = 0;
		prog_info[89].address = 0x8000184e;
		prog_info[89].value = 0;
		prog_info[90].address = 0x8000184f;
		prog_info[90].value = 0;
		prog_info[91].address = 0x80001850;
		prog_info[91].value = 0;
		prog_info[92].address = 0x80001851;
		prog_info[92].value = 0;
		prog_info[93].address = 0x80001852;
		prog_info[93].value = 0;
		prog_info[94].address = 0x80001853;
		prog_info[94].value = 0;
		prog_info[95].address = 0x80001854;
		prog_info[95].value = 0;
		prog_info[96].address = 0x80001855;
		prog_info[96].value = 0;
		prog_info[97].address = 0x80001856;
		prog_info[97].value = 0;
		prog_info[98].address = 0x80001857;
		prog_info[98].value = 0;
		prog_info[99].address = 0x80001858;
		prog_info[99].value = 0;
		prog_info[100].address = 0x80001859;
		prog_info[100].value = 0;
		prog_info[101].address = 0x8000185a;
		prog_info[101].value = 0;
		prog_info[102].address = 0x8000185b;
		prog_info[102].value = 0;
		prog_info[103].address = 0x8000185c;
		prog_info[103].value = 0;
		prog_info[104].address = 0x8000185d;
		prog_info[104].value = 0;
		prog_info[105].address = 0x8000185e;
		prog_info[105].value = 0;
		prog_info[106].address = 0x8000185f;
		prog_info[106].value = 0;
		prog_info[107].address = 0x80001860;
		prog_info[107].value = 0;
		prog_info[108].address = 0x80001861;
		prog_info[108].value = 0;
		prog_info[109].address = 0x80001862;
		prog_info[109].value = 0;

		/* AMU_ABI_NUM_WAVEFRONT_PER_SIMD */
		prog_info[110].address = 0x8000000a;
		prog_info[110].value = 1;
		
		/* AMU_ABI_WAVEFRONT_SIZE - Set to 64*/
		prog_info[111].address = 0x80000078;
		prog_info[111].value = 0x40;

		/* AMU_ABI_LDS_SIZE_AVAIL */
		prog_info[112].address = 0x80000081;
		prog_info[112].value = 0x8000;

		/* AMU_ABI_LDS_SIZE_USED */
		prog_info[113].address = 0x80000082;
		prog_info[113].value = inner_bin->getPgmRsrc2()->lds_size * 256;

		entry->newNote(InnerBinNoteTypeProgInfo, 912, prog_info);

		/* ELF_NOTE_ATI_SUB_CONSTANT_BUFFERS */
		note_ptr[14] = NULL;
		entry->newNote(InnerBinNoteTypeSubConstantBuffers, 0, note_ptr[14]);

		/* ELF_NOTE_ATI_UAV_MAILBOX_SIZE */
		note_ptr[15] = new char[4]();
		entry->newNote(InnerBinNoteTypeUAVMailboxSize, 4, note_ptr[15]);

		/* ELF_NOTE_ATI_UAV_OP_MASK */
		note_ptr[16] = new char[128]();
		
		*((int*)note_ptr[16]) = rat_op;

		entry->newNote(InnerBinNoteTypeUAVOPMask, 128, note_ptr[16]);

		/* Data Section - Not supported yet (section is empty right now) */
		ptr = new char[4736]();
		entry->getDataSectionBuffer()->Write(ptr, 4736);


		/* Generate Inner Bin File */
		std::ostringstream oss;
	
		/* Output Inner ELF */
		//std::ofstream of("kernel.bin");
		//inner_bin->Generate(of);
		//of.close();
		

		inner_bin->Generate(oss);

		pos = oss.tellp();
		oss.seekp(0, std::ios_base::end);
		kernel_size = oss.tellp();
		oss.seekp(pos, std::ios_base::beg);

		kernel_str = oss.str();
			
			
		/* Create kernel symbol and add it to the symbol table */
		line = "__OpenCL_" + inner_bin->getName() + "_kernel";

		kernel_symbol = symbol_table->newSymbol(line);
		kernel_symbol->setShndx(5);
		kernel_symbol->setSize(kernel_size);
		kernel_symbol->setValue(text_buffer->getSize());
		kernel_symbol->setInfo(ELF32_ST_TYPE(STT_FUNC));

		text_buffer->Write(kernel_str.c_str(), kernel_size);


	}
	
	/* Set e_machine value on outer elf */
	switch (getDevice())
	{
		case OuterBinCapeVerde:
			writer.setMachine(0x3ff);
			break;

		case OuterBinPitcairn:
			writer.setMachine(0x3fe);
			break;

		case OuterBinTahiti:
			writer.setMachine(0x3fd);
			break;

		default:
			misc::fatal("%s: unrecognized device type", __FUNCTION__);
	}
        
	writer.setVersion(1);
	
	symbol_table->Generate();

	/* Generate final binary */
        writer.Generate(os);
}

} /* namespace si2bin */

