#ifndef M2C_FRM2BIN_BINARY_H
#define M2C_FRM2BIN_BINARY_H


#include <lib/class/class.h>
#include <lib/class/list.h>
#include <stdio.h>
#include <lib/class/elf-writer.h>


#include "Frm2binBinaryGlobalInfo.h"
#include "Frm2binBinaryKernel.h"

/*
 * This is the definition for Class Frm2binBinary
 * It's inherited from ELFWriter. This class can
 * generate .cubin file that can be run on real
 * GPU device.
 */

/* Class definition */
CLASS_BEGIN(Frm2binBinary, ELFWriter)

	/* number of kernels */
	unsigned int numKernel;

	/* list of kernels, each contains info specific to that kernel, element
	 * of the list is of type "Frm2binBinaryKernel".  */
	List *kernel_list;

	/* global info section for the binary */
	Frm2binBinaryGlobalInfo	*global_info;

CLASS_END(Frm2binBinary)



void Frm2binBinaryCreate(Frm2binBinary *self);
void Frm2binBinaryDestroy(Frm2binBinary *self);

#endif
