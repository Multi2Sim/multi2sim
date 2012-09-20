#include <opengl-vertex.h>

struct x86_opengl_vertex_t *x86_opengl_vertex_create(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	struct x86_opengl_vertex_t *vtx;

	vtx = calloc(1, sizeof(struct x86_opengl_vertex_t));

	/* Initialize */
	vtx->x = x;
	vtx->y = y;
	vtx->z = z;
	vtx->w = w;

	x86_opengl_debug("\t\tCreate vertex \t[%f, %f, %f, %f]\n", vtx->x, vtx->y, vtx->z, vtx->w);

	return vtx;
}
void x86_opengl_vertex_free(struct x86_opengl_vertex_t *vtx)
{
	x86_opengl_debug("\t\tFree vertex \t[%f, %f, %f, %f]\n", vtx->x, vtx->y, vtx->z, vtx->w);

	free(vtx);
}

struct x86_opengl_vertex_group_t *x86_opengl_vertex_group_create(GLenum primitive_type)
{
	struct x86_opengl_vertex_group_t * vtxgp;

	vtxgp = calloc(1, sizeof(struct x86_opengl_vertex_group_t));

	/* Initialize */
	vtxgp->primitive_type = primitive_type;
	vtxgp->vertex_list = list_create();

	x86_opengl_debug("\t\tPrimitive group type %d\n", primitive_type);

	/* Return */	
	return vtxgp;
}

void x86_opengl_vertex_group_free(struct x86_opengl_vertex_group_t *vtxgp)
{
	/* Free vertices in the list */
	while (list_count(vtxgp->vertex_list))
		x86_opengl_vertex_free(list_remove_at(vtxgp->vertex_list, 0));	

	list_free(vtxgp->vertex_list);
	free(vtxgp);
}

static void x86_opengl_vertex_group_add_vertex(struct x86_opengl_vertex_group_t *vtxgp, struct x86_opengl_vertex_t *vtx)
{
	list_add(vtxgp->vertex_list, vtx);
}

struct x86_opengl_vertex_buffer_t *x86_opengl_vertex_buffer_create()
{
	struct x86_opengl_vertex_buffer_t *vtxbf;

	vtxbf = calloc(1, sizeof(struct x86_opengl_vertex_buffer_t));

	/* Initialize */
	vtxbf->vertex_groups = list_create();
	vtxbf->current_vertex_group = list_tail(vtxbf->vertex_groups);

	/* Return */
	return vtxbf;
}
void x86_opengl_vertex_buffer_free(struct x86_opengl_vertex_buffer_t *vtxbf)
{
	if (vtxbf)
	{
		while (list_count(vtxbf->vertex_groups))
			x86_opengl_vertex_group_free(list_remove_at(vtxbf->vertex_groups, 0));

		list_free(vtxbf->vertex_groups);
		free(vtxbf);
	}
}

void x86_opengl_vertex_buffer_add_vertex_group(struct x86_opengl_vertex_buffer_t *vtxbf, struct x86_opengl_vertex_group_t *vtxgp)
{
	list_add(vtxbf->vertex_groups, vtxgp);
	vtxbf->current_vertex_group = vtxgp;
}

void x86_opengl_vertex_buffer_add_vertex(struct x86_opengl_vertex_buffer_t *vtxbf, struct x86_opengl_vertex_t *vtx)
{
	x86_opengl_debug("\t\tAdd vertex \t[%f, %f, %f, %f]\n", vtx->x, vtx->y, vtx->z, vtx->w);

	x86_opengl_vertex_group_add_vertex(vtxbf->current_vertex_group, vtx);
}
