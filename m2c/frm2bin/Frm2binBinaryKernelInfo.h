#ifndef M2C_FRM2BIN_KERNELINFO_H
#define M2C_FRM2BIN_KERNELINFO_H

#include <lib/class/class.h>
#include <stdio.h>
#include <lib/class/elf-writer.h>
#include <lib/class/string.h>

#include <lib/util/list.h>


/* some structure definition that will be
 * used by Frm2binBinaryKernelInfo */
typedef struct
{
	/* ID, fixed value, 0x00080a04 */
	unsigned int id;

	/* Symbol idx, it specifies symbol index in
	 * the .nv.constant0.<name> section */
	unsigned int SymIdx;

	/* it specifies a value that's transformed from the size
	 * of parameters: (paramTotalSize << 16 | 0x0020) */
	unsigned int paramsizeX;

} Frm2binBinaryKernelInfoParamCBank;

typedef struct
{
	/* meaning of these fields not known yet */
	unsigned int logAlign : 8;	// 0:7
	unsigned int space : 4;		// 8:11
	unsigned int cbank : 8;		//12:19
	unsigned int size : 12;		//20:31

} Frm2binBinaryKernelInfoKparamInfoCplx;

typedef struct
{
	/* ID, fixed value, 0x000c1704*/
	unsigned int id;

	/* index, not known */
	unsigned int index;

	/* mix of ordinal and offset */
	unsigned int offset_ordinal;

	/* ordinal, not known */
	//unsigned short ordinal;

	/* offset, not known */
	//unsigned short offset;

	/* a complex info structure */
	Frm2binBinaryKernelInfoKparamInfoCplx cplx;

}Frm2binBinaryKernelInfoKparamInfo;

typedef struct
{
	unsigned int id;
	/* not known yet */
	unsigned int part1;
	unsigned int part2;

} Frm2binBinaryKernelInfoSyncStack;

/*
 * This is the definition for Class Frm2binBinaryKernelInfo
 * It's inherited from ELFWriter. This class contians info
 * specific for each kernel in .cubin file. It corresponds
 * to .nv.info.<name> section
 */

/* Class definition */
CLASS_BEGIN(Frm2binBinaryKernelInfo, ELFWriter)

	/* parameter in constant memory bank ?? Not known yet */
	Frm2binBinaryKernelInfoParamCBank paramCbank;

	/* transform of paramSize, it's actually a value transformed
	 * from parameter size: (0x00001903 | ParamTotalSize << 16)*/
	unsigned int param_size;

	/* list of kernel parameter info, one for each kernel
	 * each of type Frm2binBinaryKernelInfoKparamInfo  */
	struct list_t *kparam_info_list;

	/* Sync stack ?? Not known yet */
	Frm2binBinaryKernelInfoSyncStack sync_stack;

CLASS_END(Frm2binBinaryKernelInfo)

void Frm2binBinaryKernelInfoCreate(Frm2binBinaryKernelInfo *self);
void Frm2binBinaryKernelInfoDestroy(Frm2binBinaryKernelInfo *self);

#endif
