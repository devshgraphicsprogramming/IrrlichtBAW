#include <irrlicht.h>

#include "../../../include/irr/asset/filters/CSummedAreaTableImageFilter.h"
#include "../ext/ScreenShot/ScreenShot.h"

using namespace irr;
using namespace core;
using namespace asset;
using namespace video;

/*
	Discrete convolution for getting input image after SAT calculations

	- support [-1.5,1.5]
	- (weight = -1) in [-1.5,-0.5]
	- (weight = 1) in [-0.5,0.5]
	- (weight = 0) in [0.5,1.5] and in range over the support
*/

using CDiscreteConvolutionRatioForSupport = std::ratio<3, 2>; //!< 1.5
class CDiscreteConvolutionFilterKernel : public CFloatingPointIsotropicSeparableImageFilterKernelBase<CDiscreteConvolutionFilterKernel, CDiscreteConvolutionRatioForSupport >
{
	using Base = CFloatingPointIsotropicSeparableImageFilterKernelBase<CDiscreteConvolutionFilterKernel, CDiscreteConvolutionRatioForSupport >;

public:
	inline float weight(float x) const
	{
		if (x >= -1.5f && x <= -0.5f)
			return -1.0f;
		else if (x >= -0.5f && x <= 0.5f)
			return 1.0f;
		else
			return 0.0f;
	}

};

int main()
{
	irr::SIrrlichtCreationParameters params;
	params.Bits = 32;
	params.ZBufferBits = 24;
	params.DriverType = video::EDT_OPENGL;
	params.WindowSize = dimension2d<uint32_t>(1600, 900);
	params.Fullscreen = false;
	params.Doublebuffer = true;
	params.Vsync = true;
	params.Stencilbuffer = false;

	auto device = createDeviceEx(params);
	if (!device)
		return false;

	device->getCursorControl()->setVisible(false);
	auto driver = device->getVideoDriver();
	auto assetManager = device->getAssetManager();
	auto sceneManager = device->getSceneManager();

	IAssetLoader::SAssetLoadParams lp(0ull, nullptr, IAssetLoader::ECF_DONT_CACHE_REFERENCES);


	auto bundle = assetManager->getAsset("../kernelImageTest.exr", lp);
	auto cpuImage = core::smart_refctd_ptr_static_cast<asset::ICPUImage>(bundle.getContents().begin()[0]);

	ICPUImageView::SCreationParams viewParams;
	viewParams.flags = static_cast<ICPUImageView::E_CREATE_FLAGS>(0u);
	viewParams.image = cpuImage;
	viewParams.format = viewParams.image->getCreationParameters().format;
	viewParams.viewType = IImageView<ICPUImage>::ET_2D;
	viewParams.subresourceRange.baseArrayLayer = 0u;
	viewParams.subresourceRange.layerCount = cpuImage->getCreationParameters().arrayLayers;
	viewParams.subresourceRange.baseMipLevel = 0u;
	viewParams.subresourceRange.levelCount = cpuImage->getCreationParameters().mipLevels;

	auto cpuImageView = ICPUImageView::create(std::move(viewParams));
	assert(cpuImageView.get(), "The imageView didn't pass creation validation!");

	auto getConvolutedImage = [&](const core::smart_refctd_ptr<ICPUImage> inImage) -> core::smart_refctd_ptr<ICPUImage>
	{
		auto outImage = core::move_and_static_cast<ICPUImage>(inImage->clone());

		using DISCRETE_CONVOLUTION_BLIT_FILTER = asset::CBlitImageFilter<CDiscreteConvolutionFilterKernel>;
		DISCRETE_CONVOLUTION_BLIT_FILTER blitImageFilter;
		DISCRETE_CONVOLUTION_BLIT_FILTER::state_type state;
		
		core::vectorSIMDu32 extentLayerCount;
		state.inMipLevel = 0;
		state.outMipLevel = 0;
		extentLayerCount = core::vectorSIMDu32(0, 0, 0, inImage->getCreationParameters().arrayLayers) + inImage->getMipSize(0);
	
		state.inOffsetBaseLayer = core::vectorSIMDu32();
		state.inExtentLayerCount = extentLayerCount;
		state.inImage = inImage.get();

		state.outOffsetBaseLayer = core::vectorSIMDu32();
		state.outExtentLayerCount = extentLayerCount;
		state.outImage = outImage.get();

		state.scratchMemoryByteSize = blitImageFilter.getRequiredScratchByteSize(&state);
		state.scratchMemory = reinterpret_cast<uint8_t*>(_IRR_ALIGNED_MALLOC(state.scratchMemoryByteSize, 32));

		if (!blitImageFilter.execute(&state))
			os::Printer::log("Something went wrong while performing discrete convolution operation!", ELL_WARNING);

		_IRR_ALIGNED_FREE(state.scratchMemory);

		return outImage;
	};

	{
		_IRR_STATIC_INLINE_CONSTEXPR std::string_view outputFile = "CONVOLUTED_IMAGE.exr";

		auto convolutedImage = getConvolutedImage(cpuImage);
		viewParams.image = convolutedImage;

		auto cpuImageView = ICPUImageView::create(std::move(viewParams));
		assert(cpuImageView.get(), "The imageView didn't pass creation validation!");

		asset::IAssetWriter::SAssetWriteParams wparams(cpuImageView.get());
		assetManager->writeAsset(outputFile.data(), wparams);
	}
}