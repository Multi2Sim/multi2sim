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

#include "ScanConverter.h"

namespace SI
{

// See Mesa driver for more details
ScanConvEdge::ScanConvEdge(const PrimAsmVertex &vtx0, const PrimAsmVertex &vtx1)
{
	// for x/y coord snapping 
	const int snapMask = ~((FIXED_ONE / (1 << SUB_PIXEL_BITS)) - 1); 

	int vtx0_fx = FloatToFixed(vtx0.getX() + 0.5F) & snapMask;
	int vtx0_fy = FloatToFixed(vtx0.getY() + 0.5F) & snapMask;

	int vtx1_fx = FloatToFixed(vtx1.getX() + 0.5F) & snapMask;
	int vtx1_fy = FloatToFixed(vtx1.getY() + 0.5F) & snapMask;

	dx    = FixedToFloat(vtx0_fx - vtx1_fx);
	dy    = FixedToFloat(vtx0_fy - vtx1_fy);

	fsy   = FixedCeil(vtx1_fy);
	lines = FixedToInt(FixedCeil(vtx0_fy - fsy));

	if (lines > 0) 
	{
		dxdy  = dx / dy;
		fdxdy = SignedFloatToFixed(dxdy);
		adjy  = (float) (fsy - vtx1_fy);
		fx0   = vtx1_fx;
		fsx   = vtx0_fx + (int) (adjy * dxdy);
	}
}

PixelInfo::PixelInfo(int x, int y, int z)
{
	pos[0] = x;
	pos[1] = y;
	pos[2] = z;
	isWDCSet = true;
}

void PixelInfo::GenBaryCoord(const PrimAsmTriangle &triangle)
{
	assert(isWDCSet);

	float lamda1, lamda2, lamda3;
	float x, x1, x2, x3;
	float y, y1, y2, y3;
	float det;

	// 
	// Calculate barycentric coordinate based on current
	// pixel postion and positions of 3 associated vertex 
	//
	x = (float)pos[0];
	y = (float)pos[1];
	x1 = triangle.getVertexPos(0, 0);
	x2 = triangle.getVertexPos(1, 0);
	x3 = triangle.getVertexPos(2, 0);
	y1 = triangle.getVertexPos(0, 1);
	y2 = triangle.getVertexPos(1, 1);
	y3 = triangle.getVertexPos(2, 1);

	det = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
	lamda1 = (y2 - y3) * (x - x3) + (x3 - x2) * (y - y3);
	lamda1 /= det;
	lamda2 = (y3 - y1) * (x - x3) + (x1 - x3) * (y - y3);
	lamda2 /= det;
	lamda3 = 1 - lamda1 -lamda2;

	//
	// Only need to store lamda2 and lamda3 as AMD rearranges the formula.
	// See Southern Island ISA documentation Fig 9.3. It's P0 P10 P20, 
	// meaning P0, P1 -P0 and P2 -P0 respectively. LDS stores information of 
	// primitives such as color. So given color information of 3 vertices,
	// all pixels inside the triangle can get their color easily in fragment
	// shader. Interpolation was done this way in AMD's hardware.
	//
	i = lamda2;
	j = lamda3;

}

void ScanConverter::Rasterize(const PrimAsmTriangle &triangle, 
	const DepthBuffer &depth_buffer)
{

}


}  // namespace SI