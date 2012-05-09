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


#ifndef M2S_GLUT_H
#define M2S_GLUT_H

#include <GL/gl.h>
#include <GL/glut.h>


/* System call for GLUT runtime */
#define X86_GLUT_SYS_CODE  326


/* List of GLUT runtime calls */
enum x86_glut_call_t
{
	x86_glut_call_invalid,
#define X86_GLUT_DEFINE_CALL(name, code) x86_glut_call_##name = code,
#include "../../src/arch/x86/emu/glut.dat"
#undef X86_GLUT_DEFINE_CALL
	x86_glut_call_count
};




/*
 * GLUT Window
 */

struct x86_glut_window_t
{
	int guest_id;
	int host_id;

	void (*display_func)(void);
	void (*overlay_display_func)(void);
	void (*reshape_func)(int width, int height);
	void (*keyboard_func)(unsigned char key, int x, int y);
	void (*mouse_func)(int button, int state, int x, int y);
	void (*motion_func)(int x, int y);
	void (*passive_motion_func)(int x, int y);
	void (*visibility_func)(int state);
	void (*entry_func)(int state);
	void (*special_func)(int key, int x, int y);
	void (*spaceball_rotate_func)(int x, int y, int z);
	void (*spaceball_button_func)(int button, int state);
	void (*button_box_func)(int button, int state);
	void (*dials_func)(int dial, int value);
	void (*tablet_motion_func)(int x, int y);
	void (*tablet_button_func)(int x, int y);

	char *title;
};



void x86_glut_window_init(void);
void x86_glut_window_done(void);

struct x86_glut_window_t *x86_glut_window_create(char *title);
void x86_glut_window_free(struct x86_glut_window_t *window);

struct x86_glut_window_t *x86_glut_window_get_by_guest_id(int guest_id);
struct x86_glut_window_t *x86_glut_window_get_by_host_id(int host_id);


#endif

