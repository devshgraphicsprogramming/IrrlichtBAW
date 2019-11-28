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

#include "CBufferLoaderBIN.h"

//#ifdef _IRR_COMPILE_WITH_GLI_
#include "gli/gli.hpp"
//#else
//	#error "Need GLI"
//#endif

namespace irr
{
	namespace asset
	{
		asset::SAssetBundle CBufferLoaderBIN::loadAsset(io::IReadFile* _file, const asset::IAssetLoader::SAssetLoadParams& _params, asset::IAssetLoader::IAssetLoaderOverride* _override, uint32_t _hierarchyLevel)
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
			
		    gli::gl GL(gli::gl::PROFILE_GL33);
			gli::gl::format const format = GL.translate(texture.format(), texture.swizzles());
			GLenum target = GL.translate(texture.target());
			
			/*GLuint textureName = 0;
			glGenTextures(1, &textureName);
			glBindTexture(target, textureName);
			glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
			glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, format.Swizzles[0]);
			glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, format.Swizzles[1]);
			glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, format.Swizzles[2]);
			glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, format.Swizzles[3]);*/
			
			glm::tvec3<GLsizei> const extent(texture.extent()); // it is on 0 level without specyfing it explicitly
			GLsizei const totalFaces = static_cast<GLsizei>(texture.layers() * texture.faces());

			ICPUImage::SCreationParams imageInfo;
			imageInfo.format = format;
			imageInfo.extent.width = extent.x;
			imageInfo.extent.height = extent.y;
			imageInfo.extent.depth = extent.z;
			imageInfo.mipLevels = texture.levels();
			imageInfo.arrayLayers = texture.layers();
			imageInfo.samples = ICPUImage::ESCF_1_BIT;
			imageInfo.flags = static_cast<IImage::E_CREATE_FLAGS>(0u);
					
			// to change
			for (std::size_t layer = 0; layer < texture.layers(); ++layer)
				for (std::size_t face = 0; face < texture.faces(); ++face)
					for (std::size_t level = 0; level < texture.levels(); ++level)
					{
						GLsizei const layerGL = static_cast<GLsizei>(layer);
						glm::tvec3<GLsizei> extent(texture.extent(level));
						target = gli::is_target_cube(texture.target()) ? static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face) : target;
			
						switch (texture.target())
						{
							case gli::TARGET_1D:
							{
								imageInfo.type = ICPUImage::ET_1D;
								/*if (gli::is_compressed(texture.format()))
									glCompressedTexSubImage1D(Target, static_cast<GLint>(Level), 0, Extent.x, Format.Internal, static_cast<GLsizei>(Texture.size(Level)),Texture.data(Layer, Face, Level));
								else
									glTexSubImage1D(Target, static_cast<GLint>(Level), 0, Extent.x, Format.External, Format.Type, Texture.data(Layer, Face, Level));*/
								break;
							}

							case gli::TARGET_1D_ARRAY:
							{
								break;
							}

							case gli::TARGET_2D:
							{
								imageInfo.type = ICPUImage::ET_2D;
								break;
							}

							case gli::TARGET_CUBE:
							{
								imageInfo.type = ICPUImage::ET_2D;
								/*if (gli::is_compressed(Texture.format()))
									glCompressedTexSubImage2D(Target, static_cast<GLint>(Level), 0, 0, Extent.x, Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y, Format.Internal, static_cast<GLsizei>(Texture.size(Level)), Texture.data(Layer, Face, Level));
								else
									glTexSubImage2D(Target, static_cast<GLint>(Level), 0, 0, Extent.x, Texture.target() == gli::TARGET_1D_ARRAY ? LayerGL : Extent.y, Format.External, Format.Type, Texture.data(Layer, Face, Level));*/
								break;
							}
							
							case gli::TARGET_2D_ARRAY:
							{
								break;
							}
					
							case gli::TARGET_3D:
							{
								break;
							}

							case gli::TARGET_CUBE_ARRAY:
							{
								/*if (gli::is_compressed(Texture.format()))
									glCompressedTexSubImage3D(Target, static_cast<GLint>(Level), 0, 0, 0, Extent.x, Extent.y, Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL, Format.Internal, static_cast<GLsizei>(Texture.size(Level)), Texture.data(Layer, Face, Level));
								else
									glTexSubImage3D(Target, static_cast<GLint>(Level), 0, 0, 0, Extent.x, Extent.y, Texture.target() == gli::TARGET_3D ? Extent.z : LayerGL, Format.External, Format.Type, Texture.data(Layer, Face, Level));*/
								break;
							}
					
							default: 
							{
								assert(0);
								break;
							}
						}
					}

			ctx.file = nullptr;
			core::smart_refctd_ptr<ICPUImage> image = nullptr;
			image = ICPUImage::create(std::move(imageInfo));

			return SAssetBundle({ image });
		}

		bool CBufferLoaderBIN::isALoadableFileFormat(io::IReadFile* _file) const
		{
			return true; // gli provides a function to load files, but we can check files' signature actally if needed
		}
	}
}