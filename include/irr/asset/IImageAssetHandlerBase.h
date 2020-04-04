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

	private:
};

}
}

#endif // __IRR_I_IMAGE_ASSET_HANDLER_BASE_H_INCLUDED__
