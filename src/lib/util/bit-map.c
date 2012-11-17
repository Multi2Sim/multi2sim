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


#include <lib/mhandle/mhandle.h>

#include "bit-map.h"
#include "misc.h"


struct bit_map_t
{
	unsigned int size;
	unsigned int word_count;  /* Size in words */
	unsigned int data[0];
};


struct bit_map_t *bit_map_create(unsigned int size)
{
	struct bit_map_t *map;
	unsigned int word_count;

	word_count = (size + 31) / 32;
	map = xcalloc(1, sizeof(struct bit_map_t) + word_count * 4);
	map->size = size;
	map->word_count = word_count;
	return map;
}


void bit_map_free(struct bit_map_t *bit_map)
{
	free(bit_map);
}


/* Set 'size' bits to 'value' starting at position 'where'. Argument 'size' cannot
 * exceed 32 bits. */
void bit_map_set(struct bit_map_t *bit_map, unsigned int where, unsigned int size,
	unsigned int value)
{
	unsigned int where_word1, where_offset1, size_align1;
	unsigned int where_word2, size_align2;
	unsigned int *pword1, word1_mask, word1_value;
	unsigned int *pword2, word2_mask, word2_value;

	if (size > 32)
		abort();
	
	/* Bits to get from first word */
	where_word1 = where / 32;
	where_offset1 = where % 32;
	size_align1 = MIN(size, 32 - where_offset1);
	if (where_word1 >= bit_map->word_count)
		return;
	pword1 = &bit_map->data[where_word1];

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
	if (where_word2 >= bit_map->word_count)
		return;
	pword2 = &bit_map->data[where_word2];

	/* Add to result */
	word2_mask = (1 << size_align2) - 1;
	word2_value = (value >> size_align1) & word2_mask;
	*pword2 &= ~word2_mask;
	*pword2 |= word2_value;
}


unsigned int bit_map_get(struct bit_map_t *bit_map, unsigned int where, unsigned int size)
{
	unsigned int where_word1, where_offset1, size_align1;
	unsigned int size_align2;
	unsigned int word1, word1_mask;
	unsigned int word2_mask;
	unsigned int result;

	if (size > 32)
		abort();
	
	/* Bits to get from first word */
	where_word1 = where / 32;
	where_offset1 = where % 32;
	size_align1 = MIN(size, 32 - where_offset1);
	word1 = where_word1 < bit_map->word_count ? bit_map->data[where_word1] : 0;

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


int bit_map_count_ones(struct bit_map_t *bit_map, unsigned int where, unsigned int size)
{
	unsigned int i;
	int count = 0;

	for (i = 0; i < size; i++)
		count += bit_map_get(bit_map, where + i, 1);
	return count;
}


void bit_map_copy(struct bit_map_t *dst, unsigned int dst_where,
	struct bit_map_t *src, unsigned int src_where, unsigned int size)
{
	unsigned int current_size;
	unsigned int word;

	while (size) {
		current_size = MIN(size, 32);
		word = bit_map_get(src, src_where, current_size);
		bit_map_set(dst, dst_where, current_size, word);
		size -= current_size;
		src_where += current_size;
		dst_where += current_size;
	}
}


void bit_map_dump(struct bit_map_t *bit_map, unsigned int where, unsigned int size, FILE *f)
{
	unsigned int i;
	for (i = 0; i < size; i++)
		fprintf(f, "%d", bit_map_get(bit_map, where + i, 1));
}

