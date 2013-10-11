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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/misc.h>

#include "bitmap.h"


/*
 * Class 'Bitmap'
 */

void BitmapCreate(Bitmap *self, int size)
{
	/* Check */
	if (size < 1)
		panic("%s: invalid value for size (%d)",
				__FUNCTION__, size);

	/* Initialize */
	self->size = size;
	self->size_in_words = (size + 31) / 32;
	self->data = xcalloc(self->size_in_words, 4);

	/* Virtual functions */
	asObject(self)->Dump = BitmapDump;
	asObject(self)->Clone = BitmapClone;
}


void BitmapDestroy(Bitmap *self)
{
	free(self->data);
}


void BitmapDump(Object *self, FILE *f)
{
	Bitmap *bitmap = asBitmap(self);
	int i;

	for (i = 0; i < bitmap->size; i++)
		fprintf(f, "%d", BitmapGet(bitmap, i, 1));
	fprintf(f, "\n");
}


Object *BitmapClone(Object *self)
{
	Bitmap *src = asBitmap(self);
	Bitmap *dst;

	int i;

	/* Create new bitmap */
	dst = new(Bitmap, src->size);
	for (i = 0; i < src->size_in_words; i++)
		dst->data[i] = src->data[i];

	/* Return */
	return asObject(dst);
}


void BitmapSet(Bitmap *self, unsigned int where, unsigned int size,
		unsigned int value)
{
	unsigned int where_word1, where_offset1, size_align1;
	unsigned int where_word2, size_align2;
	unsigned int *pword1, word1_mask, word1_value;
	unsigned int *pword2, word2_mask, word2_value;

	/* Check */
	if (size > 32 || size < 0)
		panic("%s: invalid value for 'size' (%d)",
				__FUNCTION__, size);
	if (!size)
		return;

	/* Bits to get from first word */
	where_word1 = where / 32;
	where_offset1 = where % 32;
	size_align1 = MIN(size, 32 - where_offset1);
	if (where_word1 >= self->size_in_words)
		return;
	pword1 = &self->data[where_word1];

	/* Special case: 32-bit aligned word */
	if (size == 32 && !where_offset1)
	{
		*pword1 = value;
		return;
	}

	/* Get shift and mask */
	word1_mask = ((1 << size_align1) - 1) << where_offset1;
	word1_value = (value << where_offset1) & word1_mask;
	*pword1 &= ~word1_mask;
	*pword1 |= word1_value;

	/* If the whole size was covered in the first chunk, exit */
	if (size_align1 == size)
		return;

	/* Bits to get from second word */
	where_word2 = where_word1 + 1;
	size_align2 = size - size_align1;
	if (where_word2 >= self->size_in_words)
		return;
	pword2 = &self->data[where_word2];

	/* Add to result */
	word2_mask = (1 << size_align2) - 1;
	word2_value = (value >> size_align1) & word2_mask;
	*pword2 &= ~word2_mask;
	*pword2 |= word2_value;
}


unsigned int BitmapGet(Bitmap *self, unsigned int where,
		unsigned int size)
{
	unsigned int where_word1;
	unsigned int where_offset1;
	unsigned int size_align1;
	unsigned int size_align2;
	unsigned int word1;
	unsigned int word1_mask;
	unsigned int word2_mask;
	unsigned int result;

	/* Check for valid size */
	if (size < 0 || size > 32)
		panic("%s: invalid value for 'size' (%d)",
				__FUNCTION__, size);

	/* Bits to get from first word */
	where_word1 = where / 32;
	where_offset1 = where % 32;
	size_align1 = MIN(size, 32 - where_offset1);
	word1 = where_word1 < self->size_in_words ? self->data[where_word1] : 0;

	/* Special case: 32-bit aligned word */
	if (size == 32 && !where_offset1)
		return word1;

	/* Get shift and mask */
	word1_mask = ((1 << size_align1) - 1) << where_offset1;
	result = (word1 & word1_mask) >> where_offset1;

	/* If the whole size was covered in the first chunk, exit */
	if (size_align1 == size)
		return result;

	/* Bits to get from second word */
	size_align2 = size - size_align1;

	/* Add to result */
	word2_mask = (1 << size_align2) - 1;
	result |= (word1 & word2_mask) << size_align1;
	return result;
}


int BitmapCountZeros(Bitmap *self, unsigned int where, unsigned int size)
{
	unsigned int i;
	int count = 0;

	for (i = 0; i < size; i++)
		count += !BitmapGet(self, where + i, 1);
	return count;
}


int BitmapCountOnes(Bitmap *self, unsigned int where, unsigned int size)
{
	unsigned int i;
	int count = 0;

	for (i = 0; i < size; i++)
		count += BitmapGet(self, where + i, 1);
	return count;
}


void BitmapMove(Bitmap *self, unsigned int dst_where, Bitmap *src,
		unsigned int src_where, unsigned int size)
{
	unsigned int current_size;
	unsigned int word;

	while (size)
	{
		current_size = MIN(size, 32);
		word = BitmapGet(src, src_where, current_size);
		BitmapSet(self, dst_where, current_size, word);
		size -= current_size;
		src_where += current_size;
		dst_where += current_size;
	}
}


void BitmapNot(Bitmap *self)
{
	int i;

	for (i = 0; i < self->size_in_words; i++)
		self->data[i] = ~self->data[i];
}


void BitmapAnd(Bitmap *self, Bitmap *operand)
{
	int i;

	/* Check size */
	if (self->size != operand->size)
		panic("%s: bitmaps have different size (%d vs. %d)",
				__FUNCTION__, self->size, operand->size);

	/* And */
	for (i = 0; i < self->size_in_words; i++)
		self->data[i] &= operand->data[i];
}


void BitmapOr(Bitmap *self, Bitmap *operand)
{
	int i;

	/* Check size */
	if (self->size != operand->size)
		panic("%s: bitmaps have different size (%d vs. %d)",
				__FUNCTION__, self->size, operand->size);

	/* Or */
	for (i = 0; i < self->size_in_words; i++)
		self->data[i] |= operand->data[i];
}


void BitmapXor(Bitmap *self, Bitmap *operand)
{
	int i;

	/* Check size */
	if (self->size != operand->size)
		panic("%s: bitmaps have different size (%d vs. %d)",
				__FUNCTION__, self->size, operand->size);

	/* Xor */
	for (i = 0; i < self->size_in_words; i++)
		self->data[i] ^= operand->data[i];
}

void BitmapSub(Bitmap *self, Bitmap *operand)
{
	/* Check size */
	if (self->size != operand->size)
		panic("%s: bitmaps have different size (%d vs. %d)",
				__FUNCTION__, self->size, operand->size);

	/* Sub */
	BitmapNot(operand);
	BitmapAnd(self, operand);

	/* Replace operand */
	BitmapNot(operand);
}
