/*
MIT License
Copyright (c) 2019 AnastaZIuk
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "CGLILoader.h"

//#ifdef _IRR_COMPILE_WITH_GLI_
#include "gli/gli.hpp"
//#else
//	#error "Need GLI"
//#endif

namespace irr
{
	namespace asset
	{
		asset::SAssetBundle CGLILoader::loadAsset(io::IReadFile* _file, const asset::IAssetLoader::SAssetLoadParams& _params, asset::IAssetLoader::IAssetLoaderOverride* _override, uint32_t _hierarchyLevel)
		{
			if (!_file)
				return {};

			SContext ctx(_file->getSize());
			ctx.file = _file;

			const char* filename = _file->getFileName().c_str();
			gli::texture texture = gli::load(filename);
			if (texture.empty())
			{
				os::Printer::log("Failed to load texture at path ", filename, ELL_ERROR);
				return {};
			}
			
		    const gli::gl glVersion(gli::gl::PROFILE_GL33);
			const GLenum target = glVersion.translate(texture.target());
			const E_FORMAT format = getTranslatedGLIFormat(texture, glVersion);

			ICPUImage* rawImagesBundle = _IRR_NEW_ARRAY(ICPUImage, texture.levels());
			//const uint32_t dataShiftForImagesBundle = texture.levels() * texture.faces();

			for(std::size_t mipmapLevel = 0; mipmapLevel < texture.levels(); ++mipmapLevel)
			{
				glm::tvec3<GLsizei> extent(texture.extent(mipmapLevel));

				ICPUImage::SCreationParams imageInfo;
				imageInfo.format = format;
				imageInfo.extent.width = extent.x;
				imageInfo.extent.height = extent.y;
				imageInfo.extent.depth = extent.z;
				imageInfo.mipLevels = texture.levels();
				imageInfo.arrayLayers = texture.faces();
				imageInfo.samples = ICPUImage::ESCF_1_BIT;
				imageInfo.flags = static_cast<IImage::E_CREATE_FLAGS>(0u);

				/*
					According to https://www.khronos.org/opengl/wiki/Texture_Storage#Anatomy_of_storage,
					for each mipmap level there is a texture object
					that may be an array texture or a texture. If we
					handle an array texture object, the layer variable
					will be greater than 1. Furthermore if we deal with
					a cubemap, we have to consider it's faces. So note that
					array texture being a cubemap has some layers and each
					layer has faces.
				*/

				core::smart_refctd_ptr<ICPUImage>** imagesOnAMipMapLevelToHandle;
				*imagesOnAMipMapLevelToHandle = _IRR_NEW_ARRAY(core::smart_refctd_ptr<ICPUImage>, texture.layers());
				for(uint16_t i = 0; i < texture.layers(); ++i)
					imagesOnAMipMapLevelToHandle[i] = _IRR_NEW_ARRAY(core::smart_refctd_ptr<ICPUImage>, texture.faces());

				for (std::size_t layer = 0; layer < texture.layers(); ++layer)					// if it is an array of textures of certain type 
					for (std::size_t face = 0; face < texture.faces(); ++face)					// if it is a cube, otherwise there is only single face
					{	
						// TODO, written as pseudo code
						// core::smart_refctd_ptr<CCustomAllocatorCPUBuffer<>> buffer = CCustomAllocatorCPUBuffer<>(XSIZEX, XDATAX);	inaccessable!
						
						imagesOnAMipMapLevelToHandle[layer][face] = ICPUImage::create(std::move(imageInfo));

						// imagesOnAMipMapLevelToHandle[layer][face]->data = texture.data(layer, face, mipmapLevel);
						// imagesOnAMipMapLevelToHandle[layer][face]->size = texture.size(layer);
						// rawImagesBundle = imagesOnAMipMapLevelToHandle[0][0].get();    it is wrong, but we need a way to bind rawImagesBundle and imagesOnAMipMapLevelToHandle in a way iterating through it will be valid
					}
			}

			ctx.file = nullptr;

			return SAssetBundle({ core::smart_refctd_ptr<IAsset>(rawImagesBundle, core::dont_grab) });
		}

		bool CGLILoader::isALoadableFileFormat(io::IReadFile* _file) const
		{
			return true; // gli provides a function to load files, but we can check files' signature actually if needed
		}

		inline E_FORMAT CGLILoader::getTranslatedGLIFormat(const gli::texture& texture, const gli::gl& glVersion)
		{
			using namespace gli;
			gli::gl::format formatToTranslate = glVersion.translate(texture.format(), texture.swizzles());

			// TODO
			switch (formatToTranslate.Internal)
			{
				case gl::INTERNAL_RGB_UNORM: return EF_R8G8B8_UNORM;			//GL_RGB
				case gl::INTERNAL_BGR_UNORM: return;	//GL_BGR
				case gl::INTERNAL_RGBA_UNORM: return;		//GL_RGBA
				case gl::INTERNAL_BGRA_UNORM: return;		//GL_BGRA
				case gl::INTERNAL_BGRA8_UNORM: return;		//GL_BGRA8_EXT

				// unorm formats
				case gl::INTERNAL_R8_UNORM: return;			//GL_R8
				case gl::INTERNAL_RG8_UNORM: return;		//GL_RG8
				case gl::INTERNAL_RGB8_UNORM: return;		//GL_RGB8
				case gl::INTERNAL_RGBA8_UNORM: return;		//GL_RGBA8

				case gl::INTERNAL_R16_UNORM: return;		//GL_R16
				case gl::INTERNAL_RG16_UNORM: return;		//GL_RG16
				case gl::INTERNAL_RGB16_UNORM: return;		//GL_RGB16
				case gl::INTERNAL_RGBA16_UNORM: return;		//GL_RGBA16

				case gl::INTERNAL_RGB10A2_UNORM: return;	//GL_RGB10_A2
				case gl::INTERNAL_RGB10A2_SNORM_EXT: return;

				// snorm formats
				case gl::INTERNAL_R8_SNORM: return;			//GL_R8_SNORM
				case gl::INTERNAL_RG8_SNORM: return;		//GL_RG8_SNORM
				case gl::INTERNAL_RGB8_SNORM: return;		//GL_RGB8_SNORM
				case gl::INTERNAL_RGBA8_SNORM: return;		//GL_RGBA8_SNORM

				case gl::INTERNAL_R16_SNORM: return;		//GL_R16_SNORM
				case gl::INTERNAL_RG16_SNORM: return;		//GL_RG16_SNORM
				case gl::INTERNAL_RGB16_SNORM: return;		//GL_RGB16_SNORM
				case gl::INTERNAL_RGBA16_SNORM: return;		//GL_RGBA16_SNORM

				// unsigned integer formats
				case gl::INTERNAL_R8U: return;				//GL_R8UI
				case gl::INTERNAL_RG8U: return;				//GL_RG8UI
				case gl::INTERNAL_RGB8U: return;			//GL_RGB8UI
				case gl::INTERNAL_RGBA8U: return;			//GL_RGBA8UI

				case gl::INTERNAL_R16U: return;				//GL_R16UI
				case gl::INTERNAL_RG16U: return;			//GL_RG16UI
				case gl::INTERNAL_RGB16U: return;			//GL_RGB16UI
				case gl::INTERNAL_RGBA16U: return;			//GL_RGBA16UI

				case gl::INTERNAL_R32U: return;				//GL_R32UI
				case gl::INTERNAL_RG32U: return;			//GL_RG32UI
				case gl::INTERNAL_RGB32U: return;			//GL_RGB32UI
				case gl::INTERNAL_RGBA32U: return;			//GL_RGBA32UI

				case gl::INTERNAL_RGB10A2U: return;			//GL_RGB10_A2UI
				case gl::INTERNAL_RGB10A2I_EXT: return;

				// signed integer formats
				case gl::INTERNAL_R8I: return;				//GL_R8I
				case gl::INTERNAL_RG8I: return;				//GL_RG8I
				case gl::INTERNAL_RGB8I: return;			//GL_RGB8I
				case gl::INTERNAL_RGBA8I: return;			//GL_RGBA8I

				case gl::INTERNAL_R16I: return;				//GL_R16I
				case gl::INTERNAL_RG16I: return;			//GL_RG16I
				case gl::INTERNAL_RGB16I: return;			//GL_RGB16I
				case gl::INTERNAL_RGBA16I: return;			//GL_RGBA16I

				case gl::INTERNAL_R32I: return;				//GL_R32I
				case gl::INTERNAL_RG32I: return;			//GL_RG32I
				case gl::INTERNAL_RGB32I: return;			//GL_RGB32I
				case gl::INTERNAL_RGBA32I: return;			//GL_RGBA32I

				// Floating formats
				case gl::INTERNAL_R16F: return;				//GL_R16F
				case gl::INTERNAL_RG16F: return;			//GL_RG16F
				case gl::INTERNAL_RGB16F: return;			//GL_RGB16F
				case gl::INTERNAL_RGBA16F: return;			//GL_RGBA16F

				case gl::INTERNAL_R32F: return;				//GL_R32F
				case gl::INTERNAL_RG32F: return;			//GL_RG32F
				case gl::INTERNAL_RGB32F: return;			//GL_RGB32F
				case gl::INTERNAL_RGBA32F: return;			//GL_RGBA32F

				case gl::INTERNAL_R64F_EXT: return;			//GL_R64F
				case gl::INTERNAL_RG64F_EXT: return;		//GL_RG64F
				case gl::INTERNAL_RGB64F_EXT: return;		//GL_RGB64F
				case gl::INTERNAL_RGBA64F_EXT: return;		//GL_RGBA64F

				// sRGB formats
				case gl::INTERNAL_SR8: return;				//GL_SR8_EXT
				case gl::INTERNAL_SRG8: return;				//GL_SRG8_EXT
				case gl::INTERNAL_SRGB8: return;			//GL_SRGB8
				case gl::INTERNAL_SRGB8_ALPHA8: return;		//GL_SRGB8_ALPHA8

				// Packed formats
				case gl::INTERNAL_RGB9E5: return;			//GL_RGB9_E5
				case gl::INTERNAL_RG11B10F: return;			//GL_R11F_G11F_B10F
				case gl::INTERNAL_RG3B2: return;			//GL_R3_G3_B2
				case gl::INTERNAL_R5G6B5: return;			//GL_RGB565
				case gl::INTERNAL_RGB5A1: return;			//GL_RGB5_A1
				case gl::INTERNAL_RGBA4: return;			//GL_RGBA4

				case gl::INTERNAL_RG4_EXT: return;

				// Luminance Alpha formats
				case gl::INTERNAL_LA4: return;				//GL_LUMINANCE4_ALPHA4
				case gl::INTERNAL_L8: return;				//GL_LUMINANCE8
				case gl::INTERNAL_A8: return;				//GL_ALPHA8
				case gl::INTERNAL_LA8: return;				//GL_LUMINANCE8_ALPHA8
				case gl::INTERNAL_L16: return;				//GL_LUMINANCE16
				case gl::INTERNAL_A16: return;				//GL_ALPHA16
				case gl::INTERNAL_LA16: return;				//GL_LUMINANCE16_ALPHA16

				// Depth formats
				case gl::INTERNAL_D16: return;				//GL_DEPTH_COMPONENT16
				case gl::INTERNAL_D24: return;				//GL_DEPTH_COMPONENT24
				case gl::INTERNAL_D16S8_EXT: return;
				case gl::INTERNAL_D24S8: return;			//GL_DEPTH24_STENCIL8
				case gl::INTERNAL_D32: return;				//GL_DEPTH_COMPONENT32
				case gl::INTERNAL_D32F: return;				//GL_DEPTH_COMPONENT32F
				case gl::INTERNAL_D32FS8X24: return;		//GL_DEPTH32F_STENCIL8
				case gl::INTERNAL_S8_EXT: return;			//GL_STENCIL_INDEX8

				// Compressed formats
				case gl::INTERNAL_RGB_DXT1: return;						//GL_COMPRESSED_RGB_S3TC_DXT1_EXT
				case gl::INTERNAL_RGBA_DXT1: return;					//GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
				case gl::INTERNAL_RGBA_DXT3: return;					//GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
				case gl::INTERNAL_RGBA_DXT5: return;					//GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
				case gl::INTERNAL_R_ATI1N_UNORM: return;				//GL_COMPRESSED_RED_RGTC1
				case gl::INTERNAL_R_ATI1N_SNORM: return;				//GL_COMPRESSED_SIGNED_RED_RGTC1
				case gl::INTERNAL_RG_ATI2N_UNORM: return;				//GL_COMPRESSED_RG_RGTC2
				case gl::INTERNAL_RG_ATI2N_SNORM: return;				//GL_COMPRESSED_SIGNED_RG_RGTC2
				case gl::INTERNAL_RGB_BP_UNSIGNED_FLOAT: return;		//GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT
				case gl::INTERNAL_RGB_BP_SIGNED_FLOAT: return;			//GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT
				case gl::INTERNAL_RGB_BP_UNORM: return;					//GL_COMPRESSED_RGBA_BPTC_UNORM
				case gl::INTERNAL_RGB_PVRTC_4BPPV1: return;				//GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
				case gl::INTERNAL_RGB_PVRTC_2BPPV1: return;				//GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
				case gl::INTERNAL_RGBA_PVRTC_4BPPV1: return;			//GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
				case gl::INTERNAL_RGBA_PVRTC_2BPPV1: return;			//GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG
				case gl::INTERNAL_RGBA_PVRTC_4BPPV2: return;			//GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG
				case gl::INTERNAL_RGBA_PVRTC_2BPPV2: return;			//GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG
				case gl::INTERNAL_ATC_RGB: return;						//GL_ATC_RGB_AMD
				case gl::INTERNAL_ATC_RGBA_EXPLICIT_ALPHA: return;		//GL_ATC_RGBA_EXPLICIT_ALPHA_AMD
				case gl::INTERNAL_ATC_RGBA_INTERPOLATED_ALPHA: return;	//GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD

				case gl::INTERNAL_RGB_ETC: return;						//GL_COMPRESSED_RGB8_ETC1
				case gl::INTERNAL_RGB_ETC2: return;						//GL_COMPRESSED_RGB8_ETC2
				case gl::INTERNAL_RGBA_PUNCHTHROUGH_ETC2: return;		//GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
				case gl::INTERNAL_RGBA_ETC2: return;					//GL_COMPRESSED_RGBA8_ETC2_EAC
				case gl::INTERNAL_R11_EAC: return;						//GL_COMPRESSED_R11_EAC
				case gl::INTERNAL_SIGNED_R11_EAC: return;				//GL_COMPRESSED_SIGNED_R11_EAC
				case gl::INTERNAL_RG11_EAC: return;						//GL_COMPRESSED_RG11_EAC
				case gl::INTERNAL_SIGNED_RG11_EAC: return;				//GL_COMPRESSED_SIGNED_RG11_EAC

				case gl::INTERNAL_RGBA_ASTC_4x4: return;				//GL_COMPRESSED_RGBA_ASTC_4x4_KHR
				case gl::INTERNAL_RGBA_ASTC_5x4: return;				//GL_COMPRESSED_RGBA_ASTC_5x4_KHR
				case gl::INTERNAL_RGBA_ASTC_5x5: return;				//GL_COMPRESSED_RGBA_ASTC_5x5_KHR
				case gl::INTERNAL_RGBA_ASTC_6x5: return;				//GL_COMPRESSED_RGBA_ASTC_6x5_KHR
				case gl::INTERNAL_RGBA_ASTC_6x6: return;				//GL_COMPRESSED_RGBA_ASTC_6x6_KHR
				case gl::INTERNAL_RGBA_ASTC_8x5: return;				//GL_COMPRESSED_RGBA_ASTC_8x5_KHR
				case gl::INTERNAL_RGBA_ASTC_8x6: return;				//GL_COMPRESSED_RGBA_ASTC_8x6_KHR
				case gl::INTERNAL_RGBA_ASTC_8x8: return;				//GL_COMPRESSED_RGBA_ASTC_8x8_KHR
				case gl::INTERNAL_RGBA_ASTC_10x5: return; 				//GL_COMPRESSED_RGBA_ASTC_10x5_KHR
				case gl::INTERNAL_RGBA_ASTC_10x6: return;				//GL_COMPRESSED_RGBA_ASTC_10x6_KHR
				case gl::INTERNAL_RGBA_ASTC_10x8: return;				//GL_COMPRESSED_RGBA_ASTC_10x8_KHR
				case gl::INTERNAL_RGBA_ASTC_10x10: return;				//GL_COMPRESSED_RGBA_ASTC_10x10_KHR
				case gl::INTERNAL_RGBA_ASTC_12x10: return;				//GL_COMPRESSED_RGBA_ASTC_12x10_KHR
				case gl::INTERNAL_RGBA_ASTC_12x12: return;				//GL_COMPRESSED_RGBA_ASTC_12x12_KHR

				// sRGB formats
				case gl::INTERNAL_SRGB_DXT1: return;					//GL_COMPRESSED_SRGB_S3TC_DXT1_EXT
				case gl::INTERNAL_SRGB_ALPHA_DXT1: return;				//GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
				case gl::INTERNAL_SRGB_ALPHA_DXT3: return;				//GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT
				case gl::INTERNAL_SRGB_ALPHA_DXT5: return;				//GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
				case gl::INTERNAL_SRGB_BP_UNORM: return;				//GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM
				case gl::INTERNAL_SRGB_PVRTC_2BPPV1: return;			//GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT
				case gl::INTERNAL_SRGB_PVRTC_4BPPV1: return;			//GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT
				case gl::INTERNAL_SRGB_ALPHA_PVRTC_2BPPV1: return;		//GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT
				case gl::INTERNAL_SRGB_ALPHA_PVRTC_4BPPV1: return;		//GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT
				case gl::INTERNAL_SRGB_ALPHA_PVRTC_2BPPV2: return;		//COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG
				case gl::INTERNAL_SRGB_ALPHA_PVRTC_4BPPV2: return;		//GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG
				case gl::INTERNAL_SRGB8_ETC2: return;						//GL_COMPRESSED_SRGB8_ETC2
				case gl::INTERNAL_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2: return;	//GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2
				case gl::INTERNAL_SRGB8_ALPHA8_ETC2_EAC: return;			//GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC
				case gl::INTERNAL_SRGB8_ALPHA8_ASTC_4x4: return;		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR
				case gl::INTERNAL_SRGB8_ALPHA8_ASTC_5x4: return;		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR
				case gl::INTERNAL_SRGB8_ALPHA8_ASTC_5x5: return;		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR
				case gl::INTERNAL_SRGB8_ALPHA8_ASTC_6x5: return;		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR
				case gl::INTERNAL_SRGB8_ALPHA8_ASTC_6x6: return;		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR
				case gl::INTERNAL_SRGB8_ALPHA8_ASTC_8x5: return;		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR
				case gl::INTERNAL_SRGB8_ALPHA8_ASTC_8x6: return;		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR
				case gl::INTERNAL_SRGB8_ALPHA8_ASTC_8x8: return;		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR
				case gl::INTERNAL_SRGB8_ALPHA8_ASTC_10x5: return;		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR
				case gl::INTERNAL_SRGB8_ALPHA8_ASTC_10x6: return;		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR
				case gl::INTERNAL_SRGB8_ALPHA8_ASTC_10x8: return;		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR
				case gl::INTERNAL_SRGB8_ALPHA8_ASTC_10x10: return;		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR
				case gl::INTERNAL_SRGB8_ALPHA8_ASTC_12x10: return;		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR
				case gl::INTERNAL_SRGB8_ALPHA8_ASTC_12x12: return;		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR

				case gl::INTERNAL_R8_USCALED_GTC: return;
				case gl::INTERNAL_R8_SSCALED_GTC: return;
				case gl::INTERNAL_RG8_USCALED_GTC: return;
				case gl::INTERNAL_RG8_SSCALED_GTC: return;
				case gl::INTERNAL_RGB8_USCALED_GTC: return;
				case gl::INTERNAL_RGB8_SSCALED_GTC: return;
				case gl::INTERNAL_RGBA8_USCALED_GTC: return;
				case gl::INTERNAL_RGBA8_SSCALED_GTC: return;
				case gl::INTERNAL_RGB10A2_USCALED_GTC: return;
				case gl::INTERNAL_RGB10A2_SSCALED_GTC: return;
				case gl::INTERNAL_R16_USCALED_GTC: return;
				case gl::INTERNAL_R16_SSCALED_GTC: return;
				case gl::INTERNAL_RG16_USCALED_GTC: return;
				case gl::INTERNAL_RG16_SSCALED_GTC: return;
				case gl::INTERNAL_RGB16_USCALED_GTC: return;
				case gl::INTERNAL_RGB16_SSCALED_GTC: return;
				case gl::INTERNAL_RGBA16_USCALED_GTC: return;
				case gl::INTERNAL_RGBA16_SSCALED_GTC: return;
				default: assert(0);
			}
		}
	}
}