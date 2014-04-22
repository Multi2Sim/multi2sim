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

#include <arch/southern-islands/asm/Arg.h>
#include <arch/southern-islands/emu/NDRange.h>
#include <arch/southern-islands/emu/WorkGroup.h>
#include <arch/southern-islands/emu/Wavefront.h>
#include <arch/southern-islands/emu/WorkItem.h>


#include <driver/opengl/OpenGLDriver.h>

#include "DepthBuffer.h"
#include "ScanConverter.h"
#include "ShaderExport.h"
#include "SPI.h"
#include "PrimitiveAssembler.h"

namespace SI
{

std::unique_ptr<SPI> SPI::instance;

SPI::SPI()
{
	
}

SPI *SPI::getInstance()
{
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

	unsigned id = ndrange->getId();

	for( auto &elem : init_data_repo)
	{
		if (elem->getId() == id)
			ndrange->ReceiveInitData(std::move(elem));
	}
}

void SPI::setViewport(int x, int y, int width, int height)
{
	viewport.reset(new ViewPort(x, y, width, height));
	depth_buffer.reset(new DepthBuffer((unsigned)width, (unsigned)height));
}

void SPI::genNDRange(ShaderExport *sx)
{
	Driver::OpenGLSIDriver *opengl_driver = Driver::OpenGLSIDriver::getInstance();

	// Currently only process export target pos[0]
	std::vector<std::unique_ptr<ExportData>> &positions = 
		opengl_driver->getShaderExportModule()->getExportTarget(12);

	// Currently only support triangle
	std::unique_ptr<Primitive> triangles(new Primitive(OpenGLPaTriangles, positions, viewport.get()));

	// 1 NDRange per primitive
	for( auto i = triangles.get()->TriangleBegin(), e = triangles.get()->TriangleEnd(); i != e; i++)
	{
		// FIXME: should pass SI Emu pointer
		// Create a NDRange
		std::unique_ptr<NDRange> ndrange(new NDRange(nullptr));

		// Rasterize a triangle
		std::unique_ptr<ScanConverter> sc(new ScanConverter());
		sc->Rasterize((*i).get(), depth_buffer.get());
		
		// Prepare initialization data(GPRs)
		DataForPixelShader init_data(ndrange->getId());
		for( auto pi = sc->PixelInfoBegin(), pe = sc->PixelInfoEnd(); pi != pe; pi++)
			init_data.setVGPRs((*pi).get());

		// Release ScanConverter
		sc.reset();
		
		// FIXME: Prepare initialization data(LDS)
		InitDataToNDRange(ndrange.get());

		// FIXME: Setup NDRange dimensions 

		// Add to central NDRange repository
		opengl_driver->AddNDRange(std::move(ndrange));
	}

	// Finished NDRange creation, export target can be clear
	opengl_driver->getShaderExportModule()->ClearAll();

}

}  // namespace SI