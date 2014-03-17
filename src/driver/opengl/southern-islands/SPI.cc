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

#include <arch/southern-islands/emu/NDRange.h>
#include <driver/opengl/OpenGLDriver.h>

#include "ScanConverter.h"
#include "SPI.h"

namespace SI
{

std::unique_ptr<SPI> SPI::instance;

SPI *SPI::getInstance()
{
	opengl_driver = Driver::OpenGLSIDriver::getInstance();

	// Instance already exists
	if (instance.get())
		return instance.get();
	
	// Create instance
	instance.reset(new SPI());
	return instance.get();	
}

DataForVGPRs::DataForVGPRs(const PixelInfo *pixel_info)
{
	x = pixel_info->getX();
	y = pixel_info->getY();
	i = pixel_info->getI();
	j = pixel_info->getJ();
}

DataForLDS::DataForLDS(const char *buffer, unsigned size)
{
	lds.reset(new char[size]);
	for (unsigned i = 0; i < size; ++i)
		lds[i] = buffer[i];
}

void DataForPixelShader::setLDS(const char *buffer, unsigned size)
{
	lds_init_data.reset(new DataForLDS(buffer, size));
}

void DataForPixelShader::setVGPRs(const PixelInfo *pixel_info)
{
	vgpr_init_data.push_back(std::unique_ptr<DataForVGPRs>(new 
		DataForVGPRs(pixel_info)));
}

void DataForPixelShader::setVGPRs(int x, int y, float i, float j)
{
	vgpr_init_data.push_back(std::unique_ptr<DataForVGPRs>(new 
		DataForVGPRs(x, y, i, j)));
}

void SPI::InitDataToNDRange(NDRange *ndrange)
{
	assert(ndrange->getStage() == NDRangeStagePixelShader);

	unsigned id = ndrange->getID();

	for( auto &elem : init_data_repo)
	{
		if (elem->getID() == id)
			ndrange->ReceiveInitData(std::move(elem));
	}
}

}  // namespace SI