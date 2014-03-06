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

#include <src/lib/cpp/Misc.h>
#include "PrimitiveAssembler.h"

using namespace misc;

namespace SI
{

PrimAsmVertex::PrimAsmVertex(float x, float y, float z, float w)
{
	pos[0] = x;
	pos[1] = y;
	pos[2] = z;
	pos[3] = w;
}

PrimAsmEdge::PrimAsmEdge(const PrimAsmVertex &vtx0, const PrimAsmVertex &vtx1)
{
	a = vtx0.getY() - vtx1.getY();
	b = vtx1.getX() - vtx0.getX();
	c = -0.5 * (a * (vtx0.getX() + vtx1.getX()) + 
		b * (vtx0.getY() + vtx1.getY()));
}

Primitive::Primitive(PrimAsmMode mode, std::vector<std::unique_ptr<float>> pos_repo, 
	int x, int y, int width, int height)
{
	switch (mode)
	{

	case OpenGLPaTriangles:
	{
		if (pos_repo.size() % 12)
		{
			panic("Position data error, must be multiples of 12.");
			return;
		}
		for (unsigned i = 0; i < pos_repo.size() / 12; ++i)
		{
			int loc = i * 12;

			// Create 3 vetices
			PrimAsmVertex vtx0(*pos_repo[loc + 0], *pos_repo[loc + 1], *pos_repo[loc + 2], *pos_repo[loc + 3]);
			PrimAsmVertex vtx1(*pos_repo[loc + 4], *pos_repo[loc + 5], *pos_repo[loc + 6], *pos_repo[loc + 7]);
			PrimAsmVertex vtx2(*pos_repo[loc + 8], *pos_repo[loc + 9], *pos_repo[loc + 10], *pos_repo[loc + 11]);

			// Apply viewport transformation
			vtx0.ApplyViewPort(x, y, width, height);
			vtx1.ApplyViewPort(x, y, width, height);
			vtx2.ApplyViewPort(x, y, width, height);

			// Add to triangle repository
			triangles.push_back(std::unique_ptr<PrimAsmTriangle> (new PrimAsmTriangle(vtx0, vtx1, vtx2)));

		}
		break;
	}

	default:
		panic("Mode(%d) is not supported by Primitive Assembler.", mode);
		break;
	}
}

}  // namespace SI