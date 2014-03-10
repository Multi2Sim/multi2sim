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

#ifndef DRIVER_OPENGL_SI_DEPTH_BUFFER_H
#define DRIVER_OPENGL_SI_DEPTH_BUFFER_H

#include <memory>
#include <vector>
#include <src/lib/cpp/Misc.h>

using namespace misc;

namespace SI
{

enum DepthBufferCompareFunc 
{
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS	
};

class DepthBuffer
{
	// FIXME: maybe need to make it a part of global memory
	std::vector<std::unique_ptr<double>> buffer;

	unsigned width;
	unsigned height;

	DepthBufferCompareFunc compare_func;
	double clear_value;

public:
	// Constructor, width/height = window width/height
	DepthBuffer(unsigned width, unsigned height);

	/// Getters
	///
	/// Get depth value at position (x, y)
	double getValue(unsigned x, unsigned y) const {
		assert( x > 0 && x < width && y > 0 && y < height);
		return *buffer[y * width + x];
	}

	/// Setters
	///
	/// Set clear value;
	void setClearValue(double value) { clear_value = value; }

	/// Set compare function
	void setCompareFunc(DepthBufferCompareFunc func) { compare_func = func; }

	/// Resize Depth Buffer but doesn't clear content
	///
	/// \param width Width of new size
	/// \param heigh Height of new size
	void Resize(unsigned width, unsigned height);

	/// Resize depth buffer and clear all content with clear value
	///
	/// \param width Width of new size
	/// \param height Height of new size
	void Resize(unsigned width, unsigned height, double clear_value);

	/// Clear depth buffer, using clear value(default: 1.0)
	void Clear();

	/// Clear depth buffer with a clear value
	///
	/// \param value Value used to clear the whole depth buffer
	void Clear(double value);

	/// Check if a value passes depth test at certain position
	///
	/// \param x X position
	/// \param y Y position
	/// \param value The value to be compared
	bool isPass (unsigned x, unsigned y, double value) const;

	bool isPass(unsigned x, unsigned y, float value) const;

};

} // namespace SI

#endif
