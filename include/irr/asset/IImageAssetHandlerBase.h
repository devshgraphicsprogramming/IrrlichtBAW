#ifndef __IRR_I_IMAGE_ASSET_HANDLER_BASE_H_INCLUDED__
#define __IRR_I_IMAGE_ASSET_HANDLER_BASE_H_INCLUDED__

#include "irr/core/core.h"

namespace irr
{
namespace asset
{

class IImageAssetHandlerBase : public virtual core::IReferenceCounted
{
	protected:

		IImageAssetHandlerBase() = default;
		virtual ~IImageAssetHandlerBase() = 0;

	public:

		static const uint32_t MAX_PITCH_ALIGNMENT = 8u;										             

		/*
			 Returns pitch for buffer row lenght, because
			 OpenGL cannot transfer rows with arbitrary padding
		*/

		static inline uint32_t calcPitchInBlocks(uint32_t width, uint32_t blockByteSize)       
		{
			auto rowByteSize = width * blockByteSize;
			for (uint32_t _alignment = MAX_PITCH_ALIGNMENT; _alignment > 1u; _alignment >>= 1u)
			{
				auto paddedSize = core::alignUp(rowByteSize, _alignment);
				if (paddedSize % blockByteSize)
					continue;
				return paddedSize / blockByteSize;
			}
			return width;
		}

		static inline core::vector3du32_SIMD calcPitchInBlocks(uint32_t width, uint32_t height, uint32_t depth, uint32_t blockByteSize)
		{
			constexpr auto VALUE_FOR_ALIGNMENT = 1;
			core::vector3du32_SIMD retVal;
			retVal.X = calcPitchInBlocks(width, blockByteSize);
			retVal.Y = calcPitchInBlocks(height, VALUE_FOR_ALIGNMENT);
			retVal.Z = calcPitchInBlocks(depth, VALUE_FOR_ALIGNMENT);
			return retVal;
		}

		/*
			Patch for not supported by OpenGL R8_SRGB formats.
			Returns converted image. Input image needs to have
			all the regions filled and texel buffer as well.

			@devshgraphicsprogramming it should support layers and mipmaps
			as well for GLI, do we have a filter that can handle it?
		*/

		static inline core::smart_refctd_ptr<ICPUImage> convertR8ToR8G8B8Image(core::smart_refctd_ptr<ICPUImage> image)
		{
			core::smart_refctd_ptr<ICPUImage> newConvertedImage;
			{
				auto copyImageForConverting = core::smart_refctd_ptr_static_cast<ICPUImage>(image->clone());
				auto copyImageParams = copyImageForConverting->getCreationParameters();
				auto copyBuffer = copyImageForConverting->getBuffer();
				auto copyRegion = copyImageForConverting->getRegions().begin();

				auto newCpuBuffer = core::make_smart_refctd_ptr<ICPUBuffer>(copyBuffer->getSize());
				memcpy(newCpuBuffer->getPointer(), copyBuffer->getPointer(), newCpuBuffer->getSize());

				auto newRegions = core::make_refctd_dynamic_array<core::smart_refctd_dynamic_array<ICPUImage::SBufferCopy>>(1u);
				ICPUImage::SBufferCopy& region = newRegions->front();
				region.imageSubresource.mipLevel = copyRegion->imageSubresource.mipLevel;
				region.imageSubresource.baseArrayLayer = copyRegion->imageSubresource.baseArrayLayer;
				region.imageSubresource.layerCount = copyRegion->imageSubresource.layerCount;
				region.bufferOffset = copyRegion->bufferOffset;
				region.bufferRowLength = copyRegion->bufferRowLength * asset::getTexelOrBlockBytesize(EF_R8G8B8_SRGB);
				region.bufferImageHeight = copyRegion->bufferImageHeight;
				region.imageOffset = copyRegion->imageOffset;
				region.imageExtent = copyRegion->imageExtent;

				CMatchedSizeInOutImageFilterCommon::state_type state;
				state.extent = copyImageParams.extent;

				switch (copyImageParams.format)
				{
				case asset::EF_R8_SRGB:
				{
					copyImageParams.format = EF_R8G8B8_SRGB;
					newConvertedImage = ICPUImage::create(std::move(copyImageParams));
					newConvertedImage->setBufferAndRegions(std::move(newCpuBuffer), newRegions);
					state.inImage = newConvertedImage.get();

					CConvertFormatImageFilter<EF_R8_SRGB, EF_R8G8B8_SRGB> convertFiler;
					if (!convertFiler.execute(&state))
						os::Printer::log("LOAD PNG: something went wrong while converting from R8 to R8G8B8 format!", ELL_WARNING);
				}
				break;

				default:
				{
					newConvertedImage = std::move(copyImageForConverting);
				}
				break;
				}
			}

			return newConvertedImage;
		};

	private:
};

}
}

#endif // __IRR_I_IMAGE_ASSET_HANDLER_BASE_H_INCLUDED__
