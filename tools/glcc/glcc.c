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

#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>


#define PROGRAM_BINARY_RETRIEVABLE_HINT  0x8257

static char *syntax =
	"Syntax: %s [<options>]\n"
	"\tOptions:\n"
	"\t-a\t\t\t\tDump intermediate ELF files\n"
	"\t-v  <shader source file>\tVertex shader source code\n"
	"\t-e  <shader source file>\tEvaluation shader source code\n"
	"\t-c  <shader source file>\tControl shader source code\n"
	"\t-g  <shader source file>\tGeometry shader source code\n"
	"\t-f  <shader source file>\tFragment shader source code\n"
	"\t-o\t\t\t\tOutput binary file name\n";


int dump_intermediate = 0;	/* Dump intermediate files */

/* For context initialization */
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
typedef Bool (*glXMakeContextCurrentARBProc)(Display*, GLXDrawable, GLXDrawable, GLXContext);
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLPROGRAMPARAMETERIPROC glProgramParameteri;
PFNGLGETPROGRAMBINARYPROC glGetProgramBinary;

int pbuffer_width = 32;
int pbuffer_height = 32;
int framebuffer_width = 1024;
int framebuffer_height = 768;

/* X & Context */
Display* dpy ;
int fbcount;
GLXFBConfig* fbc;
GLXContext ctx;
GLXPbuffer pbuf;

/* Buffer IDs */
GLuint m_framebuffer1;
GLuint m_colorRenderbuffer1;
GLuint m_depthRenderbuffer;

static glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
static glXMakeContextCurrentARBProc glXMakeContextCurrentARB = 0;

int opengl_context_init_x(unsigned int major_version, unsigned int minor_version)
{
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
		printf("GL Vendor: %s\n", (const char*)glGetString(GL_VENDOR));
		printf("GL Renderer : %s\n", (const char*)glGetString(GL_RENDERER));
		printf("GL Version (string) : %s\n", (const char*)glGetString(GL_VERSION));
		printf("GLSL Version : %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
		return 1;
	}
	return 0;
}

void opengl_context_init_buffer()
{
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
	else
		printf("Success, finally did it!\n");
}

/* For read shaders */
void *read_buffer(char *file_name, int *psize)
{
	FILE *f;
	void *buf;
	int size;

	f = fopen(file_name, "rb");
	if (!f)
		return NULL;
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);

	buf = xmalloc(size + 1);
	memset(buf, 0, size + 1);
	fread(buf, 1, size, f);
	if (psize)
		*psize = size;
	return buf;
}

int opengl_binary_is_elf(char *buf)
{
	char ELF_MAGIC[4] = { 0x7F, 0x45, 0x4C, 0x46 };
	if (!memcmp(buf, ELF_MAGIC, sizeof(ELF_MAGIC)))
		return 1;
	else
		return 0;
}

void opengl_shader_binary_analyze_inner_elf(char *file_name)
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

void opengl_shader_binary_analyze(char *file_name)
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
	if (opengl_binary_is_elf(file_buffer))
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
				opengl_shader_binary_analyze_inner_elf(file_name_dest);
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

void opengl_check_compile_error(GLuint opengl_shader_id)
{
	char *log;
	GLint result;
	GLint logLen;
	GLsizei written;

	/* Check compilation errors */
	glGetShaderiv(opengl_shader_id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		fprintf(stderr, "Shader compilation failed!\n");

		glGetShaderiv(opengl_shader_id, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			log = (char *) xmalloc(logLen);
			glGetShaderInfoLog(opengl_shader_id, logLen, &written,
				log);
			fprintf(stderr, "Shader log:\n%s", log);
			free(log);
			exit(1);
		}
	}
}

GLuint opengl_compile_shader(const GLchar ** opengl_shader_source,
	GLenum opengl_shader_type)
{
	GLuint opengl_shader_id;

	opengl_shader_id = glCreateShader(opengl_shader_type);
	if (opengl_shader_id == 0)
	{
		fprintf(stderr, "Error creating shader.\n");
		exit(1);
	}
	glShaderSource(opengl_shader_id, 1, opengl_shader_source, NULL);
	glCompileShader(opengl_shader_id);
	opengl_check_compile_error(opengl_shader_id);
	return opengl_shader_id;
}

int opengl_shader_binary_generator(const GLchar ** opengl_vertex_source,
	const GLchar ** opengl_fragment_source,
	const GLchar ** opengl_tessellation_control_source,
	const GLchar ** opengl_tessellation_evaluation_source,
	const GLchar ** opengl_geometry_source, char *outputfile)
{
	int i;
	void *bin_buffer;
	FILE *f;

	GLuint opengl_vertex_shader_id = 0;
	GLuint opengl_tessellation_control_shader_id = 0;
	GLuint opengl_tessellation_evaluation_shader_id = 0;
	GLuint opengl_geometry_shader_id = 0;
	GLuint opengl_fragment_shader_id = 0;

	GLint status;
	GLint formats;
	GLint logLen;
	GLsizei written;
	GLint bin_length;

	if (!opengl_vertex_source || !opengl_fragment_source)
		return -1;

	/* Compile shaders */
	opengl_vertex_shader_id =
		opengl_compile_shader(opengl_vertex_source, GL_VERTEX_SHADER);
	if (opengl_tessellation_control_source)
		opengl_tessellation_control_shader_id =
			opengl_compile_shader
			(opengl_tessellation_control_source,
			GL_TESS_CONTROL_SHADER);
	if (opengl_tessellation_evaluation_source)
		opengl_tessellation_evaluation_shader_id =
			opengl_compile_shader
			(opengl_tessellation_evaluation_source,
			GL_TESS_EVALUATION_SHADER);
	if (opengl_geometry_source)
		opengl_geometry_shader_id =
			opengl_compile_shader(opengl_geometry_source,
			GL_GEOMETRY_SHADER);
	opengl_fragment_shader_id =
		opengl_compile_shader(opengl_fragment_source,
		GL_FRAGMENT_SHADER);

	/* Create GL program */
	GLuint glprogram = glCreateProgram();

	if (glprogram == 0)
	{
		fprintf(stderr, "Error creating program object.\n");
		exit(1);
	}

	/* Attach shaders to program */
	glAttachShader(glprogram, opengl_vertex_shader_id);
	if (opengl_tessellation_control_shader_id != 0)
		glAttachShader(glprogram,
			opengl_tessellation_control_shader_id);
	if (opengl_tessellation_evaluation_shader_id != 0)
		glAttachShader(glprogram,
			opengl_tessellation_evaluation_shader_id);
	if (opengl_geometry_shader_id != 0)
		glAttachShader(glprogram, opengl_geometry_shader_id);
	glAttachShader(glprogram, opengl_fragment_shader_id);

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
	GLint *binaryFormats = xcalloc(formats, sizeof(GLint));

	glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, binaryFormats);
	for (i = 0; i < formats; i++)
		printf("Format[%d]=%d\n", i, binaryFormats[i]);

	glGetProgramiv(glprogram, GL_PROGRAM_BINARY_LENGTH, &bin_length);
	printf("Shader binary has %d bytes\n", bin_length);
	bin_buffer = xmalloc(bin_length);

	glGetProgramBinary(glprogram, bin_length, NULL, (GLenum*)binaryFormats, bin_buffer);

	f = fopen(outputfile, "wb");
	fwrite(bin_buffer, bin_length, 1, f);
	fclose(f);

	/* Process generated binary */
	if (dump_intermediate)
		opengl_shader_binary_analyze(outputfile);

	free(bin_buffer);
	return 0;
}

int main(int argc, char *argv[])
{
	int argi;
	int VS_source_size;
	int FS_source_size;
	int CT_source_size;
	int EV_source_size;
	int GM_source_size;

	char *vsfile = NULL;
	char *fsfile = NULL;
	char *ctfile = NULL;
	char *evfile = NULL;
	char *gmfile = NULL;
	char *output = "shader.bin";
	char *VSShaderSource = NULL;
	char *FSShaderSource = NULL;
	char *CTShaderSource = NULL;
	char *EVShaderSource = NULL;
	char *GMShaderSource = NULL;

	const GLchar **VSShaderArray = NULL;
	const GLchar **CTShaderArray = NULL;
	const GLchar **EVShaderArray = NULL;
	const GLchar **GMShaderArray = NULL;
	const GLchar **FSShaderArray = NULL;

	/* No arguments */
	if (argc == 1)
	{
		fprintf(stderr, syntax, argv[0]);
		return 1;
	}

	for (argi = 1; argi < argc; argi++)
	{
		/* Dump intermediate ELF files */
		if (!strcmp(argv[argi], "-a"))
		{
			argi++;
			dump_intermediate = 1;
			if (argc == 3 && dump_intermediate)
			{
				opengl_shader_binary_analyze(argv[argi]);
				exit(1);
			}	
			continue;
		}
		/* vertex source */
		if (!strcmp(argv[argi], "-v"))
		{
			argi++;
			vsfile = argv[argi];
			continue;
		}

		/* fragment source */
		if (!strcmp(argv[argi], "-f"))
		{
			argi++;
			fsfile = argv[argi];
			continue;
		}

		/* control source */
		if (!strcmp(argv[argi], "-c"))
		{
			argi++;
			ctfile = argv[argi];
			continue;
		}

		/* evaluation source */
		if (!strcmp(argv[argi], "-e"))
		{
			argi++;
			evfile = argv[argi];
			continue;
		}

		/* geometry source */
		if (!strcmp(argv[argi], "-g"))
		{
			argi++;
			gmfile = argv[argi];
			continue;
		}

		/* binary output name */
		if (!strcmp(argv[argi], "-o"))
		{
			argi++;
			output = argv[argi];
			continue;
		}
	}

	if (vsfile == NULL && fsfile == NULL)
	{
		printf("Error, need at least 1 vertex shader and 1 fragment shader\n");
		exit(-1);
	}

	if (opengl_context_init_x(4,2))
	{
		opengl_context_init_buffer();
	}

	VSShaderSource = (GLchar *) read_buffer(vsfile, &VS_source_size);
	VSShaderArray = xmalloc(VS_source_size);
	VSShaderArray[0] = VSShaderSource;

	FSShaderSource = (GLchar *) read_buffer(fsfile, &FS_source_size);
	FSShaderArray = xmalloc(FS_source_size);
	FSShaderArray[0] = FSShaderSource;

	if (ctfile != NULL)
	{
		CTShaderSource =
			(GLchar *) read_buffer(ctfile, &CT_source_size);
		CTShaderArray = xmalloc(CT_source_size);
		CTShaderArray[0] = CTShaderSource;
	}

	if (evfile != NULL)
	{
		EVShaderSource =
			(GLchar *) read_buffer(evfile, &EV_source_size);
		EVShaderArray = xmalloc(EV_source_size);
		EVShaderArray[0] = EVShaderSource;
	}

	if (gmfile != NULL)
	{
		GMShaderSource =
			(GLchar *) read_buffer(gmfile, &GM_source_size);
		GMShaderArray = xmalloc(GM_source_size);
		GMShaderArray[0] = GMShaderSource;
	}

	opengl_shader_binary_generator(VSShaderArray, FSShaderArray,
		CTShaderArray, EVShaderArray, GMShaderArray, output);

	/* Free */
	free(VSShaderArray);
	free(FSShaderArray);
	if (CTShaderArray != NULL)
		free(CTShaderArray);
	if (EVShaderArray != NULL)
		free(EVShaderArray);
	if (GMShaderArray != NULL)
		free(GMShaderArray);

	return 0;
}
