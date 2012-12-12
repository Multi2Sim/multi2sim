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

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glu.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <lib/mhandle/mhandle.h>
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

int opengl_context_init(int argc, char *argv[])
{
	/* Init */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitContextVersion(4, 1);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutCreateWindow("OpenGL offline compiler");

	/* Platform & OpenGL infomation */
	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *vendor = glGetString(GL_VENDOR);
	const GLubyte *version = glGetString(GL_VERSION);
	const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	GLint major, minor;

	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);

	printf("GL Vendor: %s\n", vendor);
	printf("GL Renderer : %s\n", renderer);
	printf("GL Version (string) : %s\n", version);
	printf("GL Version (integer) : %d.%d\n", major, minor);
	printf("GLSL Version : %s\n", glslVersion);

	GLenum err = glewInit();

	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return -1;
	}

	/* Check if the ARB_get_program_binary is supported */
	if (!GLEW_ARB_get_program_binary)
	{
		fprintf(stderr, "Need ARB_get_program_binary support!\n");
		return -1;
	}

	return 0;
}

void opengl_shader_binary_analyze(const char *file_name)
{
	char ELF_MAGIC[4] = { 0x7F, 0x45, 0x4C, 0x46 };
	char file_name_dest[MAX_STRING_SIZE];
	char *file_buffer;

	int file_size;
	int i;
	int count = 0;

	FILE *f;
	void *tmp_buf;

	file_buffer = (char *) read_buffer((char *) file_name, &file_size);

	/* Find ELF magic in shader binary and dump */
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
	const GLchar ** opengl_geometry_source, const char *outputfile)
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
		printf("Error, make sure there is at least 1 shader source\n");
		exit(-1);
	}

	if (opengl_context_init(argc, argv))
	{
		printf("Initilize OpenGL Context Failed !\n");
		return -1;
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
