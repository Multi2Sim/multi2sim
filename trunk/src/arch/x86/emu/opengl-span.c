#include "opengl-span.h"

struct x86_opengl_span_t *x86_opengl_span_create()
{
	struct x86_opengl_span_t *spn;

	spn = calloc(1, sizeof(struct x86_opengl_span_t));
	if (!spn)
		fatal("%s: out of memory", __FUNCTION__);

	return spn;
}

void x86_opengl_span_free(struct x86_opengl_span_t *spn)
{
	free(spn);
}

void x86_opengl_span_interpolate_z(struct x86_opengl_span_t *spn)
{
	const GLuint n = spn->end;
	GLuint i;

	/* Deep Z buffer, no fixed->int shift */
	GLuint zval = spn->z;
	GLuint *z = spn->array->z;
	for (i = 0; i < n; i++) 
	{
		z[i] = zval;
		zval += spn->zStep;
	}

}
