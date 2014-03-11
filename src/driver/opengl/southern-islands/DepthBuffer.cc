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

namespace SI
{

DepthBuffer::DepthBuffer(unsigned width, unsigned height) :
	compare_func(GL_LESS), 
	clear_value(1.0f)
{
	this->width  = width;
	this->height = height;

	for (unsigned i = 0; i < width * height; ++i)
			buffer.push_back(std::unique_ptr<double> (new double(0.0f)));
}

void DepthBuffer::Resize(unsigned width, unsigned height)
{
	assert(width > 0 && height > 0);

	unsigned count = this->width * this->height;
	unsigned new_count = width * height;
	bool isIncrease = count < new_count ? true : false;

	if (isIncrease)
		for (int i = 0; i < abs(new_count - count); ++i)
			buffer.push_back(std::unique_ptr<double> (new double(0.0f)));
	else
		for (int i = 0; i < abs(new_count - count); ++i)
			buffer.pop_back();
}

void DepthBuffer::Resize(unsigned width, unsigned height, double clear_value)
{
	buffer.clear();

	for (unsigned i = 0; i < width * height; ++i)
			buffer.push_back(std::unique_ptr<double> (new double(clear_value)));
}

void DepthBuffer::Clear()
{
	for (unsigned i = 0; i < buffer.size(); ++i)
		*buffer[i] = clear_value;
}

void DepthBuffer::Clear(double value)
{
	for (unsigned i = 0; i < buffer.size(); ++i)
		*buffer[i] = value;
}

bool DepthBuffer::isPass(unsigned x, unsigned y, double value) const
{
	assert(x > 0 && x < width && y > 0 && y < height);

	bool is_pass;
	double depth = *buffer[y * width + x];

	switch(compare_func)
	{
		case GL_NEVER:
			is_pass = true;
			break;
		case GL_LESS:
			is_pass = value < depth ? true : false;
			break;
		case GL_EQUAL:
			is_pass = value == depth ? true : false;
			break;
		case GL_LEQUAL:
			is_pass = value <= depth ? true : false;
			break;
		case GL_GREATER:
			is_pass = value > depth ? true : false;
			break;
		case GL_NOTEQUAL:
			is_pass = value != depth ? true : false;
			break;
		case GL_GEQUAL:
			is_pass = value >= depth ? true : false;
			break;
		case GL_ALWAYS:
			is_pass = 1;
			break;
		default:
			is_pass = value < depth ? true : false;
			break;
	}

	return is_pass;
}

bool DepthBuffer::isPass(unsigned x, unsigned y, float value) const
{
	return isPass(x, y, (float)value);
}


}  // namespace SI
