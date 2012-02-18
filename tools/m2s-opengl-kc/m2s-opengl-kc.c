#include <stdio.h>
#include <unistd.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include "misc.h"

#define		MAX_STRING_SIZE  200

#define		PROGRAM_BINARY_RETRIEVABLE_HINT 	0x8257

static char *syntax =
	"Syntax: %s [<options>] <Vertex/Fragment shader source file>\n"
	"\tOptions:\n"
	"\t-a           Dump intermediate ELF files\n"
	"\t-v           Vertex shader source code\n"
	"\t-f           Fragment shader source code\n"
	"\t-e           Evaluation shader source code\n"
	"\t-g           Geometry shader source code\n"
	"\t-c           Control shader source code\n"
	"\t-o           Output binary file name\n";


int dump_intermediate = 0;  /* Dump intermediate files */

int InitGLContext(int argc, char *argv[])
{
	// Init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitContextVersion(4,1);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutCreateWindow("OpenGL offline compiler");

	// Platform & OpenGL infomation
	const GLubyte *renderer = glGetString( GL_RENDERER );
	const GLubyte *vendor = glGetString( GL_VENDOR );
	const GLubyte *version = glGetString( GL_VERSION );
	const GLubyte *glslVersion = glGetString( GL_SHADING_LANGUAGE_VERSION );
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
	
	// Check if the ARB_get_program_binary is supported
	if (!GLEW_ARB_get_program_binary)
	{
		fprintf(stderr, "Need ARB_get_program_binary support!\n");
		return -1;
	}

	return 0;
}

void kernel_binary_analyze(const char *file_name)
{
	const char ELF_magic[4]={0x7F,0x45,0x4C,0x46};
	char file_name_dest[MAX_STRING_SIZE];
	// read shader binary file into a buffer
	char *	file_buffer = NULL;
	size_t	file_size;
	file_buffer = (char*)read_buffer((char*)file_name, &file_size);

	// find ELF magic in shader binary and dump 
	int i;
	int count = 0;
	for (i = 0; i < file_size - sizeof(ELF_magic); ++i)
	{
		if (memcmp(file_buffer,ELF_magic,sizeof(ELF_magic)) == 0)
		{
			printf("Find ELF Magic in position %d\n", i);
			sprintf(file_name_dest, "%s.elf.%d", file_name, count);
			count++;

			void* tmp_buf = (void*)malloc(file_size-i);
			memcpy(tmp_buf, file_buffer, file_size-i);

			FILE *f;
			f = fopen(file_name_dest, "wb");
			fwrite(tmp_buf, file_size-i, 1, f);
			free(tmp_buf);
			fclose(f);	
		}
		file_buffer++;

	}
}

int MainGetBinary(const GLchar** vertexSource, const GLchar** fragmentSource, const GLchar** controlSource, const GLchar** evaluationSource, const GLchar** geometrySource, const char* outputfile)
{
	GLint result;

	if ( !vertexSource || !fragmentSource)
	{
		return -1;
	}

	// Compile Vertex Shader
	GLuint vertShader = glCreateShader( GL_VERTEX_SHADER );
	if( 0 == vertShader )
	{
		fprintf(stderr, "Error creating vertex shader.\n");
		exit(1);
	}

	// printf("%s\n", vertexSource[0]);
	glShaderSource( vertShader, 1, vertexSource, NULL );

	glCompileShader( vertShader );
	
	glGetShaderiv( vertShader, GL_COMPILE_STATUS, &result );
	if( GL_FALSE == result )
	{
		fprintf( stderr, "Vertex shader compilation failed!\n" );
		GLint logLen;
		glGetShaderiv( vertShader, GL_INFO_LOG_LENGTH, &logLen );
		if( logLen > 0 )
		{
			char * log = (char *)malloc(logLen);
			GLsizei written;
			glGetShaderInfoLog(vertShader, logLen, &written, log);
			fprintf(stderr, "Shader log:\n%s", log);
			free(log);
			exit(1);		
		}
	}

	GLuint ctrlShader = 0;
	GLuint evalShader = 0;
	GLuint geomShader = 0;

	// Compile Control Shader
	if (controlSource)
	{
		ctrlShader = glCreateShader(GL_TESS_CONTROL_SHADER);

		if( 0 == ctrlShader )
		{
			fprintf(stderr, "Error creating control shader.\n");
			exit(1);
		}
		glShaderSource( ctrlShader, 1, controlSource, NULL );

		glCompileShader( ctrlShader );
		
		glGetShaderiv( ctrlShader, GL_COMPILE_STATUS, &result );
		if( GL_FALSE == result )
		{
			fprintf( stderr, "Control shader compilation failed!\n" );
			GLint logLen;
			glGetShaderiv( ctrlShader, GL_INFO_LOG_LENGTH, &logLen );
			if( logLen > 0 )
			{
				char * log = (char *)malloc(logLen);
				GLsizei written;
				glGetShaderInfoLog(ctrlShader, logLen, &written, log);
				fprintf(stderr, "Shader log:\n%s", log);
				free(log);
				exit(1);		
			}
		}
	}

	// Compile Evaluation Shader
	if (evaluationSource)
	{
		evalShader = glCreateShader(GL_TESS_EVALUATION_SHADER);

		if( 0 == evalShader )
		{
			fprintf(stderr, "Error creating evaluation shader.\n");
			exit(1);
		}
		glShaderSource( evalShader, 1, evaluationSource, NULL );

		glCompileShader( evalShader );
		
		glGetShaderiv( evalShader, GL_COMPILE_STATUS, &result );
		if( GL_FALSE == result )
		{
			fprintf( stderr, "Evaluation shader compilation failed!\n" );
			GLint logLen;
			glGetShaderiv( evalShader, GL_INFO_LOG_LENGTH, &logLen );
			if( logLen > 0 )
			{
				char * log = (char *)malloc(logLen);
				GLsizei written;
				glGetShaderInfoLog(evalShader, logLen, &written, log);
				fprintf(stderr, "Shader log:\n%s", log);
				free(log);
				exit(1);		
			}
		}
	}

	// Compile Geometry Shader
	if (geometrySource)
	{
		geomShader = glCreateShader(GL_GEOMETRY_SHADER);

		if( 0 == geomShader )
		{
			fprintf(stderr, "Error creating geometry shader.\n");
			exit(1);
		}
		glShaderSource( geomShader, 1, geometrySource, NULL );

		glCompileShader( geomShader );
		
		glGetShaderiv( geomShader, GL_COMPILE_STATUS, &result );
		if( GL_FALSE == result )
		{
			fprintf( stderr, "Geometry shader compilation failed!\n" );
			GLint logLen;
			glGetShaderiv( geomShader, GL_INFO_LOG_LENGTH, &logLen );
			if( logLen > 0 )
			{
				char * log = (char *)malloc(logLen);
				GLsizei written;
				glGetShaderInfoLog(geomShader, logLen, &written, log);
				fprintf(stderr, "Shader log:\n%s", log);
				free(log);
				exit(1);		
			}
		}
	}	

	// Compile Fragment Shader
	GLuint fragShader = glCreateShader( GL_FRAGMENT_SHADER );
	if( 0 == fragShader )
	{
		fprintf(stderr, "Error creating fragment shader.\n");
		exit(1);
	}

	glShaderSource( fragShader, 1, fragmentSource, NULL );

	glCompileShader( fragShader );
	
	glGetShaderiv( fragShader, GL_COMPILE_STATUS, &result );
	if( GL_FALSE == result )
	{
		fprintf( stderr, "Fragment shader compilation failed!\n" );
		GLint logLen;
		glGetShaderiv( fragShader, GL_INFO_LOG_LENGTH, &logLen );
		if( logLen > 0 )
		{
			char * log = (char *)malloc(logLen);
			GLsizei written;
			glGetShaderInfoLog(fragShader, logLen, &written, log);
			fprintf(stderr, "Shader log:\n%s", log);
			free(log);
			exit(1);
		}
	}

	// Create GL program
	GLuint glprogram = glCreateProgram();
	if( 0 == glprogram )
	{
		fprintf(stderr, "Error creating program object.\n");
		exit(1);
	}

	// Attach Vert&Frag shader to program
	glAttachShader(glprogram,vertShader);

	if (ctrlShader)
	{
		glAttachShader(glprogram, ctrlShader);
	}

	if (evalShader)
	{
		glAttachShader(glprogram, evalShader);
	}

	if (geomShader)
	{
		glAttachShader(glprogram, geomShader);
	}		

	glAttachShader(glprogram,fragShader);

	// Link
	glProgramParameteri(glprogram, PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
	glLinkProgram( glprogram );
	GLint status;
	glGetProgramiv( glprogram, GL_LINK_STATUS, &status );

	if( GL_FALSE == status ) 
	{
		fprintf( stderr, "Failed to link shader program!\n" );
		GLint logLen;
		glGetProgramiv(glprogram, GL_INFO_LOG_LENGTH, &logLen);
		if( logLen > 0 )
		{
			char * log = (char *)malloc(logLen);
			GLsizei written;
			glGetProgramInfoLog(glprogram, logLen, &written, log);
			fprintf(stderr, "Program log: \n%s", log);
			free(log);
			exit(1);
		}
	}
	else
	{
		glUseProgram( glprogram );
	}

	// Dump binary
	GLint formats = 0;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
	GLint *binaryFormats = calloc(formats, sizeof(GLint));
	glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, binaryFormats);
	int i;
	for (i = 0; i < formats; i++)
		printf("Format[%d]=%d\n", i, binaryFormats[i]);

	GLint bin_length=0;
	void *bin_buffer;

	glGetProgramiv(glprogram, GL_PROGRAM_BINARY_LENGTH, &bin_length);
	printf("Shader binary has %d bytes\n", bin_length);
	bin_buffer = malloc(bin_length);
	
	GLsizei ret_bin_length=0;
	GLenum ret_bin_fmt=0;
	glGetProgramBinary(glprogram, bin_length, &ret_bin_length, &ret_bin_fmt, bin_buffer);
	printf("Binary length = %d\n", ret_bin_length);
	printf("Binary format = %d\n", ret_bin_fmt);

	FILE *f;
	f = fopen(outputfile, "wb");
	fwrite(bin_buffer, bin_length, 1, f);
	fclose(f);

	/* Process generated binary */
	if (dump_intermediate)
		kernel_binary_analyze(outputfile);
	
	free(bin_buffer);
	return 0;
}

int main(int argc, char *argv[])
{
	// printf("%s\n", ELF_magic);
	/* No arguments */
	if (argc == 1) {
		fprintf(stderr, syntax, argv[0]);
		return 1;
	}

	char *vsfile = NULL;
	char *fsfile = NULL;
	char *ctfile = NULL;
	char *evfile = NULL;
	char *gmfile = NULL;
	char *output = "shader.bin";

	int argi;
	for (argi = 1; argi < argc; argi++)
	{
		//  
		if (!strcmp(argv[argi], "-a")) {
			argi++;
			dump_intermediate = 1;
			continue;
		}			
		// vertex source
		if (!strcmp(argv[argi], "-v")) {
			argi++;
			vsfile = argv[argi];
			continue;
		}		

		// fragment source
		if (!strcmp(argv[argi], "-f")) {
			argi++;
			fsfile = argv[argi];
			continue;
		}

		// control source
		if (!strcmp(argv[argi], "-c")) {
			argi++;
			ctfile = argv[argi];
			continue;
		}

		// evaluation source
		if (!strcmp(argv[argi], "-e")) {
			argi++;
			evfile = argv[argi];
			continue;
		}

		// geometry source
		if (!strcmp(argv[argi], "-g")) {
			argi++;
			gmfile = argv[argi];
			continue;
		}

		// binary output name
		if (!strcmp(argv[argi], "-o")) {
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

	if (InitGLContext(argc, argv))
	{
		printf("Initilize Context Failed !\n");
		return -1;
	}

	char *VSShaderSource = NULL;
	char *FSShaderSource = NULL;
	char *CTShaderSource = NULL;
	char *EVShaderSource = NULL;
	char *GMShaderSource = NULL;
	size_t VS_source_size;
	size_t FS_source_size;
	size_t CT_source_size;
	size_t EV_source_size;
	size_t GM_source_size;

	VSShaderSource = (GLchar*)read_buffer(vsfile, &VS_source_size);
	FSShaderSource = (GLchar*)read_buffer(fsfile, &FS_source_size);

	const GLchar** CTShaderArray = NULL;
	const GLchar** EVShaderArray = NULL;
	const GLchar** GMShaderArray = NULL;

	if (ctfile != NULL)
	{
		CTShaderSource = (GLchar*)read_buffer(ctfile, &CT_source_size);
		CTShaderArray = malloc(CT_source_size);
		CTShaderArray[0] = CTShaderSource;
	}

	if (evfile != NULL)
	{
		EVShaderSource = (GLchar*)read_buffer(evfile, &EV_source_size);
		EVShaderArray = malloc(EV_source_size);
		EVShaderArray[0] = EVShaderSource;
	}

	if (gmfile != NULL)
	{
		GMShaderSource = (GLchar*)read_buffer(gmfile, &GM_source_size);
		GMShaderArray = malloc(GM_source_size);
		GMShaderArray[0] = GMShaderSource;
	}

	const GLchar* VSShaderArray[] = {VSShaderSource};
	const GLchar* FSShaderArray[] = {FSShaderSource};

	MainGetBinary(VSShaderArray, FSShaderArray, CTShaderArray, EVShaderArray, GMShaderArray, output);

	return 0;
}
