/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef DRIVER_OPENGL_SI_PRIMITIVE_ASSEMLBER_H
#define DRIVER_OPENGL_SI_PRIMITIVE_ASSEMLBER_H

#include <memory>
#include <vector>
#include <src/lib/cpp/Misc.h>

#include "ShaderExport.h"

using namespace misc;

namespace SI
{

enum PrimAsmMode
{
	OpenGLPaPoints,
	OpenGLPaLineStrip,
	OpenGLPaLineLoop,
	OpenGLPaLines,
	OpenGLPaLineStripAdjacency,
	OpenGLPaLinesAdjacency,
	OpenGLPaTriangleStrip,
	OpenGLPaTriangleFan,
	OpenGLPaTriangles,
	OpenGLPaTriangleStripAdjacency,
	OpenGLPaTrianglesAdjacency,
	OpenGLPaPatches,
	OpenGLPaInvalid
};

class ViewPort
{
	int x;
	int y;
	int width;
	int height;

public:

	ViewPort(int x, int y, int width, int height) :
		x(x), y(y), width(width), height(height) { };

	/// Getters
	///
	/// Get X
	int getX() const { return x; }

	/// Get Y
	int getY() const { return y; }

	/// Get width
	int getWidth() const { return width; }

	/// Get height
	int getHeight() const { return height; }

	/// Setters
	///
	/// Set viewport
	void setViewport(int x, int y, int width, int height);
};

class DepthRange
{
	double n;
	double f;

public:
	DepthRange(double n, double f) :
		n(n), f(f) { };

	/// Getters
	///
	/// Get n
	double getNear() const { return n; }

	/// Get f
	double getFar() const { return f; }
};

class PrimAsmVertex
{
	// In normalized device coordinate
	float pos[4];

public:
	PrimAsmVertex(float x, float y, float z, float w);

	/// Getters
	///
	/// Get PrimAsmVertex.X
	float getX() const { return pos[0]; }

	/// Get PrimAsmVertex.Y
	float getY() const { return pos[1]; }

	/// Get PrimAsmVertex.Z
	float getZ() const { return pos[2]; }
	
	/// Get PrimAsmVertex.W
	float getW() const { return pos[3]; }

	/// Get by index
	float getComp(int idx) const {
		assert(idx < 4);
		return pos[idx];
	}

	/// Setters
	///
	/// Set PrimAsmVertex.X
	void setX(float value) { pos[0] = value; }

	/// Set PrimAsmVertex.Y
	void setY(float value) { pos[1] = value; }

	/// Set PrimAsmVertex.Z
	void setZ(float value) { pos[2] = value; }

	/// Set PrimAsmVertex.W
	void setW(float value) { pos[3] = value; }

	/// Apply ViewPort(x, y, width, height)
	void ApplyViewPort(ViewPort *viewport) {
		pos[0] = 0.5 * viewport->getWidth() * (pos[0] + 1) + viewport->getX(); 
		pos[1] = 0.5 * viewport->getHeight() * (pos[1] + 1) + viewport->getY();
	}

	/// Apply DepthRange(n, f)
	void ApplyDepthRange(DepthRange *depth) {
		pos[2] = 0.5 * (depth->getFar() - depth->getNear()) * pos[2] 
			+ 0.5 * (depth->getFar() + depth->getNear());
	}

};

class PrimAsmEdge
{
	float a;
	float b;
	float c;
public:
	PrimAsmEdge(const PrimAsmVertex &vtx0, const PrimAsmVertex &vtx1);

};

class PrimAsmTriangle
{
	PrimAsmVertex vtx0;
	PrimAsmVertex vtx1;
	PrimAsmVertex vtx2;

	PrimAsmEdge edge0;
	PrimAsmEdge edge1;
	PrimAsmEdge edge2;

public:
	PrimAsmTriangle(const PrimAsmVertex &vtx0, const PrimAsmVertex &vtx1, 
		const PrimAsmVertex &vtx2) : 
	vtx0(vtx0), 
	vtx1(vtx1), 
	vtx2(vtx2),
	edge0(vtx0, vtx1),
	edge1(vtx1, vtx2),
	edge2(vtx2, vtx0) { }

	/// Getters
	///
	/// Get vertex component
	float getVertexPos(int vertex_idx, int component_idx) const {
		assert(vertex_idx < 3);
		assert(component_idx < 4);
		switch (vertex_idx)
		{
		
		case 0:
			return vtx0.getComp(component_idx);
			break;

		case 1:
			return vtx1.getComp(component_idx);
			break;

		case 2:
			return vtx2.getComp(component_idx);
			break;

		default:
			return 0.0f;
			break;
		}
	}

	/// Get pointer of a vertex
	const PrimAsmVertex *getVertex(unsigned id) const {
		assert(id > 0 && id < 3);
		switch (id)
		{
		case 0:
			return &vtx0;
		case 1:
			return &vtx1;
		case 2:
			return &vtx2;
		default:
			return nullptr;
		}
		return nullptr;
	}

};

class Primitive
{
	PrimAsmMode mode;
	std::vector<std::unique_ptr<PrimAsmTriangle>> triangles;

public:

	/// Constructor
	///
	/// Create primitive collection from raw position data
	/// \param mode Mode of primitives to be generated
	/// \param pos_repo Repository of position data from export module
	/// \param x X as set in ViewPort
	/// \param y Y as set in ViewPort
	/// \param width Width as set in ViewPort
	/// \param height Height as set in ViewPort
	Primitive(PrimAsmMode mode, std::vector<std::unique_ptr<ExportData>> &pos_repo, 
		ViewPort *viewport);

	/// Getters
	///
	/// Return an iterator to the first triangle in the triangle repository. The
	/// following code can then be used to iterate over all triangles
	///
	/// \code
	/// for (auto i = primitive->TriangleBegin(),
	///		e = pritmitive->TriangleEnd(); i != e; ++i)
	/// {
	/// 	CODE
	/// }
	/// \endcode
	std::vector<std::unique_ptr<PrimAsmTriangle>>::iterator TriangleBegin() { 
		return triangles.begin(); }

	/// Return a past-the-end iterator to the list of triangles
	std::vector<std::unique_ptr<PrimAsmTriangle>>::iterator TriangleEnd() { 
		return triangles.end(); }
};

} // namespace Driver

#endif
