#ifndef M2C_FRM2BIN_KERNEL_H
#define M2C_FRM2BIN_KERNEL_H

#include <lib/class/class.h>
#include <stdio.h>
#include <lib/class/elf-writer.h>
#include <lib/class/string.h>

#include "Frm2binBinaryKernelInfo.h"

/*
 * This is the definition for Class Frm2binBinaryKernel
 * It's inherited from ELFWriter. This class info
 * specific for each kernel in .cubin file
 */

/* Class definition */
CLASS_BEGIN(Frm2binBinaryKernel, ELFWriter)

	/* name of the kernel */
	String *name;

	/* total size of the arguments */
	unsigned int argSize;

	/* info specific to this kernel, corresponds to
	 * .nv.info.<name> section */
	Frm2binBinaryKernelInfo *kInfo;

	/* meaning not clearly known yet, value
	 * equals = (0x20 + kernel_total_arg_size)
	 * corresponds to .nv.constant0.<name> */
	unsigned int constant0Size;

CLASS_END(Frm2binBinaryKernel)

void Frm2binBinaryKernelCreate(Frm2binBinaryKernel *self);
void Frm2binBinaryKernelDestroy(Frm2binBinaryKernel *self);

#endif
