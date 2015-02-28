lib_LIBRARIES = libsi_gl.a

if HAVE_OPENGL

libsi_gl_a_SOURCES = \
	ABI.cc \
	ABI.h \
	\
	DepthBuffer.cc \
	DepthBuffer.h \
	\
	FetchShader.cc \
	FetchShader.h \
	\
	Input.cc \
	Input.h \
	\
	PrimitiveAssembler.cc \
	PrimitiveAssembler.h \
	\
	Program.cc \
	Program.h \
	\
	ScanConverter.cc \
	ScanConverter.h \
	\
	Shader.cc \
	Shader.h \
	\
	ShaderExport.cc \
	ShaderExport.h \
	\
	SPI.cc \
	SPI.h 

else

libsi_gl_a_SOURCES = 


endif

AM_CPPFLAGS = @M2S_INCLUDES@
