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

#include <assert.h>

#include <arch/southern-islands/emu/opengl-bin-file.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>

#include "context.h"
#include "opengl.h"
#include "program.h"
#include "shader.h"


static unsigned int program_id = 1;

struct opengl_program_t *opengl_program_create()
{
	struct opengl_program_t *prg;

	/* Allocate */
	prg = xcalloc(1, sizeof(struct opengl_program_t));

	/* Initialize */
	prg->id = program_id;
	prg->ref_count = 0;
	prg->delete_pending = GL_FALSE;
	prg->attached_shader_id_list = linked_list_create();
	prg->si_shader_binary = NULL;

	/* Update global program id */
	program_id += 1;

	/* Return */
	return prg;
}

static unsigned int *opengl_program_get_shader_id_obj(struct opengl_program_t* prg)
{
	unsigned int *shdr_id;

	LINKED_LIST_FOR_EACH(prg->attached_shader_id_list)
	{
		shdr_id = linked_list_get(prg->attached_shader_id_list);
		opengl_debug("\tGet Shader ID object #%d [%p] from list [%p]\n", *shdr_id, shdr_id, prg->attached_shader_id_list);
		assert(shdr_id);
		return shdr_id;
	}
	return NULL;
}

/* Free doesn't check flags */
void opengl_program_free(struct opengl_program_t *prg)
{
	unsigned int *shdr_id;

	/* Free */
	while((shdr_id = opengl_program_get_shader_id_obj(prg)))
	{
		free(shdr_id);
	}
	linked_list_free(prg->attached_shader_id_list);
	si_opengl_bin_file_free(prg->si_shader_binary);
	free(prg);
}

/* Delete checks flags */
void opengl_program_detele(struct opengl_program_t *prg)
{
	prg->delete_pending = GL_TRUE;
	opengl_debug("\tProgram #%d delete pending\n", prg->id);

	if (prg->ref_count == 0)
	{
		opengl_program_free(prg);
		opengl_debug("\tProgram #%d deleted\n", prg->id);
	}
}

/* Attach shader doesn't actually attach shader object, it only record the
 * shader ID */
void opengl_program_attach_shader(struct opengl_program_t *prg,
	struct opengl_shader_t *shdr)
{
	unsigned int *shdr_id = xcalloc(1, sizeof(unsigned int));
	struct si_opengl_shader_t *shader;
	int i;

	if (shdr && prg)
	{
		/* Add shader to ID list */
		shdr->ref_count += 1;
		*shdr_id = shdr->id;
		linked_list_add(prg->attached_shader_id_list, shdr_id);
		opengl_debug("\tShader ID #%d [%p] added to ID list [%p]\n",
			*shdr_id, shdr_id, prg->attached_shader_id_list);
		opengl_debug
			("\tShader #%d [%p] attached to Program #%d [%p]\n",
			shdr->id, shdr, prg->id, prg);

		/* Copy ISA from program */
		switch (shdr->type)
		{
		case GL_VERTEX_SHADER:
		{
			/* Find appropiate shader in shader binary stored in program object */
			for (i = 0; i < list_count(prg->si_shader_binary->shader_list); ++i)
			{
				shader = list_get(prg->si_shader_binary->shader_list, i);
				if (shader->shader_kind == SI_OPENGL_SHADER_VERTEX)
				{
					shdr->isa_buffer = xcalloc(1, shader->isa_buffer.size);
					memcpy(shdr->isa_buffer, shader->isa_buffer.ptr, shader->isa_buffer.size);
					opengl_debug("\tSet Shader ISA buffer: copy %d byte from binary [%p] in program #%d [%p]\n", shader->isa_buffer.size, prg->si_shader_binary, prg->id, prg);
				}
			}
			break;
		}
		case GL_FRAGMENT_SHADER:
		{
			/* Find appropiate shader in shader binary stored in program object */
			for (i = 0; i < list_count(prg->si_shader_binary->shader_list); ++i)
			{
				shader = list_get(prg->si_shader_binary->shader_list, i);
				if (shader->shader_kind == SI_OPENGL_SHADER_FRAGMENT)
				{
					shdr->isa_buffer = xcalloc(1, shader->isa_buffer.size);
					memcpy(shdr->isa_buffer, shader->isa_buffer.ptr, shader->isa_buffer.size);
					opengl_debug("\tSet Shader ISA buffer: copy %d byte from binary [%p] in program #%d [%p]\n", shader->isa_buffer.size, prg->si_shader_binary, prg->id, prg);
				}
			}
			break;
		}
		case GL_GEOMETRY_SHADER:
		{
			/* Find appropiate shader in shader binary stored in program object */
			for (i = 0; i < list_count(prg->si_shader_binary->shader_list); ++i)
			{
				shader = list_get(prg->si_shader_binary->shader_list, i);
				if (shader->shader_kind == SI_OPENGL_SHADER_GEOMETRY)
				{
					shdr->isa_buffer = xcalloc(1, shader->isa_buffer.size);
					memcpy(shdr->isa_buffer, shader->isa_buffer.ptr, shader->isa_buffer.size);
					opengl_debug("\tSet Shader ISA buffer: copy %d byte from binary [%p] in program #%d [%p]\n", shader->isa_buffer.size, prg->si_shader_binary, prg->id, prg);
				}
			}
			break;
		}
		default:
			opengl_debug("\tUnable to find Shader type = %x\n", shdr->type);
			break;
		}
	}
	else
		opengl_debug("\tError: Invalid Shader [%p] / Program [%p]\n", shdr, prg);
}

void opengl_program_detach_shader(struct opengl_program_t *prg, struct opengl_shader_t *shdr)
{
	unsigned int *shdr_id;

	if (shdr && prg)
	{
		shdr->ref_count -= 1;
		/* Search shader ID */
		LINKED_LIST_FOR_EACH(prg->attached_shader_id_list)
		{
			shdr_id = linked_list_get(prg->attached_shader_id_list);
			assert(shdr_id);
			if (shdr->id == *shdr_id)
				/* Remove shader ID from list */
				linked_list_remove(prg->attached_shader_id_list);
		}
		opengl_debug("\tShader #%d [%p] detached to Program #%d [%p]\n", shdr->id, shdr, prg->id, prg);
	}
	else
		opengl_debug("\tError: Invalid Shader [%p] / Program [%p]\n", shdr, prg);
}

struct elf_buffer_t *opengl_program_get_shader(struct opengl_program_t *prg, int shader_kind)
{
	struct list_t *shader_list;
	struct si_opengl_shader_t *si_shader;
	int i;

	shader_list = prg->si_shader_binary->shader_list;
	LIST_FOR_EACH(shader_list, i)
	{
		si_shader = list_get(shader_list, i);
		if (si_shader->shader_kind == shader_kind)
		{
			return &si_shader->isa_buffer;
		}
	}
	/* Not found */
	fatal("Shader not found!");
	return NULL;
}

void opengl_program_bind(struct opengl_program_t *prg, struct opengl_context_t *ctx)
{
	if (prg && ctx)
	{
		if (ctx->current_program)
		{
			/* Remove and update current  program */
			ctx->current_program->ref_count -= 1;
			/* Bind program */
			prg->ref_count += 1;
			ctx->current_program = prg;
			opengl_debug("\tProgram #%d [%p] bind to OpenGL context [%p]\n", prg->id, prg, ctx);
		}
	}
}

void opengl_program_unbind(struct opengl_program_t *prg, struct opengl_context_t *ctx)
{
	if (prg && ctx)
	{
		prg->ref_count -= 1;
		ctx->current_program = NULL;
		opengl_debug("\tProgram #%d [%p] bind to OpenGL context [%p]\n", prg->id, prg, ctx);
	}
}

struct linked_list_t *opengl_program_repo_create()
{
	struct linked_list_t *lst;

	/* Allocate */
	lst = linked_list_create();

	opengl_debug("\tCreated Program Repo [%p]\n", lst);

	/* Return */
	return lst;
}

void opengl_program_repo_free(struct linked_list_t *prg_repo)
{
	struct opengl_program_t *prg;

	/* Free all elements */
	LINKED_LIST_FOR_EACH(prg_repo)
	{
		prg = linked_list_get(prg_repo);
		assert(prg);
		opengl_program_free(prg);
	}

	/* Free program repository */
	linked_list_free(prg_repo);

	opengl_debug("\tFreed Program Repo [%p]\n", prg_repo);
}

struct opengl_program_t *opengl_program_repo_get(struct linked_list_t *prg_repo, int id)
{
	struct opengl_program_t *prg;

	/* Search shader */
	LINKED_LIST_FOR_EACH(prg_repo)
	{
		prg = linked_list_get(prg_repo);
		assert(prg);
		if (prg->id == id)
			return prg;
	}

	/* Not found */
	fatal("%s: requested Program does not exist (id=0x%x)",
		__FUNCTION__, id);
	return NULL;
}

void opengl_program_repo_add(struct linked_list_t *prg_repo, struct opengl_program_t *prg)
{
	linked_list_add(prg_repo, prg);
	opengl_debug("\tAdded Program %d [%p] to Program Repo [%p]\n", prg->id, prg, prg_repo);
}

int opengl_program_repo_remove(struct linked_list_t *prg_repo, struct opengl_program_t *prg)
{
	if (prg->ref_count != 0)
	{
		opengl_debug("\tProgram %d [%p] cannot be removed immediately, as reference counter = %d\n", prg->id, prg, prg->ref_count);
		return -1;
	}
	else 
	{
		/* Check if program exists */
		linked_list_find(prg_repo, prg);
		if (prg_repo->error_code)
			fatal("%s: Program does not exist", __FUNCTION__);

		linked_list_remove(prg_repo);
		opengl_debug("\tProgram %d [%p] removed from Program table\n", prg->id, prg);
		return 1;
	}
}