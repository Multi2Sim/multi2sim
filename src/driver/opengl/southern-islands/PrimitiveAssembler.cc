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
#include "ShaderExport.h"
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

Primitive::Primitive(PrimAsmMode mode, std::vector<std::unique_ptr<ExportData>> &export_target, 
	ViewPort *viewport)
{
	switch (mode)
	{

	case OpenGLPaTriangles:
	{
		if (export_target.size() % 3)
		{
			panic("Position data error, must be multiples of 3.");
			return;
		}
		for (unsigned i = 0; i < export_target.size() / 3; ++i)
		{
			// Create 3 vetices
			PrimAsmVertex vtx0(export_target[i]->getXAsFloat(), export_target[i]->getYAsFloat(), 
				export_target[i]->getZAsFloat(), export_target[i]->getWAsFloat());
			PrimAsmVertex vtx1(export_target[i + 1]->getXAsFloat(), export_target[i + 1]->getYAsFloat(), 
				export_target[i + 1]->getZAsFloat(), export_target[i + 1]->getWAsFloat());
			PrimAsmVertex vtx2(export_target[i + 2]->getXAsFloat(), export_target[i + 2]->getYAsFloat(), 
				export_target[i + 2]->getZAsFloat(), export_target[i + 2]->getWAsFloat());

			// Apply viewport transformation
			vtx0.ApplyViewPort(viewport);
			vtx1.ApplyViewPort(viewport);
			vtx2.ApplyViewPort(viewport);

			// Add to triangle repository
			triangles.push_back(std::unique_ptr<PrimAsmTriangle> (new 
				PrimAsmTriangle(vtx0, vtx1, vtx2)));
		}
		break;
	}

	default:
		panic("Assemble mode(%d) is currently not supported.", mode);
		break;
	}
}

}  // namespace SI