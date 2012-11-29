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

#ifndef ARCH_EVERGREEN_EMU_OPENGL_BIN_FILE_H
#define ARCH_EVERGREEN_EMU_OPENGL_BIN_FILE_H

#include <lib/util/elf-format.h>


/* Shader types */
enum si_opengl_shader_kind_t
{
	si_OPENGL_SHADER_VERTEX,
	si_OPENGL_SHADER_FRAGMENT,
	si_OPENGL_SHADER_GEOMETRY,
	si_OPENGL_SHADER_EVALUATION,
	si_OPENGL_SHADER_CONTROL
};

/* OpenGL shader binary */
struct si_opengl_shader_t
{
	/* Shader kind */
	enum si_opengl_shader_kind_t shader_kind;

	/* Associated ELF file */
	struct elf_file_t *external_elf_file;
	struct elf_file_t *internal_elf_file;

	/* ISA buffer, which ptr element points to .text section in internel_elf_file  */
	struct elf_buffer_t isa_buffer;
};

/* OpenGL shader binary */
struct si_opengl_bin_file_t
{
	/* Name of the associated binary file */
	char *name;
	
	/* List of shaders associated with binary file.
	 * Elements are of type 'struct si_opengl_shader_t' */
	struct list_t *shader_list;

	/* NEED or NOT ? */
	// struct si_opengl_shader_t *amd_opengl_shader;
};

struct si_opengl_bin_file_t *si_opengl_bin_file_create(void *ptr, int size, char *name);
void si_opengl_bin_file_free(struct si_opengl_bin_file_t *bin_file);


#endif

