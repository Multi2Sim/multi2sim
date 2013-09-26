#ifndef M2C_FRM2BIN_GLOBALINFOITEM_H
#define M2C_FRM2BIN_GLOBALINFOITEM_H


#include <lib/class/class.h>
#include <stdio.h>

/*
 * This is the definition for Class Frm2binBinaryGlobalInfoItem
 * It's inherited from Object. This class include info for each
 * kernel in the Frm2bindBinaryGlobalInfo in a .cubin file
 */

/* Class definition */
CLASS_BEGIN(Frm2binBinaryGlobalInfoItem, Object)

	/* id_A, fixed value */
	unsigned int id_A;

	/* GlobaSymIdx_A, index of this section's symbol in
	 * the global symbol table */
	unsigned int GlobSymIdx_A;

	/* MinStackSize, not known, asfermi
	 * just set it to zero */
	unsigned int MinStackSize;

	/* id_B, fixed value */
	unsigned int id_B;

	/* GlobaSymIdx_B, index of this section's symbol in
	 * the global symbol table */
	unsigned int GlobSymIdx_B;

	/* FrameSize, not known, asfermi
	 * just set it to zero */
	unsigned int FrameSize;

CLASS_END(Frm2binBinaryGlobalInfoItem)

void Frm2binBinaryGlobalInfoItemCreate(Frm2binBinaryGlobalInfoItem *self);
void Frm2binBinaryGlobalInfoItemDestroy(Frm2binBinaryGlobalInfoItem *self);

#endif
