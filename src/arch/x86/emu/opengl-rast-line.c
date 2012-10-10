#include "opengl-rast-line.h"

/* TODO: interpolate */
/* Bresenham's line algorithm */
void x86_opengl_rasterizer_draw_line(struct x86_opengl_context_t *ctx, GLint x1, GLint y1, GLint x2, GLint y2, GLuint color)
{

	GLint s_x;
	GLint s_y;
	GLint e_x;
	GLint e_y;

	if (x1 == x2)
	{
		x86_opengl_debug("\t\tSlope = infinite\n");
		s_y = y1 < y2 ? y1 : y2;
		e_y = y1 > y2 ? y1: y2;
		x86_glut_frame_buffer_pixel(x1, s_y, color);		
		while(s_y < e_y)
		{ 
			s_y++;
			x86_glut_frame_buffer_pixel(x1, s_y, color);				
		}
		return;
	}

	if (x1 < x2)
	{
		s_x = x1;
		s_y = y1;
		e_x = x2;
		e_y = y2;
	} else {
		s_x = x2;
		s_y = y2;
		e_x = x1;
		e_y = y1;
	}

	GLfloat m = (GLfloat)(y2 - y1) / (x2 - x1);
	x86_opengl_debug("\t\tSlope = %f\n", m);

	if (m >= 0.0f && m < 1.0f)
	{
		GLint dx = abs(e_x - s_x);
		GLint dy = abs(e_y - s_y);
		GLint e = 2*dy -dx;
		GLint incrE = 2*dy;
		GLint incrNE = 2*(dy-dx);
		GLint x = s_x;
		GLint y = s_y;
		x86_glut_frame_buffer_pixel(x, y, color);
		while(x < e_x)
		{
			x++;
			if ( e <= 0)
				e += incrE;
			else
			{
				y++;
				e += incrNE;
			}
			x86_glut_frame_buffer_pixel(x, y, color);
		}
	}

	if ( m >= 1.0f )
	{
		GLint dx = abs(e_x - s_x);
		GLint dy = abs(e_y - s_y);
		GLint e = 2*dx -dy;
		GLint incrE = 2*dx;
		GLint incrNE = 2*(dx-dy);
		GLint x = s_x;
		GLint y = s_y;
		x86_glut_frame_buffer_pixel(x, y, color);
		while(y < e_y)
		{
			y++;
			if ( e <= 0)
				e += incrE;
			else
			{
				x++;
				e += incrNE;
			}
			x86_glut_frame_buffer_pixel(x, y, color);
		}
	}

	if (m < 0.0f && m > -1.0f)
	{
		GLint dx = abs(e_x - s_x);
		GLint dy = abs(e_y - s_y);
		GLint e = 2*dy -dx;
		GLint incrE = 2*dy;
		GLint incrNE = 2*(dy-dx);
		GLint x = s_x;
		GLint y = s_y;
		x86_glut_frame_buffer_pixel(x, y, color);
		while(x < e_x)
		{
			x++;
			if ( e <= 0)
				e += incrE;
			else
			{
				y--;
				e += incrNE;
			}
			x86_glut_frame_buffer_pixel(x, y, color);
		}
	}

	if (m <= -1.0f)
	{
		GLint dx = abs(e_x - s_x);
		GLint dy = abs(e_y - s_y);
		GLint e = 2*dx -dy;
		GLint incrE = 2*dx;
		GLint incrNE = 2*(dx-dy);
		GLint x = s_x;
		GLint y = s_y;
		x86_glut_frame_buffer_pixel(x, y, color);
		while(x < e_x)
		{
			y--;
			if ( e >= 0)
				e -= incrE;
			else
			{
				x++;
				e -= incrNE;
			}
			x86_glut_frame_buffer_pixel(x, y, color);
		}
	}
}
