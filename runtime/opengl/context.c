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

#include <unistd.h>
#include <stdio.h>

#include "buffer.h"
#include "context.h"
#include "debug.h"
#include "linked-list.h"
#include "mhandle.h"
#include "program.h"
#include "shader.h"

/* 
 * Other libraries are responsible for initializing OpenGL context.
 * In Multi2Sim, it is initialized in glutInit() in glut runtime library.
 */ 
static unsigned int opengl_context_initialized;
struct opengl_context_t *opengl_ctx;


/*
 * Private Functions
 */

static struct opengl_context_state_t *opengl_context_state_create()
{
	struct opengl_context_state_t *state;

	/* Allocate */
	state = xcalloc(1, sizeof(struct opengl_context_state_t));

	/* Debug */
	opengl_debug("%s: [%p] created\n", __FUNCTION__, state);

	/* Return */
	return state;
}

static void opengl_context_state_free(struct opengl_context_state_t *state)
{
	/* Free */
	free(state);

	/* Debug */
	opengl_debug("%s: [%p] freed\n", __FUNCTION__, state);
}

static struct opengl_context_props_t *opengl_context_props_create()
{
	struct opengl_context_props_t *properties;

	/* Allocate */
	properties = xcalloc(1, sizeof(struct opengl_context_props_t));

	/* Initialize */
	opengl_context_props_set_version(0, 0, properties);
	properties->vendor = xstrdup("Multi2Sim");
	properties->renderer = xstrdup("OpenGL Emulator");

	/* Debug */
	opengl_debug("%s: [%p] created\n", __FUNCTION__, properties);

	/* Return */
	return properties;
}

static void opengl_context_props_free(struct opengl_context_props_t *props)
{
	/* Free */
	free(props->vendor);
	free(props->renderer);
	free(props);

	/* Debug */
	opengl_debug("%s: [%p] freed\n", __FUNCTION__, props);	
}

static struct opengl_context_t *opengl_context_create()
{
	struct opengl_context_t *context;

	/* Allocate */
	context = xcalloc(1, sizeof(struct opengl_context_t));
	context->state = opengl_context_state_create();
	context->props = opengl_context_props_create();

	/* Debug */
	opengl_debug("%s: OpenGL context [%p] created\n", __FUNCTION__, context);
	/* Return */
	return context;
}

static void opengl_context_free(struct opengl_context_t *context)
{
	/* Free */
	opengl_context_state_free(context->state);
	opengl_context_props_free(context->props);
	free(context);

	/* Debug */
	opengl_debug("%s: OpenGL context [%p] freed\n", __FUNCTION__, context);
}

/*
 * Public Functions
 */

void opengl_context_props_set_version(unsigned int major, unsigned int minor, 
	struct opengl_context_props_t *props)
{
	props->gl_major = major;
	props->gl_minor = minor;
	props->glsl_major = major;
	props->glsl_minor = minor;

	/* Debug */
	opengl_debug("%s: Opengl properties [%p] set, GL %d.%d, GLSL %d.%d\n",
		__FUNCTION__, props, major, minor, major, minor);
}

void opengl_context_init()
{
	if (!opengl_context_initialized)
	{
		/* Initialize OpenGL context */
		opengl_ctx = opengl_context_create();
		opengl_context_initialized = 1;

		opengl_ctx->buffer_binding_points = opengl_buffer_binding_points_create();

		/*FIXME: Repository Initialization */

		/* Debug */
		opengl_debug("%s: OpenGL context [%p] initialized\n", __FUNCTION__, opengl_ctx);	
	}
	if (!buffer_repo)
		buffer_repo = opengl_buffer_obj_repo_create();
	if (!program_repo)
		program_repo = opengl_program_obj_repo_create();
	if (!shader_repo)
		shader_repo = opengl_shader_obj_repo_create();
}

void opengl_context_destroy()
{
	if (opengl_context_initialized)
	{
		opengl_context_state_free(opengl_ctx->state);
		opengl_context_props_free(opengl_ctx->props);
		opengl_buffer_binding_points_free(opengl_ctx->buffer_binding_points);

		/*FIXME: Repository Free*/

		opengl_context_initialized = 0;
		opengl_context_free(opengl_ctx);
	}
	if (buffer_repo)
		opengl_buffer_obj_repo_free(buffer_repo);
	if (program_repo)
		opengl_program_obj_repo_free(program_repo);
	if (shader_repo)
		opengl_shader_obj_repo_free(shader_repo);
}

/* 
 * OpenGL API functions 
 */

 const GLubyte *glGetString( GLenum name )
{
	opengl_debug("OpenGL API %s \n", __FUNCTION__);

	const GLubyte *str;

	switch(name)
	{

	case GL_VENDOR:
	{
		sprintf(opengl_ctx->props->info, "%s\n", opengl_ctx->props->vendor);
		str = (const GLubyte *)opengl_ctx->props->info;
		break;
	}

	case GL_RENDERER:
	{
		sprintf(opengl_ctx->props->info, "%s\n", opengl_ctx->props->renderer);
		str = (const GLubyte *)opengl_ctx->props->info;
		break;
	}

	case GL_VERSION:
	{
		sprintf(opengl_ctx->props->info, "Multi2Sim OpenGL %u.%u\n", 
			opengl_ctx->props->gl_major, opengl_ctx->props->gl_minor);
		str = (const GLubyte *)opengl_ctx->props->info;
		break;
	}

	case GL_SHADING_LANGUAGE_VERSION:
	{
		sprintf(opengl_ctx->props->info, "Multi2Sim OpenGL GLSL %u.%u\n", 
			opengl_ctx->props->gl_major, opengl_ctx->props->gl_minor);
		str = (const GLubyte *)opengl_ctx->props->info;
		break;
	}

	default:
		str = NULL;
		break;
	}

	/* Return */
	return str;
}

void glEnable( GLenum cap )
{
	opengl_debug("OpenGL runtime: %s\n", __FUNCTION__);	
}
