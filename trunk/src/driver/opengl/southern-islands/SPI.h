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

#ifndef DRIVER_OPENGL_SI_SPI_H
#define DRIVER_OPENGL_SI_SPI_H

#include <memory>
#include <vector>

namespace Driver
{
	class OpenGLSIDriver;
}  // namespace Driver

namespace SI
{

// Forward declaration
class PixelInfo;

// Pixels(WorkItems) created for Pixel Shader need to load their barycentric 
// coordinate into VGPRs, which will work together with data in LDS(color...)
class DataInitVGPRs
{
	// Window Space Coordinate
	int x;
	int y;

	// Barycentric Coordinate
	float i;
	float j;

public:
	DataInitVGPRs(int x, int y, float i, float j) :
		x(x), y(y), i(i), j(j) { };

	DataInitVGPRs(const PixelInfo *pixel_info);
};

// Data load to LDS()
class DataInitLDS
{
	std::unique_ptr<char []> lds;

public:
	DataInitLDS(const char *buffer, unsigned size);
};

class DataInitPixelShader
{
	// Contain data to initialize VGPRs
	std::vector<std::unique_ptr<DataInitVGPRs>> vgpr_init_data;
	std::unique_ptr<DataInitLDS> lds_init_data;

public:
	DataInitPixelShader();
};

// Shader Processor Interpolator
class SPI
{
	static Driver::OpenGLSIDriver *opengl_driver;

	// Unique instance of SPI
	static std::unique_ptr<SPI> instance;

	// Private constructor. The only possible instance of the SPI
	// can be obtained with a call to getInstance()
	SPI();

	std::vector<std::unique_ptr<DataInitPixelShader>> pixel_shader_init_data;

public:

	/// Get the only instance of the OpenGL Driver. If the instance does not
	/// exist yet, it will be created, and will remain allocated until the
	/// end of the execution.
	static SPI *getInstance();
	
};

} // namespace SI

#endif
