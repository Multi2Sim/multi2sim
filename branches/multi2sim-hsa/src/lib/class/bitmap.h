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

#ifndef LIB_CLASS_BITMAP_H
#define LIB_CLASS_BITMAP_H

#include "class.h"


/*
 * Class 'Bitmap'
 */

CLASS_BEGIN(Bitmap, Object)

	int size;
	int size_in_words;
	unsigned int *data;

CLASS_END(Bitmap)


void BitmapCreate(Bitmap *self, int size);
void BitmapDestroy(Bitmap *self);

void BitmapDump(Object *self, FILE *f);
Object *BitmapClone(Object *self);

/* Set 'size' bits to 'value' starting at position 'where'. Argument 'size'
 * must be a value between 0 and 32. If 'size' is 0, the function has no
 * effect. */
void BitmapSet(Bitmap *self, unsigned int where, unsigned int size,
		unsigned int value);

/* Return the value of the bitmap at position 'where' taking 'size' bits.
 * The value of 'size' must be between 0 and 32. */
unsigned int BitmapGet(Bitmap *self, unsigned int where,
		unsigned int size);

/* Return the number of zeros/ones in a given range of the bitmap. */
int BitmapCountZeros(Bitmap *self, unsigned int where, unsigned int size);
int BitmapCountOnes(Bitmap *self, unsigned int where, unsigned int size);

/* Read 'size' bits from bitmap 'src' at position 'src_where' and copy them
 * into position 'dst_where' of bitmap 'self'. */
void BitmapMove(Bitmap *self, unsigned int dst_where, Bitmap *src,
		unsigned int src_where, unsigned int size);

/* Bitwise operations. Those operations taking one 'operand' as a source
 * argument rely on 'operand' having the same size as the 'self' bitmap. */
void BitmapNot(Bitmap *self);
void BitmapAnd(Bitmap *self, Bitmap *operand);
void BitmapOr(Bitmap *self, Bitmap *operand);
void BitmapXor(Bitmap *self, Bitmap *operand);
void BitmapSub(Bitmap *self, Bitmap *operand);

#endif
