/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include <stdarg.h>


#include <arch/fermi/asm/Wrapper.h>

#include <lib/class/class.h>
#include <lib/class/array.h>
#include <lib/class/list.h>
#include <lib/class/string.h>
#include <lib/class/elf-writer.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-encode.h>
#include <lib/util/list.h>

#include <elf.h>

#include "inst-info.h"
#include "outer-bin.h"
#include "frm2bin.h"
#include "Frm2binBinary.h"
#include "Frm2binBinaryGlobalInfo.h"
#include "Frm2binBinaryGlobalInfoItem.h"
#include "Frm2binBinaryKernel.h"
#include "Frm2binBinaryKernelInfo.h"
/*
*/

struct frm2bin_outer_bin_t *frm2bin_outer_bin;

struct frm2bin_inner_bin_t *frm2bin_inner_bin;

struct frm2bin_inner_bin_entry_t *frm2bin_entry;

struct elf_enc_buffer_t *bin_buffer;

int frm2bin_assemble;		/* Command-line option set */

char *frm2bin_file_name;	/* Current file */


/* global variables */

/* output text_section buffer */
struct elf_enc_buffer_t *text_section_buffer;
Frm2binBinary *cubinary;

void frm2bin_yyerror(const char *s)
{
	fprintf(stderr, "%s:%d: error: %s\n",
		frm2bin_file_name, frm2bin_yylineno, s);
	exit(1);
}


void frm2bin_yyerror_fmt(char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	fprintf(stderr, "%s:%d: error: ",
		frm2bin_file_name, frm2bin_yylineno);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	fflush(NULL);
	exit(1);
}



/* 
 * Public Functions
 */

struct FrmAsmWrap *frm_asm;


void Frm2binCreate(Frm2bin *self)
{
	/* Initialize */
	frm_asm = FrmAsmWrapCreate();
	frm2bin_inst_info_init();

	/* Temporarily register class we need */
	CLASS_REGISTER(Frm2binBinary);
	CLASS_REGISTER(Frm2binBinaryGlobalInfo);
	CLASS_REGISTER(Frm2binBinaryGlobalInfoItem);
	CLASS_REGISTER(Frm2binBinaryKernel);
	CLASS_REGISTER(Frm2binBinaryKernelInfo);

	/* task list is for label processing, lable is not supported now
	 * hence, it's commented */
	/* frm_task_list_init(); */
	/* frm_symbol_table_init(); */
	/* frm_stream_init(); */
	/* frm2bin_bin_init(); */

}


void Frm2binDestroy(Frm2bin *self)
{
	/* Finalize */
	/* task list is for label processing, lable is not supported now
	 * hence, it's commented */
	/* frm_task_list_done(); */
	/* frm_symbol_table_done(); */
	/* This should be changed for fermi */
	/* frm_stream_done(); */
	frm2bin_inst_info_done();
	FrmAsmWrapFree(frm_asm);
}


void Frm2binCompile(Frm2bin *self,
		struct list_t *source_file_list,
		struct list_t *bin_file_list)
{
	int index;

	Frm2binBinaryKernel *tmpKernel;
	Frm2binBinaryKernelInfo *tmpKernelInfo;
	Frm2binBinaryKernelInfoKparamInfo tmpKparamInfo[4];
	Frm2binBinaryKernelInfoKparamInfoCplx tmpCplx[4];
	Frm2binBinaryKernelInfoSyncStack tmpSyncStack;

	cubinary = new(Frm2binBinary);
	LIST_FOR_EACH(source_file_list, index)
	{
		/* Open file */
		frm2bin_file_name = list_get(source_file_list, index);
		frm2bin_yyin = fopen(frm2bin_file_name, "r");
		if (!frm2bin_yyin)
			fatal("%s: cannot open input file",
				frm2bin_file_name);

		/* create text_section buffer, we only produce text_section
		 * now */
		text_section_buffer = elf_enc_buffer_create();

		frm2bin_outer_bin = frm2bin_outer_bin_create();
		bin_buffer = elf_enc_buffer_create();

		/* Parse input */
		frm2bin_yyparse();
		//printf("argSize: %d Bytes\n",
		//((asFrm2binBinaryKernel)(ListHead(cubinary->kernel_list)))->argSize );

		/* do some processing after yyparse() */
		/* we only test vectorAdd.cubin example */
		/* set param_size in Frm2binBinaryKernelInfo */
		tmpKernel = (asFrm2binBinaryKernel)(ListHead(cubinary->kernel_list));
		tmpKernelInfo = tmpKernel->kInfo;
		tmpKernelInfo->param_size = ((tmpKernel->argSize) << 16) | 0x00001903;

		tmpKernelInfo->paramCbank.id = 0x00080a04;
		/* fixme hardcode this guy, please */
		tmpKernelInfo->paramCbank.SymIdx = 0x2;
		tmpKernelInfo->paramCbank.paramsizeX =
				((tmpKernel->argSize) << 16) | 0x0020;

		/* prepare 1st Frm2binBinaryKernelInfoKparamInfo for vectorAdd example */
		tmpKparamInfo[0].id = 0x000c1704;
		tmpKparamInfo[0].index = 0x0; // asfermi always set it to zero
		tmpKparamInfo[0].offset_ordinal = (0xc << 16) | 0x3;

		/* Please hardcode these guys! */
		tmpCplx[0].logAlign = 0x0;
		tmpCplx[0].space = 0x0;
		tmpCplx[0].cbank = 0x1f;
		tmpCplx[0].size = 0x1;

		tmpKparamInfo[0].cplx = tmpCplx[0];

		list_add(tmpKernelInfo->kparam_info_list, &tmpKparamInfo[0]);


		/* prepare 2nd Frm2binBinaryKernelInfoKparamInfo for vectorAdd example */
		tmpKparamInfo[1].id = 0x000c1704;
		tmpKparamInfo[1].index = 0x0; // asfermi always set it to zero
		tmpKparamInfo[1].offset_ordinal = (0x8 << 16) | 0x2;

		/* Please hardcode these guys! */
		tmpCplx[1].logAlign = 0x0;
		tmpCplx[1].space = 0x0;
		tmpCplx[1].cbank = 0x1f;
		tmpCplx[1].size = 0x1;

		tmpKparamInfo[1].cplx = tmpCplx[1];

		list_add(tmpKernelInfo->kparam_info_list, &tmpKparamInfo[1]);

		/* prepare 3rd Frm2binBinaryKernelInfoKparamInfo for vectorAdd example */
		tmpKparamInfo[2].id = 0x000c1704;
		tmpKparamInfo[2].index = 0x0; // asfermi always set it to zero
		tmpKparamInfo[2].offset_ordinal = (0x4 << 16) | 0x1;

		/* Please hardcode these guys! */
		tmpCplx[2].logAlign = 0x0;
		tmpCplx[2].space = 0x0;
		tmpCplx[2].cbank = 0x1f;
		tmpCplx[2].size = 0x1;

		tmpKparamInfo[2].cplx = tmpCplx[2];

		list_add(tmpKernelInfo->kparam_info_list, &tmpKparamInfo[2]);

		/* prepare 4th Frm2binBinaryKernelInfoKparamInfo for vectorAdd example */
		tmpKparamInfo[3].id = 0x000c1704;
		tmpKparamInfo[3].index = 0x0; // asfermi always set it to zero
		tmpKparamInfo[3].offset_ordinal = (0x0 << 16) | 0x0;

		/* Please hardcode these guys! */
		tmpCplx[3].logAlign = 0x0;
		tmpCplx[3].space = 0x0;
		tmpCplx[3].cbank = 0x1f;
		tmpCplx[3].size = 0x1;

		tmpKparamInfo[3].cplx = tmpCplx[3];

		list_add(tmpKernelInfo->kparam_info_list, &tmpKparamInfo[3]);


		tmpSyncStack.id = 0x80d04;
		tmpSyncStack.part1 = 0x1000c; // please hardcode this guy!
		tmpSyncStack.part2 = 0x1;
		tmpKernelInfo->sync_stack = tmpSyncStack;

		tmpKernel->constant0Size = 0x20 + tmpKernel->argSize;

		/* Call this func to populate sections, segments and buffers */
		Frm2binBinaryGenerate(self, cubinary);

		/* call the Fermi disassbmler for text_section it's just a
		 * temporarily implementation */
		{
			struct FrmAsmWrap *as = FrmAsmWrapCreate();
//			FrmAsmWrapDisassembleBuffer(as, text_section_buffer->ptr,
//					text_section_buffer->size);
			FrmAsmWrapFree(as);
		}

		/* free the text_section buffer */
		elf_enc_buffer_free(text_section_buffer);


		/* Process pending tasks */

		/* task list is for label processing, lable is not supported
		 * now hence, it's commented */
		/* frm_task_list_process(); */

		/* Close */
		fclose(frm2bin_yyin);

		/* Dump output buffer and free it */
		frm2bin_outer_bin_generate(frm2bin_outer_bin, bin_buffer);

		/* Free Outer ELF and bin_buffer */
		frm2bin_outer_bin_free(frm2bin_outer_bin);
		elf_enc_buffer_free(bin_buffer);
	}

	//fixme here
	delete(cubinary);
	//ELFWriterDestroy(asELFWriter(cubinary));
}

void Frm2binBinaryGenerate(Frm2bin *self, Frm2binBinary *cubinary)
{
	/* create sections and populate them with data from sub-class */
	ELFWriterBuffer *tmpBuffer;
	ELFWriterSection *tmpSection;
	ELFWriterSegment *tmpSegment;
	ELFWriterSymbolTable *tmpSymbolTable;

	/* used later by segment to section mapping */
	ELFWriterSection *tmpConstantSection, *tmpTextSection;
	ELFWriterBuffer *firstBuf, *lastBuf;// *tmpTextBuf;
	ELFWriterSymbol *tmpSymbol;

	List *GlobalInfoList;
	Frm2binBinaryGlobalInfoItem *tmpGlobalInfoItem;

	Frm2binBinaryKernel *tmpKernel;
	Frm2binBinaryKernelInfo *tmpKernelInfo;
	Frm2binBinaryKernelInfoKparamInfo *tmpKparamInfo;
	//Frm2binBinaryKernelInfoKparamInfoCplx *tmpCplx;
	Frm2binBinaryKernelInfoSyncStack *tmpSyncStack;

	String *tmpString;
	String *tmpConstantString;
	//String *tmpStrNvInfo = new(String, ".nv.info");

	int *tmpConstantBuf;
	int iter;

	/* configure the ELF Header info specific for .cubin */
	asELFWriter(cubinary)->header.e_type = 0x2;
	asELFWriter(cubinary)->header.e_machine = 0xbe;
	asELFWriter(cubinary)->header.e_version = 0x1;
	asELFWriter(cubinary)->header.e_entry = 0x0;
	asELFWriter(cubinary)->header.e_ident[7] = 0x33;
	asELFWriter(cubinary)->header.e_ident[8] = 0x6;
	asELFWriter(cubinary)->header.e_flags = 0x140114;

	/*
	 * * create the symtable which includes a string table
	 */
	tmpSymbolTable = new(ELFWriterSymbolTable, ".symtab", ".strtab");

	/* add symbol table to the elf binary file */
	ELFWriterAddSymbolTable(asELFWriter(cubinary), tmpSymbolTable);


	/* only consider one kernel case */
	GlobalInfoList = cubinary->global_info->info_list;
	tmpGlobalInfoItem = (asFrm2binBinaryGlobalInfoItem)(ListHead(GlobalInfoList));

	/* hardcode globalInfoItem */
	tmpGlobalInfoItem->GlobSymIdx_A = 0x3;
	tmpGlobalInfoItem->GlobSymIdx_B = 0x3;

	/*
	 * * .nv.info section
	 */

	/* create a buffer */
	tmpBuffer = new(ELFWriterBuffer);

	/* create a section */
	tmpSection = new(ELFWriterSection, ".nv.info", tmpBuffer, tmpBuffer);

	/* set section header properties */
	tmpSection->header.sh_type = 0x70000000;	//SHT_LOPROC
	tmpSection->header.sh_flags = 0x0;
	tmpSection->header.sh_link = 0x3;
	tmpSection->header.sh_info = 0x0;
	tmpSection->header.sh_addralign = 0x4;
	tmpSection->header.sh_entsize = 0x0;


	/* populate globalInfoItem data to this buffer */
	ELFWriterBufferWrite(tmpBuffer, &(tmpGlobalInfoItem->id_A), 4);
	ELFWriterBufferWrite(tmpBuffer, &(tmpGlobalInfoItem->GlobSymIdx_A), 4);
	ELFWriterBufferWrite(tmpBuffer, &(tmpGlobalInfoItem->MinStackSize), 4);
	ELFWriterBufferWrite(tmpBuffer, &(tmpGlobalInfoItem->id_B), 4);
	ELFWriterBufferWrite(tmpBuffer, &(tmpGlobalInfoItem->GlobSymIdx_B), 4);
	ELFWriterBufferWrite(tmpBuffer, &(tmpGlobalInfoItem->FrameSize), 4);

	/* add section to the binary */
	ELFWriterAddSection(asELFWriter(cubinary), tmpSection);

	/* add the buffer to the buffer_array of the binary */
	ELFWriterAddBuffer(asELFWriter(cubinary), tmpBuffer);

	/*
	 * * .nv.info.<name> section
	 */

	/* create a buffer */
	tmpBuffer = new(ELFWriterBuffer);

	tmpKernel = asFrm2binBinaryKernel(ListHead(cubinary->kernel_list));

	/* create name for the section, insert .nv.info before kernel_name */
	tmpString = new(String, tmpKernel->name->text);
	StringInsert(tmpString, 0, ".nv.info.");

	/* create a section */
	tmpSection = new(ELFWriterSection, tmpString->text, tmpBuffer, tmpBuffer);

	/* set section header properties */
	tmpSection->header.sh_type = 0x70000000;	//SHT_LOPROC
	tmpSection->header.sh_flags = 0x0;
	tmpSection->header.sh_link = 0x3;
	tmpSection->header.sh_info = 0x7;
	tmpSection->header.sh_addralign = 0x4;
	tmpSection->header.sh_entsize = 0x0;

	/* populate data to this section */
	tmpKernelInfo = tmpKernel->kInfo;
	ELFWriterBufferWrite(tmpBuffer, &(tmpKernelInfo->paramCbank.id), 4);
	ELFWriterBufferWrite(tmpBuffer, &(tmpKernelInfo->paramCbank.SymIdx), 4);
	ELFWriterBufferWrite(tmpBuffer, &(tmpKernelInfo->paramCbank.paramsizeX), 4);

	ELFWriterBufferWrite(tmpBuffer, &(tmpKernelInfo->param_size), 4);

	/* only consider 1 kernel case in vectorAdd */
	LIST_FOR_EACH(tmpKernelInfo->kparam_info_list, iter)
	{
		tmpKparamInfo = list_get(tmpKernelInfo->kparam_info_list, iter);
		ELFWriterBufferWrite(tmpBuffer, &(tmpKparamInfo->id), 4);
		ELFWriterBufferWrite(tmpBuffer, &(tmpKparamInfo->index), 4);
		ELFWriterBufferWrite(tmpBuffer, &(tmpKparamInfo->offset_ordinal), 4);
		ELFWriterBufferWrite(tmpBuffer, &(tmpKparamInfo->cplx), 4);
	}



	/* add sync_stack to it */
	tmpSyncStack = &(tmpKernelInfo->sync_stack);
	ELFWriterBufferWrite(tmpBuffer, tmpSyncStack, 12);

	/* add section to the binary */
	ELFWriterAddSection(asELFWriter(cubinary), tmpSection);

	/* add the buffer to the buffer_array of the binary */
	ELFWriterAddBuffer(asELFWriter(cubinary), tmpBuffer);

	/*
	 * * .nv.constant0.<name> section
	 */

	/* create a buffer */
	tmpBuffer = new(ELFWriterBuffer);

	tmpKernel = asFrm2binBinaryKernel(ListHead(cubinary->kernel_list));

	/* create name for the section, insert .nv.info before kernel_name */
	tmpString = new(String, tmpKernel->name->text);
	StringInsert(tmpString, 0, ".nv.constant0.");
	tmpConstantString = new(String, tmpString->text);

	/* create a section */
	tmpSection = new(ELFWriterSection, tmpString->text, tmpBuffer, tmpBuffer);

	/* set section header properties */
	tmpSection->header.sh_type = 0x1;	//SHT_PROGBITS
	tmpSection->header.sh_flags = 0x2;
	tmpSection->header.sh_link = 0x0;
	tmpSection->header.sh_info = 0x7;
	tmpSection->header.sh_addralign = 0x4;
	tmpSection->header.sh_entsize = 0x0;

	tmpConstantSection = tmpSection;

	/* populate data to this section */
	tmpConstantBuf = (int *)xmalloc(tmpKernel->constant0Size);
	memset(tmpConstantBuf, 0, tmpKernel->constant0Size);

	ELFWriterBufferWrite(tmpBuffer, tmpConstantBuf, tmpKernel->constant0Size);

	/* add section to the binary */
	ELFWriterAddSection(asELFWriter(cubinary), tmpSection);

	/* add the buffer to the buffer_array of the binary */
	//ArrayAdd((asELFWriter(cubinary))->buffer_array, asObject(tmpBuffer));
	ELFWriterAddBuffer(asELFWriter(cubinary), tmpBuffer);



	/*
	 * * .text.<name> section
	 */

	/* create a buffer */
	tmpBuffer = new(ELFWriterBuffer);

	tmpKernel = asFrm2binBinaryKernel(ListHead(cubinary->kernel_list));

	/* create name for the section, insert .nv.info before kernel_name */
	tmpString = new(String, tmpKernel->name->text);
	StringInsert(tmpString, 0, ".text.");

	/* create a section */
	tmpSection = new(ELFWriterSection, tmpString->text, tmpBuffer, tmpBuffer);
	tmpTextSection = tmpSection;

	/* set section header properties */
	tmpSection->header.sh_type = 0x1;	//SHT_PROGBITS
	tmpSection->header.sh_flags = 0x6;
	tmpSection->header.sh_link = 0x3;
	tmpSection->header.sh_info = 0x4000003;
	tmpSection->header.sh_addralign = 0x4;
	tmpSection->header.sh_entsize = 0x0;

	/* populate data to this section */
	ELFWriterBufferWrite(tmpBuffer, text_section_buffer->ptr, text_section_buffer->size);


	/* add section to the binary */
	ELFWriterAddSection(asELFWriter(cubinary), tmpSection);

	/* add the buffer to the buffer_array of the binary */
	ELFWriterAddBuffer(asELFWriter(cubinary), tmpBuffer);

	// debug, print out the hex of text_buffer
	int i;
	printf("####\n");
	for(i=0; i<28; i++)
		printf("%08x \n", *((unsigned int *)(text_section_buffer->ptr) + i));




	/* create one symbol for this section, it will be
	 * one entry in .symtab */
	tmpSymbol = new(ELFWriterSymbol, tmpString->text);

	/* set value of fields for this symbol */
	tmpSymbol->symbol.st_value = 0x0;
	tmpSymbol->symbol.st_size = 0x0;
	tmpSymbol->symbol.st_info = ELF32_ST_INFO(0x0, 0x3);
	tmpSymbol->symbol.st_other = 0x0;
	tmpSymbol->symbol.st_shndx = 0x7;

	/* add this symbol to the symbol table */
	ELFWriterSymbolTableAdd(tmpSymbolTable, tmpSymbol);


	/* create one symbol for .nv.constant0 section, it will be
	 * one entry in .symtab */
	tmpSymbol = new(ELFWriterSymbol, tmpConstantString->text);

	/* set value of fields for this symbol */
	tmpSymbol->symbol.st_value = 0x0;
	tmpSymbol->symbol.st_size = 0x0;
	tmpSymbol->symbol.st_info = ELF32_ST_INFO(0x0, 0x3);
	tmpSymbol->symbol.st_other = 0x0;
	tmpSymbol->symbol.st_shndx = 0x6;

	/* add this symbol to the symbol table */
	ELFWriterSymbolTableAdd(tmpSymbolTable, tmpSymbol);


	/* create one symbol for global function this section,
	 * it will be one entry in .symtab */
	tmpSymbol = new(ELFWriterSymbol, tmpKernel->name->text);

	/* set value of fields for this symbol */
	tmpSymbol->symbol.st_value = 0x0;
	tmpSymbol->symbol.st_size = 0x70;
	tmpSymbol->symbol.st_info = ELF32_ST_INFO(0x1, 0x2);
	tmpSymbol->symbol.st_other = 0x10; //harcode, not known exact meaning
	tmpSymbol->symbol.st_shndx = 0x7;

	/* add this symbol to the symbol table */
	ELFWriterSymbolTableAdd(tmpSymbolTable, tmpSymbol);




	/* generate program segments, there are 3 segments for vectorAdd.s example */

	/*
	 * * segments 00, no sections associated, segment name is empty
	 */
	/* assign empty buffer to this segment now,
	 * FIXME: may need to change later */
	tmpBuffer = new(ELFWriterBuffer);
	tmpSegment = new(ELFWriterSegment, "", tmpBuffer, tmpBuffer);
	/* add the buffer to the buffer_array of the binary */
	//ArrayAdd((asELFWriter(cubinary))->buffer_array, asObject(tmpBuffer));
	ELFWriterAddBuffer(asELFWriter(cubinary), tmpBuffer);


	/* set segment header */
	tmpSegment->header.p_type = 0x1;
	tmpSegment->header.p_vaddr = 0x0;
	tmpSegment->header.p_paddr = 0x0;
	tmpSegment->header.p_flags = PF_W | PF_R; //write + read ??
	tmpSegment->header.p_align = 0x4;

	/* add this segment to the binary */
	ELFWriterAddSegment(asELFWriter(cubinary), tmpSegment);


	/*
	 * * segments 01, associated section: .nv.constant0.<name>
	 * * and .text.<name>, segment name is empty
	 */
	firstBuf = tmpConstantSection->first_buffer;
	lastBuf = tmpTextSection->last_buffer;
	tmpSegment = new(ELFWriterSegment, "", firstBuf, lastBuf);

	/* set segment header */
	tmpSegment->header.p_type = 0x1;
	tmpSegment->header.p_vaddr = 0x0;
	tmpSegment->header.p_paddr = 0x0;
	tmpSegment->header.p_flags = PF_X | PF_R; //exec + read ??
	tmpSegment->header.p_align = 0x4;

	/* add this segment to the binary */
	ELFWriterAddSegment(asELFWriter(cubinary), tmpSegment);


	/*
	 * * segments 02, no sections associated, segment name is empty
	 * * size should be 0x60
	 */
	/* assign empty buffer to this segment now,
	 * FIXME: may need to change later */
	tmpBuffer = new(ELFWriterBuffer);
	tmpSegment = new(ELFWriterSegment, "", tmpBuffer, tmpBuffer);
	/* add the buffer to the buffer_array of the binary */
	//ArrayAdd((asELFWriter(cubinary))->buffer_array, asObject(tmpBuffer));
	ELFWriterAddBuffer(asELFWriter(cubinary), tmpBuffer);

	/* set segment header */
	tmpSegment->header.p_type = 0x6;
	tmpSegment->header.p_vaddr = 0x0;
	tmpSegment->header.p_paddr = 0x0;
	tmpSegment->header.p_flags = PF_X | PF_R;
	tmpSegment->header.p_align = 0x4;
	/* following should be populated automatically by elf_enc_generate ??*/
	tmpSegment->header.p_filesz = 0x60;
	tmpSegment->header.p_memsz = 0x60;


	/* add this segment to the binary */
	ELFWriterAddSegment(asELFWriter(cubinary), tmpSegment);


	/* generate the ELF file */
	tmpBuffer = new(ELFWriterBuffer);
	ELFWriterGenerate(asELFWriter(cubinary), tmpBuffer);

	/* dump the ELF file */
	FILE *bin_file;
	char *file_name = "myVectorAdd.cubin";
	bin_file = fopen(file_name, "wb");
	if (!bin_file)
		printf("%s: cannot output output file", file_name);


	ELFWriterBufferWriteToFile(tmpBuffer, bin_file);

}
