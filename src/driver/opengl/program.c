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
#include <arch/southern-islands/emu/isa.h>
#include <arch/southern-islands/emu/ndrange.h>
#include <arch/southern-islands/emu/wavefront.h>
#include <arch/southern-islands/emu/work-group.h>
#include <arch/southern-islands/emu/work-item.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>

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
	pthread_mutex_init(&prg->ref_mutex, NULL);
	prg->delete_pending = GL_FALSE;
	prg->attached_shader = linked_list_create();
	prg->si_shader_binary = NULL;

	/* Update global program id */
	program_id += 1;

	/* Return */
	return prg;
}

/* Free doesn't check flags */
void opengl_program_free(struct opengl_program_t *prg)
{
	struct opengl_shader_t *shdr;

	/* Update attached shader reference */
	LINKED_LIST_FOR_EACH(prg->attached_shader)
	{
		shdr = linked_list_get(prg->attached_shader);
		if(shdr)
			opengl_shader_ref_update(shdr, -1);
	}

	pthread_mutex_destroy(&prg->ref_mutex);
	linked_list_free(prg->attached_shader);
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

/* Update reference count */
void opengl_program_ref_update(struct opengl_program_t *prg, int change)
{
	int count;

	pthread_mutex_lock(&prg->ref_mutex);
	prg->ref_count += change;
	count = prg->ref_count;
	pthread_mutex_unlock(&prg->ref_mutex);

	if (count < 0)
		panic("%s: number of references is negative", __FUNCTION__);
}

/* Attach shader record the shader pointer */
void opengl_program_attach_shader(struct opengl_program_t *prg,
	struct opengl_shader_t *shdr)
{
	struct si_opengl_shader_t *shader;
	int i;

	if (shdr && prg)
	{
		/* Add shader to  list */
		linked_list_add(prg->attached_shader, shdr);

		/* Update reference count */
		opengl_shader_ref_update(shdr, 1);

		/* Debug */
		opengl_debug	("\tShader #%d [%p] attached to Program #%d [%p]\n",
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
					opengl_debug("\tSet Shader ISA buffer: copy %d byte from binary [%p] in program #%d [%p]\n", 
						shader->isa_buffer.size, prg->si_shader_binary, prg->id, prg);
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
					opengl_debug("\tSet Shader ISA buffer: copy %d byte from binary [%p] in program #%d [%p]\n", 
						shader->isa_buffer.size, prg->si_shader_binary, prg->id, prg);
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
					opengl_debug("\tSet Shader ISA buffer: copy %d byte from binary [%p] in program #%d [%p]\n",
					 shader->isa_buffer.size, prg->si_shader_binary, prg->id, prg);
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
	struct opengl_shader_t *shader;

	if (shdr && prg)
	{
		/* Search shader */
		LINKED_LIST_FOR_EACH(prg->attached_shader)
		{
			shader = linked_list_get(prg->attached_shader);
			assert(shader);
			if (shader == shdr)
			{
				/* Update reference count */
				opengl_shader_ref_update(shdr, -1);
		
				/* Remove shader from list */
				linked_list_remove(prg->attached_shader);
			}
		}
		opengl_debug("\tShader #%d [%p] detached to Program #%d [%p]\n", shdr->id, shdr, prg->id, prg);
	}
	else
		opengl_debug("\tError: Invalid Shader [%p] / Program [%p]\n", shdr, prg);
}

struct si_opengl_shader_t *opengl_program_get_shader(struct opengl_program_t *prg, int shader_kind)
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
			return si_shader;
		}
	}
	/* Not found */
	fatal("Shader not found!");
	return NULL;

}

struct elf_buffer_t *opengl_program_get_shader_isa(struct opengl_program_t *prg, int shader_kind)
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

/* Bind program to program binding point */
void opengl_program_bind(struct opengl_program_t *prg, struct opengl_program_t **prg_bnd_ptr)
{
	if (prg)
	{
		/* Dereference current  program */
		if (*prg_bnd_ptr)
		{
			opengl_program_ref_update(*prg_bnd_ptr, -1);
			*prg_bnd_ptr = NULL;			
		}

		/* Reference and update binding point */
		opengl_program_ref_update(prg, 1);

		/* Bind program to binding point */
		*prg_bnd_ptr = prg;
		
		/* Debug */
		opengl_debug("\tProgram #%d [%p] bind to Binding Point [%p]\n", prg->id, prg,*prg_bnd_ptr);
	}
}

/* Unbind program from program binding point */
void opengl_program_unbind(struct opengl_program_t *prg, struct opengl_program_t **prg_bnd_ptr)
{
	if (prg)
	{
		/* Update reference count */
		if (*prg_bnd_ptr)
			opengl_program_ref_update(prg, -1);

		/* Clear binding point */
		*prg_bnd_ptr = NULL;

		/* Debug */
		opengl_debug("\tProgram #%d [%p] unbind from Binding point [%p]\n", prg->id, prg,*prg_bnd_ptr);
	}
}

void opengl_program_setup_ndrange_state(struct opengl_program_t *prg,
		struct si_ndrange_t *ndrange)
{
#if 0
	struct si_wavefront_t *wavefront;
	struct si_work_item_t *work_item;

	int work_item_id;
	int wavefront_id;

	unsigned int user_sgpr=0;
	unsigned int zero = 0;

	float f;

	/* Save local IDs in registers */
	SI_FOREACH_WORK_ITEM_IN_NDRANGE(ndrange, work_item_id)
	{
		work_item = ndrange->work_items[work_item_id];
		work_item->vreg[0].as_int = work_item->id_in_work_group_3d[0];  /* V0 */
		work_item->vreg[1].as_int = work_item->id_in_work_group_3d[1];  /* V1 */
		work_item->vreg[2].as_int = work_item->id_in_work_group_3d[2];  /* V2 */
	}

	/* Initialize the wavefronts */
	SI_FOREACH_WAVEFRONT_IN_NDRANGE(ndrange, wavefront_id)
	{
		/* Get wavefront */
		wavefront = ndrange->wavefronts[wavefront_id];

		/* Save work-group IDs in registers */
		wavefront->sreg[user_sgpr].as_int =
			wavefront->work_group->id_3d[0];
		wavefront->sreg[user_sgpr + 1].as_int =
			wavefront->work_group->id_3d[1];
		wavefront->sreg[user_sgpr + 2].as_int =
			wavefront->work_group->id_3d[2];

		/* Initialize the execution mask */
		wavefront->sreg[SI_EXEC].as_int = 0xffffffff;
		wavefront->sreg[SI_EXEC + 1].as_int = 0xffffffff;
		wavefront->sreg[SI_EXECZ].as_int = 0;
	}

	/* CB0 bytes 0:15 */

	/* Global work size for the {x,y,z} dimensions */
	si_ndrange_const_buf_write(ndrange, 0, 0, 
		&ndrange->global_size3[0], 4);
	si_ndrange_const_buf_write(ndrange, 0, 4, 
		&ndrange->global_size3[1], 4);
	si_ndrange_const_buf_write(ndrange, 0, 8, 
		&ndrange->global_size3[2], 4);

	/* Number of work dimensions */
	si_ndrange_const_buf_write(ndrange, 0, 12, &ndrange->work_dim, 4);

	/* CB0 bytes 16:31 */

	/* Local work size for the {x,y,z} dimensions */
	si_ndrange_const_buf_write(ndrange, 0, 16, 
		&ndrange->local_size3[0], 4);
	si_ndrange_const_buf_write(ndrange, 0, 20, 
		&ndrange->local_size3[1], 4);
	si_ndrange_const_buf_write(ndrange, 0, 24, 
		&ndrange->local_size3[2], 4);

	/* 0  */
	si_ndrange_const_buf_write(ndrange, 0, 28, &zero, 4);

	/* CB0 bytes 32:47 */

	/* Global work size {x,y,z} / local work size {x,y,z} */
	si_ndrange_const_buf_write(ndrange, 0, 32, 
		&ndrange->group_count3[0], 4);
	si_ndrange_const_buf_write(ndrange, 0, 36, 
		&ndrange->group_count3[1], 4);
	si_ndrange_const_buf_write(ndrange, 0, 40, 
		&ndrange->group_count3[2], 4);

	/* 0  */
	si_ndrange_const_buf_write(ndrange, 0, 44, &zero, 4);

	/* CB0 bytes 48:63 */

	/* FIXME Offset to private memory ring (0 if private memory is
	 * not emulated) */

	/* FIXME Private memory allocated per work_item */

	/* 0  */
	si_ndrange_const_buf_write(ndrange, 0, 56, &zero, 4);

	/* 0  */
	si_ndrange_const_buf_write(ndrange, 0, 60, &zero, 4);

	/* CB0 bytes 64:79 */

	/* FIXME Offset to local memory ring (0 if local memory is
	 * not emulated) */

	/* FIXME Local memory allocated per group */

	/* 0 */
	si_ndrange_const_buf_write(ndrange, 0, 72, &zero, 4);

	/* FIXME Pointer to location in global buffer where math library
	 * tables start. */

	/* CB0 bytes 80:95 */

	/* 0.0 as IEEE-32bit float - required for math library. */
	f = 0.0f;
	si_ndrange_const_buf_write(ndrange, 0, 80, &f, 4);

	/* 0.5 as IEEE-32bit float - required for math library. */
	f = 0.5f;
	si_ndrange_const_buf_write(ndrange, 0, 84, &f, 4);

	/* 1.0 as IEEE-32bit float - required for math library. */
	f = 1.0f;
	si_ndrange_const_buf_write(ndrange, 0, 88, &f, 4);

	/* 2.0 as IEEE-32bit float - required for math library. */
	f = 2.0f;
	si_ndrange_const_buf_write(ndrange, 0, 92, &f, 4);

	/* CB0 bytes 96:111 */

	/* Global offset for the {x,y,z} dimension of the work_item spawn */
	si_ndrange_const_buf_write(ndrange, 0, 96, &zero, 4);
	si_ndrange_const_buf_write(ndrange, 0, 100, &zero, 4);
	si_ndrange_const_buf_write(ndrange, 0, 104, &zero, 4);

	/* Global single dimension flat offset: x * y * z */
	si_ndrange_const_buf_write(ndrange, 0, 108, &zero, 4);

	/* CB0 bytes 112:127 */

	/* Group offset for the {x,y,z} dimensions of the work_item spawn */
	si_ndrange_const_buf_write(ndrange, 0, 112, &zero, 4);
	si_ndrange_const_buf_write(ndrange, 0, 116, &zero, 4);
	si_ndrange_const_buf_write(ndrange, 0, 120, &zero, 4);

	/* Group single dimension flat offset, x * y * z */
	si_ndrange_const_buf_write(ndrange, 0, 124, &zero, 4);

	/* CB0 bytes 128:143 */

	/* FIXME Offset in the global buffer where data segment exists */
	/* FIXME Offset in buffer for printf support */
	/* FIXME Size of the printf buffer */
#endif
}


struct linked_list_t *opengl_program_repo_create()
{
	struct linked_list_t *lst;

	/* Allocate */
	lst = linked_list_create();

	opengl_debug("\tCreated Program repository [%p]\n", lst);

	/* Return */
	return lst;
}

void opengl_program_repo_free(struct linked_list_t *prg_repo)
{
	opengl_debug("\tFree Program repository [%p]\n", prg_repo);

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

}

struct opengl_program_t *opengl_program_repo_get(struct linked_list_t *prg_repo, int id)
{
	struct opengl_program_t *prg;

	/* Search program */
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

struct opengl_program_t *opengl_program_repo_reference(struct linked_list_t *prg_repo, int id)
{
	struct opengl_program_t *prg;

	/* Get and update reference count */
	prg = opengl_program_repo_get(prg_repo, id);
	opengl_program_ref_update(prg, 1);

	/* Return */
	return prg;
}
