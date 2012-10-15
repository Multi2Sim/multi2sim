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

#ifndef LIB_UTIL_FILE_H
#define LIB_UTIL_FILE_H

#include <stdio.h>


FILE *file_open_for_read(char *file_name);
FILE *file_open_for_write(char *file_name);
void file_close(FILE *f);

int file_can_open_for_read(char *file_name);
int file_can_open_for_write(char *file_name);

int file_read_line(FILE *f, char *line, int size);
FILE *file_create_temp(char *ret_path, int ret_path_size);

void file_full_path(char *file_name, char *default_path, char *full_path, int size);

#endif

