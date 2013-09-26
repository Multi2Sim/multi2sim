#ifndef M2C_FRM2BIN_GLOBALINFO_H
#define M2C_FRM2BIN_GLOBALINFO_H


#include <lib/class/class.h>
#include <stdio.h>
#include <lib/class/elf-writer.h>

/*
 * This is the definition for Class Frm2binBinaryGlobalInfo
 * It's inherited from ELFWriter. This class include global
 * info in a .cubin file
 */

/* Class definition */
CLASS_BEGIN(Frm2binBinaryGlobalInfo, Object)

	/* each kernel has one item inside this list
	 * type of the item is Frm2binBinaryGlobalInfoItem */
	List *info_list;

CLASS_END(Frm2binBinaryGlobalInfo)

void Frm2binBinaryGlobalInfoCreate(Frm2binBinaryGlobalInfo *self);
void Frm2binBinaryGlobalInfoDestroy(Frm2binBinaryGlobalInfo *self);

#endif
