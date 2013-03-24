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


#ifndef RUNTIME_OPENGL_API_H
#define RUNTIME_OPENGL_API_H


/* Debug */
extern int opengl_debug_enable;

#define opengl_debug(stream, ...) (opengl_debug_enable ? fprintf((stream), __VA_ARGS__) : (void) 0)


/* System call for OpenGL runtime */
#define OPENGL_SYSCALL_CODE  327

struct opengl_runtime_info_t
{
	unsigned int version_major;
	unsigned int version_minor;
	unsigned int glsl_version;
	char vendor[20];
	char renderer[20];
};

extern struct opengl_runtime_info_t gl_runtime_info;

/* List of OpenGL runtime calls */
enum opengl_call_t
{
	opengl_call_invalid,

	opengl_call_glClearIndex, 
	opengl_call_glClearColor, 
	opengl_call_glClear, 
	opengl_call_glIndexMask, 
	opengl_call_glColorMask, 			/*5*/ 
	opengl_call_glAlphaFunc, 
	opengl_call_glBlendFunc, 
	opengl_call_glLogicOp, 
	opengl_call_glCullFace, 
	opengl_call_glFrontFace, 			/*10*/ 
	opengl_call_glPointSize, 
	opengl_call_glLineWidth, 
	opengl_call_glLineStipple, 
	opengl_call_glPolygonMode, 
	opengl_call_glPolygonOffset, 			/*15*/ 
	opengl_call_glPolygonStipple, 
	opengl_call_glGetPolygonStipple, 
	opengl_call_glEdgeFlag, 
	opengl_call_glEdgeFlagv, 
	opengl_call_glScissor, 			/*20*/ 
	opengl_call_glClipPlane, 
	opengl_call_glGetClipPlane, 
	opengl_call_glDrawBuffer, 
	opengl_call_glReadBuffer, 
	opengl_call_glEnable, 			/*25*/ 
	opengl_call_glDisable, 
	opengl_call_glIsEnabled, 
	opengl_call_glEnableClientState, 
	opengl_call_glDisableClientState, 
	opengl_call_glGetBooleanv, 			/*30*/ 
	opengl_call_glGetDoublev, 
	opengl_call_glGetFloatv, 
	opengl_call_glGetIntegerv, 
	opengl_call_glPushAttrib, 
	opengl_call_glPopAttrib, 			/*35*/ 
	opengl_call_glPushClientAttrib, 
	opengl_call_glPopClientAttrib, 
	opengl_call_glRenderMode, 
	opengl_call_glGetError, 
	opengl_call_glFinish, 			/*40*/ 
	opengl_call_glFlush, 
	opengl_call_glHint, 
	opengl_call_glClearDepth, 
	opengl_call_glDepthFunc, 
	opengl_call_glDepthMask, 			/*45*/ 
	opengl_call_glDepthRange, 
	opengl_call_glClearAccum, 
	opengl_call_glAccum, 
	opengl_call_glMatrixMode, 
	opengl_call_glOrtho, 			/*50*/ 
	opengl_call_glFrustum, 
	opengl_call_glViewport, 
	opengl_call_glPushMatrix, 
	opengl_call_glPopMatrix, 
	opengl_call_glLoadIdentity, 			/*55*/ 
	opengl_call_glLoadMatrixd, 
	opengl_call_glLoadMatrixf, 
	opengl_call_glMultMatrixd, 
	opengl_call_glMultMatrixf, 
	opengl_call_glRotated, 			/*60*/ 
	opengl_call_glRotatef, 
	opengl_call_glScaled, 
	opengl_call_glScalef, 
	opengl_call_glTranslated, 
	opengl_call_glTranslatef, 			/*65*/ 
	opengl_call_glIsList, 
	opengl_call_glDeleteLists, 
	opengl_call_glGenLists, 
	opengl_call_glNewList, 
	opengl_call_glEndList, 			/*70*/ 
	opengl_call_glCallList, 
	opengl_call_glCallLists, 
	opengl_call_glListBase, 
	opengl_call_glBegin, 
	opengl_call_glEnd, 			/*75*/ 
	opengl_call_glVertex2d, 
	opengl_call_glVertex2f, 
	opengl_call_glVertex2i, 
	opengl_call_glVertex2s, 
	opengl_call_glVertex3d, 			/*80*/ 
	opengl_call_glVertex3f, 
	opengl_call_glVertex3i, 
	opengl_call_glVertex3s, 
	opengl_call_glVertex4d, 
	opengl_call_glVertex4f, 			/*85*/ 
	opengl_call_glVertex4i, 
	opengl_call_glVertex4s, 
	opengl_call_glVertex2dv, 
	opengl_call_glVertex2fv, 
	opengl_call_glVertex2iv, 			/*90*/ 
	opengl_call_glVertex2sv, 
	opengl_call_glVertex3dv, 
	opengl_call_glVertex3fv, 
	opengl_call_glVertex3iv, 
	opengl_call_glVertex3sv, 			/*95*/ 
	opengl_call_glVertex4dv, 
	opengl_call_glVertex4fv, 
	opengl_call_glVertex4iv, 
	opengl_call_glVertex4sv, 
	opengl_call_glNormal3b, 			/*100*/ 
	opengl_call_glNormal3d, 
	opengl_call_glNormal3f, 
	opengl_call_glNormal3i, 
	opengl_call_glNormal3s, 
	opengl_call_glNormal3bv, 			/*105*/ 
	opengl_call_glNormal3dv, 
	opengl_call_glNormal3fv, 
	opengl_call_glNormal3iv, 
	opengl_call_glNormal3sv, 
	opengl_call_glIndexd, 			/*110*/ 
	opengl_call_glIndexf, 
	opengl_call_glIndexi, 
	opengl_call_glIndexs, 
	opengl_call_glIndexub, 
	opengl_call_glIndexdv, 			/*115*/ 
	opengl_call_glIndexfv, 
	opengl_call_glIndexiv, 
	opengl_call_glIndexsv, 
	opengl_call_glIndexubv, 
	opengl_call_glColor3b, 			/*120*/ 
	opengl_call_glColor3d, 
	opengl_call_glColor3f, 
	opengl_call_glColor3i, 
	opengl_call_glColor3s, 
	opengl_call_glColor3ub, 			/*125*/ 
	opengl_call_glColor3ui, 
	opengl_call_glColor3us, 
	opengl_call_glColor4b, 
	opengl_call_glColor4d, 
	opengl_call_glColor4f, 			/*130*/ 
	opengl_call_glColor4i, 
	opengl_call_glColor4s, 
	opengl_call_glColor4ub, 
	opengl_call_glColor4ui, 
	opengl_call_glColor4us, 			/*135*/ 
	opengl_call_glColor3bv, 
	opengl_call_glColor3dv, 
	opengl_call_glColor3fv, 
	opengl_call_glColor3iv, 
	opengl_call_glColor3sv, 			/*140*/ 
	opengl_call_glColor3ubv, 
	opengl_call_glColor3uiv, 
	opengl_call_glColor3usv, 
	opengl_call_glColor4bv, 
	opengl_call_glColor4dv, 			/*145*/ 
	opengl_call_glColor4fv, 
	opengl_call_glColor4iv, 
	opengl_call_glColor4sv, 
	opengl_call_glColor4ubv, 
	opengl_call_glColor4uiv, 			/*150*/ 
	opengl_call_glColor4usv, 
	opengl_call_glTexCoord1d, 
	opengl_call_glTexCoord1f, 
	opengl_call_glTexCoord1i, 
	opengl_call_glTexCoord1s, 			/*155*/ 
	opengl_call_glTexCoord2d, 
	opengl_call_glTexCoord2f, 
	opengl_call_glTexCoord2i, 
	opengl_call_glTexCoord2s, 
	opengl_call_glTexCoord3d, 			/*160*/ 
	opengl_call_glTexCoord3f, 
	opengl_call_glTexCoord3i, 
	opengl_call_glTexCoord3s, 
	opengl_call_glTexCoord4d, 
	opengl_call_glTexCoord4f, 			/*165*/ 
	opengl_call_glTexCoord4i, 
	opengl_call_glTexCoord4s, 
	opengl_call_glTexCoord1dv, 
	opengl_call_glTexCoord1fv, 
	opengl_call_glTexCoord1iv, 			/*170*/ 
	opengl_call_glTexCoord1sv, 
	opengl_call_glTexCoord2dv, 
	opengl_call_glTexCoord2fv, 
	opengl_call_glTexCoord2iv, 
	opengl_call_glTexCoord2sv, 			/*175*/ 
	opengl_call_glTexCoord3dv, 
	opengl_call_glTexCoord3fv, 
	opengl_call_glTexCoord3iv, 
	opengl_call_glTexCoord3sv, 
	opengl_call_glTexCoord4dv, 			/*180*/ 
	opengl_call_glTexCoord4fv, 
	opengl_call_glTexCoord4iv, 
	opengl_call_glTexCoord4sv, 
	opengl_call_glRasterPos2d, 
	opengl_call_glRasterPos2f, 			/*185*/ 
	opengl_call_glRasterPos2i, 
	opengl_call_glRasterPos2s, 
	opengl_call_glRasterPos3d, 
	opengl_call_glRasterPos3f, 
	opengl_call_glRasterPos3i, 			/*190*/ 
	opengl_call_glRasterPos3s, 
	opengl_call_glRasterPos4d, 
	opengl_call_glRasterPos4f, 
	opengl_call_glRasterPos4i, 
	opengl_call_glRasterPos4s, 			/*195*/ 
	opengl_call_glRasterPos2dv, 
	opengl_call_glRasterPos2fv, 
	opengl_call_glRasterPos2iv, 
	opengl_call_glRasterPos2sv, 
	opengl_call_glRasterPos3dv, 			/*200*/ 
	opengl_call_glRasterPos3fv, 
	opengl_call_glRasterPos3iv, 
	opengl_call_glRasterPos3sv, 
	opengl_call_glRasterPos4dv, 
	opengl_call_glRasterPos4fv, 			/*205*/ 
	opengl_call_glRasterPos4iv, 
	opengl_call_glRasterPos4sv, 
	opengl_call_glRectd, 
	opengl_call_glRectf, 
	opengl_call_glRecti, 			/*210*/ 
	opengl_call_glRects, 
	opengl_call_glRectdv, 
	opengl_call_glRectfv, 
	opengl_call_glRectiv, 
	opengl_call_glRectsv, 			/*215*/ 
	opengl_call_glVertexPointer, 
	opengl_call_glNormalPointer, 
	opengl_call_glColorPointer, 
	opengl_call_glIndexPointer, 
	opengl_call_glTexCoordPointer, 			/*220*/ 
	opengl_call_glEdgeFlagPointer, 
	opengl_call_glGetPointerv, 
	opengl_call_glArrayElement, 
	opengl_call_glDrawArrays, 
	opengl_call_glDrawElements, 			/*225*/ 
	opengl_call_glInterleavedArrays, 
	opengl_call_glShadeModel, 
	opengl_call_glLightf, 
	opengl_call_glLighti, 
	opengl_call_glLightfv, 			/*230*/ 
	opengl_call_glLightiv, 
	opengl_call_glGetLightfv, 
	opengl_call_glGetLightiv, 
	opengl_call_glLightModelf, 
	opengl_call_glLightModeli, 			/*235*/ 
	opengl_call_glLightModelfv, 
	opengl_call_glLightModeliv, 
	opengl_call_glMaterialf, 
	opengl_call_glMateriali, 
	opengl_call_glMaterialfv, 			/*240*/ 
	opengl_call_glMaterialiv, 
	opengl_call_glGetMaterialfv, 
	opengl_call_glGetMaterialiv, 
	opengl_call_glColorMaterial, 
	opengl_call_glPixelZoom, 			/*245*/ 
	opengl_call_glPixelStoref, 
	opengl_call_glPixelStorei, 
	opengl_call_glPixelTransferf, 
	opengl_call_glPixelTransferi, 
	opengl_call_glPixelMapfv, 			/*250*/ 
	opengl_call_glPixelMapuiv, 
	opengl_call_glPixelMapusv, 
	opengl_call_glGetPixelMapfv, 
	opengl_call_glGetPixelMapuiv, 
	opengl_call_glGetPixelMapusv, 			/*255*/ 
	opengl_call_glBitmap, 
	opengl_call_glReadPixels, 
	opengl_call_glDrawPixels, 
	opengl_call_glCopyPixels, 
	opengl_call_glStencilFunc, 			/*260*/ 
	opengl_call_glStencilMask, 
	opengl_call_glStencilOp, 
	opengl_call_glClearStencil, 
	opengl_call_glTexGend, 
	opengl_call_glTexGenf, 			/*265*/ 
	opengl_call_glTexGeni, 
	opengl_call_glTexGendv, 
	opengl_call_glTexGenfv, 
	opengl_call_glTexGeniv, 
	opengl_call_glGetTexGendv, 			/*270*/ 
	opengl_call_glGetTexGenfv, 
	opengl_call_glGetTexGeniv, 
	opengl_call_glTexEnvf, 
	opengl_call_glTexEnvi, 
	opengl_call_glTexEnvfv, 			/*275*/ 
	opengl_call_glTexEnviv, 
	opengl_call_glGetTexEnvfv, 
	opengl_call_glGetTexEnviv, 
	opengl_call_glTexParameterf, 
	opengl_call_glTexParameteri, 			/*280*/ 
	opengl_call_glTexParameterfv, 
	opengl_call_glTexParameteriv, 
	opengl_call_glGetTexParameterfv, 
	opengl_call_glGetTexParameteriv, 
	opengl_call_glGetTexLevelParameterfv, 			/*285*/ 
	opengl_call_glGetTexLevelParameteriv, 
	opengl_call_glTexImage1D, 
	opengl_call_glTexImage2D, 
	opengl_call_glGetTexImage, 
	opengl_call_glGenTextures, 			/*290*/ 
	opengl_call_glDeleteTextures, 
	opengl_call_glBindTexture, 
	opengl_call_glPrioritizeTextures, 
	opengl_call_glAreTexturesResident, 
	opengl_call_glIsTexture, 			/*295*/ 
	opengl_call_glTexSubImage1D, 
	opengl_call_glTexSubImage2D, 
	opengl_call_glCopyTexImage1D, 
	opengl_call_glCopyTexImage2D, 
	opengl_call_glCopyTexSubImage1D, 			/*300*/ 
	opengl_call_glCopyTexSubImage2D, 
	opengl_call_glMap1d, 
	opengl_call_glMap1f, 
	opengl_call_glMap2d, 
	opengl_call_glMap2f, 			/*305*/ 
	opengl_call_glGetMapdv, 
	opengl_call_glGetMapfv, 
	opengl_call_glGetMapiv, 
	opengl_call_glEvalCoord1d, 
	opengl_call_glEvalCoord1f, 			/*310*/ 
	opengl_call_glEvalCoord1dv, 
	opengl_call_glEvalCoord1fv, 
	opengl_call_glEvalCoord2d, 
	opengl_call_glEvalCoord2f, 
	opengl_call_glEvalCoord2dv, 			/*315*/ 
	opengl_call_glEvalCoord2fv, 
	opengl_call_glMapGrid1d, 
	opengl_call_glMapGrid1f, 
	opengl_call_glMapGrid2d, 
	opengl_call_glMapGrid2f, 			/*320*/ 
	opengl_call_glEvalPoint1, 
	opengl_call_glEvalPoint2, 
	opengl_call_glEvalMesh1, 
	opengl_call_glEvalMesh2, 
	opengl_call_glFogf, 			/*325*/ 
	opengl_call_glFogi, 
	opengl_call_glFogfv, 
	opengl_call_glFogiv, 
	opengl_call_glFeedbackBuffer, 
	opengl_call_glPassThrough, 			/*330*/ 
	opengl_call_glSelectBuffer, 
	opengl_call_glInitNames, 
	opengl_call_glLoadName, 
	opengl_call_glPushName, 
	opengl_call_glPopName, 			/*335*/ 
	opengl_call_glDrawRangeElements, 
	opengl_call_glTexImage3D, 
	opengl_call_glTexSubImage3D, 
	opengl_call_glCopyTexSubImage3D, 
	opengl_call_glColorTable, 			/*340*/ 
	opengl_call_glColorSubTable, 
	opengl_call_glColorTableParameteriv, 
	opengl_call_glColorTableParameterfv, 
	opengl_call_glCopyColorSubTable, 
	opengl_call_glCopyColorTable, 			/*345*/ 
	opengl_call_glGetColorTable, 
	opengl_call_glGetColorTableParameterfv, 
	opengl_call_glGetColorTableParameteriv, 
	opengl_call_glBlendEquation, 
	opengl_call_glBlendColor, 			/*350*/ 
	opengl_call_glHistogram, 
	opengl_call_glResetHistogram, 
	opengl_call_glGetHistogram, 
	opengl_call_glGetHistogramParameterfv, 
	opengl_call_glGetHistogramParameteriv, 			/*355*/ 
	opengl_call_glMinmax, 
	opengl_call_glResetMinmax, 
	opengl_call_glGetMinmax, 
	opengl_call_glGetMinmaxParameterfv, 
	opengl_call_glGetMinmaxParameteriv, 			/*360*/ 
	opengl_call_glConvolutionFilter1D, 
	opengl_call_glConvolutionFilter2D, 
	opengl_call_glConvolutionParameterf, 
	opengl_call_glConvolutionParameterfv, 
	opengl_call_glConvolutionParameteri, 			/*365*/ 
	opengl_call_glConvolutionParameteriv, 
	opengl_call_glCopyConvolutionFilter1D, 
	opengl_call_glCopyConvolutionFilter2D, 
	opengl_call_glGetConvolutionFilter, 
	opengl_call_glGetConvolutionParameterfv, 			/*370*/ 
	opengl_call_glGetConvolutionParameteriv, 
	opengl_call_glSeparableFilter2D, 
	opengl_call_glGetSeparableFilter, 
	opengl_call_glActiveTexture, 
	opengl_call_glClientActiveTexture, 			/*375*/ 
	opengl_call_glCompressedTexImage1D, 
	opengl_call_glCompressedTexImage2D, 
	opengl_call_glCompressedTexImage3D, 
	opengl_call_glCompressedTexSubImage1D, 
	opengl_call_glCompressedTexSubImage2D, 			/*380*/ 
	opengl_call_glCompressedTexSubImage3D, 
	opengl_call_glGetCompressedTexImage, 
	opengl_call_glMultiTexCoord1d, 
	opengl_call_glMultiTexCoord1dv, 
	opengl_call_glMultiTexCoord1f, 			/*385*/ 
	opengl_call_glMultiTexCoord1fv, 
	opengl_call_glMultiTexCoord1i, 
	opengl_call_glMultiTexCoord1iv, 
	opengl_call_glMultiTexCoord1s, 
	opengl_call_glMultiTexCoord1sv, 			/*390*/ 
	opengl_call_glMultiTexCoord2d, 
	opengl_call_glMultiTexCoord2dv, 
	opengl_call_glMultiTexCoord2f, 
	opengl_call_glMultiTexCoord2fv, 
	opengl_call_glMultiTexCoord2i, 			/*395*/ 
	opengl_call_glMultiTexCoord2iv, 
	opengl_call_glMultiTexCoord2s, 
	opengl_call_glMultiTexCoord2sv, 
	opengl_call_glMultiTexCoord3d, 
	opengl_call_glMultiTexCoord3dv, 			/*400*/ 
	opengl_call_glMultiTexCoord3f, 
	opengl_call_glMultiTexCoord3fv, 
	opengl_call_glMultiTexCoord3i, 
	opengl_call_glMultiTexCoord3iv, 
	opengl_call_glMultiTexCoord3s, 			/*405*/ 
	opengl_call_glMultiTexCoord3sv, 
	opengl_call_glMultiTexCoord4d, 
	opengl_call_glMultiTexCoord4dv, 
	opengl_call_glMultiTexCoord4f, 
	opengl_call_glMultiTexCoord4fv, 			/*410*/ 
	opengl_call_glMultiTexCoord4i, 
	opengl_call_glMultiTexCoord4iv, 
	opengl_call_glMultiTexCoord4s, 
	opengl_call_glMultiTexCoord4sv, 
	opengl_call_glLoadTransposeMatrixd, 			/*415*/ 
	opengl_call_glLoadTransposeMatrixf, 
	opengl_call_glMultTransposeMatrixd, 
	opengl_call_glMultTransposeMatrixf, 
	opengl_call_glSampleCoverage, 
	opengl_call_glActiveTextureARB, 			/*420*/ 
	opengl_call_glClientActiveTextureARB, 
	opengl_call_glMultiTexCoord1dARB, 
	opengl_call_glMultiTexCoord1dvARB, 
	opengl_call_glMultiTexCoord1fARB, 
	opengl_call_glMultiTexCoord1fvARB, 			/*425*/ 
	opengl_call_glMultiTexCoord1iARB, 
	opengl_call_glMultiTexCoord1ivARB, 
	opengl_call_glMultiTexCoord1sARB, 
	opengl_call_glMultiTexCoord1svARB, 
	opengl_call_glMultiTexCoord2dARB, 			/*430*/ 
	opengl_call_glMultiTexCoord2dvARB, 
	opengl_call_glMultiTexCoord2fARB, 
	opengl_call_glMultiTexCoord2fvARB, 
	opengl_call_glMultiTexCoord2iARB, 
	opengl_call_glMultiTexCoord2ivARB, 			/*435*/ 
	opengl_call_glMultiTexCoord2sARB, 
	opengl_call_glMultiTexCoord2svARB, 
	opengl_call_glMultiTexCoord3dARB, 
	opengl_call_glMultiTexCoord3dvARB, 
	opengl_call_glMultiTexCoord3fARB, 			/*440*/ 
	opengl_call_glMultiTexCoord3fvARB, 
	opengl_call_glMultiTexCoord3iARB, 
	opengl_call_glMultiTexCoord3ivARB, 
	opengl_call_glMultiTexCoord3sARB, 
	opengl_call_glMultiTexCoord3svARB, 			/*445*/ 
	opengl_call_glMultiTexCoord4dARB, 
	opengl_call_glMultiTexCoord4dvARB, 
	opengl_call_glMultiTexCoord4fARB, 
	opengl_call_glMultiTexCoord4fvARB, 
	opengl_call_glMultiTexCoord4iARB, 			/*450*/ 
	opengl_call_glMultiTexCoord4ivARB, 
	opengl_call_glMultiTexCoord4sARB, 
	opengl_call_glMultiTexCoord4svARB, 
	opengl_call_glCreateDebugObjectMESA, 
	opengl_call_glClearDebugLogMESA, 			/*455*/ 
	opengl_call_glGetDebugLogMESA, 
	opengl_call_glGetDebugLogLengthMESA, 
	opengl_call_glBlendEquationSeparateATI, 
	opengl_call_glBlendFuncSeparate, 
	opengl_call_glMultiDrawArrays, 			/*460*/ 
	opengl_call_glMultiDrawElements, 
	opengl_call_glPointParameterf, 
	opengl_call_glPointParameterfv, 
	opengl_call_glPointParameteri, 
	opengl_call_glPointParameteriv, 			/*465*/ 
	opengl_call_glFogCoordf, 
	opengl_call_glFogCoordfv, 
	opengl_call_glFogCoordd, 
	opengl_call_glFogCoorddv, 
	opengl_call_glFogCoordPointer, 			/*470*/ 
	opengl_call_glSecondaryColor3b, 
	opengl_call_glSecondaryColor3bv, 
	opengl_call_glSecondaryColor3d, 
	opengl_call_glSecondaryColor3dv, 
	opengl_call_glSecondaryColor3f, 			/*475*/ 
	opengl_call_glSecondaryColor3fv, 
	opengl_call_glSecondaryColor3i, 
	opengl_call_glSecondaryColor3iv, 
	opengl_call_glSecondaryColor3s, 
	opengl_call_glSecondaryColor3sv, 			/*480*/ 
	opengl_call_glSecondaryColor3ub, 
	opengl_call_glSecondaryColor3ubv, 
	opengl_call_glSecondaryColor3ui, 
	opengl_call_glSecondaryColor3uiv, 
	opengl_call_glSecondaryColor3us, 			/*485*/ 
	opengl_call_glSecondaryColor3usv, 
	opengl_call_glSecondaryColorPointer, 
	opengl_call_glWindowPos2d, 
	opengl_call_glWindowPos2dv, 
	opengl_call_glWindowPos2f, 			/*490*/ 
	opengl_call_glWindowPos2fv, 
	opengl_call_glWindowPos2i, 
	opengl_call_glWindowPos2iv, 
	opengl_call_glWindowPos2s, 
	opengl_call_glWindowPos2sv, 			/*495*/ 
	opengl_call_glWindowPos3d, 
	opengl_call_glWindowPos3dv, 
	opengl_call_glWindowPos3f, 
	opengl_call_glWindowPos3fv, 
	opengl_call_glWindowPos3i, 			/*500*/ 
	opengl_call_glWindowPos3iv, 
	opengl_call_glWindowPos3s, 
	opengl_call_glWindowPos3sv, 
	opengl_call_glGenQueries, 
	opengl_call_glDeleteQueries, 			/*505*/ 
	opengl_call_glIsQuery, 
	opengl_call_glBeginQuery, 
	opengl_call_glEndQuery, 
	opengl_call_glGetQueryiv, 
	opengl_call_glGetQueryObjectiv, 			/*510*/ 
	opengl_call_glGetQueryObjectuiv, 
	opengl_call_glBindBuffer, 
	opengl_call_glDeleteBuffers, 
	opengl_call_glGenBuffers, 
	opengl_call_glIsBuffer, 			/*515*/ 
	opengl_call_glBufferData, 
	opengl_call_glBufferSubData, 
	opengl_call_glGetBufferSubData, 
	opengl_call_glMapBuffer, 
	opengl_call_glUnmapBuffer, 			/*520*/ 
	opengl_call_glGetBufferParameteriv, 
	opengl_call_glGetBufferPointerv, 
	opengl_call_glBlendEquationSeparate, 
	opengl_call_glDrawBuffers, 
	opengl_call_glStencilOpSeparate, 			/*525*/ 
	opengl_call_glStencilFuncSeparate, 
	opengl_call_glStencilMaskSeparate, 
	opengl_call_glAttachShader, 
	opengl_call_glBindAttribLocation, 
	opengl_call_glCompileShader, 			/*530*/ 
	opengl_call_glCreateProgram, 
	opengl_call_glCreateShader, 
	opengl_call_glDeleteProgram, 
	opengl_call_glDeleteShader, 
	opengl_call_glDetachShader, 			/*535*/ 
	opengl_call_glDisableVertexAttribArray, 
	opengl_call_glEnableVertexAttribArray, 
	opengl_call_glGetActiveAttrib, 
	opengl_call_glGetActiveUniform, 
	opengl_call_glGetAttachedShaders, 			/*540*/ 
	opengl_call_glGetAttribLocation, 
	opengl_call_glGetProgramiv, 
	opengl_call_glGetProgramInfoLog, 
	opengl_call_glGetShaderiv, 
	opengl_call_glGetShaderInfoLog, 			/*545*/ 
	opengl_call_glGetShaderSource, 
	opengl_call_glGetUniformLocation, 
	opengl_call_glGetUniformfv, 
	opengl_call_glGetUniformiv, 
	opengl_call_glGetVertexAttribdv, 			/*550*/ 
	opengl_call_glGetVertexAttribfv, 
	opengl_call_glGetVertexAttribiv, 
	opengl_call_glGetVertexAttribPointerv, 
	opengl_call_glIsProgram, 
	opengl_call_glIsShader, 			/*555*/ 
	opengl_call_glLinkProgram, 
	opengl_call_glShaderSource, 
	opengl_call_glUseProgram, 
	opengl_call_glUniform1f, 
	opengl_call_glUniform2f, 			/*560*/ 
	opengl_call_glUniform3f, 
	opengl_call_glUniform4f, 
	opengl_call_glUniform1i, 
	opengl_call_glUniform2i, 
	opengl_call_glUniform3i, 			/*565*/ 
	opengl_call_glUniform4i, 
	opengl_call_glUniform1fv, 
	opengl_call_glUniform2fv, 
	opengl_call_glUniform3fv, 
	opengl_call_glUniform4fv, 			/*570*/ 
	opengl_call_glUniform1iv, 
	opengl_call_glUniform2iv, 
	opengl_call_glUniform3iv, 
	opengl_call_glUniform4iv, 
	opengl_call_glUniformMatrix2fv, 			/*575*/ 
	opengl_call_glUniformMatrix3fv, 
	opengl_call_glUniformMatrix4fv, 
	opengl_call_glValidateProgram, 
	opengl_call_glVertexAttrib1d, 
	opengl_call_glVertexAttrib1dv, 			/*580*/ 
	opengl_call_glVertexAttrib1f, 
	opengl_call_glVertexAttrib1fv, 
	opengl_call_glVertexAttrib1s, 
	opengl_call_glVertexAttrib1sv, 
	opengl_call_glVertexAttrib2d, 			/*585*/ 
	opengl_call_glVertexAttrib2dv, 
	opengl_call_glVertexAttrib2f, 
	opengl_call_glVertexAttrib2fv, 
	opengl_call_glVertexAttrib2s, 
	opengl_call_glVertexAttrib2sv, 			/*590*/ 
	opengl_call_glVertexAttrib3d, 
	opengl_call_glVertexAttrib3dv, 
	opengl_call_glVertexAttrib3f, 
	opengl_call_glVertexAttrib3fv, 
	opengl_call_glVertexAttrib3s, 			/*595*/ 
	opengl_call_glVertexAttrib3sv, 
	opengl_call_glVertexAttrib4Nbv, 
	opengl_call_glVertexAttrib4Niv, 
	opengl_call_glVertexAttrib4Nsv, 
	opengl_call_glVertexAttrib4Nub, 			/*600*/ 
	opengl_call_glVertexAttrib4Nubv, 
	opengl_call_glVertexAttrib4Nuiv, 
	opengl_call_glVertexAttrib4Nusv, 
	opengl_call_glVertexAttrib4bv, 
	opengl_call_glVertexAttrib4d, 			/*605*/ 
	opengl_call_glVertexAttrib4dv, 
	opengl_call_glVertexAttrib4f, 
	opengl_call_glVertexAttrib4fv, 
	opengl_call_glVertexAttrib4iv, 
	opengl_call_glVertexAttrib4s, 			/*610*/ 
	opengl_call_glVertexAttrib4sv, 
	opengl_call_glVertexAttrib4ubv, 
	opengl_call_glVertexAttrib4uiv, 
	opengl_call_glVertexAttrib4usv, 
	opengl_call_glVertexAttribPointer, 			/*615*/ 
	opengl_call_glUniformMatrix2x3fv, 
	opengl_call_glUniformMatrix3x2fv, 
	opengl_call_glUniformMatrix2x4fv, 
	opengl_call_glUniformMatrix4x2fv, 
	opengl_call_glUniformMatrix3x4fv, 			/*620*/ 
	opengl_call_glUniformMatrix4x3fv, 
	opengl_call_glColorMaski, 
	opengl_call_glGetBooleani_v, 
	opengl_call_glGetIntegeri_v, 
	opengl_call_glEnablei, 			/*625*/ 
	opengl_call_glDisablei, 
	opengl_call_glIsEnabledi, 
	opengl_call_glBeginTransformFeedback, 
	opengl_call_glEndTransformFeedback, 
	opengl_call_glBindBufferRange, 			/*630*/ 
	opengl_call_glBindBufferBase, 
	opengl_call_glTransformFeedbackVaryings, 
	opengl_call_glGetTransformFeedbackVarying, 
	opengl_call_glClampColor, 
	opengl_call_glBeginConditionalRender, 			/*635*/ 
	opengl_call_glEndConditionalRender, 
	opengl_call_glVertexAttribIPointer, 
	opengl_call_glGetVertexAttribIiv, 
	opengl_call_glGetVertexAttribIuiv, 
	opengl_call_glVertexAttribI1i, 			/*640*/ 
	opengl_call_glVertexAttribI2i, 
	opengl_call_glVertexAttribI3i, 
	opengl_call_glVertexAttribI4i, 
	opengl_call_glVertexAttribI1ui, 
	opengl_call_glVertexAttribI2ui, 			/*645*/ 
	opengl_call_glVertexAttribI3ui, 
	opengl_call_glVertexAttribI4ui, 
	opengl_call_glVertexAttribI1iv, 
	opengl_call_glVertexAttribI2iv, 
	opengl_call_glVertexAttribI3iv, 			/*650*/ 
	opengl_call_glVertexAttribI4iv, 
	opengl_call_glVertexAttribI1uiv, 
	opengl_call_glVertexAttribI2uiv, 
	opengl_call_glVertexAttribI3uiv, 
	opengl_call_glVertexAttribI4uiv, 			/*655*/ 
	opengl_call_glVertexAttribI4bv, 
	opengl_call_glVertexAttribI4sv, 
	opengl_call_glVertexAttribI4ubv, 
	opengl_call_glVertexAttribI4usv, 
	opengl_call_glGetUniformuiv, 			/*660*/ 
	opengl_call_glBindFragDataLocation, 
	opengl_call_glGetFragDataLocation, 
	opengl_call_glUniform1ui, 
	opengl_call_glUniform2ui, 
	opengl_call_glUniform3ui, 			/*665*/ 
	opengl_call_glUniform4ui, 
	opengl_call_glUniform1uiv, 
	opengl_call_glUniform2uiv, 
	opengl_call_glUniform3uiv, 
	opengl_call_glUniform4uiv, 			/*670*/ 
	opengl_call_glTexParameterIiv, 
	opengl_call_glTexParameterIuiv, 
	opengl_call_glGetTexParameterIiv, 
	opengl_call_glGetTexParameterIuiv, 
	opengl_call_glClearBufferiv, 			/*675*/ 
	opengl_call_glClearBufferuiv, 
	opengl_call_glClearBufferfv, 
	opengl_call_glClearBufferfi, 
	opengl_call_glGetStringi, 
	opengl_call_glDrawArraysInstanced, 			/*680*/ 
	opengl_call_glDrawElementsInstanced, 
	opengl_call_glTexBuffer, 
	opengl_call_glPrimitiveRestartIndex, 
	opengl_call_glGetInteger64i_v, 
	opengl_call_glGetBufferParameteri64v, 			/*685*/ 
	opengl_call_glFramebufferTexture, 
	opengl_call_glVertexAttribDivisor, 
	opengl_call_glMinSampleShading, 
	opengl_call_glBlendEquationi, 
	opengl_call_glBlendEquationSeparatei, 			/*690*/ 
	opengl_call_glBlendFunci, 
	opengl_call_glBlendFuncSeparatei, 
	opengl_call_glCompressedTexSubImage3DARB, 
	opengl_call_glCompressedTexSubImage2DARB, 
	opengl_call_glCompressedTexSubImage1DARB, 			/*695*/ 
	opengl_call_glGetCompressedTexImageARB, 
	opengl_call_glPointParameterfARB, 
	opengl_call_glPointParameterfvARB, 
	opengl_call_glWeightbvARB, 
	opengl_call_glWeightsvARB, 			/*700*/ 
	opengl_call_glWeightivARB, 
	opengl_call_glWeightfvARB, 
	opengl_call_glWeightdvARB, 
	opengl_call_glWeightubvARB, 
	opengl_call_glWeightusvARB, 			/*705*/ 
	opengl_call_glWeightuivARB, 
	opengl_call_glWeightPointerARB, 
	opengl_call_glVertexBlendARB, 
	opengl_call_glCurrentPaletteMatrixARB, 
	opengl_call_glMatrixIndexubvARB, 			/*710*/ 
	opengl_call_glMatrixIndexusvARB, 
	opengl_call_glMatrixIndexuivARB, 
	opengl_call_glMatrixIndexPointerARB, 
	opengl_call_glWindowPos2dARB, 
	opengl_call_glWindowPos2dvARB, 			/*715*/ 
	opengl_call_glWindowPos2fARB, 
	opengl_call_glWindowPos2fvARB, 
	opengl_call_glWindowPos2iARB, 
	opengl_call_glWindowPos2ivARB, 
	opengl_call_glWindowPos2sARB, 			/*720*/ 
	opengl_call_glWindowPos2svARB, 
	opengl_call_glWindowPos3dARB, 
	opengl_call_glWindowPos3dvARB, 
	opengl_call_glWindowPos3fARB, 
	opengl_call_glWindowPos3fvARB, 			/*725*/ 
	opengl_call_glWindowPos3iARB, 
	opengl_call_glWindowPos3ivARB, 
	opengl_call_glWindowPos3sARB, 
	opengl_call_glWindowPos3svARB, 
	opengl_call_glVertexAttrib1dARB, 			/*730*/ 
	opengl_call_glVertexAttrib1dvARB, 
	opengl_call_glVertexAttrib1fARB, 
	opengl_call_glVertexAttrib1fvARB, 
	opengl_call_glVertexAttrib1sARB, 
	opengl_call_glVertexAttrib1svARB, 			/*735*/ 
	opengl_call_glVertexAttrib2dARB, 
	opengl_call_glVertexAttrib2dvARB, 
	opengl_call_glVertexAttrib2fARB, 
	opengl_call_glVertexAttrib2fvARB, 
	opengl_call_glVertexAttrib2sARB, 			/*740*/ 
	opengl_call_glVertexAttrib2svARB, 
	opengl_call_glVertexAttrib3dARB, 
	opengl_call_glVertexAttrib3dvARB, 
	opengl_call_glVertexAttrib3fARB, 
	opengl_call_glVertexAttrib3fvARB, 			/*745*/ 
	opengl_call_glVertexAttrib3sARB, 
	opengl_call_glVertexAttrib3svARB, 
	opengl_call_glVertexAttrib4NbvARB, 
	opengl_call_glVertexAttrib4NivARB, 
	opengl_call_glVertexAttrib4NsvARB, 			/*750*/ 
	opengl_call_glVertexAttrib4NubARB, 
	opengl_call_glVertexAttrib4NubvARB, 
	opengl_call_glVertexAttrib4NuivARB, 
	opengl_call_glVertexAttrib4NusvARB, 
	opengl_call_glVertexAttrib4bvARB, 			/*755*/ 
	opengl_call_glVertexAttrib4dARB, 
	opengl_call_glVertexAttrib4dvARB, 
	opengl_call_glVertexAttrib4fARB, 
	opengl_call_glVertexAttrib4fvARB, 
	opengl_call_glVertexAttrib4ivARB, 			/*760*/ 
	opengl_call_glVertexAttrib4sARB, 
	opengl_call_glVertexAttrib4svARB, 
	opengl_call_glVertexAttrib4ubvARB, 
	opengl_call_glVertexAttrib4uivARB, 
	opengl_call_glVertexAttrib4usvARB, 			/*765*/ 
	opengl_call_glVertexAttribPointerARB, 
	opengl_call_glEnableVertexAttribArrayARB, 
	opengl_call_glDisableVertexAttribArrayARB, 
	opengl_call_glProgramStringARB, 
	opengl_call_glBindProgramARB, 			/*770*/ 
	opengl_call_glDeleteProgramsARB, 
	opengl_call_glGenProgramsARB, 
	opengl_call_glProgramEnvParameter4dARB, 
	opengl_call_glProgramEnvParameter4dvARB, 
	opengl_call_glProgramEnvParameter4fARB, 			/*775*/ 
	opengl_call_glProgramEnvParameter4fvARB, 
	opengl_call_glProgramLocalParameter4dARB, 
	opengl_call_glProgramLocalParameter4dvARB, 
	opengl_call_glProgramLocalParameter4fARB, 
	opengl_call_glProgramLocalParameter4fvARB, 			/*780*/ 
	opengl_call_glGetProgramEnvParameterdvARB, 
	opengl_call_glGetProgramEnvParameterfvARB, 
	opengl_call_glGetProgramLocalParameterdvARB, 
	opengl_call_glGetProgramLocalParameterfvARB, 
	opengl_call_glGetProgramivARB, 			/*785*/ 
	opengl_call_glGetProgramStringARB, 
	opengl_call_glGetVertexAttribdvARB, 
	opengl_call_glGetVertexAttribfvARB, 
	opengl_call_glGetVertexAttribivARB, 
	opengl_call_glGetVertexAttribPointervARB, 			/*790*/ 
	opengl_call_glIsProgramARB, 
	opengl_call_glBindBufferARB, 
	opengl_call_glDeleteBuffersARB, 
	opengl_call_glGenBuffersARB, 
	opengl_call_glIsBufferARB, 			/*795*/ 
	opengl_call_glBufferDataARB, 
	opengl_call_glBufferSubDataARB, 
	opengl_call_glGetBufferSubDataARB, 
	opengl_call_glMapBufferARB, 
	opengl_call_glUnmapBufferARB, 			/*800*/ 
	opengl_call_glGetBufferParameterivARB, 
	opengl_call_glGetBufferPointervARB, 
	opengl_call_glGenQueriesARB, 
	opengl_call_glDeleteQueriesARB, 
	opengl_call_glIsQueryARB, 			/*805*/ 
	opengl_call_glBeginQueryARB, 
	opengl_call_glEndQueryARB, 
	opengl_call_glGetQueryivARB, 
	opengl_call_glGetQueryObjectivARB, 
	opengl_call_glGetQueryObjectuivARB, 			/*810*/ 
	opengl_call_glDeleteObjectARB, 
	opengl_call_glGetHandleARB, 
	opengl_call_glDetachObjectARB, 
	opengl_call_glCreateShaderObjectARB, 
	opengl_call_glShaderSourceARB, 			/*815*/ 
	opengl_call_glCompileShaderARB, 
	opengl_call_glCreateProgramObjectARB, 
	opengl_call_glAttachObjectARB, 
	opengl_call_glLinkProgramARB, 
	opengl_call_glUseProgramObjectARB, 			/*820*/ 
	opengl_call_glValidateProgramARB, 
	opengl_call_glUniform1fARB, 
	opengl_call_glUniform2fARB, 
	opengl_call_glUniform3fARB, 
	opengl_call_glUniform4fARB, 			/*825*/ 
	opengl_call_glUniform1iARB, 
	opengl_call_glUniform2iARB, 
	opengl_call_glUniform3iARB, 
	opengl_call_glUniform4iARB, 
	opengl_call_glUniform1fvARB, 			/*830*/ 
	opengl_call_glUniform2fvARB, 
	opengl_call_glUniform3fvARB, 
	opengl_call_glUniform4fvARB, 
	opengl_call_glUniform1ivARB, 
	opengl_call_glUniform2ivARB, 			/*835*/ 
	opengl_call_glUniform3ivARB, 
	opengl_call_glUniform4ivARB, 
	opengl_call_glUniformMatrix2fvARB, 
	opengl_call_glUniformMatrix3fvARB, 
	opengl_call_glUniformMatrix4fvARB, 			/*840*/ 
	opengl_call_glGetObjectParameterfvARB, 
	opengl_call_glGetObjectParameterivARB, 
	opengl_call_glGetInfoLogARB, 
	opengl_call_glGetAttachedObjectsARB, 
	opengl_call_glGetUniformLocationARB, 			/*845*/ 
	opengl_call_glGetActiveUniformARB, 
	opengl_call_glGetUniformfvARB, 
	opengl_call_glGetUniformivARB, 
	opengl_call_glGetShaderSourceARB, 
	opengl_call_glBindAttribLocationARB, 			/*850*/ 
	opengl_call_glGetActiveAttribARB, 
	opengl_call_glGetAttribLocationARB, 
	opengl_call_glDrawBuffersARB, 
	opengl_call_glClampColorARB, 
	opengl_call_glDrawArraysInstancedARB, 			/*855*/ 
	opengl_call_glDrawElementsInstancedARB, 
	opengl_call_glIsRenderbuffer, 
	opengl_call_glBindRenderbuffer, 
	opengl_call_glDeleteRenderbuffers, 
	opengl_call_glGenRenderbuffers, 			/*860*/ 
	opengl_call_glRenderbufferStorage, 
	opengl_call_glGetRenderbufferParameteriv, 
	opengl_call_glIsFramebuffer, 
	opengl_call_glBindFramebuffer, 
	opengl_call_glDeleteFramebuffers, 			/*865*/ 
	opengl_call_glGenFramebuffers, 
	opengl_call_glCheckFramebufferStatus, 
	opengl_call_glFramebufferTexture1D, 
	opengl_call_glFramebufferTexture2D, 
	opengl_call_glFramebufferTexture3D, 			/*870*/ 
	opengl_call_glFramebufferRenderbuffer, 
	opengl_call_glGetFramebufferAttachmentParameteriv, 
	opengl_call_glGenerateMipmap, 
	opengl_call_glBlitFramebuffer, 
	opengl_call_glRenderbufferStorageMultisample, 			/*875*/ 
	opengl_call_glFramebufferTextureLayer, 
	opengl_call_glProgramParameteriARB, 
	opengl_call_glFramebufferTextureARB, 
	opengl_call_glFramebufferTextureLayerARB, 
	opengl_call_glFramebufferTextureFaceARB, 			/*880*/ 
	opengl_call_glVertexAttribDivisorARB, 
	opengl_call_glMapBufferRange, 
	opengl_call_glFlushMappedBufferRange, 
	opengl_call_glTexBufferARB, 
	opengl_call_glBindVertexArray, 			/*885*/ 
	opengl_call_glDeleteVertexArrays, 
	opengl_call_glGenVertexArrays, 
	opengl_call_glIsVertexArray, 
	opengl_call_glGetUniformIndices, 
	opengl_call_glGetActiveUniformsiv, 			/*890*/ 
	opengl_call_glGetActiveUniformName, 
	opengl_call_glGetUniformBlockIndex, 
	opengl_call_glGetActiveUniformBlockiv, 
	opengl_call_glGetActiveUniformBlockName, 
	opengl_call_glUniformBlockBinding, 			/*895*/ 
	opengl_call_glCopyBufferSubData, 
	opengl_call_glDrawElementsBaseVertex, 
	opengl_call_glDrawRangeElementsBaseVertex, 
	opengl_call_glDrawElementsInstancedBaseVertex, 
	opengl_call_glMultiDrawElementsBaseVertex, 			/*900*/ 
	opengl_call_glProvokingVertex, 
	opengl_call_glFenceSync, 
	opengl_call_glIsSync, 
	opengl_call_glDeleteSync, 
	opengl_call_glClientWaitSync, 			/*905*/ 
	opengl_call_glWaitSync, 
	opengl_call_glGetInteger64v, 
	opengl_call_glGetSynciv, 
	opengl_call_glTexImage2DMultisample, 
	opengl_call_glTexImage3DMultisample, 			/*910*/ 
	opengl_call_glGetMultisamplefv, 
	opengl_call_glSampleMaski, 
	opengl_call_glBlendEquationiARB, 
	opengl_call_glBlendEquationSeparateiARB, 
	opengl_call_glBlendFunciARB, 			/*915*/ 
	opengl_call_glBlendFuncSeparateiARB, 
	opengl_call_glMinSampleShadingARB, 
	opengl_call_glNamedStringARB, 
	opengl_call_glDeleteNamedStringARB, 
	opengl_call_glCompileShaderIncludeARB, 			/*920*/ 
	opengl_call_glIsNamedStringARB, 
	opengl_call_glGetNamedStringARB, 
	opengl_call_glGetNamedStringivARB, 
	opengl_call_glBindFragDataLocationIndexed, 
	opengl_call_glGetFragDataIndex, 			/*925*/ 
	opengl_call_glGenSamplers, 
	opengl_call_glDeleteSamplers, 
	opengl_call_glIsSampler, 
	opengl_call_glBindSampler, 
	opengl_call_glSamplerParameteri, 			/*930*/ 
	opengl_call_glSamplerParameteriv, 
	opengl_call_glSamplerParameterf, 
	opengl_call_glSamplerParameterfv, 
	opengl_call_glSamplerParameterIiv, 
	opengl_call_glSamplerParameterIuiv, 			/*935*/ 
	opengl_call_glGetSamplerParameteriv, 
	opengl_call_glGetSamplerParameterIiv, 
	opengl_call_glGetSamplerParameterfv, 
	opengl_call_glGetSamplerParameterIuiv, 
	opengl_call_glQueryCounter, 			/*940*/ 
	opengl_call_glGetQueryObjecti64v, 
	opengl_call_glGetQueryObjectui64v, 
	opengl_call_glVertexP2ui, 
	opengl_call_glVertexP2uiv, 
	opengl_call_glVertexP3ui, 			/*945*/ 
	opengl_call_glVertexP3uiv, 
	opengl_call_glVertexP4ui, 
	opengl_call_glVertexP4uiv, 
	opengl_call_glTexCoordP1ui, 
	opengl_call_glTexCoordP1uiv, 			/*950*/ 
	opengl_call_glTexCoordP2ui, 
	opengl_call_glTexCoordP2uiv, 
	opengl_call_glTexCoordP3ui, 
	opengl_call_glTexCoordP3uiv, 
	opengl_call_glTexCoordP4ui, 			/*955*/ 
	opengl_call_glTexCoordP4uiv, 
	opengl_call_glMultiTexCoordP1ui, 
	opengl_call_glMultiTexCoordP1uiv, 
	opengl_call_glMultiTexCoordP2ui, 
	opengl_call_glMultiTexCoordP2uiv, 			/*960*/ 
	opengl_call_glMultiTexCoordP3ui, 
	opengl_call_glMultiTexCoordP3uiv, 
	opengl_call_glMultiTexCoordP4ui, 
	opengl_call_glMultiTexCoordP4uiv, 
	opengl_call_glNormalP3ui, 			/*965*/ 
	opengl_call_glNormalP3uiv, 
	opengl_call_glColorP3ui, 
	opengl_call_glColorP3uiv, 
	opengl_call_glColorP4ui, 
	opengl_call_glColorP4uiv, 			/*970*/ 
	opengl_call_glSecondaryColorP3ui, 
	opengl_call_glSecondaryColorP3uiv, 
	opengl_call_glVertexAttribP1ui, 
	opengl_call_glVertexAttribP1uiv, 
	opengl_call_glVertexAttribP2ui, 			/*975*/ 
	opengl_call_glVertexAttribP2uiv, 
	opengl_call_glVertexAttribP3ui, 
	opengl_call_glVertexAttribP3uiv, 
	opengl_call_glVertexAttribP4ui, 
	opengl_call_glVertexAttribP4uiv, 			/*980*/ 
	opengl_call_glDrawArraysIndirect, 
	opengl_call_glDrawElementsIndirect, 
	opengl_call_glUniform1d, 
	opengl_call_glUniform2d, 
	opengl_call_glUniform3d, 			/*985*/ 
	opengl_call_glUniform4d, 
	opengl_call_glUniform1dv, 
	opengl_call_glUniform2dv, 
	opengl_call_glUniform3dv, 
	opengl_call_glUniform4dv, 			/*990*/ 
	opengl_call_glUniformMatrix2dv, 
	opengl_call_glUniformMatrix3dv, 
	opengl_call_glUniformMatrix4dv, 
	opengl_call_glUniformMatrix2x3dv, 
	opengl_call_glUniformMatrix2x4dv, 			/*995*/ 
	opengl_call_glUniformMatrix3x2dv, 
	opengl_call_glUniformMatrix3x4dv, 
	opengl_call_glUniformMatrix4x2dv, 
	opengl_call_glUniformMatrix4x3dv, 
	opengl_call_glGetUniformdv, 			/*1000*/ 
	opengl_call_glGetSubroutineUniformLocation, 
	opengl_call_glGetSubroutineIndex, 
	opengl_call_glGetActiveSubroutineUniformiv, 
	opengl_call_glGetActiveSubroutineUniformName, 
	opengl_call_glGetActiveSubroutineName, 			/*1005*/ 
	opengl_call_glUniformSubroutinesuiv, 
	opengl_call_glGetUniformSubroutineuiv, 
	opengl_call_glGetProgramStageiv, 
	opengl_call_glPatchParameteri, 
	opengl_call_glPatchParameterfv, 			/*1010*/ 
	opengl_call_glBindTransformFeedback, 
	opengl_call_glDeleteTransformFeedbacks, 
	opengl_call_glGenTransformFeedbacks, 
	opengl_call_glIsTransformFeedback, 
	opengl_call_glPauseTransformFeedback, 			/*1015*/ 
	opengl_call_glResumeTransformFeedback, 
	opengl_call_glDrawTransformFeedback, 
	opengl_call_glDrawTransformFeedbackStream, 
	opengl_call_glBeginQueryIndexed, 
	opengl_call_glEndQueryIndexed, 			/*1020*/ 
	opengl_call_glGetQueryIndexediv, 
	opengl_call_glReleaseShaderCompiler, 
	opengl_call_glShaderBinary, 
	opengl_call_glGetShaderPrecisionFormat, 
	opengl_call_glDepthRangef, 			/*1025*/ 
	opengl_call_glClearDepthf, 
	opengl_call_glGetProgramBinary, 
	opengl_call_glProgramBinary, 
	opengl_call_glProgramParameteri, 
	opengl_call_glUseProgramStages, 			/*1030*/ 
	opengl_call_glActiveShaderProgram, 
	opengl_call_glCreateShaderProgramv, 
	opengl_call_glBindProgramPipeline, 
	opengl_call_glDeleteProgramPipelines, 
	opengl_call_glGenProgramPipelines, 			/*1035*/ 
	opengl_call_glIsProgramPipeline, 
	opengl_call_glGetProgramPipelineiv, 
	opengl_call_glProgramUniform1i, 
	opengl_call_glProgramUniform1iv, 
	opengl_call_glProgramUniform1f, 			/*1040*/ 
	opengl_call_glProgramUniform1fv, 
	opengl_call_glProgramUniform1d, 
	opengl_call_glProgramUniform1dv, 
	opengl_call_glProgramUniform1ui, 
	opengl_call_glProgramUniform1uiv, 			/*1045*/ 
	opengl_call_glProgramUniform2i, 
	opengl_call_glProgramUniform2iv, 
	opengl_call_glProgramUniform2f, 
	opengl_call_glProgramUniform2fv, 
	opengl_call_glProgramUniform2d, 			/*1050*/ 
	opengl_call_glProgramUniform2dv, 
	opengl_call_glProgramUniform2ui, 
	opengl_call_glProgramUniform2uiv, 
	opengl_call_glProgramUniform3i, 
	opengl_call_glProgramUniform3iv, 			/*1055*/ 
	opengl_call_glProgramUniform3f, 
	opengl_call_glProgramUniform3fv, 
	opengl_call_glProgramUniform3d, 
	opengl_call_glProgramUniform3dv, 
	opengl_call_glProgramUniform3ui, 			/*1060*/ 
	opengl_call_glProgramUniform3uiv, 
	opengl_call_glProgramUniform4i, 
	opengl_call_glProgramUniform4iv, 
	opengl_call_glProgramUniform4f, 
	opengl_call_glProgramUniform4fv, 			/*1065*/ 
	opengl_call_glProgramUniform4d, 
	opengl_call_glProgramUniform4dv, 
	opengl_call_glProgramUniform4ui, 
	opengl_call_glProgramUniform4uiv, 
	opengl_call_glProgramUniformMatrix2fv, 			/*1070*/ 
	opengl_call_glProgramUniformMatrix3fv, 
	opengl_call_glProgramUniformMatrix4fv, 
	opengl_call_glProgramUniformMatrix2dv, 
	opengl_call_glProgramUniformMatrix3dv, 
	opengl_call_glProgramUniformMatrix4dv, 			/*1075*/ 
	opengl_call_glProgramUniformMatrix2x3fv, 
	opengl_call_glProgramUniformMatrix3x2fv, 
	opengl_call_glProgramUniformMatrix2x4fv, 
	opengl_call_glProgramUniformMatrix4x2fv, 
	opengl_call_glProgramUniformMatrix3x4fv, 			/*1080*/ 
	opengl_call_glProgramUniformMatrix4x3fv, 
	opengl_call_glProgramUniformMatrix2x3dv, 
	opengl_call_glProgramUniformMatrix3x2dv, 
	opengl_call_glProgramUniformMatrix2x4dv, 
	opengl_call_glProgramUniformMatrix4x2dv, 			/*1085*/ 
	opengl_call_glProgramUniformMatrix3x4dv, 
	opengl_call_glProgramUniformMatrix4x3dv, 
	opengl_call_glValidateProgramPipeline, 
	opengl_call_glGetProgramPipelineInfoLog, 
	opengl_call_glVertexAttribL1d, 			/*1090*/ 
	opengl_call_glVertexAttribL2d, 
	opengl_call_glVertexAttribL3d, 
	opengl_call_glVertexAttribL4d, 
	opengl_call_glVertexAttribL1dv, 
	opengl_call_glVertexAttribL2dv, 			/*1095*/ 
	opengl_call_glVertexAttribL3dv, 
	opengl_call_glVertexAttribL4dv, 
	opengl_call_glVertexAttribLPointer, 
	opengl_call_glGetVertexAttribLdv, 
	opengl_call_glViewportArrayv, 			/*1100*/ 
	opengl_call_glViewportIndexedf, 
	opengl_call_glViewportIndexedfv, 
	opengl_call_glScissorArrayv, 
	opengl_call_glScissorIndexed, 
	opengl_call_glScissorIndexedv, 			/*1105*/ 
	opengl_call_glDepthRangeArrayv, 
	opengl_call_glDepthRangeIndexed, 
	opengl_call_glGetFloati_v, 
	opengl_call_glGetDoublei_v, 
	opengl_call_glCreateSyncFromCLeventARB, 			/*1110*/ 
	opengl_call_glDebugMessageControlARB, 
	opengl_call_glDebugMessageInsertARB, 
	opengl_call_glDebugMessageCallbackARB, 
	opengl_call_glGetDebugMessageLogARB, 
	opengl_call_glGetGraphicsResetStatusARB, 			/*1115*/ 
	opengl_call_glGetnMapdvARB, 
	opengl_call_glGetnMapfvARB, 
	opengl_call_glGetnMapivARB, 
	opengl_call_glGetnPixelMapfvARB, 
	opengl_call_glGetnPixelMapuivARB, 			/*1120*/ 
	opengl_call_glGetnPixelMapusvARB, 
	opengl_call_glGetnPolygonStippleARB, 
	opengl_call_glGetnColorTableARB, 
	opengl_call_glGetnConvolutionFilterARB, 
	opengl_call_glGetnSeparableFilterARB, 			/*1125*/ 
	opengl_call_glGetnHistogramARB, 
	opengl_call_glGetnMinmaxARB, 
	opengl_call_glGetnTexImageARB, 
	opengl_call_glReadnPixelsARB, 
	opengl_call_glGetnCompressedTexImageARB, 			/*1130*/ 
	opengl_call_glGetnUniformfvARB, 
	opengl_call_glGetnUniformivARB, 
	opengl_call_glGetnUniformuivARB, 
	opengl_call_glGetnUniformdvARB, 
	opengl_call_glDrawArraysInstancedBaseInstance, 			/*1135*/ 
	opengl_call_glDrawElementsInstancedBaseInstance, 
	opengl_call_glDrawElementsInstancedBaseVertexBaseInstance, 
	opengl_call_glDrawTransformFeedbackInstanced, 
	opengl_call_glDrawTransformFeedbackStreamInstanced, 
	opengl_call_glGetInternalformativ, 			/*1140*/ 
	opengl_call_glGetActiveAtomicCounterBufferiv, 
	opengl_call_glBindImageTexture, 
	opengl_call_glMemoryBarrier, 
	opengl_call_glTexStorage1D, 
	opengl_call_glTexStorage2D, 			/*1145*/ 
	opengl_call_glTexStorage3D, 
	opengl_call_glTextureStorage1DEXT, 
	opengl_call_glTextureStorage2DEXT, 
	opengl_call_glTextureStorage3DEXT, 
	opengl_call_glBlendColorEXT, 			/*1150*/ 
	opengl_call_glPolygonOffsetEXT, 
	opengl_call_glTexImage3DEXT, 
	opengl_call_glTexSubImage3DEXT, 
	opengl_call_glGetTexFilterFuncSGIS, 
	opengl_call_glTexFilterFuncSGIS, 			/*1155*/ 
	opengl_call_glTexSubImage1DEXT, 
	opengl_call_glTexSubImage2DEXT, 
	opengl_call_glCopyTexImage1DEXT, 
	opengl_call_glCopyTexImage2DEXT, 
	opengl_call_glCopyTexSubImage1DEXT, 			/*1160*/ 
	opengl_call_glCopyTexSubImage2DEXT, 
	opengl_call_glCopyTexSubImage3DEXT, 
	opengl_call_glGetHistogramEXT, 
	opengl_call_glGetHistogramParameterfvEXT, 
	opengl_call_glGetHistogramParameterivEXT, 			/*1165*/ 
	opengl_call_glGetMinmaxEXT, 
	opengl_call_glGetMinmaxParameterfvEXT, 
	opengl_call_glGetMinmaxParameterivEXT, 
	opengl_call_glHistogramEXT, 
	opengl_call_glMinmaxEXT, 			/*1170*/ 
	opengl_call_glResetHistogramEXT, 
	opengl_call_glResetMinmaxEXT, 
	opengl_call_glConvolutionFilter1DEXT, 
	opengl_call_glConvolutionFilter2DEXT, 
	opengl_call_glConvolutionParameterfEXT, 			/*1175*/ 
	opengl_call_glConvolutionParameterfvEXT, 
	opengl_call_glConvolutionParameteriEXT, 
	opengl_call_glConvolutionParameterivEXT, 
	opengl_call_glCopyConvolutionFilter1DEXT, 
	opengl_call_glCopyConvolutionFilter2DEXT, 			/*1180*/ 
	opengl_call_glGetConvolutionFilterEXT, 
	opengl_call_glGetConvolutionParameterfvEXT, 
	opengl_call_glGetConvolutionParameterivEXT, 
	opengl_call_glGetSeparableFilterEXT, 
	opengl_call_glSeparableFilter2DEXT, 			/*1185*/ 
	opengl_call_glColorTableSGI, 
	opengl_call_glColorTableParameterfvSGI, 
	opengl_call_glColorTableParameterivSGI, 
	opengl_call_glCopyColorTableSGI, 
	opengl_call_glGetColorTableSGI, 			/*1190*/ 
	opengl_call_glGetColorTableParameterfvSGI, 
	opengl_call_glGetColorTableParameterivSGI, 
	opengl_call_glPixelTexGenSGIX, 
	opengl_call_glPixelTexGenParameteriSGIS, 
	opengl_call_glPixelTexGenParameterivSGIS, 			/*1195*/ 
	opengl_call_glPixelTexGenParameterfSGIS, 
	opengl_call_glPixelTexGenParameterfvSGIS, 
	opengl_call_glGetPixelTexGenParameterivSGIS, 
	opengl_call_glGetPixelTexGenParameterfvSGIS, 
	opengl_call_glTexImage4DSGIS, 			/*1200*/ 
	opengl_call_glTexSubImage4DSGIS, 
	opengl_call_glAreTexturesResidentEXT, 
	opengl_call_glBindTextureEXT, 
	opengl_call_glDeleteTexturesEXT, 
	opengl_call_glGenTexturesEXT, 			/*1205*/ 
	opengl_call_glIsTextureEXT, 
	opengl_call_glPrioritizeTexturesEXT, 
	opengl_call_glDetailTexFuncSGIS, 
	opengl_call_glGetDetailTexFuncSGIS, 
	opengl_call_glSharpenTexFuncSGIS, 			/*1210*/ 
	opengl_call_glGetSharpenTexFuncSGIS, 
	opengl_call_glSampleMaskSGIS, 
	opengl_call_glSamplePatternSGIS, 
	opengl_call_glArrayElementEXT, 
	opengl_call_glColorPointerEXT, 			/*1215*/ 
	opengl_call_glDrawArraysEXT, 
	opengl_call_glEdgeFlagPointerEXT, 
	opengl_call_glGetPointervEXT, 
	opengl_call_glIndexPointerEXT, 
	opengl_call_glNormalPointerEXT, 			/*1220*/ 
	opengl_call_glTexCoordPointerEXT, 
	opengl_call_glVertexPointerEXT, 
	opengl_call_glBlendEquationEXT, 
	opengl_call_glSpriteParameterfSGIX, 
	opengl_call_glSpriteParameterfvSGIX, 			/*1225*/ 
	opengl_call_glSpriteParameteriSGIX, 
	opengl_call_glSpriteParameterivSGIX, 
	opengl_call_glPointParameterfEXT, 
	opengl_call_glPointParameterfvEXT, 
	opengl_call_glPointParameterfSGIS, 			/*1230*/ 
	opengl_call_glPointParameterfvSGIS, 
	opengl_call_glGetInstrumentsSGIX, 
	opengl_call_glInstrumentsBufferSGIX, 
	opengl_call_glPollInstrumentsSGIX, 
	opengl_call_glReadInstrumentsSGIX, 			/*1235*/ 
	opengl_call_glStartInstrumentsSGIX, 
	opengl_call_glStopInstrumentsSGIX, 
	opengl_call_glFrameZoomSGIX, 
	opengl_call_glTagSampleBufferSGIX, 
	opengl_call_glDeformationMap3dSGIX, 			/*1240*/ 
	opengl_call_glDeformationMap3fSGIX, 
	opengl_call_glDeformSGIX, 
	opengl_call_glLoadIdentityDeformationMapSGIX, 
	opengl_call_glReferencePlaneSGIX, 
	opengl_call_glFlushRasterSGIX, 			/*1245*/ 
	opengl_call_glFogFuncSGIS, 
	opengl_call_glGetFogFuncSGIS, 
	opengl_call_glImageTransformParameteriHP, 
	opengl_call_glImageTransformParameterfHP, 
	opengl_call_glImageTransformParameterivHP, 			/*1250*/ 
	opengl_call_glImageTransformParameterfvHP, 
	opengl_call_glGetImageTransformParameterivHP, 
	opengl_call_glGetImageTransformParameterfvHP, 
	opengl_call_glColorSubTableEXT, 
	opengl_call_glCopyColorSubTableEXT, 			/*1255*/ 
	opengl_call_glHintPGI, 
	opengl_call_glColorTableEXT, 
	opengl_call_glGetColorTableEXT, 
	opengl_call_glGetColorTableParameterivEXT, 
	opengl_call_glGetColorTableParameterfvEXT, 			/*1260*/ 
	opengl_call_glGetListParameterfvSGIX, 
	opengl_call_glGetListParameterivSGIX, 
	opengl_call_glListParameterfSGIX, 
	opengl_call_glListParameterfvSGIX, 
	opengl_call_glListParameteriSGIX, 			/*1265*/ 
	opengl_call_glListParameterivSGIX, 
	opengl_call_glIndexMaterialEXT, 
	opengl_call_glIndexFuncEXT, 
	opengl_call_glLockArraysEXT, 
	opengl_call_glUnlockArraysEXT, 			/*1270*/ 
	opengl_call_glCullParameterdvEXT, 
	opengl_call_glCullParameterfvEXT, 
	opengl_call_glFragmentColorMaterialSGIX, 
	opengl_call_glFragmentLightfSGIX, 
	opengl_call_glFragmentLightfvSGIX, 			/*1275*/ 
	opengl_call_glFragmentLightiSGIX, 
	opengl_call_glFragmentLightivSGIX, 
	opengl_call_glFragmentLightModelfSGIX, 
	opengl_call_glFragmentLightModelfvSGIX, 
	opengl_call_glFragmentLightModeliSGIX, 			/*1280*/ 
	opengl_call_glFragmentLightModelivSGIX, 
	opengl_call_glFragmentMaterialfSGIX, 
	opengl_call_glFragmentMaterialfvSGIX, 
	opengl_call_glFragmentMaterialiSGIX, 
	opengl_call_glFragmentMaterialivSGIX, 			/*1285*/ 
	opengl_call_glGetFragmentLightfvSGIX, 
	opengl_call_glGetFragmentLightivSGIX, 
	opengl_call_glGetFragmentMaterialfvSGIX, 
	opengl_call_glGetFragmentMaterialivSGIX, 
	opengl_call_glLightEnviSGIX, 			/*1290*/ 
	opengl_call_glDrawRangeElementsEXT, 
	opengl_call_glApplyTextureEXT, 
	opengl_call_glTextureLightEXT, 
	opengl_call_glTextureMaterialEXT, 
	opengl_call_glAsyncMarkerSGIX, 			/*1295*/ 
	opengl_call_glFinishAsyncSGIX, 
	opengl_call_glPollAsyncSGIX, 
	opengl_call_glGenAsyncMarkersSGIX, 
	opengl_call_glDeleteAsyncMarkersSGIX, 
	opengl_call_glIsAsyncMarkerSGIX, 			/*1300*/ 
	opengl_call_glVertexPointervINTEL, 
	opengl_call_glNormalPointervINTEL, 
	opengl_call_glColorPointervINTEL, 
	opengl_call_glTexCoordPointervINTEL, 
	opengl_call_glPixelTransformParameteriEXT, 			/*1305*/ 
	opengl_call_glPixelTransformParameterfEXT, 
	opengl_call_glPixelTransformParameterivEXT, 
	opengl_call_glPixelTransformParameterfvEXT, 
	opengl_call_glSecondaryColor3bEXT, 
	opengl_call_glSecondaryColor3bvEXT, 			/*1310*/ 
	opengl_call_glSecondaryColor3dEXT, 
	opengl_call_glSecondaryColor3dvEXT, 
	opengl_call_glSecondaryColor3fEXT, 
	opengl_call_glSecondaryColor3fvEXT, 
	opengl_call_glSecondaryColor3iEXT, 			/*1315*/ 
	opengl_call_glSecondaryColor3ivEXT, 
	opengl_call_glSecondaryColor3sEXT, 
	opengl_call_glSecondaryColor3svEXT, 
	opengl_call_glSecondaryColor3ubEXT, 
	opengl_call_glSecondaryColor3ubvEXT, 			/*1320*/ 
	opengl_call_glSecondaryColor3uiEXT, 
	opengl_call_glSecondaryColor3uivEXT, 
	opengl_call_glSecondaryColor3usEXT, 
	opengl_call_glSecondaryColor3usvEXT, 
	opengl_call_glSecondaryColorPointerEXT, 			/*1325*/ 
	opengl_call_glTextureNormalEXT, 
	opengl_call_glMultiDrawArraysEXT, 
	opengl_call_glMultiDrawElementsEXT, 
	opengl_call_glFogCoordfEXT, 
	opengl_call_glFogCoordfvEXT, 			/*1330*/ 
	opengl_call_glFogCoorddEXT, 
	opengl_call_glFogCoorddvEXT, 
	opengl_call_glFogCoordPointerEXT, 
	opengl_call_glTangent3bEXT, 
	opengl_call_glTangent3bvEXT, 			/*1335*/ 
	opengl_call_glTangent3dEXT, 
	opengl_call_glTangent3dvEXT, 
	opengl_call_glTangent3fEXT, 
	opengl_call_glTangent3fvEXT, 
	opengl_call_glTangent3iEXT, 			/*1340*/ 
	opengl_call_glTangent3ivEXT, 
	opengl_call_glTangent3sEXT, 
	opengl_call_glTangent3svEXT, 
	opengl_call_glBinormal3bEXT, 
	opengl_call_glBinormal3bvEXT, 			/*1345*/ 
	opengl_call_glBinormal3dEXT, 
	opengl_call_glBinormal3dvEXT, 
	opengl_call_glBinormal3fEXT, 
	opengl_call_glBinormal3fvEXT, 
	opengl_call_glBinormal3iEXT, 			/*1350*/ 
	opengl_call_glBinormal3ivEXT, 
	opengl_call_glBinormal3sEXT, 
	opengl_call_glBinormal3svEXT, 
	opengl_call_glTangentPointerEXT, 
	opengl_call_glBinormalPointerEXT, 			/*1355*/ 
	opengl_call_glFinishTextureSUNX, 
	opengl_call_glGlobalAlphaFactorbSUN, 
	opengl_call_glGlobalAlphaFactorsSUN, 
	opengl_call_glGlobalAlphaFactoriSUN, 
	opengl_call_glGlobalAlphaFactorfSUN, 			/*1360*/ 
	opengl_call_glGlobalAlphaFactordSUN, 
	opengl_call_glGlobalAlphaFactorubSUN, 
	opengl_call_glGlobalAlphaFactorusSUN, 
	opengl_call_glGlobalAlphaFactoruiSUN, 
	opengl_call_glReplacementCodeuiSUN, 			/*1365*/ 
	opengl_call_glReplacementCodeusSUN, 
	opengl_call_glReplacementCodeubSUN, 
	opengl_call_glReplacementCodeuivSUN, 
	opengl_call_glReplacementCodeusvSUN, 
	opengl_call_glReplacementCodeubvSUN, 			/*1370*/ 
	opengl_call_glReplacementCodePointerSUN, 
	opengl_call_glColor4ubVertex2fSUN, 
	opengl_call_glColor4ubVertex2fvSUN, 
	opengl_call_glColor4ubVertex3fSUN, 
	opengl_call_glColor4ubVertex3fvSUN, 			/*1375*/ 
	opengl_call_glColor3fVertex3fSUN, 
	opengl_call_glColor3fVertex3fvSUN, 
	opengl_call_glNormal3fVertex3fSUN, 
	opengl_call_glNormal3fVertex3fvSUN, 
	opengl_call_glColor4fNormal3fVertex3fSUN, 			/*1380*/ 
	opengl_call_glColor4fNormal3fVertex3fvSUN, 
	opengl_call_glTexCoord2fVertex3fSUN, 
	opengl_call_glTexCoord2fVertex3fvSUN, 
	opengl_call_glTexCoord4fVertex4fSUN, 
	opengl_call_glTexCoord4fVertex4fvSUN, 			/*1385*/ 
	opengl_call_glTexCoord2fColor4ubVertex3fSUN, 
	opengl_call_glTexCoord2fColor4ubVertex3fvSUN, 
	opengl_call_glTexCoord2fColor3fVertex3fSUN, 
	opengl_call_glTexCoord2fColor3fVertex3fvSUN, 
	opengl_call_glTexCoord2fNormal3fVertex3fSUN, 			/*1390*/ 
	opengl_call_glTexCoord2fNormal3fVertex3fvSUN, 
	opengl_call_glTexCoord2fColor4fNormal3fVertex3fSUN, 
	opengl_call_glTexCoord2fColor4fNormal3fVertex3fvSUN, 
	opengl_call_glTexCoord4fColor4fNormal3fVertex4fSUN, 
	opengl_call_glTexCoord4fColor4fNormal3fVertex4fvSUN, 			/*1395*/ 
	opengl_call_glReplacementCodeuiVertex3fSUN, 
	opengl_call_glReplacementCodeuiVertex3fvSUN, 
	opengl_call_glReplacementCodeuiColor4ubVertex3fSUN, 
	opengl_call_glReplacementCodeuiColor4ubVertex3fvSUN, 
	opengl_call_glReplacementCodeuiColor3fVertex3fSUN, 			/*1400*/ 
	opengl_call_glReplacementCodeuiColor3fVertex3fvSUN, 
	opengl_call_glReplacementCodeuiNormal3fVertex3fSUN, 
	opengl_call_glReplacementCodeuiNormal3fVertex3fvSUN, 
	opengl_call_glReplacementCodeuiColor4fNormal3fVertex3fSUN, 
	opengl_call_glReplacementCodeuiColor4fNormal3fVertex3fvSUN, 			/*1405*/ 
	opengl_call_glReplacementCodeuiTexCoord2fVertex3fSUN, 
	opengl_call_glReplacementCodeuiTexCoord2fVertex3fvSUN, 
	opengl_call_glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN, 
	opengl_call_glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN, 
	opengl_call_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN, 			/*1410*/ 
	opengl_call_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN, 
	opengl_call_glBlendFuncSeparateEXT, 
	opengl_call_glBlendFuncSeparateINGR, 
	opengl_call_glVertexWeightfEXT, 
	opengl_call_glVertexWeightfvEXT, 			/*1415*/ 
	opengl_call_glVertexWeightPointerEXT, 
	opengl_call_glFlushVertexArrayRangeNV, 
	opengl_call_glVertexArrayRangeNV, 
	opengl_call_glCombinerParameterfvNV, 
	opengl_call_glCombinerParameterfNV, 			/*1420*/ 
	opengl_call_glCombinerParameterivNV, 
	opengl_call_glCombinerParameteriNV, 
	opengl_call_glCombinerInputNV, 
	opengl_call_glCombinerOutputNV, 
	opengl_call_glFinalCombinerInputNV, 			/*1425*/ 
	opengl_call_glGetCombinerInputParameterfvNV, 
	opengl_call_glGetCombinerInputParameterivNV, 
	opengl_call_glGetCombinerOutputParameterfvNV, 
	opengl_call_glGetCombinerOutputParameterivNV, 
	opengl_call_glGetFinalCombinerInputParameterfvNV, 			/*1430*/ 
	opengl_call_glGetFinalCombinerInputParameterivNV, 
	opengl_call_glResizeBuffersMESA, 
	opengl_call_glWindowPos2dMESA, 
	opengl_call_glWindowPos2dvMESA, 
	opengl_call_glWindowPos2fMESA, 			/*1435*/ 
	opengl_call_glWindowPos2fvMESA, 
	opengl_call_glWindowPos2iMESA, 
	opengl_call_glWindowPos2ivMESA, 
	opengl_call_glWindowPos2sMESA, 
	opengl_call_glWindowPos2svMESA, 			/*1440*/ 
	opengl_call_glWindowPos3dMESA, 
	opengl_call_glWindowPos3dvMESA, 
	opengl_call_glWindowPos3fMESA, 
	opengl_call_glWindowPos3fvMESA, 
	opengl_call_glWindowPos3iMESA, 			/*1445*/ 
	opengl_call_glWindowPos3ivMESA, 
	opengl_call_glWindowPos3sMESA, 
	opengl_call_glWindowPos3svMESA, 
	opengl_call_glWindowPos4dMESA, 
	opengl_call_glWindowPos4dvMESA, 			/*1450*/ 
	opengl_call_glWindowPos4fMESA, 
	opengl_call_glWindowPos4fvMESA, 
	opengl_call_glWindowPos4iMESA, 
	opengl_call_glWindowPos4ivMESA, 
	opengl_call_glWindowPos4sMESA, 			/*1455*/ 
	opengl_call_glWindowPos4svMESA, 
	opengl_call_glMultiModeDrawArraysIBM, 
	opengl_call_glMultiModeDrawElementsIBM, 
	opengl_call_glColorPointerListIBM, 
	opengl_call_glSecondaryColorPointerListIBM, 			/*1460*/ 
	opengl_call_glEdgeFlagPointerListIBM, 
	opengl_call_glFogCoordPointerListIBM, 
	opengl_call_glIndexPointerListIBM, 
	opengl_call_glNormalPointerListIBM, 
	opengl_call_glTexCoordPointerListIBM, 			/*1465*/ 
	opengl_call_glVertexPointerListIBM, 
	opengl_call_glTbufferMask3DFX, 
	opengl_call_glSampleMaskEXT, 
	opengl_call_glSamplePatternEXT, 
	opengl_call_glTextureColorMaskSGIS, 			/*1470*/ 
	opengl_call_glooInterfaceSGIX, 
	opengl_call_glDeleteFencesNV, 
	opengl_call_glGenFencesNV, 
	opengl_call_glIsFenceNV, 
	opengl_call_glTestFenceNV, 			/*1475*/ 
	opengl_call_glGetFenceivNV, 
	opengl_call_glFinishFenceNV, 
	opengl_call_glSetFenceNV, 
	opengl_call_glMapControlPointsNV, 
	opengl_call_glMapParameterivNV, 			/*1480*/ 
	opengl_call_glMapParameterfvNV, 
	opengl_call_glGetMapControlPointsNV, 
	opengl_call_glGetMapParameterivNV, 
	opengl_call_glGetMapParameterfvNV, 
	opengl_call_glGetMapAttribParameterivNV, 			/*1485*/ 
	opengl_call_glGetMapAttribParameterfvNV, 
	opengl_call_glEvalMapsNV, 
	opengl_call_glCombinerStageParameterfvNV, 
	opengl_call_glGetCombinerStageParameterfvNV, 
	opengl_call_glAreProgramsResidentNV, 			/*1490*/ 
	opengl_call_glBindProgramNV, 
	opengl_call_glDeleteProgramsNV, 
	opengl_call_glExecuteProgramNV, 
	opengl_call_glGenProgramsNV, 
	opengl_call_glGetProgramParameterdvNV, 			/*1495*/ 
	opengl_call_glGetProgramParameterfvNV, 
	opengl_call_glGetProgramivNV, 
	opengl_call_glGetProgramStringNV, 
	opengl_call_glGetTrackMatrixivNV, 
	opengl_call_glGetVertexAttribdvNV, 			/*1500*/ 
	opengl_call_glGetVertexAttribfvNV, 
	opengl_call_glGetVertexAttribivNV, 
	opengl_call_glGetVertexAttribPointervNV, 
	opengl_call_glIsProgramNV, 
	opengl_call_glLoadProgramNV, 			/*1505*/ 
	opengl_call_glProgramParameter4dNV, 
	opengl_call_glProgramParameter4dvNV, 
	opengl_call_glProgramParameter4fNV, 
	opengl_call_glProgramParameter4fvNV, 
	opengl_call_glProgramParameters4dvNV, 			/*1510*/ 
	opengl_call_glProgramParameters4fvNV, 
	opengl_call_glRequestResidentProgramsNV, 
	opengl_call_glTrackMatrixNV, 
	opengl_call_glVertexAttribPointerNV, 
	opengl_call_glVertexAttrib1dNV, 			/*1515*/ 
	opengl_call_glVertexAttrib1dvNV, 
	opengl_call_glVertexAttrib1fNV, 
	opengl_call_glVertexAttrib1fvNV, 
	opengl_call_glVertexAttrib1sNV, 
	opengl_call_glVertexAttrib1svNV, 			/*1520*/ 
	opengl_call_glVertexAttrib2dNV, 
	opengl_call_glVertexAttrib2dvNV, 
	opengl_call_glVertexAttrib2fNV, 
	opengl_call_glVertexAttrib2fvNV, 
	opengl_call_glVertexAttrib2sNV, 			/*1525*/ 
	opengl_call_glVertexAttrib2svNV, 
	opengl_call_glVertexAttrib3dNV, 
	opengl_call_glVertexAttrib3dvNV, 
	opengl_call_glVertexAttrib3fNV, 
	opengl_call_glVertexAttrib3fvNV, 			/*1530*/ 
	opengl_call_glVertexAttrib3sNV, 
	opengl_call_glVertexAttrib3svNV, 
	opengl_call_glVertexAttrib4dNV, 
	opengl_call_glVertexAttrib4dvNV, 
	opengl_call_glVertexAttrib4fNV, 			/*1535*/ 
	opengl_call_glVertexAttrib4fvNV, 
	opengl_call_glVertexAttrib4sNV, 
	opengl_call_glVertexAttrib4svNV, 
	opengl_call_glVertexAttrib4ubNV, 
	opengl_call_glVertexAttrib4ubvNV, 			/*1540*/ 
	opengl_call_glVertexAttribs1dvNV, 
	opengl_call_glVertexAttribs1fvNV, 
	opengl_call_glVertexAttribs1svNV, 
	opengl_call_glVertexAttribs2dvNV, 
	opengl_call_glVertexAttribs2fvNV, 			/*1545*/ 
	opengl_call_glVertexAttribs2svNV, 
	opengl_call_glVertexAttribs3dvNV, 
	opengl_call_glVertexAttribs3fvNV, 
	opengl_call_glVertexAttribs3svNV, 
	opengl_call_glVertexAttribs4dvNV, 			/*1550*/ 
	opengl_call_glVertexAttribs4fvNV, 
	opengl_call_glVertexAttribs4svNV, 
	opengl_call_glVertexAttribs4ubvNV, 
	opengl_call_glTexBumpParameterivATI, 
	opengl_call_glTexBumpParameterfvATI, 			/*1555*/ 
	opengl_call_glGetTexBumpParameterivATI, 
	opengl_call_glGetTexBumpParameterfvATI, 
	opengl_call_glGenFragmentShadersATI, 
	opengl_call_glBindFragmentShaderATI, 
	opengl_call_glDeleteFragmentShaderATI, 			/*1560*/ 
	opengl_call_glBeginFragmentShaderATI, 
	opengl_call_glEndFragmentShaderATI, 
	opengl_call_glPassTexCoordATI, 
	opengl_call_glSampleMapATI, 
	opengl_call_glColorFragmentOp1ATI, 			/*1565*/ 
	opengl_call_glColorFragmentOp2ATI, 
	opengl_call_glColorFragmentOp3ATI, 
	opengl_call_glAlphaFragmentOp1ATI, 
	opengl_call_glAlphaFragmentOp2ATI, 
	opengl_call_glAlphaFragmentOp3ATI, 			/*1570*/ 
	opengl_call_glSetFragmentShaderConstantATI, 
	opengl_call_glesiATI, 
	opengl_call_glesfATI, 
	opengl_call_glNewObjectBufferATI, 
	opengl_call_glIsObjectBufferATI, 			/*1575*/ 
	opengl_call_glUpdateObjectBufferATI, 
	opengl_call_glGetObjectBufferfvATI, 
	opengl_call_glGetObjectBufferivATI, 
	opengl_call_glFreeObjectBufferATI, 
	opengl_call_glArrayObjectATI, 			/*1580*/ 
	opengl_call_glGetArrayObjectfvATI, 
	opengl_call_glGetArrayObjectivATI, 
	opengl_call_glVariantArrayObjectATI, 
	opengl_call_glGetVariantArrayObjectfvATI, 
	opengl_call_glGetVariantArrayObjectivATI, 			/*1585*/ 
	opengl_call_glBeginVertexShaderEXT, 
	opengl_call_glEndVertexShaderEXT, 
	opengl_call_glBindVertexShaderEXT, 
	opengl_call_glGenVertexShadersEXT, 
	opengl_call_glDeleteVertexShaderEXT, 			/*1590*/ 
	opengl_call_glShaderOp1EXT, 
	opengl_call_glShaderOp2EXT, 
	opengl_call_glShaderOp3EXT, 
	opengl_call_glSwizzleEXT, 
	opengl_call_glWriteMaskEXT, 			/*1595*/ 
	opengl_call_glInsertComponentEXT, 
	opengl_call_glExtractComponentEXT, 
	opengl_call_glGenSymbolsEXT, 
	opengl_call_glSetInvariantEXT, 
	opengl_call_glSetLocalConstantEXT, 			/*1600*/ 
	opengl_call_glVariantbvEXT, 
	opengl_call_glVariantsvEXT, 
	opengl_call_glVariantivEXT, 
	opengl_call_glVariantfvEXT, 
	opengl_call_glVariantdvEXT, 			/*1605*/ 
	opengl_call_glVariantubvEXT, 
	opengl_call_glVariantusvEXT, 
	opengl_call_glVariantuivEXT, 
	opengl_call_glVariantPointerEXT, 
	opengl_call_glEnableVariantClientStateEXT, 			/*1610*/ 
	opengl_call_glDisableVariantClientStateEXT, 
	opengl_call_glBindLightParameterEXT, 
	opengl_call_glBindMaterialParameterEXT, 
	opengl_call_glBindTexGenParameterEXT, 
	opengl_call_glBindTextureUnitParameterEXT, 			/*1615*/ 
	opengl_call_glBindParameterEXT, 
	opengl_call_glIsVariantEnabledEXT, 
	opengl_call_glGetVariantBooleanvEXT, 
	opengl_call_glGetVariantIntegervEXT, 
	opengl_call_glGetVariantFloatvEXT, 			/*1620*/ 
	opengl_call_glGetVariantPointervEXT, 
	opengl_call_glGetInvariantBooleanvEXT, 
	opengl_call_glGetInvariantIntegervEXT, 
	opengl_call_glGetInvariantFloatvEXT, 
	opengl_call_glGetLocalConstantBooleanvEXT, 			/*1625*/ 
	opengl_call_glGetLocalConstantIntegervEXT, 
	opengl_call_glGetLocalConstantFloatvEXT, 
	opengl_call_glVertexStream1sATI, 
	opengl_call_glVertexStream1svATI, 
	opengl_call_glVertexStream1iATI, 			/*1630*/ 
	opengl_call_glVertexStream1ivATI, 
	opengl_call_glVertexStream1fATI, 
	opengl_call_glVertexStream1fvATI, 
	opengl_call_glVertexStream1dATI, 
	opengl_call_glVertexStream1dvATI, 			/*1635*/ 
	opengl_call_glVertexStream2sATI, 
	opengl_call_glVertexStream2svATI, 
	opengl_call_glVertexStream2iATI, 
	opengl_call_glVertexStream2ivATI, 
	opengl_call_glVertexStream2fATI, 			/*1640*/ 
	opengl_call_glVertexStream2fvATI, 
	opengl_call_glVertexStream2dATI, 
	opengl_call_glVertexStream2dvATI, 
	opengl_call_glVertexStream3sATI, 
	opengl_call_glVertexStream3svATI, 			/*1645*/ 
	opengl_call_glVertexStream3iATI, 
	opengl_call_glVertexStream3ivATI, 
	opengl_call_glVertexStream3fATI, 
	opengl_call_glVertexStream3fvATI, 
	opengl_call_glVertexStream3dATI, 			/*1650*/ 
	opengl_call_glVertexStream3dvATI, 
	opengl_call_glVertexStream4sATI, 
	opengl_call_glVertexStream4svATI, 
	opengl_call_glVertexStream4iATI, 
	opengl_call_glVertexStream4ivATI, 			/*1655*/ 
	opengl_call_glVertexStream4fATI, 
	opengl_call_glVertexStream4fvATI, 
	opengl_call_glVertexStream4dATI, 
	opengl_call_glVertexStream4dvATI, 
	opengl_call_glNormalStream3bATI, 			/*1660*/ 
	opengl_call_glNormalStream3bvATI, 
	opengl_call_glNormalStream3sATI, 
	opengl_call_glNormalStream3svATI, 
	opengl_call_glNormalStream3iATI, 
	opengl_call_glNormalStream3ivATI, 			/*1665*/ 
	opengl_call_glNormalStream3fATI, 
	opengl_call_glNormalStream3fvATI, 
	opengl_call_glNormalStream3dATI, 
	opengl_call_glNormalStream3dvATI, 
	opengl_call_glClientActiveVertexStreamATI, 			/*1670*/ 
	opengl_call_glVertexBlendEnviATI, 
	opengl_call_glVertexBlendEnvfATI, 
	opengl_call_glElementPointerATI, 
	opengl_call_glDrawElementArrayATI, 
	opengl_call_glDrawRangeElementArrayATI, 			/*1675*/ 
	opengl_call_glDrawMeshArraysSUN, 
	opengl_call_glGenOcclusionQueriesNV, 
	opengl_call_glDeleteOcclusionQueriesNV, 
	opengl_call_glIsOcclusionQueryNV, 
	opengl_call_glBeginOcclusionQueryNV, 			/*1680*/ 
	opengl_call_glEndOcclusionQueryNV, 
	opengl_call_glGetOcclusionQueryivNV, 
	opengl_call_glGetOcclusionQueryuivNV, 
	opengl_call_glPointParameteriNV, 
	opengl_call_glPointParameterivNV, 			/*1685*/ 
	opengl_call_glActiveStencilFaceEXT, 
	opengl_call_glElementPointerAPPLE, 
	opengl_call_glDrawElementArrayAPPLE, 
	opengl_call_glDrawRangeElementArrayAPPLE, 
	opengl_call_glMultiDrawElementArrayAPPLE, 			/*1690*/ 
	opengl_call_glMultiDrawRangeElementArrayAPPLE, 
	opengl_call_glGenFencesAPPLE, 
	opengl_call_glDeleteFencesAPPLE, 
	opengl_call_glSetFenceAPPLE, 
	opengl_call_glIsFenceAPPLE, 			/*1695*/ 
	opengl_call_glTestFenceAPPLE, 
	opengl_call_glFinishFenceAPPLE, 
	opengl_call_glTestObjectAPPLE, 
	opengl_call_glFinishObjectAPPLE, 
	opengl_call_glBindVertexArrayAPPLE, 			/*1700*/ 
	opengl_call_glDeleteVertexArraysAPPLE, 
	opengl_call_glGenVertexArraysAPPLE, 
	opengl_call_glIsVertexArrayAPPLE, 
	opengl_call_glVertexArrayRangeAPPLE, 
	opengl_call_glFlushVertexArrayRangeAPPLE, 			/*1705*/ 
	opengl_call_glVertexArrayParameteriAPPLE, 
	opengl_call_glDrawBuffersATI, 
	opengl_call_glProgramNamedParameter4fNV, 
	opengl_call_glProgramNamedParameter4dNV, 
	opengl_call_glProgramNamedParameter4fvNV, 			/*1710*/ 
	opengl_call_glProgramNamedParameter4dvNV, 
	opengl_call_glGetProgramNamedParameterfvNV, 
	opengl_call_glGetProgramNamedParameterdvNV, 
	opengl_call_glVertex2hNV, 
	opengl_call_glVertex2hvNV, 			/*1715*/ 
	opengl_call_glVertex3hNV, 
	opengl_call_glVertex3hvNV, 
	opengl_call_glVertex4hNV, 
	opengl_call_glVertex4hvNV, 
	opengl_call_glNormal3hNV, 			/*1720*/ 
	opengl_call_glNormal3hvNV, 
	opengl_call_glColor3hNV, 
	opengl_call_glColor3hvNV, 
	opengl_call_glColor4hNV, 
	opengl_call_glColor4hvNV, 			/*1725*/ 
	opengl_call_glTexCoord1hNV, 
	opengl_call_glTexCoord1hvNV, 
	opengl_call_glTexCoord2hNV, 
	opengl_call_glTexCoord2hvNV, 
	opengl_call_glTexCoord3hNV, 			/*1730*/ 
	opengl_call_glTexCoord3hvNV, 
	opengl_call_glTexCoord4hNV, 
	opengl_call_glTexCoord4hvNV, 
	opengl_call_glMultiTexCoord1hNV, 
	opengl_call_glMultiTexCoord1hvNV, 			/*1735*/ 
	opengl_call_glMultiTexCoord2hNV, 
	opengl_call_glMultiTexCoord2hvNV, 
	opengl_call_glMultiTexCoord3hNV, 
	opengl_call_glMultiTexCoord3hvNV, 
	opengl_call_glMultiTexCoord4hNV, 			/*1740*/ 
	opengl_call_glMultiTexCoord4hvNV, 
	opengl_call_glFogCoordhNV, 
	opengl_call_glFogCoordhvNV, 
	opengl_call_glSecondaryColor3hNV, 
	opengl_call_glSecondaryColor3hvNV, 			/*1745*/ 
	opengl_call_glVertexWeighthNV, 
	opengl_call_glVertexWeighthvNV, 
	opengl_call_glVertexAttrib1hNV, 
	opengl_call_glVertexAttrib1hvNV, 
	opengl_call_glVertexAttrib2hNV, 			/*1750*/ 
	opengl_call_glVertexAttrib2hvNV, 
	opengl_call_glVertexAttrib3hNV, 
	opengl_call_glVertexAttrib3hvNV, 
	opengl_call_glVertexAttrib4hNV, 
	opengl_call_glVertexAttrib4hvNV, 			/*1755*/ 
	opengl_call_glVertexAttribs1hvNV, 
	opengl_call_glVertexAttribs2hvNV, 
	opengl_call_glVertexAttribs3hvNV, 
	opengl_call_glVertexAttribs4hvNV, 
	opengl_call_glPixelDataRangeNV, 			/*1760*/ 
	opengl_call_glFlushPixelDataRangeNV, 
	opengl_call_glPrimitiveRestartNV, 
	opengl_call_glPrimitiveRestartIndexNV, 
	opengl_call_glMapObjectBufferATI, 
	opengl_call_glUnmapObjectBufferATI, 			/*1765*/ 
	opengl_call_glStencilOpSeparateATI, 
	opengl_call_glStencilFuncSeparateATI, 
	opengl_call_glVertexAttribArrayObjectATI, 
	opengl_call_glGetVertexAttribArrayObjectfvATI, 
	opengl_call_glGetVertexAttribArrayObjectivATI, 			/*1770*/ 
	opengl_call_glDepthBoundsEXT, 
	opengl_call_glBlendEquationSeparateEXT, 
	opengl_call_glIsRenderbufferEXT, 
	opengl_call_glBindRenderbufferEXT, 
	opengl_call_glDeleteRenderbuffersEXT, 			/*1775*/ 
	opengl_call_glGenRenderbuffersEXT, 
	opengl_call_glRenderbufferStorageEXT, 
	opengl_call_glGetRenderbufferParameterivEXT, 
	opengl_call_glIsFramebufferEXT, 
	opengl_call_glBindFramebufferEXT, 			/*1780*/ 
	opengl_call_glDeleteFramebuffersEXT, 
	opengl_call_glGenFramebuffersEXT, 
	opengl_call_glCheckFramebufferStatusEXT, 
	opengl_call_glFramebufferTexture1DEXT, 
	opengl_call_glFramebufferTexture2DEXT, 			/*1785*/ 
	opengl_call_glFramebufferTexture3DEXT, 
	opengl_call_glFramebufferRenderbufferEXT, 
	opengl_call_glGetFramebufferAttachmentParameterivEXT, 
	opengl_call_glGenerateMipmapEXT, 
	opengl_call_glStringMarkerGREMEDY, 			/*1790*/ 
	opengl_call_glStencilClearTagEXT, 
	opengl_call_glBlitFramebufferEXT, 
	opengl_call_glRenderbufferStorageMultisampleEXT, 
	opengl_call_glGetQueryObjecti64vEXT, 
	opengl_call_glGetQueryObjectui64vEXT, 			/*1795*/ 
	opengl_call_glProgramEnvParameters4fvEXT, 
	opengl_call_glProgramLocalParameters4fvEXT, 
	opengl_call_glBufferParameteriAPPLE, 
	opengl_call_glFlushMappedBufferRangeAPPLE, 
	opengl_call_glProgramLocalParameterI4iNV, 			/*1800*/ 
	opengl_call_glProgramLocalParameterI4ivNV, 
	opengl_call_glProgramLocalParametersI4ivNV, 
	opengl_call_glProgramLocalParameterI4uiNV, 
	opengl_call_glProgramLocalParameterI4uivNV, 
	opengl_call_glProgramLocalParametersI4uivNV, 			/*1805*/ 
	opengl_call_glProgramEnvParameterI4iNV, 
	opengl_call_glProgramEnvParameterI4ivNV, 
	opengl_call_glProgramEnvParametersI4ivNV, 
	opengl_call_glProgramEnvParameterI4uiNV, 
	opengl_call_glProgramEnvParameterI4uivNV, 			/*1810*/ 
	opengl_call_glProgramEnvParametersI4uivNV, 
	opengl_call_glGetProgramLocalParameterIivNV, 
	opengl_call_glGetProgramLocalParameterIuivNV, 
	opengl_call_glGetProgramEnvParameterIivNV, 
	opengl_call_glGetProgramEnvParameterIuivNV, 			/*1815*/ 
	opengl_call_glProgramVertexLimitNV, 
	opengl_call_glFramebufferTextureEXT, 
	opengl_call_glFramebufferTextureFaceEXT, 
	opengl_call_glProgramParameteriEXT, 
	opengl_call_glVertexAttribI1iEXT, 			/*1820*/ 
	opengl_call_glVertexAttribI2iEXT, 
	opengl_call_glVertexAttribI3iEXT, 
	opengl_call_glVertexAttribI4iEXT, 
	opengl_call_glVertexAttribI1uiEXT, 
	opengl_call_glVertexAttribI2uiEXT, 			/*1825*/ 
	opengl_call_glVertexAttribI3uiEXT, 
	opengl_call_glVertexAttribI4uiEXT, 
	opengl_call_glVertexAttribI1ivEXT, 
	opengl_call_glVertexAttribI2ivEXT, 
	opengl_call_glVertexAttribI3ivEXT, 			/*1830*/ 
	opengl_call_glVertexAttribI4ivEXT, 
	opengl_call_glVertexAttribI1uivEXT, 
	opengl_call_glVertexAttribI2uivEXT, 
	opengl_call_glVertexAttribI3uivEXT, 
	opengl_call_glVertexAttribI4uivEXT, 			/*1835*/ 
	opengl_call_glVertexAttribI4bvEXT, 
	opengl_call_glVertexAttribI4svEXT, 
	opengl_call_glVertexAttribI4ubvEXT, 
	opengl_call_glVertexAttribI4usvEXT, 
	opengl_call_glVertexAttribIPointerEXT, 			/*1840*/ 
	opengl_call_glGetVertexAttribIivEXT, 
	opengl_call_glGetVertexAttribIuivEXT, 
	opengl_call_glGetUniformuivEXT, 
	opengl_call_glBindFragDataLocationEXT, 
	opengl_call_glGetFragDataLocationEXT, 			/*1845*/ 
	opengl_call_glUniform1uiEXT, 
	opengl_call_glUniform2uiEXT, 
	opengl_call_glUniform3uiEXT, 
	opengl_call_glUniform4uiEXT, 
	opengl_call_glUniform1uivEXT, 			/*1850*/ 
	opengl_call_glUniform2uivEXT, 
	opengl_call_glUniform3uivEXT, 
	opengl_call_glUniform4uivEXT, 
	opengl_call_glDrawArraysInstancedEXT, 
	opengl_call_glDrawElementsInstancedEXT, 			/*1855*/ 
	opengl_call_glTexBufferEXT, 
	opengl_call_glDepthRangedNV, 
	opengl_call_glClearDepthdNV, 
	opengl_call_glDepthBoundsdNV, 
	opengl_call_glRenderbufferStorageMultisampleCoverageNV, 			/*1860*/ 
	opengl_call_glProgramBufferParametersfvNV, 
	opengl_call_glProgramBufferParametersIivNV, 
	opengl_call_glProgramBufferParametersIuivNV, 
	opengl_call_glColorMaskIndexedEXT, 
	opengl_call_glGetBooleanIndexedvEXT, 			/*1865*/ 
	opengl_call_glGetIntegerIndexedvEXT, 
	opengl_call_glEnableIndexedEXT, 
	opengl_call_glDisableIndexedEXT, 
	opengl_call_glIsEnabledIndexedEXT, 
	opengl_call_glBeginTransformFeedbackNV, 			/*1870*/ 
	opengl_call_glEndTransformFeedbackNV, 
	opengl_call_glTransformFeedbackAttribsNV, 
	opengl_call_glBindBufferRangeNV, 
	opengl_call_glBindBufferOffsetNV, 
	opengl_call_glBindBufferBaseNV, 			/*1875*/ 
	opengl_call_glTransformFeedbackVaryingsNV, 
	opengl_call_glActiveVaryingNV, 
	opengl_call_glGetVaryingLocationNV, 
	opengl_call_glGetActiveVaryingNV, 
	opengl_call_glGetTransformFeedbackVaryingNV, 			/*1880*/ 
	opengl_call_glTransformFeedbackStreamAttribsNV, 
	opengl_call_glUniformBufferEXT, 
	opengl_call_glGetUniformBufferSizeEXT, 
	opengl_call_glGetUniformOffsetEXT, 
	opengl_call_glTexParameterIivEXT, 			/*1885*/ 
	opengl_call_glTexParameterIuivEXT, 
	opengl_call_glGetTexParameterIivEXT, 
	opengl_call_glGetTexParameterIuivEXT, 
	opengl_call_glClearColorIiEXT, 
	opengl_call_glClearColorIuiEXT, 			/*1890*/ 
	opengl_call_glFrameTerminatorGREMEDY, 
	opengl_call_glBeginConditionalRenderNV, 
	opengl_call_glEndConditionalRenderNV, 
	opengl_call_glPresentFrameKeyedNV, 
	opengl_call_glPresentFrameDualFillNV, 			/*1895*/ 
	opengl_call_glGetVideoivNV, 
	opengl_call_glGetVideouivNV, 
	opengl_call_glGetVideoi64vNV, 
	opengl_call_glGetVideoui64vNV, 
	opengl_call_glBeginTransformFeedbackEXT, 			/*1900*/ 
	opengl_call_glEndTransformFeedbackEXT, 
	opengl_call_glBindBufferRangeEXT, 
	opengl_call_glBindBufferOffsetEXT, 
	opengl_call_glBindBufferBaseEXT, 
	opengl_call_glTransformFeedbackVaryingsEXT, 			/*1905*/ 
	opengl_call_glGetTransformFeedbackVaryingEXT, 
	opengl_call_glClientAttribDefaultEXT, 
	opengl_call_glPushClientAttribDefaultEXT, 
	opengl_call_glMatrixLoadfEXT, 
	opengl_call_glMatrixLoaddEXT, 			/*1910*/ 
	opengl_call_glMatrixMultfEXT, 
	opengl_call_glMatrixMultdEXT, 
	opengl_call_glMatrixLoadIdentityEXT, 
	opengl_call_glMatrixRotatefEXT, 
	opengl_call_glMatrixRotatedEXT, 			/*1915*/ 
	opengl_call_glMatrixScalefEXT, 
	opengl_call_glMatrixScaledEXT, 
	opengl_call_glMatrixTranslatefEXT, 
	opengl_call_glMatrixTranslatedEXT, 
	opengl_call_glMatrixFrustumEXT, 			/*1920*/ 
	opengl_call_glMatrixOrthoEXT, 
	opengl_call_glMatrixPopEXT, 
	opengl_call_glMatrixPushEXT, 
	opengl_call_glMatrixLoadTransposefEXT, 
	opengl_call_glMatrixLoadTransposedEXT, 			/*1925*/ 
	opengl_call_glMatrixMultTransposefEXT, 
	opengl_call_glMatrixMultTransposedEXT, 
	opengl_call_glTextureParameterfEXT, 
	opengl_call_glTextureParameterfvEXT, 
	opengl_call_glTextureParameteriEXT, 			/*1930*/ 
	opengl_call_glTextureParameterivEXT, 
	opengl_call_glTextureImage1DEXT, 
	opengl_call_glTextureImage2DEXT, 
	opengl_call_glTextureSubImage1DEXT, 
	opengl_call_glTextureSubImage2DEXT, 			/*1935*/ 
	opengl_call_glCopyTextureImage1DEXT, 
	opengl_call_glCopyTextureImage2DEXT, 
	opengl_call_glCopyTextureSubImage1DEXT, 
	opengl_call_glCopyTextureSubImage2DEXT, 
	opengl_call_glGetTextureImageEXT, 			/*1940*/ 
	opengl_call_glGetTextureParameterfvEXT, 
	opengl_call_glGetTextureParameterivEXT, 
	opengl_call_glGetTextureLevelParameterfvEXT, 
	opengl_call_glGetTextureLevelParameterivEXT, 
	opengl_call_glTextureImage3DEXT, 			/*1945*/ 
	opengl_call_glTextureSubImage3DEXT, 
	opengl_call_glCopyTextureSubImage3DEXT, 
	opengl_call_glMultiTexParameterfEXT, 
	opengl_call_glMultiTexParameterfvEXT, 
	opengl_call_glMultiTexParameteriEXT, 			/*1950*/ 
	opengl_call_glMultiTexParameterivEXT, 
	opengl_call_glMultiTexImage1DEXT, 
	opengl_call_glMultiTexImage2DEXT, 
	opengl_call_glMultiTexSubImage1DEXT, 
	opengl_call_glMultiTexSubImage2DEXT, 			/*1955*/ 
	opengl_call_glCopyMultiTexImage1DEXT, 
	opengl_call_glCopyMultiTexImage2DEXT, 
	opengl_call_glCopyMultiTexSubImage1DEXT, 
	opengl_call_glCopyMultiTexSubImage2DEXT, 
	opengl_call_glGetMultiTexImageEXT, 			/*1960*/ 
	opengl_call_glGetMultiTexParameterfvEXT, 
	opengl_call_glGetMultiTexParameterivEXT, 
	opengl_call_glGetMultiTexLevelParameterfvEXT, 
	opengl_call_glGetMultiTexLevelParameterivEXT, 
	opengl_call_glMultiTexImage3DEXT, 			/*1965*/ 
	opengl_call_glMultiTexSubImage3DEXT, 
	opengl_call_glCopyMultiTexSubImage3DEXT, 
	opengl_call_glBindMultiTextureEXT, 
	opengl_call_glEnableClientStateIndexedEXT, 
	opengl_call_glDisableClientStateIndexedEXT, 			/*1970*/ 
	opengl_call_glMultiTexCoordPointerEXT, 
	opengl_call_glMultiTexEnvfEXT, 
	opengl_call_glMultiTexEnvfvEXT, 
	opengl_call_glMultiTexEnviEXT, 
	opengl_call_glMultiTexEnvivEXT, 			/*1975*/ 
	opengl_call_glMultiTexGendEXT, 
	opengl_call_glMultiTexGendvEXT, 
	opengl_call_glMultiTexGenfEXT, 
	opengl_call_glMultiTexGenfvEXT, 
	opengl_call_glMultiTexGeniEXT, 			/*1980*/ 
	opengl_call_glMultiTexGenivEXT, 
	opengl_call_glGetMultiTexEnvfvEXT, 
	opengl_call_glGetMultiTexEnvivEXT, 
	opengl_call_glGetMultiTexGendvEXT, 
	opengl_call_glGetMultiTexGenfvEXT, 			/*1985*/ 
	opengl_call_glGetMultiTexGenivEXT, 
	opengl_call_glGetFloatIndexedvEXT, 
	opengl_call_glGetDoubleIndexedvEXT, 
	opengl_call_glGetPointerIndexedvEXT, 
	opengl_call_glCompressedTextureImage3DEXT, 			/*1990*/ 
	opengl_call_glCompressedTextureImage2DEXT, 
	opengl_call_glCompressedTextureImage1DEXT, 
	opengl_call_glCompressedTextureSubImage3DEXT, 
	opengl_call_glCompressedTextureSubImage2DEXT, 
	opengl_call_glCompressedTextureSubImage1DEXT, 			/*1995*/ 
	opengl_call_glGetCompressedTextureImageEXT, 
	opengl_call_glCompressedMultiTexImage3DEXT, 
	opengl_call_glCompressedMultiTexImage2DEXT, 
	opengl_call_glCompressedMultiTexImage1DEXT, 
	opengl_call_glCompressedMultiTexSubImage3DEXT, 			/*2000*/ 
	opengl_call_glCompressedMultiTexSubImage2DEXT, 
	opengl_call_glCompressedMultiTexSubImage1DEXT, 
	opengl_call_glGetCompressedMultiTexImageEXT, 
	opengl_call_glNamedProgramStringEXT, 
	opengl_call_glNamedProgramLocalParameter4dEXT, 			/*2005*/ 
	opengl_call_glNamedProgramLocalParameter4dvEXT, 
	opengl_call_glNamedProgramLocalParameter4fEXT, 
	opengl_call_glNamedProgramLocalParameter4fvEXT, 
	opengl_call_glGetNamedProgramLocalParameterdvEXT, 
	opengl_call_glGetNamedProgramLocalParameterfvEXT, 			/*2010*/ 
	opengl_call_glGetNamedProgramivEXT, 
	opengl_call_glGetNamedProgramStringEXT, 
	opengl_call_glNamedProgramLocalParameters4fvEXT, 
	opengl_call_glNamedProgramLocalParameterI4iEXT, 
	opengl_call_glNamedProgramLocalParameterI4ivEXT, 			/*2015*/ 
	opengl_call_glNamedProgramLocalParametersI4ivEXT, 
	opengl_call_glNamedProgramLocalParameterI4uiEXT, 
	opengl_call_glNamedProgramLocalParameterI4uivEXT, 
	opengl_call_glNamedProgramLocalParametersI4uivEXT, 
	opengl_call_glGetNamedProgramLocalParameterIivEXT, 			/*2020*/ 
	opengl_call_glGetNamedProgramLocalParameterIuivEXT, 
	opengl_call_glTextureParameterIivEXT, 
	opengl_call_glTextureParameterIuivEXT, 
	opengl_call_glGetTextureParameterIivEXT, 
	opengl_call_glGetTextureParameterIuivEXT, 			/*2025*/ 
	opengl_call_glMultiTexParameterIivEXT, 
	opengl_call_glMultiTexParameterIuivEXT, 
	opengl_call_glGetMultiTexParameterIivEXT, 
	opengl_call_glGetMultiTexParameterIuivEXT, 
	opengl_call_glProgramUniform1fEXT, 			/*2030*/ 
	opengl_call_glProgramUniform2fEXT, 
	opengl_call_glProgramUniform3fEXT, 
	opengl_call_glProgramUniform4fEXT, 
	opengl_call_glProgramUniform1iEXT, 
	opengl_call_glProgramUniform2iEXT, 			/*2035*/ 
	opengl_call_glProgramUniform3iEXT, 
	opengl_call_glProgramUniform4iEXT, 
	opengl_call_glProgramUniform1fvEXT, 
	opengl_call_glProgramUniform2fvEXT, 
	opengl_call_glProgramUniform3fvEXT, 			/*2040*/ 
	opengl_call_glProgramUniform4fvEXT, 
	opengl_call_glProgramUniform1ivEXT, 
	opengl_call_glProgramUniform2ivEXT, 
	opengl_call_glProgramUniform3ivEXT, 
	opengl_call_glProgramUniform4ivEXT, 			/*2045*/ 
	opengl_call_glProgramUniformMatrix2fvEXT, 
	opengl_call_glProgramUniformMatrix3fvEXT, 
	opengl_call_glProgramUniformMatrix4fvEXT, 
	opengl_call_glProgramUniformMatrix2x3fvEXT, 
	opengl_call_glProgramUniformMatrix3x2fvEXT, 			/*2050*/ 
	opengl_call_glProgramUniformMatrix2x4fvEXT, 
	opengl_call_glProgramUniformMatrix4x2fvEXT, 
	opengl_call_glProgramUniformMatrix3x4fvEXT, 
	opengl_call_glProgramUniformMatrix4x3fvEXT, 
	opengl_call_glProgramUniform1uiEXT, 			/*2055*/ 
	opengl_call_glProgramUniform2uiEXT, 
	opengl_call_glProgramUniform3uiEXT, 
	opengl_call_glProgramUniform4uiEXT, 
	opengl_call_glProgramUniform1uivEXT, 
	opengl_call_glProgramUniform2uivEXT, 			/*2060*/ 
	opengl_call_glProgramUniform3uivEXT, 
	opengl_call_glProgramUniform4uivEXT, 
	opengl_call_glNamedBufferDataEXT, 
	opengl_call_glNamedBufferSubDataEXT, 
	opengl_call_glMapNamedBufferEXT, 			/*2065*/ 
	opengl_call_glUnmapNamedBufferEXT, 
	opengl_call_glMapNamedBufferRangeEXT, 
	opengl_call_glFlushMappedNamedBufferRangeEXT, 
	opengl_call_glNamedCopyBufferSubDataEXT, 
	opengl_call_glGetNamedBufferParameterivEXT, 			/*2070*/ 
	opengl_call_glGetNamedBufferPointervEXT, 
	opengl_call_glGetNamedBufferSubDataEXT, 
	opengl_call_glTextureBufferEXT, 
	opengl_call_glMultiTexBufferEXT, 
	opengl_call_glNamedRenderbufferStorageEXT, 			/*2075*/ 
	opengl_call_glGetNamedRenderbufferParameterivEXT, 
	opengl_call_glCheckNamedFramebufferStatusEXT, 
	opengl_call_glNamedFramebufferTexture1DEXT, 
	opengl_call_glNamedFramebufferTexture2DEXT, 
	opengl_call_glNamedFramebufferTexture3DEXT, 			/*2080*/ 
	opengl_call_glNamedFramebufferRenderbufferEXT, 
	opengl_call_glGetNamedFramebufferAttachmentParameterivEXT, 
	opengl_call_glGenerateTextureMipmapEXT, 
	opengl_call_glGenerateMultiTexMipmapEXT, 
	opengl_call_glFramebufferDrawBufferEXT, 			/*2085*/ 
	opengl_call_glFramebufferDrawBuffersEXT, 
	opengl_call_glFramebufferReadBufferEXT, 
	opengl_call_glGetFramebufferParameterivEXT, 
	opengl_call_glNamedRenderbufferStorageMultisampleEXT, 
	opengl_call_glNamedRenderbufferStorageMultisampleCoverageEXT, 			/*2090*/ 
	opengl_call_glNamedFramebufferTextureEXT, 
	opengl_call_glNamedFramebufferTextureLayerEXT, 
	opengl_call_glNamedFramebufferTextureFaceEXT, 
	opengl_call_glTextureRenderbufferEXT, 
	opengl_call_glMultiTexRenderbufferEXT, 			/*2095*/ 
	opengl_call_glProgramUniform1dEXT, 
	opengl_call_glProgramUniform2dEXT, 
	opengl_call_glProgramUniform3dEXT, 
	opengl_call_glProgramUniform4dEXT, 
	opengl_call_glProgramUniform1dvEXT, 			/*2100*/ 
	opengl_call_glProgramUniform2dvEXT, 
	opengl_call_glProgramUniform3dvEXT, 
	opengl_call_glProgramUniform4dvEXT, 
	opengl_call_glProgramUniformMatrix2dvEXT, 
	opengl_call_glProgramUniformMatrix3dvEXT, 			/*2105*/ 
	opengl_call_glProgramUniformMatrix4dvEXT, 
	opengl_call_glProgramUniformMatrix2x3dvEXT, 
	opengl_call_glProgramUniformMatrix2x4dvEXT, 
	opengl_call_glProgramUniformMatrix3x2dvEXT, 
	opengl_call_glProgramUniformMatrix3x4dvEXT, 			/*2110*/ 
	opengl_call_glProgramUniformMatrix4x2dvEXT, 
	opengl_call_glProgramUniformMatrix4x3dvEXT, 
	opengl_call_glGetMultisamplefvNV, 
	opengl_call_glSampleMaskIndexedNV, 
	opengl_call_glTexRenderbufferNV, 			/*2115*/ 
	opengl_call_glBindTransformFeedbackNV, 
	opengl_call_glDeleteTransformFeedbacksNV, 
	opengl_call_glGenTransformFeedbacksNV, 
	opengl_call_glIsTransformFeedbackNV, 
	opengl_call_glPauseTransformFeedbackNV, 			/*2120*/ 
	opengl_call_glResumeTransformFeedbackNV, 
	opengl_call_glDrawTransformFeedbackNV, 
	opengl_call_glGetPerfMonitorGroupsAMD, 
	opengl_call_glGetPerfMonitorCountersAMD, 
	opengl_call_glGetPerfMonitorGroupStringAMD, 			/*2125*/ 
	opengl_call_glGetPerfMonitorCounterStringAMD, 
	opengl_call_glGetPerfMonitorCounterInfoAMD, 
	opengl_call_glGenPerfMonitorsAMD, 
	opengl_call_glDeletePerfMonitorsAMD, 
	opengl_call_glSelectPerfMonitorCountersAMD, 			/*2130*/ 
	opengl_call_glBeginPerfMonitorAMD, 
	opengl_call_glEndPerfMonitorAMD, 
	opengl_call_glGetPerfMonitorCounterDataAMD, 
	opengl_call_glTessellationFactorAMD, 
	opengl_call_glTessellationModeAMD, 			/*2135*/ 
	opengl_call_glProvokingVertexEXT, 
	opengl_call_glBlendFuncIndexedAMD, 
	opengl_call_glBlendFuncSeparateIndexedAMD, 
	opengl_call_glBlendEquationIndexedAMD, 
	opengl_call_glBlendEquationSeparateIndexedAMD, 			/*2140*/ 
	opengl_call_glTextureRangeAPPLE, 
	opengl_call_glGetTexParameterPointervAPPLE, 
	opengl_call_glEnableVertexAttribAPPLE, 
	opengl_call_glDisableVertexAttribAPPLE, 
	opengl_call_glIsVertexAttribEnabledAPPLE, 			/*2145*/ 
	opengl_call_glMapVertexAttrib1dAPPLE, 
	opengl_call_glMapVertexAttrib1fAPPLE, 
	opengl_call_glMapVertexAttrib2dAPPLE, 
	opengl_call_glMapVertexAttrib2fAPPLE, 
	opengl_call_glObjectPurgeableAPPLE, 			/*2150*/ 
	opengl_call_glObjectUnpurgeableAPPLE, 
	opengl_call_glGetObjectParameterivAPPLE, 
	opengl_call_glBeginVideoCaptureNV, 
	opengl_call_glBindVideoCaptureStreamBufferNV, 
	opengl_call_glBindVideoCaptureStreamTextureNV, 			/*2155*/ 
	opengl_call_glEndVideoCaptureNV, 
	opengl_call_glGetVideoCaptureivNV, 
	opengl_call_glGetVideoCaptureStreamivNV, 
	opengl_call_glGetVideoCaptureStreamfvNV, 
	opengl_call_glGetVideoCaptureStreamdvNV, 			/*2160*/ 
	opengl_call_glVideoCaptureNV, 
	opengl_call_glVideoCaptureStreamParameterivNV, 
	opengl_call_glVideoCaptureStreamParameterfvNV, 
	opengl_call_glVideoCaptureStreamParameterdvNV, 
	opengl_call_glCopyImageSubDataNV, 			/*2165*/ 
	opengl_call_glUseShaderProgramEXT, 
	opengl_call_glActiveProgramEXT, 
	opengl_call_glCreateShaderProgramEXT, 
	opengl_call_glMakeBufferResidentNV, 
	opengl_call_glMakeBufferNonResidentNV, 			/*2170*/ 
	opengl_call_glIsBufferResidentNV, 
	opengl_call_glMakeNamedBufferResidentNV, 
	opengl_call_glMakeNamedBufferNonResidentNV, 
	opengl_call_glIsNamedBufferResidentNV, 
	opengl_call_glGetBufferParameterui64vNV, 			/*2175*/ 
	opengl_call_glGetNamedBufferParameterui64vNV, 
	opengl_call_glGetIntegerui64vNV, 
	opengl_call_glUniformui64NV, 
	opengl_call_glUniformui64vNV, 
	opengl_call_glGetUniformui64vNV, 			/*2180*/ 
	opengl_call_glProgramUniformui64NV, 
	opengl_call_glProgramUniformui64vNV, 
	opengl_call_glBufferAddressRangeNV, 
	opengl_call_glVertexFormatNV, 
	opengl_call_glNormalFormatNV, 			/*2185*/ 
	opengl_call_glColorFormatNV, 
	opengl_call_glIndexFormatNV, 
	opengl_call_glTexCoordFormatNV, 
	opengl_call_glEdgeFlagFormatNV, 
	opengl_call_glSecondaryColorFormatNV, 			/*2190*/ 
	opengl_call_glFogCoordFormatNV, 
	opengl_call_glVertexAttribFormatNV, 
	opengl_call_glVertexAttribIFormatNV, 
	opengl_call_glGetIntegerui64i_vNV, 
	opengl_call_glTextureBarrierNV, 			/*2195*/ 
	opengl_call_glBindImageTextureEXT, 
	opengl_call_glMemoryBarrierEXT, 
	opengl_call_glVertexAttribL1dEXT, 
	opengl_call_glVertexAttribL2dEXT, 
	opengl_call_glVertexAttribL3dEXT, 			/*2200*/ 
	opengl_call_glVertexAttribL4dEXT, 
	opengl_call_glVertexAttribL1dvEXT, 
	opengl_call_glVertexAttribL2dvEXT, 
	opengl_call_glVertexAttribL3dvEXT, 
	opengl_call_glVertexAttribL4dvEXT, 			/*2205*/ 
	opengl_call_glVertexAttribLPointerEXT, 
	opengl_call_glGetVertexAttribLdvEXT, 
	opengl_call_glVertexArrayVertexAttribLOffsetEXT, 
	opengl_call_glProgramSubroutineParametersuivNV, 
	opengl_call_glGetProgramSubroutineParameteruivNV, 			/*2210*/ 
	opengl_call_glUniform1i64NV, 
	opengl_call_glUniform2i64NV, 
	opengl_call_glUniform3i64NV, 
	opengl_call_glUniform4i64NV, 
	opengl_call_glUniform1i64vNV, 			/*2215*/ 
	opengl_call_glUniform2i64vNV, 
	opengl_call_glUniform3i64vNV, 
	opengl_call_glUniform4i64vNV, 
	opengl_call_glUniform1ui64NV, 
	opengl_call_glUniform2ui64NV, 			/*2220*/ 
	opengl_call_glUniform3ui64NV, 
	opengl_call_glUniform4ui64NV, 
	opengl_call_glUniform1ui64vNV, 
	opengl_call_glUniform2ui64vNV, 
	opengl_call_glUniform3ui64vNV, 			/*2225*/ 
	opengl_call_glUniform4ui64vNV, 
	opengl_call_glGetUniformi64vNV, 
	opengl_call_glProgramUniform1i64NV, 
	opengl_call_glProgramUniform2i64NV, 
	opengl_call_glProgramUniform3i64NV, 			/*2230*/ 
	opengl_call_glProgramUniform4i64NV, 
	opengl_call_glProgramUniform1i64vNV, 
	opengl_call_glProgramUniform2i64vNV, 
	opengl_call_glProgramUniform3i64vNV, 
	opengl_call_glProgramUniform4i64vNV, 			/*2235*/ 
	opengl_call_glProgramUniform1ui64NV, 
	opengl_call_glProgramUniform2ui64NV, 
	opengl_call_glProgramUniform3ui64NV, 
	opengl_call_glProgramUniform4ui64NV, 
	opengl_call_glProgramUniform1ui64vNV, 			/*2240*/ 
	opengl_call_glProgramUniform2ui64vNV, 
	opengl_call_glProgramUniform3ui64vNV, 
	opengl_call_glProgramUniform4ui64vNV, 
	opengl_call_glVertexAttribL1i64NV, 
	opengl_call_glVertexAttribL2i64NV, 			/*2245*/ 
	opengl_call_glVertexAttribL3i64NV, 
	opengl_call_glVertexAttribL4i64NV, 
	opengl_call_glVertexAttribL1i64vNV, 
	opengl_call_glVertexAttribL2i64vNV, 
	opengl_call_glVertexAttribL3i64vNV, 			/*2250*/ 
	opengl_call_glVertexAttribL4i64vNV, 
	opengl_call_glVertexAttribL1ui64NV, 
	opengl_call_glVertexAttribL2ui64NV, 
	opengl_call_glVertexAttribL3ui64NV, 
	opengl_call_glVertexAttribL4ui64NV, 			/*2255*/ 
	opengl_call_glVertexAttribL1ui64vNV, 
	opengl_call_glVertexAttribL2ui64vNV, 
	opengl_call_glVertexAttribL3ui64vNV, 
	opengl_call_glVertexAttribL4ui64vNV, 
	opengl_call_glGetVertexAttribLi64vNV, 			/*2260*/ 
	opengl_call_glGetVertexAttribLui64vNV, 
	opengl_call_glVertexAttribLFormatNV, 
	opengl_call_glGenNamesAMD, 
	opengl_call_glDeleteNamesAMD, 
	opengl_call_glIsNameAMD, 			/*2265*/ 
	opengl_call_glDebugMessageEnableAMD, 
	opengl_call_glDebugMessageInsertAMD, 
	opengl_call_glDebugMessageCallbackAMD, 
	opengl_call_glGetDebugMessageLogAMD, 
	opengl_call_glVDPAUInitNV, 			/*2270*/ 
	opengl_call_glVDPAUFiniNV, 
	opengl_call_glVDPAURegisterVideoSurfaceNV, 
	opengl_call_glVDPAURegisterOutputSurfaceNV, 
	opengl_call_glVDPAUIsSurfaceNV, 
	opengl_call_glVDPAUUnregisterSurfaceNV, 			/*2275*/ 
	opengl_call_glVDPAUGetSurfaceivNV, 
	opengl_call_glVDPAUSurfaceAccessNV, 
	opengl_call_glVDPAUMapSurfacesNV, 
	opengl_call_glVDPAUUnmapSurfacesNV, 
	opengl_call_glTexImage2DMultisampleCoverageNV, 			/*2280*/ 
	opengl_call_glTexImage3DMultisampleCoverageNV, 
	opengl_call_glTextureImage2DMultisampleNV, 
	opengl_call_glTextureImage3DMultisampleNV, 
	opengl_call_glTextureImage2DMultisampleCoverageNV, 
	opengl_call_glTextureImage3DMultisampleCoverageNV, 			/*2285*/ 
	opengl_call_glSetMultisamplefvAMD, 
	opengl_call_glImportSyncEXT, 
	opengl_call_glMultiDrawArraysIndirectAMD, 
	opengl_call_glMultiDrawElementsIndirectAMD, 

	opengl_call_count
};

#endif

