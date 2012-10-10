#include "opengl-edge.h"

struct x86_opengl_edge_t *x86_opengl_edge_create(struct x86_opengl_vertex_t *vtx0, struct x86_opengl_vertex_t *vtx1)
{
	struct x86_opengl_edge_t * edge;

	edge = calloc(1, sizeof(struct x86_opengl_edge_t));
	if (!edge)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	edge->vtx0 = vtx0;
	edge->vtx1 = vtx1;
	edge->dx = vtx1->x - vtx0->x;
	edge->dy = vtx1->y - vtx0->y;
	edge->dxdy = (GLfloat) edge->dx / edge->dy;
	edge->fdxdy = SignedFloatToFixed(edge->dxdy);
	x86_opengl_debug("\t\tEdge \t[%f, %f] - [%f, %f]\n\t\t\tdx = %f, dy = %f \n\t\t\tdxdy = %f, fdxdy = %d\n", 
		vtx0->x, vtx0->y, vtx1->x, vtx1->y,
		edge->dx, edge->dy,	edge->dxdy, edge->fdxdy);

	/* Return */
	return edge;
}

void x86_opengl_edge_free(struct x86_opengl_edge_t *edge)
{
	free(edge);
}