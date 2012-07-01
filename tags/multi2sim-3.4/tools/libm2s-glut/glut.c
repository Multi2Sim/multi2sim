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
#include <list.h>
#include <misc.h>
#include <unistd.h>



/*
 * Error Messages
 */

#define __X86_GLUT_NOT_IMPL__  fatal("%s: not implemented.\n%s", \
	__FUNCTION__, err_x86_glut_not_impl);


static char *err_x86_glut_not_impl =
	"\tMulti2Sim provides partial support for the OpenGL Utility Toolkit (GLUT)\n"
	"\tlibrary. To request the implementation of a certain functionality, please\n"
	"\temail development@multi2sim.org.\n";

static char *err_x86_glut_version =
	"\tYour guest application is using a version of the GLUT runtime library\n"
	"\tthat is incompatible with this version of Multi2Sim. Please download the\n"
	"\tlatest Multi2Sim version, and recompile your application with the latest\n"
	"\tGLUT runtime library ('libm2s-glut').\n";

static char *err_x86_glut_native =
	"\tYou are trying to run natively an application using the Multi2Sim GLUT\n"
	"\tlibrary implementation ('libm2s-glut'). Please run this program on top of\n"
	"\tMulti2Sim.\n";




/*
 * GLUT Internal State
 */

unsigned int x86_glut_initial_display_mode;

int x86_glut_initial_window_position_x = -1;
int x86_glut_initial_window_position_y = -1;
int x86_glut_initial_window_width = 300;
int x86_glut_initial_window_height = 300;

struct x86_glut_window_t *x86_glut_current_window = NULL;



/* Multi2Sim GLUT Runtime required */
#define X86_GLUT_RUNTIME_VERSION_MAJOR	1
#define X86_GLUT_RUNTIME_VERSION_MINOR	690

struct x86_glut_version_t
{
	int major;
	int minor;
};

void glutInit(int *argcp, char **argv)
{
	struct x86_glut_version_t version;

	int ret;

	/* Runtime function 'init' */
	ret = syscall(X86_GLUT_SYS_CODE, x86_glut_call_init, &version);

	/* Check that we are running on Multi2Sim. If a program linked with this library
	 * is running natively, system call X86_GLUT_SYS_CODE is not supported. */
	if (ret)
		fatal("native execution not supported.\n%s",
			err_x86_glut_native);

	/* Check that exact major version matches */
	if (version.major != X86_GLUT_RUNTIME_VERSION_MAJOR
			|| version.minor < X86_GLUT_RUNTIME_VERSION_MINOR)
		fatal("incompatible GLUT runtime versions.\n"
			"\tGuest library v. %d.%d / Host implementation v. %d.%d.\n"
			"%s", X86_GLUT_RUNTIME_VERSION_MAJOR, X86_GLUT_RUNTIME_VERSION_MINOR,
			version.major, version.minor, err_x86_glut_version);

	/* Initialize */
	x86_glut_window_init();
}


void glutInitDisplayMode(unsigned int mode)
{
	x86_glut_initial_display_mode = mode;
}


void glutInitDisplayString(const char *string)
{
	__X86_GLUT_NOT_IMPL__
}


void glutInitWindowPosition(int x, int y)
{
	x86_glut_initial_window_position_x = x;
	x86_glut_initial_window_position_y = y;
}


void glutInitWindowSize(int width, int height)
{
	x86_glut_initial_window_width = width;
	x86_glut_initial_window_height = height;
}


enum x86_glut_event_type_t
{
	x86_glut_event_invalid = 0,
	x86_glut_event_display,
	x86_glut_event_overlay_display,
	x86_glut_event_reshape,
	x86_glut_event_keyboard,
	x86_glut_event_mouse,
	x86_glut_event_motion,
	x86_glut_event_passive_motion,
	x86_glut_event_visibility,
	x86_glut_event_entry,
	x86_glut_event_special,
	x86_glut_event_spaceball_motion,
	x86_glut_event_spaceball_rotate,
	x86_glut_event_spaceball_button,
	x86_glut_event_button_box,
	x86_glut_event_dials,
	x86_glut_event_tablet_motion,
	x86_glut_event_tablet_button,
	x86_glut_event_menu_status,
	x86_glut_event_idle,
	x86_glut_event_timer
};

struct x86_glut_event_t
{
	enum x86_glut_event_type_t type;

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
	struct x86_glut_event_t event;
	struct x86_glut_window_t *window;

	/* GLUT guest loop */
	for (;;)
	{
		/* Get a new event */
		syscall(X86_GLUT_SYS_CODE, x86_glut_call_get_event, &event);

		/* Process event */
		switch (event.type)
		{

		case x86_glut_event_display:
		{
			window = x86_glut_window_get_by_host_id(event.u.display.win);
			if (!window)
				panic("libm2s-glut: display event: invalid window");
			if (window->display_func)
				window->display_func();
			break;
		}

		case x86_glut_event_reshape:
		{
			window = x86_glut_window_get_by_host_id(event.u.reshape.win);
			if (!window)
				panic("libm2s-glut: reshape event: invalid window");
			if (window->reshape_func)
				window->reshape_func(event.u.reshape.width,
					event.u.reshape.height);
			break;
		}

		case x86_glut_event_keyboard:
		{
			window = x86_glut_window_get_by_host_id(event.u.keyboard.win);
			if (!window)
				panic("libm2s-glut: keyboard event: invalid window");
			if (window->keyboard_func)
				window->keyboard_func(event.u.keyboard.key,
					event.u.keyboard.x, event.u.keyboard.y);
			break;
		}

		case x86_glut_event_motion:
		{
			window = x86_glut_window_get_by_host_id(event.u.motion.win);
			if (!window)
				panic("libm2s-glut: motion event: invalid window");
			if (window->motion_func)
				window->motion_func(event.u.motion.x, event.u.motion.y);
			break;
		}

		case x86_glut_event_passive_motion:
		{
			window = x86_glut_window_get_by_host_id(event.u.motion.win);
			if (!window)
				panic("libm2s-glut: passive motion event: invalid window");
			if (window->passive_motion_func)
				window->passive_motion_func(event.u.motion.x, event.u.motion.y);
			break;
		}

		case x86_glut_event_visibility:
		{
			window = x86_glut_window_get_by_host_id(event.u.visibility.win);
			if (!window)
				panic("libm2s-glut: visibility event: invalid window");
			if (window->visibility_func)
				window->visibility_func(event.u.visibility.state);
			break;
		}

		case x86_glut_event_entry:
		{
			window = x86_glut_window_get_by_host_id(event.u.entry.win);
			if (!window)
				panic("libm2s-glut: entry event: invalid window");
			if (window->entry_func)
				window->entry_func(event.u.entry.state);
			break;
		}

		case x86_glut_event_idle:

			break;

		default:
			fatal("GLUT event not supported (type=%d).\n%s",
				event.type, err_x86_glut_not_impl);
		}
	}
}


struct x86_glut_window_properties_t
{
	int x;
	int y;
	int width;
	int height;
};


int glutCreateWindow(const char *title)
{
	struct x86_glut_window_t *window;
	struct x86_glut_window_properties_t properties;

	/* Create window */
	window = x86_glut_window_create((char *) title);
	x86_glut_current_window = window;

	/* Create host window */
	properties.x = x86_glut_initial_window_position_x;
	properties.y = x86_glut_initial_window_position_y;
	properties.width = x86_glut_initial_window_width;
	properties.height = x86_glut_initial_window_height;
	window->host_id = syscall(X86_GLUT_SYS_CODE, x86_glut_call_new_window,
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
	__X86_GLUT_NOT_IMPL__
}


void glutPostWindowRedisplay(int win)
{
	__X86_GLUT_NOT_IMPL__
}


void glutSwapBuffers(void)
{
	__X86_GLUT_NOT_IMPL__
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
	if (x86_glut_current_window)
		x86_glut_current_window->display_func = func;
}


void glutReshapeFunc(void (*func)(int width, int height))
{
	if (x86_glut_current_window)
		x86_glut_current_window->reshape_func = func;
}


void glutKeyboardFunc(void (*func)(unsigned char key, int x, int y))
{
	if (x86_glut_current_window)
		x86_glut_current_window->keyboard_func = func;
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
	__X86_GLUT_NOT_IMPL__
}


void glutIdleFunc(void (*func)(void))
{
	__X86_GLUT_NOT_IMPL__
}


void glutTimerFunc(unsigned int millis, void (*func)(int value), int value)
{
	__X86_GLUT_NOT_IMPL__
}


void glutMenuStateFunc(void (*func)(int state))
{
	__X86_GLUT_NOT_IMPL__
}


void glutSpecialFunc(void (*func)(int key, int x, int y))
{
	__X86_GLUT_NOT_IMPL__
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


int glutExtensionSupported(const char *name)
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


void glutReportErrors(void)
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


