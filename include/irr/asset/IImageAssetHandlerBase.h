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
			all the regions filled and texel buffer attached as well.
			Beware it doesn't validate input image, you have to
			make sure it's R8 format
		*/

		template<E_FORMAT inputR8Format = EF_R8_SRGB>
		static inline core::smart_refctd_ptr<ICPUImage> convertR8ToR8G8B8Image(core::smart_refctd_ptr<ICPUImage> image)
		{
			constexpr auto formatPair = matchAndReturnPairFormat<inputR8Format>();
			constexpr auto inputFormat = formatPair.first;
			constexpr auto outputFormat = formatPair.second;
			using CONVERSION_FILTER = CConvertFormatImageFilter<inputFormat, outputFormat>;

			core::smart_refctd_ptr<ICPUImage> newConvertedImage;
			{
				auto referenceImageParams = image->getCreationParameters();
				auto referenceBuffer = image->getBuffer();
				auto referenceRegions = image->getRegions();

				const auto newTexelOrBlockByteSize = asset::getTexelOrBlockBytesize(outputFormat);
				auto newImageParams = referenceImageParams;
				auto newCpuBuffer = core::make_smart_refctd_ptr<ICPUBuffer>(referenceBuffer->getSize() * newTexelOrBlockByteSize);
				auto newRegions = core::make_refctd_dynamic_array<core::smart_refctd_dynamic_array<ICPUImage::SBufferCopy>>(referenceRegions.size());

				newImageParams.format = outputFormat;

				uint64_t regionItr = {};
				for (auto newRegion = newRegions->begin(); newRegion != newRegions->end(); ++newRegion)
				{
					auto referenceRegion = referenceRegions.begin() + regionItr++;

					*newRegion = *referenceRegion;
					newRegion->bufferOffset = referenceRegion->bufferOffset * newTexelOrBlockByteSize;
				}
			
				CONVERSION_FILTER::state_type state;
				state.extent = newImageParams.extent;
				state.extentLayerCount = ? ; // why it's vector?
				state.inBaseLayer = 0;
				state.inImage = image.get();
				state.inMipLevel = ? ; // it's for per mip map, so region?
				state.inOffset = ? ; //old region->bufferOffset ?
				state.inOffsetBaseLayer = ? ; // probably old single layer byte size
				state.layerCount = newImageParams.arrayLayers;
				state.outBaseLayer = 0; // ?
				state.outMipLevel = ? ; // seems that it's per region
				state.outOffset = ? ; // new region->bufferOffset * R8G8B8 byte size ?
				state.outOffsetBaseLayer = ? ; // new offset in bytes to layer ? old layer byte size * R8G8B8 byte size

				newConvertedImage = ICPUImage::create(std::move(newImageParams));
				newConvertedImage->setBufferAndRegions(std::move(newCpuBuffer), newRegions);
				state.outImage = newConvertedImage.get();

				CONVERSION_FILTER convertFiler; // in that I think I can't execute it with single call
				if (!convertFiler.execute(&state))
					os::Printer::log("LOAD PNG: something went wrong while converting from R8 to R8G8B8 format!", ELL_WARNING);
			}

			return newConvertedImage;
		};

	private:

		template<E_FORMAT inputR8Format>
		static inline constexpr std::pair<E_FORMAT, E_FORMAT> matchAndReturnPairFormat()
		{
			bool status = true;
			if constexpr (inputR8Format == EF_R8_SINT)
				return std::make_pair(EF_R8_SINT, EF_R8G8B8_SINT);
			else if constexpr(inputR8Format == EF_R8_SNORM)
				return std::make_pair(EF_R8_SNORM, EF_R8G8B8_SNORM);
			else if constexpr(inputR8Format == EF_R8_SRGB)
				return std::make_pair(EF_R8_SRGB, EF_R8G8B8_SRGB);
			else if constexpr(inputR8Format == EF_R8_SSCALED)
				return std::make_pair(EF_R8_SSCALED, EF_R8G8B8_SSCALED);
			else if constexpr(inputR8Format == EF_R8_UINT)
				return std::make_pair(EF_R8_UINT, EF_R8G8B8_UINT);
			else if constexpr(inputR8Format == EF_R8_UNORM)
				return std::make_pair(EF_R8_UNORM, EF_R8G8B8_UNORM);
			else if constexpr(inputR8Format == EF_R8_USCALED)
				return std::make_pair(EF_R8_USCALED, EF_R8G8B8_USCALED);
			else
			{
				status = false;
				static_assert(status, "Invalid input R8 format type!");
			}
		}
};

}
}

#endif // __IRR_I_IMAGE_ASSET_HANDLER_BASE_H_INCLUDED__
