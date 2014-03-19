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

#include <driver/opengl/southern-islands/ShaderExport.h>
#include <driver/opengl/southern-islands/SPI.h>
#include "OpenGLDriver.h"

namespace Driver
{

// Unique instance of OpenGL driver
std::unique_ptr<OpenGLSIDriver> OpenGLSIDriver::instance;

OpenGLSIDriver::OpenGLSIDriver()
{
	// Obtain instance of emulators
	// si_emu = SI::Emu::getInstance();

	// Obtain instance of Shader Export module
	sx = SI::ShaderExport::getInstance();

	// Obtain instance of SPI module
	spi = SI::SPI::getInstance();
}

OpenGLSIDriver *OpenGLSIDriver::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create instance
	instance.reset(new OpenGLSIDriver());
	return instance.get();
}
	
}  // namespace Driver
