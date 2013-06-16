/*
 *  Multi2Sim
 *  Copyright (C) 2013  Chris Barton (barton.ch@husky.neu.edu)
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

#include "lib/mhandle/mhandle.h"

#include "format.h"

char *extract_file_name(char *text)
{
	int i = 1;
	int j = 0;
	char buffer[100];
	char *ret_buffer;
	
	/*Copy file name to buffer*/
	while(text[i] != 34)
	{
		i++;
	}
	i++;
	while(text[i] != 34)
	{
		buffer[j] = text[i];
		i++;
		j++;
	}
	buffer[j] = '\00';
	ret_buffer = xstrdup(buffer);

	return ret_buffer;
}

