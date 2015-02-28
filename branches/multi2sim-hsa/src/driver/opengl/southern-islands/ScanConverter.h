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

#ifndef DRIVER_OPENGL_SI_SCAN_CONVERTER_H
#define DRIVER_OPENGL_SI_SCAN_CONVERTER_H

#include "PrimitiveAssembler.h"

namespace SI
{

#define SPAN_MAX_WIDTH 16384
#define MAX_GLUINT	0xffffffff

#define PIXEL_TEST_PASS 1
#define PIXEL_TEST_FAIL 0

// Convert float to int by rounding to nearest integer, away from zero 
static inline int IROUND(float f)
{
	return (int) ((f >= 0.0F) ? (f + 0.5F) : (f - 0.5F));
}

#define SUB_PIXEL_BITS 4

// Fixed point arithmetic macros 
#ifndef FIXED_FRAC_BITS
#define FIXED_FRAC_BITS 11
#endif

#define FIXED_SHIFT     FIXED_FRAC_BITS
#define FIXED_ONE       (1 << FIXED_SHIFT)
#define FIXED_HALF      (1 << (FIXED_SHIFT-1))
#define FIXED_FRAC_MASK (FIXED_ONE - 1)
#define FIXED_INT_MASK  (~FIXED_FRAC_MASK)
#define FIXED_EPSILON   1
#define FIXED_SCALE     ((float) FIXED_ONE)
#define FIXED_DBL_SCALE ((double) FIXED_ONE)
#define FloatToFixed(X) (IROUND((X) * FIXED_SCALE))
#define FixedToDouble(X) ((X) * (1.0 / FIXED_DBL_SCALE))
#define IntToFixed(I)   ((I) << FIXED_SHIFT)
#define FixedToInt(X)   ((X) >> FIXED_SHIFT)
#define FixedToUns(X)   (((unsigned int)(X)) >> FIXED_SHIFT)
#define FixedCeil(X)    (((X) + FIXED_ONE - FIXED_EPSILON) & FIXED_INT_MASK)
#define FixedFloor(X)   ((X) & FIXED_INT_MASK)
#define FixedToFloat(X) ((X) * (1.0F / FIXED_SCALE))
#define PosFloatToFixed(X)      FloatToFixed(X)
#define SignedFloatToFixed(X)   FloatToFixed(X)

class DepthBuffer;
class PrimAsmVertex;

class ScanConvEdge
{
	float          dx;    // X(vtx1) - X(vtx0)
	float          dy;    // Y(vtx1) - Y(vtx0)
	float          dxdy;  // dx/dy
	int            fdxdy; // dx/dy in fixed-point
	float          adjy;  // adjust from v[0]->fy to fsy, scaled
	int            fsx;   // first sample point x coord
	int            fsy;   // first sample point y coord
	int            fx0;   // fixed pt X of lower endpoint
	int            lines; // number of lines to be sampled on this edge

	const PrimAsmVertex  *vtx0;
	const PrimAsmVertex  *vtx1;

public:
	ScanConvEdge(const PrimAsmVertex *vtx0, const PrimAsmVertex *vtx1);

	/// Getters
	///
	/// Get dx
	float getDx() const { return dx; }

	/// Get dy
	float getDy() const { return dy; }

	/// Get dxdy
	float getDxDy() const { return dxdy; }

	/// Get fdxdy
	int getFDxDy() const { return fdxdy; }

	/// Get adjy
	float getAdjy() const {return adjy; }

	/// Get fsx
	int getFsx() const { return fsx; }

	/// Get fsy
	int getFsy() const { return fsy; }

	int getFx0() const { return fx0; }

	/// Get number of lines on this edge
	int getLines() const { return lines; }

	/// Get pointer to vertex 0
	const PrimAsmVertex *getVertex(int index) {
		assert(index < 2);
		if (index == 0)
			return vtx0;
		else
			return vtx1;
		return nullptr;
	}

};

class ScanConvSpan
{
	// Coord of first fragment in horizontal span/run 
	int x;
	int y;

	float attrStart[4];   // initial value 
	float attrStepX[4];   // dvalue/dx
	float attrStepY[4];   // dvalue/dy

	int z;
	int zStep;

	// Number of fragments in the span 
	unsigned end;

	// fragment Z coords 
	unsigned frag_z[SPAN_MAX_WIDTH];  

public:
	ScanConvSpan();

	/// Getters
	int getX() const { return x; }

	int getY() const { return y; }

	int getZ() const { return z; }

	float getStepX(int index) const {
		assert(index < 4);
		return attrStepX[index];
	}

	float getStepY(int index) const {
		assert(index < 4);
		return attrStepY[index];
	}

	int getStepZ() const { return zStep; }

	unsigned getEnd() const { return end; }

	/// Setters
	void setX(int value) { x = value; }

	void setY(int value) { y = value; }

	void setZ(int value) { z = value; }

	void setStartX(int index, float value) {
		assert(index < 4);
		attrStart[index] = value;
	}

	void setStepX(int index, float value) {
		assert(index < 4);
		attrStepX[index] = value;
	}

	void setStepY(int index, float value) {
		assert(index < 4);
		attrStepY[index] = value;
	}

	void setStepZ(int value) { zStep = value; }

	void setEnd(unsigned value) { end = value; }

	void incX() { x++; }

	void incY() { y++; }

};

class PixelInfo
{
	// Positions in window coordinates
	bool isPosSet;
	int pos[4];

	// Barycentric coordinates to be load to VGPRs
	float i;
	float j;

public:
	PixelInfo(int x, int y, int z);

	/// Getters
	int getX() const { return pos[0]; }

	int getY() const { return pos[1]; }

	int getZ() const { return pos[2]; }

	int getW() const { return pos[3]; }

	float getI() const { return i; }

	float getJ() const { return j; }

	/// Generate baricentric coordinates
	void GenBaryCoord(const PrimAsmTriangle *triangle);

};

// The major functionality of ScanConverter is to get barycentric 
// coordinate of pixels inside a triangle/line, which will be loaded 
// to VGPRs in fragment shader
class ScanConverter
{
	bool scan_from_left_to_right;

	std::vector<std::unique_ptr<PixelInfo>> pixel_info_repo;

public:
	ScanConverter();

	/// Getters
	///
	/// Return an iterator to the first PixelInfo in the pixel_info_repo. The
	/// following code can then be used to iterate over all PixelInfo object
	///
	/// \code
	/// for (auto i = primitive->PixelInfoBegin(),
	///		e = pritmitive->PixelInfoEnd(); i != e; ++i)
	/// {
	/// 	CODE
	/// }
	/// \endcode
	std::vector<std::unique_ptr<PixelInfo>>::iterator PixelInfoBegin() { 
		return pixel_info_repo.begin(); }

	/// Return a past-the-end iterator to the list of triangles
	std::vector<std::unique_ptr<PixelInfo>>::iterator PixelInfoEnd() { 
		return pixel_info_repo.end(); }


	/// Rasterize an triangle and store pixel information in repository
	void Rasterize(const PrimAsmTriangle *triangle, const DepthBuffer *depth_buffer);

	/// Clear pixel_info_repo
	void Clear();
};

} // namespace SI

#endif
