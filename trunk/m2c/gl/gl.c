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

#include <GL/glx.h>
#include <sys/stat.h>
#include <assert.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "gl.h"

/*
 * Public Variables
 */

int gl_dump_all;  /* Dump intermediate files */
char *gl_control_shader;
char *gl_eval_shader;
char *gl_geo_shader;

/* These are passed as parameters in the source file list,
 * declared as private variables. */
static char *gl_vertex_shader;
static char *gl_fragment_shader;


/*
 * Private Functions
 */

#define PROGRAM_BINARY_RETRIEVABLE_HINT  0x8257

/* For context initialization */
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
typedef Bool (*glXMakeContextCurrentARBProc)(Display*, GLXDrawable, GLXDrawable, GLXContext);
static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
static PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
static PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
static PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
static PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
static PFNGLGETSHADERIVPROC glGetShaderiv;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
static PFNGLCREATESHADERPROC glCreateShader;
static PFNGLSHADERSOURCEPROC glShaderSource;
static PFNGLCOMPILESHADERPROC glCompileShader;
static PFNGLCREATEPROGRAMPROC glCreateProgram;
static PFNGLATTACHSHADERPROC glAttachShader;
static PFNGLLINKPROGRAMPROC glLinkProgram;
static PFNGLGETPROGRAMIVPROC glGetProgramiv;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
static PFNGLUSEPROGRAMPROC glUseProgram;
static PFNGLPROGRAMPARAMETERIPROC glProgramParameteri;
static PFNGLGETPROGRAMBINARYPROC glGetProgramBinary;

/* X & Context */
static Display* dpy ;
static int fbcount;
static GLXFBConfig* fbc;
static GLXContext ctx;
static GLXPbuffer pbuf;

/* Buffer IDs */
static GLuint m_framebuffer1;
static GLuint m_colorRenderbuffer1;
static GLuint m_depthRenderbuffer;

static glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
static glXMakeContextCurrentARBProc glXMakeContextCurrentARB = 0;


static int gl_context_init_x(unsigned int major_version, unsigned int minor_version)
{
	/* FIXME - check if scope of these variables can be made local */
	int pbuffer_width = 32;
	int pbuffer_height = 32;

	static int visual_attribs[] = 
	{
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		GLX_STENCIL_SIZE, 8,
		None
	};

	int context_attribs[] = 
	{
		GLX_CONTEXT_MAJOR_VERSION_ARB, major_version,
		GLX_CONTEXT_MINOR_VERSION_ARB, minor_version,
		None
	};

	dpy = XOpenDisplay(0);
	fbcount = 0;
	fbc = NULL;

	/* open display */
	if ( ! (dpy = XOpenDisplay(0)) )
	{
		fprintf(stderr, "Failed to open display\n");
		exit(1);
	}

	/* get framebuffer configs, any is usable (might want to add proper attribs) */
	if ( !(fbc = glXChooseFBConfig(dpy, DefaultScreen(dpy), visual_attribs, &fbcount) ) )
	{
		fprintf(stderr, "Failed to get FBConfig\n");
		exit(1);
	}

	/* get the required extensions */
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB");
	glXMakeContextCurrentARB = (glXMakeContextCurrentARBProc)glXGetProcAddressARB( (const GLubyte *) "glXMakeContextCurrent");
	glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC)glXGetProcAddressARB( (const GLubyte *) "glProgramParameteri");
	glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC)glXGetProcAddressARB( (const GLubyte *) "glGetProgramBinary");
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)glXGetProcAddressARB( (const GLubyte *) "glGenFramebuffers");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)glXGetProcAddressARB( (const GLubyte *) "glBindFramebuffer");
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)glXGetProcAddressARB( (const GLubyte *) "glGenRenderbuffers");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)glXGetProcAddressARB( (const GLubyte *) "glBindRenderbuffer");
	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)glXGetProcAddressARB( (const GLubyte *) "glRenderbufferStorage");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)glXGetProcAddressARB( (const GLubyte *) "glFramebufferRenderbuffer");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)glXGetProcAddressARB( (const GLubyte *) "glCheckFramebufferStatus");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)glXGetProcAddressARB( (const GLubyte *) "glGetShaderiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)glXGetProcAddressARB( (const GLubyte *) "glGetShaderInfoLog");
	glCreateShader = (PFNGLCREATESHADERPROC)glXGetProcAddressARB( (const GLubyte *) "glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)glXGetProcAddressARB( (const GLubyte *) "glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)glXGetProcAddressARB( (const GLubyte *) "glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)glXGetProcAddressARB( (const GLubyte *) "glCreateProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC)glXGetProcAddressARB( (const GLubyte *) "glAttachShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)glXGetProcAddressARB( (const GLubyte *) "glLinkProgram");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)glXGetProcAddressARB( (const GLubyte *) "glGetProgramiv");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)glXGetProcAddressARB( (const GLubyte *) "glGetProgramInfoLog");
	glUseProgram = (PFNGLUSEPROGRAMPROC)glXGetProcAddressARB( (const GLubyte *) "glUseProgram");

	if ( !(glXCreateContextAttribsARB && glXMakeContextCurrentARB))
	{
		fprintf(stderr, "missing support for GLX_ARB_create_context\n");
		XFree(fbc);
		exit(1);
	}

	if ( !(glProgramParameteri && glGetProgramBinary))
	{
		fprintf(stderr, "missing support for GLX_ARB_get_program_binary\n");
		XFree(fbc);
		exit(1);
	}

	/* create a context using glXCreateContextAttribsARB */
	if ( !( ctx = glXCreateContextAttribsARB(dpy, fbc[0], 0, True, context_attribs)) )
	{
		fprintf(stderr, "Failed to create opengl context\n");
		XFree(fbc);
		exit(1);
	}

	/* create temporary pbuffer */
	int pbuffer_attribs[] = 
	{
		GLX_PBUFFER_WIDTH, pbuffer_width,
		GLX_PBUFFER_HEIGHT, pbuffer_height,
		None
	};
	pbuf = glXCreatePbuffer(dpy, fbc[0], pbuffer_attribs);

	XFree(fbc);
	XSync(dpy, False);

	/* try to make it the current context */
	if ( !glXMakeContextCurrent(dpy, pbuf, pbuf, ctx) )
	{
		/* some drivers does not support context without default framebuffer, so fallback on
		* using the default window.
		*/
		if ( !glXMakeContextCurrent(dpy, DefaultRootWindow(dpy), DefaultRootWindow(dpy), ctx) )
		{
			fprintf(stderr, "failed to make current\n");
			exit(1);
		}
	} 
	else
	{
		printf("GL context init completed.\n");
		printf("\tGL Vendor: %s\n", (const char*)glGetString(GL_VENDOR));
		printf("\tGL Renderer : %s\n", (const char*)glGetString(GL_RENDERER));
		printf("\tGL Version (string) : %s\n", (const char*)glGetString(GL_VERSION));
		printf("\tGLSL Version : %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
		return 1;
	}
	return 0;
}


static void gl_context_init_buffer()
{
	int framebuffer_width = 1024;
	int framebuffer_height = 768;

	glGenFramebuffers(1, &m_framebuffer1);
	glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer1);

	glGenRenderbuffers(1, &m_colorRenderbuffer1);
	glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderbuffer1);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, framebuffer_width, framebuffer_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorRenderbuffer1);

	glGenRenderbuffers(1, &m_depthRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebuffer_width, framebuffer_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("Failed to make complete framebuffer object %x\n",glCheckFramebufferStatus(GL_FRAMEBUFFER));
}


static int gl_binary_is_elf(char *buf)
{
	char ELF_MAGIC[4] = { 0x7F, 0x45, 0x4C, 0x46 };
	
	return !memcmp(buf, ELF_MAGIC, sizeof(ELF_MAGIC));
}


static void gl_shader_binary_analyze_inner_elf(char *file_name)
{
	struct elf_file_t *elf_file;
	struct elf_section_t *section;

	char file_name_dest[MAX_STRING_SIZE];

	int index;
	FILE *f;

	/* Open File */
	elf_file = elf_file_create_from_path(file_name);

	/* Explore sections */
	LIST_FOR_EACH(elf_file->section_list, index)
	{
		/* Get section */
		section = list_get(elf_file->section_list, index);
		if (!*section->name)
			continue;

		/* Open file to dump section */
		snprintf(file_name_dest, sizeof file_name_dest, "%s.%d%s",
			file_name, index, section->name);
		f = fopen(file_name_dest, "wb");
		if (!f)
			fatal("%s: cannot open file '%s'",
				__FUNCTION__, file_name_dest);

		/* Dump section and close file */
		elf_buffer_dump(&section->buffer, f);
		fclose(f);

		/* Info */
		printf("\t  %s: inner section '%s' dumped\n",
			file_name_dest, section->name);
	}

	/* Close ELF file */
	elf_file_free(elf_file);
}


static void gl_shader_binary_analyze(char *file_name)
{
	char ELF_MAGIC[4] = { 0x7F, 0x45, 0x4C, 0x46 };
	char file_name_dest[MAX_STRING_SIZE];
	char *file_buffer;
	int file_size;
	int index;
	int i;
	int count = 0;

	FILE *f;
	void *tmp_buf;
	struct elf_file_t *elf_file;

	char file_name_prefix[MAX_STRING_SIZE];
	char subdir[MAX_STRING_SIZE];
	int len;

	/* Get file name prefix */
	strcpy(file_name_prefix, file_name);
	len = strlen(file_name);
	if (len > 4 && !strcmp(file_name + len - 4, ".bin"))
		file_name_prefix[len - 4] = '\0';

	/* Create subdirectory */
	snprintf(subdir, sizeof subdir, "%s_files", file_name_prefix);
	mkdir(subdir, 0755);

	file_buffer = (char *) read_buffer((char *) file_name, &file_size);
	/* Shader binary may have different format */
	if (gl_binary_is_elf(file_buffer))
	{
		/* Read ELF file */
		elf_file = elf_file_create_from_path(file_name);
		if (!elf_file)
			fatal("%s: cannot open ELF file", file_name);

		/* List ELF sections */
		printf("ELF sections:\n");
		LIST_FOR_EACH(elf_file->section_list, index)
		{
			struct elf_section_t *section;

			/* Get section */
			section = list_get(elf_file->section_list, index);
			if (!section->header->sh_size)
				continue;

			/* Dump to file */
			snprintf(file_name_dest, sizeof file_name_dest, "%s/%s.%s",
				subdir, file_name_prefix,
				*section->name == '.' ? section->name + 1 : section->name);
			f = fopen(file_name_dest, "wb");
			if (!f)
				fatal("%s: cannot open file '%s'", __FUNCTION__,
					file_name_dest);
			
			/* Dump section and close file */
			elf_buffer_dump(&section->buffer, f);
			fclose(f);

			/* Info */
			printf("  section '%s': addr=0x%x, offset=0x%x, size=%d, flags=0x%x\n",
				section->name, section->header->sh_addr, section->header->sh_offset,
				section->header->sh_size, section->header->sh_flags);
		}

		/* Get symbols */
		LIST_FOR_EACH(elf_file->symbol_table, index)
		{
			struct elf_symbol_t *symbol;
			struct elf_section_t *section;

			char shader_func_name[MAX_STRING_SIZE];
			int shader_func_len;

			int symbol_length;

			size_t size;

			/* Get symbol */
			symbol = list_get(elf_file->symbol_table, index);
			if (strncmp(symbol->name, "__Shader_", 9))
				continue;
			symbol_length = strlen(symbol->name);
			if (!symbol_length)
				continue;

			/* Get shader function name */
			shader_func_len = symbol_length - 9;
			strncpy(shader_func_name, symbol->name + 9, shader_func_len);
			shader_func_name[shader_func_len] = '\0';

			/* Read section */
			section = list_get(elf_file->section_list, symbol->section);
			assert(section && symbol->value + symbol->size
				<= section->header->sh_size);

			/* Open file name */
			snprintf(file_name_dest, sizeof file_name_dest, "%s/%s.%s",
				subdir, file_name_prefix, shader_func_name);
			f = fopen(file_name_dest, "wb");
			if (!f)
				fatal("%s: cannot create file '%s'", __FUNCTION__,
					file_name_dest);

			/* Dump buffer */
			size = fwrite(section->buffer.ptr + symbol->value, 1, symbol->size, f);
			if (size != symbol->size)
				fatal("%s: cannot dump '%s' symbol contents", __FUNCTION__,
					symbol->name);

			/* Close output file */
			printf("\t%s: data dumped\n", file_name_dest);
			fclose(f);

			if (str_suffix(symbol->name, "ElfBinary_0_"))
				gl_shader_binary_analyze_inner_elf(file_name_dest);
		}
	}
	else
	{
		/* Not standard ELF, linear search and dump */
		for (i = 0; i < file_size - sizeof(ELF_MAGIC); ++i)
		{
			if (memcmp(file_buffer, ELF_MAGIC, sizeof(ELF_MAGIC)) == 0)
			{
				printf("Find ELF Magic at position %d\n", i);
				sprintf(file_name_dest, "%s.elf.%d", file_name,
					count);
				count++;
				tmp_buf = (void *) xmalloc(file_size - i);
				memcpy(tmp_buf, file_buffer, file_size - i);
				f = fopen(file_name_dest, "wb");
				fwrite(tmp_buf, file_size - i, 1, f);
				free(tmp_buf);
				fclose(f);
			}
			file_buffer++;
		}
	}
}


static void gl_check_compile_error(GLuint id)
{
	char *log;
	GLint result;
	GLint logLen;
	GLsizei written;

	/* Check compilation errors */
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		fprintf(stderr, "Shader compilation failed!\n");
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			log = (char *) xmalloc(logLen);
			glGetShaderInfoLog(id, logLen, &written, log);
			fprintf(stderr, "Shader log:\n%s", log);
			free(log);
			exit(1);
		}
	}
}


static GLuint gl_compile_shader(const GLchar *source, GLenum type, GLint size)
{
	GLuint id;

	/* Create shader */
	id = glCreateShader(type);
	if (!id)
		fatal("cannot create shader");

	/* Compile */
	glShaderSource(id, 1, &source, &size);

	glCompileShader(id);
	gl_check_compile_error(id);

	/* Return shader ID */
	return id;
}

static int gl_shader_binary_generator(struct list_t *source_list, char *outputfile)
{
	int i;

	GLuint shader_id;
	struct gl_shader_source *shader_source;

	GLint status;
	GLint formats;
	GLint logLen;
	GLsizei written;
	void *bin_buffer;
	GLint bin_length;
	GLint *bin_formats;

	/* Create GL program */
	GLuint glprogram = glCreateProgram();

	if (glprogram == 0)
	{
		fprintf(stderr, "Error creating program object.\n");
		exit(1);
	}

	/* Compile and attach shaders */
	LIST_FOR_EACH(source_list, i)
	{
		shader_source = list_get(source_list, i);
		if (shader_source)
		{
			shader_id = gl_compile_shader(shader_source->source, 
				shader_source->shader_type,
				shader_source->size);
			glAttachShader(glprogram, shader_id);
		}
	}


	/* Link */
	glProgramParameteri(glprogram, PROGRAM_BINARY_RETRIEVABLE_HINT,
		GL_TRUE);
	glLinkProgram(glprogram);

	glGetProgramiv(glprogram, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		fprintf(stderr, "Failed to link shader program!\n");
		glGetProgramiv(glprogram, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			char *log = (char *) xmalloc(logLen);
			glGetProgramInfoLog(glprogram, logLen, &written, log);
			fprintf(stderr, "Program log: \n%s", log);
			free(log);
			exit(1);
		}
	}
	else
		glUseProgram(glprogram);

	/* Dump binary */
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
	bin_formats = xcalloc(formats, sizeof(GLint));
	glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, bin_formats);
	for (i = 0; i < formats; i++)
		printf("Shader format[%d] = 0x%x\n", i, bin_formats[i]);

	glGetProgramiv(glprogram, GL_PROGRAM_BINARY_LENGTH, &bin_length);
	printf("Shader binary size = %d bytes\n", bin_length);

	bin_buffer = xmalloc(bin_length);
	glGetProgramBinary(glprogram, bin_length, NULL, (GLenum*)bin_formats, bin_buffer);
	write_buffer(outputfile, bin_buffer, bin_length);

	/* Process generated binary */
	if (gl_dump_all)
		gl_shader_binary_analyze(outputfile);

	/* Free */
	free(bin_formats);
	free(bin_buffer);
	return 0;
}

static char* gl_shader_file_read(const char* file_name, int *file_size)
{
	FILE* input ;
	long size;
	char *content;

	input = fopen(file_name, "rb");
	if(input == NULL) 
		return NULL;

	if(fseek(input, 0, SEEK_END) == -1) 
		return NULL;
	
	size = ftell(input);
	if(size == -1) 
		return NULL;
	if(fseek(input, 0, SEEK_SET) == -1) 
		return NULL;

	content = (char*) xcalloc(1, (size_t) size +1); 
	if(content == NULL) 
		return NULL;

	fread(content, 1, (size_t)size, input);
	if(ferror(input)) {
		free(content);
		return NULL;
	}

	fclose(input);
	content[size] = '\0';
	*file_size = size;
	return content;
}

static struct gl_shader_source *gl_shader_source_create(char *source_file, unsigned int shader_type)
{
	struct gl_shader_source *shdr_src;
	int source_size;

	/* Initialize */
	if (source_file)
	{
		/* Allocate */
		shdr_src = xcalloc(1, sizeof(struct gl_shader_source));
	
		shdr_src->source = gl_shader_file_read(source_file, &source_size);
		if (!shdr_src->source)
			fatal("Error loading shader source!");
		shdr_src->shader_type = shader_type;
		shdr_src->size = source_size;

		/* Return */
		return shdr_src;	
	}
	else
		return NULL;
}

static void gl_shader_source_free(struct gl_shader_source *shdr_src)
{
	/* Free */
	free(shdr_src->source);
	free(shdr_src);
}

static struct list_t *gl_shader_source_list_create()
{
	struct list_t *lst;

	/* Allocate */
	lst = list_create();

	/* Return */
	return lst;
}

static void gl_shader_source_list_free(struct list_t *shdr_src_lst)
{
	struct gl_shader_source *shdr_src;
	int i;

	/* Free all shader source */
	LIST_FOR_EACH(shdr_src_lst, i)
	{
		shdr_src = list_get(shdr_src_lst, i);
		if (shdr_src)
			gl_shader_source_free(shdr_src);
	}

	list_free(shdr_src_lst);
}

static void gl_shader_source_list_add(struct list_t *shdr_src_lst, char *source_file, unsigned int shader_type)
{
	struct gl_shader_source *shdr_src;

	shdr_src = gl_shader_source_create(source_file, shader_type);

	if (shdr_src)
		list_add(shdr_src_lst, shdr_src);
}

/*
 * Public functions
 */

void gl_init(void)
{
}


void gl_done(void)
{
}


void gl_compile(struct list_t *source_file_list,	struct list_t *output_file_list)
{
	struct list_t *shdr_src_lst;
	char *output_file;

	/* Two sources needed for OpenGL */
	if (source_file_list->count != 2)
		fatal("2 source files expected, 1 for vertex, 1 for fragment shader");
	
	/* Extract vertex and fragment shader sources from 'source_file_list'. */
	gl_vertex_shader = list_get(source_file_list, 0);
	gl_fragment_shader = list_get(source_file_list, 1);

	/* Output file is given in the first element of 'output_file_list'. */
	assert(output_file_list->count == 2);
	output_file = list_get(output_file_list, 0);
	shdr_src_lst = gl_shader_source_list_create();

	/* Create Shaders and add to shader source list */
	gl_shader_source_list_add(shdr_src_lst, gl_vertex_shader, GL_VERTEX_SHADER);
	gl_shader_source_list_add(shdr_src_lst, gl_control_shader, GL_TESS_CONTROL_SHADER);
	gl_shader_source_list_add(shdr_src_lst, gl_eval_shader, GL_TESS_EVALUATION_SHADER);
	gl_shader_source_list_add(shdr_src_lst, gl_geo_shader, GL_GEOMETRY_SHADER);
	gl_shader_source_list_add(shdr_src_lst, gl_fragment_shader, GL_FRAGMENT_SHADER);

	/* Initialize OpenGL context */
	if (gl_context_init_x(4, 2))
		gl_context_init_buffer();

	/* Compile */
	gl_shader_binary_generator(shdr_src_lst, output_file);
	
	/* Dump binary information */
	if (gl_dump_all)
		gl_shader_binary_analyze(output_file);

	/* Free */
	gl_shader_source_list_free(shdr_src_lst);
}

