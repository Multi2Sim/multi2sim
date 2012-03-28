/*
 *  Multi2Sim Tools
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <memvisual-private.h>


/*
 * Network Node
 */

struct vnet_node_t
{
	struct vmod_t *vmod;
};


struct vnet_node_t *vnet_node_create(void)
{
	struct vnet_node_t *node;

	/* Allocate */
	node = calloc(1, sizeof(struct vnet_node_t));
	if (!node)
		fatal("%s: out of memory", __FUNCTION__);

	/* Return */
	return node;
}


void vnet_node_free(struct vnet_node_t *node)
{
	free(node);
}



/*
 * Network
 */

struct vnet_t
{
	char *name;

	struct list_t *node_list;
};

struct vnet_t *vnet_create(char *name, int num_nodes)
{
	struct vnet_t *vnet;

	int i;

	/* Allocate */
	vnet = calloc(1, sizeof(struct vnet_t));
	if (!vnet)
		fatal("%s: out of memory", __FUNCTION__);

	/* Name */
	vnet->name = strdup(name);
	if (!vnet->name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize node list */
	vnet->node_list = list_create();
	for (i = 0; i < num_nodes; i++)
		list_add(vnet->node_list, vnet_node_create());

	/* Return */
	return vnet;
}


void vnet_free(struct vnet_t *vnet)
{
	/* Free node list */
	while (vnet->node_list->count)
		vnet_node_free(list_remove_at(vnet->node_list, 0));
	list_free(vnet->node_list);

	/* Free object */
	free(vnet->name);
	free(vnet);
}


char *vnet_get_name(struct vnet_t *vnet)
{
	return vnet->name;
}


void vnet_attach_vmod(struct vnet_t *vnet, struct vmod_t *vmod, int node_index)
{
	struct vnet_node_t *node;

	/* Check bounds */
	if (!IN_RANGE(node_index, 0, vnet->node_list->count - 1))
		panic("%s: node index out of bounds", __FUNCTION__);

	/* Attach */
	node = list_get(vnet->node_list, node_index);
	node->vmod = vmod;
}


struct vmod_t *vnet_get_vmod(struct vnet_t *vnet, int node_index)
{
	struct vnet_node_t *node;

	/* Check bounds */
	if (!IN_RANGE(node_index, 0, vnet->node_list->count - 1))
		panic("%s: node index out of bounds", __FUNCTION__);

	/* Return */
	node = list_get(vnet->node_list, node_index);
	return node->vmod;
}
