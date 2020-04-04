#ifndef _IRR_EXT_SCREEN_SHOT_INCLUDED_
#define _IRR_EXT_SCREEN_SHOT_INCLUDED_

#include "irrlicht.h"

#include "../source/Irrlicht/COpenGLBuffer.h"
#include "../source/Irrlicht/COpenGLExtensionHandler.h"
#include "irr/asset/IImageAssetHandlerBase.h"

namespace irr
{
	namespace ext
	{
		namespace ScreenShot
		{
			/*
				Creates useful FBO that may be used for instance 
				to fetch default render target color data attachment 
				after rendering scene.

				The usage is following:

				- create frame buffer using the function
				- blit framebuffers, driver->blitRenderTargets(nullptr, frameBuffer, false, false);
				Note that in the call above we don't want to copy depth and stencil buffer, but event though default
				FBO contains depth buffer.
				- pass frame buffer to performScreenShot(video::IFrameBuffer*)

				Notes:

				- color buffer is placed under video::EFAP_COLOR_ATTACHMENT0 attachment
				- depth buffer is placed under video::EFAP_DEPTH_ATTACHMENT attachment
			*/

			irr::video::IFrameBuffer* createDefaultFBOForScreenshoting(core::smart_refctd_ptr<IrrlichtDevice> device)
			{
				auto driver = device->getVideoDriver();

				auto createAttachement = [&](bool colorBuffer)
				{
					asset::ICPUImage::SCreationParams imgInfo;
					imgInfo.format = colorBuffer ? asset::EF_R8G8B8A8_SRGB : asset::EF_D24_UNORM_S8_UINT;
					imgInfo.type = asset::ICPUImage::ET_2D;
					imgInfo.extent.width = driver->getCurrentRenderTargetSize().Width;
					imgInfo.extent.height = driver->getCurrentRenderTargetSize().Height;
					imgInfo.extent.depth = 1u;
					imgInfo.mipLevels = 1u;
					imgInfo.arrayLayers = 1u;
					imgInfo.samples = asset::ICPUImage::ESCF_1_BIT;
					imgInfo.flags = static_cast<asset::IImage::E_CREATE_FLAGS>(0u);

					auto image = asset::ICPUImage::create(std::move(imgInfo));
					const auto texelFormatBytesize = getTexelOrBlockBytesize(image->getCreationParameters().format);

					auto texelBuffer = core::make_smart_refctd_ptr<asset::ICPUBuffer>(image->getImageDataSizeInBytes());
					auto regions = core::make_refctd_dynamic_array<core::smart_refctd_dynamic_array<asset::ICPUImage::SBufferCopy>>(1u);
					asset::ICPUImage::SBufferCopy& region = regions->front();

					region.imageSubresource.mipLevel = 0u;
					region.imageSubresource.baseArrayLayer = 0u;
					region.imageSubresource.layerCount = 1u;
					region.bufferOffset = 0u;
					region.bufferRowLength = image->getCreationParameters().extent.width;
					region.bufferImageHeight = 0u;
					region.imageOffset = { 0u, 0u, 0u };
					region.imageExtent = image->getCreationParameters().extent;

					image->setBufferAndRegions(std::move(texelBuffer), regions);

					asset::ICPUImageView::SCreationParams imgViewInfo;
					imgViewInfo.image = std::move(image);
					imgViewInfo.format = colorBuffer ? asset::EF_R8G8B8A8_SRGB : asset::EF_D24_UNORM_S8_UINT;
					imgViewInfo.viewType = asset::IImageView<asset::ICPUImage>::ET_2D;
					imgViewInfo.flags = static_cast<asset::ICPUImageView::E_CREATE_FLAGS>(0u);
					imgViewInfo.subresourceRange.baseArrayLayer = 0u;
					imgViewInfo.subresourceRange.baseMipLevel = 0u;
					imgViewInfo.subresourceRange.layerCount = imgInfo.arrayLayers;
					imgViewInfo.subresourceRange.levelCount = imgInfo.mipLevels;

					auto imageView = asset::ICPUImageView::create(std::move(imgViewInfo));
					auto gpuImageView = driver->getGPUObjectsFromAssets(&imageView.get(), &imageView.get() + 1)->front();

					return std::move(gpuImageView);
				};

				auto gpuImageViewDepthBuffer = createAttachement(false);
				auto gpuImageViewColorBuffer = createAttachement(true);

				auto frameBuffer = driver->addFrameBuffer();
				frameBuffer->attach(video::EFAP_DEPTH_ATTACHMENT, std::move(gpuImageViewDepthBuffer));
				frameBuffer->attach(video::EFAP_COLOR_ATTACHMENT0, std::move(gpuImageViewColorBuffer));

				return frameBuffer;
			};

			/*
				Create a ScreenShot with gpu image usage.
			*/

			bool createScreenShoot(core::smart_refctd_ptr<IrrlichtDevice> device, core::smart_refctd_ptr<video::IGPUImage> gpuImage, const std::string& outFileName)
			{
				auto driver = device->getVideoDriver();
				auto assetManager = device->getAssetManager();

				auto fetchedImageParams = gpuImage->getCreationParameters();
				auto image = asset::ICPUImage::create(std::move(fetchedImageParams));

				auto trueBufferRowLength = asset::IImageAssetHandlerBase::calcPitchInBlocks(fetchedImageParams.extent.width, asset::getTexelOrBlockBytesize(fetchedImageParams.format));

				auto regions = core::make_refctd_dynamic_array<core::smart_refctd_dynamic_array<asset::ICPUImage::SBufferCopy>>(1u);
				asset::ICPUImage::SBufferCopy& region = regions->front();

				region.imageSubresource.mipLevel = 0u;
				region.imageSubresource.baseArrayLayer = 0u;
				region.imageSubresource.layerCount = 1u;
				region.bufferOffset = 0u;
				region.bufferRowLength = trueBufferRowLength;
				region.bufferImageHeight = 0u;
				region.imageOffset = { 0u, 0u, 0u };
				region.imageExtent = image->getCreationParameters().extent;

				auto destinationBuffer = core::smart_refctd_ptr<video::IGPUBuffer>(driver->createDownStreamingGPUBufferOnDedMem(image->getImageDataSizeInBytes()));
				destinationBuffer->getBoundMemory()->mapMemoryRange(video::IDriverMemoryAllocation::EMCAF_READ, { 0u, destinationBuffer->getSize() });

				driver->copyImageToBuffer(gpuImage.get(), destinationBuffer.get(), 1, regions->begin());

				auto texelBuffer = core::make_smart_refctd_ptr<asset::ICPUBuffer>(image->getImageDataSizeInBytes());
				auto rawData = reinterpret_cast<uint8_t*>(destinationBuffer->getBoundMemory()->getMappedPointer());

				memcpy(texelBuffer->getPointer(), rawData, image->getImageDataSizeInBytes());

				image->setBufferAndRegions(std::move(texelBuffer), regions);

				asset::IAssetWriter::SAssetWriteParams wparams(image.get());
				return assetManager->writeAsset(outFileName, wparams);
			}

/*
//! TODO: HANDLE UNPACK ALIGNMENT
core::smart_refctd_ptr<video::IDriverFence> createScreenShot(video::IDriver* driver, video::IGPUImage* source, video::IGPUBuffer* destination, uint32_t sourceMipLevel=0u, size_t destOffset=0ull, bool implicitflush=true)
{
	// will change this, https://github.com/buildaworldnet/IrrlichtBAW/issues/148
	if (isBlockCompressionFormat(source->getCreationParameters().format))
		return nullptr;

	auto extent = source->getMipSize(sourceMipLevel);
	video::IGPUImage::SBufferCopy pRegions[1u] = { {destOffset,extent.x,extent.y,{static_cast<asset::IImage::E_ASPECT_FLAGS>(0u),sourceMipLevel,0u,1u},{0u,0u,0u},{extent.x,extent.y,extent.z}} };
	driver->copyImageToBuffer(source,destination,1u,pRegions);

	return driver->placeFence(implicitflush);
}

template<typename PathOrFile>
void writeBufferAsImageToFile(asset::IAssetManager* mgr, const PathOrFile& _outFile, core::vector2d<uint32_t> _size, asset::E_FORMAT _format, video::IGPUBuffer* buff, size_t offset=0ull, bool flipY=true)
{
	const uint32_t zero[3] = { 0,0,0 };
	const uint32_t sizeArray[3] = { _size.X,_size.Y,1u };
	auto img = core::make_smart_refctd_ptr<asset::CImageData>(nullptr, zero, sizeArray, 0u, _format);

	//! Wonder if we'll need it after Vulkan ?
	const auto rowSize = (img->getBytesPerPixel()*sizeArray[0]).getRoundedUpInteger();
	const auto imagePitch = img->getPitchIncludingAlignment();
	const uint8_t* inData = reinterpret_cast<const uint8_t*>(buff->getBoundMemory()->getMappedPointer());
	uint8_t* outData = reinterpret_cast<uint8_t*>(img->getData())+imagePitch*(flipY ? (sizeArray[1]-1u):0u);
	for (uint32_t y=0u; y<sizeArray[1]; y++)
	{
		std::move(inData,inData+rowSize,outData);
		inData += imagePitch;
		if (flipY)
			outData -= imagePitch;
		else
			outData += imagePitch;
	}

	asset::IAssetWriter::SAssetWriteParams wparams(img.get());
	mgr->writeAsset(_outFile, wparams);
}

template<typename PathOrFile>
void dirtyCPUStallingScreenshot(IrrlichtDevice* device, const PathOrFile& _outFile, video::IGPUImage* source, uint32_t sourceMipLevel = 0u, bool flipY=true)
{
	auto texSize = source->getSize();
	if (outputFormatOverride==asset::EF_UNKNOWN)
		outputFormatOverride = source->getColorFormat();

	auto buff = core::smart_refctd_ptr<video::IGPUBuffer>(driver->createDownStreamingGPUBufferOnDedMem((asset::getBytesPerPixel(outputFormatOverride)*texSize[0]*texSize[1]).getIntegerApprox()), core::dont_grab); // TODO
	buff->getBoundMemory()->mapMemoryRange(video::IDriverMemoryAllocation::EMCAF_READ,{0u,buff->getSize()});

	auto fence = ext::ScreenShot::createScreenShot(driver, source, buff.get(), sourceMipLevel, 0ull,true,outputFormatOverride);
	while (fence->waitCPU(1000ull, fence->canDeferredFlush()) == video::EDFR_TIMEOUT_EXPIRED) {}
	ext::ScreenShot::writeBufferAsImageToFile(assetManager, _outFile, { texSize[0],texSize[1] }, outputFormatOverride, buff.get(), 0ull, flipY);
}
*/


		} // namespace ScreenShot
	} // namespace ext
} // namespace irr

#endif // _IRR_EXT_SCREEN_SHOT_INCLUDED_
