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

#include "DepthBuffer.h"
#include "ScanConverter.h"

namespace SI
{

// See Mesa driver for more details
ScanConvEdge::ScanConvEdge(const PrimAsmVertex *vtx0, const PrimAsmVertex *vtx1)
{
	this->vtx0 = vtx0;
	this->vtx1 = vtx1;

	// for x/y coord snapping 
	const int snapMask = ~((FIXED_ONE / (1 << SUB_PIXEL_BITS)) - 1); 

	int vtx0_fx = FloatToFixed(vtx0->getX() + 0.5F) & snapMask;
	int vtx0_fy = FloatToFixed(vtx0->getY() + 0.5F) & snapMask;

	int vtx1_fx = FloatToFixed(vtx1->getX() + 0.5F) & snapMask;
	int vtx1_fy = FloatToFixed(vtx1->getY() + 0.5F) & snapMask;

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
	isPosSet = true;
}

void PixelInfo::GenBaryCoord(const PrimAsmTriangle *triangle)
{
	assert(isPosSet);

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
	x1 = triangle->getVertexPos(0, 0);
	x2 = triangle->getVertexPos(1, 0);
	x3 = triangle->getVertexPos(2, 0);
	y1 = triangle->getVertexPos(0, 1);
	y2 = triangle->getVertexPos(1, 1);
	y3 = triangle->getVertexPos(2, 1);

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
	// primitives such as color, normal... So given color information of 3 vertices,
	// all pixels inside the triangle can get their color easily in fragment
	// shader. Interpolation was done this way in AMD's hardware.
	//
	i = lamda2;
	j = lamda3;

}

ScanConverter::ScanConverter()
{

}

ScanConvSpan::ScanConvSpan()
{
	
}

void ScanConverter::Rasterize(const PrimAsmTriangle *triangle, 
	const DepthBuffer *depth_buffer)
{
	const PrimAsmVertex *vtx_max;
	const PrimAsmVertex *vtx_mid;
	const PrimAsmVertex *vtx_min;
	
	// for x/y coord snapping 
	const int snapMask = ~((FIXED_ONE / (1 << SUB_PIXEL_BITS)) - 1);

	const int fy0 = FloatToFixed(triangle->getVertexPos(0, 1) - 0.5F) & snapMask;
	const int fy1 = FloatToFixed(triangle->getVertexPos(1, 1) - 0.5F) & snapMask;
	const int fy2 = FloatToFixed(triangle->getVertexPos(2, 1) - 0.5F) & snapMask;

	/* Find the order of vertex */
	if (fy0 <= fy1)
	{
		if (fy1 <= fy2)
		{
			/* y0 < y1 < y2 */
			vtx_max = triangle->getVertex(2); 
			vtx_mid = triangle->getVertex(1); 
			vtx_min = triangle->getVertex(0);
		}
		else if (fy2 <= fy0)
		{
			/* y2 < y0 < y1 */
			vtx_max = triangle->getVertex(1); 
			vtx_mid = triangle->getVertex(0); 
			vtx_min = triangle->getVertex(2);
		}
		else {
			/* y0 < y2 < y1 */
			vtx_max = triangle->getVertex(1); 
			vtx_mid = triangle->getVertex(2); 
			vtx_min = triangle->getVertex(0);
		}
	}
	else {
		if (fy0 <= fy2)
		{
			/* y1 < y0 < y2 */
			vtx_max = triangle->getVertex(2); 
			vtx_mid = triangle->getVertex(0); 
			vtx_min = triangle->getVertex(1);
		}
		else if (fy2 <= fy1)
		{
			/* y2 < y1 < y0 */
			vtx_max = triangle->getVertex(0); 
			vtx_mid = triangle->getVertex(1); 
			vtx_min = triangle->getVertex(2);
		}
		else {
			/* y1 < y2 < y0 */
			vtx_max = triangle->getVertex(0); 
			vtx_mid = triangle->getVertex(2); 
			vtx_min = triangle->getVertex(1);
		}
	}

	std::unique_ptr<ScanConvEdge> edge_major(new ScanConvEdge(vtx_max, vtx_min));
	std::unique_ptr<ScanConvEdge> edge_top(new ScanConvEdge(vtx_max, vtx_mid));
	std::unique_ptr<ScanConvEdge> edge_bottom(new ScanConvEdge(vtx_mid, vtx_min));

	// Compute area 	
	const float area = edge_major->getDx() * edge_bottom->getDy() - 
		edge_bottom->getDx() * edge_major->getDy();
	const float one_over_area = 1.0f / area;

	// Culled as major edge covers no lines
	if (edge_major->getLines() <= 0)
		return;

	// Decide scan direction 
	scan_from_left_to_right = one_over_area < 0.0F ? true : false;

	// Interpolate depth 
	float edge_major_dz = vtx_max->getComp(2) - vtx_min->getComp(2);
	float edge_bottom_dz = vtx_mid->getComp(2) - vtx_min->getComp(2);

	std::unique_ptr<ScanConvSpan> spn(new ScanConvSpan());

	spn->setStepX(2, one_over_area * (edge_major_dz * edge_bottom->getDy() - edge_major->getDy() * edge_bottom_dz));
	spn->setStepY(2, one_over_area * (edge_major->getDx() * edge_bottom_dz - edge_major_dz * edge_bottom->getDx()));
	spn->setStepZ(SignedFloatToFixed(spn->getStepX(2)));

	int subTriangle;
	int fxLeftEdge = 0, fxRightEdge = 0;
	int fdxLeftEdge = 0, fdxRightEdge = 0;
	int fError = 0, fdError = 0;

	unsigned int zLeft = 0;
	int fdzOuter = 0, fdzInner;

	// Setup order of edges 
	for (subTriangle=0; subTriangle<=1; subTriangle++)
	{
		ScanConvEdge *edge_left;
		ScanConvEdge *edge_right;
		int setupLeft, setupRight;
		int lines;

		if (subTriangle==0) {
			/* bottom half */
			if (scan_from_left_to_right) {
				edge_left = edge_major.get();
				edge_right = edge_bottom.get();
				lines = edge_right->getLines();
				setupLeft = 1;
				setupRight = 1;
			}
			else {
				edge_left = edge_bottom.get();
				edge_right = edge_major.get();
				lines = edge_left->getLines();
				setupLeft = 1;
				setupRight = 1;
			}
		} 
		else {
			/* top half */
			if (scan_from_left_to_right) 
			{
				edge_left = edge_major.get();
				edge_right = edge_top.get();
				lines = edge_right->getLines();
				setupLeft = 0;
				setupRight = 1;
			}
			else {
				edge_left = edge_top.get();
				edge_right = edge_major.get();
				lines = edge_left->getLines();
				setupLeft = 1;
				setupRight = 0;
			}
			if (lines == 0)
				return;
		}

		if (setupLeft && edge_left->getLines() > 0)
		{
			const PrimAsmVertex *vtx_lower = edge_left->getVertex(1);
			const int fsy = edge_left->getFsy();
			const int fsx = edge_left->getFsx();  /* no fractional part */
			const int fx = FixedCeil(fsx);  /* no fractional part */
			const int adjx = (int) (fx - edge_left->getFx0()); /* SCALED! */
			const int adjy = (int) edge_left->getAdjy();      /* SCALED! */
			int idxOuter;
			float dxOuter;			
			int fdxOuter;

			fError = fx - fsx - FIXED_ONE;
			fxLeftEdge = fsx - FIXED_EPSILON;
			fdxLeftEdge = edge_left->getFDxDy();
			fdxOuter = FixedFloor(fdxLeftEdge - FIXED_EPSILON);
			fdError = fdxOuter - fdxLeftEdge + FIXED_ONE;
			idxOuter = FixedToInt(fdxOuter);
			dxOuter = (float) idxOuter;
			spn->setY(FixedToInt(fsy));

			/* Interpolate Z */
			float z0 = vtx_lower->getComp(2);
			float tmp = (z0 * FIXED_SCALE + spn->getStepX(2) * adjx + spn->getStepY(2) * adjy) + FIXED_HALF;
			if (tmp < MAX_GLUINT / 2)
				zLeft = (int) tmp;
			else
				zLeft = MAX_GLUINT / 2;
			fdzOuter = SignedFloatToFixed(spn->getStepY(2) + dxOuter * spn->getStepX(2));								

		}

		if (setupRight && edge_right->getLines()>0) 
		{
			fxRightEdge = edge_right->getFsx() - FIXED_EPSILON;
			fdxRightEdge = edge_right->getFDxDy();
		}

		if (lines==0)
			continue;

		/* Interpolate Z */
		fdzInner = fdzOuter + spn->getStepZ();		

		/* Rasterize setup */
		while (lines > 0)
		{

			/* initialize the spn->interpolants to the leftmost value */
			/* ff = fixed-pt fragment */
			const int right = FixedToInt(fxRightEdge);
			spn->setX(FixedToInt(fxLeftEdge));
			if (right <= spn->getX())
				spn->setEnd(0);
			else
				spn->setEnd(right - spn->getX());

			/* Interpolate Z */
			spn->setZ(zLeft);

			/* This is where we actually generate fragments */
			if (spn->getEnd() > 0 && spn->getY() >= 0)
			{
				const int len = spn->getEnd();

				int i;
				for (i = 0; i < len; ++i)
				{
					// Add if pass depth test 
					if (depth_buffer->isPass(spn->getX(), spn->getY(), FixedToFloat(spn->getZ())))
					{
						pixel_info_repo.push_back(std::unique_ptr<PixelInfo> (new 
							PixelInfo(spn->getX(), spn->getY(), spn->getZ())));
					}
					spn->setZ(spn->getZ() + spn->getStepZ());
					spn->incX();
				}
			}

			/*
			* Advance to the next scan line.  Compute the new edge coordinates, and adjust the
			* pixel-center x coordinate so that it stays on or inside the major edge.
			*/
			spn->incY();
			lines--;

			fxLeftEdge += fdxLeftEdge;
			fxRightEdge += fdxRightEdge;

			fError += fdError;

			if (fError >= 0)
			{
				zLeft += fdzOuter;
				fError -= FIXED_ONE;				
			}
			else
				zLeft += fdzInner;
			
		} /*while lines>0*/
	}
}

void ScanConverter::Clear()
{
	pixel_info_repo.clear();
}

}  // namespace SI