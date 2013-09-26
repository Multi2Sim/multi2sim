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


#include <arch/fermi/asm/Asm.h>

#include <lib/class/class.h>
#include <lib/class/list.h>
#include <lib/class/string.h>
#include <lib/class/elf-writer.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-encode.h>
#include <lib/util/list.h>

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
	Frm2binBinaryKernelInfoKparamInfo tmpKparamInfo;
	Frm2binBinaryKernelInfoKparamInfoCplx tmpCplx;
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
		printf("argSize: %d Bytes\n",
		((asFrm2binBinaryKernel)(ListHead(cubinary->kernel_list)))->argSize );

		/* do some processing after yyparse() */
		/* we only test vectorAdd.cubin example */
		/* set param_size in Frm2binBinaryKernelInfo */
		tmpKernel = (asFrm2binBinaryKernel)(ListHead(cubinary->kernel_list));
		tmpKernelInfo = tmpKernel->kInfo;
		tmpKernelInfo->param_size = ((tmpKernel->argSize) << 16) | 0x00001903;

		tmpKernelInfo->paramCbank.id = 0x00080a04;
		/* fixme hardcode this guy, please */
		tmpKernelInfo->paramCbank.SymIdx = 0;
		tmpKernelInfo->paramCbank.paramsizeX =
				((tmpKernel->argSize) << 16) | 0x0020;

		/* prepare Frm2binBinaryKernelInfoKparamInfo for vectorAdd example */
		tmpKparamInfo.id = 0x000c1704;
		tmpKparamInfo.index = 0; // asfermi always set it to zero
		tmpKparamInfo.offset = 0; // please hardcode it to right value
		tmpKparamInfo.ordinal = 0; // please hardcode it to right value

		/* Please hardcode these guys! */
		tmpCplx.logAlign = 0;
		tmpCplx.space = 0;
		tmpCplx.cbank = 0;
		tmpCplx.size = 0;

		tmpKparamInfo.cplx = tmpCplx;

		list_add(tmpKernelInfo->kparam_info_list, &tmpKparamInfo);

		tmpSyncStack.notKnownYet = 0; // please hardcode this guy!
		tmpKernelInfo->sync_stack = tmpSyncStack;

		tmpKernel->constant0Size = 0x20 + tmpKernel->argSize;

		/* Call this func to populate sections, segments and buffers */
		//Frm2binBinaryGenerate(self, cubinary);

		/* call the Fermi disassbmler for text_section it's just a
		 * temporarily implementation */
		{
			struct FrmAsmWrap *as = FrmAsmWrapCreate();
			FrmAsmWrapDisassembleBuffer(as, text_section_buffer->ptr,
					text_section_buffer->size);
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

	List *GlobalInfoList;
	Frm2binBinaryGlobalInfoItem *tmpGlobalInfoItem;

	//String *tmpString;

	/* only consider one kernel case */
	GlobalInfoList = cubinary->kernel_list;
	tmpGlobalInfoItem = (asFrm2binBinaryGlobalInfoItem)(ListHead(GlobalInfoList));

	/*
	 * * .nv.info section
	 */

	/* create a buffer */
	tmpBuffer = new(ELFWriterBuffer);

	/* create a section */
	tmpSection = new(ELFWriterSection, ".nv.info", tmpBuffer, tmpBuffer);

	/* populate globalInfoItem data to this buffer */
	ELFWriterBufferWrite(tmpBuffer, &(tmpGlobalInfoItem->id_A), 4);
	ELFWriterBufferWrite(tmpBuffer, &(tmpGlobalInfoItem->GlobSymIdx_A), 4);
	ELFWriterBufferWrite(tmpBuffer, &(tmpGlobalInfoItem->MinStackSize), 4);
	ELFWriterBufferWrite(tmpBuffer, &(tmpGlobalInfoItem->id_B), 4);
	ELFWriterBufferWrite(tmpBuffer, &(tmpGlobalInfoItem->GlobSymIdx_B), 4);
	ELFWriterBufferWrite(tmpBuffer, &(tmpGlobalInfoItem->FrameSize), 4);

	/* add section to the binary */
	ELFWriterAddSection(asELFWriter(cubinary), tmpSection);

	/*
	 * * .nv.info.<name> section
	 */

	/* create a buffer */
	tmpBuffer = new(ELFWriterBuffer);

	/* create a section */
	tmpSection = new(ELFWriterSection, ".nv.info", tmpBuffer, tmpBuffer);


}
