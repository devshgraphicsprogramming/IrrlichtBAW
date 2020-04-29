#define _IRR_STATIC_LIB_
#include "ApplicationHandler.hpp"

using namespace irr;
using namespace core;

int main()
{
	irr::SIrrlichtCreationParameters params;
	params.Bits = 24; 
	params.ZBufferBits = 24; 
	params.DriverType = video::EDT_OPENGL; 
	params.WindowSize = dimension2d<uint32_t>(1280, 720);
	params.Fullscreen = false;
	params.Vsync = true; 
	params.Doublebuffer = true;
	params.Stencilbuffer = false;
	auto device = createDeviceEx(params);

	if (!device)
		return 1; 

	ApplicationHandler applicationHandler(device);
	if (!applicationHandler.getStatus())
		return 1;

	applicationHandler.work();
}