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

#include <m2s-glut.h>
#include <debug.h>


#define __M2S_GLUT_NOT_IMPL__  fatal("%s: not implemented.\n%s", \
	__FUNCTION__, err_m2s_glut_not_impl);


static char *err_m2s_glut_not_impl =
	"\tMulti2Sim provides partial support for the OpenGL Utility Toolkit (GLUT)\n"
	"\tlibrary. To request the implementation of a certain functionality, please\n"
	"\temail development@multi2sim.org.\n";


void glutInit(int *argcp, char **argv)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutInitDisplayMode(unsigned int mode)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutInitDisplayString(const char *string)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutInitWindowPosition(int x, int y)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutInitWindowSize(int width, int height)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutMainLoop(void)
{
	__M2S_GLUT_NOT_IMPL__
}


int glutCreateWindow(const char *title)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


int glutCreateSubWindow(int win, int x, int y, int width, int height)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


void glutDestroyWindow(int win)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutPostRedisplay(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutPostWindowRedisplay(int win)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSwapBuffers(void)
{
	__M2S_GLUT_NOT_IMPL__
}


int glutGetWindow(void)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


void glutSetWindow(int win)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSetWindowTitle(const char *title)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSetIconTitle(const char *title)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutPositionWindow(int x, int y)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutReshapeWindow(int width, int height)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutPopWindow(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutPushWindow(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutIconifyWindow(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutShowWindow(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutHideWindow(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutFullScreen(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSetCursor(int cursor)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutWarpPointer(int x, int y)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutEstablishOverlay(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutRemoveOverlay(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutUseLayer(GLenum layer)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutPostOverlayRedisplay(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutPostWindowOverlayRedisplay(int win)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutShowOverlay(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutHideOverlay(void)
{
	__M2S_GLUT_NOT_IMPL__
}


int glutCreateMenu(void (*func)(int))
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


void glutDestroyMenu(int menu)
{
	__M2S_GLUT_NOT_IMPL__
}


int glutGetMenu(void)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


void glutSetMenu(int menu)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutAddMenuEntry(const char *label, int value)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutAddSubMenu(const char *label, int submenu)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutChangeToMenuEntry(int item, const char *label, int value)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutChangeToSubMenu(int item, const char *label, int submenu)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutRemoveMenuItem(int item)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutAttachMenu(int button)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutDetachMenu(int button)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutDisplayFunc(void (*func)(void))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutReshapeFunc(void (*func)(int width, int height))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutKeyboardFunc(void (*func)(unsigned char key, int x, int y))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutMouseFunc(void (*func)(int button, int state, int x, int y))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutMotionFunc(void (*func)(int x, int y))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutPassiveMotionFunc(void (*func)(int x, int y))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutEntryFunc(void (*func)(int state))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutVisibilityFunc(void (*func)(int state))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutIdleFunc(void (*func)(void))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutTimerFunc(unsigned int millis, void (*func)(int value), int value)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutMenuStateFunc(void (*func)(int state))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSpecialFunc(void (*func)(int key, int x, int y))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSpaceballMotionFunc(void (*func)(int x, int y, int z))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSpaceballRotateFunc(void (*func)(int x, int y, int z))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSpaceballButtonFunc(void (*func)(int button, int state))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutButtonBoxFunc(void (*func)(int button, int state))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutDialsFunc(void (*func)(int dial, int value))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutTabletMotionFunc(void (*func)(int x, int y))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutTabletButtonFunc(void (*func)(int button, int state, int x, int y))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutMenuStatusFunc(void (*func)(int status, int x, int y))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutOverlayDisplayFunc(void (*func)(void))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutWindowStatusFunc(void (*func)(int state))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutKeyboardUpFunc(void (*func)(unsigned char key, int x, int y))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSpecialUpFunc(void (*func)(int key, int x, int y))
{
	__M2S_GLUT_NOT_IMPL__
}


void glutJoystickFunc(void (*func)(unsigned int buttonMask, int x, int y, int z), int pollInterval)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSetColor(int cell, GLfloat red, GLfloat green, GLfloat blue)
{
	__M2S_GLUT_NOT_IMPL__
}


GLfloat glutGetColor(int ndx, int component)
{
	__M2S_GLUT_NOT_IMPL__
	return 0.0;
}


void glutCopyColormap(int win)
{
	__M2S_GLUT_NOT_IMPL__
}


int glutGet(GLenum type)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


int glutDeviceGet(GLenum type)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


int glutExtensionSupported(const char *name)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


int glutGetModifiers(void)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


int glutLayerGet(GLenum type)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


void glutBitmapCharacter(void *font, int character)
{
	__M2S_GLUT_NOT_IMPL__
}


int glutBitmapWidth(void *font, int character)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


void glutStrokeCharacter(void *font, int character)
{
	__M2S_GLUT_NOT_IMPL__
}


int glutStrokeWidth(void *font, int character)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


int glutBitmapLength(void *font, const unsigned char *string)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


int glutStrokeLength(void *font, const unsigned char *string)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


void glutWireSphere(GLdouble radius, GLint slices, GLint stacks)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSolidSphere(GLdouble radius, GLint slices, GLint stacks)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutWireCone(GLdouble base, GLdouble height, GLint slices, GLint stacks)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSolidCone(GLdouble base, GLdouble height, GLint slices, GLint stacks)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutWireCube(GLdouble size)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSolidCube(GLdouble size)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutWireTorus(GLdouble innerRadius, GLdouble outerRadius, GLint sides, GLint rings)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSolidTorus(GLdouble innerRadius, GLdouble outerRadius, GLint sides, GLint rings)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutWireDodecahedron(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSolidDodecahedron(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutWireTeapot(GLdouble size)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSolidTeapot(GLdouble size)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutWireOctahedron(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSolidOctahedron(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutWireTetrahedron(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSolidTetrahedron(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutWireIcosahedron(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSolidIcosahedron(void)
{
	__M2S_GLUT_NOT_IMPL__
}


int glutVideoResizeGet(GLenum param)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


void glutSetupVideoResizing(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutStopVideoResizing(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutVideoResize(int x, int y, int width, int height)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutVideoPan(int x, int y, int width, int height)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutReportErrors(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutIgnoreKeyRepeat(int ignore)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutSetKeyRepeat(int repeatMode)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutForceJoystickFunc(void)
{
	__M2S_GLUT_NOT_IMPL__
}


void glutGameModeString(const char *string)
{
	__M2S_GLUT_NOT_IMPL__
}


int glutEnterGameMode(void)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


void glutLeaveGameMode(void)
{
	__M2S_GLUT_NOT_IMPL__
}


int glutGameModeGet(GLenum mode)
{
	__M2S_GLUT_NOT_IMPL__
	return 0;
}


