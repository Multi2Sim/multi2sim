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

#include <stdio.h>
#include <unistd.h>

#include "../include/GL/gl.h"
#include "../opengl/opengl.h"
#include "../opengl/context.h"
#include "api.h"
#include "debug.h"
#include "window.h"


/* Debug */
int glut_debug = 0;

/*
 * Error Messages
 */

#define __X86_GLUT_NOT_IMPL__  fatal("%s: not implemented.\n%s", \
	__FUNCTION__, glut_err_not_impl);


static char *glut_err_not_impl =
	"\tMulti2Sim provides partial support for the OpenGL Utility Toolkit (GLUT)\n"
	"\tlibrary. To request the implementation of a certain functionality, please\n"
	"\temail development@multi2sim.org.\n";

static char *glut_err_version =
	"\tYour guest application is using a version of the GLUT runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tGLUT runtime library ('libm2s-glut').\n";

static char *glut_err_native =
	"\tYou are trying to run natively an application using the Multi2Sim GLUT\n"
	"\tlibrary implementation ('libm2s-glut'). Please run this program on top of\n"
	"\tMulti2Sim.\n";




/*
 * GLUT Internal State
 */

unsigned int glut_initial_display_mode;

int glut_initial_window_position_x = -1;
int glut_initial_window_position_y = -1;
int glut_initial_window_width = 300;
int glut_initial_window_height = 300;

struct glut_window_t *glut_current_window = NULL;



/* Multi2Sim GLUT Runtime required */
#define glut_RUNTIME_VERSION_MAJOR	1
#define glut_RUNTIME_VERSION_MINOR	690

struct glut_version_t
{
	int major;
	int minor;
};

void glutInit(int *argcp, char **argv)
{
	struct glut_version_t version;

	int ret;

	/* Runtime function 'init' */
	ret = syscall(GLUT_SYSCALL_CODE, glut_call_init, &version);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call glut_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			glut_err_native);

	/* Check that exact major version matches */
	if (version.major != glut_RUNTIME_VERSION_MAJOR
			|| version.minor < glut_RUNTIME_VERSION_MINOR)
		fatal("incompatible GLUT runtime versions.\n"
			"\tGuest library v. %d.%d / Host implementation v. %d.%d.\n"
			"%s", glut_RUNTIME_VERSION_MAJOR, glut_RUNTIME_VERSION_MINOR,
			version.major, version.minor, glut_err_version);

	/* Initialize */
	glut_window_init();

	/* Initialize OpenGL context */
	opengl_context_init();
}


void glutInitDisplayMode(unsigned int mode)
{
	glut_initial_display_mode = mode;
}


void glutInitDisplayString(const char *string)
{
	__X86_GLUT_NOT_IMPL__
}


void glutInitWindowPosition(int x, int y)
{
	glut_initial_window_position_x = x;
	glut_initial_window_position_y = y;
}


void glutInitWindowSize(int width, int height)
{
	glut_initial_window_width = width;
	glut_initial_window_height = height;
}


enum glut_event_type_t
{
	glut_event_invalid = 0,
	glut_event_display,
	glut_event_overlay_display,
	glut_event_reshape,
	glut_event_keyboard,
	glut_event_mouse,
	glut_event_motion,
	glut_event_passive_motion,
	glut_event_visibility,
	glut_event_entry,
	glut_event_special,
	glut_event_spaceball_motion,
	glut_event_spaceball_rotate,
	glut_event_spaceball_button,
	glut_event_button_box,
	glut_event_dials,
	glut_event_tablet_motion,
	glut_event_tablet_button,
	glut_event_menu_status,
	glut_event_idle,
	glut_event_close,
	glut_event_timer
};

struct glut_event_t
{
	enum glut_event_type_t type;

	union
	{
		struct
		{
			int win;
		} display;

		struct
		{
			int win;
			int width;
			int height;
		} reshape;

		struct
		{
			int win;
		} overlay_display;

		struct
		{
			int win;
			unsigned char key;
			int x;
			int y;
		} keyboard;

		struct
		{
			int win;
			int button;
			int state;
			int x;
			int y;
		} mouse;

		struct
		{
			int win;
			int x;
			int y;
		} motion;

		struct
		{
			int win;
			int state;
		} visibility;

		struct
		{
			int win;
			int state;
		} entry;

		struct
		{
			int win;
			int key;
			int x;
			int y;
		} special;

		struct
		{
			int win;
			int x;
			int y;
			int z;
		} spaceball_motion;

		struct
		{
			int win;
			int x;
			int y;
			int z;
		} spaceball_rotate;

		struct
		{
			int win;
			int button;
			int state;
		} spaceball_button;

		struct
		{
			int win;
			int button;
			int state;
		} button_box;

		struct
		{
			int win;
			int dial;
			int value;
		} dials;

		struct
		{
			int win;
			int x;
			int y;
		} tablet_motion;

		struct
		{
			int win;
			int button;
			int state;
			int x;
			int y;
		} tablet_button;

		struct
		{
			int status;
			int x;
			int y;
		} menu_status;

		struct
		{
			int value;
		} timer;
	} u;
};

void glutMainLoop(void)
{
	struct glut_event_t event;
	struct glut_window_t *window;

	/* GLUT guest loop */
	for (;;)
	{
		/* Get a new event */
		syscall(GLUT_SYSCALL_CODE, glut_call_get_event, &event);

		/* Process event */
		switch (event.type)
		{

		case glut_event_display:
		{
			window = glut_window_get_by_host_id(event.u.display.win);
			if (!window)
				panic("libm2s-glut: display event: invalid window");
			if (window->display_func)
				window->display_func();
			break;
		}

		case glut_event_reshape:
		{
			window = glut_window_get_by_host_id(event.u.reshape.win);
			if (!window)
				panic("libm2s-glut: reshape event: invalid window");
			if (window->reshape_func)
				window->reshape_func(event.u.reshape.width,
					event.u.reshape.height);
			break;
		}

		case glut_event_keyboard:
		{
			window = glut_window_get_by_host_id(event.u.keyboard.win);
			if (!window)
				panic("libm2s-glut: keyboard event: invalid window");
			if (window->keyboard_func)
				window->keyboard_func(event.u.keyboard.key,
					event.u.keyboard.x, event.u.keyboard.y);
			break;
		}

		case glut_event_motion:
		{
			window = glut_window_get_by_host_id(event.u.motion.win);
			if (!window)
				panic("libm2s-glut: motion event: invalid window");
			if (window->motion_func)
				window->motion_func(event.u.motion.x, event.u.motion.y);
			break;
		}

		case glut_event_passive_motion:
		{
			window = glut_window_get_by_host_id(event.u.motion.win);
			if (!window)
				panic("libm2s-glut: passive motion event: invalid window");
			if (window->passive_motion_func)
				window->passive_motion_func(event.u.motion.x, event.u.motion.y);
			break;
		}

		case glut_event_visibility:
		{
			window = glut_window_get_by_host_id(event.u.visibility.win);
			if (!window)
				panic("libm2s-glut: visibility event: invalid window");
			if (window->visibility_func)
				window->visibility_func(event.u.visibility.state);
			break;
		}

		case glut_event_entry:
		{
			window = glut_window_get_by_host_id(event.u.entry.win);
			if (!window)
				panic("libm2s-glut: entry event: invalid window");
			if (window->entry_func)
				window->entry_func(event.u.entry.state);
			break;
		}

		case glut_event_idle:

			break;

		default:
			opengl_context_destroy();
			fatal("GLUT event not supported (type=%d).\n%s",
				event.type, glut_err_not_impl);
		}
	}
}


struct glut_window_properties_t
{
	int x;
	int y;
	int width;
	int height;
};


int glutCreateWindow(const char *title)
{
	struct glut_window_t *window;
	struct glut_window_properties_t properties;

	/* Create window */
	window = glut_window_create((char *) title);
	glut_current_window = window;

	/* Create host window */
	properties.x = glut_initial_window_position_x;
	properties.y = glut_initial_window_position_y;
	properties.width = glut_initial_window_width;
	properties.height = glut_initial_window_height;
	window->host_id = syscall(GLUT_SYSCALL_CODE, glut_call_new_window,
		(char *) title, &properties);

	/* Return guest ID */
	return window->guest_id;
}


int glutCreateSubWindow(int win, int x, int y, int width, int height)
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}


void glutDestroyWindow(int win)
{
	__X86_GLUT_NOT_IMPL__
}


void glutPostRedisplay(void)
{
	/* FIXME : mark the current window as needing to be redisplayed */
}


void glutPostWindowRedisplay(int win)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSwapBuffers(void)
{
	/* FIXME */
}


int glutGetWindow(void)
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}


void glutSetWindow(int win)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSetWindowTitle(const char *title)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSetIconTitle(const char *title)
{
	__X86_GLUT_NOT_IMPL__
}


void glutPositionWindow(int x, int y)
{
	__X86_GLUT_NOT_IMPL__
}


void glutReshapeWindow(int width, int height)
{
	__X86_GLUT_NOT_IMPL__
}


void glutPopWindow(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutPushWindow(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutIconifyWindow(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutShowWindow(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutHideWindow(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutFullScreen(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSetCursor(int cursor)
{
	__X86_GLUT_NOT_IMPL__
}


void glutWarpPointer(int x, int y)
{
	__X86_GLUT_NOT_IMPL__
}


void glutEstablishOverlay(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutRemoveOverlay(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutUseLayer(GLenum layer)
{
	__X86_GLUT_NOT_IMPL__
}


void glutPostOverlayRedisplay(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutPostWindowOverlayRedisplay(int win)
{
	__X86_GLUT_NOT_IMPL__
}


void glutShowOverlay(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutHideOverlay(void)
{
	__X86_GLUT_NOT_IMPL__
}


int glutCreateMenu(void (*func)(int))
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}


void glutDestroyMenu(int menu)
{
	__X86_GLUT_NOT_IMPL__
}


int glutGetMenu(void)
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}


void glutSetMenu(int menu)
{
	__X86_GLUT_NOT_IMPL__
}


void glutAddMenuEntry(const char *label, int value)
{
	__X86_GLUT_NOT_IMPL__
}


void glutAddSubMenu(const char *label, int submenu)
{
	__X86_GLUT_NOT_IMPL__
}


void glutChangeToMenuEntry(int item, const char *label, int value)
{
	__X86_GLUT_NOT_IMPL__
}


void glutChangeToSubMenu(int item, const char *label, int submenu)
{
	__X86_GLUT_NOT_IMPL__
}


void glutRemoveMenuItem(int item)
{
	__X86_GLUT_NOT_IMPL__
}


void glutAttachMenu(int button)
{
	__X86_GLUT_NOT_IMPL__
}


void glutDetachMenu(int button)
{
	__X86_GLUT_NOT_IMPL__
}


void glutDisplayFunc(void (*func)(void))
{
	if (glut_current_window)
		glut_current_window->display_func = func;
}


void glutReshapeFunc(void (*func)(int width, int height))
{
	if (glut_current_window)
		glut_current_window->reshape_func = func;
}


void glutKeyboardFunc(void (*func)(unsigned char key, int x, int y))
{
	if (glut_current_window)
		glut_current_window->keyboard_func = func;
}


void glutMouseFunc(void (*func)(int button, int state, int x, int y))
{
	__X86_GLUT_NOT_IMPL__
}


void glutMotionFunc(void (*func)(int x, int y))
{
	__X86_GLUT_NOT_IMPL__
}


void glutPassiveMotionFunc(void (*func)(int x, int y))
{
	__X86_GLUT_NOT_IMPL__
}


void glutEntryFunc(void (*func)(int state))
{
	__X86_GLUT_NOT_IMPL__
}


void glutVisibilityFunc(void (*func)(int state))
{
	if (glut_current_window)
		glut_current_window->visibility_func = func;	
}


void glutIdleFunc(void (*func)(void))
{
	if (glut_current_window)
		glut_current_window->idle_func = func;	
}


void glutTimerFunc(unsigned int millis, void (*func)(int value), int value)
{
	glut_debug(stdout, "Glut time function: millis %d, function %p", millis, func);
	if (glut_current_window)
		glut_current_window->timer_func = func;
}


void glutMenuStateFunc(void (*func)(int state))
{
	__X86_GLUT_NOT_IMPL__
}


void glutSpecialFunc(void (*func)(int key, int x, int y))
{
	if (glut_current_window)
		glut_current_window->special_func = func;

}


void glutSpaceballMotionFunc(void (*func)(int x, int y, int z))
{
	__X86_GLUT_NOT_IMPL__
}


void glutSpaceballRotateFunc(void (*func)(int x, int y, int z))
{
	__X86_GLUT_NOT_IMPL__
}


void glutSpaceballButtonFunc(void (*func)(int button, int state))
{
	__X86_GLUT_NOT_IMPL__
}


void glutButtonBoxFunc(void (*func)(int button, int state))
{
	__X86_GLUT_NOT_IMPL__
}


void glutDialsFunc(void (*func)(int dial, int value))
{
	__X86_GLUT_NOT_IMPL__
}


void glutTabletMotionFunc(void (*func)(int x, int y))
{
	__X86_GLUT_NOT_IMPL__
}


void glutTabletButtonFunc(void (*func)(int button, int state, int x, int y))
{
	__X86_GLUT_NOT_IMPL__
}


void glutMenuStatusFunc(void (*func)(int status, int x, int y))
{
	__X86_GLUT_NOT_IMPL__
}


void glutOverlayDisplayFunc(void (*func)(void))
{
	__X86_GLUT_NOT_IMPL__
}


void glutWindowStatusFunc(void (*func)(int state))
{
	__X86_GLUT_NOT_IMPL__
}


void glutKeyboardUpFunc(void (*func)(unsigned char key, int x, int y))
{
	__X86_GLUT_NOT_IMPL__
}


void glutSpecialUpFunc(void (*func)(int key, int x, int y))
{
	__X86_GLUT_NOT_IMPL__
}


void glutJoystickFunc(void (*func)(unsigned int buttonMask, int x, int y, int z), int pollInterval)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSetColor(int cell, GLfloat red, GLfloat green, GLfloat blue)
{
	__X86_GLUT_NOT_IMPL__
}


GLfloat glutGetColor(int ndx, int component)
{
	__X86_GLUT_NOT_IMPL__
	return 0.0;
}


void glutCopyColormap(int win)
{
	__X86_GLUT_NOT_IMPL__
}


int glutGet(GLenum type)
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}


int glutDeviceGet(GLenum type)
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}

int glutGetModifiers(void)
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}


int glutLayerGet(GLenum type)
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}


void glutBitmapCharacter(void *font, int character)
{
	__X86_GLUT_NOT_IMPL__
}


int glutBitmapWidth(void *font, int character)
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}


void glutStrokeCharacter(void *font, int character)
{
	__X86_GLUT_NOT_IMPL__
}


int glutStrokeWidth(void *font, int character)
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}


int glutBitmapLength(void *font, const unsigned char *string)
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}


int glutStrokeLength(void *font, const unsigned char *string)
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}


void glutWireSphere(GLdouble radius, GLint slices, GLint stacks)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSolidSphere(GLdouble radius, GLint slices, GLint stacks)
{
	__X86_GLUT_NOT_IMPL__
}


void glutWireCone(GLdouble base, GLdouble height, GLint slices, GLint stacks)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSolidCone(GLdouble base, GLdouble height, GLint slices, GLint stacks)
{
	__X86_GLUT_NOT_IMPL__
}


void glutWireCube(GLdouble size)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSolidCube(GLdouble size)
{
	__X86_GLUT_NOT_IMPL__
}


void glutWireTorus(GLdouble innerRadius, GLdouble outerRadius, GLint sides, GLint rings)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSolidTorus(GLdouble innerRadius, GLdouble outerRadius, GLint sides, GLint rings)
{
	__X86_GLUT_NOT_IMPL__
}


void glutWireDodecahedron(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSolidDodecahedron(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutWireTeapot(GLdouble size)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSolidTeapot(GLdouble size)
{
	__X86_GLUT_NOT_IMPL__
}


void glutWireOctahedron(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSolidOctahedron(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutWireTetrahedron(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSolidTetrahedron(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutWireIcosahedron(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSolidIcosahedron(void)
{
	__X86_GLUT_NOT_IMPL__
}


int glutVideoResizeGet(GLenum param)
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}


void glutSetupVideoResizing(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutStopVideoResizing(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutVideoResize(int x, int y, int width, int height)
{
	__X86_GLUT_NOT_IMPL__
}


void glutVideoPan(int x, int y, int width, int height)
{
	__X86_GLUT_NOT_IMPL__
}

void glutIgnoreKeyRepeat(int ignore)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSetKeyRepeat(int repeatMode)
{
	__X86_GLUT_NOT_IMPL__
}


void glutForceJoystickFunc(void)
{
	__X86_GLUT_NOT_IMPL__
}


void glutGameModeString(const char *string)
{
	__X86_GLUT_NOT_IMPL__
}


int glutEnterGameMode(void)
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}


void glutLeaveGameMode(void)
{
	__X86_GLUT_NOT_IMPL__
}


int glutGameModeGet(GLenum mode)
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}

int glutExtensionSupported(const char *extension)
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}

void glutReportErrors(void)
{
	__X86_GLUT_NOT_IMPL__
}

void glutMainLoopEvent( void )
{
	__X86_GLUT_NOT_IMPL__
}

void glutLeaveMainLoop( void )
{
	__X86_GLUT_NOT_IMPL__
}

void glutExit ( void )
{
	__X86_GLUT_NOT_IMPL__
}

void glutFullScreenToggle( void )
{
	__X86_GLUT_NOT_IMPL__
}

void glutMouseWheelFunc( void (* callback)( int, int, int, int ) )
{
	__X86_GLUT_NOT_IMPL__
}

void glutCloseFunc( void (* callback)( void ) )
{
	glut_debug(stdout, "Glut close func: %p\n", callback);
	if (glut_current_window)
		glut_current_window->close_func = callback;	

}

void glutWMCloseFunc( void (* callback)( void ) )
{
	__X86_GLUT_NOT_IMPL__
}

void glutMenuDestroyFunc( void (* callback)( void ) )
{
	__X86_GLUT_NOT_IMPL__
}

void glutSetOption ( GLenum option_flag, int value )
{
	glut_debug(stdout, "Glut set option: flag %d, value %d", option_flag, value);
}

int * glutGetModeValues(GLenum mode, int * size)
{
	__X86_GLUT_NOT_IMPL__
	return NULL;
}

void* glutGetWindowData( void )
{
	__X86_GLUT_NOT_IMPL__
	return NULL;
}

void glutSetWindowData(void* data)
{
	__X86_GLUT_NOT_IMPL__
}

void* glutGetMenuData( void )
{
	__X86_GLUT_NOT_IMPL__
	return NULL;
}

void glutSetMenuData(void* data)
{
	__X86_GLUT_NOT_IMPL__
}

int glutBitmapHeight( void* font )
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}

GLfloat glutStrokeHeight( void* font )
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}

void glutBitmapString( void* font, const unsigned char *string )
{
	__X86_GLUT_NOT_IMPL__
}

void glutStrokeString( void* font, const unsigned char *string )
{
	__X86_GLUT_NOT_IMPL__
}

void glutWireRhombicDodecahedron( void )
{
	__X86_GLUT_NOT_IMPL__
}

void glutSolidRhombicDodecahedron( void )
{
	__X86_GLUT_NOT_IMPL__
}

void glutWireSierpinskiSponge ( int num_levels, GLdouble offset[3], GLdouble scale )
{
	__X86_GLUT_NOT_IMPL__
}

void glutSolidSierpinskiSponge ( int num_levels, GLdouble offset[3], GLdouble scale )
{
	__X86_GLUT_NOT_IMPL__
}

void glutWireCylinder( GLdouble radius, GLdouble height, GLint slices, GLint stacks)
{
	__X86_GLUT_NOT_IMPL__
}

void glutSolidCylinder( GLdouble radius, GLdouble height, GLint slices, GLint stacks)
{
	__X86_GLUT_NOT_IMPL__
}

int glutJoystickGetNumAxes( int ident )
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}

int glutJoystickGetNumButtons( int ident )
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}

int glutJoystickNotWorking( int ident )
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}

float glutJoystickGetDeadBand( int ident, int axis )
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}

void glutJoystickSetDeadBand( int ident, int axis, float db )
{
	__X86_GLUT_NOT_IMPL__
}

float glutJoystickGetSaturation( int ident, int axis )
{
	__X86_GLUT_NOT_IMPL__
	return 0;
}

void glutJoystickSetSaturation( int ident, int axis, float st )
{
	__X86_GLUT_NOT_IMPL__
}

void glutJoystickSetMinRange( int ident, float *axes )
{
	__X86_GLUT_NOT_IMPL__
}

void glutJoystickSetMaxRange( int ident, float *axes )
{
	__X86_GLUT_NOT_IMPL__
}

void glutJoystickSetCenter( int ident, float *axes )
{
	__X86_GLUT_NOT_IMPL__
}

void glutJoystickGetMinRange( int ident, float *axes )
{
	__X86_GLUT_NOT_IMPL__
}

void glutJoystickGetMaxRange( int ident, float *axes )
{
	__X86_GLUT_NOT_IMPL__
}

void glutJoystickGetCenter( int ident, float *axes )
{
	__X86_GLUT_NOT_IMPL__
}

void glutInitContextVersion( int majorVersion, int minorVersion )
{
	opengl_context_props_set_version(majorVersion, minorVersion, opengl_ctx->props);
	glut_debug(stdout, "Initialize OpenGL context version %d.%d\n", majorVersion, minorVersion);
}

void glutInitContextFlags( int flags )
{
	glut_debug(stdout, "Initialize OpenGL context flags: %d\n", flags);
}

void glutInitContextProfile( int profile )
{
	glut_debug(stdout, "Initialize OpenGL context profile: %d\n", profile);
}

