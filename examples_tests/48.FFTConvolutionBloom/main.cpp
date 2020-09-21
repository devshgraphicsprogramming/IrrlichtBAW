#define _IRR_STATIC_LIB_
#include "FFT2D.h"
#include "../../ext/ScreenShot/ScreenShot.h"

int main()
{
	irr::SIrrlichtCreationParameters params;
	params.Bits = 24;
	params.ZBufferBits = 24;
	params.DriverType = irr::video::EDT_OPENGL;
	params.WindowSize = irr::core::dimension2d<uint32_t>(790, 447);
	params.Fullscreen = false;
	params.Vsync = true;
	params.Doublebuffer = true;
	params.Stencilbuffer = false;
	params.AuxGLContexts = 16;
	auto device = createDeviceEx(params);

	if (!device)
	{
		assert(false);
		return 1;
	}

	FFT2D inputImage(device, loadImage(device, "../inputImage.png", true));
	FFT2D kernelImage(device, loadImage(device, "../kernelImage.png"));

	inputImage.perfromFFT();
	kernelImage.perfromFFT();

	inputImage.multiply(kernelImage);

	inputImage.perfromIFFT();

	auto driver = device->getVideoDriver();

	auto blitFBO = driver->addFrameBuffer();
	blitFBO->attach(irr::video::EFAP_COLOR_ATTACHMENT0, std::move(inputImage.getInputImage().gpuImageView));

	while (device->run())
	{
		driver->beginScene();

		driver->blitRenderTargets(blitFBO, nullptr, false, false);

		driver->endScene();
	}

	auto screenShotFrameBuffer = irr::ext::ScreenShot::createDefaultFBOForScreenshoting(device);
	driver->blitRenderTargets(blitFBO, screenShotFrameBuffer, false, false);

	irr::ext::ScreenShot::createScreenShot(device, screenShotFrameBuffer->getAttachment(irr::video::EFAP_COLOR_ATTACHMENT0),
		"../result.png");

	return 0;
}
